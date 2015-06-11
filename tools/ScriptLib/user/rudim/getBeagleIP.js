var resultIP = "";
try
{
    var inFile = FSOOpenTextFile("\\\\rudim2\\e$\\ip.wpe.txt", FSOForReading);
    resultIP = inFile.ReadLine();
}
finally
{
    inFile.Close();
}

resultIP;
