/*********************************************************************************/
/*                               productStudio.js                                */
/*********************************************************************************/
/*********************************************************************************/

/* utilities for interfacing with productStudio (creating and fetchin bugs) */

// AUTHOR: Vance Morrison 
// DATE: 11/1/2003

/*********************************************************************************/

var produtStudioModuleDefined = 1;         // Indicate that this module exist
if (!logModuleDefined) throw new Error(1, "Need to include log.js");

var LogProductStudio = logNewFacility("productStudio");

if (WshShell == undefined)
    var WshShell = WScript.CreateObject("WScript.Shell");
if (Env == undefined)
    var Env = WshShell.Environment("PROCESS");

/********************************************************************************/
/* Connect to a particular Product Studio database.  This is the first step in
   making or querying about bugs in that database.  It returns a psDataStore 
   which can be used for making bugs or querying 

     Parameters:
       dbName : Name of database to connect ot (eg. DevDiv Schedule, VSWhidbey)
*/
function bugConnect(dbName) {
    logMsg(LogProductStudio, LogInfo10, "bugConnect(", dbName, ")\n");
    var psDirectory = undefined;
    try { psDirectory = WScript.CreateObject("ProductStudio.Directory"); } catch(e) {}
    if (psDirectory == undefined)
        throw Error(1, "Could not find 'ProductStudio.Directory' COM object (Product Studio installed?)");

    psDirectory.Connect();
    var psProduct = psDirectory.GetProductByName(dbName);
    var psDataStore = psProduct.Connect();
    return psDataStore;
}

/****************************************************************************/
/**************************   QUERY OPERATORS   *****************************/
/****************************************************************************/

/****************************************************************************/
/* Given psDataStore (which you get from calling bugConnect), and a query 
   (See http://bgit/applications/help/productstudiosdk/default.asp)
   return an array of psDataItem, each of which represents one bug.  The 
   fields of this bug can then be fetched by using bugFieldGet.
   The returned data list is read-only. It can be made writeable by calling 
   bugEdit.  Once writeable, items can also be updated using bugFieldSet, 
   and saved used bugSave.
   
   Do runjs /? bugQ for a listing of methods used to create queryXml in
   jscript relatively painlessly.  Do bugDumpFieldValue to see the list
   of legal values for a field.

     Parameters:
       psDataStore   : A handle returned from bugConnect 
       queryXml      : The query (in XML)  (See examples below and PS docs)
*/

function bugQuery(psDataStore, queryXML) {

        // TODO this is half baked (don't give back info nicely, don't allow updates ...)
    logMsg(LogProductStudio, LogInfo100, "bugQuery(psDataStore, queryXML, fieldsDesired)\n");

    if (!queryXML.match(/^\s*<query>(.|\s)*<\/query>\s*/i))
        queryXML = "<query>" + queryXML + "</query>";

        // put in { } so that we pretty print nicely
    logMsg(LogProductStudio, LogInfo100, "bugQuery: (ignore {}) queryXML = ({\n",
        queryXML.replace(/(<\w.*?>)/gm, "$1 {\n").replace(/(<[\/].*?>)/gm, "\n} $1"), "\n}\n");


    var psQuery = WScript.CreateObject("ProductStudio.Query");
    psQuery.SelectionCriteria = queryXML;
    psQuery.CountOnly = false;

    /*** If we want, you can limit the number of fields returned, but it is not presently worth it
    if (fieldsDesired != undefined) {
        logMsg(LogProductStudio, LogInfo, "bugQuery: Selecting particular fields\n");
        psQuery.QueryFields.Clear();
        for (var i = 0; i < fieldsDesired.length; i++) {
            var fieldDesired = fieldsDesired[i];
            logMsg(LogProductStudio, LogInfo, "bugQuery: Selecting field ", fieldDesired, "\n");
            psQuery.QueryFields.Add(psFieldDefs.Item(fieldDesired));
        }
    }
    ****/

        
        //  Bind the data list to our query.
    var psDataList = WScript.CreateObject("ProductStudio.DataStoreItemList");
    psDataList.DataStore = psDataStore; 
    psDataList.Query = psQuery;

    psDataList.Execute();
    logMsg(LogProductStudio, LogInfo100, "bugQuery: Count = ", psDataList.DataStoreItems.Count, "\n");

    var ret = [];
    for (var i = 0; i < psDataList.DataStoreItems.Count; i++) { 
        var item = psDataList.DataStoreItems.Item(i);
        logMsg(LogProductStudio, LogInfo10000, "bugQuery: Bug ", item.Fields("ID").Value, " ", item.Fields("Title").Value, "\n");
        ret.push(item);    
    }
    return ret;
}

/*****************************************************************************/
/* given a bug number, fetch bug by its bug number

   Parameters:
      bugDb   : Handle returned from bugConnect
      bugNum  : Bug number to edit
*/

function bugGetById(bugDb, bugNum) {

    results = bugQuery(bugDb, bugQEquals("ID", bugNum));
    if (results.length != 1)
        throw Error(1, "Could not find bug with number " + bugNum);
    return results[0];
}

/*******************************************************************************/
/* returns a query string that specifieds 'field' should be equal to 'value' */

function bugQEquals(field, value) {
    return bugQOp("equals", field, value);
}

/*******************************************************************************/
/* returns a query string that specifieds 'field' should be 'relop' to 'value'.
   relop can be any product studio relational operator (equals, less than, 
   contains, ...)  */

function bugQOp(relOp, field, value) {
    var ret = "<Expression Column='" + field + "' Operator='" + relOp + "'> " +
                  "<String> " + value + " </String> "  +
              "</Expression> ";
    return ret;
}

/*******************************************************************************/
/* returns a query string that the 'and' of a list of query strings */

function bugQAnd(exp1, exp2 /* ... */) {

    var ret = "<Group GroupOperator='and'> ";
    for(var i = 0; i < arguments.length; i++) {
        ret += arguments[i];
        ret += " ";
    }
    ret += "</Group>";
    return ret;
}

/*******************************************************************************/
/* returns a query string that the 'or' of a list of query strings */

function bugQOr(exp1, exp2 /* ... */) {

    var ret = "<Group GroupOperator='or'> ";
    for(var i = 0; i < arguments.length; i++) {
        ret += arguments[i];
        ret += " ";
    }
    ret += "</Group>";
    return ret;
}

