#
# Script used for perf testing for Language Service backend.  This script supports "official" mode, which dumps an XML file
# filled with all the data for analysis or sucking into Excel.  The XML file schema was chosen so we could steal
# some already written perf analysis tools from the C++ team.
#

use Win32::ODBC;
use strict;
use Cwd;

my $iter = 35;

# test class
my @variants = ("DomTests" , "DomAsContextFileTests",  "JQueryTests", "Box4Tests", "Box4AsContextFileTests", "SolitaireTests", "OpenMailTests");

# test name -- each line maps to a variant
my @tests = (
              # Dom Tests
              ["GetFunctionHelpFirstRequest", "GetFunctionHelpSecondRequest", "GetRegionsFirstRequest", "GetRegionsSecondRequest", "GetCompletionsFirstRequest" , "GetCompletionsSecondRequest" , "GetASTFirstRequest", "GetASTSecondRequest", "GetMessagesFirstRequest", "GetMessagesSecondRequest"],
              # Dom As Context File Tests
              ["GetFunctionHelpFirstRequest", "GetFunctionHelpSecondRequest","GetCompletionsFirstRequest" , "GetCompletionsSecondRequest"],
              # JQuery Tests
              ["GetFunctionHelpFirstRequest", "GetFunctionHelpSecondRequest", "GetRegionsFirstRequest", "GetRegionsSecondRequest", "GetCompletionsFirstRequest" , "GetCompletionsSecondRequest" , "GetASTFirstRequest", "GetASTSecondRequest", "GetMessagesFirstRequest", "GetMessagesSecondRequest"],
              # Box4 Tests
              ["GetFunctionHelpFirstRequest", "GetFunctionHelpSecondRequest", "GetRegionsFirstRequest", "GetRegionsSecondRequest", "GetCompletionsFirstRequest" , "GetCompletionsSecondRequest" , "GetASTFirstRequest", "GetASTSecondRequest", "GetMessagesFirstRequest", "GetMessagesSecondRequest"],
              # Box4 As Context File Tests
              ["GetFunctionHelpFirstRequest", "GetCompletionsFirstRequest"],
              # Solitaire (WWA) Tests
              ["GetFunctionHelpFirstRequest", "GetFunctionHelpSecondRequest","GetCompletionsFirstRequest" , "GetCompletionsSecondRequest"],
              # OpenMailTests As Context File Tests
              ["GetFunctionHelpFirstRequest", "GetCompletionsFirstRequest"]
             );

my $official_name;
my $is_official = 0;
my $is_baseline = 0;
my $use_database = 0;
my $basefile = "perfbase.txt";
my $test_solution = "VSTests.sln";
my $test_binary_location = cwd() . "\\DirectAuthorTestsOutput\\";
my $test_binary_name = "PerformanceTests.dll";
my $test_binary = $test_binary_location . $test_binary_name;
my $buildType = $ENV{'_BuildType'};
my $mstest_exe = "";
my $msbuild_exe = "";
my $args;
my $dsn_name = "";
my $changeSet = "";

my %measured_data;
my %measured;
my %variances;
my %baseline;
my %baseline_variances;

parse_args();

set_msbuild_path();
set_mstest_path();

# make sure the baseline file exists
if($is_baseline == 0 && !-e $basefile)
{
    die "Baseline file $basefile not found. Rerun this script with -baseline.\n";
}

if ($use_database == 1 && ($changeSet eq "" || $dsn_name eq ""))
{
  die "When specifing -database option you need to provide both dsn name and a changeset number.\n";
}

if ($buildType ne "fre")
{
    print "\n";
    print "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
    print "!!        WARNING: Perf tests should be run against fre builds.        !!\n";
    print "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
    print "\n";
}

print "\n";
print "---------------------------------------------------------------------------\n";
print "NOTE: Baseline Run\n" if $is_baseline;
print "NOTE: Official Run\n" if $is_official;
print "NOTE: Database Run\n" if $use_database;
print "Baseline file    :  $basefile\n";
print "Iterations       :  $iter\n";
print "MSBuild location :  $msbuild_exe\n";
print "MSTest location  :  $mstest_exe\n";
print "Test Binary      :  $test_binary\n";
if ($use_database == 1) {
  print "Changeset        :  $changeSet\n";
  print "Database name    :  $dsn_name\n";
}
print "---------------------------------------------------------------------------\n";

