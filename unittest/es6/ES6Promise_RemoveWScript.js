// ES6 Promise test to ensure we don't crash if WScript.SetTimeout is not defined
// This test will be obsolete when trident delivers the task queue

function echo(str) {
    WScript.Echo(str);
}

var tests = [
    {
        name: "Host Enqueue Function is undefined so we'll throw when calling then on a promise",
        body: function () {
            WScript.SetTimeout = undefined;
            
            Promise.resolve(42).then(
                function(result) {
                    echo('success: ' + result);
                },
                function(err) {
                    echo('failure: ' + err);
                }
            );
        }
    },
];

var index = 0;

function runTest(test) {
    echo('Executing test #' + index + ' - ' + test.name);
    
    try {
        test.body(index);
    } catch(e) {
        echo('Caught exception: ' + e);
    }
    
    index++;
}

tests.forEach(runTest);
