var resultIP = "";
try {
    var canUseBeagleBoard = true;
    if (FSOFileExists("\\\\GRANTRI5\\C$\\dd\\BeagleBoard\\scripts\\armlocked.txt")) {
        var lockMachine = FSOReadFromFile("\\\\GRANTRI5\\C$\\dd\\BeagleBoard\\scripts\\armlocked.txt");
        if (lockMachine != "" && lockMachine != Env("COMPUTERNAME")) {
            logMsg(LogBeagle, LogInfo, "Not using beagle board because it is locked by \'", lockMachine, "\'\n");
            canUseBeagleBoard = false;
        }
    }
    if (canUseBeagleBoard) {
        var inFile = FSOOpenTextFile("\\\\GRANTRI5\\C$\\dd\\BeagleBoard\\scripts\\armip.txt", FSOForReading);
        resultIP = inFile.ReadLine();
    }
}
finally {
    inFile.Close();
}

resultIP;
