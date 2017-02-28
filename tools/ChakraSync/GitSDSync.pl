# Chakra Git<->SD sync
# Brings in changes from SD and rebases Git changes on to them. Commits the result to Git and SD.
# TODO:
# - Auto push build branch
# - Cleanup OPTIONS case convention
# - Bare repo init for core
# - Fetch only target branches unless in debug mode
# - Repo cache when possible, be very paranoid
# - Quick check for Git changes using diff

use strict;
use warnings;

# The following libraries are distributed under Razzle, so don't dynamically check for their existence.
use POSIX qw(strftime);
my @ARGVSave = @ARGV; # Save ARGV since Getopt will destroy it. We need it to print the resume command later.
use Getopt::Long qw(GetOptions);
use IPC::Cmd qw(can_run run);
$IPC::Cmd::USE_IPC_OPEN3 = 1;
$IPC::Cmd::WARN = 0; # Suppress noisy warning about IPC::Run missing. We don't end up using this module.
use Pod::Usage qw(pod2usage);
use Cwd 'abs_path';
use File::Temp qw(tempfile);
use Storable;

use Capture::Tiny qw(capture_merged tee_merged);
use Data::Dumper;
use Log::Message::Simple;

use FindBin;
use lib "$FindBin::Bin\\";

our %OPTIONS;
require "Config.pl";
require "Common.pl";

use lib "$ENV{ProgramFiles}\\Git\\lib\\perl5\\site_perl";

# Globals
our $PRIVATE_SD_CHANGED  = 0;
our $PRIVATE_VSO_CHANGED = 0;
our $CORE_SD_CHANGED     = 0;
our $CORE_VSO_CHANGED    = 0;

sub chdirAndLog {
    my $dir = shift;
    msg("Changing directory to '$dir'", 1);
    chdir($dir);
}

sub chdirFull { chdirAndLog($OPTIONS{'SDEnlistment'}); }
sub chdirCore { chdirAndLog("$OPTIONS{'SDEnlistment'}\\core"); }

sub logHeader { msg("", 1); msg(':: ' . shift, 1); msg("", 1); msg("", 1); }

my $shortUsage = <<"END";
Chakra Git<->SD Sync Script

Usage:
    perl $0 Command --gitHash hash --sdChange change [options]

Commands:
    Sync commands:
        FullSync        Performs a SD to Git sync, then a Git to SD sync.
        SDToGit         Performs a SD to Git sync. Requires manual review of changes.
        GitToSD         Performs a Git to SD sync. Automatable if there are no SD changes.
        WriteSyncPoint  Overwrites the sync point used by the auto sync. Used after doing a fullsync.

    Utility commands:
        SDToGitMsg      Generates the commit message used in Git to convey SD changes.
        GitToSdMsg      Generates the changelist description used in SD to convey Git changes.
        StageSD         Stages all changes that are currently not in SD, including added/deleted files.
        StopAutoSync    Stop the Git to SD Automatic sync.
        StartAutoSync   Start the Git to SD Automatic sync.
        InitEnlistment  Create a superimposed Git respository on top of a SD enlistment.
                        Removes all existing state, and resets to the last sync point.
END

my $usage = $shortUsage . <<"END";

Mandatory parameters (excluding StopAutoSync/StartAutoSync):
    --SDChange CL#      The last synced SD change that had a corresponding Git commit.
    --GitHash hash      The last synced Git full hash that had a corresponding SD commit.

Optional parameters:
    Debugging:
        -v --verbose    Show status info. Use flag twice for more verbosity.
                        Default verbosity is currently: $OPTIONS{'Verbose'}
           --email      Send error and summary emails
           --no-cleanup Don't delete temporary branch
        -d --dry-run    Don't make any changes that could result in breakage and angry people
                        (Implies --no-email, default for FullSync)
           --cache-repo Don't blow away Git state during initialization (DO NOT USE OUTSIDE DEVELOPMENT)

    Locations:
        --sdEnlistment location     Root of the SD enlistment to use (jscript folder)
        --coreBranch branch         Remote core branch name (default: $OPTIONS{'CoreBranch'})
        --fullBranch branch         Remote full branch name (default: $OPTIONS{'FullBranch'})

    Ranges (experimental):
        --endGitHash hash   The upper limit of the Git full hash to sync.
        --endSDChange CL#   The upper limit of the SD change to sync. Cannot be used if there a
                            are incoming Git commits.

    Internal parameters (used by the script for state tracking):
        -r --resume             Resume rebasing Git after fixing a merge conflict
           --repo   [core|full] Resume in core or full
        --auto                  Use state data to fill in hash / change info

    Misc:
        -? --help                This message

Examples:
    perl $0 FullSync --gitHash 1d40249 --sdChange 1d40249       Performs a full sync

END

$shortUsage .= "\nUse --help to see full help.";

