#This script does a basic DRT test failure analysis and generates a csv file with results
#   - Parse last 'n' DRT job logs given a branch from the \\iesnap share.
#   - Look for failed DRTs and do basic analysis and categorization of failures:
#        - Ignored failures: Depending on inetcore\mshtml\src\f3\drt\drt-suite.xml DRT failures can be automatically ignored in the following scenarios:
#           $test->{status} == labpending                                     =>     ignore test failure
#           $test->{status} eq "unstable" || $test->{status} =~ /^disabled/  =>  retry tests & ignore if it eventually passed
#        - Passed on manual re-run: Required ChakraHot intervention to be re-run to get it to pass.
#        - Failed: Failed causing the snap job to fail - This might be a case of genuine failure or queue was running automated and the job got aborted because of this 'flaky' failure.

#use strict;
use warnings;
use JSON;
use File::Basename;

my $debug=0;
my $headerParsed = 0;
my @jobs;
my @failures;
my @ignoredFailures;
my @passOnRerun;
my $header;
my %rerunsPerFile;
my $branch=$ENV{'_BuildBranch'};
my $checkins=30;
my $days=-1;
my $outputFile="out.csv";

# Enumerate last "$count" job dirs, or dirs modified in last "$days" (ignore "$count" if specified)
# to mine DRT logs.
#
#   enumerateLatestDirs($count, $days)
#
sub enumerateLatestDirs
{
    my ($count, $days, $topdir) = @_;
    my $first = -1;
    if ($days && $days > 0) {
        $first = time() - $days * 24 * 3600;
        $count = -1; #ignore count if $days specified
    }
    
    opendir(my $DH, $topdir) or die "Error opening $topdir: $!";
    my @files = map { [ stat "$topdir\\$_", $_ ] } grep(! /^\.\.?$/, readdir($DH));
    closedir($DH);
    sub rev_by_date { $b->[9] <=> $a->[9] }
    my @sorted_files = sort rev_by_date @files;
    my @return_result;
    foreach my $line (@sorted_files) {
        last if $line->[9] < $first;
        push(@return_result, "$topdir\\$line->[13]");
        last if scalar(@return_result) == $count;
    }
    
    return @return_result;
}

sub getFileTime
{
    my $f = shift;
    if($debug)
    {
        print "Getting file time: ${f}\n";
    }
    return (stat($f))[9] || undef;
}

# EDMAURER best effort to find something useful when the failure type is "Other"
sub findFailedSuite
{
	my $logFileName = shift;

	open(LOG, "<", $logFileName) or die "couldn't open log file $logFileName";
	while (<LOG>)
	{
		chomp;
		my $line = $_;

		if (my ($suiteName) = $line =~ m/suite FAILED (.+)/)
		{
			close(LOG);
			return $suiteName;
		}

		if ($line =~ m/== END OF TEST SUMMARIES/)
		{
			chomp($line = <LOG>);
			my ($suiteName) = $line =~ m/Test scripts '(.+)' failed/;
			close(LOG);
			return $suiteName;
		}
	}

	close(LOG);

	return $logFileName =~ m/.*\\([^\.]+)/ ? $1 : undef;
}

