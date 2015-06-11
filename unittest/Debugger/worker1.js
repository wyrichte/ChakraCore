/// <reference group="Dedicated Worker" />

var intervalInt;

importScripts('util_worker.js');

onmessage = function (event) {
    switch (event.data) {
        case 'init':
            init(); 
            break;

        case 'kill':
            postMessage('killing');
            WScript.Echo("Webworker is being closed") 
            close();
    }
}

function init() {
    setTimeout(think, 100);
    setTimeout(think, 200);
    setTimeout(think, 300);
}

function think() {
    postMessage(getRandom()); 
}

postMessage('alive');