my $SYNC_DATA_LOCATION = "$OPTIONS{SDEnlistment}\\..\\..\\.chakraGitSyncPoint";

sub writeSyncPoint {
    if (!$OPTIONS{'Auto'}) {
        die "Sync points can be used in auto mode only";
    }

    $OPTIONS{'fullEndHash'} =~ /^\w+$/ or die "Invalid sync point git hash to save";
    $OPTIONS{'endSDChange'} =~ /^\d+$/ or die "Invalid sync point sd change to save";
    my %syncPointData = (
        sdChange => $OPTIONS{'endSDChange'},
        gitHash => $OPTIONS{'fullEndHash'}
    );

    # Store outside of inetcore, 'just to be sure.'
    store \%syncPointData, $SYNC_DATA_LOCATION;
    msg("Wrote sync point to $SYNC_DATA_LOCATION: Git Hash $syncPointData{'gitHash'}, SD Change $syncPointData{'sdChange'}", 1);
}

sub readSyncPoint {
    if (!$OPTIONS{'Auto'}) {
        die "Sync points can be used in auto mode only";
    }

    if ($OPTIONS{'fullStartHash'} ne '' && $OPTIONS{'startSDChange'} ne '') {
        msg('Overriding sync point using command line args', 1);
        return;
    }

    if (!-e $SYNC_DATA_LOCATION) {
        die "No sync data found - must be specified using the --gitHash and --sdChange command line options.";
    }
    my $syncPointDataRef = retrieve $SYNC_DATA_LOCATION;
    $syncPointDataRef->{'gitHash'}  =~ /^\w+$/ or die "Invalid sync point git hash read";
    $syncPointDataRef->{'sdChange'} =~ /^\d+$/ or die "Invalid sync point sd change to save";

    msg("Read sync point from $SYNC_DATA_LOCATION: Git Hash $syncPointDataRef->{'gitHash'}, SD Change $syncPointDataRef->{'sdChange'}", 1);

    $OPTIONS{'fullStartHash'} = $syncPointDataRef->{'gitHash'};
    $OPTIONS{'startSDChange'} = $syncPointDataRef->{'sdChange'};
}

sub parseCmd {
    $OPTIONS{'Command'} = shift @ARGV;

    #TODO Fold this into the main switch
    use feature qw/switch/;
    given ($OPTIONS{'Command'}) {
        when (/^FullSync$/i) {}
        when (/^SDToGit$/i) {}
        when (/^GitToSD$/i) {}
        when (/^SDToGitMsg$/i) {}
        when (/^GitToSDMsg$/i) {}
        when (/^StageSD$/i) {}
        when (/^WriteSyncPoint$/i) {}
        when (/^StartAutoSync$/i) {}
        when (/^StopAutoSync$/i) {}
        when (/^InitEnlistment$/i) {}
        default { die $usage; };
    }
}

sub checkAndParseArgs {
    $OPTIONS{'Email'} = 0;

    parseCmd();

    # Vars we need to initialize because they may not be passed.
    $OPTIONS{'Resume'} = '';
    $OPTIONS{'Repo'} = '';
    $OPTIONS{'endSDChange'} = '';
    $OPTIONS{'fullStartHash'} = '';
    $OPTIONS{'fullEndHash'} = '';

    my $help = 0;
    GetOptions('auto'           => \$OPTIONS{'Auto'},
               'cleanup!'       => \$OPTIONS{'Cleanup'},
               'coreBranch=s'   => \$OPTIONS{'CoreBranch'},
               'dry-run|d'      => \$OPTIONS{'DryRun'},
               'email!'         => \$OPTIONS{'Email'},
               'endGitHash=s'   => \$OPTIONS{'fullEndHash'},
               'endSdChange=s'  => \$OPTIONS{'endSDChange'},
               'fullBranch=s'   => \$OPTIONS{'FullBranch'},
               'gitHash=s'      => \$OPTIONS{'fullStartHash'},
               'repo=s'         => \$OPTIONS{'Repo'},
               'resume'         => \$OPTIONS{'Resume'},
               'sdChange=s'     => \$OPTIONS{'startSDChange'},
               'sdEnlistment=s' => \$OPTIONS{'SDEnlistment'},
               'verbose|v+'     => \$OPTIONS{'Verbose'},
               'cache-repo'     => \$OPTIONS{'CacheRepo'},
               'help|?'         => \$help)
        or die $usage;
    $help |= $OPTIONS{'SDEnlistment'} eq '';
    die $usage if $help;

    if ($OPTIONS{'DryRun'}) {
        $OPTIONS{'Email'} = 0;
    }

    if ($OPTIONS{'Repo'} ne '' && $OPTIONS{'Resume'} eq '') {
        die "--repo option can only be used with --resume present";
    }

    if ($OPTIONS{'Resume'} ne '' && $OPTIONS{'Repo'} eq '') {
        die "--resume option can only be used with --repo present";
    }

    debug("Options: " . Dumper(\%OPTIONS), $OPTIONS{'Verbose'} > 1);
}

