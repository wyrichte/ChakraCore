/*
    This file provides generalized functionality to connect to a database
    and execute queries
    OWNER: sanket
 */

if (!fsoModuleDefined)   throw new Error(1, "Need to include fso.js");
if (!logModuleDefined)   throw new Error(1, "Need to include log.js");
if (!runModuleDefined)   throw new Error(1, "Need to include run.js");
if (!Env)                throw new Error(1, "Need to have Env defined. This script expects it to be defined by run.js");

var databaseModuleDefined = 1; // Indicate that this module exists

/**********************************************************/
/* DEFAULT VALUES FOR ACCESSING THE RETAIL LAB DATABASE */
/**********************************************************/
var DEFAULT_SQLSERVER = "CLRRET0\\CLRRETSQLSERVER";
var DEFAULT_SQLUSER = "REDMOND\\clrgnt";
var DEFAULT_SQLPWD = "rim$mAy05";
var DEFAULT_SQLDB = "RetailLab";
var DEFAULT_SQLSECURITY="SSPI";
var DEFAULT_SQLPROVIDER="SQLOLEDB.1";

/**********************************************************/
/* GLOBAL VARIABLES FOR ACCESSING THE DATABASE */
/**********************************************************/
var g_DatabaseLogObject = logNewFacility("sqlDatabase");
var g_DatabaseActiveXObject = undefined;
var g_DatabaseObject = undefined;

/**********************************************************/
/* CORE DATABASE FUNCTIONALITY */
/**********************************************************/

/**********************************************************/
/* Function to get the database object to execute a query */
function _getDatabaseObject(sProvider, sIntegratedSecurity, sServer, sDatabase)
{
    logCall(g_DatabaseLogObject, LogInfo10, "_getDatabaseObject", arguments, "{");
    if (sProvider == undefined)
        sProvider = DEFAULT_SQLPROVIDER;
    if (sIntegratedSecurity == undefined)
        sIntegratedSecurity = DEFAULT_SQLSECURITY;
    if (sServer == undefined)
        sServer = DEFAULT_SQLSERVER;
    if (sDatabase == undefined)
        sDatabase = DEFAULT_SQLDB;

    var nReTries = 0;
    if (g_DatabaseObject == undefined || g_DatabaseObject == null)
    {
        while (g_DatabaseActiveXObject == undefined || g_DatabaseActiveXObject == null)
        {
            if((++nReTries % 3) == 0) {
                throw new Error(1, "Database Object creation failed multiple times. Please investigate...\r\n");
            }
            // We try 3 times before we fail, waiting for a total of 15 minutes            
            WScript.Sleep(5 * 60);
            g_DatabaseActiveXObject = new ActiveXObject("ADODB.Connection");
        }
        var connString = "PROVIDER=" + sProvider + ";Integrated Security=" + sIntegratedSecurity + ";SERVER=" + sServer + ";DataBase=" + sDatabase;
        logMsg(g_DatabaseLogObject, LogInfo, "Connection string for connecting to the database: " + connString + "\r\n");
        g_DatabaseActiveXObject.CommandTimeout = 60;
        g_DatabaseActiveXObject.Open(connString);
        g_DatabaseObject = g_DatabaseActiveXObject;
    }

    return g_DatabaseObject;
}

/****************************************************/
/* Function to execute a database query */
/* Example: "SELECT * FROM RetailLab.dbo.Job_Table" */
function _executeDatabaseQuery(sSqlQuery, sProvider, sIntegratedSecurity, sServer, sDatabase)
{
    logCall(g_DatabaseLogObject, LogInfo10, "_executeDatabaseQuery", arguments, "{");

    if (sSqlQuery == undefined)
        throw new Error(1, "SQL query needs to be specified to be able to run it");

    var nReTries = 0;
    var oSqlResult = undefined;
    while(true)
    {
        try
        {
            oSqlResult = _getDatabaseObject(sProvider, sIntegratedSecurity, sServer, sDatabase).Execute(sSqlQuery);
            break;
        }
        catch(e)
        {
            // Try to execute the query 3 times (15 minutes). If we are still failing, bail out
            if((++nReTries % 3) == 0) {
                logMsg(g_DatabaseLogObject, LogInfo, "Failed to execute SQL query: " + sSqlQuery + ". Please investigate...\r\n");
                oSqlResult = new ActiveXObject("ADODB.RecordSet");
                break;
            } else {
                logMsg(g_DatabaseLogObject, LogInfo, "SQL string " + sSqlQuery + " failed to execute.  Failure was " + e.description + ". Attempt number " + nReTries + ". Retrying in five minutes...");
                WScript.Sleep(5 * 60);
            }
        }
    }

/*
    // This is how we iterate over the result of a SQL query
    var tempSqlResult = oSqlResult;
    while (!tempSqlResult.EOF)
    {
        WScript.StdOut.Write("Job_Id: " + tempSqlResult("Job_Id") + "\r\n");
        WScript.StdOut.Write("Start_Time: " + tempSqlResult("Start_Time") + "\r\n");
        WScript.StdOut.Write("End_Time: " + tempSqlResult("End_Time") + "\r\n");
        WScript.StdOut.Write("Machine_Name: " + tempSqlResult("Machine_Name") + "\r\n");
        WScript.StdOut.Write("Job_Type: " + tempSqlResult("Job_Type") + "\r\n");
        WScript.StdOut.Write("===================================================\r\n");
        tempSqlResult.MoveNext();
    }
*/

    return oSqlResult;
}

/**********************************************************/
/* UTILITY DATABASE FUNCTIONALITY */
/**********************************************************/

/******************************************************/
/*     
    Function to convert a javascript date object into a string which 
         can be converted to a SQL date object

    Arguments:
    oDate: The javascript date object (Default: date object 
                        corresponding to the current time)
  */
function _toSqlTime(oDate)
{
    if(oDate == undefined)
        oDate = new Date();
    var oMonthNames = ["Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"];
    var nMonthIndex = (oDate.getMonth()*1);    
    var sSqlDate = oMonthNames[nMonthIndex] + " " + oDate.getDate() + " " + oDate.getFullYear();
    var nHours = oDate.getHours()*1;
    var nMinutes = oDate.getMinutes()*1;
    var sAmPm = "AM";
    if (nHours >= 12) {
        sAmPm = "PM";
    }    
    if (nHours > 12) {
        nHours -= 12;
    }
    if (nMinutes < 10) {
        nMinutes = "0" + String(nMinutes);
    }
    
    sSqlDate = sSqlDate + " " + nHours + ":" + nMinutes + " " + sAmPm;
    return sSqlDate;
}

/****************************************************************************/
/*
    Function to determine if the value obtained from the database is null or not

    Arguments:
    sEntry: The entry from the database
 */
function _isNullDatabaseEntry(sEntry)
{
    sEntry = String(sEntry).toUpperCase();
    if (sEntry == "UNDEFINED" || sEntry == "NULL" || sEntry == "")
        return true;
    return false;
}

