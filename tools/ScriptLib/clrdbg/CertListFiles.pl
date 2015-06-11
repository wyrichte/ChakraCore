# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
#                                                                  #
# This script will create the CERT list files based on the         #
#  SMARTY results.                                                 #
#                                                                  #
# Input: $ARGV[0] - Smarty Results Directory                       #
#        $ARGV[1] - Location where CERT_ALL_INCLUDES, etc. will be #
#                   placed.                                        #
#                                                                  #
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #

my $g_SmartyResultsDir = $ARGV[0];
my $g_InputLstFileDir = $ARGV[1];
my $g_OutputLstFileDir = $ARGV[2];

$InvalidInput = 0;
if (($g_SmartyResultsDir eq "") || (!-d $g_SmartyResultsDir)) {
	print "The smarty results directory does not exist ($g_SmartyResultsDir)\n";
	$InvalidInput = 1;
}
if (($g_InputLstFileDir eq "") || (!-d $g_InputLstFileDir)) {
	print "The input directory is empty or not an absolute path ($g_InputLstFileDir)\n";
	$InvalidInput = 1;
}
if ($g_OutputLstFileDir eq "" || (! -d $g_OutputLstFileDir)) {
	print "The output directory is empty or not an absolute path ($g_OutputLstFileDir).\n";
	$InvalidInput = 1;
}
if ($InvalidInput == 1) {
	print "Usage: CERTListFiles.pl [Smarty Results Directory] [Input LST Directory [Output LST Directory]\n";
	exit(0);
}

opendir SMARTYOUTDIR, $g_SmartyResultsDir or die "Unable to open [$g_SmartyResultsDir]! $!\n";
my @failedListFiles = sort {$a > $b} grep /.*\.fail\.smrt/i, readdir SMARTYOUTDIR;
closedir SMARTYOUTDIR;

my $hash = {};

my $totalFailures = 0;

# if there are any failure, get them into the hash table
if(scalar(@failedListFiles) > 0) {

	foreach my $elem (@failedListFiles)
	{
		my $localfailures= 0;
		my $failedList="$g_SmartyResultsDir\\$elem";
		open FAILEDLIST, $failedList or die "Unable to open [$failedList]!$!\n";

		while(<FAILEDLIST>) {
			chomp;
			my ($lstfile, $guid) = split(/\s*=\s*/, $_);
			$guid =~ s/[,].*$//;
			$lstfile =~ s/^.*\\([^\\]+)$/$1/i;
			$hash->{uc $lstfile}->{uc $guid}=1;
			$totalFailures++;
			$localfailures++;
		}

		print "CertListFiles.pl:: Found $localfailures failure(s) in $failedList\n";
	}		
}

close FAILEDLIST;

print "Total number of failures: $totalFailures!\n";


#################################
# CERT_ALL_INCLUDES
if(-e "$g_InputLstFileDir\\CERT_ALL_INCLUDES.LST") {
	print `$ENV{_DBVT_CONST_CLIENT_SCRIPT_ROOT}\\Tools\\RoboCopy.exe /TBD /R:30 /W:60 $g_InputLstFileDir $g_OutputLstFileDir CERT_ALL_INCLUDES.LST`;
} else {
	open BVT, "$g_InputLstFileDir\\BVT_ALL_INCLUDES.LST" or die "Unable to open [$g_InputLstFileDir\\BVT_ALL_INCLUDES.LST]! $!\n";
	open CERT, ">$g_OutputLstFileDir\\CERT_ALL_INCLUDES.LST" or die "Unable to write to [$g_OutputLstFileDir\\CERT_ALL_INCLUDES.LST]! $!\n";
	while(<BVT>) {
		$_=~ s/\bBVT_(.*\.lst)/CERT_$1/i;
		print CERT $_;
	}
	close BVT;
	close CERT;
}

#################################
# CERT_CheckinBvt and CERT_BuildBvt
my $failedFiles = join(' ', keys %$hash);
if($failedFiles !~ /CheckinBVT.LST/i) {
	if(-e "$g_InputLstFileDir\\BVT_CheckinBVT.LST") {print `echo f | xcopy /Y $g_InputLstFileDir\\BVT_CheckinBVT.LST $g_OutputLstFileDir\\CERT_CheckinBVT.LST`;}
	if(-e "$g_InputLstFileDir\\CERT_CheckinBVT.LST") {print `echo f | xcopy /Y $g_InputLstFileDir\\CERT_CheckinBVT.LST $g_OutputLstFileDir\\CERT_CheckinBVT.LST`;}
}
if($failedFiles !~ /BuildBVT.LST/i) {
	if(-e "$g_InputLstFileDir\\BVT_BuildBVT.LST") {print `echo f | xcopy /Y $g_InputLstFileDir\\BVT_BuildBVT.LST $g_OutputLstFileDir\\CERT_BuildBVT.LST`;}
	if(-e "$g_InputLstFileDir\\CERT_BuildBVT.LST") {print `echo f | xcopy /Y $g_InputLstFileDir\\CERT_BuildBVT.LST $g_OutputLstFileDir\\CERT_BuildBVT.LST`;}
}
if($failedFiles !~ /SelfHostBVT.LST/i) {
	if(-e "$g_InputLstFileDir\\BVT_SelfHostBVT.LST") {print `echo f | xcopy /Y $g_InputLstFileDir\\BVT_SelfHostBVT.LST $g_OutputLstFileDir\\CERT_SelfHostBVT.LST`;}
	if(-e "$g_InputLstFileDir\\CERT_SelfHostBVT.LST") {print `echo f | xcopy /Y $g_InputLstFileDir\\CERT_SelfHostBVT.LST $g_OutputLstFileDir\\CERT_SelfHostBVT.LST`;}
}

foreach my $key (keys %$hash) {
	open BVT, "$g_InputLstFileDir\\$key" or die "Unable to open [$g_InputLstFileDir\\$key]! $!\n";
	my $filename = $key;
	$filename=~s/\bBVT_/CERT_/i;
	open CERT, ">$g_OutputLstFileDir\\$filename" or die "Unable to write to [$g_OutputLstFileDir\\$filename]! $!\n";

	my $foundFailedTest=0;
	while(<BVT>) {
		if(/\[(.*)\]/) {
			if($hash->{uc $key}->{uc $1}) {
				$foundFailedTest=1;
			}
			print CERT $_;
		} elsif(/Categories\s*=/i) {
			my $cat = $_;
			chomp $cat;
			
			if(1 == $foundFailedTest) {
				if($cat !~ /\bINVALID\b/i) {
					#remove trailing ';'
					$cat =~ s/;$//;
					$cat = $cat.";INVALID";
				}
				$foundFailedTest=0;
			}
			print CERT "$cat\n";
		} else {
			print CERT $_;
		}
	}
}