sub sendErrorEmail {
    my $error = shift;

    logHeader("Generating error email");

    # Generate the dynamic parts of the message
    my $body = <<"END";
Git<->SD sync failed with the following error: $error

Consult the log below for more information.

-ChakraGit sync service
END
    sendMail(
        subject => 'ACTION REQUIRED: ChakraGit Sync failed for ' . strftime('%Y/%m/%d', localtime),
        body    => $body
    );
}

sub mergeConflictMessage {
    if ($ARGVSave[-3] eq '--repo' && $ARGVSave[-1] eq '--resume') {
        # Remove the last resume commands that we added.
        splice @ARGVSave, -3;
    }

    #my $resumePath = abs_path($0);
    my $resumeCmd = "cd $OPTIONS{'SDEnlistment'} & perl c:\\rs3_synctools\\GitSDSync.pl @ARGVSave --repo $OPTIONS{'Repo'} --resume";

    my $mergeCmd = "cd $OPTIONS{'SDEnlistment'}";
    $mergeCmd .= "\\core" if $OPTIONS{'Repo'} =~ /core/i;
    $mergeCmd .= " & git mergetool";

    msg("",                                                                 1);
    msg("",                                                                 1);
    msg("Rebase did not finish cleanly",                                    1);
    msg("Fix conflicts by running:",                                        1);
    msg("\t$mergeCmd",                                                      1);
    msg("Do *not* 'git rebase --continue', the script will do it for you.", 1);
    msg("Resume the script after conflict resolution using:",               1);
    msg("\t$resumeCmd",                                                     1);
}
sub getHeadCommitHash {
    my $headHash = execCmd('git rev-parse HEAD');
    $headHash =~ s/\s+//g;
    return $headHash;
}

# Error handler. If we get an error, we need to email the error and gracefully exit.
my $originalDieHandler = $SIG{__DIE__};
$SIG{__DIE__} = sub {
    # Restore the error handler so that we can die() for real if something goes wrong.
    $SIG{__DIE__} = $originalDieHandler;

    my $error = shift;

    error($error. Carp::longmess());

    sendErrorEmail($error) if $OPTIONS{'Auto'};

    cleanup() if $OPTIONS{'Cleanup'};

    exit(1);
};

if (abs_path() =~ /core$/) {
    die 'Please run this script from the full directory.';
}

checkAndParseArgs();

logHeader("Checking preconditions");
-e $OPTIONS{'SDEnlistment'} or die "Can't find enlistment at $OPTIONS{'SDEnlistment'}\n";

# Verify Git, and SD executables are available
sub canRunGit {
    # Silence 'unrecognised escape' warning. This occurs on Windows and is expected.
    local $SIG{__WARN__} = sub {
        if (scalar @ARGV) {
            my $warning = shift;
            if ($warning !~ /unrecognized escape/i) {
                print $warning;
            }
        }
    };
    can_run 'git.exe' or die "Unable to find Git executable on %PATH%. Ensure Git for Windows is installed and on %PATH% before Razzle tools.\n";
}

canRunGit();
can_run 'SD.exe' or die "Unable to find SD executable on %PATH%. This script needs to be run inside Razzle.\n";

my $sd_old_revision;
my $sd_new_revision;
my $sd_head;

sub cleanUp {
    # Clean up
    if ($OPTIONS{'Cleanup'}) {
        msg("Removing temporary Git repository in $OPTIONS{'Repo'}\n", $OPTIONS{'Verbose'});
        execCmd("rmdir /q /s .git") or die ".git directory is in use in $OPTIONS{'Repo'}. Is there a duplicate instance of the script running, or a git tool e.g. SourceTree?";
        if ($OPTIONS{'Repo'} !~ /core/i) {
            execCmd("rmdir /q /s core") or die "Core submodule is in use. Is there an instance of explorer, SourceTree, or cmd open in $OPTIONS{'Repo'}\\core?";
        }
    } else {
        msg("Skipping cleanup of $OPTIONS{'Repo'}\n", $OPTIONS{'Verbose'});
    }
}