/*****************************************************************************/
/* Gets the value of a field named 'field'  from a bug represented by 
   'psDataItem'.  Use bugDumpFields to get a list of valid field names

     Parameters:
       psDataStore : A handle returned from bugConnect 
       psDataItem  : A handle for the bug you want set 
       field:        The string name of the field to fetch.
*/
function bugFieldGet(psDataStore, psDataItem, field) {

    var ret;
    var psFields = psDataItem.Fields;
    if (field == "Path") {
        // TODO handle the case when 'field' 
        throw Error(1, "Not Implemented, fetching the path of the bug");
    }
    else 
        ret = psFields.Item(field).Value;
    logMsg(LogProductStudio, LogInfo1000, "bugFieldGet(psDataItem, ", field, ") = ", ret, "\n");
    return ret;
}

/****************************************************************************/
/**************************    EDIT OPERATORS   *****************************/
/****************************************************************************/

/******************************************************************************/
/* create a bug in the product studio database refered to by 'psDataStore'
   (which you get from bugConnect).  It rreturns a 'psDataItem' which 
   represents a blank bug.  To finish the bug, use hte bugFieldSet to update 
   fields and then call bugSave to commit the change 

     Parameters:
       psDataStore : A handle returned from bugConnect 

*/
function bugCreate(psDataStore) {

    logMsg(LogProductStudio, LogInfo100, "bugCreate()\n");
    var psDataList = WScript.CreateObject("ProductStudio.DataStoreItemList");
    psDataList.DataStore = psDataStore;

    var psDataStoreItemTypeBugs = -100;
    psDataList.CreateBlank(psDataStoreItemTypeBugs);
    var psDataItem = psDataList.DataStoreItems.Add();

    return psDataItem;
}

/*****************************************************************************/
/* Opens the bug for editing. This must be done on bugs obtained by a
   bugQuery on which we want to use bugFieldSet and bugSave.

     Parameters:
       psDataItem  : A handle for the bug you want set 
       editType    : Edit kind. (def) "edit", "resolve", "close", "readOnly"
*/

function bugEdit(psDataItem, editType) {

    if (editType == undefined)
        editType = "edit"
    
    logMsg(LogProductStudio, LogInfo1000, "bugEdit(psDataItem)\n");

    var code = 0;
    if (editType != "edit") {
        if (editType == "readOnly")
            code = 1;     // psBugEditActionReadOnly 
        else if (editType == "activate")
            code = 2;     // psBugEditActionActivate 
        else if (editType == "resolve")
            code = 3;     // psBugEditActionResolve
        else if (editType == "close")
            code = 4;     // psBugEditActionClose
        else
            throw new Error(1, "Unknown edit type " + editType);
    }

    psDataItem.Edit(code);
}

/*****************************************************************************/
/* Sets the value of a field named 'field'  from a bug represented by 
   'psDataItem'  to the value 'value'
    The dataItem must be readable. That can be from bugCreate or from bugEdit.

     Parameters:
       psDataStore : A handle returned from bugConnect 
       psDataItem  : A handle for the bug you want set 
       field:        The string name of the field to fetch.
       value:        The value to update the field to
*/
function bugFieldSet(psDataStore, psDataItem, field, value) {

    logMsg(LogProductStudio, LogInfo1000, "bugFieldSet(psDataItem, ", field, ", ", value, ")\n");
    var psFields = psDataItem.Fields;
    if (field == "Path") {
        psFields.Item("TreeID").Value = _bugTreeIDFromPath(psDataStore.RootNode, value);
    }
    else {
        psFields.Item(field).Value = value;
    }
}

/*****************************************************************************/
/* Attach a file to a bug 

   Parameters:
     psDataStore : A handle returned from bugConnect 
     psDataItem  : A handle for the bug you want set 
     fileName    : The name of the file to attach.
*/
function bugAddFile(psDataStore, psDataItem, fileName) {
    logMsg(LogProductStudio, LogInfo1000, "bugAddFile(psDataItem, ", fileName, ")\n");    
    psDataItem.Files.Add(fileName);
}

/*****************************************************************************/
/* Save a bug represented by psDataItem (which you get by calling bugQuery).  
   Thus after making a query, you can update fields and call this routine 
   to save it 

     Parameters:  
       psDataItem  : A handle for the bug you want set 
*/
function bugSave(psDataItem) {

    logMsg(LogProductStudio, LogInfo100, "bugSave(psDataItem): validating fields\n");
    var validityReport = "";
    var psFieldStatusValid = 0;
    for(var field in psDataItem.Fields) {
        if (field.Validity != psFieldStatusValid) 
            validityReport += "Field '" + field.Name + "' with value '" + field.Value + "' invalid\r\n";
    }
    if (validityReport != "") 
        throw Error(1, "Error validating bug\r\n" + validityReport);

    logMsg(LogProductStudio, LogInfo100, "bugSave: saving\n");
    psDataItem.Save(1);
}

/************************************************************************************/
/* Product studio does not just store the path for category associated with a bug
   Instead each path is given a node ID (this seems like an unnecessary complication
   at least for the automation).  This starts with the root node and given a string
   path reutrns the node id for that path (or fails in the attmpt 
*/

function _bugTreeIDFromPath(node, path)  {

    logMsg(LogProductStudio, LogInfo10000, "_bugTreeIDFromPath(node, ", path, ")\n");
    var elems = path.split("\\");
    for (var i = 0; i < elems.length; i++) {
        var elem = elems[i];
        logMsg(LogProductStudio, LogInfo10000, "_bugTreeIDFromPath: elem: ", elem, "\n");
        node = node.Nodes.Item(elem);
    }

    var ret = node.ID;
    logMsg(LogProductStudio, LogInfo10000, "_bugTreeIDFromPath = ", ret, "\n");
    return ret
}

/****************************************************************************/
/**************************       VALUE ADD       ***************************/
/****************************************************************************/

/****************************************************************************/
/* given a list of bug numbers (can be a list or a string of comma/space
   separated list).  resolve all the bugs as fixed by changeNum.  It returns
   the number of failures it encountered.

   Parameters
     bugNums   : Product studio bug numbers (list or comma sep list)
     changeNum : Source depot change number
     dbName    : Product studio database name (eg VSWhidbey)
     depotName : Produce Studio name for depot (eg DevDiv Sources)
     branchName  : The full name of the branch changeNum was checked in (eg CLR Post Orcas)
     sdSpec    : Source depot connection spec (see sdConnect) (eg DDRTSD:4000)
*/

