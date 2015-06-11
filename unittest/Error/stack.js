function printError(e) {
    print(e.name);
    print(e.number);
    print(e.description);
}
function print(str) {
    if (typeof (WScript) == "undefined") {
        print(str);
    }
    else {
        WScript.Echo(str);
    }
}

for (var i = 1; i < 4; i++) {
    print("\n#" + i);
    try {
        try {
            function f() {
                f();
            }
            f();
        } finally {
            print("In finally");

            //        f();   // This crashes Eze....
        }
    }
    catch (e) {
        printError(e);
    }
}

print("testing stack overflow handling with catch block");
try {
    function stackOverFlowCatch() {
        try {
            stackOverFlowCatch();
            while (true) {
            }

        }
        catch (e) {
            throw e;
        }
    }
    stackOverFlowCatch();
}
catch (e) {
    printError(e);
}

print("testing stack overflow handling with finally block");
try
{
function stackOverFlowFinally() {
    try {
        stackOverFlowFinally();    
        while (true) {
        }
    }
    finally {
       DoSomething();
    }
}
   stackOverFlowFinally();
}
catch(e) {
	printError(e);
}

function DoSomething()
{
}

try
{
   var count = 10000;

   var a = {};
   var b = a;

   for (var i = 0; i < count; i++)
   {
	a.x = {};
	a = a.x;	
   }
   eval("JSON.stringify(b)");
}
catch(e) {
	printError(e);
}

