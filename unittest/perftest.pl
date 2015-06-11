#
# Script used for perf testing against SunSpider.  This script supports "official" mode, which dumps an XML file
# filled with all the data for analysis or sucking into Excel.  The XML file schema was chosen so we could steal
# some already written perf analysis tools from the C++ team.
#


#use strict;
use File::Basename;

my $defaultIter = 35;
my $iter = $defaultIter;

my @testlist = ("3d-cube", "3d-morph", "3d-raytrace", "access-binary-trees", "access-fannkuch",
        "access-nbody", "access-nsieve", "bitops-3bit-bits-in-byte", "bitops-bits-in-byte",
        "bitops-bitwise-and", "bitops-nsieve-bits", "controlflow-recursive", "crypto-aes", "crypto-md5",
        "crypto-sha1", "date-format-tofte", "date-format-xparb", "math-cordic", "math-partial-sums",
        "math-spectral-norm", "regexp-dna", "string-base64", "string-fasta", "string-tagcloud",
                "string-unpack-code", "string-validate-input");

my @testlistSwitches = ();

my @variants = ("native", "interpreted");

my %switches = ( "interpreted" => "-NoNative",
         "native" => "",
         "debugmode" => "-ForceDiagnosticsMode");

my $other_switches = "";
my $official_name;
my $is_official = 0;
my $is_baseline = 0;
my $buildArch = $ENV{_BuildArch};
my $buildType = $ENV{_BuildType};
my $basefile = "perfbase$buildArch$buildType.txt";
my $binary = "JC.exe";
my $dir = "";
my $highprecisiondate = 1;
my $args;
my $defaultVersion = 6;
my $version = $defaultVersion;
my $parse_scores = 0;
my $parse_time = 1;
my $parse_latency = 0;
my $is_dynamicProfileRun = 1;
my $is_serializedRun = 0;
my $profileFile="dynamicProfileInfo.dpl";
my $perlScriptDir = dirname(__FILE__);
my $testDescription = "SunSpider";

my %measured_data;
my %measured;
my %variances;
my %baseline;
my %baseline_variances;

parse_args();

if($dir)
{
    $testDescription = $dir;
}


# make sure the baseline file exists
if($is_baseline == 0 && !-e $basefile)
{
    print "Baseline file $basefile not found.  Put a clean JC.exe on your path\n";
    print "and rerun this script with -baseline.\n";
    die();
}

check_switch();
$other_switches .= " -dynamicProfileCache:$profileFile" if $is_dynamicProfileRun;
$other_switches .= " -recreatenativecodefile -nativecodeserialized" if $is_serializedRun;

print "NOTE: Baseline Run\n" if $is_baseline;
print "NOTE: Official Run\n" if $is_official;
print "Baseline file: $basefile\n";
print "JNConsole binary: $binary\n";
print "Iterations: $iter\n";
print "High Precision Date not used!\n" if !$highprecisiondate;
print "Switches: $other_switches\n" unless ($other_switches eq "");

$other_switches .= " -highprecisiondate" if $highprecisiondate;

if(!$is_baseline)
{
    # otherwise, suck in the baseline file
    open(IN,"$basefile") or die "Couldn't open $basefile for reading.";
    while(<IN>)
    {
        if(/^([\w\-\\\.]+)\((\w+)\)\s([-+]?[0-9]*\.?[0-9]+)\s([-+]?[0-9]*\.?[0-9]+)\s([-+]?[0-9]*\.?[0-9]+)\s([-+]?[0-9]*\.?[0-9]+)$/)
        {
            $baseline{$1}{$2}{"time"}=$3;
            $baseline{$1}{$2}{"score"}=$5;
            $baseline_variances{$1}{$2}{"time"}=$4;
            $baseline_variances{$1}{$2}{"score"}=$6;
        }
        elsif(/^([\w\-\\\.]+)\((\w+)\)\s([-+]?[0-9]*\.?[0-9]+)\s([-+]?[0-9]*\.?[0-9]+)\s*$/)
        {
            $baseline{$1}{$2}{"time"}=$3;
            $baseline{$1}{$2}{"score"}=0;
            $baseline_variances{$1}{$2}{"time"}=$4;
            $baseline_variances{$1}{$2}{"score"}=0;
        }
        elsif(/^([\w\-\\\.]+)\((\w+)\)\s\s\s([-+]?[0-9]*\.?[0-9]+)\s([-+]?[0-9]*\.?[0-9]+)\s*$/)
        {
            $baseline{$1}{$2}{"time"}=0;
            $baseline{$1}{$2}{"score"}=$3;
            $baseline_variances{$1}{$2}{"time"}=0;
            $baseline_variances{$1}{$2}{"score"}=$4;
        }
        else
        {
            die "Couldn't match baseline file line: $_";
        }
    }
    close(IN);
}
else
{
    delete_baseline();
}

