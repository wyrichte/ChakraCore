our %OPTIONS;

use Capture::Tiny qw(capture_merged tee_merged);
use Data::Dumper;
use Log::Message::Simple;

sub execCmd {
    my $command = shift;

    # Execute options
    # - HasSideEffects: Skip the command if true (usually any command that results in permanent changes)
    # - SurviveError: Do not die on non-zero exit code. Allows custom error handling, e.g. rebase merge conflict.
    # - ReturnExitCode: Returns the exit code instead of the output. Implies SurviveError.
    # - Noisy: Force another level of verbosity before we output this command
    my %execOptions = @_;

    $execOptions{'SurviveError'} = 1 if $execOptions{'ReturnExitCode'};

    if ($OPTIONS{'DryRun'} && $execOptions{'HasSideEffects'}) {
        msg("Skipping: $command", $OPTIONS{'Verbose'});
        return '';
    } else {
        my ($output, $exit, $spawnError);
        msg("Executing '$command'"
            . (($execOptions{'Noisy'} && $OPTIONS{'Verbose'} > 1)
                ? ' [silenced output - increase verbosity to see it]'
                : '')
            . ($execOptions{'SurviveError'} ? ' [errors OK]' : ''), 1);
        if (($OPTIONS{'Verbose'} > 1 && !$execOptions{'Noisy'})
            || $OPTIONS{'Verbose'} > 2) {
           ($output, $exit) = tee_merged {
                system($command);
            };
        } else {
            ($output, $exit) = capture_merged {
                system($command);
            };
        }

        if ($execOptions{'SurviveError'} || $exit =~ /^0$/) {
            debug("Ignoring exit code $exit\n") if $execOptions{'SurviveError'};
            debug($output);
        } else {
            die "Command '$command' failed with exit code $exit. Output follows:\n$output";
        }
        return $execOptions{'ReturnExitCode'} ? $exit : $output;
    }
}

sub sendMail {
    my %args = @_;
    print ($args{body}) if $OPTIONS{'Verbose'} > 2;

    if (!$OPTIONS{'Email'}) {
        msg("Email NOT sent due to run settings");
        return;
    }

    # Fetch the log stack for inclusion.
    my $msgs  = Log::Message::Simple->stack_as_string;
    # Sanitize tokens
    $msgs =~ s/chakrabot:\w+@/chakrabot:<censored>@/;
    print $msgs if $OPTIONS{'Verbose'} > 2;
    $args{body} .= "\n\n--- LOG FOLLOWS ---\n\n$msgs\n";

    require $ENV{'sdxroot'} . '\TOOLS\sendmsg.pl';
    sendmsg($OPTIONS{'ServiceAccountEmail'},
            $args{subject},
            $args{body},
            'tcare@microsoft.com');#, 'cc:curtism@microsoft.com', 'cc:hiteshk@microsoft.com', 'cc:doilij@microsoft.com', 'cc:chakrahot@microsoft.com');
    msg("Email sent");
}
1;