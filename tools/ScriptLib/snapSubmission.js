/*                             snapSubmission.js                             */
/*****************************************************************************/

/* Used for SNAP submission form data entry and parsing.
*/

/* AUTHOR: Joel Hendrix
   DATE: 2/8/2007 */

/******************************************************************************/
var snapSubmissionModuleDefined = 1; // indicates that the module exists

if (!InternetExplorerModuleDefined) throw new Error(1, "need to include internetExplorer.js");

// global variable that will contain various SDTrack information
var SDTrackInfo = {};
SDTrackInfo.Tag = undefined; // this is the tag that will be inserted into the check-in info
SDTrackInfo.UnapprovedCount = 0; /* this is the number of entries in the bug table that are not approved.
                                    if this is greater than zero then the submission will be blocked. */

/*Regular expressions used for SDTrack validation*/
var SDTrackRegEx = {};
SDTrackRegEx.BuildTag = "\\[\\s*BUILD\\s*\\]";
SDTrackRegEx.BugTag = "\\[\\s*PSBUG\\s*=\\s*[a-zA-Z0-9]+(?:\\s+[a-zA-Z0-9]*)*\\s*,\\s*[0-9]+(?:\\s*\\|\\s*[a-zA-Z0-9]+(?:\\s+[a-zA-Z0-9]*)*\\s*,\\s*[0-9]+)*\\s*\\]";

// the "greedy" ones are only used when stripping out
// any malformed tags when loading default values.
// assume that the first two letters are correct but ignore the case.
SDTrackRegEx.GreedyBuildTag = "\\[\\s*(?:b|B)(?:u|U).*?(?=\\])\\]";
SDTrackRegEx.GreedyBugTag = "\\[\\s*(?:p|P)(?:s|S).*?(?=\\])\\]";

// this tag is used when checking in to directories in
// the depot that are not monitored by SD Track. this would
// typically be used by test/infrastructure fixes.
SDTrackRegEx.NoSDTrackTag = "\\[\\s*NOSDTRACK\\s*\\]";
SDTrackRegEx.GreedyNoSDTrackTag = "\\[\\s*(?:n|N)(?:o|O).*?(?=\\])\\]";

/******************************************************************************/
/* checks for the presence of any one of the SDTrack tags. */
function checkForAnySDTrackTag(text, greedySearch) {

    if (greedySearch == false) {
        if (text.match(SDTrackRegEx.BuildTag) ||
            text.match(SDTrackRegEx.BugTag) ||
            text.match(SDTrackRegEx.NoSDTrackTag))
            return true;
        else
            return false;
    } else {
        if (text.match(SDTrackRegEx.GreedyBuildTag) ||
            text.match(SDTrackRegEx.GreedyBugTag) ||
            text.match(SDTrackRegEx.GreedyNoSDTrackTag))
            return true;
        else
            return false;
    }
}

/******************************************************************************/
/* read a change description from a source depot change, load it into
   the GUI, and then write out the updated change description

   Parameters
      changeNum : The change number to read from and update
      optStr    : Options multiple options allowed, but spaces must be quoted

*/
function checkinFormForChange(changeNum, optStr) {
    //doesn't look like a codepath we hit, but let's be safe
    if (_inTFS())
    {
        FatalError("internetExplorer.js: checkinFormForChange() was called unexpectedly in TFS environment.  Please send mailto:clrontfs.");
    }

    var changeInfo = sdDescribe(changeNum);
    var newSpec = checkinFormGui(changeInfo.desc, optStr);

    if (newSpec != undefined) {
        newSpec = newSpec.replace(/\r/g, "");
        
        logMsg(LogClrAutomation, LogInfo, "Updating the description for change ", changeNum, "\n");
        sdChange(newSpec, changeNum);
    }
    return 0;
}