function bugsResolveFixedByChange(bugNums, changeNum, dbName, branchName, depotName, sdSpec) {

    if (bugNums == undefined)
        throw new Error(1, "Required arg 'bugNums' not present"); 
    if (changeNum == undefined)
        throw new Error(1, "Required arg 'changeNum' not present"); 

    logMsg(LogProductStudio, LogInfo, "bugsResolveFixedByChange('", bugNums, "',", changeNum, ") {\n");
    if (typeof(bugNums) == "string") {
        if (bugNums.match(/^\s*$/))
            bugNums = [];
        else 
            bugNums = bugNums.split(/(\s+|\s*,\s*)/);
    }

    var ret = 0;
    for(var i = 0; i < bugNums.length; i++) {
        var bugNum = bugNums[i];
        logMsg(LogProductStudio, LogInfo, "Processing bug ", bugNum, " {\n");
        try {
            bugResolveFixedByChange(bugNum, changeNum, dbName, branchName, depotName, sdSpec);
        } catch(e) {
            logMsg(LogSourceDepot, LogError, "bugsResolvedFixedByChange: EXCEPTION ", e.description, "\n");
            ret++;
        }
        logMsg(LogProductStudio, LogInfo, "}\n");
    }
    logMsg(LogProductStudio, LogInfo, "}\n");
    return ret;
}

/****************************************************************************/
/* given a bug number resolve as fixed and indicate that it was fixed by 
   changeNum

   Parameters
     bugNum    : Product studio bug number
     changeNum : Source depot change number
     dbName    : Product studio database name (eg VSWhidbey)
     depotName : Produce Studio name for depot (eg DevDiv Sources)
     branchName  : The full name of the branch changeNum was checked in (eg CLR Post Orcas)
     sdSpec    : Source depot connection spec (see sdConnect) (eg DDRTSD:4000)
*/

function bugResolveFixedByChange(bugNum, changeNum, dbName, branchName, depotName, sdSpec) {
    logMsg(LogProductStudio, LogInfo, "bugResolveFixedByChange('" + bugNum + "', '" + changeNum + "', '" + dbName + "', '" + branchName + "', '" + depotName + "', '" + sdSpec + "')");
    if (bugNum == undefined)
        throw new Error(1, "Required arg 'bugNum' not present"); 
    if (changeNum == undefined)
        throw new Error(1, "Required arg 'changeNum' not present"); 
    if (dbName == undefined)
        dbName = "DevDiv Bugs";
    if (branchName == undefined)
        branchName = "CLR Post Orcas";

    if (depotName == undefined)
        depotName = "DevDiv Sources";

    var branchRegex;
    if (branchName == "CLR Post Orcas")
        branchRegex = new RegExp("^(?:puclr|pu\/clr|(?:CLR Post Orcas))", "i");
    else
        branchRegex = new RegExp("^" + branchName + "\\b", "i");

    logMsg(LogProductStudio, LogInfo, "Attempting to Resolving bug ", bugNum, " as fixed by change ", changeNum, "\n");
    var bugDb = bugConnect(dbName);
    var bug = bugGetById(bugDb, bugNum, "resolve");

    var status = bugFieldGet(bugDb, bug, "Status");
    if (!status.match(/active/i))
        throw new Error(1, "Error: bug " + bugNum + " has status " + status + " != Active");

    var sdObj = sdConnect(sdSpec);
    var sdChangeInfo = sdDescribe(changeNum, sdSpec);
    // logMsg(LogProductStudio, LogInfo, "SdDescribe {\n", dump(sdChangeInfo), "\n}\n");

    var user = sdChangeInfo.user.replace(/.*\\/, "").toLowerCase();

    var assignedTo = bugFieldGet(bugDb, bug, "Assigned To");
    if (assignedTo.toLowerCase() != user)
    {
        logMsg(LogProductStudio, LogInfo, "Assigned To = " + assignedTo + " is not submitter = " + user + "\n");
    }

    var openBranch = bugFieldGet(bugDb, bug, "Opened Branch");

        // only resolve if the bugs was opened in the VBL we are checking into
    var resolveBug = true;
    if (openBranch.match(branchRegex))
        bugEdit(bug, "resolve");
    else  {
        logMsg(LogProductStudio, LogInfo, "Open Branch  = ", openBranch, " is not for branch ", branchName, " will mark bug as \"Blocked by Integration\"\n");
        bugEdit(bug, "edit");
        resolveBug = false;
        bugFieldSet(bugDb, bug, "Sub Status", "Blocked by Integration");
    }

    var changeInfo = bugFieldGet(bugDb, bug, "ChangeListInfo");
    // logMsg(LogProductStudio, LogInfo, "ChangeListInfo: {\n", changeInfo, "\n}\n");
    
    if (changeInfo == undefined)
        changeInfo = "";
    else 
        changeInfo += "\r\n";

    // Add the new information.  It is comma separated fields, newline separted records
    // Fields : changeNum, depotName, changeType, user, date, descr

    var date = new Date(sdChangeInfo.time);
    var dateStr = (date.getMonth()+1) + "/" + date.getDate() + "/" + date.getYear() + " " + 
                  date.getHours() + ":" + date.getMinutes() + ":" + date.getSeconds();

    changeInfo += changeNum + ",";
    changeInfo += depotName + ",";
    changeInfo += "Fixed,";
    changeInfo += sdChangeInfo.user + ","
    changeInfo += dateStr + ",";
    changeInfo += sdChangeInfo.desc.replace(/\s*\n(.|\s)*/m, "");

    // logMsg(LogProductStudio, LogInfo, "New ChangeListInfo: {\n", changeInfo, "\n}\n");

    bugFieldSet(bugDb, bug, "ChangeListInfo", changeInfo);

    var description = "Updated by runjs bugResolveFixedByChange " + bugNum + " " + changeNum + "\r\n\r\n" + sdChangeInfo.desc;
    if (resolveBug) {
            // Compute the specification for the lab we checked into
        var tomorrow = new Date();
        tomorrow.setDate(tomorrow.getDate()+1);
        var buildSpec = (tomorrow.getYear() % 10) + 
                        padLeft(tomorrow.getMonth()+1, 2, true) + 
                        padLeft(tomorrow.getDate(), 2, true) + ".00";

        logMsg(LogProductStudio, LogInfo, "Resolved Build ", buildSpec, "\n");

        bugFieldSet(bugDb, bug, "Resolved Branch", branchName)
        bugFieldSet(bugDb, bug, "Resolution", "Fixed");
        bugFieldSet(bugDb, bug, "Fix Build", buildSpec);
        //Don't use the user since it might not match.  Just resolve it as assignedTo.
        bugFieldSet(bugDb, bug, "Resolved By", assignedTo);
    }
    else {
        description += "\r\nDid not resolve the bug because it was opened in another VBL\r\n";
                           "Bug opened in branch " + openBranch + " but resolved in " + branchName + "\r\n";
                           "Bug was marked as \"Blocked by Integration\" instead\r\n";
    }
    bugFieldSet(bugDb, bug, "Description", description);
    bugSave(bug);
    
    if (user != assignedTo)
    {
        logMsg(LogProductStudio, LogInfo, "Successuflly ", resolveBug ? "Resolved" : "Edited", " bug ", bugNum, " for user ", user, " as user ", assignedTo , "\n");
    }
    else
    {
        logMsg(LogProductStudio, LogInfo, "Successuflly ", resolveBug ? "Resolved" : "Edited", " bug ", bugNum, " for user ", user, "\n");
    }
    return 0;
}