sub parseHashes {
    # Verify gitHash parameter exists in full, and expand it to the full hash.
    $OPTIONS{'fullStartHash'} = execCmd("git rev-parse $OPTIONS{'fullStartHash'}");
    $OPTIONS{'fullStartHash'} =~ s/\s+//g;
    $OPTIONS{'coreStartHash'} = execCmd("git rev-parse $OPTIONS{'fullStartHash'}:core");
    $OPTIONS{'coreStartHash'} =~ s/\s+//g;

    # Fill out the upper hash range if it wasn't provided, using the head of the remote
    if ($OPTIONS{'fullEndHash'} eq '') {
        $OPTIONS{'fullEndHash'} = execCmd("git rev-parse origin/$OPTIONS{'FullBranch'}");
        $OPTIONS{'fullEndHash'} =~ s/\s+//g;
    }
    $OPTIONS{'coreEndHash'} = execCmd("git rev-parse $OPTIONS{'fullEndHash'}:core");
    $OPTIONS{'coreEndHash'} =~ s/\s+//g;

    msg("", 1);
    msg(":: Git ranges summary", 1);
    msg("Full sync range: $OPTIONS{'fullStartHash'}..$OPTIONS{'fullEndHash'}", 1);
    msg("Core sync range: $OPTIONS{'coreStartHash'}..$OPTIONS{'coreEndHash'}", 1);
    msg("", 1);
}

sub initGitEnlistment {
    msg('Initializing temporary working Git repository', $OPTIONS{'Verbose'});

    if (-e '.git') {
        msg("Removing existing Git repository in $OPTIONS{'SDEnlistment'}\n", $OPTIONS{'Verbose'});
        if (execCmd("rmdir /q /s .git") =~ /The process cannot access the file because it is being used by another process./) {
            die ".git directory is in use in $OPTIONS{'Repo'}. Is there a duplicate instance of the script running, a git tool, or a hung git process?";
        }
    }
    if (-e 'core') {
        msg("Removing existing core submodule in $OPTIONS{'SDEnlistment'}\n", $OPTIONS{'Verbose'});
        if (execCmd("rmdir /q /s core") =~ /The process cannot access the file because it is being used by another process./) {
            die "Core submodule is in use. Is there an instance of explorer or cmd open in $OPTIONS{'Repo'}\\core?";
        }
    }

    execCmd('git init');
    execCmd('git remote add origin '. $OPTIONS{'RemoteGitURL'});
    execCmd("git fetch origin", Noisy => 1); # Fetch all refs; the last synced revision may not be in the current synced branch! (e.g. branch migration)
    execCmd("git checkout $OPTIONS{'FullBranch'} --force");
    execCmd('git config mergetool.keepBackup false');

    parseHashes();

    msg('Initializing core submodule', $OPTIONS{'Verbose'});
    execCmd('git submodule update --init'); # Run before first SD sync since it will overwrite core.
    chdirCore();
    execCmd("git fetch origin", Noisy => 1); # Fetch all refs; the last synced revision may not be in the current synced branch! (e.g. branch migration)
    execCmd('git config mergetool.keepBackup false');

    msg('Initialization complete. Running sync commands.', $OPTIONS{'Verbose'});
}

sub getLatestSDChangeForDir {
    my $latestChange = execCmd('sd changes -m 1 ...');
    $latestChange =~ s/^Change (\d+).*$/$1/;
    $latestChange =~ s/\s//sg;
    return $latestChange;
}

sub haveIncomingSDChanges {
    # Change directory to full, since we can see all SD changes from there.
    chdirFull();
    my $latestChange = getLatestSDChangeForDir();

    msg(":: Checking for incoming SD changes", 1);

    if ($latestChange > $OPTIONS{'startSDChange'}) {
        print "[" . $latestChange . "==" .$OPTIONS{'startSDChange'} . "]";
        return 1;
    }

    # TODO: Assert there are no changes in SD? This may not be true if the previous sync got messed up.
    return 0;
}

sub generateSDSummary {
    my $newestChange = shift;

    # Get the next most recent change number.
    # TODO: Surely SD CLI has a nicer way to do this..
    my @changes = reverse split("\n", execCmd("sd changes -r ...@" . $OPTIONS{'startSDChange'} . ','));
    my $startChange = (scalar @changes == 1) ? $changes[0] : $changes[1];
    print $startChange;
    $startChange =~ s/^Change (\d+).*$/$1/ or die "Couldn't extract starting change number for the SD change log.";
    $startChange =~ s/\s//sg;

    my $log = "Add SD changes from CL#$startChange";
    $log .= " to CL#$newestChange" if $startChange ne $newestChange;
    $log .= ".\n\n" . execCmd('sd changes -l ...@' . $startChange . ',', (Noisy => 1));
    return $log;
}

sub generateAndCommitGitLog {
    my $newestChange = shift;

    my $log = generateSDSummary($newestChange);

    # Strange hack for a weird state that happens after the previous SD command.
    # Without this status call, the process spawn for the following add command will fail.
    # This is most likely a windows perl bug.
    execCmd("git status", Noisy => 1);

    # Add all changes. THIS ASSUMES .gitignore IS UP TO DATE.
    execCmd('git add -A', Noisy => 1);

    # Grab a temporary file to put the message in Git
    # tempFile will evaluate to both the filename and the file handle in the correct contexts automatically.
    my $tempFile = File::Temp->new(UNLINK => 0) or die "Couldn't create temporary file for log message";
    print $tempFile $log;

    # Commit the sd changes
    execCmd("git commit --file=$tempFile");
    File::Temp::cleanup() if $OPTIONS{'Cleanup'}; # We no longer need the tempFile and don't need to wait until process completion.
}