/******************************************************************************/
/* read a change description from a file (need not exist), load it into
   the GUI, and then write out the updated change description 

   Parameters
      fileName  : The file to read from and then write the updated spec to.k
      optStr    : Options multiple options allowed, but spaces must be quoted

*/
function checkinFormForFile(fileName, queueName, formValidationLevel, optStr) {
    var spec = undefined;
    if (fileName != undefined) {
        if (FSOFileExists(fileName))
            spec = FSOReadFromFile(fileName, optStr);
        else 
            logMsg(LogClrAutomation, LogInfo, "File ", fileName, " does not exist, creating from scratch\n");
    }
    var newSpec = checkinFormGui(spec, queueName, formValidationLevel, optStr);
    if (newSpec != undefined) {
        newSpec = newSpec.replace(/\r*\n/g, "\r\n");
        logMsg(LogClrAutomation, LogInfo, "Updating the description in file ", fileName, "\n");


        //the below code fragment loops through the string passed as an argument to FSOWriteToFile() and searches for
        //non ASCII characters. According to documentation for JScript5.6, there is no type to represent a single character,
        //except the string of length 1, which means that comparing a number to a char can not be done in the usual way 
        //because it will do an alphabetical comparison of those 2.
        //below each character is compared alphabetically to '~', which code is 126 (character with code 127 is <DEL>)

        for (var i = 0; i < newSpec.length; i++) {
            if ('~' <= newSpec.charAt(i)) {
                logMsg(LogClrAutomation, LogInfo, "Found an invalid character at position "+ i +" in parameter for FSOWriteToFile() method\n");
                break;
            }
        }    
        try {
            FSOWriteToFile(newSpec, fileName);
        } 
        catch(e) {
            logMsg(LogFSO, LogWarn, "Exception thrown: " + e.description);
            throw e;
        }
        return 0;
    }
    return 1;
}

/******************************************************************************/
/* display the code base checkin gui, and return the result as a specification
   that is both human and machine readable.  If 'specStr' is present, it is 
   used to 'precharge' the fields of the form. 
*/

function checkinFormGui(specStr, queueName, formValidationLevel, optStr) {

    var spec = {};
    if (specStr != undefined) 
        spec = humanTextParse(specStr, "Change_description");
        
    var opts = getOptions(["testonly"], optStr);
    
    // opts.TestOnly     - indicates this SNAP job is test only and is not submitted to the depot
    // opts.PreserveTags - indicates that any SDTrack tags in specStr should not be removed

    if (spec.Checkin_tests == undefined)
        spec.Checkin_tests = findDailyDevReport();

	spec.Bugs_fixed = undefined;		// For now ignore previous input here.

    if (queueName == undefined) 
        queueName = "default";

    IEData = IEFormNew(ScriptDir + "\\scriptLib\\html\\checkinDialog\\" + queueName.toLowerCase() + ".html", 
                950, 1100, spec, checkinFormValidate, opts, checkinFormFieldsInit, checkinFormFieldsGather);
    IEData.formValidationLevel = formValidationLevel; 
    var inputDoc = IEData.inputDoc;

		// Set callback on the 'FindFixedBugs' button.
    inputDoc.all.FindFixedBugs.onclick = checkinFormFindFixedBugsCallBack;

        // By default, the title is the first line of the change description 
    if (spec.Title == undefined && spec.Change_description) {
        if (spec.Change_description.match(/^(.*)/)) 
            inputDoc.all.Title_input.value = RegExp.$1;
    }
    
        // Give focus first to the title field
    inputDoc.all.Title_input.focus(); 

        // Check that we ran dailyDevRun
    if (!spec.Checkin_tests.match(/^dailyDevRun run at/i)) {
        var msg = "Could not find your dailyDevRun report in the standard automation direcotry.\r\n" +
                  "While submitting without this testing is sometimes required, it is strongly discouraged.\r\n" +
                  "If you did not run dailyDevRun please indicate why in the 'Checkin Tests' field.";
        IEPoll(IEData);        // makes the form visble.
        inputDoc.parentWindow.alert(msg);
    }
    else if (spec.Checkin_tests.match(/\(\d+ hours old\)/i)) {
        var msg = "Found only a stale report is more than 30 hours old.\r\n" +
                  "We prefer checkins to have run dailyDevRun within 30 hours.";
        IEPoll(IEData);        // makes the form visble.
        inputDoc.parentWindow.alert(msg);
    }

    // Get the result of presenting the form to the user.
    var retVal = IEFormResult(IEData);
    if (retVal != undefined) {
        checkinFormMorph(retVal);
        var ret = humanTextPrint(retVal, retVal._ORDER, {Bugs_fixed:true});
			// put the title on the title line
		ret = ret.replace(/^\s*Title:\s*/, "Title: ");
		return ret;
    }

    logMsg(LogClrAutomation, LogInfo, "Form Canceled changes abandoned\n");  
    return undefined;
}