/****************************************************************************/
/**************************    SPECIALIZED OPS    ***************************/
/****************************************************************************/

/************************************************************************************/
/* Get the history for a bug
     
     Parameters:
       bug: the bug to get the history for.
*/
function _getBugHistory(bug) {

   bug.Edit(1 /* psDatastoreItemEditActionReadOnly */);
   return bug.History;
}

/************************************************************************************/
/* Get date from query (input time is in UTC but for some reason marked as PST)

     Parameters:
         date: the date from product studio that needs the conversion from PST to UTC
*/
function _getDate(date) {

    var s = "" + date;
    return new Date(s.replace("PST", "UTC"));
}

/************************************************************************************/
/* Formats a date as xx/xx/xxxx

     Parameters:
         date: the date to format
*/
function _formatDate(date) {

    return "" + (date.getMonth() + 1) + "/" + date.getDate() + "/" + date.getYear();
}


/*****************************************************************************/
/* Gets the value of a field named 'field'  from a bug represented by 
   'psDataItem' at a past date.

     Parameters:
       psDataStore : A handle returned from bugConnect 
       psDataItem  : A handle for the bug you want set 
       field:        The string name of the field to fetch.
       date:         The date at which we want to get this value
*/
function _bugFieldGetAtDate(psDataStore, psDataItem, field, date) {

    var ret;
    var psFields = psDataItem.Fields;
    if (field == "Path") {
        // TODO handle the case when 'field' 
        throw Error(1, "Not Implemented, fetching the path of the bug");
    }
    else {
        var history     = _getBugHistory(psDataItem);
        var ret         = bugFieldGet(psDataStore, psDataItem, field);
        var changedDate = bugFieldGet(psDataStore, psDataItem, "Changed Date");
        
        if ((date.valueOf() > (_getDate(changedDate)).valueOf())) {
            // The last change to this bug occured before the passed in date.  We
            // can just return the current field value (guaranteed not to have changed).
            return ret;
        }

        for (var j = 0; j < history.Count; j++) {
            var bugDelta     = history.Item(j);
            changedDate      = bugFieldGet(psDataStore, bugDelta, "Changed Date");
            var changedField = bugFieldGet(psDataStore, bugDelta, field);
            if (changedField) {
                ret = changedField;
            }
            
            if ((date.valueOf() > (_getDate(changedDate)).valueOf())) {
                // bugDelta is older than date
                return ret;
            }
        }

        return undefined;
    }

    logMsg(LogProductStudio, LogInfo1000, "bugFieldGet(psDataItem, ", field, ") = ", ret, "\n");
    return ret;
}


/*****************************************************************************/
/* Formats a bug into a string

     Parameters:
       bugDb -- the bug database containing the bug
       bug -- the bug to format
*/
function _formatBug(bugDb, bug) {

    str = "<TR>";
    var bugID = bugFieldGet(bugDb, bug, "ID");
    str += "<TD>";
    str += "<A href=\"http://vbqa/raid/BugForm.aspx?sBugDB=VSWhidbey&amp;lBugID=" + bugID + "\">" + bugID + "</A>";  
    str += "</TD> <TD>";
    str += bugFieldGet(bugDb, bug, "Priority");
    str += "</TD> <TD>";
    substatus = "" + bugFieldGet(bugDb, bug, "SubStatus");
    if (substatus == "undefined") {
        substatus = " ";
    }
    str += substatus;
    str += "</TD> <TD>";
    str += bugFieldGet(bugDb, bug, "Title");
    str += "</TD> </TR>\n";

    return str;
}


/*****************************************************************************/
/* Gets the list of bugs assigned to a dev

     Parameters:
       devAlias -- the dev that owns the bug
*/
function _getAssignedToList(devAlias) {

    logMsg(LogProductStudio, LogInfo100, "_getAssignedToList(" + devAlias + ")\n");
    
    var ret = [];
    var bugDb = bugConnect("VSWhidbey");
    var queryXML =  "<Query> " + 
                        "<Group GroupOperator='and'> " +
                          "<Expression Column='Assigned to' Operator='equals'> " +
                            "<String> " + devAlias + " </String> "  +
                          "</Expression> "  +

                          "<Expression Column='Status' Operator='equals'> " +
                            "<String>Active</String> "  +
                          "</Expression> "  +
                      
                          "<Expression Column='Fix By' Operator='equals'> " +
                            "<String>Whidbey RTM RC</String> "  +
                          "</Expression> "  +
        
                        "</Group> " +
                      "</Query> ";

    var results = bugQuery(bugDb, queryXML);

    for (var i = 0; i < results.length; i++) {
        var result = results[i];
        var str = _formatBug(bugDb, result);

        ret.push(str);
    }

    return ret;
}


