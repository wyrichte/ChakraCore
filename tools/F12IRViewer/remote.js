//
// Copyright (C) Microsoft. All rights reserved.
//


// browser is a global in remote code that points to the diagnostics OM
// It has a whole bunch of stuff on it, take a look at:
// $\src\bpt\diagnostics\common\DiagnosticsOM.d.ts for the typescript interface definitions

// toolUI is a global in remote code (with a legacy name - sorry) that represents the APIs for communication
// we essentially only use it for creating the port


var window = browser.document.parentWindow;

// Create communications port back to toolwindow
var _port = toolUI.createPort("toolPort");

// Add a listener for messages from the F12 side
_port.addEventListener("message", onMessage);

// Connect back to the other side sending in the port we just created
toolUI.connect(_port);

function onMessage(msgObj) {
    var data = msgObj.data;
    var message = JSON.parse(data);

    log("< REMOTE PAGE RECEIVED MESSAGE: " + message.type);

    if (message.type == HeaderMessage.CONNECTION_SUCCESSFUL) {
        sendMessage(_port, RemoteMessage.ACK_CONNECTION_SUCCESSFUL);
    } else if (message.type == HeaderMessage.REQUEST_FUNCTION_LIST) {
        sendMessageWithData(_port, RemoteMessage.FUNCTION_LIST, functionList());
    } else if (message.type == HeaderMessage.REQUEST_JIT) {
        var ptr = message.data.utf8SrcInfoPtr;
        var id = message.data.funcId;
        var jit = rejitFunction(ptr, id);

        sendMessageWithData(_port, RemoteMessage.IR_VIEWER_JIT, jit);
    }
}
