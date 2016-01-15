use strict;
use warnings;

our %OPTIONS;

require "Common.pl";
require "Config.pl";

use POSIX qw(strftime);

$OPTIONS{'Verbose'} = 2;

sub sendErrorEmail {
    my $subject = 'ACTION REQUIRED: Chakra VSO->GitHub Sync failed for ' . strftime("%x %X", localtime);
    my $body = <<"END";
VSO to GitHub pump has detected a conflicting commit on GitHub and cannot fast forward. Please investigate GitHub.

The pump has been stopped and must be MANUALLY restarted. Restart the 'VSO to GitHub Pump' scheduled task on ChakraGit when issues are resolved and a fast-forward merge is possible.

-Chakra VSO -> GitHub sync service
END
    sendMail(
        subject => $subject,
        body    => $body
    );

    debug("$subject\n\n$body", $OPTIONS{'Verbose'});

    exit(1);
}

# Error handler. If we get an error, we need to email the error and gracefully exit.
my $originalDieHandler = $SIG{__DIE__};
$SIG{__DIE__} = sub {
    # Restore the error handler so that we can die() for real if something goes wrong.
    $SIG{__DIE__} = $originalDieHandler;

    my $error = shift;

    error($error. Carp::longmess());

    sendErrorEmail();

    exit(1);
};

msg('Starting sync', $OPTIONS{'Verbose'});

if (not -e $OPTIONS{'GitHubRepoName'}) {
    msg('ChakraCore enlistment not found, cloning...', $OPTIONS{'Verbose'});
    execCmd("git clone $OPTIONS{'GitHubURL'}");

    chdir($OPTIONS{'GitHubRepoName'});
    execCmd("git remote add upstream $OPTIONS{'VSOCoreURL'}");
    
    execCmd('git config user.name ChakraBot');
    execCmd('git config user.email chakrabot@users.noreply.github.com');
} else {
    msg('Cached ChakraCore enlistment found, resetting state...', $OPTIONS{'Verbose'});
    chdir($OPTIONS{'GitHubRepoName'});
    execCmd('git checkout master -f');
    execCmd('git reset --hard origin/master');
}

# Get update from github
msg('Fetching changes', $OPTIONS{'Verbose'});
execCmd("git fetch origin master");
execCmd("git fetch upstream master");

msg('Attempting merge', $OPTIONS{'Verbose'});
execCmd("git merge upstream/master --ff-only");

msg('Fast-forward successful, pushing changes to GitHub', $OPTIONS{'Verbose'});
execCmd("git push origin master", (SurviveError => 1));

msg('Verifying there are no extra commits', $OPTIONS{'Verbose'});
my $extraGitCommits = execCmd("git rev-list upstream/master..origin/master");

if ($extraGitCommits =~ /\w+/g) {
    sendErrorEmail();
}

msg('Sync completed successfully', $OPTIONS{'Verbose'});