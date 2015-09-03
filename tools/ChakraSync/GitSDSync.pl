# Chakra Git<->SD sync
# Brings in changes from SD and rebases Git changes on to them. Commits the result to Git and SD.
# TODO:
# Verify we are using Git directly rather than the buggy razzle git
use strict;
use warnings;

# The following libraries are distributed under Razzle, so don't dynamically check for their existence.
use POSIX qw(strftime);
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

use lib "$ENV{ProgramFiles}\\Git\\lib\\perl5\\site_perl";

# Globals
our $PRIVATE_SD_CHANGED  = 0;
our $PRIVATE_VSO_CHANGED = 0;
our $CORE_SD_CHANGED     = 0;
our $CORE_VSO_CHANGED    = 0;

sub execCmd {
    my $command = shift;
    
    # Execute options
    # - Skip: Skip the command if true (usually any command that results in permanent changes)
    # - SurviveError: Do not die on non-zero exit code. Allows custom error handling, e.g. rebase merge conflict.
    my %execOptions = @_;

    if ($OPTIONS{'DryRun'} && $execOptions{'Skip'}) {
        msg("Skipping: $command", $OPTIONS{'Verbose'});
        return '';
    } else {
        msg("Executing command '$command'", 1);
        my ($output, $exit);
        if (($OPTIONS{'Verbose'} > 1 && !$execOptions{'NOISY'})
            || $OPTIONS{'Verbose'} > 2) {
           ($output, $exit) = tee_merged {
                system($command);
            };
        } else {
            ($output, $exit) = capture_merged {
                system($command);
            };
        }

        if ($execOptions{'SurviveError'} || $exit == 0) {
            debug("Ignoring exit code $exit") if $execOptions{'SurviveError'};
            debug($output);
        } else {
            die "Error while running $command. Output follows:\n$output";
        }
        return $output;
    }
}

sub checkAndParseArgs {
    $OPTIONS{'Resume'} = 0;
    
    my $usage = <<"END";
perl $0 --repo [core|private] [options]

   --repo               Core or private repo (TODO: remove)

 Options:
   -v --verbose         Show status info. Use flag twice for more verbosity.
   -r --resume          Resume after fixing a merge conflict
      --no-email        Don't send error or summary emails
      --no-cleanup      Don't delete temporary branch
      --no-snap         Don't submit a SNAP job
   -d --dry-run         Don't make any changes that could result in breakage and angry people
                        (Implies --no-email and --no-snap)
   -? --help            This message
END
    my $help = 0;
    GetOptions('repo=s'      => \$OPTIONS{'Repo'},
               'verbose|v+'  => \$OPTIONS{'Verbose'},
               'dry-run|d'   => \$OPTIONS{'DryRun'},
               'resume=s'    => \$OPTIONS{'Resume'},
               'email!'      => \$OPTIONS{'Email'},
               'cleanup!'    => \$OPTIONS{'Cleanup'},
               'snap!'       => \$OPTIONS{'SNAP'},
               'sdChange=s'  => \$OPTIONS{'sdChange'},
               'gitHash=s'   => \$OPTIONS{'gitHash'},
               'help|?'      => \$help) or die $usage;

    $help |= $OPTIONS{'RootSDEnlistment'} eq '';
    $help |= $OPTIONS{'Repo'} eq '';

    die $usage if $help;

    if ($OPTIONS{'DryRun'}) {
        $OPTIONS{'Email'} = 0;
        $OPTIONS{'SNAP'} = 0;
    }

    debug("Options: " . Dumper(\%OPTIONS), $OPTIONS{'Verbose'} > 1);
}