/******************************************************************************/
/* This routine is called when the 'Find Fixed Bugs' button is pressed.  It
   populates the set of fixed bugs in the form */

function checkinFormFindFixedBugsCallBack() {

    // logMsg(LogClrAutomation, LogInfo, "In checkinFormFindFixedBugsCallBack\n");
    var inputDoc = IEData.inputDoc;
	try {
        var htmlTable = inputDoc.all.Bugs_fixed_table;
		var i = htmlTable.rows.length
		while (i > 0) 
			htmlTable.deleteRow(--i);
                
		var bugsFixedData = checkinGetFixedBugs();
		checkinFormCreateBugTable(inputDoc, htmlTable, bugsFixedData);
	}
	catch(e) {
		var failureMsg = "There was a failure looking up your fixed bugs\r\n" +
						 "Error Information: " + e.description + "\r\n";
		inputDoc.parentWindow.alert(failureMsg);
	}
}

/*****************************************************************************/
/* fill in any daily dev run report information */

function findDailyDevReport() {

    var ret = "No DailyDevRun done.";
    var latestReport = Env("_NTBINDIR") + "\\automation\\run.current\\taskReport.html";
    if (!FSOFileExists(latestReport)) {
        latestReport = "\\\\" + Env("COMPUTERNAME") + "\\automation\\run.current\\taskReport.html";
    }
    if (FSOFileExists(latestReport)) {
        var runTime = new Date(FSOGetFile(latestReport).DateLastModified);
        var now = new Date();
        var hours = Math.floor((now.getTime() - runTime.getTime()) / (3600 * 1000));
        if (hours < 3.5 * 24) {        // only consider things less than 3.5 days old
            ret = "dailyDevRun run at " + prettyTime(runTime);
            if (hours > 30)
                ret += " (" + hours + " hours old)";
        }
    }
    // logMsg(LogClrAutomation, LogInfo, "returning ", ret, "\n");
    return ret;
}

/*****************************************************************************/
/* returns a string representing bugs fixed by 'user' for the database 'dbName'
   The string has the form of <bugid> <title> one per line */

function checkinGetFixedBugsSD(user, dbName) {
    logMsg(LogClrAutomation, LogInfo, "checkinGetFixedBugsSD()\n");

    if (user == undefined)
        user = Env("USERNAME");
    if (dbName == undefined)
        dbName = "DevDiv Bugs";

    logMsg(LogClrAutomation, LogInfo, "Connecting to Product Studio DB ", dbName, "\n");
    var bugDb = bugConnect(dbName);

    logMsg(LogClrAutomation, LogInfo, "Doing Query for bugs with fixed substatus for user ", user, "\n");
    var results = bugQuery(bugDb, bugQAnd(
        bugQEquals("Assigned to", user), 
        bugQEquals("Status", "active"),
        bugQOr(
            bugQEquals("Sub Status", "Fix Available"),
            bugQEquals("Sub Status", "Fix Queued"),
            bugQEquals("Sub Status", "Fix Tested"))));

    logMsg(LogClrAutomation, LogInfo, "Query finished\n");
    var ret = "";
    for (var i = 0; i < results.length; i++) {
        var result = results[i];
        var id = bugFieldGet(bugDb, result, "ID");
        var title = bugFieldGet(bugDb, result, "Title");
        ret += id + " " + title + "\r\n";
    }

    return ret;
}