if(!$is_baseline)
{
    # otherwise, suck in the baseline file
    open(IN,"$basefile") or die "Couldn't open $basefile for reading.";
    while(<IN>)
    {
        if(/^([\w\-]+)\((\w+)\)\s([-+]?[0-9]*\.?[0-9]+)\s([-+]?[0-9]*\.?[0-9]+)$/)
        {
            $baseline{$1}{$2}=$3;
            $baseline_variances{$1}{$2}=$4
        }
        else
        {
            die "Couldn't match baseline file line: $_";
        }
    }
    close(IN);
}


# build the tests
build_test_binary();

# run the tests
for (my $variantIndex = 0; $variantIndex < scalar(@tests); $variantIndex++)
{
    my $variant = @variants[$variantIndex];
    my @testlist = @{@tests[$variantIndex]}; 

    print("\n");
    if(!$is_baseline)
    {
        printf("TEST - %-10s        BASE(ms)         TEST(ms)     DIFF(ms)  RATIO(%)\n", uc($variant), 0);
        print("---------------------------------------------------------------------------\n");
    }
    else
    {
        printf("Running %s tests...\n", uc($variant));
    }
    my $totalVariance = 0;
    my $totalBaseVariance = 0;
    my $basesum = 0;
    my $measuredsum = 0;
    my $testValue;
    my $base;
    my $diff;
    my $ratio;
    my $base_variance;
    my $test_variance;
    my $investigate;
    foreach my $test(@testlist)
    {
        if($is_baseline)
        {
            print "Running $test...\n";
        }
        for(my $i = 0; $i < $iter; ++$i)
        {
            # store each data point
            $measured_data{$test}{$variant}[$i] = runtest($test, $variant);
        }

        # sort the timings
        @{$measured_data{$test}{$variant}} = sort {$a <=> $b} @{$measured_data{$test}{$variant}};

        # discard the maximum 10%
        my $count = int(@{$measured_data{$test}{$variant}} * 9 / 10);
        if ($count == 0 )
        {
            $count = 1;
        }

        for(my $i = 0; $i < $count; ++$i)
        {
            $measured{$test}{$variant} += $measured_data{$test}{$variant}[$i];
        }

        $measured{$test}{$variant} = $measured{$test}{$variant}/$count;
        my $variance = 0;
        for(my $i = 0; $i < $count; ++$i)
        {
            $variance += ($measured_data{$test}{$variant}[$i] - $measured{$test}{$variant}) * ($measured_data{$test}{$variant}[$i] - $measured{$test}{$variant});
        }
        $variances{$test}{$variant} = (sqrt($variance/$count)/$measured{$test}{$variant}) * 100;
        
        #print the results only if it is not a baseline run.
        if(!$is_baseline)
        {	
            $base = $baseline{$test}{$variant};
            $testValue = $measured{$test}{$variant};
            $base_variance = $baseline_variances{$test}{$variant};
            $test_variance = $variances{$test}{$variant};
            
            $totalBaseVariance += ($base * $base_variance) / 100;
            $totalVariance += ($testValue * $test_variance) / 100;
                    
            $basesum += $base;
            $measuredsum += $testValue;

            # prevent divide by zero.  the lack of precision is acceptable if the tests are taking <1ms
            if($base == 0)
            {
                $base = 1;
            }

            $diff = $testValue - $base;
            $ratio = ($testValue/$base - 1) * 100.0;
            
            $investigate = "";
            if($test_variance < abs($ratio) && $base_variance < abs($ratio))
            {
                if($ratio > 4 )
                {
                    $investigate = "<-CHECK";
                }
                if($ratio < -4)
                {
                    $investigate = "<-IMPROVED";
                }
            }
        
            printf ("%-24.24s %5.1f +-%2.1f%%\t%5.1f +-%2.1f%%\t% 2.1f\t% 2.1f%% %s\n", $test, $base, $base_variance,  $testValue, $test_variance, $diff, $ratio, $investigate);
        }
    }
        
    #print the total only if it is not a baseline run.    
    if(!$is_baseline)
    {
        $base_variance = ($totalBaseVariance / $basesum) * 100;
        $test_variance = ($totalVariance / $measuredsum) *100;
        
        $diff = $measuredsum - $basesum;
        $ratio = ($measuredsum/$basesum - 1) * 100.0;
        
        $investigate = "";
        if($test_variance < abs($ratio) && $base_variance < abs($ratio))
        {
            if($ratio > 4 )
            {
                $investigate = "<-CHECK";
            }
            if($ratio < -4)
            {
                $investigate = "<-IMPROVED";
            }
        }
        
        print("---------------------------------------------------------------------------\n");
        printf ("%-24.24s %5.1f +-%2.1f%%\t%5.1f +-%2.1f%%\t%2.1f\t%2.1f%% %s\n", "TOTAL", $basesum, $base_variance,  $measuredsum, $test_variance, $diff, $ratio, $investigate);
        print("\n");
    }
}

