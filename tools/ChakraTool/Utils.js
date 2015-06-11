/// <reference path="JSExtensions.js" /> 

this.OSVersions = null;
this.IOModes = null;
this.RoboCopyExitCodes = null;
this.XCopyUNCTarget = null;

this.LoggedError = null;
this.Storage = null;
this.Logger = null;
this.Shell = null;
this.Utils = {};
(function () {

    //General Exceptions
    LoggedError = (function () {
        function constructor(message, tag) {
            var toReturn = Utils.CreateObject(new Error(message), constructor);
            toReturn.logged = true;
            toReturn.tag = tag;
            return;
        }
        return constructor;
    }());

    this.CreateObject = function (prototype, constructor) {
        function newObjFunc() { };
        newObjFunc.prototype = prototype;
        if (constructor != undefined) {
            newObjFunc.constructor = constructor;
        }
        return new newObjFunc();
    };

    this.GetChakraToolRoot = function(shell) {
        return shell.getEnv("SDXROOT") + "\\inetcore\\jscript\\tools\\ChakraTool";
    }

    this.CreateEnum = function (obj, flagBased) {
        if (flagBased === undefined) {
            flagBased = false;
        }

        var valueToProp = {};
        var toReturn = Utils.CreateObject({
            isValid: function (value) {
                if (flagBased) {
                    for (var eValue in valueToProp) {
                        if (this.isSet(value, eValue)) {
                            return true;
                        }
                    }
                    return false;
                }
                return valueToProp.hasOwnProperty(value);
            },
            getName: function (value) {
                if (flagBased) {
                    var multiple = [];
                    for (var eValue in valueToProp) {
                        if (this.isSet(value, eValue)) {
                            multiple.push(valueToProp[eValue]);
                        }
                    }
                    return multiple.join("|");
                }

                return valueToProp[Number.prototype.toString.call(value)];
            }, isSet: function (one, other) {
                return flagBased ? ((Number(one) & Number(other)) !== 0 || Number(one) === Number(other) /*For the 0 case*/) : Number(one) === Number(other);
            },
            getInstance: function (value) {
                value = Object(value);
                if (!this.isValid(value)) {
                    throw new Error("Invalid enum value of '{0}'.".format(value));
                }

                for (var prop in obj) {
                    value["is{0}".format(prop)] = this.isSet(value, obj[prop]);
                }

                var that = this;
                value.toString = function () { return that.getName(value); };
                value.equals = function (other) { return that.isSet(value, other); };
                return value;
            }
        });

        for (var prop in obj) {
            var value = flagBased ? Number(obj[prop]) : obj[prop];
            toReturn[prop] = value;
            valueToProp[value] = prop;
        }

        return toReturn;
    };

    //Objects
    Shell = (function () {
        OSVersions = Utils.CreateEnum({
            Win7: "6.1",
            Win8: "6.2",
            WinBlue: "6.3",
            WinTH: "10.0"
        });

        XCopyUNCTarget = Utils.CreateEnum({
            File: "F",
            Directory: "D"
        })

        XCopyExitCodes = Utils.CreateEnum({
            //Files were copied without error.
            Success: 0,

            //No files were found to copy.
            NoFiles: 1,

            //The user pressed CTRL+C to terminate xcopy.
            Terminated: 2,

            //Initialization error occurred. There is not enough memory or disk space, or you entered an invalid drive name or invalid syntax on the command line.
            InitializationError: 4,

            //Disk write error occurred.
            DiskWriteError: 5
        });

        RoboCopyExitCodes = Utils.CreateEnum({
            //Some error happened, 16, 8, 4
            Error: 28,

            //Serious error. Robocopy did not copy any files.
            //Either a usage error or an error due to insufficient access privileges
            //on the source or destination directories.
            SeriousError: 16,
            
            //Some files or directories could not be copied
            //(copy errors occurred and the retry limit was exceeded).
            //Check these errors further.
            SomeFailed: 8,

            //Some Mismatched files or directories were detected.
            //Examine the output log. Some housekeeping may be needed.
            SomeMismatched: 4,

            //Some Extra files or directories were detected.
            //Examine the output log for details. 
            SomeExtra: 2,

            //One or more files were copied successfully (that is, new files have arrived).
            OneOrMoreSucceeded: 1,

            //No errors occurred, and no copying was done.
            //The source and destination directory trees are completely synchronized. 
            NothingHappened: 0
        }, true);

        function constructor() {
            var wsShellObject = WScript.CreateObject("WScript.Shell");
            var stdOutWriter = arguments.length > 0 ? arguments[0] : undefined;
            var stdErrWriter = arguments.length > 1 ? arguments[1] : undefined;
            var env = wsShellObject.Environment("Process");
            var envVariablesForRestore = {};
            var directoryForRestore = wsShellObject.CurrentDirectory;
            var disposed = false;
            var logger = undefined;
            var version = undefined;
            var pushedDirectories = [];

            return Utils.CreateObject({
                getCurrentDirectory: function () {
                    if (disposed) {
                        throw new Error("Shell has already been disposed.");
                    }
                    return wsShellObject.CurrentDirectory;
                }, setCurrentDirectory: function (path) {
                    if (disposed) {
                        throw new Error("Shell has already been disposed.");
                    }
                    wsShellObject.CurrentDirectory = String(path);
                }, pushDirectory: function (path) {
                    var currentDir = this.getCurrentDirectory();
                    this.setCurrentDirectory(path);
                    pushedDirectories.push(currentDir);
                }, popDirectory: function () {
                    this.setCurrentDirectory(pushedDirectories.pop());
                }, setStdOutWriter: function (writer) {
                    if (disposed) {
                        throw new Error("Shell has already been disposed.");
                    }
                    stdOutWriter = writer;
                }, setStdErrWriter: function (writer) {
                    if (disposed) {
                        throw new Error("Shell has already been disposed.");
                    }
                    stdErrWriter = writer;
                }, getEnv: function (key) {
                    if (disposed) {
                        throw new Error("Shell has already been disposed.");
                    }
                    return env(key);
                }, setEnv: function (key, value) {
                    if (disposed) {
                        throw new Error("Shell has already been disposed.");
                    } else if (value !== env(key) && !envVariablesForRestore.hasOwnProperty(key)) {
                        envVariablesForRestore[key] = env(key);
                    }
                    env(key) = value;
                }, getOSVersion: function () {
                    if (version) {
                        return version;
                    }
                    version = OSVersions.getInstance(/[\s\S]*\[Version (.*)\.[0-9]*\]/.exec(this.executeToObject("ver"))[1]);
                    return version;
                }, xCopy: function (source, destination, rawOptions, uncTarget) {

                    if (uncTarget === undefined)
                    {
                        uncTarget = XCopyUNCTarget.File;
                    }

                    var command = "echo {3} | xcopy {0} {1} {2}".format(source, destination, rawOptions, uncTarget);
                    
                    if (stdOutWriter !== undefined) {
                        stdOutWriter("\r\nExecuting: '{0}'\r\n".format(command));
                    }

                    var exitCode = this.execute(command);

                    var toReturn = XCopyExitCodes.getInstance(exitCode);

                    if (stdOutWriter !== undefined) {
                        stdOutWriter("\r\nxCopy Returned: '{0}'\r\n\r\n".format(toReturn));
                    }

                    return toReturn;
                }, roboCopy: function (source, destination, rawOptions) {
                    //This wrapper is simple, the useful part is the enum it returns.
                    var command = "robocopy {0} {1} {2}".format(source, destination, rawOptions);

                    if (stdOutWriter !== undefined) {
                        stdOutWriter("\r\nExecuting: '{0}'\r\n".format(command));
                    }
                    
                    var exitCode = this.execute(command);
                    
                    var toReturn = RoboCopyExitCodes.getInstance(exitCode);

                    if (stdOutWriter !== undefined) {
                        stdOutWriter("\r\nRoboCopy Returned: '{0}'\r\n\r\n".format(toReturn));
                    }

                    return toReturn;
                }, execute: function (command, timeOut) {
                    if (disposed) {
                        throw new Error("Shell has already been disposed.");
                    }

                    var fileName = "temp{0}.js".format(new Date().getTime());
                    var file = Storage.GetNonTrackedFile(Storage.FromRelativePath(fileName), IOModes.ForWriting);
                    try{
                        var execObject = wsShellObject.Exec('cmd /c "{0}"'.format(command));
                    
                        if (timeOut === undefined) {
                            timeOut = 60 * 60 * 1000; // 60 Minutes in milliseconds
                        }

                        // The script below will be run in a different process to monitor the command we are executing.
                        // This is impossible to do in the current function/process, because we are blocking on reading StdOut and StdErr 
                        //     in order to support real time printing of output.
                        // The monitor script will terminate the command above after the elapse of the timeout.
                        var monitorScript = "\
                            var endDate = new Date(new Date().getTime() + {0}).valueOf();\r\n\
                            var dateNow = new Date().valueOf();\
                            var pid = {1};\r\n\
                            while (true) {\r\n\
                            if (dateNow > endDate) {\r\n\
                                WScript.StdErr.Write('Exec: timed out.\\r\\n');\r\n\
                                WScript.CreateObject('WScript.Shell').Exec('kill -f ' + pid);\r\n\
                                WScript.Quit(1);\r\n\
                            }\r\n\
                            WScript.Sleep(100);\
                            dateNow += 100;\
                            }".format(timeOut, execObject.ProcessID);

                        file.write(monitorScript);
                        file.dispose();

                        var monitorObject = wsShellObject.Exec('cscript //nologo "{0}" 2>&1'.format(file));

                        while (execObject.Status === 0) {
                            while (!execObject.StdOut.AtEndOfStream) {
                                if (stdOutWriter !== undefined) {
                                    stdOutWriter(execObject.StdOut.ReadLine() + "\r\n");
                                } else {
                                    execObject.StdOut.ReadAll();
                                }
                            }
                            while (!execObject.StdErr.AtEndOfStream) {
                                if (stdErrWriter !== undefined) {
                                    stdErrWriter(execObject.StdErr.ReadLine() + "\r\n");

                                } else {
                                    execObject.StdErr.ReadAll();
                                }
                            }
                        }
                        if (monitorObject.Status !== 0) {
                            stdErrWriter(monitorObject.StdOut.ReadAll());
                            throw new Error("Script timed out!");
                        } else {
                            try {
                                var killCommand = "taskkill /T /F /PID {0}".format(monitorObject.ProcessID);
                                var exitCode = wsShellObject.Exec(killCommand).exitCode;
                            } catch (ex) {
                                throw new Error("Failed execute kill command: '{0}'".format(killCommand));
                            }
                        }

                        try {
                            file.deleteFile();
                        } catch (ex) {
                            if (stdErrWriter !== undefined) {
                                stdErrWriter("Shell.execute: Failed to delete temporrary file, message: " + ex);
                            }
                        }
                        return execObject.exitCode;
                    }
                    catch (ex) {
                        if (stdErrWriter !== undefined) {
                            stdErrWriter("::ERROR: Shell.execute: An error happened, message: {0}".format(ex.message));
                        }
                        try {
                            file.deleteFile();
                        } catch (ex) {
                            if (stdErrWriter !== undefined) {
                                stdErrWriter("::ERROR: Shell.execute: Failed to delete temporrary file, message: " + ex);
                            }
                        }
                        throw ex;
                    }
                }, executeToObject: function (command) {
                    if (disposed) {
                        throw new Error("Shell has already been disposed.");
                    }

                    var execObject = wsShellObject.Exec("cmd /c \"" + command + "\" 2>&1");

                    var toReturn = "";
                    while (execObject.Status === 0) {
                        while (!execObject.StdOut.AtEndOfStream) {
                            toReturn += execObject.StdOut.ReadAll();
                        }
                    }
                    return toReturn;
                }, run: function (command, windowStyle, timeOut) {
                    throw new Error("Not implemented.");
                    var wait = false;
                    if (windowStyle === undefined) {
                        windowStyle = 5;
                    }
                    var endDate = undefined;
                    //Special case of timeout
                    if (timeOut === 0) {
                        wait = true;
                    } else {

                        if (timeOut === undefined) {
                            timeOut = 60 * 60 * 1000; // 60 Minutes in milliseconds
                        }

                        endDate = new Date(new Date().getTime() + timeOut);
                    }
                    

                    function checkTimeOut() {
                        if (new Date() > endDate) {
                            stdErrWriter("Exec: timed out.\r\n");
                            return true;
                        }
                        return false;
                    }

                    //Window Styles
                    //0 Hide the window and activate another window. 
                    //1 Activate and display the window. (restore size and position) Specify this flag when displaying a window for the first time. 
                    //2 Activate & minimize. 
                    //3 Activate & maximize. 
                    //4 Restore. The active window remains active. 
                    //5 Activate & Restore. 
                    //6 Minimize & activate the next top-level window in the Z order. 
                    //7 Minimize. The active window remains active. 
                    //8 Display the window in its current state. The active window remains active. 
                    //9 Restore & Activate. Specify this flag when restoring a minimized window. 
                    //10 Sets the show-state based on the state of the program that started the application.
                    if (disposed) {
                        throw new Error("Shell has already been disposed.");
                    }
                    var execObject = null;
                    if (wait) {
                        execObject = wsShellObject.Run(command, windowStyle, true);
                    } else {
                        execObject = wsShellObject.Run(command, windowStyle, false);
                        while (true) {
                            if (checkTimeOut()) {
                                execObject.Terminate();
                                throw new Error("Timed out.");
                            }
                            WScript.sleep(100);
                        }
                    }
                    while (!execObject.StdOut.AtEndOfStream) {
                        if (stdOutWriter !== undefined) {
                            stdOutWriter(execObject.StdOut.ReadLine() + "\r\n");
                        } else {
                            execObject.StdOut.ReadAll();
                        }
                    }
                    while (!execObject.StdErr.AtEndOfStream) {
                        if (stdErrWriter !== undefined) {
                            stdErrWriter(execObject.StdErr.ReadLine() + "\r\n");

                        } else {
                            execObject.StdErr.ReadAll();
                        }
                    }
                }, popup: function (text, title, onAccept, onCancel, onReject) {
                    throw new Error("Not Implemented.");
                    //http://msdn.microsoft.com/en-us/library/x83z1d9f(v=vs.84).aspx
                    if (disposed) {
                        throw new Error("Shell has already been disposed.");
                    }
                    //    Button Types

                    //    Value Description 
                    //    0   OK button. 
                    //    1   OK and Cancel buttons. 
                    //    2   Abort, Retry, and Ignore buttons. 
                    //    3   Yes, No, and Cancel buttons. 
                    //    4   Yes and No buttons. 
                    //    5   Retry and Cancel buttons. 

                    //Icon Types

                    //    Value Description 
                    //    16  "Stop Mark" icon. 
                    //    32  "Question Mark" icon. 
                    //    48  "Exclamation Mark" icon. 
                    //    64  "Information Mark" icon. 

                    // Returns
                    //1  OK button 
                    //2  Cancel button 
                    //3  Abort button 
                    //4  Retry button 
                    //5  Ignore button 
                    //6  Yes button 
                    //7  No button

                    return wsShellObject.Popup(text, 0, title, popupType);
                }, dispose: function () {
                    if (disposed) {
                        throw new Error("Shell has already been disposed.");
                    }
                    disposed = true;

                    for (var key in envVariablesForRestore) {
                        env(key) = envVariablesForRestore[key];
                    }
                    wsShellObject.CurrentDirectory = directoryForRestore;
                }, constructor: constructor
            });
        }
        return constructor;
    }());

    Storage = (function () {
        //http://msdn.microsoft.com/en-us/library/08xz3c5a(v=vs.84).aspx for Reference
        var fileSystem = WScript.CreateObject("Scripting.FileSystemObject");
        var internalRef = function () { };

        var Directory = (function () {
            function dirConstructor(path, attached) {
                path = String(path);
                if (path.charAt(path.length - 1) === '\\') {
                    path = path.substr(0, path.length - 1);
                }

                var folderObject  = undefined;
                if (attached) {
                    folderObject = constructor.DoesDirectoryExist(path) ? fileSystem.GetFolder(path) : constructor.CreateDirectory(path);
                }
                var objectsTracked = [];
                var tmpFiles = [];

                return Utils.CreateObject({
                    attach: function (storageObject) {
                        if (attached) {
                            return;
                        }
                        folderObject = constructor.DoesDirectoryExist(path) ? fileSystem.GetFolder(path) : constructor.CreateDirectory(path);
                        storageObject.attach(this, internalRef);
                        attached = true;
                    }, deleteDirectory: function (force) {
                        objectsTracked.forEach(function (item) { item.dispose(); });
                        folderObject.Delete(force);
                    }, cleanDirectory: function (force) {
                        objectsTracked.forEach(function (item) { item.dispose(); });
                        folderObject.Delete(force);
                        folderObject = constructor.CreateDirectory(path);
                    }, openFile: function (fileName, ioMode) {
                        if (!attached) {
                            throw new Error("Directory is not attached to a storage object.");
                        }
                        fileName = String(fileName);
                        if (fileName.contains("\\")) {
                            throw new Error("[Directory Object].getFile: File name provided '{0}' can't contain '\\' characters.".format(fileName));
                        }
                        var fullPath = path + "\\" + fileName;

                        if (!constructor.DoesFileExist(fullPath)) {
                            return undefined;
                        }

                        var toReturn = new File(fullPath, ioMode, true);
                        objectsTracked.push(toReturn);
                        return toReturn;
                    }, getDirectory: function (dirPath) {
                        if (!attached) {
                            throw new Error("Directory is not attached to a storage object.");
                        }
                        dirPath = String(dirPath);
                        if (dirPath.contains("\\")) {
                            throw new Error("[Directory Object].getFile: Directory name provided '{0}' can't contain '\\' characters.".format(fileName));
                        }
                        var toReturn = new Directory("{0}\\{1}".format(path, dirPath), true);
                        objectsTracked.push(toReturn);
                        return toReturn;
                    }, getFile: function (fileName, ioMode) {
                        if (!attached) {
                            throw new Error("Directory is not attached to a storage object.");
                        }
                        fileName = String(fileName);
                        if (fileName.contains("\\")) {
                            throw new Error("[Directory Object].getFile: File name provided '{0}' can't contain '\\' characters.".format(fileName));
                        }
                        var toReturn = new File(path + "\\" + fileName, ioMode, true);
                        objectsTracked.push(toReturn);
                        return toReturn;
                    }, getNewFile: function (fileName, ioMode) {
                        if (!attached) {
                            throw new Error("Directory is not attached to a storage object.");
                        }
                        fileName = String(fileName);
                        if (fileName.contains("\\")) {
                            throw new Error("[Directory Object].getFile: File name provided '{0}' can't contain '\\' characters.".format(fileName));
                        }
                        var filePath = path + "\\" + fileName;

                        if (constructor.DoesFileExist(filePath)) {
                            constructor.DeleteFile(filePath);
                        }
                        var toReturn = new File(filePath, ioMode, true);
                        objectsTracked.push(toReturn);
                        return toReturn;
                    }, getTempFile: function () {
                        if (!attached) {
                            throw new Error("Directory is not attached to a storage object.");
                        }
                        var toReturn = this.getFile(new Date().toFileNameString() + ".temp", IOModes.ForWriting);
                        tmpFiles.push(toReturn);
                        return toReturn;
                    }, getConfigFromFile: function (configFileName, baseConfigObject, shell) {
                        if (!attached) {
                            throw new Error("Directory is not attached to a storage object.");
                        }
                        var file = this.getFile(configFileName, IOModes.ForReading);
                        var toReturn = Utils.CreateObject(baseConfigObject);
                        Utils.ApplyConfigOn(Utils.ParseJSON(file.readAll()), toReturn, {}, shell);
                        file.dispose();
                        return toReturn;
                    }, dispose: function () {
                        objectsTracked.forEach(function (item) { item.dispose(); });
                        tmpFiles.forEach(function (item) { item.deleteFile(); });
                    }, toString: function () {
                        return path;
                    }, constructor: dirConstructor
                });
            }
            return dirConstructor;
        }());

        var File = (function () {
            function fileConstructor(path, ioMode, attached) {
                path = String(path);
                if (!IOModes.isValid(ioMode)) {
                    throw new Error("Invalid IO mode: {0}".format(ioMode));
                }
                if (ioMode === IOModes.ReadWrite) {
                    throw new Error("Not yet supported.");
                }
                var path = path;
                var disposed = false;
                var deleted = false;

                var fileObject = undefined;

                // If a file is detached, this object is a shell until the file is attached to a storage object.
                if (attached) {
                    try{
                        fileObject = fileSystem.OpenTextFile(path, ioMode, true, 0/* unicode = false */);
                    } catch (ex) {
                        throw new Error("Failed to open text file '{0}' with mode: {1}".format(path, IOModes.getName(ioMode)));
                    }
                }

                return Utils.CreateObject({
                    attach: function (storage) {
                        if (disposed) {
                            throw new Error("File object disposed.");
                        }
                        if (attached) {
                            throw new Error("File is already attached to a storage object.");
                        }
                        storage.attach(this, internalRef);
                        attached = true;
                        this.reOpen(ioMode, true);
                    }, read: function (numChars) {
                        if (disposed) {
                            throw new Error("File object disposed.");
                        }
                        if (!attached) {
                            throw new Error("File is not attached to a storage object, call attach(storageObject).");
                        }
                        return fileObject.Read(numChars);
                    }, readLine: function () {
                        if (disposed) {
                            throw new Error("File object disposed.");
                        }
                        if (!attached) {
                            throw new Error("File is not attached to a storage object, call attach(storageObject).");
                        }
                        return fileObject.ReadLine();
                    }, readAll: function () {
                        if (disposed) {
                            throw new Error("File object disposed.");
                        }
                        if (!attached) {
                            throw new Error("File is not attached to a storage object, call attach(storageObject).");
                        }
                        return fileObject.ReadAll();
                    }, write: function (str) {
                        if (disposed) {
                            throw new Error("File object disposed.");
                        }
                        if (!attached) {
                            throw new Error("File is not attached to a storage object, call attach(storageObject).");
                        }
                        fileObject.Write(str);
                    }, writeLine: function (str) {
                        if (disposed) {
                            throw new Error("File object disposed.");
                        }
                        if (!attached) {
                            throw new Error("File is not attached to a storage object, call attach(storageObject).");
                        }
                        fileObject.WriteLine(str);
                    }, dispose: function () {
                        if (disposed) {
                            return;
                        }
                        dispose = true;
                        fileObject.Close();
                    }, deleteFile: function (force) {
                        if (deleted) {
                            return;
                        }
                        deleted = true;
                        this.dispose();
                        try{
                            fs.DeleteFile(path);
                        } catch (ex) {
                            throw new Error("Failed to delete file '{0}'.".format(path));
                        }
                    }, flush: function () {
                        if (disposed) {
                            throw new Error("File object disposed.");
                        }
                        if (!attached) {
                            throw new Error("File is not attached to a storage object, call attach(storageObject).");
                        }
                        // This is a bit hacky, but no other way of doing it
                        this.reOpen(IOModes.ForAppending, true);
                    }, reOpen: function (newIoMode, force) {
                        if (disposed) {
                            throw new Error("File object disposed.");
                        }
                        if (!attached) {
                            throw new Error("File is not attached to a storage object, call attach(storageObject).");
                        }
                        if (newIoMode === ioMode & !force) {
                            return;
                        }
                        ioMode = newIoMode;

                        if (fileObject !== undefined) {
                            fileObject.Close();
                        } 
                        try {
                            fileObject = fileSystem.OpenTextFile(path, ioMode, true, 0/* unicode = true */);
                        } catch (ex) {
                            throw new Error("Failed to open text file '{0}' with mode: {1}".format(path, IOModes.getName(newIoMode)));
                        }
                    }, reOpenForReading: function () {
                        if (disposed) {
                            throw new Error("File object disposed.");
                        }
                        if (!attached) {
                            throw new Error("File is not attached to a storage object, call attach(storageObject).");
                        }
                        this.reOpen(IOModes.ForReading, false);
                    }, reOpenForWriting: function (ioMode) {
                        if (disposed) {
                            throw new Error("File object disposed.");
                        } if (!attached) {
                            throw new Error("File is not attached to a storage object, call attach(storageObject).");
                        }
                        this.reOpen(IOModes.ForWriting, false);
                    }, constructor: fileConstructor, toString: function () {
                        return path;
                    }, CanRead: function () {
                        return !fileObject.AtEndOfStream;
                    }
                });
            }
            return fileConstructor;
        }());

        constructor.IsFile = function (obj) {
            return obj.constructor === File;
        }

        constructor.IsDirectory = function (obj) {
            return obj.constructor === Directory;
        }

        constructor.DoesFileExist = function (fullPath) {
            return fileSystem.FileExists(String(fullPath));
        };
        constructor.DoesDirectoryExist = function (fullPath) {
            return fileSystem.FolderExists(String(fullPath));
        };
        constructor.DeleteFile = function (fullPath) {
            fileSystem.DeleteFile(fullPath);
        };
        constructor.CreateDirectory = function (fullPath) {
            fullPath = String(fullPath);

            var isUNCPath = false;
            if (fullPath.length > 0 && fullPath.charAt(0) === '\\' && fullPath.charAt(1) === '\\') {
                isUNCPath = true;
            }

            var parts = (isUNCPath ? fullPath.substring(2, fullPath.length) : fullPath).split("\\");
            var currentPath = isUNCPath ? "\\\\" + parts[0] + "\\" : "";
            for (var i = isUNCPath ? 1 : 0; i < parts.length; i++) {
                currentPath += parts[i] + "\\";
                if (!constructor.DoesDirectoryExist(currentPath)) {
                    try {
                        fileSystem.CreateFolder(currentPath);
                    } catch (ex) {
                        throw new Error("Failed to create folder '{0}'.".format(currentPath));
                    }
                }
            }
            return fileSystem.GetFolder(fullPath);
        };
        constructor.CopyFile = function (source, destination, overwrite) {
            fileSystem.CopyFile(source, destination, overwrite);
        };
        constructor.CopyDirectory = function (source, destination, overwrite) {
            fileSystem.CopyFolder(source, detination, overwrite);
        }
        constructor.TestHostRootDirectory = (function () {
            return String(WScript.ScriptFullName).replace("\\" + WScript.ScriptName, "");
        }());
        constructor.FromRelativePath = function (relativePath) {
            return constructor.TestHostRootDirectory + "\\" + relativePath
        };
        IOModes = Utils.CreateEnum({
            ForReading: 1,
            ForWriting: 2,
            ForAppending: 8
            //ReadWrite: 4
        });
        constructor.GetNonTrackedFile = function (path, ioMode) {
            return new File(path, ioMode, true);
        };
        constructor.GetNonTrackedDirectory = function (path) {
            return new Directory(path, true);
        };
        constructor.GetDetachedFile = function (path, ioMode) {
            return new File(path, ioMode, false);
        };
        constructor.GetDetachedDirectory = function (path) {
            return new Directory(path, false);
        };
        constructor.Directory = Directory;
        constructor.File = File;

        function constructor(basePath, createIfNotExists) {
            var currentDirectory = undefined;
            if (Storage.IsDirectory(basePath)) {
                currentDirectory = basePath;
            } else {
                var basePath = String(basePath);
                if (!constructor.DoesDirectoryExist(basePath)) {
                    if (createIfNotExists) {
                        constructor.CreateDirectory(basePath);
                    } else {
                        throw new Error("Base path provided '{0}' is not a directory.".format(basePath));
                    }
                }
                if (basePath.charAt(basePath.length - 1) === '\\') {
                    basePath = basePath.substr(0, basePath.length - 1);
                }

                function ifRelativeMakeAbsolutePath(path) {
                    path = String(path);
                    switch (path.charAt(0)) {
                        case '\\':
                            // This is treated as root or network reference
                            return path;
                            break;
                        case '.':
                        case '~':
                            if (path.charAt(1) === '\\') {
                                return basePath + path.substr(1);
                            }
                            break;
                        default:
                            if (path.charAt(1) === ':') {
                                //Absolute path
                                return path;
                            }
                            return basePath + "\\" + path;
                            break;
                    }
                    throw new Error("The path '{0}' isn't recognized to be relative or absolute.".format(path));
                }
                currentDirectory = new Directory(basePath, true);
            }
            var objectsTracked = [];
            var tmpDirs = [];
            return Utils.CreateObject({
                attach: function (fileOrDirectory, internalReference) {
                    if (internalReference === internalRef) {
                        objectsTracked.push(fileOrDirectory);
                    } else {
                        fileOrDirectory.attach(this);
                    }
                    return fileOrDirectory;
                }, getFile: function (path, ioMode) {
                    path = String(path);
                    if (path.contains("\\")) {
                        var absolutePath = ifRelativeMakeAbsolutePath(path);
                        var toReturn = new File(absolutePath, ioMode, true);
                        objectsTracked.push(toReturn);
                        return toReturn;
                    } else {
                        return currentDirectory.getFile(path, ioMode)
                    }
                }, getTempFile: currentDirectory.getTempFile,
                getConfigFromFile: currentDirectory.getConfigFromFile,
                openFile: currentDirectory.openFile,
                getTempDirectory: function () {
                    var toReturn = this.getDirectory("TEMP_" + new Date().toFileNameString());
                    tmpDirs.push(toReturn);
                    return toReturn;
                }, getDirectory: function (relativeOrAbsolutePath) {
                    var absolutePath = ifRelativeMakeAbsolutePath(relativeOrAbsolutePath);
                    var toReturn = new Directory(absolutePath, true);
                    objectsTracked.push(toReturn);
                    return toReturn;
                }, dispose: function () {
                    tmpDirs.forEach(function (item) { item.deleteDirectory(); });
                    objectsTracked.forEach(function (item) { item.dispose(); });
                    currentDirectory.dispose();
                }, toString: function () {
                    return basePath;
                },
                doesFileExist: constructor.DoesFileExist,
                doesDirectoryExist: constructor.DoesDirectoryExist,
                copyFile: constructor.copyFile,
                copyDirectory: constructor.CopyDirectory,
                constructor: constructor
            });
        }
        return constructor;
    }());

    Logger = (function () {
        var tag = "LoggerError"
        function constructor(config, logStorage, testSuiteName, stdOutWriter, stdErrWriter, clearOutputDirectory) {
            var isForTests = testSuiteName !== undefined ? true : false;
            var traceActions = {};
            var traceErrors = {};
            function parseTraceFilter(flag, objToSetOn) {
                for (var i = 0; i < flag.length; i++) {
                    var traceItem = flag[i];

                    if (isForTests) {
                        if (traceItem.testSuite !== undefined && traceItem.testSuite !== "*" && traceItem.testSuite.toLowerCase() !== testSuiteName.toLowerCase()) {
                            continue;
                        } else if (traceItem.testSuite === undefined) {
                            continue;
                        }
                    }

                    if (traceItem.tag === "*") {
                        objToSetOn.hasOwnProperty = function () { return true; };
                    } else {
                        objToSetOn[traceItem.tag] = true;
                    }
                }
            }
            
            var outputDirectory = config.baseLogDirectory + (isForTests ? "\\" + testSuiteName + "TestSuite\\" + config.testOutputRelativePath : "")
            
            var outputDir = logStorage.getDirectory(outputDirectory);

            if (clearOutputDirectory) {
                outputDir.deleteDirectory();
                outputDir.dispose();
                outputDir = logStorage.getDirectory(outputDirectory);
            }

            var file = outputDir.getFile(config.verboseLogFileName, IOModes.ForWriting);
            file.writeLine("Log for: " + new Date().toString());
            file.dispose();
            file = outputDir.getFile(config.errorLogFileName, IOModes.ForWriting);
            file.writeLine("Log for: " + new Date().toString());
            file.dispose();

            var testOutputFileNameFormat = config.testOutputFileNameFormat;
            var testErrorLogFileNameFormat = config.testErrorLogFileNameFormat;
            var testSummaryLogFileNameFormat = config.testSummaryLogFileNameFormat;

            parseTraceFilter(config.trace, traceActions);
            parseTraceFilter(config.traceErrors, traceErrors);
            var traceTestOutput = config.traceTestOutput;
            var traceTestSummary = config.traceTestSummary;
            var traceTestErrors = config.traceTestErrors;

            var testOutputFile = undefined;
            var testErrorFile = undefined;
            var testSummaryFile = undefined;

            var testSummaryOutput = "\r\nTest summary for {0} test script:\r\n\r\n".format(testSuiteName);

            function writeToLogFile(output) {
                var file = outputDir.getFile(config.verboseLogFileName, IOModes.ForAppending);
                file.write(output);
                file.dispose();
            }

            function writeToErrorFile(output) {
                var file = outputDir.getFile(config.errorLogFileName, IOModes.ForAppending);
                file.write(output);
                file.dispose();
            }

            function logError(message, tag) {
                message = message === undefined || message === "" ? " " : message;
                if (tag !== undefined) {
                    var output = "::ERROR: {2}{0} - {1}".format(tag, message, isForTests ? "{0}.".format(testSuiteName.toFirstUpper()) : "");
                    if (traceErrors.hasOwnProperty(tag)) {
                        stdErrWriter(output);
                    }
                    writeToErrorFile(output);
                } else {
                    stdErrWriter(message);
                    writeToErrorFile(message);
                }
            }

            function log(message, tag) {
                message = message === undefined || message === "" ? " " : message;
                if (tag !== undefined) {
                    var output = "LogItem: {2}{0} - {1}".format(tag, message, isForTests ? "{0}.".format(testSuiteName.toFirstUpper()) : "");

                    if (traceActions.hasOwnProperty(tag)) {
                        stdOutWriter(output);
                    }
                    writeToLogFile(output);
                } else {
                    stdOutWriter(message);
                    writeToLogFile(message);
                }
            }

            function logTestDataTo(file, data) {
                if (!isForTests) {
                    logError("logTestDataTo: Attempted to log data for a test run on a non-test logger. \r\n", tag);
                    throw new LoggedError(tag, "Attempted to log data for a test run on a non-test logger.", tag);
                }

                if (file === undefined) {
                    logError("logTestDataTo: Attempted to log data for a test run, while 'startRun' wasn't called. \r\n", tag);
                    throw new LoggedError("Attempted to log data for a test run, while 'startRun' wasn't called.", tag);
                }

                file.write(data);
            }

            var flushPerTestWrite = false;

            return Utils.CreateObject({
                log: log,
                logLine: function (msg, tag) {
                    msg = msg === undefined ? "" : msg;
                    log(msg + "\r\n", tag);
                },
                logRaw: writeToLogFile,
                logError: logError,
                logErrorLine: function (msg, tag) {
                    msg = msg === undefined ? "" : msg;
                    logError(msg + "\r\n", tag);
                },
                logErrorRaw: writeToErrorFile,
                logException: function (tag, ex){
                    logError(tag, "Exception Occured - Message: {0}".format(ex.description));
                },
                startRun: function (runLabel) {
                    if (!isForTests) {
                        this.logErrorLine("startRun: Attempted to start a test run on a non-test logger.", tag);
                        throw new LoggedError("Attempted to start a test run on a non-test logger.", tag);
                    }
                    if (testOutputFile !== undefined || testErrorFile !== undefined || testSummaryFile !== undefined) {
                        this.logErrorLine("startRun: Attempted to start a test run, while 'finishRun' wasn't called for previous run.", tag);
                        throw new LoggedError("Attempted to start a test run, while 'finishRun' wasn't called for previous run.", tag);
                    }

                    testOutputFile = outputDir.getNewFile(testOutputFileNameFormat.format(runLabel), IOModes.ForWriting);
                    testErrorFile = outputDir.getNewFile(testErrorLogFileNameFormat.format(runLabel), IOModes.ForWriting);
                    testSummaryFile = outputDir.getNewFile(testSummaryLogFileNameFormat.format(runLabel), IOModes.ForWriting);

                    testOutputFile.writeLine("Log for: " + new Date().toString());
                    testErrorFile.writeLine("Log for: " + new Date().toString());
                    testSummaryFile.writeLine("Log for: " + new Date().toString());

                    testSummaryOutput = "{0}Summary for '{1}': \r\n".format(testSummaryOutput, runLabel);
                },
                finishRun: function () {
                    if (!isForTests) {
                        this.logErrorLine("finishRun: Attempted to finish a test run on a non-test logger.", tag);
                        throw new LoggedError("Attempted to finish a test run on a non-test logger.", tag);
                    }
                    if (testOutputFile === undefined || testErrorFile === undefined || testSummaryFile === undefined) {
                        this.logErrorLine("finishRun: Attempted to finish a test run, while 'startRun' wasn't called.", tag);
                        throw new LoggedError("Attempted to finish a test run, while 'startRun' wasn't called.", tag);
                    }

                    testOutputFile.dispose();
                    testErrorFile.dispose();
                    testSummaryFile.dispose();

                    testSummaryOutput = "{0}\r\nEnd of summary.\r\n".format(testSummaryOutput);

                    testOutputFile = testErrorFile = testSummaryFile = undefined;
                },
                logTestOutput: function (data) {
                    if (Storage.IsFile(data)) {
                        data = data.readAll();
                    }
                    if (traceTestOutput) {
                        stdOutWriter(data);
                    }
                    logTestDataTo(testOutputFile, data);
                    if (flushPerTestWrite) {
                        testOutputFile.flush();
                    }
                },
                logTestError: function (data) {
                    if (Storage.IsFile(data)) {
                        data = data.readAll();
                    }
                    if (traceTestErrors) {
                        stdErrWriter(data);
                    }
                    logTestDataTo(testErrorFile, data);
                    if (flushPerTestWrite) {
                        testErrorFile.flush();
                    }
                },
                logTestSummary: function (data) {
                    if (Storage.IsFile(data)) {
                        data = data.readAll();
                    }

                    testSummaryOutput += data;

                    if (traceTestSummary) {
                        stdOutWriter(data);
                    }

                    logTestDataTo(testSummaryFile, data);

                    if (flushPerTestWrite) {
                        testSummaryFile.flush();
                    }

                }, getCompleteTestSummary: function () {
                    return testSummaryOutput;
                },
                setFlushPerWrite: function (value) {
                    flushPerTestWrite = value;
                }, constructor: constructor
            });
        }
        return constructor;
    }());


    var WshShell = WScript.CreateObject("WScript.Shell");
    var fs = WScript.CreateObject("Scripting.FileSystemObject");
    var env = WshShell.Environment("PROCESS");

    function parseArg(arg, argObject) {
        if (arg.charAt(0) !== '-' || arg.length === 1) {
            throw new Error("Each argument must begin with '-' and can't follow it with an empty string, this one doesn't: '" + arg + "'!");
        }
        var collonIndex = arg.indexOf(':', 0);
        if (collonIndex == -1) {
            var parts = arg.split('-');
            if (parts.length > 3 || parts[0] !== "" || (parts.length == 3 && parts[2] !== "")) {
                throw new Error("Each argument must at most contain 2 of '-', one at the start and one at the end, this one doesn't: '" + arg + "'!");
            }
            argObject[parts[1]] = parts.length == 2 ? true : false;
        }
        else {
            //Contains a value
            var key = arg.substring(1, collonIndex);
            var value = arg.substring(collonIndex + 1, arg.length);
            argObject[key] = value === "undefined" ? undefined : value;
        }
    }

    this.ParseArguments = function (str, quoteForStrings) {
        var argObject = {};
        var argArray = [];
        var insideQuote = false;
        var lastStartIndex = 0;
        for (var j = 0; j < str.length; j++) {
            switch (str.charAt(j)) {
                case " ":
                    if (!insideQuote && lastStartIndex !== j - 1) {
                        argArray.push(str.substring(lastStartIndex, j).replace(new RegExp(quoteForStrings, "g"), ""));
                        lastStartIndex = j + 1;
                    }
                    break;
                case quoteForStrings:
                    insideQuote = !insideQuote;
                    break;
                default:
                    break;
            }
        }
        if (insideQuote) {
            throw new Error("Unterminated quote (" + quoteForStrings + ") in arg string '" + str + "'.");
        } else if (lastStartIndex !== j - 1) {
            argArray.push(str.substring(lastStartIndex, j));
        }

        for (var i = 0; i < argArray.length; i++) {
            parseArg(argArray[i], argObject);
        }
        return argObject;
    };

    this.GetArgumentsStartingAt = function (startingIndex) {
        var argObject = {};
        for (var i = startingIndex; i < WScript.Arguments.length; i++) {
            parseArg(String(WScript.Arguments(i)), argObject);
        }
        return argObject;
    };

    var originalBooleanConstructor = Boolean;
    var originalArrayConstructor = Array;
    this.ParseJSON = function (fileContents) {
        try {
            var File = function (path) {
                if (path === undefined) {
                    return undefined;
                }

                return Storage.GetDetachedFile(String(path), IOModes.ForReading);
            };
            File.name = "File";
            var Directory = function (path) {
                try{
                    if (path === undefined) {
                        return undefined;
                    }

                    return Storage.GetDetachedDirectory(String(path));
                } catch (ex) {
                    throw new Error("Failed to insantiate a config object to Directory type from '{0}'.".format(path));
                }
            };
            Directory.name = "Directory";
            var Enum = function (values) {
                if (values.constructor !== originalArrayConstructor) {
                    throw new Error("Invalid use of Enum constructor, it must be passed an array argument.");
                }

                var validateEnum = function (value) {
                    for (var i = 0; i < values.length; i++) {
                        if (String(value).toLowerCase() === String(values[i]).toLowerCase()) {
                            return values[i];
                        }
                    }
                    throw new Error("Invalid enum value specified: '{0}'; expected: '{1}'".format(value, values.join("','")));
                }
                validateEnum.name = "Enum({0})".format(values.join());
                return validateEnum;
            };
            
            var Array = function (obj) {
                if (obj === undefined) {
                    return undefined;
                }
                var toReturn = undefined;
                if (obj.constructor === String) {
                    toReturn = obj.split(';');
                } else if (obj.constructor === originalArrayConstructor) {
                    toReturn = obj;
                } else {
                    toReturn = originalArrayConstructor(obj);
                }
                toReturn.toString = function () { return this.join("; "); };
                return toReturn;
            }
            Array.name = "Array";

            var ArrayOf = function (itemType) {
                if (itemType === undefined) {
                    throw new Error("ArrayOf: The item type of this array wasn't passed.");
                } else if (!itemType.hasOwnProperty("name") || itemType.name === undefined) {
                    throw new Error("ArrayOf: The item type of this array is not a constructor, or doesn't have the 'name' property.");
                }

                var validateArrayOf = function (value) {
                    var array = Array(value);
                    array.toString = function () { return this.join("; "); }
                    array.map(function (item) {
                        return itemType(item);
                    }, true /* in place */);
                    return array;
                }
                validateArrayOf.name = "ArrayOf({0})".format(itemType.name);
                return validateArrayOf;
            }

            var TraceItem = function (obj) {
                if (obj === undefined || obj === "") {
                    throw new Error("TraceItem: Value '{0}' is either undefined or an empty string.".format(obj));
                }
                var parts = String(obj).split('.');
                if(parts.length > 2){
                    throw new Error("TraceItem: Unrecognized format '{0}' for a trace item, can't have more than 2 parts delimited by '.'.".format(obj));
                }

                var testSuite = parts.length === 2 ? parts[0] : "";
                var tag = parts[parts.length - 1];
                var obj = {
                    toString: function () {
                        if(this.tag === ""){
                            return "No tracing.";
                        }
                        var toReturn = " Trace {0} tag{1}".format(this.tag === "*" ? "all" : this.tag, this.tag === "*" ? "s" : "");
                        if (this.testSuite !== undefined) {
                            toReturn += ": for {0} Test Script{1}".format(this.testSuite === "*" ? "Test Harness and all" : this.testSuite, this.testSuite === "*" ? "s" : "");
                        } else {
                            toReturn += ": for Test Harness only";
                        }
                        return toReturn;
                    }};
                if (testSuite !== "") {
                    obj.testSuite = testSuite;
                } 
                obj.tag = tag === "" ? "*" : tag;

                return Utils.CreateObject(obj, TraceItem);
            }
            TraceItem.name = "TraceItem";

            var Boolean = function (obj) {
                if (obj.constructor === String) {
                    var value = String(obj);
                    if (value.toLowerCase() === "true" || value === '1') {
                        return true;
                    }
                    else return false;
                }
                return originalBooleanConstructor(obj);
            };
            Boolean.name = "Boolean"

            var Dictionary = function (obj) {
                var toReturn = {};
                if (obj.constructor === String) {
                    var keyValuePairs = String(obj).split(";");

                    for (var i = 0; i < keyValuePairs.length; i++) {
                        if (keyValuePairs[i] === "") {
                            continue;
                        }

                        var keyValue = keyValuePairs[i].split("=");

                        if (keyValue.length != 2) {
                            throw new Error("Incorrect arg format for Dictionary type, expected: -switch:<key>=<value>;<key>=<value>.");
                        }
                        toReturn[keyValue[0]] = keyValue[1];
                    }
                } else {
                    toReturn = Object(obj);
                }

                toReturn.toString = function () {
                    var stringRepresentation = "";
                    for (var prop in this) {
                        stringRepresentation += "{0}={1};".format(prop, this[prop]);
                    }
                    return stringRepresentation;
                }
                
                return toReturn;
            };
            Dictionary.name = "Dictionary";

            Number.name = "Number";
            String.name = "String";
            return (function () { eval("var __json__ = " + fileContents + ";"); return __json__; }.call(undefined));
        }
        catch (ex) {
            WScript.Echo("Parse JSON - Exception Occured: {0}".format(ex.description));
            WScript.Echo("JSON Object:\r\n" + fileContents);
        }
    }

    this.ApplyConfigOn = function (configDefinitionObject, targetObject, args, shell, baseConfigObject) {
        var defaultValues = {};
        var environmentValues = {};
        var argumentValues = {};
        var resolvedReferenceValues = {};
        var unrecognizedArguments = {};
        for (var prop in configDefinitionObject)
        {
            var value = configDefinitionObject[prop];
            var constructor = value.type;
            if (typeof constructor === 'string') {
                constructor = Utils.ParseJSON(constructor);
                configDefinitionObject[prop].type = constructor;
            } else if (constructor === undefined) {
                throw new Error("ApplyConfigOn: The type argument for config switch can't be undefined, switch: '{0}'".format(prop));
            }

            var defaultValue = value.defaultValue === undefined || value.defaultValue.constructor === constructor ? value.defaultValue : constructor(value.defaultValue);
            
            defaultValues[prop] = defaultValue;

            if (shell !== undefined && value.lookupFromEnvironment) {
                var envKey = value.environmentKey;
                if (envKey === undefined){
                    envKey = prop;
                }
                var envValue = shell.getEnv(envKey);
                if (envValue !== "") {
                    targetObject[prop] = envValue;
                    environmentValues[prop] = envValue;
                    continue;
                }
            }
            targetObject[prop] = defaultValue;
        }

        for (var prop in args) {
            if (configDefinitionObject.hasOwnProperty(prop)) {
                value = configDefinitionObject[prop].type(args[prop]);
                targetObject[prop] = value;
                argumentValues[prop] = value;
            } else if (baseConfigObject !== undefined && baseConfigObject.hasOwnProperty(prop)) {
                var value = baseConfigObject[prop].type(args[prop]);
                targetObject[prop] = value;
                argumentValues[prop] = value;
            } else {
                unrecognizedArguments[prop] = args[prop];
            }
        }

        function resolveReferences(definition, targetObject, references, argumentValues) {
            // DefaultValue can contain a reference to another config value, handle it here.
            // The support is done through String.format, replacing {n} with n'th item in the array of defaultValueReferences
            for (var prop in definition) {
                if (definition[prop].defaultValueReferences !== undefined && !argumentValues.hasOwnProperty(prop) && String(targetObject[prop]) === String(definition[prop].type(definition[prop].defaultValue))) {
                    var value = String.formatGivenArray(definition[prop].defaultValue, definition[prop].defaultValueReferences.map(function (key) { return String(targetObject[key]); }, false));;

                    value = definition[prop].type(value);

                    targetObject[prop] = value;
                    references[prop] = value;
                }
            }
        }
        resolveReferences(configDefinitionObject, targetObject, resolvedReferenceValues, argumentValues);

        // The object returned is the unflattened config, it includes all the layers
        
        return {
            definition: configDefinitionObject,
            baseConfigObject: baseConfigObject,
            defaultValues: defaultValues,
            environmentVariables: environmentValues,
            argumentValues: argumentValues,
            unrecognizedArguments: unrecognizedArguments,
            resolvedReferences: resolvedReferenceValues,
            reApplyReferenceResolution: resolveReferences
        };
    };

}.call(this.Utils));