function checkinGetFixedBugsTFS()
{
    logMsg(LogClrAutomation, LogInfo, "checkinGetFixedBugsTFS()\n");
    
    // Note: If our devdiv TFS work item schema changes, this query might need to change.
    // Since each field gets a "friendly" name and a "reference" name, creating a query
    // can be error-prone. The best way to avoid ambiguity is to build up the query using
    // the VS GUI by creating a new query under Work Items->My Queries, and then saving
    // it to a file (which gets a "wiq" extension). You can then paste that wiq text into
    // this string (do NOT use field names as they show up in the VS GUI).
    var workItems = _tfsQueryWorkItems(
        "SELECT [System.Id], [System.Title] " +
        "  FROM WorkItems " +
        "  WHERE [System.AssignedTo]= @Me" +
        "    AND [System.State] = 'Active' " +
        "    AND ( " +
        "               [Microsoft.DevDiv.SubStatus] = 'Fix Available' " +
        "           OR  [Microsoft.DevDiv.SubStatus] = 'Fix Queued' " +
        "           OR  [Microsoft.DevDiv.SubStatus] = 'Fix tested' " +
        "        ) "
        );

    // _tfsQueryWorkItems returns the bug list as an array, with each element in the format:
    // 
    // BugID<space>BugTitle
    // 
    // The code below just appends it all together into a single string with bugs
    // separated by newlines.

    var ret = "";
    for (var i = 0; i < workItems.length; i++) {
        ret += workItems[i] + "\r\n";
    }
    return ret;
}

function checkinGetFixedBugs()
{
    logMsg(LogClrAutomation, LogInfo, "checkinGetFixedBugs()\n");
    if (_inTFS())
    {
        return checkinGetFixedBugsTFS();
    }
    else
    {
        return checkinGetFixedBugsSD(Env("USERNAME"), "DevDiv Bugs");
    }
}

/****************************************************************************/
/* does validity checkin on the values in the Checkin form.  Returns either
   an error message string on error, or undefined on success 
*/
function checkinFormValidate(values, opts) {

    var validationLevel = IEData.formValidationLevel;
    if (validationLevel == undefined) 
        validationLevel = 5; //Maximum
    
    if (values.Title == "") 
        return "Title can't be empty";

    if (values.Issue_description == "") {
		var haveBug = false;
		for (var name in values) 
			if (name.match(/Bug_(\d+)/))
				haveBug = true;
		if (!haveBug)
			return "The Issue Description field can not be empty unless\r\n" +
				   "entries in the 'Find My Fixed Bugs' table are selected."
	}


    if (values.Checkin_tests == "")
        return "The Checkin Tests field can not be empty.";

	if (opts.testonly != undefined) {
		if (values.Code_review != "") 
			return "Do not fill in the code review field for test only jobs";
		if (values.Test_review != undefined && values.Test_review != "")
			return "Do not fill in the test review field for test only jobs";
		if (values.Security_review != undefined && values.Security_review != "")
			return "Do not fill in the security review field for test only jobs";
	}
	else {
    if (values.Code_review == "") 
        return "The Code Review field can't be empty.\r\n" + 
               "Please place 'Not Needed' if you really believe the change does not need it";

    if (values.Test_review != undefined && values.Test_review == "" && validationLevel > 2) 
        return "The Test Review field can't be empty.\r\n" +
               "Please place 'Not Needed' if you really believe the change does not need it";
        
    if (values.Security_review != undefined && values.Security_review == "" && validationLevel > 3) 
        return "The Security Review field can't be empty.\r\n" +
               "Please place 'Not Needed' if you really believe the change does not need it";
        
    if (values.Security_review != undefined 
         && values.Code_review.toLowerCase() == values.Security_review.toLowerCase())
        return "The Code Reviewer should not be the same as the Security reviewer.\r\n" +
               "Even if the Code Reviewer is the best person to do the Security review,\r\n" +
               "you do need to have a second pair of eyes take a look at the changes.\r\n" +
               "Please email ClrSecCR for clarifications.\r\n";
        
    if (values.Test_review != undefined 
        && values.Code_review.toLowerCase() == values.Test_review.toLowerCase())
        return "The Code Reviewer should not be the same as the Test reviewer.\r\n" +
               "Even if the Code Reviewer is the best person to do the Test review,\r\n" +
               "you do need to have the test owner take a look at the changes.\r\n";
	}
	
	
	/*
	The info entered in the checkin form is saved in an .XML file. After the SNAP job finishes running, 
	the job description is read from the .XML, and sent to jobReport.html file. Because the report is in HTML 
	format, characters '<', '>' and '&' must be properly HTML-escaped
	*/
	
    for(var i in values) {
        if(typeof(values[i]) == "string") {
            values[i] = escapeHTML(values[i]);
		}
	}
    return undefined;
}

