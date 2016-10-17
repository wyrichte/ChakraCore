# Medusa branch mirrorer. Takes a map of remotes and branches and mirrors them, contacting owners on unexpected errors.

use strict;
use warnings;

require "MedusaConfig.pl";

use POSIX qw(strftime);
use Storable;
use Log::Message::Simple;

# Install these modules with CPAN
use Capture::Tiny qw(tee_merged capture_merged);
use Mail::Sender;

# Import settings from config
our $PROJECT;
our $PROJECT_OWNERS;
our $EMAIL_AUTH;
our $REMINDER_INTERVAL;
our $REMOTES;
our $BRANCH_DATA;

# Static script settings
my $BLACKLIST_DATA_LOCATION = "..\\.branchBlacklist";
my $VERBOSITY = 1; # Default verbosity
my $DRY_RUN = 0; # Debug commands

#
# Utilities
#

# Execute wrapper
sub execCmd {
    my $command = shift;

    # Execute options
    # - HasSideEffects: Skip the command if true (usually any command that results in permanent changes)
    # - SurviveError: Do not die on non-zero exit code. Allows custom error handling, e.g. rebase merge conflict.
    # - ReturnExitCode: Returns the exit code instead of the output. Implies SurviveError.
    # - Noisy: Force another level of verbosity before we output this command
    my %execOptions = @_;

    $execOptions{SurviveError} = 1 if $execOptions{ReturnExitCode};

    if ($DRY_RUN && $execOptions{HasSideEffects}) {
        msg("Skipping: $command", $VERBOSITY);
        return '';
    } else {
        my ($output, $exit, $spawnError);
        msg("Executing '$command'"
            . (($execOptions{Noisy} && $VERBOSITY > 1)
                ? ' [silenced output - increase verbosity to see it]'
                : '')
            . ($execOptions{SurviveError} ? ' [errors OK]' : ''), 1);
        if (($VERBOSITY > 1 && !$execOptions{Noisy})
            || $VERBOSITY > 2) {
           ($output, $exit) = tee_merged {
                system($command);
            };
        } else {
            ($output, $exit) = capture_merged {
                system($command);
            };
        }

        if ($execOptions{SurviveError} || $exit =~ /^0$/) {
            debug("Ignoring exit code $exit\n") if $execOptions{SurviveError};
            debug($output);
        } else {
            die "Command '$command' failed with exit code $exit. Output follows:\n$output";
        }
        return $execOptions{ReturnExitCode} ? $exit : $output;
    }
}

sub sendEmail {
    my %args = @_;
    print ($args{body}) if $VERBOSITY > 2;

    # Fetch the log stack for inclusion.
    my $msgs  = Log::Message::Simple->stack_as_string;
    # Sanitize tokens
    $msgs =~ s/chakrabot:\w+@/chakrabot:<censored>@/;
    print $msgs if $VERBOSITY > 2;
    $args{body} .= "\n\n--- LOG FOLLOWS ---\n\n$msgs\n";

    msg("Opening email connection...", 1);
    my $sender = new Mail::Sender {
        smtp => $EMAIL_AUTH->{Host}, #disguised
        on_errors=>'die',
        from => $EMAIL_AUTH->{From},
        to => $args{to},
        subject => $args{subject},
        auth =>'NTLM',
        authid => $EMAIL_AUTH->{Username},
        authpwd => $EMAIL_AUTH->{Password},
        authdomain => $EMAIL_AUTH->{Domain}
    }
    or die "$Mail::Sender::Error\n";

    msg("Sending email...", 1);
    $sender->MailMsg({msg => $args{body}});
}

sub initClone {
    if (-e "$PROJECT\\.git") {
        msg("Cached $PROJECT found...", $VERBOSITY);
        return;
    }
    msg("$PROJECT not found, creating...", $VERBOSITY);
    execCmd("mkdir $PROJECT");
    chdir($PROJECT);
    execCmd("git init");

    for my $remote (@$REMOTES) {
        execCmd("git remote add $remote->{Name} $remote->{URL}");
    }
}

