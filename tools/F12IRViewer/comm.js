//
// Copyright (C) Microsoft. All rights reserved.
//

function log(message) {
    //console.log(message);  // uncomment this line to turn on logging
}

/**
 * Send a simple message to the target page.
 */
function sendMessage(port, type) {
    var obj = {};
    obj.type = type;
    if (!port) return;
    log(">> REMOTE PAGE SENDING MESSAGE: " + type);
    port.postMessage(JSON.stringify(obj));
}

/**
 * Send a message to the target page which includes a data object payload.
 */
function sendMessageWithData(port, type, data) {
    var obj = {};
    obj.type = type;
    obj.data = data;
    if (!port) return;
    log(">>> REMOTE PAGE SENDING MESSAGE WITH DATA: " + type);
    port.postMessage(JSON.stringify(obj));
}