/****************************************************************************/
/* The values as returned by the IE form have fields Bug_<xxx> which are 
   either undefined or the title of bug.  This morphs all of these into
   a single 'Bugs_fixed' field, which reflects how we want to output
   this information */

function checkinFormMorph(values) {

    var fields = [];
	var bugsFixedInited = false;
    for (var i = 0; i < values._ORDER.length; i++) {
        var name = values._ORDER[i];
        if (name == "Bugs_fixed") {
			values.Bugs_fixed = values.Bugs_fixed.replace(/\s*(\s|,)\s*/g, "\r\n");
			if (!bugsFixedInited)
				fields.push(name);
			bugsFixedInited = true;
		}
		else if (name.match(/Bug_(\d+)/)) {
            var bugID = RegExp.$1;
            // logMsg(LogClrAutomation, LogInfo, "matched bug ", bugID, "\n");
            if (values.Bugs_fixed != "") 
                values.Bugs_fixed += "\r\n"
			values.Bugs_fixed += bugID + " " + values[name];

			if (!bugsFixedInited)
				fields.push(name);
			bugsFixedInited = true;
        }
        else 
            fields.push(name);
    }
    // logMsg(LogClrAutomation, LogInfo, "fields after morph = ", dump(fields), "\n");

	// Add the user name who checked in the change 
	var checkedInBy = Env("USERNAME");
	if (checkedInBy != null) {
		if (values["Checked in by"] == null)
			fields.push("Checked in by");
		values["Checked in by"] = checkedInBy;
	}

    values._ORDER = fields;

    // logMsg(LogClrAutomation, LogInfo, "In checkinFormMorph ret = ", dump(values, 2), "\n");
    return values;
}

/******************************************************************************/
/* Given a specification of bugs 'bugSpecStr' (<bugNum> <title> one per line)
   And a htmlTable object, add rows to the table for each bug, containing a 
   checkbox, the title an the title for the user to select */

