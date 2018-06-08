param([string]$streamName, [string]$startDateParam, [string]$endDateParam)

if ([System.String]::IsNullOrEmpty($streamName)) {
  Throw "must specify stream name"
}

if ([System.String]::IsNullOrEmpty($startDateParam)) {
  Throw "must specify start date"
}

if ([System.String]::IsNullOrEmpty($endDateParam)) {
  Throw "must specify end date"
}

write-host "deleting streamset for ${streamName}..."

$vc = "https://cosmos15.osdinfra.net/cosmos/asimov.partner.osg"
$root = "/shares/asimov.prod.data/PublicPartner/Processed/ChakraJavaScript/CookedChakraTelemetry/";

$startDate = [DateTime]::Parse($startDateParam);
$endDate = [DateTime]::Parse($endDateParam);
$currentDate = $startDate;

while ($currentDate.CompareTo($endDate) -le 0) {
  $y = $currentDate.Year
  $m = "{0:D2}" -f $currentDate.Month
  $d = "{0:D2}" -f $currentDate.Day
  $t = ${vc} + ${root} + ${y} + "/" + ${m} + "/" + ${d} + "/" + $streamName
  $exists = Test-CosmosStream -Stream $t
  if ($exists) {
    Remove-CosmosStream -Stream $t
    Write-Host removed stream $t
  }
  $currentDate = $currentDate.AddDays(1);
}