/*****************************************************************************/
/* Gets the list of bugs assigned to a dev at some time in the past

     Parameters:
       devAlias -- the dev that owns the bug
       date -- the date at which the bugs were assigned to the dev
*/
function _getAssignedToListAtDate(devAlias, date) {

    logMsg(LogProductStudio, LogInfo100, "_getAssignedToListAtDate(" + devAlias + ", " +date + ")\n");

    var ret = [];
    var bugDb = bugConnect("VSWhidbey");
    var queryXML =  "<Query> " + 
                        "<Group GroupOperator='and'> " +
                          "<Expression Column='Assigned to' Operator='ever'> " +
                            "<String> " + devAlias + " </String> "  +
                          "</Expression> "  +
                        "</Group> " +
                      "</Query> ";

    var results = bugQuery(bugDb, queryXML);

    for (var i = 0; i < results.length; i++) {
        var result = results[i];

        var assignedToAtDate = _bugFieldGetAtDate(bugDb, result, "Assigned to", date);
        if (assignedToAtDate != devAlias) {
            continue;
        }

        var statusAtDate = _bugFieldGetAtDate(bugDb, result, "Status", date);
        if (statusAtDate != "Active") {
            continue;
        }
        
        var fixByAtDate = _bugFieldGetAtDate(bugDb, result, "Fix By", date);
        if (fixByAtDate != "Whidbey RTM RC") {
            continue;
        }

        var str = _formatBug(bugDb, result);

        ret.push(str);
    }

    return ret;
}

/*****************************************************************************/
/* Gets the number of bugs assigned to devs in the past

     Parameters:
       devAlias -- the dev that owns the bug
       numDays -- number of days to get history for
*/
function _getAssignedToCountHistory(devAlias, numDays) {
    
    var today = new Date();
    var date = new Date(today.valueOf() - (1000 * 3600 * 24 * numDays));

    for (var i = 0; i < numDays; i++) {
        var assignedToList = _getAssignedToListAtDate(devAlias, date);

        logMsg(LogProductStudio, LogInfo, "Date: " + date + "  Number of bugs = " + assignedToList.length + "\n");
        date = new Date(date.valueOf() + 1000 * 3600 * 24);
    }

    return 0;
}

/*****************************************************************************/
/* Gets the list of bugs resolved by a dev after a given date

     Parameters:
       devAlias -- the dev that owns the bug
       date -- the date at which the bugs were assigned to the dev
*/
function _getResolvedByList(devAlias, date) {

    logMsg(LogProductStudio, LogInfo100, "_getResolvedByList(" + devAlias + ", " +date + ")\n");
    
    if (date == undefined) {
        var today = new Date();
        date = new Date(today.valueOf() - (1000 * 3600 * 24 * 10));
    }

    var ret = [];
    var bugDb = bugConnect("VSWhidbey");
    var queryXML =  "<Query> " + 
                        "<Group GroupOperator='and'> " +
                          "<Expression Column='Resolved by' Operator='equals'> " +
                            "<String> " + devAlias + " </String> "  +
                          "</Expression> "  +
                        "</Group> " +
                      "</Query> ";

    var results = bugQuery(bugDb, queryXML);

    for (var i = 0; i < results.length; i++) {
        
        var result = results[i];
        var resolvedDate = bugFieldGet(bugDb, result, "Resolved date");

        if ((_getDate(resolvedDate).valueOf()) > date.valueOf()) {
            // Bug is more recent than date

            var str = _formatBug(bugDb, result);
            ret.push(str);
        }
    }

    return ret;
}
      
/*****************************************************************************/
/* Gets the list of incoming bug for a dev after a given date

     Parameters:
       devAlias -- the dev that owns the bug
       date -- the date at which the bugs were assigned to the dev
*/
function _getIncomingList(devAlias, date) {

    logMsg(LogProductStudio, LogInfo100, "_getDispatchedByList(" + devAlias + ", " +date + ")\n");

    if (date == undefined) {
        var today = new Date();
        date = new Date(today.valueOf() - (1000 * 3600 * 24 * 7));
    }

    var ret = [];
    var bugDb = bugConnect("VSWhidbey");
    var queryXML =  "<Query> " + 
                      "<Group GroupOperator='and'> " +
                        "<Expression Column='Assigned to' Operator='ever'> " +
                          "<String> " + devAlias + " </String> "  +
                        "</Expression> "  +
                       "</Group> " +
                    "</Query> ";

    var results = bugQuery(bugDb, queryXML);

    for (var i = 0; i < results.length; i++) {
        var result = results[i];

        var assignedToDev = false;
        var assignedToDevDate;
        var bugStatus  = bugFieldGet(bugDb, result, "Status"); 
        var assignedTo = bugFieldGet(bugDb, result, "Assigned To"); 

        //
        // First we need to deal with boundary cases 
        //

        // Case 1:  bug hasn't changed in past week--this means that it could
        // not have been assigned to the dev in the past week.  Our logic below
        // assumes that the first "implicit" delta occured in the past week because
        // there is no other information on it (this is in fact the invariant of the loop,
        // that is, that the previous change occured in the past week).
        var changedDate = bugFieldGet(bugDb, result, "Changed Date");
        if (_getDate(changedDate).valueOf() < date) {
            // Bug hasn't changed since date.  Nothing interesting here.
            continue;
        }

        // Case 2: Bug was opened in the past week, and has no other history
        var openedDate   = bugFieldGet(bugDb, result, "Opened Date");
        if (_getDate(openedDate).valueOf() >= date) {
            // bug was opened after date and was at some point assigned to dev
            assignedToDev = true;
        }

        var history = _getBugHistory(result);

        for (var j = 0; j < history.Count; j++) {
            var bugDelta = history.Item(j);

            var deltaAssignedTo   = bugFieldGet(bugDb, bugDelta, "Assigned To");
            var deltaChangedDate  = bugFieldGet(bugDb, bugDelta, "Changed Date");
            var deltaBugStatus = bugFieldGet(bugDb, bugDelta, "Status");

            // Look for a transition where a bug was not previously assigned to the
            // dev and is now assigned to the dev
            if (deltaAssignedTo &&
                assignedTo == devAlias &&
                bugStatus  == "Active") {

                assignedToDev = true;
            }

            // Update field values to reflect that point in time
            if (deltaAssignedTo) {
                assignedTo = deltaAssignedTo;
            }
            if (deltaBugStatus) {
                bugStatus = deltaBugStatus;
            }

            if (_getDate(deltaChangedDate).valueOf() < date) {
                // This delta is already the one just before this time period
                break;
            }
        }

        if (assignedToDev) {
                
            // One last filter...  If the bug was assigned to the dev at the
            // beginning of the time period, then it's not really an incoming
            // bug.

            if ((_bugFieldGetAtDate(bugDb, result, "Assigned to", date) == devAlias) &&
                (_bugFieldGetAtDate(bugDb, result, "Status", date) == "Active")) {

                continue;
            }

            var str = _formatBug(bugDb, result);
            ret.push(str);
        }
    }

    return ret;
}

