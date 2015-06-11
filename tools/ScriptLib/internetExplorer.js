/*                             internetExplorer.js                           */
/*****************************************************************************/

/* Routines associated with using Internet Explorer (mostly as a data entry
   mechanism)
*/

/* AUTHOR: Vance Morrison
   DATE: 4/26/05 */

/******************************************************************************/
var InternetExplorerModuleDefined = 1;              // Indicate that this module exists

if (!fsoModuleDefined) throw new Error(1, "Need to include fso.js");
if (!logModuleDefined) throw new Error(1, "Need to include log.js");

var LogInternetExploer = logNewFacility("CLRAutomation");

if (ScriptDir == undefined)
        var ScriptDir  = WScript.ScriptFullName.match(/^(.*)\\/)[1];

if (Env == undefined)
        var Env = WshShell.Environment("PROCESS");

/* Define the IE READYSTATE values */
var READYSTATE_UNINITIALIZED = 0;
var READYSTATE_LOADING = 1;
var READYSTATE_LOADED = 2;
var READYSTATE_INTERACTIVE = 3;
var READYSTATE_COMPLETE = 4;

/* Define the wait count and time for IE. Don't wait too long; 30 seconds seems like too long. */
var IEMaximumWaitCount = 300;
var IEWaitTime = 100; // ms

/*****************************************************************************/
/*                        GENERIC FORMS FUNCTIONALITY                        */
/*****************************************************************************/

/*****************************************************************************/
/* Currently we wire together the GUI object with global variabes (yuk), 
   which means you can have only one IE object that uses this vars active
   at one time (probably not a hardship */

var IEData;     // Our only global variable. 
function IE_onQuit() {
    IEData.done = true;
}

/*****************************************************************************/
/* Wait until IE is ready.
 */

function WaitForIEReady(IE) {
    var waitCount = 0;
    while ((IE.ReadyState < READYSTATE_COMPLETE) && (waitCount < IEMaximumWaitCount)) {
        WScript.Sleep(IEWaitTime);
        waitCount++;
    }
    if (waitCount >= IEMaximumWaitCount) {
        logMsg(LogClrAutomation, LogError, "IE failed to reach ready state in expected time\n");
    }
}

/*****************************************************************************/
/* create a new IE form for the html file. It makse few assumtions and
   thus adds little value  */

function IENew(htmlFileName, height, width) {

    // logMsg(LogClrAutomation, LogInfo, "Creating IE object\n");  
    IE = WScript.CreateObject("InternetExplorer.Application", "IE_");

    IE.height=height;
    IE.width=width;
    IE.left=0;
    IE.top=0;

    IE.menubar=0;    
    IE.toolbar=0;
    IE.statusBar=0;
    IE.Navigate(htmlFileName);
    while (IE.busy) { WScript.Sleep(100); }

    IEData = {};                // This is the handle that holds all state associated with the form
    IEData.IEDoc = IE.Document;
    IEData.IE = IE;
    IEData.done = false;
    IEData.visible = false;

    while (IE.busy) { WScript.Sleep(100); }    // Not clear we need this but 

    WaitForIEReady(IE);

    return (IEData);
}

/*****************************************************************************/
/* returns true if IE is done */

function IEPoll(IEData) {

    if (!IEData.done && !IEData.visible) {
        IEData.visible = true;
        IEData.IE.Visible = 1;
    }
    return IEData.done;
}

/*****************************************************************************/
/* create a new IE form that has context senstive help and will return a
   list of values in the form.  It is pretty generic and is driven
   by the html.
   
   Parameters:

   validationFtn  - validates field values gathered from fieldsGatherFn.
                    the function passed in takes two parameters; parameter 1 is the
                    the data to verify and parameter 2 is validationArg.

   fieldsInitFn   - initializes fields with default values; can be undefined.
                    the function passed in takes two parameters; parameter 1 is the
                    IE form object and parameter 2 is the default value data.

   fieldsGatherFn - walks the fields on the form gathering data; return value passed to validationFtn
                    the function passed in takes one parameter and is the IE form object.

*/

