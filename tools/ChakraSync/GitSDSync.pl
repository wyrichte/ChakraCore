# Chakra Git<->SD sync
# Brings in changes from SD and rebases Git changes on to them. Commits the result to Git and SD.
# TODO:
# - Verify we are using Git directly rather than the buggy razzle git
# - SD and git ranges for sync
use strict;
use warnings;

# The following libraries are distributed under Razzle, so don't dynamically check for their existence.
use POSIX qw(strftime);
my @ARGVSave = @ARGV; # Save ARGV since Getopt will destroy it. We need it to print the resume command later.
use Getopt::Long qw(GetOptions);
use IPC::Cmd qw(can_run run);
$IPC::Cmd::USE_IPC_OPEN3 = 1;
$IPC::Cmd::WARN = 0; # Suppress noisy warning about IPC::Run missing. We don't end up using this module.use Pod::Usage qw(pod2usage);
use Cwd 'abs_path';
use File::Temp qw(tempfile);

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

sub checkAndParseArgs {
    $OPTIONS{'Resume'} = '';
    $OPTIONS{'Repo'} = '';
    $OPTIONS{'endSdChange'} = '';
    $OPTIONS{'endFullGitHash'} = '';

    my $usage = <<"END";
Usage:
perl $0 --gitHash hash --sdChange change [options]

 Options:
   -v --verbose         Show status info. Use flag twice for more verbosity.
   -r --resume          Resume after fixing a merge conflict
      --no-email        Don't send error or summary emails
      --no-cleanup      Don't delete temporary branch
      --no-snap         Don't submit a SNAP job
   -d --dry-run         Don't make any changes that could result in breakage and angry people
                        (Implies --no-email and --no-snap)
      --sdEnlistment    Root of the SD enlistment to use (jscript folder)
   -? --help            This message
END
    my $help = 0;
    GetOptions('repo=s'        => \$OPTIONS{'Repo'},
               'coreBranch=s'  => \$OPTIONS{'CoreBranch'},
               'fullBranch=s'  => \$OPTIONS{'FullBranch'},
               'verbose|v+'    => \$OPTIONS{'Verbose'},
               'dry-run|d'     => \$OPTIONS{'DryRun'},
               'resume=s'      => \$OPTIONS{'Resume'},
               'email!'        => \$OPTIONS{'Email'},
               'cleanup!'      => \$OPTIONS{'Cleanup'},
               'snap!'         => \$OPTIONS{'SNAP'},
               'sdChange=s'    => \$OPTIONS{'sdChange'},
               'gitHash=s'     => \$OPTIONS{'fullGitHash'},
               'endSdChange=s' => \$OPTIONS{'endSdChange'},
               'endGitHash=s'  => \$OPTIONS{'endFullGitHash'},
               'sdEnlistment'  => \$OPTIONS{'SDEnlistment'},
               'help|?'        => \$help)
        or die $usage;
    $help |= $OPTIONS{'SDEnlistment'} eq '';

    $OPTIONS{'Email'} = 0 if $help;
    die $usage if $help;

    if ($OPTIONS{'DryRun'}) {
        $OPTIONS{'Email'} = 0;
        $OPTIONS{'SNAP'} = 0;
    }

    if ($OPTIONS{'Repo'} ne '' && $OPTIONS{'Resume'} eq '') {
        die "--repo option can only be used with --resume present";
    }
    
    if ($OPTIONS{'Resume'} ne '' && $OPTIONS{'Repo'} eq '') {
        die "--resume option can only be used with --repo present";
    }

    debug("Options: " . Dumper(\%OPTIONS), $OPTIONS{'Verbose'} > 1);
}

sub generateChangesTable {
    my $dryRun = $OPTIONS{'DryRun'} ? "[DRY RUN] " : "";
    $PRIVATE_SD_CHANGED  = $PRIVATE_SD_CHANGED  ? "CHANGES SUBMITTED" : "No changes";
    $PRIVATE_VSO_CHANGED = $PRIVATE_VSO_CHANGED ? "CHANGES PUSHED "   : "No changes";
    $CORE_SD_CHANGED     = $CORE_SD_CHANGED     ? "CHANGES SUBMITTED" : "No changes";
    $CORE_VSO_CHANGED    = $CORE_VSO_CHANGED    ? "CHANGES PUSHED"    : "No changes";

    my $changesTable = <<"END";
Private SD:      $dryRun$PRIVATE_SD_CHANGED
Private VSO Git: $dryRun$PRIVATE_VSO_CHANGED
Core SD:         $dryRun$CORE_SD_CHANGED
Core VSO Git:    $dryRun$CORE_VSO_CHANGED
END
    return $changesTable;
}

