param([string]$trackerFileName, [string]$startDateParam, [string]$endDateParam)

if ([System.String]::IsNullOrEmpty($trackerFileName)) {
  Throw "must specify tracker file name"
}

if ([System.String]::IsNullOrEmpty($startDateParam)) {
  Throw "must specify start date"
}

if ([System.String]::IsNullOrEmpty($endDateParam)) {
  Throw "must specify end date"
}

$vc = "https://cosmos15.osdinfra.net/cosmos/asimov.partner.osg"
$root = "/shares/asimov.prod.data/PublicPartner/Processed/ChakraJavaScript/Tracker/";
$trackerFileRootPath = $vc + $root + $trackerFileName + "/v1/" + $trackerFileName

$startDate = [DateTime]::Parse($startDateParam);
$endDate = [DateTime]::Parse($endDateParam);
$currentDate = $startDate;


while ($currentDate.CompareTo($endDate) -le 0) {
  $y = $currentDate.Year
  $m = "{0:D2}" -f $currentDate.Month
  $d = "{0:D2}" -f $currentDate.Day
  $t = ${trackerFileRootPath} + "_" + ${y} + "_" + ${m} + "_" + ${d} + ".txt"
  $exists = Test-CosmosStream -Stream $t
  if (! $exists) {
    New-CosmosStream -Path $t
    Write-Host wrote stream $t
  }
  $currentDate = $currentDate.AddDays(1);
}