function checkinFormCreateBugTable(inputDoc, htmlTable, bugSpecStr) {

    var bugListStr = "";
    bugs = bugSpecStr.split("\n");
    for (var i = 0; i < bugs.length; i++) {
        if (!bugs[i].match(/^\s*(\d+)\s+(.*?)\s*$/))
            continue;
        var id = RegExp.$1;
        var title = RegExp.$2
        title = title.replace(/'/g, "\"");

        var row = htmlTable.insertRow();
        var cell = row.insertCell()
        cell.innerHTML = "<INPUT TYPE=CHECKBOX VALUE='" + title + "' ID=Bug_" + id + "_input></INPUT>";
        cell.children[0].onclick = checkinTitleUpdateCallBack;
        cell.children[0].onfocus = checkinHelpUpdateCallBack;
        row.insertCell().innerHTML = id;
        row.insertCell().innerHTML = title;
        
        if (bugListStr != "")
            bugListStr += ", ";
        bugListStr += id;
    }

    if (bugListStr != "") {
        if (inputDoc.all.Title_input.value.match(/^\s*(Bug\S*)?\s*(Fix\S*)?\s*(Bug\S*)?\s*$/i))
            inputDoc.all.Title_input.value = "Fix bugs: "; //  + bugListStr;
        htmlTable.border = 1;
    }
    else 
        htmlTable.insertRow().insertCell().innerHTML = "No Fixed Bugs";
}

/******************************************************************************/
/* This is called back when a field gets focus.  Its job is to highlight the 
   help associated with the field.  It assumes the global variable IEData */

function checkinHelpUpdateCallBack() {

    var helpDoc = IEData.helpDoc;
    var helpTag = helpDoc.all.Bugs_fixed_helpSec;
    IEFormHelpHighligth(IEData, helpTag);
}

/*****************************************************************************/
/* This is called when a bug checkbox is changed.  Its job is to update the
   title to either add or remove the bugNumber from the title of the change */

function checkinTitleUpdateCallBack() {

    // logMsg(LogClrAutomation, LogInfo, "Updating titleElem\n");  
    var inputDoc = IEData.inputDoc;
    var titleElem = inputDoc.all.Title_input;
    if (!titleElem.value.match(/^Fix bugs: /i))
        return;

    var checkBox = inputDoc.parentWindow.event.srcElement;
    checkBox.id.match(/Bug_(\d+)/);
    var bugNum = RegExp.$1;

    if (checkBox.checked) {
        if (!titleElem.value.match(/:\s*$/))
            titleElem.value += ", "
        titleElem.value += bugNum;
    }
    else  {
        var newTitle = titleElem.value.replace(new RegExp("\\b\\s*" + bugNum + "\\b\\s*,?\\s*"), "");
        titleElem.value = newTitle.replace(/\s*,\s*$/, "");
        if (!titleElem.value.match(/\d+/))
            titleElem.value = "";
    }
}

/****************************************************************************/
/* given a object IE document 'IEData' look for input tags of the form 
   (.*_input) and set their values to the the cooresponding in 'initValues 
*/
function checkinFormFieldsInit(IEData, initValues) {

    // logMsg(LogClrAutomation, LogInfo, "In IEFormFieldInit values = {\n", dump(initValues), "\n}\n");
    for (var name in initValues) {
        var initValue = initValues[name];
        // logMsg(LogClrAutomation, LogInfo, "IEFormFieldInit: ", name, " ", initValue, "\n");
        if (initValue == undefined)
            continue;

        inputFieldElem = IEData.inputDoc.all[name + "_input"];
        if (inputFieldElem == undefined)  {
            // logMsg(LogClrAutomation, LogWarn, "IEFormFieldInit: ignoreing value ", name, "\n");
            continue;
        }
            
        if (inputFieldElem.length) {        // We have more than one input field (radio buttons)
            for(var i = 0; ; i++) {
                if (i >= inputFieldElem.length)  {
                    logMsg(LogClrAutomation, LogWarn, "Field: ", name, " has value ", initValue, " which is not valid value, ignoring\n");
                    break;
                }
                var radioButton = inputFieldElem[i];
                if (radioButton.type != "radio")
                    logMsg(LogClrAutomation, LogError, "Multiple input elements have name ", name, " and are not radio buttons\n");

                // logMsg(LogClrAutomation, LogInfo, "Got radio '", radioButton.value, "' have value '", initValue, "'\n");
                if (radioButton.value == initValue) {
                    radioButton.checked = true;
                    break;
                }
            }
        }
        else  {
            inputFieldElem.value = initValue;
            if (inputFieldElem.checked == false) 
                inputFieldElem.checked = true;
        }
    }
}

/****************************************************************************/
/* given an internet explorer document object, walk all the elements looking
   for tags with the ID of (.*_input), and collect their values into a object
   and return it.  Since the order of these elements in the form may be
   important the returned obect also has the field '_ORDER' which is a array
   of all the fields that it found 
*/
function checkinFormFieldsGather(IEData) {

    var ret = {};
    var order = ret._ORDER = [];
    for (var i = 0; i < IEData.inputDoc.all.length; i++) {
        var inputElement = IEData.inputDoc.all[i];
        var id = inputElement.id

        if (!id.match(/^(\w+)_input$/))
            continue;
        var name = RegExp.$1;
        if (ret[name] != undefined)        // you get this with radio buttons
            continue;

        if (inputElement.type == "radio" || inputElement.type == "checkbox") {
            if (inputElement.checked) 
                ret[name] = inputElement.value;    
        }
        else 
            ret[name] = inputElement.value;

        if (ret[name] != undefined)
            order.push(name);
    }
    return ret;
}

/******************************************************************************/
/* prints out the object 'obj' as a string of the form.  fields with empty
   strings are not printed.  

     <name of field1>: 
        <multi-line field data1>

     <name of field2>: 
        <multi-line field data2>
*/
function humanTextPrint(obj, fieldsToPrint, noWrapFields) {

    // logMsg(LogClrAutomation, LogInfo, "In humanTextprint\n");
    var out = ""
    for (var i = 0; i < fieldsToPrint.length; i++) {
        var name = fieldsToPrint[i];
        var value = obj[name];
        // logMsg(LogClrAutomation, LogInfo, "humanTextprint: name ", name, " value ", value, "\n");

        if (value != undefined && value != "") {
            value = value.replace(/\s*$/g, "");
            var printName = name.replace(/_/g, " ");
            out += printName + ":\r\n";

			value = value.replace(/\r*/, "");
			if (noWrapFields && noWrapFields[name]) {
				out += "  " + value.replace(/\n/g, "\n  ") + "\r\n";
			}
			else {
				var lines = value.split("\n");
				for (var j = 0; j < lines.length; j++) {
				    var preferredWidth = adjustForSDTrackTag(lines[j], 75);
					out += splitToFit("  " + lines[j], preferredWidth, 2) + "\r\n";
			}
			}
            out += "\r\n";
        }
    }
    return out;
}

/******************************************************************************/
/* this is used by humanTextPrint.  for cases where we want to wrap the text
   we cannot wrap the text in the middle of an SDTrack tag.  this function
   takes the text and the proposed split width and checks to see if the split
   would end up inside an SDTrack tag.  if it does then adjust the split width
   so the split would occur before the SDTrack tag. */
function adjustForSDTrackTag(text, index) {

    if (text.length < index)
        return index;

    if (checkForAnySDTrackTag(text, false)) {
    
        if (index > RegExp.index && index < RegExp.lastIndex) {
        
            index = RegExp.index - 1;
                
            return index;
        }
        else {
            return index;
        }
    }
    else {
        return index;
    }
}

/******************************************************************************/
/* takes 'str' which is assumed to be a multi-line string of the form 
    
     <name of field1>: 
        <multi-line field data1>

     <name of field2>: 
        <multi-line field data2>


    and returns it as an object with field names given above (Normalized
    capitalization (First lettter capitalized, the rest lower case).  This
    was meant as a compromize between human and machine readability. 

    defaultField is the name of the field that default text (text before
    the first parsed field), is to go.  If it is undefined, this text
    is ignored.
    
    if retObj is defined, it is the object the fields go into.
*/

function humanTextParse(str, defaultField, retObj) {
    
    if (retObj == undefined)
        retObj = {};

    var fieldName = defaultField
    lines = str.split("\n");
    for (var i = 0; i < lines.length; i++) {
        var line = lines[i];

        if (line.match(/^(\w[\w\d ]+):(.*)/)) {
            fieldName = RegExp.$1;
            fieldName = fieldName.replace(/\s+/g, "_");
            fieldName = fieldName.substr(0, 1).toUpperCase() + fieldName.substr(1).toLowerCase();
            line = RegExp.$2;
        }

        if (!line.match(/^\s*(.*\S)\s*$/))        // remove blank lines and leading and trailing blanks
            continue;
        line = RegExp.$1;

        if (fieldName != undefined) {
            if (retObj[fieldName] == undefined)
                retObj[fieldName] = line;
            else 
                retObj[fieldName] = retObj[fieldName] + "\r\n" + line;
        }
    }
    return retObj;
}