sub sendErrorEmail {
    my $error = shift;

    msg("Generating error email");

    # Generate the dynamic parts of the message
    my $changesTable = generateChangesTable();

    my $body = <<"END";
Git<->SD sync failed with the following error: $error

$changesTable

Consult the log below for more information.

-ChakraGit sync service
END
    sendMail(
        subject => 'ACTION REQUIRED: ChakraGit Sync failed for ' . strftime('%Y/%m/%d', localtime),
        body    => $body
    );
}

sub mergeConflictEmail {
    msg("Rebase did not finish cleanly, sending conflict email\n", $OPTIONS{'Verbose'});
    
    my $rebaseLog = shift;

    my $scriptLocation = abs_path($0);

    # Generate the dynamic parts of the message
    my $changesTable = generateChangesTable();

    if ($ARGVSave[-4] eq '--repo' && $ARGVSave[-2] eq '--resume') {
        # Remove the last resume commands that we added.
        splice @ARGVSave, -4;
    }

    my $resumeCmd = "perl $0 @ARGVSave --repo $OPTIONS{'Repo'} --resume continue";

    my $body = <<"END";
This is an automatically generated message from the ChakraGit Git<->SD sync tool.

The $ENV{'_BuildBranch'} RI from Git to SD did not merge cleanly in the temporary git branch GitToSD. A manual rebase is required. TS to 'ChakraGit' as 'REDMOND\\ChakraAut' and run the following commands from Razzle:
cd $OPTIONS{'SDEnlistment'}
git mergetool
$resumeCmd

Rebase output is below.

-ChakraGit sync service
END

    sendMail(
        subject => 'ChakraGit: ACTION REQUIRED: ' . $ENV{'_BuildBranch'} . ' ' . strftime('%Y/%m/%d', localtime) . ' Git->SD sync has merge conflicts',
        body    => $body
    );
    
    debug("Resume using: cd $OPTIONS{'SDEnlistment'} & $resumeCmd", $OPTIONS{'Verbose'});
}
sub getHeadCommitHash {
    my $headHash = execCmd('git rev-parse HEAD');
    $headHash =~ s/\s+//g;
    return $headHash;
}

sub summaryEmail {
    # Send summary email.
    msg("Sending summary email\n", $OPTIONS{'Verbose'});

    # Get last sync data
    my ($startSdChange, $startGitHash) = getLastSyncedData();
    my $currSdChange = getLatestSDChangeForDir();
    my $currGitChange = getHeadCommitHash();
    my $sdChangeCount = 1;

    # Generate the dynamic parts of the message
    my $changesTable = generateChangesTable();

    my $summary = <<"END";
$ENV{'_BuildBranch'} sync successfully completed.

$changesTable

Git updated from $startGitHash to $currGitChange.

SD updated from CL#$startSdChange to CL#$currSdChange.

-ChakraGit sync service
END
    sendMail(
        subject =>  'ChakraGit: Sync summary for ' . strftime('%Y/%m/%d', localtime),
        body    => $summary
    );
    return 1;
};

# Error handler. If we get an error, we need to email the error and gracefully exit.
my $originalDieHandler = $SIG{__DIE__};
$SIG{__DIE__} = sub {
    # Restore the error handler so that we can die() for real if something goes wrong.
    $SIG{__DIE__} = $originalDieHandler;

    my $error = shift;

    error($error. Carp::longmess());

    sendErrorEmail($error);

    # Decide whether to cleanup
    cleanup() if $OPTIONS{'Cleanup'};

    exit(1);
};

checkAndParseArgs();

use IPC::Cmd qw(can_run run);

msg("Checking preconditions", 1);
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
    can_run 'git.exe'        or die "Unable to find Git executable on %PATH%. Ensure Git for Windows is installed and on %PATH% before Razzle tools.\n";
}

canRunGit();
can_run 'SD.exe'             or die "Unable to find SD executable on %PATH%. This script needs to be run inside Razzle.\n";

my $sd_old_revision;
my $sd_new_revision;
my $sd_head;

sub cleanUp {
    # Clean up
    if ($OPTIONS{'Cleanup'}) {
        msg("Removing temporary Git repository in $OPTIONS{'Repo'}\n", $OPTIONS{'Verbose'});
        execCmd("rmdir /q /s .git");
        execCmd("rmdir /q /s core") if $OPTIONS{'Repo'} !~ /core/i;
    } else {
        msg("Skipping cleanup of $OPTIONS{'Repo'}\n", $OPTIONS{'Verbose'});
    }
}