sub resetToLastSyncPoint {
    chdirFull();
    # Restore previous sync state. Assumes we are in full directory.
    execCmd("sd revert ...", Noisy => 1);
    execCmd("sd sync -f ...@" . $OPTIONS{'startSDChange'}, Noisy => 1);
    # Strange hack for a weird state that happens after the previous SD command.
    # Without this status call, the process spawn for the add command will fail.
    execCmd("git status", Noisy => 1);
    execCmd('git clean -x -d --force');
    execCmd("git reset --hard $OPTIONS{'fullStartHash'}");
    chdirCore();
    execCmd('git clean -x -d --force');
    execCmd("git reset --hard");
    chdirFull();
    execCmd("git submodule update --force");
}

sub stageSDToGit {
    logHeader('Staging SD changes as a Git commit in local branch SDToGit');

    # Checkout LKG revision
    chdirFull();
    execCmd("git checkout $OPTIONS{'FullBranch'} --force");
    execCmd('git branch -D SDToGit', (SurviveError => 1));
    execCmd('git checkout -b SDToGit');

    # Restore previous sync state.
    resetToLastSyncPoint();

    # Clear non-staged core changes before submodule update.
    chdirCore();
    execCmd('git branch -D SDToGit', (SurviveError => 1));
    execCmd('git checkout -b SDToGit');

    # Update with changes from SD.
    chdirFull();
    execCmd("sd sync -f ..." . ($OPTIONS{'endSDChange'} eq '' ? '' : '@') . $OPTIONS{'endSDChange'}, Noisy => 1);

    # Process core first.
    msg('Checking for Core changes from SD...', $OPTIONS{'Verbose'});
    chdirCore();
    my $latestCoreChange = getLatestSDChangeForDir();
    my $coreChanges = 0;
    if ($latestCoreChange <= $OPTIONS{'startSDChange'}) {
        # No changes to core. We can use the existing commit as the submodule.
        $OPTIONS{'coreStartHash'} = getHeadCommitHash();

        msg("No core changes, using $OPTIONS{'coreStartHash'}", $OPTIONS{'Verbose'});
    } else {
        msg('Staging Core changes...', $OPTIONS{'Verbose'});
        generateAndCommitGitLog($latestCoreChange);
        $OPTIONS{'coreStartHash'} = getHeadCommitHash() or die "Error committing core SD changes";
        $coreChanges = 1;
    }

    # Now process full.
    msg('Checking for Full changes from SD...', $OPTIONS{'Verbose'});
    chdirFull();
    msg('Staging Full changes...', $OPTIONS{'Verbose'});
    my $fullChanges = 0;
    my $latestFullChange = getLatestSDChangeForDir();
    if ($latestFullChange <= $OPTIONS{'startSDChange'} && !$coreChanges) {
        # No changes to full.
        msg('No core or full changes. Skipping full commit.');
        return 1;
    } else {
        $fullChanges = 1;
        generateAndCommitGitLog($latestFullChange);
        $OPTIONS{'fullStartHash'} = getHeadCommitHash() or die "Error committing core SD changes";
    }

    msg('SD changes staged as full ' .  $OPTIONS{'fullStartHash'} . ' with core ' . $OPTIONS{'coreStartHash'}, $OPTIONS{'Verbose'});

    return 1;
}