# handle the official run
if($is_official)
{
    official_header();

    # record each test
    for (my $variantIndex = 0; $variantIndex < scalar(@tests); $variantIndex++)
    {
      my $variant = @variants[$variantIndex];
      my @testlist = @{@tests[$variantIndex]}; 
 
      official_start_variant($variant);

      foreach my $test(@testlist)
      {
          official_start_test($test);
          for(my $i = 0; $i < $iter; ++$i)
          {
              official_record($measured_data{$test}{$variant}[$i]);
          }
          official_end_test($test);
      }

      official_end_variant($variant);
    }

    # record the total
    official_start_test("TOTAL");
    for (my $variantIndex = 0; $variantIndex < scalar(@tests); $variantIndex++)
    {
      my $variant = @variants[$variantIndex];
      my @testlist = @{@tests[$variantIndex]}; 
      
        official_start_variant($variant);
        for(my $i = 0; $i < $iter; ++$i)
        {
            my $sum = 0;

            # for each iteration, sum across all tests
            foreach my $test(@testlist)
            {
                $sum += $measured_data{$test}{$variant}[$i];
            }
            official_record($sum);
        }
        official_end_variant($variant);
    }
    official_end_test("TOTAL");

    official_footer();
}

# handle the database run
if($use_database)
{
    my $connection = new Win32::ODBC($dsn_name);
    # Check to make sure the connection is valid
    if (!$connection)
    {
        die "Could not open connection to DSN '$dsn_name' because of [$!]";
    }
    my $CurrentDate = GetCurrentDateTime();

    # Insert each test
    for (my $variantIndex = 0; $variantIndex < scalar(@tests); $variantIndex++)
    {
      my $variant = @variants[$variantIndex];
      my @testlist = @{@tests[$variantIndex]}; 
 
      foreach my $test(@testlist)
      {
          for(my $i = 0; $i < $iter; ++$i)
          {
              my $value = $measured_data{$test}{$variant}[$i];
              my $SQL= "insert into PerformanceTestData (TimeStamp, Changeset, ClassName, TestCaseName, Iteration, Time)" .
                    " values('$CurrentDate', '$changeSet', '$variant', '$test', $i, $value)";
              $connection->Sql($SQL);
          }
      }
    }
    # Close the connection
    $connection->Close();
}

# if it's a baseline run, just output to the baseline file and exit
if($is_baseline)
{
    open(OUT, ">$basefile") or die "Couldn't open $basefile for writing.";

    for (my $variantIndex = 0; $variantIndex < scalar(@tests); $variantIndex++)
    {    
        my $variant = @variants[$variantIndex];
        my @testlist = @{@tests[$variantIndex]}; 
        foreach my $test(@testlist)
        {
            print OUT "$test($variant) $measured{$test}{$variant} $variances{$test}{$variant}\n";
        }
    }

    close(OUT);
    exit(0);
}


