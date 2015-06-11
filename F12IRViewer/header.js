//
// Copyright (C) Microsoft. All rights reserved.
//

var _port = null;

function irviewerMain() {
    // I called this function from the html, but really it can be called however you want when you are initializing your page

    try {
        // Hook up the events for F12 attaching and detaching from IE
        var external = window.external;
        external.addEventListener("attach", onAttach);
        external.addEventListener("detach", onDetach);
        if (external.isAttached) {
            onAttach();
        }
    } catch (e) {
        // console.log(e);
    }
}

function onAttach() {
    // Get the APIs that allow you to inject code into the IE page
    var diagOMEngine = window.external;

    // Load up the remote file
    diagOMEngine.loadScriptInProc("js/helpers.js");
    diagOMEngine.loadScriptInProc("data.js");
    diagOMEngine.loadScriptInProc("comm.js");
    diagOMEngine.loadScriptInProc("remote.js");

    // Wait for the remote side to connect back
    diagOMEngine.addEventListener("connect", onConnect);
}

function onDetach() {
    // F12 has now closed, do any cleanup here
    var diagOMEngine = window.external;
    diagOMEngine.removeEventListener("connect", onConnect);

    _port = null;
}

function onConnect(port) {
    // Store the port object that is used for communication so we can use it later
    _port = port;

    // Now we have a port we can listen to messages from it
    _port.addEventListener("message", onMessage);

    // We can also communicate by posting a message to the port
    // postMessage can only send strings, so JSON.stringify if you need to

    sendMessage(_port, HeaderMessage.CONNECTION_SUCCESSFUL);
}

/**
 * Called when a message is received from the target page.
 */
function onMessage(msgObj) {
    var data = msgObj.data;
    var message = JSON.parse(data);

    console.log("< F12 PAGE RECEIVED MESSAGE: " + message.type);

    if (message.type == RemoteMessage.ACK_CONNECTION_SUCCESSFUL) {
        sendMessage(_port, HeaderMessage.REQUEST_FUNCTION_LIST);
    } else if (message.type == RemoteMessage.FUNCTION_LIST) {
        var fnlist = message.data;
        PopulateFunctionList(fnlist);
    } else if (message.type == RemoteMessage.IR_VIEWER_JIT) {
        var ir = message.data;
        $("#resultOutput").html(_.escape(JSON.stringify(ir)));
        $("#resultError").html("OK");
        DisplayIRData(ir);
    }
}
