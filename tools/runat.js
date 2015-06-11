var WshShell   = new ActiveXObject("WScript.Shell");
var WshFSO = new ActiveXObject("Scripting.FileSystemObject");
var Env = WshShell.Environment("PROCESS");

var args = WScript.Arguments;

var homeDir = Env("HOMEDRIVE") + Env("HOMEPATH");
logFileName = homeDir + "\\RunAt.log";

    // Do we want our own log file?
var i = 0; 
if (i < args.length && args(i).match(/[\/-]log:(.*)/)) {
    logFileName = RegExp.$1;
    i++;
}
var outFileName = logFileName.replace(/\.log$/, ".out.txt");

    // Get the time
if (i + 1 >= args.length || !args(i).match(/(\d+):(\d+)/)) {
    WScript.Echo("Usage runAt [/log:fileName] hh:mm <command>");
    WScript.Echo("");
    WScript.Echo("RunAt is a simple script that runs a given command every day at the same time.");
    WScript.Echo("You give the time as well as the command to run at the 'runAt' command line.");
    WScript.Echo("RunAt detaches itself from the console to do its work.   You can confirm");
    WScript.Echo("that it is running by looking for a 'wscript' program in task manager.");
    WScript.Echo("");
    WScript.Echo("RunAt creates two logs while doing its work: RunAt.log and Runat.out.txt");
    WScript.Echo("RunAt.log is the log for the script itself which logs when tries to run");
    WScript.Echo("a command and what its return code is.  RunAt.out.txt the output of the");
    WScript.Echo("last command that RunAt attempted to invoke.  By default these files are");
    WScript.Echo("placed in your home directory (%HOMEPATH%), however you can overrride this");
    WScript.Echo("default with the /log qualifier.  The out.txt file is always derived from the ");
    WScript.Echo("name of the log file.");
    WScript.Echo("");
    WScript.Echo("Currently RunAt supports running only one command once a day.  However you can");
    WScript.Echo("run mulitple RunAt processes simultaneously.  If you do this you need to ");
    WScript.Echo("use the /log qualifier. to give each separate instance its own log file.");
    WScript.Echo("");
    WScript.Quit(1);
}
var time = args(i);
var targetFromMid = msecFromMidNight(RegExp.$1 - 0, RegExp.$2 - 0, 0);
i++;

    // Get the command
var command = "";
while(i < args.length) {
    command += " " + args(i);
    i++;
}
if (!command.match(/>/))
    command += " > \"" + outFileName + "\" 2>&1";
command = "cmd /c " + command;

var haveStdOut = WScript.FullName.match(/cscript.exe/i);
if (haveStdOut)
    WScript.Echo("RunAt: logging to " + logFileName);

var logFile;
try {
    logFile = WshFSO.OpenTextFile(logFileName, 2, true);
}
catch(e) {
    WScript.Echo("ERROR: Could not open log file " + logFileName + "\r\nIs there another Instance of runat?");
    WScript.Quit(1);
}
logFile.WriteLine(new Date() + " Time to Run command: " + time);

for(;;) {
    var now = new Date();
    var nowFromMid = msecFromMidNight(now.getHours(), now.getMinutes(), now.getSeconds());

    var timeLeft = targetFromMid - nowFromMid;
    if (timeLeft <= 0)
        timeLeft += 24*3600000;
    
    logFile.WriteLine(new Date() + " Sleeping " + (timeLeft /3600000).toFixed(2) + " hours");
    WScript.Sleep(timeLeft)
    logFile.WriteLine(new Date() + " Running Cmd:" + command);
    var ret = WshShell.run(command, 7, true);
    logFile.WriteLine(new Date() + " Command returned: " + ret);
}

function msecFromMidNight(hour, min, sec) {
    return (((hour * 60 + min) * 60) + sec) * 1000;
}