/*****************************************************************************/
/* Gets the list of bugs dispatched by a dev after a given date

     Parameters:
       devAlias -- the dev that owns the bug
       date -- the date at which the bugs were assigned to the dev
*/
function _getDispatchedByList(devAlias, date) {

    logMsg(LogProductStudio, LogInfo100, "_getDispatchedByList(" + devAlias + ", " +date + ")\n");

    if (date == undefined) {
        var today = new Date();
        date = new Date(today.valueOf() - (1000 * 3600 * 24 * 7));
    }

    var ret = [];
    var bugDb = bugConnect("VSWhidbey");
    var queryXML =  "<Query> " + 
                      "<Group GroupOperator='and'> " +
                        "<Expression Column='Assigned to' Operator='ever'> " +
                          "<String> " + devAlias + " </String> "  +
                        "</Expression> "  +

                        "<Expression Column='Assigned to' Operator='notEquals'> " +
                          "<String> " + devAlias + " </String> "  +
                        "</Expression> "  +

                        "<Expression Column='Resolved by' Operator='notEquals'> " +
                          "<String> " + devAlias + " </String> "  +
                        "</Expression> "  +
                       "</Group> " +
                    "</Query> ";

    var results = bugQuery(bugDb, queryXML);

    for (var i = 0; i < results.length; i++) {
        var result = results[i];

        var assignedToDev = false;
        var assignedToDevDate;
        var bugStatus  = bugFieldGet(bugDb, result, "Status"); 
        var assignedTo = bugFieldGet(bugDb, result, "Assigned To"); 

        //
        // First we need to deal with boundary cases 
        //

        // Case 1:  bug hasn't changed in past week--this means that it could
        // not have been assigned to the dev in the past week.  Our logic below
        // assumes that the first "implicit" delta occured in the past week because
        // there is no other information on it (this is in fact the invariant of the loop,
        // that is, that the previous change occured in the past week).
        var changedDate = bugFieldGet(bugDb, result, "Changed Date");
        if (_getDate(changedDate).valueOf() < date) {
            // Bug hasn't changed since date.  Nothing interesting here.
            continue;
        }

        var history = _getBugHistory(result);

        for (var j = 0; j < history.Count; j++) {
            var bugDelta = history.Item(j);

            var deltaAssignedTo   = bugFieldGet(bugDb, bugDelta, "Assigned To");
            var deltaChangedDate  = bugFieldGet(bugDb, bugDelta, "Changed Date");
            var deltaBugStatus = bugFieldGet(bugDb, bugDelta, "Status");

            // Update field values to reflect that point in time
            if (deltaAssignedTo) {
                assignedTo = deltaAssignedTo;
            }
            if (deltaBugStatus) {
                bugStatus = deltaBugStatus;
            }

            // If the bug is assigned to the dev, it must have been dispatched in the past week
            if (assignedTo == devAlias &&
                bugStatus  == "Active") {

                assignedToDev = true;
            }

            if (_getDate(deltaChangedDate).valueOf() < date) {
                // This delta is already the one just before this time period
                break;
            }
        }

        if (assignedToDev) {
                
            var str = _formatBug(bugDb, result);
            ret.push(str);
        }
    }

    return ret;
}

/*****************************************************************************/
/* Returns a record for a given bug

     Parameters:
       description - a description of the record
       value       - the bug list of this record
*/
function _getBugRecord(description, value)
{
    var ret = {};
    ret.description = description;
    ret.value       = value;
    return ret;
}



/*****************************************************************************/
/* Gets interesting bug lists for a dev

     Parameters:
       devAlias -- the dev that owns the bug
       date -- the date at which the bugs were assigned to the dev
       today -- today's date
*/
function _getBugLists(devAlias, date, today) {

    logMsg(LogProductStudio, LogInfo100, "_getBugLists(" + devAlias + ", " +date + ")\n");

    if (date == undefined) {
        var today = new Date();
        date = new Date(today.valueOf() - (1000 * 3600 * 24 * 7));
    }
    
    var ret = [];

    ret.push(_getBugRecord("Assigned To at " + _formatDate(today),  _getAssignedToList(devAlias)));
    ret.push(_getBugRecord("Assigned To at " + _formatDate(date),   _getAssignedToListAtDate(devAlias, date)));
    ret.push(_getBugRecord("Incoming",                              _getIncomingList(devAlias, date)));
    ret.push(_getBugRecord("Resolved By",                           _getResolvedByList(devAlias, date)));
    ret.push(_getBugRecord("Dispatched By",                         _getDispatchedByList(devAlias, date)));

    return ret;
}

/*****************************************************************************/
/* Outputs array of fields as a table row in an html document

     Parameters:
       l - array of fields to tabulate
*/
function _tabulateLine(l)
{
    return "<TR><TD>" + l.join("</TD><TD>") + "</TD></TR>";
}

/*****************************************************************************/
/* Adds the bug as rows in an html table

     Parameters:
       a1 -- the output string array
       a2 -- the bug list
       
*/
function _appendBugTable(a1, a2)
{
    a1.push("<Table frame=border>\n");

    var c = [];
    c.push("ID");
    c.push("Priority");
    c.push("SubStatus");
    c.push("Title");
    a1.push(_tabulateLine(c));
    
    for (var i = 0; i < a2.length; i++) {
        a1.push(a2[i]);
    }

    a1.push("</Table>\n");

    return a1;
}

