function _razzleBuildDependenciesTaskGroup(bldType, bldArch) {
    bldType = getBldType(bldType);
    bldArch = getBldArch(bldArch);

    return taskGroup("buildDependencies_" + bldArch + bldType, [
        _razzleBuildTask(bldType, bldArch, "inetcore\\published\\sdk\\inc", undefined, 3 * HOUR), // Give this a long timeout as publics can take a while
        _razzleBuildTask(bldType, bldArch, "inetcore\\published\\sdk\\uuid"),
        _razzleBuildTask(bldType, bldArch, "inetcore\\published\\internal\\inc", undefined, 3 * HOUR), // Give this a long timeout as publics can take a while
        _razzleBuildTask(bldType, bldArch, "inetcore\\published\\internal\\uuid"),
        _razzleBuildTask(bldType, bldArch, "inetcore\\manifests\\inbox"),
        _razzleBuildTask(bldType, bldArch, "inetcore\\lib\\codex"),
        _razzleBuildTask(bldType, bldArch, "inetcore\\lib\\nav\\fck\\iel2"),
        _razzleBuildTask(bldType, bldArch, "inetcore\\lib\\common\\iel1"),
        _razzleBuildTask(bldType, bldArch, "inetcore\\lib\\common\\iel1_mc"),
        _razzleBuildTask(bldType, bldArch, "inetcore\\lib\\nav\\iel2"),
        _razzleBuildTask(bldType, bldArch, "inetcore\\lib\\devtb\\dtbhost"),
        _razzleBuildTask(bldType, bldArch, "inetcore\\lib\\indexeddb\\scalookup")
    ]);
}

var _taskBuildDependenciesHere =
    taskGroup("buildDependenciesHere", [
        _razzleBuildDependenciesTaskGroup(),
    ]);

_tasksAll[_tasksAll.length] = _taskBuildDependenciesHere;
