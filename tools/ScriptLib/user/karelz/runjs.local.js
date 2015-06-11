// 
// HOW TO:
// 
// Run SelfHost:
//    runjs copyAndRunSelfHost <alias> [<alias> ...]
//        where aliases can be:
//            TypeSystem
//            MetaData
//            Reflection
//            Debugger
//            Mdbg
//            See file:..\..\clrSelfHost.js#AddAliasToGlobalList
// 
// Create smarty diff files:
//    runjs my_smartyCreateDiffFiles <smarty output dir> [<smarty baseline dir>]
//        e.g.:
//          runjs my_smartyCreateDiffFiles J:\puclr_j\automation\run.current\x86chk\test.devBVT
//        See: code:#my_smartyCreateDiffFiles
// 
//=======================================================================================

//=======================================================================================
//=======================================================================================
//=======================================================================================

//#my_smartyCreateDiffFiles
function my_smartyCreateDiffFiles(snapDir, baselineDir, smartyData) {

    var smartyDataFileName = snapDir + "\\Smarty.err.xml";
    if (!FSOFileExists(smartyDataFileName))
        return;
    
    if (baselineDir == undefined)
        baselineDir = snapDir + "\\baseline";
    if (smartyData == undefined)
        smartyData = xmlRead(smartyDataFileName);

    var testsInfo = smartyData.TESTRESULT.MYBLOCK.TESTCASES;
    var tests = toList(testsInfo.TESTCASE);

    var baselineDataFileName = baselineDir + "\\Smarty.xml";
    if (!FSOFileExists(baselineDataFileName)) {
        return;
    }
    var baselineData = xmlRead(baselineDataFileName);
    var baselineTestsInfo = baselineData.TESTRESULT.MYBLOCK.TESTCASES;
    var baselineTests = toList(baselineTestsInfo.TESTCASE);

    var baselineTable = {}
    for(var i = 0; i < baselineTests.length; i++) {
        var baselineTest = baselineTests[i];
        baselineTable[baselineTest.TESTID] = baselineTest;
    }

    var failuresDeterministic = [];
    var failuresNondeterministic = [];
    var failuresDifferFromBaseline = [];
    var failuresDifferFromBaseline_Deterministic = [];
    var failuresDifferFromBaseline_Nondeterministic = [];
    var failuresSameAsBaseline = [];
    var failuresNoBaseline = [];

    for(var i = 0; i < tests.length; i++) {
        var test = tests[i];
        if (test.SUCCEEDED == "yes")
            continue;

        var isFailureDeterministic;
        if (test.PASSRATE == "0%") {
            isFailureDeterministic = true;
            failuresDeterministic.push(test.TESTID);
        }
        else {
            isFailureDeterministic = false;
            failuresNondeterministic.push(test.TESTID);
        }

        var baselineTest = baselineTable[test.TESTID];
        if (baselineTest != undefined) {
            if (baselineTest.SUCCEEDED == "yes") {
                failuresDifferFromBaseline.push(test.TESTID);
                if (isFailureDeterministic) {
                    failuresDifferFromBaseline_Deterministic.push(test.TESTID);
                } else {
                    failuresDifferFromBaseline_Nondeterministic.push(test.TESTID);
                }
            }
            else {
                failuresSameAsBaseline.push(test.TESTID);
            }
        }
        else {
            failuresNoBaseline.push(test.TESTID);
        }
    }
    
    var failuresFileName = snapDir + "\\Smarty.0.fail.smrt";
    _smartyCreateFilteredFailureFile(failuresFileName, failuresDeterministic, snapDir + "\\Smarty.0.fail_Deterministic.smrt");
    _smartyCreateFilteredFailureFile(failuresFileName, failuresNondeterministic, snapDir + "\\Smarty.0.fail_Nondeterministic.smrt");
    _smartyCreateFilteredFailureFile(failuresFileName, failuresSameAsBaseline, snapDir + "\\Smarty.0.fail_SameAsBaseline.smrt");
    _smartyCreateFilteredFailureFile(failuresFileName, failuresNoBaseline, snapDir + "\\Smarty.0.fail_NoBaseline.smrt");
    _smartyCreateFilteredFailureFile(failuresFileName, failuresDifferFromBaseline, snapDir + "\\Smarty.0.fail_DifferFromBaseline.smrt");
    _smartyCreateFilteredFailureFile(failuresFileName, failuresDifferFromBaseline_Deterministic, snapDir + "\\Smarty.0.fail_DifferFromBaseline_Deterministic.smrt");
    _smartyCreateFilteredFailureFile(failuresFileName, failuresDifferFromBaseline_Nondeterministic, snapDir + "\\Smarty.0.fail_DifferFromBaseline_Nondeterministic.smrt");
}

function _smartyCreateFilteredFailureFile(failuresFileName, testIDs, outputFileName) {

    if (testIDs.length == 0) {
        // There's nothing to write
        return;
    }
    if (FSOFileExists(outputFileName)) {
        return;
    }
	var outputFile = FSOOpenTextFile(outputFileName, 2, true);

    var inputFile = FSOOpenTextFile(failuresFileName, 1, false, -2); // -2 = TriStateDefault
    var line;
    var testsSectionFound = false;
	while (!inputFile.AtEndOfStream) {
	    line = inputFile.ReadLine();
	    if (!testsSectionFound) {
	        if (line == "[TESTS]") {
	            testsSectionFound = true;
	        }
	        outputFile.WriteLine(line);
	    } else {
	        // Filter only tests present in testIDs
	        for (var testIndex in testIDs) {
	            if (line.indexOf(testIDs[testIndex]) != -1) {
	                // Test ID found, print the test to the output
	                outputFile.WriteLine(line);
	                break;
	            }
	        }
	    }
	}
}