# run the SunSpider tests
print "Running $testDescription...\n";


foreach my $variant(@variants)
{
    print("\n");
    if(!$is_baseline)
    {
        printf("TEST - %-10s            BASE            TEST         DIFF   RATIO(%)\n", uc($variant), 0);
        print("---------------------------------------------------------------------------\n");
    }
    else
    {
        printf("Running %s tests...\n", uc($variant));
    }
    my %totalVariance = {"time", 0, "score", 0};
    my %totalBaseVariance = {"time", 0, "score", 0};
    my %basesum = {"time", 0, "score", 0};
    my %measuredsum = {"time", 0, "score", 0};

    foreach my $test(@testlist)
    {
        my $testLatency = $test."Latency";
		
        if($is_baseline)
        {
            print "Running $test...\n";
        }
        delete_profile();
        for(my $i = 0; $i < $iter; ++$i)
        {
            # store each data point
            my %result = runtest($test, $variant);
            if ($parse_time == 1)
            {
                $measured_data{$test}{$variant}{"time"}[$i] = $result{"time"};
            }
            if ($parse_scores == 1)
            {
                $measured_data{$test}{$variant}{"score"}[$i] = $result{"score"};
            }
            if ($parse_latency == 1)
            {
                # Store Latency score as a separate test - This is used as one of the compnonents in calculating the final GM score.
                $measured_data{$testLatency}{$variant}{"score"}[$i] = $result{"latency"};				
            }
        }
        
        if ($parse_time == 1)
        {
            # process the time result
            my $testName = $test;
            if ($parse_scores == 1)
            {
                $testName = $testName." (time)";
            }
            %result = processResult($testName, $baseline{$test}{$variant}{"time"}, $baseline_variances{$test}{$variant}{"time"}, $measured_data{$test}{$variant}{"time"}, 0);

            $measured{$test}{$variant}{"time"} = $result{"measured"};
            $variances{$test}{$variant}{"time"} = $result{"variance"};
            $totalBaseVariance{"time"} += ($baseline{$test}{$variant}{"time"} * $baseline_variances{$test}{$variant}{"time"}) / 100;
            $basesum{"time"} += $baseline{$test}{$variant}{"time"};
            $totalVariance{"time"} += ($measured{$test}{$variant}{"time"} * $variances{$test}{$variant}{"time"}) / 100;
            $measuredsum{"time"} += $measured{$test}{$variant}{"time"};
        }
        if ($parse_scores == 1)
        {
            # process the score result
            my $testName = $test;
            if ($parse_time == 1)
            {
                $testName = $testName." (score)";
            }
            %result = processResult($testName, $baseline{$test}{$variant}{"score"}, $baseline_variances{$test}{$variant}{"score"}, $measured_data{$test}{$variant}{"score"}, 1);

            $measured{$test}{$variant}{"score"} = $result{"measured"};
            $variances{$test}{$variant}{"score"} = $result{"variance"};
        }
		
        if ($parse_latency == 1)
        {
            # Check if this test has a Latency score available (Only Mandreel and Splay has latency scores as of 11/08/2013)
            if($measured_data{$testLatency}{$variant}{"score"}[0] != 0)
            {
                my $testName = $testLatency;
                %result = processResult($testName, $baseline{$testLatency}{$variant}{"score"}, $baseline_variances{$testLatency}{$variant}{"score"}, $measured_data{$testLatency}{$variant}{"score"}, 1);

                $measured{$testLatency}{$variant}{"score"} = $result{"measured"};
                $variances{$testLatency}{$variant}{"score"} = $result{"variance"};
            }
        }

    }

    # generate the geometric means
    if ($parse_scores == 1)
    {
        my $measuredlog = 0;
        my $variancelog = 0;
        my $baselog = 0;
        my $basevariancelog = 0;
        foreach my $test(@testlist)
        {
            my $testLatency = $test.Latency;
            if (!$is_baseline)
            {	
                $baselog += log($baseline{$test}{$variant}{"score"});
                $basevariancelog += log(($baseline{$test}{$variant}{"score"} * $baseline_variances{$test}{$variant}{"score"}) / 100);
                if($parse_latency == 1)
                {		
                    if(exists $baseline{$testLatency})
                    {
                        $baselog += log($baseline{$testLatency}{$variant}{"score"});
                        $basevariancelog += log(($baseline{$testLatency}{$variant}{"score"} * $baseline_variances{$testLatency}{$variant}{"score"}) / 100);
                    }
                }
            }
			
            $measuredlog += log($measured{$test}{$variant}{"score"});			
            $variancelog += log(($measured{$test}{$variant}{"score"} * $variances{$test}{$variant}{"score"}) / 100);
			
            if($parse_latency == 1)
            {				
                if(exists $measured{$testLatency})
                {
                    $measuredlog += log($measured{$testLatency}{$variant}{"score"});
                    $variancelog += log(($measured{$testLatency}{$variant}{"score"} * $variances{$testLatency}{$variant}{"score"}) / 100);
                }
            }
        }

        my $count = keys(%measured);
        $measuredsum{"score"} = exp($measuredlog / $count);
        $totalVariance{"score"} = exp($variancelog / $count);
        $basesum{"score"} = exp($baselog / $count);
        $totalBaseVariance{"score"} = exp($basevariancelog / $count);
    }
    
    print("---------------------------------------------------------------------------\n");

    #print the total only if it is not a baseline run.
    if(!$is_baseline)
    {
        if ($parse_time == 1)
        {
            $base_variance = ($totalBaseVariance{"time"} / $basesum{"time"}) * 100;
            $test_variance = ($totalVariance{"time"} / $measuredsum{"time"}) * 100;

            $diff = $measuredsum{"time"} - $basesum{"time"};
            $ratio = ($measuredsum{"time"} / $basesum{"time"} - 1) * 100.0;

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
            
            printf("%-24.24s %6.1f +-%2.1f%%\t%6.1f +-%2.1f%%\t%6.1f\t%5.1f%% %s\n", "TOTAL TIME", $basesum{"time"}, $base_variance, $measuredsum{"time"}, $test_variance, $diff, $ratio, $investigate);
        }

        if ($parse_scores == 1)
        {
            $base_variance = ($totalBaseVariance{"score"} / $basesum{"score"}) * 100;
            $test_variance = ($totalVariance{"score"} / $measuredsum{"score"}) * 100;

            $diff = $measuredsum{"score"} - $basesum{"score"};
            $ratio = ($measuredsum{"score"} / $basesum{"score"} - 1) * 100.0;

            $investigate = "";
            if($test_variance < abs($ratio) && $base_variance < abs($ratio))
            {
                if($ratio > 4 )
                {
                    $investigate = "<-IMPROVED";
                }
                if($ratio < -4)
                {
                    $investigate = "<-CHECK";
                }
            }

            printf("%-24.24s %6.1f +-%2.1f%%\t%6.1f +-%2.1f%%\t%6.1f\t%5.1f%% %s\n", "MEAN SCORE", $basesum{"score"}, $base_variance, $measuredsum{"score"}, $test_variance, $diff, $ratio, $investigate);
        }
        print("\n");
    }
}

