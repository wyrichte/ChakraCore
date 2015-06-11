var x = {
    get test(){ WScript.Echo("executed as expected"); return super.x; }
}
WScript.Echo("statements before super reference");
x.test;
WScript.Echo("ERROR:statements after super reference should not be executed");