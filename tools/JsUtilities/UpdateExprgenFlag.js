if (WScript.Arguments.length < 2) {
    WScript.Echo("cscript UpdateXML.js <flag name to add/update> <value>");
} else {
    var flagName = WScript.Arguments.Item(0);
    var flagValue = WScript.Arguments.Item(1);
    var objXML = new ActiveXObject("MSXML2.DOMDocument.6.0");
    objXML.async = false;
    objXML.load("Exprgen.xml");
    var nodeString = './/flag[@Name="' + flagName + '"]';
    //var nodeString = './/flag';
    var oElem = objXML.selectSingleNode(nodeString);
    if (null == oElem) {
        var oElem = objXML.createElement("flag");
        oElem.setAttribute("Name", flagName);
        oElem.setAttribute("Value", flagValue);
        objXML.lastChild.appendChild(oElem);
    } else {
        oElem.setAttribute("Value", flagValue);
    }
    objXML.save("Exprgen.xml");
}