# Compare the baseline to the measured tests. 
# NOT CURRENTLY USED
sub printReport()
{
my @listOfTests = @_;
my $testcasename;
my $variant;
my $base;
my $test;
my $diff;
my $ratio;
my $investigate;
my $test_variance;
my $base_variance;
my $variant_substring;

write "\n";

format STDOUT_TOP=

TEST - @<<<<<<<<<<<<       BASE(ms)           TEST(ms)     DIFF(ms)  RATIO(%)     
uc($variant)
---------------------------------------------------------------------------------
.

format STDOUT=
@<<<<<<<<<<<<<<<<<<<...@#####.# +-@#.#% @#####.# +-@#.#% @###.#  @##.#% @<<<<<<<<<<
$testcasename,          $base,  $base_variance, $test,  $test_variance,   $diff, $ratio, $investigate
.

# for the test header
$= = (@listOfTests  + 4);
$^L = "\n\n";

foreach $variant(@variants)
{
    my $basesum = 0;
    my $measuredsum = 0;

    my $totalVariance = 0;
    my $totalBaseVariance = 0;
    foreach $testcasename(@listOfTests)
    {
        if(!exists $baseline{$testcasename}{$variant})
        {
            die "Unable to find test/variant: $testcasename($variant) in baseline file.";
        }

        $base = $baseline{$testcasename}{$variant};
        $test = $measured{$testcasename}{$variant};
        $base_variance = $baseline_variances{$testcasename}{$variant};
        $test_variance = $variances{$testcasename}{$variant};

        $totalBaseVariance += ($base * $base_variance) / 100;
        $totalVariance += ($test * $test_variance) / 100;
        # prevent divide by zero.  the lack of precision is acceptable if the tests are taking <1ms
        if($base == 0)
        {
            $base = 1;
        }

        $diff = $test - $base;
        $ratio = ($test/$base - 1) * 100.0;
        $investigate = "";
        
        # criteria for investigation: ratio > 5% AND diff > 5s
        if($test_variance < abs($ratio) && $base_variance < abs($ratio))
        {
            if($ratio > 5 )
            {
                $investigate = "<--CHECK";
            }
            if($ratio < -5)
            {
                $investigate = "<--IMPROVED";
            }
        }
        
        $basesum += $baseline{$testcasename}{$variant};
        $measuredsum += $measured{$testcasename}{$variant};
        
        
        write STDOUT;

    }
    # Now print the total time.

    $testcasename = "TOTAL";

    $base = $basesum;
    $test = $measuredsum;

    $base_variance = ($totalBaseVariance / $base) * 100;
    $test_variance = ($totalVariance / $test) *100;

    if($test_variance < abs($ratio) && $base_variance < abs($ratio))
    {
        if($ratio > 4 )
        {
            $investigate = "<-CHECK";
        }
        if($ratio < -4)
        {
            $investigate = "<-IMPROVED";
        }
    }
    
    $diff = $test - $base;
    $ratio = ($test/$base - 1) * 100.0;

    $investigate = "";

    write STDOUT;
}
}

