var resultIP = "";
var inFile = FSOOpenTextFile("e:\\vbl\\clr_arm\\beagleip.txt", FSOForReading);
try {
    resultIP = inFile.ReadLine();
}
finally {
    inFile.Close();
}

resultIP;