sub sendMail {
    my %args = @_;
    print ($args{body}) if $OPTIONS{'Verbose'} > 1;

    if (!$OPTIONS{'Email'}) {
        msg("Email NOT sent due to run settings");
        return;
    }
    
    # Fetch the log stack for inclusion.
    my $msgs  = Log::Message::Simple->stack_as_string;
    print $msgs if $OPTIONS{'Verbose'} > 2;
    $args{body} .= "\n\n$msgs\n";

    require $ENV{'sdxroot'} . '\TOOLS\sendmsg.pl';
    sendmsg($OPTIONS{'ServiceAccountEmail'},
            $args{subject},
            $args{body},
            $OPTIONS{'EmailTo'});
    msg("Email sent");
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

    my $body = <<"END";
This is an automatically generated message from the ChakraGit Git<->SD sync tool.

The $ENV{'_BuildBranch'} RI from Git to SD did not merge cleanly in the temporary git branch GitToSD. A manual rebase is required. TS to 'ChakraGit' as 'REDMOND\\ChakraAut' and run the following commands from Razzle:
cd $OPTIONS{'RootGitEnlistment'}
git mergetool
perl $scriptLocation --resume

Rebase output is below.

-ChakraGit sync service
END

    sendMail(
        subject => 'ChakraGit: ACTION REQUIRED: ' . $ENV{'_BuildBranch'} . ' ' . strftime('%Y/%m/%d', localtime) . ' Git->SD sync has merge conflicts',
        body    => $body
    );
}

sub summaryEmail {
    # Send summary email.
    msg("Sending summary email\n", $OPTIONS{'Verbose'});

    # Get last sync data
    my ($startSdChange, $startGitHash) = getLastSyncedData();
    my $currSdChange = `sd counter change`;
    my $currGitChange = `git rev-parse HEAD`;
    my $sdChangeCount = 1;

    # Generate the dynamic parts of the message
    my $changesTable = generateChangesTable();

    my $summary = <<"END";
$ENV{'_BuildBranch'} $OPTIONS{'Repo'} sync successfully completed.

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
-e $OPTIONS{'RootSDEnlistment'} or die "Can't find enlistment at $OPTIONS{'RootSDEnlistment'}\n";
-e $OPTIONS{'CoreSDEnlistment'} or die "Can't find enlistment at $OPTIONS{'CoreSDEnlistment'}\n";

# Verify Git, SD, and TeamHub(Manager) executables are available
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
    can_run 'git.exe'        or die "Unable to find Git executable on %PATH%. Is Git for Windows installed?\n";
}

canRunGit();
can_run 'SD.exe'             or die "Unable to find SD executable on %PATH%. Are you running this script from inside Razzle?\n";

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

sub submitSNAP {
    return if !$OPTIONS{'SNAP'};
    msg("Submitting stability job to SNAP", $OPTIONS{'Verbose'});

    # Generate a DPK and submit to SNAP.
    chdir $OPTIONS{'RootSDEnlistment'};
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
    execCmd('git remote add origin '. ($OPTIONS{'Repo'} =~ /core/i ? $OPTIONS{'VSOCoreURL'} : $OPTIONS{'VSOPrivateURL'}));
    execCmd('git fetch origin master');
    if ($OPTIONS{'Repo'} =~ /full/i) {
        execCmd('git submodule init');
    }
    msg('Initialization complete. Running sync commands.', $OPTIONS{'Verbose'});
}

sub generateSDSummary {
    # Get the changelist descriptions for all changes we are adding.
    # Takes one argument: The SD change we started to sync from.
    my $newestChange = execCmd('sd counter change');
    $newestChange =~ s/\s//sg;
    
    # Get the next most recent change number.
    # TODO: Surely SD CLI has a nicer way to do this..
    my @changes = reverse split("\n", execCmd("sd changes -r ...@" . shift . ','));
    my $startChange = (scalar @changes == 1) ? $changes[0] : $changes[1];
    $startChange =~ s/^Change (\d+).*$/$1/ or die "Couldn't extract starting change number for the SD change log.";
    $startChange =~ s/\s//sg;
    
    my $log = "Add SD changes from CL#$startChange to CL#$newestChange.\n\n" . execCmd('sd changes -l ...@' . $startChange . ',');
    return $log;
}

sub getLastSyncedData {
    # Get last change synced
    my ($sdChange, $gitHash);
    if (-e '.chakraSync')
    {
        open my $fh, '+<', '.chakraGit';
        my $line = <$fh>;
        $line =~ /^(\d+),(\w)+$/ or die('.chakraGit sync metadata file is corrupted. Delete it to reset sync.');
        ($sdChange, $gitHash) = ($1, $2);
    } else {
        $sdChange = $OPTIONS{sdChange};
        $gitHash  = $OPTIONS{'gitHash'};
    }
    return ($sdChange, $gitHash);
}