sub determineJobAbortReason
{
	my ($jobdir) = @_;
	my $failureFileName = $jobdir . '\\' . 'failures.json';
    my @failureReason;
	if (-f $failureFileName)
	{
		open(INFO, "<", $failureFileName);
		my $line = <INFO>;
		close(INFO);

		my $allFailuresHash = from_json($line);
		my $failureHash = $allFailuresHash->{failures}[0];
		push(@failureReason, $failureHash->{type});
        
         
		# EDMAURER I have seen "Other" as the failure type
		# when tests time out, also when JSProjection tests fail.
		# Try to dig deeper in that case.

		if ($failureReason[0] eq "Other")
		{
			#look at log file name
			my @logs = keys $failureHash->{logs};
            my $nonDRTFailures = scalar @logs - grep(/\w*RunDRTBucket\w*/, @logs);
			foreach my $log(@logs)
            {
                if($log =~ /\w*RunDRTBucket\w*/)
                {
                    #If there are non-DRT failures - then let's ignore the DRT failures as the primary reason for job failure
                    if(!$nonDRTFailures)
                    {
                        if (my $failedSuite = findFailedSuite($log))
                        {
                           push(@failureReason, ${failedSuite});
                        }
                    }
                }
                else
                {
                    
                    my ($testName, $machine, $arch) =  split('\.', fileparse($log));
                    my $reason = $testName;
                    $reason .= "(${arch})" if $arch;
                    push(@failureReason, $reason);
                }
            }
		}
	}
    else
    {
        push(@failureReason, "unknown");
    }
	return @failureReason;
 
}

sub findJobInfo
{
    my ($jobdir) = @_;
    
    opendir(dirHandle, $jobdir) || die("Cannot open directory");
    my @extendDRTVM =  grep (/\ExtendDrtVMExpiration.*/, readdir(dirHandle));
    rewinddir(dirHandle);
    my @sendCheckinMail =  grep (/\SendCheckinMail.*/, readdir(dirHandle));
    rewinddir(dirHandle);
    my @sendAbortMail =  grep (/\SendAbortJobMail.*/, readdir(dirHandle));
    closedir(dirHandle);
    
    my $job = {
        name => ($jobdir =~ m/\\([^\\]+\\[^\\]+)$/ ? $1 : $jobdir),
        start => (@extendDRTVM ? getFileTime($jobdir . '\\' . $extendDRTVM[0]) : undef),
        abort => (@sendAbortMail  ? getFileTime($jobdir . '\\' . $sendAbortMail[0]) : undef),
        submit => (@sendCheckinMail ? getFileTime($jobdir . '\\' . $sendCheckinMail[0]) : undef),
    }; 
    
    if ($job->{start} && ($job->{submit} || $job->{abort})) {
        $job->{completedTime} = $job->{submit} || $job->{abort};
        $job->{time} = $job->{completedTime} - $job->{start};
    }

	if ($job->{abort})
	{
		$job->{failureReason} = [determineJobAbortReason($jobdir)];
	}

    return $job;
}

sub addJobPassOnRerun
{
    my ($job, $testName) = @_;
    unless ($job->{passOnRerun}) {
        $job->{passOnRerun} = {};
    }
    $job->{passOnRerun}->{$testName} = ();
}

sub printJobsSummary
{
    printf("\n%-28s%-5s%-8s%-12s%s\n", 'Job', 'P/F', 'Time(h)', 'Completed', 'Pass_On_Rerun; Failure reason');

    my $submits = 0;
    sub sortByTime { $b->{completedTime} <=> $a->{completedTime} }
    my @sortedByTime = sort sortByTime @jobs;
    my @months = qw( Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec );
    foreach my $job (@sortedByTime)
    {
        my ($min, $hour, $mday, $mon) = (localtime $job->{completedTime}) [1, 2, 3, 4];
        printf("%-28s%-5s%-7.1f %02d %s %02d:%02d %s; %s\n",
            $job->{name},
            ($job->{submit} ? "P" : "F"),
            $job->{time} / 3600,
            $mday, $months[$mon], $hour, $min,
            $job->{passOnRerun} ? join(",", keys %{$job->{passOnRerun}}) : "      " ,
            scalar $job->{failureReason} ? join(",", @{$job->{failureReason}}) : "",
            );
        if ($job->{submit}) { $submits++; }
    }
    printf("\nJobs succeeded: %d/%d\n\n", $submits, scalar @jobs);
}

