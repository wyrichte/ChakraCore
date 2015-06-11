var resultIP = "";
var inFile = FSOOpenTextFile("C:\\BeagleBoard\\ipaddress.txt", FSOForReading);
try {
    resultIP = inFile.ReadLine();
}
finally {
    inFile.Close();
}

resultIP;