sub submitToSNAP {
    return if !$OPTIONS{'SNAP'};
    msg("Submitting stability job to SNAP", $OPTIONS{'Verbose'});

    # Generate a DPK and submit to SNAP.
    chdir $OPTIONS{'SDEnlistment'};
    execCmd('sd edit dummy.txt');
    open FH, '>>', 'dummy.txt' or die "Couldn't open dummy.txt for appending: $@";
    print FH ".\n";
    close FH;
    execCmd('snap submit -win8fi');
    print "\n";
}

sub initEnlistmentGit {
    msg('Initializing temporary working Git repository', $OPTIONS{'Verbose'});
    execCmd("rmdir /q /s .git") if -e '.git';
    execCmd("rmdir /q /s core") if -e 'core';
    execCmd('git init');
    execCmd('git remote add origin '. $OPTIONS{'RemoteGitURL'});
    execCmd("git fetch");
    execCmd("git checkout $OPTIONS{'FullBranch'} --force");
    execCmd('git config mergetool.keepBackup false');

    # Verify gitHash parameter exists in full
    execCmd("git rev-parse $OPTIONS{'fullGitHash'}");

    msg('Initializing core submodule', $OPTIONS{'Verbose'});
    execCmd('git submodule update --init'); # Run before first SD sync since it will overwrite core.
    chdirAndLog($OPTIONS{'SDEnlistment'} . '\core');
    execCmd("git fetch");
    execCmd('git config mergetool.keepBackup false');
    msg('Initialization complete. Running sync commands.', $OPTIONS{'Verbose'});
}

sub getLatestSDChangeForDir {
    my $latestChange = execCmd('sd changes -m 1 ...');
    $latestChange =~ s/^Change (\d+).*$/$1/;
    #$latestChange =~ s/\s//sg;
    return $latestChange;
}

sub generateSDSummary {
    my $newestChange = shift;
    my $sdChange = shift;

    # Get the next most recent change number.
    # TODO: Surely SD CLI has a nicer way to do this..
    my @changes = reverse split("\n", execCmd("sd changes -r ...@" . $sdChange . ','));
    my $startChange = (scalar @changes == 1) ? $changes[0] : $changes[1];
    $startChange =~ s/^Change (\d+).*$/$1/ or die "Couldn't extract starting change number for the SD change log.";
    $startChange =~ s/\s//sg;

    my $log = "Add SD changes from CL#$startChange to CL#$newestChange.\n\n" . execCmd('sd changes -l ...@' . $startChange . ',', (Noisy => 1));
    return $log;
}

sub getLastSyncedData {
    # Get last change synced
    my ($sdChange, $gitHash);
    if (-e '.chakraSync') {
        open my $fh, '+<', '.chakraGit';
        my $line = <$fh>;
        $line =~ /^(\d+),(\w)+$/ or die('.chakraGit sync metadata file is corrupted. Delete it to reset sync.');
        ($sdChange, $gitHash) = ($1, $2);
    } else {
        $sdChange = $OPTIONS{'sdChange'};
        $gitHash  = $OPTIONS{'fullGitHash'};
    }

    if (!defined $sdChange) { die "No SD change provided"; }
    if (!defined $gitHash)  { die "No Git hash provided"; }
    if ($sdChange !~ /\d+/) { die "Invalid SD change provided: $sdChange"; }
    if ($gitHash !~ /\w+/)  { die "Invalid Git hash specified: $gitHash"; }

    return ($sdChange, $gitHash);
}

sub generateAndCommitGitLog {
    my $newestChange = shift;
    my $sdChange = shift;
    
    my $log = generateSDSummary($newestChange, $sdChange);

    # Strange hack for a weird state that happens after the previous SD command.
    # Without this status call, the process spawn for the following add command will fail.
    # This is most likely a windows perl bug.
    execCmd("git status");
    
    # Add all changes. THIS ASSUMES .gitignore IS UP TO DATE.
    execCmd('git add -A', Noisy => 1);

    # Grab a temporary file to put the message in Git
    # tempFile will evaluate to both the filename and the file handle in the correct contexts automatically.
    my $tempFile = File::Temp->new(UNLINK => 0) or die "Couldn't create temporary file for log message";
    print $tempFile $log;

    # Commit the sd changes
    execCmd("git commit --file=$tempFile");
    #File::Temp::cleanup(); # We no longer need the tempFile and don't need to wait until process completion.

    return getHeadCommitHash();
}