sub resumeRebase {
    if ($OPTIONS{'Resume'} ne '') {
        my $rebaseStatus = execCmd('git status', (Noisy => 1));
            if ($rebaseStatus =~ /All conflicts fixed but you are still merging/) {
                execCmd('git commit --no-edit');
                execCmd('git rebase --continue', (SurviveError => 1));
            }
            elsif (execCmd('git status', (Noisy => 1)) =~ /nothing to commit, working directory clean/) {
                execCmd('git rebase --skip', (SurviveError => 1));
            } else {
                execCmd('git rebase --continue', (SurviveError => 1));
            }
    } else {
        execCmd('git rebase --preserve-merges SDToGit', (SurviveError => 1));
    }
    $OPTIONS{'Resume'} = '';

    # Check for core being the only conflict, resolve using remote core hash (which will unblock a chain of core conflicts)
    if ($OPTIONS{'Repo'} =~ /^Full$/i) {
        while (execCmd('git status --porcelain', (Noisy => 1)) =~ /UU\s+core\s+/g) {
            msg('Attempting auto resolve of core submodule conflict', $OPTIONS{'Verbose'});
            execCmd('git checkout --ours core'); # When rebasing, 'ours' means the remote
            execCmd('git add core');
            my $rebaseStatus = execCmd('git status', (Noisy => 1));
            
            # Keep empty commits - They are used for history in SD.
            if ($rebaseStatus =~ /All conflicts fixed but you are still merging/) {
                execCmd('git commit --no-edit');
                execCmd('git rebase --continue', (SurviveError => 1));
            }
            elsif ($rebaseStatus =~ /The previous cherry-pick is now empty/) {
                execCmd('git commit --allow-empty --no-edit');
                execCmd('git rebase --continue', (SurviveError => 1));
            }
            elsif ($rebaseStatus =~ /nothing to commit, working directory clean/) {
                execCmd('git rebase --skip', (SurviveError => 1));
            } else {
                execCmd('git rebase --continue', (SurviveError => 1));
            }
        }
    }
        
    my $rebaseStatus = execCmd('git status', (Noisy => 1));

    # If there are merge issues, send an email.
    if ($rebaseStatus =~ /rebase in progress; onto/ || $rebaseStatus =~ /You have unmerged paths/) {
        $OPTIONS{'Cleanup'} = 0;
        mergeConflictMessage();
        exit(1);
    }

    # At the end of the rebase, we need to take the effective changes that are resolved and put them back onto the original branch.
    my $origBranch = ($OPTIONS{'Repo'} =~ /core/i)? $OPTIONS{'CoreBranch'} : $OPTIONS{'FullBranch'};

    # We must have effective changes before creating a sync commit.
    if (execCmd("git diff --quiet $origBranch", (ReturnExitCode => 1))) {
        msg("Squashing resolved changes back onto $origBranch", $OPTIONS{'Verbose'});
        execCmd("git checkout $origBranch");
        execCmd("git merge --squash gitToSD --strategy-option=theirs", (SurviveError => 1));

        # Check for core being the only conflict, resolve using remote core hash (which will unblock a chain of core conflicts)
        if ($OPTIONS{'Repo'} =~ /^Full$/i) {
            while (execCmd('git status --porcelain', (Noisy => 1)) =~ /UU\s+core\s+/g) {
                msg('Attempting auto resolve of core submodule conflict', $OPTIONS{'Verbose'});
                execCmd('git checkout --ours core'); # When rebasing, 'ours' means the remote
                execCmd('git add core');
            }
        }

        generateAndCommitGitLog(getLatestSDChangeForDir());
    } else {
        # Check out the original branch for consistency even if we don't have changes.
        # This is useful if we need to manually commit anything else on top e.g. build fixes.
        execCmd("git checkout $origBranch");
    }
}

sub cleanSDDescription {
    my $desc = shift;

    # Disarm triggers
    $desc =~ s/(FW|BUILD|MSFT|OS|NIB)/\($1\)/;

    # TODO: Remove SD sync Git commits
    return $desc;
}

sub generateGitChangesLog {
    my $log = "\nFull changes:\n\n";
    $log .= execCmd("git log $OPTIONS{'fullStartHash'}..$OPTIONS{'fullEndHash'}", (Noisy => 1));

    my $logHeader = "Add Git changes from full $OPTIONS{'fullStartHash'}..$OPTIONS{'fullEndHash'}";

    if ($OPTIONS{'coreStartHash'} ne $OPTIONS{'coreEndHash'}) {
        chdirCore();
        $log .= "\nCore changes:\n\n";
        $log .= execCmd("git log $OPTIONS{'coreStartHash'}..$OPTIONS{'coreEndHash'}", (Noisy => 1));

        $logHeader .= " and core $OPTIONS{'coreStartHash'}..$OPTIONS{'coreEndHash'}";
    } else {
        $logHeader .= " and no core changes";
    }

    $log = $logHeader . "\n" . $log;

    $log = cleanSDDescription($log);

    $log = "FW: $log";

    # Indent the changelist description so SD correctly recognises it
    $log =~ s/\n/\n\t/g;

    return $log;
}

sub stageSDChanges {
    chdirFull();
    execCmd("sd online ...",    (Noisy => 1, SurviveError => 1));
    execCmd("sd edit ...",      (Noisy => 1, SurviveError => 1));
    execCmd("sd add ...",       (Noisy => 1, SurviveError => 1));
    execCmd("sd revert -a ...", (Noisy => 1, SurviveError => 1));

    execCmd("sd revert core\\.gitattributes", (SurviveError => 1));
}

