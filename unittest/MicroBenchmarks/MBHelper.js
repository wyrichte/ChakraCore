var g_Binary = WScript.Arguments.Named.Item("binary");
g_Binary = new String(g_Binary).toLowerCase();
g_Binary = g_Binary.substring(g_Binary.lastIndexOf("\\") + 1);
if(-1 != g_Binary.lastIndexOf(".")) {
    g_Binary = g_Binary.substring(0, g_Binary.lastIndexOf("."));
}

var g_Priority = 3;
try {
    g_Priority = parseInt(WScript.Arguments.Named.Item("priority"));
} catch (e) {}
if (isNaN(g_Priority)) {
    g_Priority = 3;
}

var g_TCListFile = "TCList.xml";
var g_WshShell = new ActiveXObject("wscript.shell");
var g_FileSystemObject = new ActiveXObject("Scripting.FileSystemObject");
FolderWalker(g_WshShell.currentdirectory, "");

function FolderWalker(p_RootDir, p_SubPath) {
    var tcListFile = p_SubPath == "" ? g_TCListFile : (".\\" + p_SubPath + "\\" + g_TCListFile);
	if(g_FileSystemObject.FileExists(tcListFile)) {
		XMLParser(p_SubPath, tcListFile);
	}
	var rootFolder = g_FileSystemObject.GetFolder(p_RootDir);
	var subFolders = new Enumerator(rootFolder.SubFolders);
	for (subFolders.moveFirst(); !subFolders.atEnd(); subFolders.moveNext()) {
        var sSubPath = p_SubPath == "" ? subFolders.item().name : p_SubPath + "\\" + subFolders.item().name;
        FolderWalker(subFolders.item().path, sSubPath);
	}
}

function XMLParser(p_Path, p_TCListFile) {
    var objXML = new ActiveXObject("MSXML2.DOMDocument.6.0");
    objXML.async = false;
    objXML.load(p_TCListFile);
    var rootNode = objXML.selectSingleNode("Tests");
    var testNodes = rootNode.selectNodes("Test");
    for (var i = 0; i < testNodes.length; ++i) {
        var sFileName = testNodes[i].selectSingleNode("FileName").text;
        var sPriority = parseInt(testNodes[i].selectSingleNode("Priority").text);
        var sBaseFileName = sFileName.substring(0, sFileName.lastIndexOf("."));
        if (sPriority <= g_Priority) {
            var sHosts = new String(testNodes[i].selectSingleNode("Hosts").text).toLowerCase().replace(/\s/g,"");
            var arrHosts = sHosts.split(",");
            for(iHost in arrHosts) {
                if((arrHosts[iHost] === g_Binary) || (arrHosts[iHost] === "all")){
                    var sTestFileName = p_Path == "" ? sBaseFileName : p_Path + "\\" + sBaseFileName;
                    var sAddSwitches = "";
                    var sAddSwitchesNode = testNodes[i].selectSingleNode("AdditionalSwitches");
                    if(null != sAddSwitchesNode) {
                        sAddSwitches = sAddSwitchesNode.text;
                    }
                    Print(sTestFileName + "," + sAddSwitches);
                    break;
                }
            }
        }
    }
}

function Print(str) {
	WScript.Echo(str);
}