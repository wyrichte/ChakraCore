var e = WScript.LoadScriptFile("domInClosedContext_data.js", "samethread");

var x = e.img; // Get hold of a DOM object in closed script context
WScript.Shutdown(e);

// Run in a timeout so engine "e" is actually closed
WScript.SetTimeout(function(){
    // /**exception(resume_ignore):stack()**/

    //TEMPORARY: TFS 870875 VerifyObjectAlive not passing correct requestContext
    //Currently none of following uses the correct requestContext, thus none is reported to debugger.
    try {

    // Each of following will report an exception to debugger, and dump the stack.
    var a = x.width;    // GetProperty
    x.abcd = 0;         // GetSetter
    delete x.abcd;      // DeleteProperty
    delete x[12];       // DeleteItem

    //TODO: more. every ExternalObject VerifyObjectAlive.
    
    } catch(e) {
    }

    WScript.Echo("pass");
},1000);
