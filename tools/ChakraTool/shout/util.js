///
/// Util.js
///
/// Various utility helpers
///

/*
 * Basic environment context
 */
var Environment = { 
    config: {
        debug: false
    }
};

Environment.init = function() {
    if (!Environment.initialized) 
    {
        Environment.shell = new ActiveXObject("Wscript.Shell");
        Environment.variables = Environment.shell.Environment("Process");
        Environment.fso = new ActiveXObject("Scripting.FileSystemObject");

        Environment.initialized = true;
    }
}

/*
 * String formatting helper 
 * We likely don't need this with ES6
 */

String.prototype.format = function() {
    var s = this;
    for (var i = 0; i < arguments.length; i++) {       
        var reg = new RegExp("\\{" + i + "\\}", "gm");             
        s = s.replace(reg, arguments[i]);
    }
    
    return s;
}

/*
 * Helper to manage lifetime of temporary files
 */

var TempFileHelper = new (
    function() {
        var oThis = this;
        var oFSO = null;

        oThis.createNewTempFile = function(contents, fso) {
            oFSO = fso;
            var tempFolder = fso.GetSpecialFolder(2 /* temporary folder */);
            var tempFileName = fso.GetTempName();
            
            var tempFileStream = tempFolder.CreateTextFile(tempFileName);

            tempFileStream.write(contents);
            tempFileStream.close();

            var tempFile = tempFolder.Files.Item(tempFileName);
            var tempFilePath = tempFile.Path;
            _tempFiles.push(tempFilePath);

            return tempFilePath;
        }

        oThis.clearAllTempFiles = function() {
            alert("Deleting " + tempFiles.length + " files");
            for (var i = 0; i < _tempFiles.length; i++) {
                oFSO.DeleteFile(_tempFiles[i]);
            }
        }

        var _tempFiles = [];
    })();

/*
 * Escape a string to be used as a regex
 */
function escapeRegExp(str) {
    return str.replace(/[\-\[\]\/\{\}\(\)\*\+\?\.\\\^\$\|]/g, "\\$&");
}

/*
 * Async processing routine
 * Walks through a list of objects, asynchronusly applying a function to 
 * each item
 */

function processAsync(func, context, enumerator, done) {
    if (enumerator.atEnd()) { done(context); return; }
    
    window.setTimeout(
        function() { 
            var key = enumerator.item();
            if (func(context, key)) return;
            enumerator.moveNext();
            processAsync(func, context, enumerator, done); 
        }, 1);
}

/*
 * Buffered debug printing helper
 */
var Debug = new 
    function() {
         var buffer = [];
         var oThis = this;
         var debugConsole = null;
         var debugConsoleContainer = null;
         
         oThis.init = function() {
             try {
                 if (Environment.variables("_debug") == "1") {
                     // alert(Environment.variables("_debug"));
                     Environment.config.debug = true;
                 }
             } catch (e) {}

             debugConsole = document.getElementById("debugConsole");
             debugConsoleContainer = document.getElementById("debugConsoleContainer");
             
             if (Environment.config.debug) {
                 debugConsoleContainer.className = "";
                 debugConsole.value = "";
             }       
         }

         oThis.out = function(s) {
             if (Environment.config.debug == false) return;

             if (buffer.length == 256) {
                 var value = buffer.join('\n') + "\n";
                 window.setTimeout(function() { 
                                       debugConsole.value += value;
                                   }, 1)
                 buffer = [];
             }
             else {
                 buffer.push(s);
             }
         }

         oThis.flush = function() {
             debugConsole.value += (buffer.join('\n') + "\n");
             buffer = [];            
         }
    };

function debugOut(s)
{
    Debug.out(s);
}

function debugFlush() {
    Debug.flush();
}