# handle the official run
if($is_official)
{
    official_header();

    # record each test
    foreach my $test(@testlist)
    {
        official_start_test($test);
        foreach my $variant(@variants)
        {
            official_start_variant($variant);

            for(my $i = 0; $i < $iter; ++$i)
            {
                official_record($measured_data{$test}{$variant}{"time"}[$i]);
            }

            official_end_variant($variant);
        }
        official_end_test($test);
    }

    # record the total
    official_start_test("TOTAL");
    foreach my $variant(@variants)
    {
        official_start_variant($variant);
        for(my $i = 0; $i < $iter; ++$i)
        {
            my $sum = 0;

            # for each iteration, sum across all tests
            foreach my $test(@testlist)
            {
                $sum += $measured_data{$test}{$variant}{"time"}[$i];
            }
            official_record($sum);
        }
        official_end_variant($variant);
    }
    official_end_test("TOTAL");

    official_footer();

    exit(0);
}


# if it's a baseline run, just output to the baseline file and exit
if($is_baseline)
{
    open(OUT, ">$basefile") or die "Couldn't open $basefile for writing.";

    foreach my $test(@testlist)
    {
        foreach my $variant(@variants)
        {
            print OUT "$test($variant) $measured{$test}{$variant}{'time'} $variances{$test}{$variant}{'time'} $measured{$test}{$variant}{'score'} $variances{$test}{$variant}{'score'}\n";
			
            my $testLatency = $test."Latency";
			
            if(exists $measured{$testLatency})
            {
                #Note: There are extra spaces in this output - which is deliberately added to match the regex (to read from the baseline file)
                print OUT "$testLatency($variant)   $measured{$testLatency}{$variant}{'score'} $variances{$testLatency}{$variant}{'score'}\n";
            }
        }
    }

    close(OUT);
    exit(0);
}
sub processResult()
{
    my $test = shift;
    my $baseline = shift;
    my $baseline_variance = shift;
    my $data = shift;
    my $bigger_is_better = shift;
    my $value = 0;
    my $variance = 0;

    my %result = {"measured", 0, "variance", 0};

    # sort the timings
    if ($bigger_is_better == 1)
    {
        @{$data} = sort {$b <=> $a} @{$data};
    }
    else
    {
        @{$data} = sort {$a <=> $b} @{$data};
    }

    # discard the minimum/maximum 10%
    my $count = @{$data};
    if($count >= 10)
    {
        $count = int($count * 9 / 10);
    }

    for(my $i = 0; $i < $count; ++$i)
    {
        $value += @{$data}[$i];
    }
    my $value = $value / $count;
    if ($value > 0)
    {
        for(my $i = 0; $i < $count; ++$i)
        {
            $variance += (@{$data}[$i] - $value) * (@{$data}[$i] - $value);
        }
        $variance = (sqrt($variance / $count) / $value) * 100;
        if ($variance == 0)
        {
            $variance = 0.0001; # to avoid divide by zero and ln(0) errors
        }
    }
    else
    {
        $variance = 0.0001; # to avoid divide by zero and ln(0) errors
    }

    #print the results only if it is not a baseline run.
    if(!$is_baseline)
    {
        my $diff = $value - $baseline;
        my $ratio = 0;
        if ($baseline > 0)
        {
            $ratio = ($diff / $baseline) * 100.0;
        }
        else
        {
            # prevent divide by zero.  the lack of precision is acceptable if the tests are taking <1ms
            $ratio = ($diff / 1) * 100.0;
        }

        $investigate = "";
        if($variance < abs($ratio) && $base_variance < abs($ratio))
        {
            if($ratio > 4 )
            {
                if ($bigger_is_better == 1)
                {
                    $investigate = "<-IMPROVED";
                }
                else
                {
                    $investigate = "<-CHECK";
                }
            }
            if($ratio < -4)
            {
                if ($bigger_is_better == 1)
                {
                    $investigate = "<-CHECK";
                }
                else
                {
                    $investigate = "<-IMPROVED";
                }
            }
        }

        printf ("%-24.24s %6.1f +-%2.1f%%\t%6.1f +-%2.1f%%\t%6.1f\t%5.1f%% %s\n", $test, $baseline, $baseline_variance, $value, $variance, $diff, $ratio, $investigate);
    }

    $result{"measured"} = $value;
    $result{"variance"} = $variance;

    return %result;
}

