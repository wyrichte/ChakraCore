// 
// run.local.js
//
// Nathan's personal supplements to runjs.
// 
// Add a second machine to the list.
// _myMachineMan is set up in doRun.js as machManNew()
//
// machManAdd is a little weird about speed. Presumes MHz, but only one core?
// I'll add a multiplicative factor.

//_myMachineMan = machManNew(true /* don't add self */);

//machManAdd(_myMachineMan, "NH-AMD64", "AMD64", 2200 * 2);
//machManAdd(_myMachineMan, "NH-LHS-2", "AMD64", 2200 * 8);

switch (Env("COMPUTERNAME")) {

case "BARRICADE":
    machManAdd(_myMachineMan, "RUBBLE2", "arm", 1000 * 1);
    _taskDailyDevRun.dependents.unshift(_devBVTTask("chk", "arm"));
    break;

default:
    logMsg(LogRun, LogInfo, "runjs.local.js: " + Env("COMPUTERNAME") + " is not a registered computer.\n");
case "RUBBLE2":
    break;

}

//logSetFacilityLevel(LogFSO, 6);
logSetFacilityLevel(LogTask, 2);
//logSetFacilityLevel(LogTask, 6);