/*****************************************************************************/
/* Gets interesting bug lists for a team

     Parameters:
       devAlias -- the dev that owns the bug
       date -- the date at which the bugs were assigned to the dev
       today -- today's date
*/
function _getBugReportForTeam(devAliasList, date, today) {

    logMsg(LogProductStudio, LogInfo100, "_getBugReportForTeam(" + devAliasList + ", " + date + ", " +today +")\n");

    var devAliasArray = devAliasList.split(" ");
    var devLists = [];
    var ret = [];

    // Collect the bug data
    for (var i = 0; i < devAliasArray.length; i++) {
        var devAlias = devAliasArray[i];
        var devBugLists = _getBugLists(devAlias, date, today);
        devLists[devAlias] = devBugLists;
    }

    ret.push("<CAPTION><H2> Summary bug data for the week of " + _formatDate(date) + ": </H2></CAPTION>\n");
    ret.push("<DIV> Definitions of fields:</DIV>\n");
    ret.push("<DIV> Assigned to at " + _formatDate(today) + "- RTM RC bugs currently assigned to dev</DIV>\n");
    ret.push("<DIV> Assigned to at " + _formatDate(date) + " - RTM RC bugs assigned to dev last week at this time</DIV>\n");
    ret.push("<DIV> Incoming - bugs that got assigned to dev at some point in the past week</DIV>\n");
    ret.push("<DIV> Resolved by - bugs resolved by dev in the past week</DIV>\n");
    ret.push("<DIV> Dispatched by - bugs that were assigned to dev at some point in the past week and are no longer assigned to the dev, but that were not resolved by the dev</DIV>\n");
//    ret.push("<DIV> Note that:  Assigned To - Assigned to at <date> = Incoming - (Resolved + Dispatched) </DIV>\n");
    
    ret.push("<P><TABLE frame=border>");

    // Generate the heading table (description of the various columns)
    var l = [];
    l[0] = "Developer";
    var beanCounts = [];

    var firstBugList = devLists[devAliasArray[0]];
    for (var i = 0; i < firstBugList.length; i++) {
        l.push(firstBugList[i].description);
        beanCounts[i] = 0;
    }
    ret.push(_tabulateLine(l));
    
    // Now count the beans...    
    for (var i = 0; i < devAliasArray.length; i++) {
        var devAlias = devAliasArray[i];
        var bugList = devLists[devAlias];

        var beanList = [];
        beanList.push("<A HREF=#" + devAlias + ">" + devAlias + "</A>");
        for (var j = 0; j < bugList.length; j++) {
            beanCounts[j] += bugList[j].value.length;
            beanList.push("<A HREF=#" + devAlias + bugList[j].description + ">" +
                          bugList[j].value.length + "</A>");
        }
        ret.push(_tabulateLine(beanList));
    }

    var total = [];
    total.push("TOTAL");
    for (var j = 0; j < beanCounts.length; j++) {
        total.push(beanCounts[j]);
    }
    ret.push(_tabulateLine(total));
    ret.push("</TABLE>");

    for (var i = 0; i < devAliasArray.length; i++) {
        var devAlias = devAliasArray[i];
        var bugList = devLists[devAlias];
        ret.push("\n\n");
        ret.push("<HR color=gray noshade>");
        ret.push("<P><H3>Bug results summary for <A NAME=" + devAlias + ">" + devAlias + "</A></H3></P>\n");
        
        for (var j = 0; j < bugList.length; j++) {
            ret.push("<H4><A NAME=" + devAlias + bugList[j].description + ">" + 
                     bugList[j].description + "</A></H4>");
            ret = _appendBugTable(ret, bugList[j].value);
        }
    }

    return ret;
}

/*****************************************************************************/
/* Get the weekly bug report for a dev team

     Parameters:
       devAlias -- the dev that owns the bug
*/
function getWeeklyBugReportForTeam(devAliasList, numDays) {

    logMsg(LogProductStudio, LogInfo100, "getWeeklyBugReportForTeam(" + devAliasList + ")\n");

    if (numDays == undefined) {
        numDays = 7;
    }
    var today = new Date();
    var date = new Date(today.valueOf() - (1000 * 3600 * 24 * numDays));

    var ret = _getBugReportForTeam(devAliasList, date, today);
    
    return ret;
}

/*****************************************************************************/
/* Send the weekly team bug report to an alias

     Parameters:
       devAliasList -- the lists of dev on the team
       teamAlias    -- the alias to send the mail to
*/

function sendWeeklyBugMailToTeam(devAliasList, teamAlias, numDays) {

    var report = getWeeklyBugReportForTeam(devAliasList, numDays);

    mailSendHtml(teamAlias, "Weekly Bug Mail", report.join("\n"));

    return 0;
}

/************************************************************************************/
/*****************************   SCHEMA INSPECTION    *******************************/
/************************************************************************************/ 

/************************************************************************************/
/* dumps all the names for the fields in the bug database.  Useful when you 
   are developing new routines and need to know the valid values for 'bugFieldGet' 

   Parameters
      dbName   : Name of the database to connect to
*/
function bugDumpFields(dbName) {

    if (dbName == undefined)
        dbName = "VSWhidbey";

    var bugDb = bugConnect(dbName);
    
    logMsg(LogProductStudio, LogInfo, "Field Name for ", dbName, " {\n");
    var psFieldDefs = bugDb.FieldDefinitions;
    for (var i = 0; i < psFieldDefs.Count; i++) { 
        var field = psFieldDefs.Item(i);
        logMsg(LogProductStudio, LogInfo, "Field: ", field.Name, " type ", field.Type, "\n");
    }
    logMsg(LogProductStudio, LogInfo, "}\n");
}

/************************************************************************************/
/* dumps the valid values for a field.  Useful when you are developing new routines 
   and need to know the valid values for 'bugFieldSet' 

   Parameters
      dbName   : Name of the database to connect to
      fieldName: Name of field 

*/
function bugDumpFieldValues(dbName, fieldName) {

    if (dbName == undefined)
        dbName = "VSWhidbey";

    var bugDb = bugConnect(dbName);
    var psFieldDefs = bugDb.FieldDefinitions;

    logMsg(LogProductStudio, LogInfo, "Valid Field values for ", fieldName, " in DB ", dbName, " {\n");
    psFieldDef = psFieldDefs.Item(fieldName);
    
    var psDatastoreItemTypeBugs = -100,
    psFieldValues = psFieldDef.AllowedValues(psDatastoreItemTypeBugs)
    for (var i = 0; i < psFieldValues.Count; i++) {
        logMsg(LogProductStudio, LogInfo, "Value: '", psFieldValues.Item(i), "'\n");
    }
    logMsg(LogProductStudio, LogInfo, "}\n");
}