#
# Blacklisting functionality - if a branch fails, we don't want to keep emailing about it. Keep a blacklist of branches.
#

my $blacklistRef = {};

sub writeBlacklistedBranches {
    if (scalar keys %$blacklistRef < 1) {
        if (-e $BLACKLIST_DATA_LOCATION) {
            msg("Empty blacklist after removal, deleting blacklist.", $VERBOSITY);
            unlink $BLACKLIST_DATA_LOCATION;
            return;
        } else {
            msg("Empty blacklist, skipping write.", $VERBOSITY);
            return;
        }
    }
    # Store outside, 'just to be sure.'
    store $blacklistRef, $BLACKLIST_DATA_LOCATION;
    msg("Wrote blacklist to $BLACKLIST_DATA_LOCATION", $VERBOSITY);
}

sub readBlacklistedBranches {
    if (!-e $BLACKLIST_DATA_LOCATION) {
        msg('No blacklist found', $VERBOSITY);
        return;
    }

    $blacklistRef = retrieve $BLACKLIST_DATA_LOCATION;
    my $blacklistSize = scalar keys %$blacklistRef;
    msg("Read blacklist with $blacklistSize entries", $VERBOSITY);

    for my $branch (keys %$blacklistRef) {
        msg("\t$branch with $blacklistRef->{$branch} attempts", $VERBOSITY);
    }
}

#
# Email functionality
#

# Very verbose success email.
sub sendSuccessEmail {
    msg("Generating success email...", 1);
    my $time = strftime('%Y/%m/%d', localtime);
    my $branches = "";
    for my $branch (@$BRANCH_DATA) {
        $branches .= "$branch->{FromRemote}/$branch->{FromBranch} -> $branch->{ToRemote}/$branch->{ToBranch}\n";
    }

    my $subject = "$PROJECT mirroring service completed successfully";
    my $body = <<"END";
$PROJECT mirroring completed successfully at $time.

Branches mirrored:
$branches
END

    sendEmail((
        subject => $subject,
        body => $body,
        to => $PROJECT_OWNERS
    ));
}

sub sendIncomingCommitsEmail {
    my $branchInfo = shift;

    my $commits = shift;

    my $commitLog = "";
    for my $commit (split "\n", $commits) {
        $commitLog .= execCmd("git log -n 1 --oneline $commit");
    }

    my $subject = "$PROJECT mirroring service failed for $branchInfo->{FromRemote}/$branchInfo->{FromBranch} -> $branchInfo->{ToRemote}/$branchInfo->{ToBranch}";
    my $body = <<"END";
$PROJECT mirroring failed with conflicting commits for $branchInfo->{FromRemote}/$branchInfo->{FromBranch} -> $branchInfo->{ToRemote}/$branchInfo->{ToBranch}

The following commits were not expected on $branchInfo->{ToRemote}/$branchInfo->{ToBranch}. Carefully verify the authenticity of the commits and resolve the differences by merging them into $branchInfo->{FromRemote}/$branchInfo->{FromBranch}.

Commit list:
$commitLog

END

    sendEmail((
        subject => $subject,
        body => $body,
        to => $branchInfo->{Owners}
    ));
}

sub sendBranchResolvedEmail {
    my $branchInfo = shift;

    my $subject = "$PROJECT mirroring service: $branchInfo->{FromRemote}/$branchInfo->{FromBranch} -> $branchInfo->{ToRemote}/$branchInfo->{ToBranch} was successfully resolved";
    my $body = <<"END";
$PROJECT mirroring was successfully resolved for $branchInfo->{FromRemote}/$branchInfo->{FromBranch} -> $branchInfo->{ToRemote}/$branchInfo->{ToBranch}

END

    sendEmail((
        subject => $subject,
        body => $body,
        to => $branchInfo->{Owners}
    ));
}