function IEFormNew(htmlFileName, height, width, initValues, validationFtn, validationArg, fieldsInitFn, fieldsGatherFn) {
    
    IEData = IENew(htmlFileName, height, width);
    var inputDoc = IEData.IEDoc.frames("Input").document;
        
        // My local state
    IEData.inputDoc = inputDoc;
    IEData.helpDoc = IEData.IEDoc.frames("Help").document;
    IEData.selectedHelp = undefined;
    IEData.retVal = undefined;
    IEData.validationFtn = validationFtn;
    IEData.validationArg = validationArg;

        // Wire up the button events 
    inputDoc.all.Cancel.onclick = function() { IEData.IE.Quit(); }
    inputDoc.all.Submit.onclick = function() { IEFormSubmitCallBack(fieldsGatherFn); }

        // initialize all fields with their default values
    if (fieldsInitFn != undefined)
        fieldsInitFn(IEData, initValues);

        // Highlight help text for each field as it gets focus
    for (var i = 0; i < inputDoc.all.length; i++) {
        var inputElement = inputDoc.all[i];
        var id = inputElement.id
        if (!id.match(/^(\S+)_input$/))
            continue;
        inputElement.onfocus = IEFormHelpUpdateCallBack;
    }

    return IEData;
}

/******************************************************************************/
/* After calling IEFormNew, and setting up any additional form specific 
   customization, IEFormResult waits for the form to be entered, and returns
   the data from the form 
*/ 
function IEFormResult(IEData) {

        // wait for callbacks
    while(!IEPoll(IEData)) {
        WScript.Sleep(100); 
    }
    
    return IEData.retVal;
}

/******************************************************************************/
/* This is called back when a field gets focus.  Its job is to highlight the 
   help associated with the field.  It assumes the global variable IEData */

function IEFormHelpUpdateCallBack() {

    // logMsg(LogClrAutomation, LogInfo, "In IEFormHelpUpdateCallBack\n");
    var inputDoc = IEData.inputDoc;
    if (inputDoc.parentWindow.event.srcElement.id.match(/^(\S+)_input$/)) {
        var helpName = RegExp.$1 + "_helpSec";
        var helpDoc = IEData.helpDoc;
        var helpTag = helpDoc.all[helpName];
        IEFormHelpHighligth(IEData, helpTag);
    }
}

/******************************************************************************/
/* highlights 'helpTag' in the help portion of the form */

function IEFormHelpHighligth(IEData, helpTag) {

    if (helpTag) {
        var prevHelp = IEData.selectedHelp;
        if (prevHelp != undefined) {
            prevHelp.style.backgroundColor = "white"
            prevHelp.style.color = "black";
            prevHelp.style.fontWeight = "normal";
            prevHelp.style.border = "none";
        }
        helpTag.style.backgroundColor = "lightblue";
        helpTag.style.border = "thin solid";
        helpTag.style.color = "blue";
        helpTag.style.fontWeight = "bold";
        helpTag.scrollIntoView(false);
        IEData.selectedHelp = helpTag;
    }
}

/******************************************************************************/
/* This is called back when the form has been submitted.  Its job is to 
   gather up the input, validate it, and if it succeeds, quit IE.
   It assumes the global variable IEData */

function IEFormSubmitCallBack(gatherFunction) {

    // logMsg(LogClrAutomation, LogInfo, "In IEFormSubmitCallBack \n");
    var inputDoc = IEData.inputDoc;
    var ret = gatherFunction(IEData);
    var errorMsg = IEData.validationFtn(ret, IEData.validationArg);
    if (errorMsg != undefined) {
        inputDoc.parentWindow.alert(errorMsg);
    }
    else {
        IEData.retVal = ret;
        IEData.IE.Quit();    
    }
}