/************************************************************************************/
/*******************************   SAMPLES    ***************************************/
/************************************************************************************/

/************************************************************************************/
/* An example use of bugCreate.  It is really meant to be a template
*/
function bugSampleDevDivScheduleWorkItem() {

    logSetFacilityLevel(LogProductStudio, LogInfo1000);

    var bugDb = bugConnect("DevDiv Schedule");
    var bug   = bugCreate(bugDb);

    bugFieldSet(bugDb, bug, "Title", "Test bug");
    bugFieldSet(bugDb, bug, "Assigned to", Env("USERNAME")); 
    bugFieldSet(bugDb, bug, "Opened by", Env("USERNAME"));
    bugFieldSet(bugDb, bug, "Priority", 1);
    bugFieldSet(bugDb, bug, "Severity", 1);
    bugFieldSet(bugDb, bug, "Description", "This is a test description");
    bugFieldSet(bugDb, bug, "Issue type", "Work Item");
    bugFieldSet(bugDb, bug, "SKU", "Whidbey");
    bugFieldSet(bugDb, bug, "Fix By", "Whidbey M3");
    bugFieldSet(bugDb, bug, "Days Spent", "00.00");
    bugFieldSet(bugDb, bug, "Discipline", "Dev");
    bugFieldSet(bugDb, bug, "Days Remaining", "01.00");
    bugFieldSet(bugDb, bug, "Original Estimate", "01.00");
    bugFieldSet(bugDb, bug, "Path", "CLR\\Performance\\NGEN");

    bugSave(bug);
    logMsg(LogProductStudio, LogInfo, "Created bug with ID ", bugFieldGet(bugDb, bug, "ID"), "\n");
    return 0;
}

/************************************************************************************/
/* An example use of bugCreate. It is really meant to be a template
*/
function bugSampleVsWhidbeyBug() {

    logSetFacilityLevel(LogProductStudio, LogInfo1000);

    var bugDb = bugConnect("VSWhidbey");
    var bug = bugCreate(bugDb);

    bugFieldSet(bugDb, bug, "Title", "Test bug");
    bugFieldSet(bugDb, bug, "Assigned to", Env("USERNAME")); 
    bugFieldSet(bugDb, bug, "Opened by", Env("USERNAME"));
    bugFieldSet(bugDb, bug, "Priority", 1);
    bugFieldSet(bugDb, bug, "Severity", 1);
    bugFieldSet(bugDb, bug, "Description", "This is a test description");
    bugFieldSet(bugDb, bug, "Issue type", "Work Item");
    bugFieldSet(bugDb, bug, "How found", "Ad Hoc (General)");
    bugFieldSet(bugDb, bug, "Source", "Development");
    bugFieldSet(bugDb, bug, "Open Build", "CLRPRI.00000.00");
    bugFieldSet(bugDb, bug, "SKU", "Whidbey");
    bugFieldSet(bugDb, bug, "Fix By", "Whidbey Beta 1");
    bugFieldSet(bugDb, bug, "Path", "Common Language Runtime\\JIT\\Code Gen");

    bugSave(bug);
    logMsg(LogProductStudio, LogInfo, "Created bug with ID ", bugFieldGet(bugDb, bug, "ID"), "\n");
    return 0;
}

/************************************************************************************/
/* Another example use of bugCreate. It is really meant to be a template
*/
function bugSampleDevDivWorkItem() {

    var bugDb = bugConnect("DevDiv Schedule");
    var bug = bugCreate(bugDb);

    logSetFacilityLevel(LogProductStudio, LogInfo100000);

    bugFieldSet(bugDb, bug, "Title", "Title");
    bugFieldSet(bugDb, bug, "Path", "CLR\\Longhorn");
    bugFieldSet(bugDb, bug, "Assigned to", Env("USERNAME")); 
    bugFieldSet(bugDb, bug, "Discipline", "Dev");
    bugFieldSet(bugDb, bug, "Issue type", "Dev Schedule");
    bugFieldSet(bugDb, bug, "Priority", 2);
    bugFieldSet(bugDb, bug, "Severity", 2);
    bugFieldSet(bugDb, bug, "Opened by", Env("USERNAME"));
    bugFieldSet(bugDb, bug, "SKU", "Whidbey");
    bugFieldSet(bugDb, bug, "Fix By", "Whidbey Beta2");
    bugFieldSet(bugDb, bug, "Days Spent", "00.00");
    bugFieldSet(bugDb, bug, "Days Remaining", "00.00");
    bugFieldSet(bugDb, bug, "Original Estimate", "00.00");
    bugFieldSet(bugDb, bug, "Description", "This is a test description");

    bugSave(bug);
    logMsg(LogProductStudio, LogInfo, "Created bug with ID ", bugFieldGet(bugDb, bug, "ID"), "\n");
    return 0;
}

/****************************************************************************/
/* a sample use of the query functionality */

function bugSampleVsWhidbeyQuery() {

    //logSetFacilityLevel(LogProductStudio, LogInfo1000);

    var dbName = "VSWhidbey";
    var bugDb = bugConnect(dbName);
    var queryXML =  bugQAnd(bugQEquals("Assigned to", "sborde"), bugQEquals("Status", "active"));

    var results = bugQuery(bugDb, queryXML);

    for (var i = 0; i < results.length; i++) {
        var result = results[i];
        WScript.Echo("Bug " + bugDb + " " + bugFieldGet(bugDb, result, "ID"))
        WScript.Echo("  Title:      " + splitToFit(bugFieldGet(bugDb, result, "Title"), 65, 14));
        WScript.Echo("  SubStatus:  " + bugFieldGet(bugDb, result, "SubStatus"));
        WScript.Echo("  Fix By:     " + bugFieldGet(bugDb, result, "Fix By"));
        WScript.Echo("  Priority:   " + bugFieldGet(bugDb, result, "Priority"));
        WScript.Echo("  Issue Type: " + bugFieldGet(bugDb, result, "Issue Type"));
        WScript.Echo("");
    }
    return 0;
}

/****************************************************************************/
/* a sample use of the update functionality */

function bugSampleVsUpdate() {
    var dbName = "VSWhidbey";
    var bugDb = bugConnect(dbName);
    var bug = bugGetById(bugDb, 484983);
    bugEdit(bug);
    bugFieldSet(bugDb, bug, "Priority", 3);
    bugSave(bug)
}

