var resultIP = "";
try {
    var inFile = FSOOpenTextFile("c:\\BeagleBoard\\ipaddress.txt", FSOForReading);
    try {
        resultIP = inFile.ReadLine();
    }
    finally {
        inFile.Close();
    }
} catch (ex) {
    // file probably wasn't found
}

resultIP;
