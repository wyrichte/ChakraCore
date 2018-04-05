param([string]$streamName)

if ([System.String]::IsNullOrEmpty($streamName)) {
  Throw "must specify stream name"
}

$vc = "https://cosmos15.osdinfra.net/cosmos/asimov.partner.osg"
$root = "/shares/asimov.prod.data/PublicPartner/Processed/ChakraJavaScript/CookedChakraTelemetry/";

write-host "deleting streamset for ${streamName}..."

For ($y=2016; $y -lt 2019; $y++) {
    $p1 = "${vc}${root}${y}";
    if (Test-CosmosFolder -Stream $p1) {
        For ($m=1; $m -lt 13; $m++) {
            # format month with leading zero if necessary
            $m0 = "{0:D2}" -f $m;
            $p2 = $p1 + "/${m0}";
            if (Test-CosmosFolder -Stream $p2) {
                For ($d=1; $d -lt 32; $d++) {
                     # format day with leading zeros if necessary
                    $d0 = "{0:D2}" -f $d;
                    $p3 = $p2 + "/${d0}"
                    if (Test-CosmosFolder -Stream $p3) {
                        $s = $p3 + "/" + $streamName;
                        if (Test-CosmosStream -Stream  $s) {
                            write-host "deleting $s"
                            Remove-CosmosStream -Stream $s
                        }
                    }
                }
            }
        }
    }
}