sub printFailureSummary
{
    print "Failure summary\n";
    my %failures;
    foreach my $job(@jobs)
    {
        foreach my $failureReason(@{$job->{failureReason}})
        {
            if(!$failures->{$failureReason})
            {
                $failures->{$failureReason} = [];
            }
            push (@{$failures->{$failureReason}}, $job->{name});
        }
    }
    printf("%-38s %s\n", "Failure reason", "Jobs");
    foreach my $failure(keys %{$failures})
    {
        my $header = 1;
        foreach my $jobName(@{$failures->{$failure}})
        {
            if($header)
            {
                printf("(%d)%-35s %s\n", scalar @{$failures->{$failure}}, $failure, $jobName);
                $header = 0;
            }
            else
            {
                printf("%-38s %s\n", "", $jobName);
            }
        }
    }
}

sub processFile
{
    my $dir = shift;
    opendir(dirHandle, $dir) || die("Cannot open directory");
    my @jobdirs = map { $dir . "\\" . $_ } grep (/\w*Job\w*/, readdir(dirHandle));
    closedir(dirHandle);

    foreach my $jobdir(@jobdirs)
    {
        if ($debug)
        {
            print "Opening dir: ${jobdir}\n";
        }
        my $job = findJobInfo($jobdir);
        next if not $job->{time}; #Only check completed (submitted or aborted) jobs
        push(@jobs, $job);
        
        opendir(dirHandle, $jobdir) || die("Cannot open directory");
        my @logFileList =  grep (/\w*RunDRTBucket\w*/, readdir(dirHandle));
        closedir(dirHandle);
        
        foreach my $logFile(@logFileList)
        {
            my $fullName = $jobdir . "\\" . $logFile;
            if ($debug)
            {
                print "Opening file: ${fullName}\n";
            }
            open(INFO, $fullName) or die("Could not open  file: ${fullName}");
            
            my $process = 0;
            my $succeeded = 0;
            my $currentFailureCount = scalar(@failures);
            my $headerCountInFile = 0;
            my $failed = 0;
            foreach my $line (<INFO>)  {   
                if($line =~ /^Result,FailureType/)
                {
                    if($headerParsed == 0)
                    {
                        $header = "LogPath,ManualRerunCount," . $line;
                        $headerParsed = 1;
                    }
                    $process = 1;
                    $succeeded = 0;
                    $headerCountInFile++;
                }
                elsif ($process)
                {
                    if($line =~ /^suite ([^ ]*)/)
                    {
                        if($1 =~ /SUCCEEDED/)
                        {
                            $succeeded = 1;
                        }
                        elsif($1 =~ /FAILED/)
                        {
                            $failed = 1;
                        }
                        $process = 0;
                    }
                    elsif($line =~ /^F,/)
                    {
                        push(@failures, "${fullName},${headerCountInFile},${line}");
                    }
                }
            }
            $rerunsPerFile{$fullName} = $headerCountInFile if $headerCountInFile > 1 ;
            close(INFO);
            if($succeeded)
            {
                #within the same log file - if we had failed in an earlier run & now passed
                #because of a re-run - we mark it passed on re-run bucket.
                if($failed == 1)
                {
                    my $passedOnRerunCount = scalar(@failures) - $currentFailureCount;
                    while($passedOnRerunCount > 0)
                    {
                        my $currentline = pop(@failures);
                        my ($f0,$f1,$f2,$f3,$action,$f5,$f6,$testName) = split(',', $currentline);
                        if ($action eq 'I' || $action eq 'R') #Action == Ignore or Retry
                        {
                            push(@ignoredFailures, $currentline);
                        }
                        elsif ($action eq 'A') #Action == Abort
                        {
                            push(@passOnRerun, $currentline);
                            addJobPassOnRerun($job, $testName);
                        }
                        else
                        {
                            printf("***ERROR: Unknown DRT action (%s) in %s\n", $action, $currentline);
                        }
                        $passedOnRerunCount--;
                    }
                }
                else
                {
                    my $ignoredFailureCount = scalar(@failures) - $currentFailureCount;
                    while($ignoredFailureCount > 0)
                    {
                        my $currentline = pop(@failures);
                        push(@ignoredFailures, $currentline);
                        $ignoredFailureCount--;
                    }
                }
            }
        }
    }
}


