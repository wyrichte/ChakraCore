var size = 1024 * 64;
var data = new Array(size);
var moving_averages = new Array(size);
var data_window = 13;

for (var i = 0; i < size; ++i) {
    data[i] = moving_averages[i] = Math.random();
}

function test(o, a, N) {
    for (var j = 0; j < N / size; ++j) {
        var moving_sum = 0;
        for (var i = 0; i < size; i++) {
            moving_sum += data[i];
            if(i >= data_window)
            {
                moving_sum -= data[i - data_window];
                moving_averages[i] = moving_sum / data_window;
            }
        }
    }
    return moving_averages;
}

var start = new Date();

for (var i = 0; i < 5; i++) {
    var N = 1024 * 1024 * 10;
    var o = { x: 3, y: -2, z: -1 };

    test(o, i, N);
}

var end = new Date();

recordResult(end-start);
// -------------- end benchmark --------------


function recordResult(timeInterval) {
    if ((typeof WScript !== "undefined") && (typeof WScript.Echo === "function")) {
        WScript.Echo("### TIME: " + timeInterval + " ms");
    } else {
        document.getElementById("console").innerHTML = timeInterval + "ms";
        if ((window.opener) && (window.opener.recordResult)) {
            window.opener.recordResult(timeInterval);
        } else if ((window.parent) && (parent.recordResult)) {
            window.parent.recordResult(timeInterval);
        }
    }
}