sub submitGitChangesToSD {
    # Pick up changes that need to go into SD
    logHeader("Staging Git changes in an SD change...");
    stageSDChanges();

    my $openedFiles = execCmd("sd opened ...", (Noisy => 1));
    if ($openedFiles =~ /\.\.\. - file\(s\) not opened on this client\./) {
        if ($OPTIONS{'Auto'}) {
            # If there were no effective changes, they were most likely already in SD. Update the sync point anyway.
            $OPTIONS{'endSDChange'} = $OPTIONS{'startSDChange'};
        }
        msg("Skipping empty commit (no effective changes)", $OPTIONS{'Verbose'});
        return;
    }

    my $log = generateGitChangesLog();

    chdirFull();

    # Create the change and insert the description. STDIN is complicated in Windows so we'll use a tempfile.
    my $changeSpec = execCmd("sd change -o", Noisy => 1);
    $changeSpec =~ s/\<enter description here\>/$log/ or die "Couldn't process changelist specification";
    debug("Generated changespec:\n$changeSpec", $OPTIONS{'Verbose'} > 2);
    # tempFile will evaluate to both the filename and the file handle in the correct contexts automatically.
    my $tempFile = File::Temp->new(UNLINK => 0) or die "Couldn't create temporary file for changelist specification";
    print $tempFile $changeSpec;

    # Generate the change
    msg("Creating change based on changespec at $tempFile\n", $OPTIONS{'Verbose'});
    my $changeNum = execCmd("sd change -i <$tempFile");
    $changeNum =~ s/^Change (\d+).*/$1/ or die "Couldn't generate the SD change";

    # Perform the actual SD submit.
    msg("Submitting as $OPTIONS{'ServiceAccountUser'} using change $changeNum\n", $OPTIONS{'Verbose'} && !$OPTIONS{'DryRun'});
    my $result = execCmd("sd submit -c $changeNum", (HasSideEffects => 1));
    if ($OPTIONS{'Repo'} =~ /core/i) {
        $CORE_SD_CHANGED = 1;
    } else {
        $PRIVATE_SD_CHANGED = 1;
    }
    File::Temp::cleanup() if $OPTIONS{'Cleanup'}; # We no longer need the tempFile and don't need to wait until process completion.

    # Write checked-in commit.
    my $newChangeList = getLatestSDChangeForDir();
    $OPTIONS{'endSDChange'} = $newChangeList;
    if (!$OPTIONS{'DryRun'}) {
        msg('Changes submitted as SD change ' . $newChangeList, $OPTIONS{'Verbose'});
        msg('SD changes for Git have been staged in the SDToGit branches in full and core. Please MANUALLY inspect these changes and their message before pushing');
    } else {
        msg("",                                                                                                                 1);
        msg("",                                                                                                                 1);
        msg("Changes NOT SUBMITTED due to dry run.",                                                                            1);
        msg("Changes have been staged as CL#$changeNum. To complete sync, please do the following:",                            1);
        msg("",                                                                                                                 1);
        msg("\t1. Submit to SD and stabilize the changes in SNAP. You can either:",                                             1);
        msg("\t\ta) SD submit directly and then run a stability job (usually OK for small changes that are unlikely to break)", 1);
        msg("\t\t\tsd submit -c $changeNum",                                                                                    1);
        msg("\t\t\tcscript \\\\iesnap\\SNAP\\bin\\queuestabilityjob.wsf -branch:$ENV{'_BuildBranch'}",                          1);
        msg("\t\tb) Or, queue a SNAP job to gate the change (advised for large changes that probably will break)",              1);
        msg("\t\t\ti) snap submit -c $changeNum",                                                                                1);
        msg("",                                                                                                                 1);
        msg("\t2. Test VSO using a build branch and submit the changes.",                                                       1);
        msg("\t\ta) Push a build branch",                                           1);
        msg("\t\t\tcd core",                                                                                                    1);
        msg("\t\t\tgit push origin $OPTIONS{'CoreBranch'}:build/chakraut/sdtogit -f",                                           1);
        msg("\t\t\tcd ..",                                                                                                      1);
        msg("\t\t\tgit push origin $OPTIONS{'FullBranch'}:build/chakraut/sdtogit -f",                                           1);
        msg("\t\tb) Wait for the results. If everything is good, push the changes",                                             1);
        msg("\t\t\tcd core",                                                                                                    1);
        msg("\t\t\tgit push origin $OPTIONS{'CoreBranch'}",                                                                     1);
        msg("\t\t\tcd ..",                                                                                                      1);
        msg("\t\t\tgit push origin $OPTIONS{'FullBranch'}",                                                                     1);
    }
}

sub checkoutAndCleanGit {
    # Checkout the commit and set up the staging branch
    chdirFull();
    execCmd("git checkout $OPTIONS{'FullBranch'} --force");
    execCmd('git clean -x -d --force');
    execCmd("git reset --hard $OPTIONS{'fullEndHash'}");
    execCmd('git branch -D gitToSD', (SurviveError => 1));
    execCmd('git checkout -b gitToSD');
    execCmd("git submodule update --force");

    # Do the same for core
    chdirCore();
    my $coreGitHash = getHeadCommitHash();
    execCmd("git checkout $OPTIONS{'CoreBranch'} --force");
    execCmd('git clean -x -d --force');
    execCmd("git reset --hard $OPTIONS{'coreEndHash'}");
    execCmd('git branch -D gitToSD', (SurviveError => 1));
    execCmd('git checkout -b gitToSD');
}