sub stageSDToGit {
    # Get last change synced
    my ($sdChange, $gitHash) = getLastSyncedData();

    # Create git repo locally
    initEnlistmentGit();

    msg('Staging SD changes as a Git commit in local branch SDToGit', $OPTIONS{'Verbose'});

    # Checkout LKG revision
    execCmd('git checkout master -f');
    execCmd('git checkout -b SDToGit');

    # Restore previous sync state.
    execCmd("sd revert ...");
    execCmd("sd sync -f ...@" . $sdChange, NOISY => 1);
    execCmd('git add -A');
    execCmd("git reset --hard $gitHash");
    
    # Update with changes from SD.
    execCmd("sd sync -f ...", NOISY => 1);

    # Add all changes. THIS ASSUMES .gitignore IS UP TO DATE.
    execCmd('git add -A');
    
    my $status = execCmd('git status');
    if ($status =~ /nothing to commit, working directory clean/) {
        # No effective changes from SD.
        msg('No changes from SD. Skipping SD sync.', $OPTIONS{'Verbose'});
        return 1;
    }

    # Get the changelist descriptions for all changes we are adding.
    my $log = generateSDSummary($sdChange);

    # Grab a temporary file to put the message in Git
    # tempFile will evaluate to both the filename and the file handle in the correct contexts automatically.
    my $tempFile = File::Temp->new(UNLINK => 0) or die "Couldn't create temporary file for log message";
    print $tempFile $log;

    # Commit the sd changes
    execCmd("git commit --file=$tempFile");
    #File::Temp::cleanup(); # We no longer need the tempFile and don't need to wait until process completion.

    $OPTIONS{'gitHash'} = execCmd("git rev-parse HEAD");
    $OPTIONS{'gitHash'} =~ s/\s+//g;

    msg('SD changes staged as commit ' .  $OPTIONS{'gitHash'}, $OPTIONS{'Verbose'});

    return 1;
}