sub stageSDToGit {
    # Get last change synced
    my ($sdChange, $gitHash) = getLastSyncedData();

    # Create git repo locally
    initEnlistmentGit();

    msg('Staging SD changes as a Git commit in local branch SDToGit', $OPTIONS{'Verbose'});

    # Checkout LKG revision
    chdirAndLog($OPTIONS{'SDEnlistment'});
    execCmd("git checkout $OPTIONS{'FullBranch'} --force");
    execCmd('git branch -D SDToGit', (SurviveError => 1));
    execCmd('git checkout -b SDToGit');

    # Restore previous sync state.
    execCmd("sd revert ...");
    execCmd("sd sync -f ...@" . $sdChange, Noisy => 1);
    # Strange hack for a weird state that happens after the previous SD command.
    # Without this status call, the process spawn for the add command will fail.
    execCmd("git status");
    execCmd('git add -A');
    execCmd("git reset --hard $gitHash");
    chdirAndLog($OPTIONS{'SDEnlistment'} . '\core');
    execCmd('git add -A');
    execCmd("git reset --hard");
    chdirAndLog($OPTIONS{'SDEnlistment'});
    execCmd("git submodule update");

    # Clear non-staged core changes before submodule update.
    chdirAndLog($OPTIONS{'SDEnlistment'} . '\core');
    execCmd('git branch -D SDToGit', (SurviveError => 1));
    execCmd('git checkout -b SDToGit');

    # Update with changes from SD.
    chdirAndLog($OPTIONS{'SDEnlistment'});
    execCmd("sd sync -f ..." . ($OPTIONS{'endSdChange'} eq '' ? '' : '@') . $OPTIONS{'endSdChange'}, Noisy => 1);

# TODO: More effective heuristics for detecting if no changes need to be brought across.
#    my $status = execCmd('git status');
#    if ($status =~ /nothing to commit, working directory clean/) {
#        # No effective changes from SD.
#        msg('No changes from SD. Skipping SD sync.', $OPTIONS{'Verbose'});
#        return 1;
#    }

    # Process core first.
    msg('Checking for Core changes from SD...', $OPTIONS{'Verbose'});
    chdirAndLog($OPTIONS{'SDEnlistment'} . '\core');
    my $latestCoreChange = getLatestSDChangeForDir();
    my $coreChanges = 0;
    if ($latestCoreChange == $sdChange) {
        # No changes to core. We can use the existing commit as the submodule.
        $OPTIONS{'coreGitHash'} = getHeadCommitHash();
        msg("No core changes, using $OPTIONS{'coreGitHash'}", $OPTIONS{'Verbose'});
    } else {
        msg('Staging Core changes...', $OPTIONS{'Verbose'});
        $OPTIONS{'coreGitHash'} = generateAndCommitGitLog($latestCoreChange, $sdChange) or die "Error committing full SD changes";
        $coreChanges = 1;
    }

    # Now process full.
    msg('Checking for Full changes from SD...', $OPTIONS{'Verbose'});
    chdirAndLog($OPTIONS{'SDEnlistment'}); 
    msg('Staging Full changes...', $OPTIONS{'Verbose'});
    my $fullChanges = 0;
    my $latestFullChange = getLatestSDChangeForDir();
    if ($latestFullChange == $sdChange && !$coreChanges) {
        # No changes to full.
        $OPTIONS{'fullGitHash'} = $gitHash;
        msg('No core or full changes. Skipping full commit.');
    } else {
        $fullChanges = 1;
        $OPTIONS{'fullGitHash'} = generateAndCommitGitLog($latestFullChange, $sdChange) or die "Error committing core SD changes";
    }

    msg('SD changes staged as full ' .  $OPTIONS{'fullGitHash'} . ' with core ' . $OPTIONS{'coreGitHash'}, $OPTIONS{'Verbose'});

    return 1;
}

sub resumeRebase {
    if ($OPTIONS{'Resume'} =~ /^skip$/i) {
        execCmd('git rebase --skip', (SurviveError => 1));
    } elsif ($OPTIONS{'Resume'} =~ /continue/i) {
        execCmd('git rebase --continue', (SurviveError => 1));
    } else {
        execCmd('git rebase SDToGit', (SurviveError => 1));
    }
    $OPTIONS{'Resume'} = '';

    my $rebaseStatus = execCmd('git status');

    # If there are merge issues, send an email.
    if ($rebaseStatus =~ /rebase in progress; onto/) {
        $OPTIONS{'Cleanup'} = 0;
        mergeConflictEmail();
        exit(1);
    }
}