sub gitToSDBoth {
    msg('Bringing Git changes back to SD', $OPTIONS{'Verbose'});

    chdirFull();

    # 1. Init repos for rebasing
    if($OPTIONS{'Repo'} eq '') {
        checkoutAndCleanGit();

		# TODO: Need a more reliable way of detecting changes
        # If we can detect there are no SD changes in core, we can skip the rebase/squash-merge.
        # In this case we just use the latest hash for core, which was just checked out in checkoutAndCleanGit.
        #chdirCore();
        #if (execCmd("git diff --quiet sdtogit..$OPTIONS{'coreStartHash'}", (ReturnExitCode => 1))) {
            # Start processing core repo
            $OPTIONS{'Repo'} = 'core';
        #} else {
        #    msg('No SD core changes detected, simulating fast forward and continuing with full..', $OPTIONS{'Verbose'});
        #    execCmd("git checkout $OPTIONS{'CoreBranch'} --force");
        #    $OPTIONS{'Repo'} = 'full';
        #}
    }

    # TODO: Refactor into states
    if ($OPTIONS{'Repo'} =~ /core/i) {
        chdirCore();

        resumeRebase();

        # Start processing Full repo
        $OPTIONS{'Repo'} = 'full';
    }

    if ($OPTIONS{'Repo'} =~ /full/i) {
        chdirFull();

        resumeRebase();
    }
    submitGitChangesToSD();
}

sub haveIncomingGitChanges {
    return $OPTIONS{'fullStartHash'} ne $OPTIONS{'fullEndHash'};
}

chdirFull();

use feature qw/switch/;
given ($OPTIONS{'Command'}) {
    when (/^FullSync$/i) {
        $OPTIONS{'DryRun'} = 1;
        if (!$OPTIONS{'Resume'}) {
            execCmd('sdx sync -w');
            initGitEnlistment();
            stageSDToGit();
        } else {
            parseHashes();
        }
        gitToSDBoth();
        cleanUp();
        break;
    }

    when (/^SDToGit$/i) {
        initGitEnlistment();
        stageSDToGit();
        break;
    }

    when (/^GitToSD$/i) {
        readSyncPoint() if $OPTIONS{'Auto'};
        execCmd("sd revert ...", Noisy => 1);
        # Verify no changes
        if (haveIncomingSDChanges()) {
            # We can't sync automatically.
            execCmd("schtasks /Change /TN \"$OPTIONS{AutoTaskName}\" /Disable");
            die "Incoming SD changes detected after CL#$OPTIONS{'startSDChange'} - unable to continue automatically. Please use FullSync.";
        }
        initGitEnlistment();
        if (!haveIncomingGitChanges()) {
            my $noChangesMsg = "No incoming git changes detected. Nothing to do.";
            if ($OPTIONS{'Auto'}) {
                msg($noChangesMsg, 1);
                exit 0;
            } else {
                die $noChangesMsg;
            }
        }
        # Commit back without rebasing
        checkoutAndCleanGit();
        submitGitChangesToSD();
        if ($OPTIONS{'Auto'} && !$OPTIONS{'DryRun'}) {
            writeSyncPoint();
        }
        exit 0;
    }

    when (/^SDToGitMsg$/i) {
        my $latestChange = getLatestSDChangeForDir();
        print generateSDSummary($latestChange);
        break;
    }
    when (/^GitToSDMsg$/i) {
        parseHashes();
        print generateGitChangesLog();
        break;
    }
    when (/^StageSD$/i) {
        stageSDChanges();
        break;
    }
    when (/^WriteSyncPoint$/i) {
        $OPTIONS{'Auto'} = 1;
        $OPTIONS{'fullEndHash'} = $OPTIONS{'fullStartHash'};
        $OPTIONS{'endSDChange'} = $OPTIONS{'startSDChange'};
        writeSyncPoint();
        break;
    }
    when (/^StopAutoSync$/i) {
        execCmd("schtasks /End /TN \"$OPTIONS{AutoTaskName}\"");
        execCmd("schtasks /Change /TN \"$OPTIONS{AutoTaskName}\" /Disable");
        break;
    }
    when (/^StartAutoSync$/i) {
        execCmd("schtasks /Change /TN \"$OPTIONS{AutoTaskName}\" /Enable");
        break;
    }
    when (/^InitEnlistment$/i) {
        initGitEnlistment();
        resetToLastSyncPoint();
        break;
    }
}