sub printOutput()
{
    open(OUT, ">${outputFile}") or die("Could not open  file: ${outputFile}\n");;
    if(@failures || @ignoredFailures || @passOnRerun)
    {
        print "Branch: ${branch}\n";
        if ($days > 0) {
            print "Days: ${days}\n";
        } else {
            print "Checkins: ${checkins}\n";
        }
        printf("Jobs processed: %d\n\n", scalar @jobs);
        
        print OUT "Status,${header}";
        foreach my $line (@failures) {
           print OUT "failed,${line}";
        }

        foreach my $line (@ignoredFailures) {
           print OUT "auto ignored,${line}";
        }
        
        # Tests requiring manual re-run
        {
            my %tests = ();
            foreach my $line (@passOnRerun) {
                print OUT "passed on re-run,${line}";                
                my @values = split(',', $line); 
                my $testName = $values[7];
                my $arch = $values[12];
                my $logfile=$values[0];
                # Info from all branches here: http://iesnap/snapui/q/MostRecentResultsOfDrt.aspx?TestName=${testName}&BuildType=${arch}
                unless (exists $tests{$testName})
                {
                    $tests{$testName} = [];
                }
                push($tests{$testName}, $logfile);
            }
            
            printf("Tests requiring manual re-run: %d\n", scalar keys %tests);
            foreach my $testName (sort keys %tests)
            {
                my $logs = $tests{$testName};
                printf("(%d)%-18s %s\n", scalar @$logs, $testName, @$logs[0]);
                for (my $i = 1; $i < scalar @$logs; $i++)
                {
                    printf("%-22s %s\n", '', @$logs[$i]);
                }
            }
            print "\n";
        }
        
        close(OUT);
        print "Output file generated: ${outputFile}\n";
    }
    else
    {
        print "No failures found";
    }
}

sub parseArgs()
{
    if(@ARGV == 1 && $ARGV[0] =~ /[-\/]\?/)
    {
        usage();
        exit(0);
    }

    for(my $i = 0; $i < @ARGV; ++$i)
    {
        if($ARGV[$i] =~ /[-\/]branch:(.*)$/)
        {
            if($1)
            {
                $branch = $1;
            }
        }
        elsif($ARGV[$i] =~ /[-\/]checkins:(\d+)$/)
        {
            $checkins = $1;
        }
        elsif($ARGV[$i] =~ /[-\/]days:(\d+)$/)
        {
            $days = $1;
        }
        elsif($ARGV[$i] =~ /[-\/]out:(.*)$/)
        {
            if($1)
            {
                $outputFile = $1;
            }
            else
            {
                badSwitch();
            }
        }
        else
        {
            badSwitch($ARGV[$i]);
        }
    }
}
sub badSwitch()
{
   my $switch = shift;
   usage();
   die "bad commandline switch: ${switch}";
}
sub usage()
{
    print "Usage: analyzeDrt.pl [options]\n";
    print "Options:\n";
    print "  -branch:<branch>        Specifies the branch to analyze DRT failures for. Default: ${branch}\n";
    print "  -checkins:<checkins>    Specifies number of last check-ins to analyze. Default: ${checkins}\n";
    print "  -days:<days>            Specifies number of days (dateback from now) to analyze. Ignore -checkins:.\n";
    print "  -out:<outputFile>       Specifies name of output file to generate Default:{$outputFile}\n";
}

parseArgs();
my $topdir="\\\\iesnap\\queue\\${branch}";
my @latestDirs = enumerateLatestDirs($checkins, $days, $topdir);

$| = 1;
{
    print "Processing\n";
    my $i = 0;
    my $all = scalar @latestDirs;
    foreach my $dir (@latestDirs) {
        $i++;
        printf("%-52s $i/$all\n", $dir);
        processFile($dir);
    }
    print("\n");
}

printOutput();
printJobsSummary();
printFailureSummary();