sub gitToSDBoth {
    msg('Bringing Git changes back to SD', $OPTIONS{'Verbose'});

    # Read last checked-in commit
    # Get last change synced
    my ($sdChange, $gitHash) = getLastSyncedData();
    my $coreGitHash = '';

    # 1. Init repos for rebasing
    if($OPTIONS{'Repo'} eq '') {
        # Checkout the commit and set up the staging branch
        execCmd("git checkout $OPTIONS{'FullBranch'} --force");
        execCmd('git add -A');
        execCmd("git reset --hard origin/$OPTIONS{'FullBranch'}");
        execCmd('git branch -D gitToSD', (SurviveError => 1));
        execCmd('git checkout -b gitToSD');
        execCmd("git submodule update");

        # Do the same for core
        chdirAndLog($OPTIONS{'SDEnlistment'} . '\core');
        my $coreGitHash = getHeadCommitHash();
        execCmd("git checkout $OPTIONS{'CoreBranch'} --force");
        execCmd('git add -A');
        execCmd("git reset --hard $coreGitHash");
        execCmd('git branch -D gitToSD', (SurviveError => 1));
        execCmd('git checkout -b gitToSD');
        
        # Start processing core repo
        $OPTIONS{'Repo'} = 'core';
    }

    # TODO: Refactor into states
    if ($OPTIONS{'Repo'} =~ /core/i) {
        chdirAndLog($OPTIONS{'SDEnlistment'} . '\core');

        resumeRebase();

        # Start processing Full repo
        $OPTIONS{'Repo'} = 'full';
    }

    if ($OPTIONS{'Repo'} =~ /full/i) {
        chdirAndLog($OPTIONS{'SDEnlistment'});

        resumeRebase();
    }

    # If we aren't submitting all Git changes, reset to the end change.
    if ($OPTIONS{'endFullGitHash'} ne '') {
        execCmd("git reset --hard $OPTIONS{'endFullGitHash'}");
        execCmd('git submodule update');
    }

    # Pick up changes that need to go into SD
    chdirAndLog($OPTIONS{'SDEnlistment'});
    execCmd("sd online ...",    (Noisy => 1, SurviveError => 1));
    execCmd("sd edit ...",      (Noisy => 1, SurviveError => 1));
    execCmd("sd add ...",       (Noisy => 1, SurviveError => 1));
    execCmd("sd revert -a ...", (Noisy => 1, SurviveError => 1));

    my $openedFiles = execCmd("sd opened ...", (Noisy => 1));
    if ($openedFiles =~ /\.\.\. - file\(s\) not opened on this client\./) {
        msg("Skipping empty commit (no effective changes)", $OPTIONS{'Verbose'});
    }

    my $log = execCmd("git log $gitHash..$OPTIONS{'FullBranch'}", (Noisy => 1));
    my $coreHash = execCmd("git rev-parse $gitHash:core");
    chdirAndLog($OPTIONS{'SDEnlistment'} . '\core');
    $log .= execCmd("git log $coreHash..$OPTIONS{'CoreBranch'}", (Noisy => 1));
    chdirAndLog($OPTIONS{'SDEnlistment'});

    # Create the change and insert the description. STDIN is complicated in Windows so we'll use a tempfile.
    my $changeSpec = execCmd("sd change -o");
    $changeSpec =~ s/\<enter description here\>/$log/ or die "Couldn't process changelist specification";
    debug("Generated changespec:\n$changeSpec", $OPTIONS{'Verbose'} > 1);
    # tempFile will evaluate to both the filename and the file handle in the correct contexts automatically.
    if (!$OPTIONS{'DryRun'}) {
        my $tempFile = File::Temp->new(UNLINK => 0) or die "Couldn't create temporary file for changelist specification";
        print $tempFile $changeSpec;

        # Perform the actual SD submit.
        my $submittedBy = $OPTIONS{'ServiceAccountUser'};
        msg("Submitting as $submittedBy\n", $OPTIONS{'Verbose'});
        my $result = execCmd("sd submit -i <$tempFile", (HasSideEffects => 1));
    }
    if ($OPTIONS{'Repo'} =~ /core/i) {
        $CORE_SD_CHANGED = 1;
    } else {
        $PRIVATE_SD_CHANGED = 1;
    }
    #File::Temp::cleanup(); # We no longer need the tempFile and don't need to wait until process completion.

    # Write checked-in commit.
    my $newChangeList = getLatestSDChangeForDir();
    msg('Changes submitted as SD change ' . $newChangeList, $OPTIONS{'Verbose'});

    msg('SD changes for Git have been staged in the SDToGit branches in full and core. Please MANUALLY inspect these changes and their message before pushing');
}

chdirAndLog($OPTIONS{'SDEnlistment'});

my $result = 0;
if (!$OPTIONS{'Resume'}) {
    $result = eval {
        stageSDToGit();
    };
}

gitToSDBoth();

summaryEmail();

cleanUp();