sub runtest($$)
{
    my $testcasename = shift;
    my $variant = shift;

    my %results = ("time", -1, "score", -1);

    $other_switches .= " " . @testlistSwitches{$testcasename};

    if(!$dir)
    {
        system("$binary -Version:$version $switches{$variant} $other_switches $testcasename.js > _time.txt");
    }
    else 
    {
        system("$binary -Version:$version $switches{$variant} $other_switches $dir\\$testcasename.js > _time.txt");
    }

    open(IN,"_time.txt") or die;

    my $i = 0;
    while(<IN>)
    {
        if(/###\sTIME:\s(\d+(\.\d+)*)\sms/)
        {
            $results{"time"} = $1;
        }

        if(/###\sSCORE:\s(\d+)/)
        {
            $results{"score"} = $1;
        }
		
        if(/###\sLATENCY:\s(\d+)/)
        {
            $results{"latency"} = $1;
        }
    }

    if ($testDescription == "custom")
    {
        if ($results{"time"} != -1 || $results{"score"} != -1)  
        {
            return %results;
        }
    }
    if(($parse_time == 1 && $results{"time"} == -1) || ($parse_scores == 1 && $results{"score"} == -1))
    {
        print "ERROR: test produced invalid output while running $testcasename.js\n";
        exit(1);
    }

    return %results;
}

sub badswitch()
{
    die "invalid switch combination";
}

sub usage()
{
    print "Usage: perftest.pl [options]\n";
    print "Options:\n";
    print "  -baseline           Generates a baseline, updating your local baseline file.  ";
    print "                      NOTE: use only when your enlistment is clean.\n";
    print "  -basefile:<file>    Uses <file> as your perf baseline (default: perfbase.txt)\n";
    print "  -dir:<dirpath>     Uses the test files in the <dirpath>.\n";
    print "  -binary:<filepath>  Uses <filepath> to run the JS files.\n";
    print "                      (default: JC.exe found on the path.)\n";
    print "  -iterations:<iter>  Number of iterations to run tests (default: 11)\n";
    print "  -official:<name>    Generates an official report into results-<name>.xml\n";
    print "  -native             Only run -native variation\n";
    print "  -interpreted        Only run -interpreted variation\n";
    print "  -debugmode          Run the diagnostics mode variation\n";
    print "  -nodynamicProfile   Run without dynamic profile info\n";
    print "  -dynamicProfile     Force Run with dynamic profile info\n";
    print "  -serialized         Run with native code serialization\n";
    print "  -version:<number>   Version in which to run the jscript engine. [one of 1,2,3]\n";
    print "  -v8                 Run the v8 benchmark\n";
    print "  -v8strict           Run the v8 benchmark converted to strict mode\n";
    print "  -dromaeo            Run the dromaeo benchmark\n";
    print "  -kraken             Run the kraken benchmark\n";
    print "  -yperf              Run the YPerf microbenchmarks\n";
    print "  -typescript         Run the TypeScript compiler perf test\n";
    print "  -octane             Run the Octane 2.0 benchmark\n";
    print "  -jetstream          Run the JetStream benchmark (only non octane and sunspider tests)\n";
    print "  -mb<:priority>      Run the Chakra microbenchmarks. (default:3) [one of 0,1,2,3]\n";
    print "                      run all test <= given priority. :priority is optional. \n";
    print "  -file:<file>        Run the specified js file\n";
    print "  -args:<other args>  Other arguments to jc.exe\n";
    print "  -score              Test output scores\n";
}
sub parse_args()
{
    my $forceDynamicProfileOn;
    my $forceDynamicProfileOff;
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
        elsif($ARGV[$i] =~ /[-\/]binary:(.*)$/)
        {
            $binary = $1;
        }
        elsif($ARGV[$i] =~ /[-\/]dir:(.*)$/)
        {
            if($1)
            {
                $dir = $1;
                print "dir = $1\n";
            }
            else
            {
                $dir= ".";
                print "dir = $1\n";
            }
        
            #opens the directory and read the files in it
            opendir(dirHandle,$dir) || die("Cannot open directory");
            @testlist =  grep (/.js/, readdir(dirHandle));
            closedir(dirHandle);

            for (@testlist) 
            {
                s/(.*).js/$1/;
            }
        
            $is_dynamicProfileRun = 0;
        }
        elsif($ARGV[$i] =~ /[-\/]iterations:(\d+)$/)
        {
            $iter = $1;

        }
        elsif($ARGV[$i] =~ /[-\/]version:(\d+)$/)
        {
            $version = $1;

        }
        elsif($ARGV[$i] =~ /[-\/]official:(.*)$/)
        {
            $is_official = 1;
            $official_name = $1;
            badswitch() if $is_baseline;
        }
        elsif($ARGV[$i] =~ /[-\/]native/)
        {
            @variants = ("native");
        }
        elsif($ARGV[$i] =~ /[-\/]interpreted/)
        {
            @variants = ("interpreted");
        }
        elsif($ARGV[$i] =~ /[-\/]debugmode/)
        {
            @variants = ("debugmode");
        }
        elsif($ARGV[$i] =~ /[-\/]createserializednative/)
        {
            $other_switches .= " -RecreateNativeCodeFile -NativeCodeSerialized";
        }
        elsif($ARGV[$i] =~ /[-\/]serializednative/)
        {
            $other_switches .= " -NoNative -NativeCodeSerialized";
        }
        elsif($ARGV[$i] =~ /[-\/]v8strict/)
        {
            if($iter == $defaultIter)
            {
                $iter = 10;
            }
            
            @testlist = ("crypto", "deltablue", "earley-boyer", "raytrace", "regexp", "richards", "splay");
            $testDescription = "v8 benchmark converted to use strict mode";
            $parse_scores = 1;
            $is_dynamicProfileRun = 0; # Currently v8 dyna-pogo info is not avialable in the browser - remove this when it is.
        }
        elsif($ARGV[$i] =~ /[-\/]v8/)
        {
            if($iter == $defaultIter)
            {
                $iter = 10;
            }
            @testlist = ("crypto", "deltablue", "earley-boyer", "navier-stokes", "raytrace", "regexp", "richards", "splay");
            $testDescription = "v8 benchmark";
            $parse_scores = 1;
            $is_dynamicProfileRun = 0; # Currently v8 dyna-pogo info is not avialable in the browser - remove this when it is.
        }
        elsif($ARGV[$i] =~ /[-\/]octane/)
        {
            if($iter == $defaultIter)
            {
                $iter = 10;
            }
            @testlist = ("box2d", "code-load", "crypto", "deltablue", "earley-boyer", "gbemu", "mandreel", "navier-stokes", "pdfjs", "raytrace", "regexp", "richards", "splay", "typescript", "zlib");
            $testDescription = "octane 2.0 benchmark";
            $parse_time = 0;
            $parse_scores = 1;
            $parse_latency = 1;
            $is_dynamicProfileRun = 0; # Currently octane dyna-pogo info is not avialable in the browser - remove this when it is.
        }
        elsif($ARGV[$i] =~ /[-\/]jetstream/)
        {
            if($iter == $defaultIter)
            {
                $iter = 5;
            }
            @testlist = ("bigfib.cpp", "container.cpp", "dry.c", "float-mm.c", "gcc-loops.cpp", "hash-map", "n-body.c", "quicksort.c", "towers.c");
            $testDescription = "JetStream benchmark";
            $parse_time = 0;
            $parse_scores = 1;
            $parse_latency = 0;			
            $is_dynamicProfileRun = 0; # Currently  dyna-pogo info is not avialable in the browser - remove this when it is.
        }
        elsif($ARGV[$i] =~ /[-\/]dromaeo/i)
        {
            if($iter == $defaultIter)
            {
                $iter = 10;
            }
            @testlist = ("3d-cube", "array", "base64", "eval", "regexp", "string");
            $testDescription = "dromaeo benchmark";
            $is_dynamicProfileRun = 0; # Currently dromaeo dyna-pogo info is not avialable in the browser - remove this when it is.
        }
        elsif($ARGV[$i] =~ /[-\/]kraken/i)
        {
            if($iter == $defaultIter)
            {
                $iter = 10;
            }
            @testlist = ("ai-astar", "audio-beat-detection", "audio-dft", "audio-fft", "audio-oscillator", "imaging-darkroom",
             "imaging-desaturate", "imaging-gaussian-blur", "json-parse-financial", "json-stringify-tinderbox",
             "stanford-crypto-aes", "stanford-crypto-ccm", "stanford-crypto-pbkdf2", "stanford-crypto-sha256-iterative");
            $testDescription = "kraken benchmark";
            $highprecisiondate = 0;
        }
        elsif($ARGV[$i] =~ /[-\/]yperf/i)
        {
            if($iter == $defaultIter)
            {
                $iter = 10;
            }
            @testlist = ("5_ZimbraBenchmarkTests_String_Replace", "5_ZimbraBenchmarkTests_String_SplitRegExDelim",
                         "8_Jscript_Regex_search_exec_flag_i", "8_Jscript_Regex_search_exec_repetition",
                         "8_Jscript_Regex_search_test_flag_i", "8_Jscript_Regex_search_test_repetition",
                         "8_Jscript_String_match_Method", "8_Jscript_String_search_Method", "8_ZimbraBenchmarkTests_String_Replace",
                         "8_ZimbraBenchmarkTests_String_SplitRegExDelim", "9_JScript_Regex_search_exec_flag_i",
                         "9_JScript_Regex_search_test_flag_g", "9_JScript_Regex_search_test_flag_i",
                         "9_JScript_String_match_Method", "9_ZimbraBenchmarkTests_String_Replace", "disjunction_regexp_1",
                         "term_regexp_3", "term_regexp_9");
            $testDescription = "YPerf microbenchmarks";
            $is_dynamicProfileRun = 0;
        }
        elsif($ARGV[$i] =~ /[-\/]typescript/i)
        {
            if ($iter == $defaultIter)
            {
                $iter = 10;
            }
            @testlist = ("TypeScriptCompiler", "TypeScriptEdgeCompiler");
            $testDescription = "TypeScript compiler perf";
            $is_dynamicProfileRun = 0;
        }
        elsif($ARGV[$i] =~ /[-\/]strada/i)
        {
            if ($iter == $defaultIter)
            {
                $iter = 10;
            }
            @testlist = ("stradaPerfCompiler");
            $testDescription = "Strada compiler perf";
            $is_dynamicProfileRun = 0;
        }
        elsif($ARGV[$i] =~ /[-\/]wwaperf/i)
        {
            if ($iter == $defaultIter)
            {
                $iter = 10;
            }
            @testlist = ("promises");
            $testDescription = "WWA Perf Tests";
            $is_dynamicProfileRun = 0;
        }
        elsif($ARGV[$i] =~ /[-\/]file:(.*).js$/i)
        {
            my %scoreTests = ( "box2d" => 1, "code-load" => 1, "crypto" => 1, "deltablue" => 1, 
                               "earley-boyer" => 1, "gbemu" => 1, "mandreel" => 1, "navier-stokes" => 1, 
                               "pdfjs" => 1, "raytrace" => 1, "regexp" => 1, "richards" => 1, "splay" => 1,
                                "zlib" => 1, "typescript" => 1);
            @testlist = ($1);

            my $test = lc $1;
            if (exists $scoreTests{$test}) {
                $parse_scores = 1;
                $parse_time = 0;
                $is_dynamicProfileRun = 0; # Currently octane dyna-pogo info is not avialable in the browser - remove this when it is.
                if($iter == $defaulIter)
                {
                    $iter = 10;
                }
            }
            $testDescription = "custom";
        }
        elsif($ARGV[$i] =~ /[-\/]score/i)
        {
            $parse_scores = 1;
            $parse_time = 0;
        }
        elsif($ARGV[$i] =~ /[-\/]Off:(.*)$/i)
        {
            $other_switches .= " -Off:" . $1;
        }
        elsif($ARGV[$i] =~ /[-\/]On:(.*)$/i)
        {
            $other_switches .= " -On:" . $1;
        }
        elsif($ARGV[$i] =~ /[-\/]args:(.*)$/i)
        {
            $other_switches .= " " . $1;
        }
        elsif($ARGV[$i] =~ /[-\/]noDynamicProfile/i)
        {
            $forceDynamicProfileOff = 1;
        }
        elsif($ARGV[$i] =~ /[-\/]dynamicProfile/i)
        {
            $forceDynamicProfileOn = 1;
        }
        elsif($ARGV[$i] =~ /[-\/]serialized/i)
        {
            $is_serializedRun = 1;
        }
        elsif($ARGV[$i] =~ /[-\/]mb(:(.*))*$/)
        {
            if($iter == $defaultIter)
            {
                $iter = 10;
            }
            system("cscript /nologo $perlScriptDir\\MicroBenchmarks\\MBHelper.js /binary:$binary /priority:$2 > _MBList.txt");
            open(IN,"_MBList.txt") or die "Couldn't open _MBList.txt for reading";
            @testlist = ();
            @testlistSwitches = ();
            while(<IN>)
            {
                chomp;
                @mbTestAndSwitches = split(/,/);
                push(@testlist, @mbTestAndSwitches[0]);
                @testlistSwitches{@mbTestAndSwitches[0]} = @mbTestAndSwitches[1];
            }
            $testDescription = "Chakra microbenchmarks";
        }
        elsif($ARGV[$i] =~ /[-\/]speculationcap:(.*)$/i)
        {
            $other_switches .= " -speculationcap:" . $1;
        }
    }
    if($forceDynamicProfileOff)
    {
        $is_dynamicProfileRun = 0;
    }
    if($forceDynamicProfileOn)
    {
        $is_dynamicProfileRun = 1;
    }
}

sub official_footer()
{
    print OFFICIAL "</data>\n";
    close(OFFICIAL)
}

sub official_header()
{
    open(OFFICIAL,">results-$official_name.xml") or die;
    print OFFICIAL "<data name=\"Eze Perf Tests\">\n";
}
sub official_start_variant($)
{
    my $variant = shift;
    print OFFICIAL "\t<variant name=\"$variant\">\n";
}
sub official_end_variant($)
{
    print OFFICIAL "\t</variant>\n";
}
sub official_start_test($)
{
    my $testname = shift;
    print OFFICIAL "<test name=\"$testname\">\n";
}
sub official_record($)
{
    my $time = shift;
    print OFFICIAL "\t\t<iteration time_ms=\"$time\"/>\n";
}
sub official_end_test()
{
    print OFFICIAL "</test>\n";
}

sub check_switch()
{
    system("$binary /? > _time.txt");
    open(IN,"_time.txt") or die;
    my $dynamicProfileSupported = 0;
    my $highprecisiondateSupported = 0;
    while(<IN>)
    {
        if(/\sHighPrecisionDate/)
        {
            $highprecisiondateSupported = 1;
        }
        if(/\sDynamicProfileCache/)
        {
            $dynamicProfileSupported = 1;
        }
    }
    if($is_dynamicProfileRun && !$dynamicProfileSupported)
    {
        print "Warning: Persistent Dynamic profile is not supported! \n";
        $is_dynamicProfileRun = 0;
    }
    if(!$highprecisiondateSupported)
    {
        $highprecisiondate = 0;
    }
}
sub delete_profile()
{
    if($is_dynamicProfileRun)
    {
        if(-e $profileFile)
        {
            unlink($profileFile);
            if(-e $profileFile)
            {
                 print "File could not be deleted: $profileFile\n";
                 die();
            }
        }
    }
}
sub delete_baseline()
{
    if($is_baseline)
    {
        if(-e $basefile)
        {
            unlink($basefile);
            if(-e $basefile)
            {
                 print "File could not be deleted: $basefile\n";
                 die();
            }
        }
    }
}