sub runtest($$)
{
    my $testcasename = shift;
    my $variant = shift;
    
    my $mstest_details = "/detail:errormessage /detail:errorstacktrace /detail:stderr /detail:stdout";
    my $mstest_runconfigfile = "perf.testsettings";
    my $fullyqualifiedtestcasename = "DirectAuthorTests.$variant.$testcasename";
    my $testOutputFile = "_time_$fullyqualifiedtestcasename.txt";
    
    system("\"$mstest_exe\" /runconfig:\"$mstest_runconfigfile\" /noresults /testcontainer:\"$test_binary\" $mstest_details /test:$fullyqualifiedtestcasename > $testOutputFile");
   
    ### TODO: make sure the test passed
   
    open(IN,$testOutputFile) or die;
    while(<IN>)
    {
        if(/###\sTIME:\s(\d+)\sms/)
        {
            return $1;
        }
    }
    print "ERROR: test produced invalid output.\n";
}

sub badswitch()
{
    die "invalid switch combination";
}

sub usage()
{
    print "Usage: perftest.pl [options]\n";
    print "Options:\n";
    print "  -baseline           Generates a baseline, updating your local baseline file. \n ";
    print "                      NOTE: use only when your enlistment is clean.\n";
    print "  -basefile:<file>    Uses <file> as your perf baseline (default: perfbase.txt)\n";
    print "  -iterations:<iter>  Number of iterations to run tests (default: 11)\n";
    print "  -official:<name>    Generates an official report into results-<name>.xml\n";
    print "  -database:<dsnName> Inserts the the report into a database specified by dsn\n";
    print "  -changeset:<number> The changeset number of the latest change used for this run\n";
}

sub parse_args()
{
    if(@ARGV == 1 && $ARGV[0] =~ /[-\/]\?/)
    {
        usage();
        exit(0);
    }
    
    for(my $i = 0; $i < @ARGV; ++$i)
    {
        if($ARGV[$i] =~ /[-\/]baseline/)
        {
            $is_baseline = 1;
            badswitch() if $is_official;
        }
        elsif($ARGV[$i] =~ /[-\/]basefile:(.*)$/)
        {
            $basefile = $1;
            badswitch() if $is_official;
        }
        elsif($ARGV[$i] =~ /[-\/]iterations:(\d+)$/)
        {
            $iter = $1;
        }
        elsif($ARGV[$i] =~ /[-\/]official:(.*)$/)
        {
            $is_official = 1;
            $official_name = $1;
        }
        elsif($ARGV[$i] =~ /[-\/]database:(.*)$/)
        {
            $use_database = 1;
            $dsn_name = $1;
        }
        elsif($ARGV[$i] =~ /[-\/]changeset:(.*)$/)
        {
            $changeSet = $1;
        }
    }
}

sub set_msbuild_path()
{
    my $processor_architecture = $ENV{'PROCESSOR_ARCHITECTURE'};
    my $msbuild_directory= "";
    my $msbuild_version ="";
    
    if ($processor_architecture eq "AMD64") 
    {
        $msbuild_directory=$ENV{'windir'} . "\\Microsoft.NET\\Framework64";
    }
    elsif ($processor_architecture eq "IA64") 
    {
        $msbuild_directory=$ENV{'windir'} . "\\Microsoft.NET\\Framework64";
    }
    elsif ($processor_architecture eq "x86")
    {
        $msbuild_directory=$ENV{'windir'} . "\\Microsoft.NET\\Framework";
    }

    opendir (DIR, $msbuild_directory) or die "Can’t find msbuild.exe at: $msbuild_directory";
    while (my $subdir = readdir(DIR)) 
    {
        next unless(-d "$msbuild_directory\\$subdir" && $subdir=~/v4\.0.*/);
        $msbuild_version = $subdir;
    }
    closedir(DIR);

    $msbuild_exe = "$msbuild_directory\\$msbuild_version\\msbuild.exe";
    
    unless (-e $msbuild_exe) {
        die "Can’t find msbuild.exe at:" . $msbuild_exe;
    } 
}

sub set_mstest_path()
{
    my $processor_architecture = $ENV{'PROCESSOR_ARCHITECTURE'};
    my $mstest_pfiles= "";
    
    if ($processor_architecture eq "AMD64") 
    {
        $mstest_pfiles=$ENV{'ProgramFiles(x86)'};
    }
    elsif ($processor_architecture eq "IA64") 
    {
        $mstest_pfiles=$ENV{'ProgramFiles(x86)'};
    }
    elsif ($processor_architecture eq "x86")
    {
        $mstest_pfiles=$ENV{'ProgramFiles'};
    }
    
    $mstest_exe = $mstest_pfiles ."\\Microsoft Visual Studio 10.0\\Common7\\IDE\\mstest.exe";
    
    unless (-e $mstest_exe) {
        die "Can’t find mstest.exe at:" . $mstest_exe;
    } 
}

sub build_test_binary()
{
    printf("Building test binary ...\n");
   
    if (-e $test_binary_location)
    { 
        ## delete the directory to make sure we get a fresh copy of JScript9.dll
        use File::Path;
        rmtree ( $test_binary_location ) or die ("Could not remove test binary out directory at: $test_binary_location \n");
    } 
    mkdir($test_binary_location) or die "Can't create test binary out directory at: $test_binary_location \n";

    unless (-e $test_solution)
    {
        die "Can’t find test solution at: $test_solution";
    }
 
    my $configuration = "Release";
    if ($buildType eq "chk")
    {
        $configuration = "Debug";
    } 
    
    system("\"$msbuild_exe\" \"$test_solution\" /p:OutputPath=$test_binary_location /t:rebuild /p:configuration=$configuration > _buildlog.txt");
   
    unless (-e $test_binary) 
    {
        die "Can’t find test binary at:" . $test_binary;
    } 
    
    printf("Build complete.\n");
}

sub official_footer()
{
    print OFFICIAL "</data>\n";
    close(OFFICIAL)
}

sub official_header()
{
    open(OFFICIAL,">results-$official_name.xml") or die;
    print OFFICIAL "<data name=\"JScript Language Service backend Perf Tests\">\n";
}
sub official_start_variant($)
{
    my $variant = shift;
    print OFFICIAL "<variant name=\"$variant\">\n";
}
sub official_end_variant($)
{
    print OFFICIAL "</variant>\n";
}
sub official_start_test($)
{
    my $testname = shift;
    print OFFICIAL "\t<test name=\"$testname\">\n";
}
sub official_record($)
{
    my $time = shift;
    print OFFICIAL "\t\t<iteration time_ms=\"$time\"/>\n";
}
sub official_end_test()
{
    print OFFICIAL "\t</test>\n";
}
sub GetCurrentDateTime ()
{
  my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=localtime(time);
  my $timeString = ($year + 1900) . "-" . ($mon + 1) . "-" . $mday . " ". $hour . ":" . $min . ":" . $sec;

  return $timeString;
}