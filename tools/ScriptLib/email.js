/*********************************************************************************/
/*                                  email.js                                     */
/*********************************************************************************/
/*********************************************************************************/

/* utilities associated with e-mail */

// AUTHOR: Vance Morrison   
// DATE: 11/1/2003

/*********************************************************************************/

var emailModuleDefined = 1;                     // Indicate that this module exist
if (!logModuleDefined) throw new Error(1, "Need to include log.js");

var LogEmail = logNewFacility("email");

if (WshShell == undefined)
    var WshShell = WScript.CreateObject("WScript.Shell");
if (Env == undefined)
    var Env = WshShell.Environment("PROCESS");

/**********************************************************************/
/* sends a text message to 'to', with a given subject and body.  Both
   'to' and 'from' need to be of the form <name>@<host>.   'to' can 
   be a semicolon separated list (to send to multiple places).  Each
   address must be of the form <user>@hostName.  Only the first
   three parameters are required 

   Use "-x filename" for the body arg to send the contents of a file as 
   the body of the email.  
*/

function mailSendText(to, subject, body, cc, from, server) {

    return _mailSend(to, subject, body, cc, from, server, "text");
}

/**********************************************************************/
/* sends a HTML message to 'to', with a given subject and body.  Both
   'to' and 'from' need to be of the form <name>@<host>.   'to' can 
   be a semicolon separated list (to send to multiple places).  Each
   address must be of the form <user>@hostName.  Only the first
   three parameters are required 

   Use "-x filename" for the body arg to send the contents of a file as 
   the body of the email.  
*/

function mailSendHtml(to, subject, body, cc, from, server) {

    return _mailSend(to, subject, body, cc, from, server, "html");
}

/**********************************************************************/
/* attachments is an array of files to attach to the email. If
   attachments are present, the kind must be html.
*/
function _mailSend(to, subject, body, cc, from, server, kind, attachments) {

    if (from == undefined)
        from = Env("USERNAME") + "@microsoft.com";
    if (server == undefined)
        server = "SmtpHost";    // This only works inside microsoft
    if (attachments == undefined)
        attachments = [];

    if (kind != "html" && attachments.length > 0)
        throw Error(1, "Can not attach files to email of type '" + kind + "'. Type must be 'html'.");

    logMsg(LogEmail, LogInfo, "sendMail(", to, ", ", subject, ", body, ", from, ", ", server, ", ", kind, ")\n");

    if (body.match(/^-x *(.+)/)) {
        var bodyFile = RegExp.$1;
        logMsg(LogEmail, LogInfo, "found file specifier in body arg, attempting to pull content from file ", bodyFile, "\n");
        var body = FSOReadFromFile(bodyFile);
    }
    
    var objEmail = WScript.CreateObject("CDO.Message");
    if (objEmail == undefined)
        throw Error(1, "Could not find 'CDO.Message' COM object (Some mail program needs to be installed)");

    to = to.replace(/,/g, ";");        // allow commas a separators too. 
    objEmail.From = from;
    objEmail.To = to;
    objEmail.Subject = subject;
    if (cc != undefined)
        objEmail.CC = cc;
    if (from.toLowerCase() == "clrgnt@microsoft.com") {
        if (cc != undefined && cc.toLowerCase().indexOf("clrrlnot@microsoft.com") >= 0) {
            objEmail.ReplyTo = "clrrlnot@microsoft.com";
        } else {
            objEmail.ReplyTo = "clrsnsw@microsoft.com";
        }
    }

    if (kind == "html")
        objEmail.HTMLBody = body;       
    else 
        objEmail.TextBody = body;

    // Add email attachments
    for (var i = 0; i < attachments.length; i++) {
        objEmail.AddAttachment(attachments[i]);
    }

    objEmail.Configuration.Fields.Item("http://schemas.microsoft.com/cdo/configuration/sendusing") = 2;         // send using SMTP server
    objEmail.Configuration.Fields.Item("http://schemas.microsoft.com/cdo/configuration/smtpserver") = server;
    objEmail.Configuration.Fields.Item("http://schemas.microsoft.com/cdo/configuration/smtpserverport") = 25    // SMTP port number
    objEmail.Configuration.Fields.Item("http://schemas.microsoft.com/cdo/configuration/smtpauthenticate") = 2;  // windows authentication
    objEmail.Configuration.Fields.Update();

    objEmail.Send();        // will throw an exception on failure
    return 0;
}

/**********************************************************************/
/* Composes, but doesn't send, a text message to 'to', with a given subject and body.  Both
   'to' and 'from' need to be of the form <name>@<host>.   'to' can 
   be a semicolon separated list (to send to multiple places).  Each
   address must be of the form <user>@hostName.  Only the first
   three parameters are required 

   Use "-x filename" for the body arg to send the contents of a file as 
   the body of the email.  
*/

function mailNoSend(to, subject, body, cc, from, server) {

    return _mailNoSend(to, subject, body, cc, from, server, "text");
}


/**********************************************************************/
/* attachments is an array of files to attach to the email. If
   attachments are present, the kind must be html.
*/
function _mailNoSend(recipients, subject, body, cc, from, server, kind, attachments) {

    if (from == undefined)
        from = Env("USERNAME") + "@microsoft.com";
    if (server == undefined)
        server = "SmtpHost";    // This only works inside microsoft
    if (attachments == undefined)
        attachments = [];

    if (kind != "html" && attachments.length > 0)
        throw Error(1, "Can not attach files to email of type '" + kind + "'. Type must be 'html'.");

    logMsg(LogEmail, LogInfo, "sendMail(", recipients, ", ", subject, ", body, ", from, ", ", server, ", ", kind, ")\n");

    if (body.match(/^-x *(.+)/)) {
        var bodyFile = RegExp.$1;
        logMsg(LogEmail, LogInfo, "found file specifier in body arg, attempting to pull content from file ", bodyFile, "\n");
        var body = FSOReadFromFile(bodyFile);
    }

    var objSession = WScript.CreateObject("MAPI.Session");
    objSession.Logon(from, "", false, false);
    var objMessage = objSession.Outbox.Messages.Add();
    objMessage.Subject = subject;
    objMessage.Text = body;
    
    for (var i = 0; i < recipients.length; i++) {
        var objRecipients = objMessage.Recipients;
        var recipient = objRecipients.Add;
        recipient.Name = recipients[i];
       // recipient.Type = CdoTo;
    }

    objMessage.Send(true, true, 0);
    objSession.Logoff();
    return 0;
}
