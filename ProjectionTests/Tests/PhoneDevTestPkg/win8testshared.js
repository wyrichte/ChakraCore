/// <disable>JS2085,JS2038,JS2015,JS2064,JS2003,JS2043,JS2055,JS2087</disable>
errorHandler = function (msg, url, line) {
    var text = "Msg: " + msg + "\n"
    text += "URL: " + url + "\n"
    text += "LineNumber: " + line + "\n\n"

    storeError(text);

    return true;
}

logMessage = function (msg) {
    var element = document.getElementById("log");
    if (element == undefined) {
        throw new Error("Log store not found");
    }
    if (msg == undefined) {
        throw new Error("msg undefined");
    }

    element.appendChild(msg);
}

storeError = function (message, data) {
    var element = document.getElementById("error");
    if (element == undefined) {
        throw new Error("Error store not found");
    }

    if (data != undefined) {
        element.setAttribute("data", data);
    }
    if (message != undefined) {
        element.setAttribute("message", message);
    }
}

storeResult = function (data) {
    var element = document.getElementById("result");
    if (element == undefined) {
        throw new Error("Results store not found");
    }

    if (data != undefined) {
        element.setAttribute("data", data);
    }
}

appendResult = function (data) {
    var element = document.getElementById("result");
    if (element == undefined) {
        throw new Error("Results store not found");
    }

    if (data != undefined) {
        var previousData = element.getAttribute("data");

        if (previousData == undefined || previousData.toString().length == 0) {
            element.setAttribute("data", data);
        }
        else {
            element.setAttribute("data", previousData + ";" + data);
        }
    }
}

document.evaluateJavaScript = function (text) {
    return eval(text);
}
