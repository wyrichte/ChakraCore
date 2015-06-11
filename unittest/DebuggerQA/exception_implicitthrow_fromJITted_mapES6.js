/*
  Implicit TypeError

*/

/**exception(resume_ignore):stack();locals()**/

var callcount = 0;

function Run(randomArgs) {
    var a = [1];
    callcount++;
    if (callcount == 3) {
        a.forEach(function () {
            Map.prototype.has.call({}); //throws TypeError
        })
        WScript.Echo('PASSED');
    }
}

Run();
Run();
Run(); //will throw here