sub gitToSD {
    msg('Bringing Git changes back to SD', $OPTIONS{'Verbose'});
    
    # Read last checked-in commit
    # Get last change synced
    my ($sdChange, $gitHash) = getLastSyncedData();

    my $rebaseResult = ''; 
    if ($OPTIONS{'Resume'} =~ /^skip$/i) {
        $rebaseResult = execCmd('git rebase --skip');
    } elsif ($OPTIONS{'Resume'}) {
        $rebaseResult = execCmd('git rebase --continue');
    } else {
        # Checkout the commit
        execCmd('git checkout master --force');
        execCmd("git reset --hard FETCH_HEAD");
        execCmd('git checkout -b gitToSD');

        # Fetch the changes from VSO
        execCmd('git fetch origin master');
    
        # Rebase the changes on top of the SD changes we just committed.
        $rebaseResult = execCmd('git rebase SDToGit', (SurviveError => 1));
    }
    
    # If there are merge issues, send an email.
    if ($rebaseResult =~ /CONFLICT/) {
        $OPTIONS{'Cleanup'} = 0;
        mergeConflictEmail();
        exit(1);
    }

    # Get list of commits to add to SD
    my @revs = reverse split("\n", execCmd("git log --pretty=format:%H $gitHash.."));

    msg("Importing " . scalar @revs . " revision(s) into SD\n", $OPTIONS{'Verbose'});
    debug("Revision list:\n" . join "\n", @revs);

    foreach my $rev (@revs) {
        # Skip undefined and empty lines
        next unless defined $rev and $rev =~ /\w+/;

        # Check for an empty commit. We can find these by checking for non-whitespace in the diff output.
        if (`git diff $rev~1..$rev` !~ /\w+/) {
            msg("Skipping empty git commit $rev", $OPTIONS{'Verbose'});
        }

        # Prepare the Git changes for staging to SD.
        msg("Checking out revision $rev\n", $OPTIONS{'Verbose'});
        execCmd("sd revert ...");
        execCmd("git checkout $rev --force");
        
        # Remove all excess files
        execCmd("git add --all");
        execCmd("git reset --hard");
        
        # Add files to SD changelist and remove empty changes
        execCmd("sd online");
        execCmd("sd revert -a ...");
        
        my $openedFiles = execCmd("sd opened ...");
        if ($openedFiles =~ /\.\.\. - file\(s\) not opened on this client\./) {
            msg("Skipping delta empty commit $rev (no effective changes)", $OPTIONS{'Verbose'});
            next;
        }
        
        # Added files need to be explicitly added. We can use the list of changed files in the diff to get these.
        # git diff --name-status will output added files like the following:
        # A path/to/added/file
        # We need to:
        # - filter just those lines (split/grep),
        my @changedFiles = split("\n", execCmd("git diff --name-status $rev~1..$rev"));
        # - grab the path (grep regex), and
        my @addedFiles = grep s/^A\s+(.*?)/$1/g, @changedFiles;
        # - replace forward slashes with backslashes
        @addedFiles = grep s/\//\\/g, @addedFiles;
        foreach my $addedFile (@addedFiles) {
            execCmd("sd add $addedFile");
        }
        
        # Format the Git log to be useful for SD. This primarily involves having the title on the first line (for sdv,)
        # and adding the commit hash to the SD changelist description.
        msg("Processing log for revision $rev\n", $OPTIONS{'Verbose'});
        my $log = execCmd("git log -n 1 --format=\"%s%n%nCommitted as %H by %an <%ae> at %ad%n%n%b%n\" $rev");

        # Grab the author of the file and attempt to prefix the correct domain qualifier
        my $author = execCmd('git log ' . $rev . ' -n 1 --format="%ae"');
        $author =~ /(\w+)@/ or die "Couldn't determine alias from log:\n$author\n";
        $author = $1;
        if ($author =~ /tawoll/i || $author =~ /yongqu/i) {
        # if (scalar grep qr/$author/i, $OPTIONS{'NTDEVAliases'}) {
            $author = "NTDEV\\" . $author;
        } else {
            $author = "REDMOND\\" . $author;
        }

        # Add SD trigger if the log doesnt' have one
        if ($log !~ /(FW|MSFT|NIB|BUILD):/) {
            msg("No SD trigger found, prefixing FW:", $OPTIONS{'Verbose'});
            $log = 'FW: ' . $log;
        }
        
        # Suffix [Git <repo>] to the first line so we can see that it is a git change when viewing in SDV.
        if ($OPTIONS{'Repo'} !~ /core/i) {
            $log =~ s/\n/ [Git FULL]\n/;
        } else {
            $log =~ s/\n/ [Git CORE]\n/;
        } 
        $log =~ s/\n/\n\t/g;

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
            msg("Submitting as $author using $submittedBy\n", $OPTIONS{'Verbose'});
            my $result = execCmd("sd submit -u $author -i <$tempFile", (Skip => 1));
        }
        if ($OPTIONS{'Repo'} =~ /core/i) {
            $CORE_SD_CHANGED = 1;
        } else {
            $PRIVATE_SD_CHANGED = 1;
        }
        #File::Temp::cleanup(); # We no longer need the tempFile and don't need to wait until process completion.

        # Write checked-in commit.
        my $newChangeList = `sd counter change`;
        msg('Changes submitted as SD change ' . $newChangeList, $OPTIONS{'Verbose'});
    }

    # Push the rebased changes back to VSO. We already resolved the conflicts, so just pull in all the changes.
    execCmd("git checkout master");
    execCmd("git merge --squash --strategy=recursive --strategy-option=theirs SDToGit");

    # Update the submodule reference for full.
    if ($OPTIONS{'Repo'} !~ /core/i) {
        execCmd("git submodule init");
        execCmd("git submodule update");
        execCmd("git add core");
    }

    # We have to regenerate the message here in case this is a resume. TODO: This logic could be smarter.
    # Get the changelist descriptions for all changes we are adding.
    my $log = generateSDSummary($sdChange);
    my $tempFile = File::Temp->new(UNLINK => 0) or die "Couldn't create temporary file for merge description";
    print $tempFile $log;
    execCmd("git commit --file=$tempFile");
    
    execCmd("git push origin master", (Skip => 1));
    if ($OPTIONS{'Repo'} =~ /core/i) {
        $CORE_VSO_CHANGED = 1;
    } else {
        $PRIVATE_VSO_CHANGED = 1;
    }

    submitSNAP();

    return 1;
}

chdir(($OPTIONS{'Repo'} =~ /core/i) ? $OPTIONS{'CoreSDEnlistment'} : $OPTIONS{'RootSDEnlistment'});

my $result = 0;
if (!$OPTIONS{'Resume'}) {
     $result = eval {
        stageSDToGit();
    };
}
if ($result || $OPTIONS{'Resume'}) {
    $result = eval {
        gitToSD();
    };
}

summaryEmail();

cleanUp();

