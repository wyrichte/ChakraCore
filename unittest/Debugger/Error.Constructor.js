var navigator = { userAgent: "Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.2; Trident/6.0; .NET4.0E; .NET4.0C)", appName: "Internet Explorer" };

WScript.LoadScriptFile("newGlue.js");

var testcase = new TestCase();
testcase.id="1";
testcase.desc="Testing initial value of Error.prototype, name and message property of Error Object, ES5 section 15.11.1.1 ";
testcase.test= function(){
var err1 = new Error();
if(getHOSTMode() < IE8STANDARDSMODE){
    verify(Error.prototype.toString(),"[object Error]","For IE7 and previous versions, Error.prototype should be [object Error]");
}
else {
    verify(Error.prototype.toString(),"Error","Initial value of Error.prototype is Error");
    verify(err1.message,"","Initial value of message property is \"\"");
}
    verify(typeof err1,"object","Typeof Error should be object");                                    /**bp:locals(1)**/
    verify(err1.name,"Error","Initial value of name property is Error");
    verify(err1.message,"","Initial value of message property is \"\"");                                                
}
testcase.AddTest();

Loader42_FilenName='Error.Constructor';
WScript.Attach(Run);