sub updateBranch {
    my $branchInfo = shift;

    my $FromBranch = $branchInfo->{FromBranch};
    my $ToBranch = $branchInfo->{ToBranch};
    my $ownerRef = $branchInfo->{Owner};

    # Check that there are no extra commits on either side.
    my $extraGitCommits = execCmd("git rev-list $branchInfo->{FromRemote}/$FromBranch...$branchInfo->{ToRemote}/$ToBranch",  (SurviveError => 1));

    if ($extraGitCommits !~ /\w+/) {
        if ($blacklistRef->{$ToBranch}) {
            # The branch seems to have the conflicts resolved. Remove it from the blacklist.
            msg("Branch '$ToBranch' is already on the blacklist and has been resolved after $blacklistRef->{$ToBranch} mirror attempts. Removing from the blacklist.", $VERBOSITY);
            delete $blacklistRef->{$ToBranch};
            sendBranchResolvedEmail($branchInfo);
        }
        msg('No new changes, skipping merge.', $VERBOSITY);
        return;
    }

    # Check there are no extra commits on the destination remote. E.g. from the big green button.
    $extraGitCommits = execCmd("git rev-list $branchInfo->{FromRemote}/$FromBranch..$branchInfo->{ToRemote}/$ToBranch",  (SurviveError => 1));
    if ($extraGitCommits =~ /\w+/g) {
        msg("Branch '$ToBranch' has incoming commits from the destination.", $VERBOSITY);
        if ($blacklistRef->{$ToBranch}) {
            ++$blacklistRef->{$ToBranch};
            if ($blacklistRef->{$ToBranch} % $REMINDER_INTERVAL == 0) {
                msg("Branch '$ToBranch' is already on the blacklist with $blacklistRef->{$ToBranch} mirror attempts. Sending a reminder.", $VERBOSITY);
                sendIncomingCommitsEmail($branchInfo, $extraGitCommits);
            } else {
                msg("Branch '$ToBranch' is already on the blacklist with $blacklistRef->{$ToBranch} mirror attempts. Skip sending an email.", $VERBOSITY);
            }
            return;
        }
        msg("Branch '$ToBranch' has conflicts. Adding to blacklist.", $VERBOSITY);
        sendIncomingCommitsEmail($branchInfo, $extraGitCommits);
        $blacklistRef->{$ToBranch} = 1;
        return;
    }

    msg("Fetching changes for $FromBranch", $VERBOSITY);

    # Check out the target branch and nuke the clone.
    execCmd("git checkout -B $ToBranch github/$FromBranch");

    msg("Attempting merge of $FromBranch", $VERBOSITY);
    execCmd("git merge $branchInfo->{FromRemote}/$FromBranch --ff-only");

    msg("Fast-forward successful, pushing changes to $branchInfo->{ToRemote}", $VERBOSITY);
    execCmd("git push $branchInfo->{ToRemote} $FromBranch:$ToBranch");
}

sub mirrorBranches {
    msg('Starting mirroring', $VERBOSITY);

    initClone();

    chdir($PROJECT);

    readBlacklistedBranches();

    my $remoteCount = scalar @$REMOTES;
    my $remoteIndex = 0;
    for my $remote (@$REMOTES) {
        ++$remoteIndex;
        msg("[$remoteIndex/$remoteCount] Fetching remote $remote->{Name}...", 1);
        execCmd("git fetch $remote->{Name}");
    }

    my $branchCount = scalar @$BRANCH_DATA;
    my $branchIndex = 0;
    for my $branchInfo (@$BRANCH_DATA) {
        ++$branchIndex;
        msg("[$branchIndex/$branchCount] Processing branch $branchInfo->{FromBranch}...", 1);
        updateBranch($branchInfo);
    }

    writeBlacklistedBranches();

    sendSuccessEmail() if $VERBOSITY > 1;

    msg('Mirroring completed successfully', $VERBOSITY);
}

# Error handler. If we get an error, we need to email the error and gracefully exit.
my $originallDieHandler = $SIG{__DIE__};
$SIG{__DIE__} = sub {
    # Restore the error handler so that we can die() for real if something goes wrong.
    $SIG{__DIE__} = $originallDieHandler;

    writeBlacklistedBranches();

    my $error = shift;

    error($error. Carp::longmess());

    sendFatalErrorEmail();

    exit(1);
};

#
# Entry point
#
mirrorBranches();
exit(0);
