var BREAKPOINT_DELETED = 0,
    BREAKPOINT_DISABLED = 1,
    BREAKPOINT_ENABLED = 2;

var CALLSTACK_ALL = 0xFFFFFFFF,
    CALLSTACK_DOCUMENTID = 0x1,
    CALLSTACK_FLAGS = 0x2,
    CALLSTACK_LINECOLUMN = 0x4;

var SDO_NONE = 0x00000000,
    SDO_ENABLE_FIRST_CHANCE_EXCEPTIONS = 0x00000001,
    SDO_ENABLE_WEB_WORKER_SUPPORT = 0x00000002,
    SDO_ENABLE_NONUSER_CODE_SUPPORT = 0x00000004;

// Internal tracing flags
var TRACE_COMMANDS = 0x0001; // Trace dbg commands

// Maximum object tree depth to filter known global names. TODO: Make it configurable. -1 for no filtering.
var MAX_KNOWN_GLOBALS_FILTER_DEPTH = 3;

// Flags to locals(expandLevel, flags) or evaluate(expression, expandLevel, flags).
// NOTE: Keep these flags the same with DebugPropertyFlags (debuggercontroller.h)
var LOCALS_DEFAULT = 0x0,
    // LOCALS_RADIX reserves lower byte 0xFF
    LOCALS_FULLNAME = 0x100, // Use FullName instead of default short name
    LOCALS_TYPE = 0x200,
    LOCALS_ATTRIBUTES = 0x400;

// Use this function to generate radix flags to locals/evaluate. e.g. locals(0, LOCALS_RADIX(16))
function LOCALS_RADIX(radix) {
    if (radix < 2 || radix > 36) {
        WScript.Echo("ERROR: LOCALS_RADIX out of range:", radix);
    }
    return radix & 0xFF;
}

// Debugger might run in earlier script versions. Provide some simple simulations.
(function () {
    if (!Array.prototype.forEach) {
        Array.prototype.forEach = function (f) {
            for (var i = 0; i < this.length; i++) {
                f(this[i]);
            }
        }
    }

    if (!String.prototype.trim) {
        String.prototype.trim = function () { return this.replace(/^\s+|\s+$/gm, ''); }
    }
})();

var controllerObj = {
    eventLog: [],
    baseline: undefined,
    bpMap: [],
    commandList: [],
    commandCompletions: [],
    exceptionCommands: null,
    wasResumed: false,
    mutationCommands: null,
    _trace: 0, // Internal tracing flags

    // Start internal tracing of given category
    trace: function (traceFlag) {
        this._trace |= traceFlag;
    },
    // If internal tracing of given category is on
    isTracing: function (traceFlag) {
        return this._trace & traceFlag;
    },
    // Output trace if internal tracing of given category is on
    outputTrace: function (traceFlag) {
        if (this.isTracing(traceFlag)) {
            WScript.Echo.apply(WScript, [].slice.call(arguments, 1));
        }
    },

    escape: function (json) {
        return json
            .replace(/\n/g, "\\\n")
            .replace(/\r/g, "\\\r")
            .replace(/\t/g, "\\\t");
    },

    filterLog: (function () {
        var parentFilter = { "locals": 1, "this": 1, "[Globals]": 1, "[Methods]": 1 };
        var filter = {};

        // Discard all known globals to reduce baseline noise.
        [
            "ActiveXObject",
            "Array",
            "ArrayBuffer",
            "Atomics",
            "Boolean",
            "CanvasPixelArray",
            "chWriteTraceEvent",
            "CollectGarbage",
            "console",
            "CreateDomArrayObject",
            "CreateDomMapObject",
            "CreateDomSetObject",
            "DataView",
            "Date",
            "Debug",
            "decodeURI",
            "decodeURIComponent",
            "encodeURI",
            "encodeURIComponent",
            "Enumerator",
            "Error",
            "escape",
            "eval",
            "EvalError",
            "Float32Array",
            "Float64Array",
            "Function",
            "ImageData",
            "Infinity",
            "Int16Array",
            "Int32Array",
            "Int8Array",
            "Intl",
            "isFinite",
            "isNaN",
            "JSON",
            "Map",
            "Math",
            "NaN",
            "Number",
            "Object",
            "parseFloat",
            "parseInt",
            "print",
            "Promise",
            "Proxy",
            "RangeError",
            "read",
            "readbuffer",
            "ReferenceError",
            "Reflect",
            "RegExp",
            "SCA",
            "Set",
            "SharedArrayBuffer",
            "String",
            "Symbol",
            "SyntaxError",
            "telemetryLog",
            "TypeError",
            "Uint16Array",
            "Uint32Array",
            "Uint8Array",
            "Uint8ClampedArray",
            "undefined",
            "unescape",
            "URIError",
            "VBArray",
            "WeakMap",
            "WeakSet",
            "WebAssembly",
            "WScript",
        ].forEach(function (name) {
            filter[name] = 1;
            filter[name + " - [Type]"] = 1;         // Filter out LOCALS_TYPE
            filter[name + " - [Attributes]"] = 1;   // Filter out LOCALS_ATTRIBUTES
            filter["this." + name] = 1;             // Attempt to filter out full name as well, e.g. this.SCA
        });

        function filterInternal(parentName, obj, depth) {
            if (depth > MAX_KNOWN_GLOBALS_FILTER_DEPTH) {
                return;
            }

            for (var p in obj) {
                if (parentFilter[parentName] == 1 && filter[p] == 1) {
                    delete obj[p];
                } else if (typeof obj[p] == "object") {
                    filterInternal(p.trim(), obj[p], depth + 1);
                }
            }
        }

        return function (obj) {
            try {
                filterInternal("this"/*filter root*/, obj, 0);
            } catch (ex) {
                WScript.Echo("ERROR: exception during filter: " + ex);
            }
        };
    })(),

    recordEvent: function (json) {
        json = this.escape(json);

        try {
            var obj = JSON.parse(json);
            this.filterLog(obj);
            this.eventLog.push(obj);
        } catch (ex) {
            WScript.Echo("ERROR: invalid JSON passed to recordEvent");
            WScript.Echo(json);
        }
    },

    dumpLog: function () {
        WScript.Echo(this.getOutputJson())
    },

    setBaseline: function (baseline) {
        try {
            this.baseline = JSON.parse(baseline);
        } catch (ex) {
            WScript.Echo("ERROR: invalid JSON passed to setBaseline: " + ex);
            WScript.Echo(baseline);
        }
    },

    getOutputJson: function () {
        return JSON.stringify(this.eventLog, undefined, "  ");
    },

    verify: function () {
        var PASSED = 0;
        var FAILED = 1;


        if (typeof (this.baseline) == "undefined") {
            return FAILED;
        }

        try {
            if (this.compareObjects(this.baseline, this.eventLog, "baseline")) {
                return PASSED;
            }
        }
        catch (ex) {
            WScript.Echo("EXCEPTION: " + ex);
        }
        WScript.Echo("TEST FAILED");
        return FAILED;
    },

    resetBpMap: function () {
        this.bpMap = [];
    },

    addSourceFile: function (text, srcId) {
        try {
            // Split the text into lines.  Note this doesn't take into account block comments,
            // but that's probably okay.  TODO: handle CR.
            var lines = text.split(/\n/);
            var lineBeginningOffsets = [0];
            for (var i = 0; i < lines.length; ++i) {
                lineBeginningOffsets[i + 1] = lineBeginningOffsets[i] + lines[i].length + 1;
            }

            // /**bp   <-- a breakpoint
            // /**loc  <-- a named source location
            var bpStartToken = "/**";
            var bpStartTokenLength = bpStartToken.length;
            var bpStartStrings = ["bp", "loc", "exception", "edit", "endedit", "onmbp"];
            var bpEnd = "**/";
            var tripleCommentToken = "///";
            var tripleCommentTokenLength = tripleCommentToken.length;
            var editStack = new Array();

            // Iterate through each source line, setting any breakpoints.

            for (var i = 0; i < lines.length; ++i) {
                var line = lines[i];
                var lineTrimmed = line.replace(/^\s+|\s+$/g, '');
                if (lineTrimmed.indexOf(tripleCommentToken) == 0)
                {
                    if (editStack.length != 0)
                    {
                        var currentEditScope = editStack.pop();
                        currentEditScope.lines += lineTrimmed.substring(tripleCommentTokenLength) + "\r\n";
                        editStack.push(currentEditScope);
                    }
                }

                for (var startString in bpStartStrings) {

                    // bpStart = /**bp, /**loc, /**exception, /**edit, /**endedit, /**onmbp
                    var bpStart = bpStartToken + bpStartStrings[startString];
                    var isLocationBreakpoint = (bpStart.indexOf("loc") != -1);
                    var isExceptionBreakpoint = (bpStart.indexOf("exception") != -1);
                    var isOnMutationBreakpoint = (bpStart.indexOf("onmbp") != -1);
                    var startIdx = -1;

                    while ((startIdx = line.indexOf(bpStart, startIdx + 1)) != -1) {

                        var endIdx;
                        var currLine = i;
                        var bpLine = i;
                        var currBpLineString = "";

                        // Gather up any lines within the breakpoint comment.
                        do {
                            // Since we are supporting multiple bp at the same line (with the offset), the search for end should begin with the start index.
                            var currentStartIdx = 0;
                            if (currLine == i) {
                                currentStartIdx = startIdx;
                            }
                            currBpLineString += lines[currLine++];
                            endIdx = currBpLineString.indexOf(bpEnd, currentStartIdx);
                        } while (endIdx == -1 && currLine < lines.length && lines[currLine].indexOf(bpStartToken) == -1);

                        // Move the line cursor forward, allowing the current line to be re-checked
                        i = currLine - 1;

                        // Do some checking
                        if (endIdx == -1) {
                            WScript.Echo("ERROR: unterminated breakpoint expression");
                            return;
                        }

                        var bpStrStartIdx = startIdx + bpStart.length;
                        var bpStr = currBpLineString.substring(bpStrStartIdx, endIdx);

                        var bpFullStr = currBpLineString.substring(startIdx, endIdx);
                        var isEdit = bpFullStr.indexOf("edit") == bpStartTokenLength;
                        var isEndEdit = bpFullStr.indexOf("endedit") == bpStartTokenLength;

                        if (isEdit)
                        {
                            var editLabel = bpStr.substring(1, bpStr.length - 1);
                            editStack.push({ label: editLabel, startLine: bpLine, startColumn: startIdx, lines: "" })
                            continue;
                        }
                        if (isEndEdit)
                        {
                            var editLabel = bpStr.substring(1, bpStr.length - 1);
                            if (editStack.length == 0)
                            {
                                WScript.Echo("ERROR: endedit found without matching edit");
                                return;
                            }
                            var pendingEdit = editStack.pop();
                            if (pendingEdit.label != editLabel)
                            {
                                WScript.Echo("ERROR: Mismatched endedit found");
                                return;
                            }
                            pendingEdit.endLine = bpLine;
                            pendingEdit.endColumn = endIdx + bpEnd.length;

                            var startOffset = lineBeginningOffsets[pendingEdit.startLine] + pendingEdit.startColumn;
                            var endOffset = lineBeginningOffsets[pendingEdit.endLine] + pendingEdit.endColumn;
                            var rangeLength = endOffset - startOffset;

                            WScript.RecordEdit(srcId, pendingEdit.label, startOffset, rangeLength, pendingEdit.lines);
                            continue;
                        }

                        // Quick check to make sure the breakpoint is not within a
                        // quoted string (such as an eval).  If it is within an eval, the
                        // eval will cause a separate call to have its breakpoints parsed.
                        // This check can be defeated, but it should cover the useful scenarios.
                        var quoteCount = 0;
                        var escapeCount = 0;
                        for (var j = 0; j < bpStrStartIdx; ++j) {
                            switch (currBpLineString[j]) {
                                case '\\':
                                    escapeCount++;
                                    continue;
                                case '"':
                                case '\'':
                                    if (escapeCount % 2 == 0)
                                        quoteCount++;
                                    /* fall through */
                                default:
                                    escapeCount = 0;
                            }
                        }
                        if (quoteCount % 2 == 1) {
                            // The breakpoint was in a quoted string.
                            continue;
                        }


                        // Only support strings like:
                        //  /**bp**/
                        //  /**bp(name)**/
                        //  /**bp(columnoffset)**/         takes an integer
                        //  /**bp:locals();stack()**/
                        //  /**bp(name):locals();stack()**/
                        //
                        //  /**loc(name)**/
                        //  /**loc(name):locals();stack()**/
                        //

                        // Parse the breakpoint name (if it exists)
                        var bpName = undefined;
                        var bpColumnOffset = 0;
                        var bpExecStr = undefined;
                        var parseIdx = 0;
                        if (bpStr[parseIdx] == '(') {
                            // The name and offset is overloaded with the same parameter.
                            // if that is int (determined by parseInt), then it is column offset otherwise left as name.
                            bpName = bpStr.match(/\(([\w,]+?)\)/)[1];
                            parseIdx = bpName.length + 2;
                            bpColumnOffset = parseInt(bpName);
                            if (isNaN(bpColumnOffset)) {
                                bpColumnOffset = 0;
                            }
                            else {
                                bpName = undefined;
                                if (bpColumnOffset > line.length) {
                                    bpColumnOffset = line.length - 1;
                                }
                                else if (bpColumnOffset < 0) {
                                    bpColumnOffset = 0;
                                }
                            }
                        }
                        else if (isLocationBreakpoint) {
                            WScript.Echo("ERROR: 'loc' sites require a label, for example /**loc(myFunc)**/");
                            return;
                        }

                        // Process the exception label:
                        //   exception(resume_ignore)
                        //   exception(resume_break)
                        //   exception(firstchance,resume_ignore)
                        //   exception(firstchance,resume_break)
                        //   exception(firstchance)
                        if (isExceptionBreakpoint) {
                            if (bpName !== undefined) {
                                var strings = bpName.split(/,/);
                                var isError = false;
                                var foundResume = false;

                                if (strings.length > 2) {
                                    isError = true;
                                }
                                else {
                                    for (var j = 0; j < strings.length; ++j) {
                                        switch (strings[j]) {
                                            case "resume_ignore":
                                                isError = isError || foundResume;
                                                WScript.SetExceptionResume("ignore");
                                                foundResume = true;
                                                break;
                                            case "resume_break":
                                                isError = isError || foundResume;
                                                WScript.SetExceptionResume("break");
                                                foundResume = true;
                                                break;
                                            case "firstchance":
                                                WScript.SetDebuggerOptions(SDO_ENABLE_FIRST_CHANCE_EXCEPTIONS, 1);
                                                break;
                                            case "nonusercode":
                                                WScript.SetDebuggerOptions(SDO_ENABLE_NONUSER_CODE_SUPPORT, 1);
                                                break;
                                            default:
                                                isError = true;
                                        }
                                    }
                                }

                                if (isError) {
                                    WScript.Echo("ERROR: exception annotation incorrect; please review documentation on wiki");
                                    return;
                                }
                            }
                        }


                        // Parse the breakpoint execution string
                        if (bpStr[parseIdx] == ':') {
                            bpExecStr = bpStr.substring(parseIdx + 1);
                        }
                        else if (parseIdx != bpStr.length) {
                            WScript.Echo("ERROR: invalid breakpoint string: " + bpStr);
                            return;
                        }

                        // Insert the breakpoint at the beginning of the line.  Code locations are set
                        // as disabled breakpoints.
                        if (isOnMutationBreakpoint) {
                            if (this.mutationCommands != null) {
                                WScript.Echo("ERROR: more than one 'onmbp' annotation found");
                                return;
                            }
                            this.mutationCommands = bpExecStr;
                        }
                        else if (isExceptionBreakpoint) {
                            if (this.exceptionCommands != null) {
                                WScript.Echo("ERROR: more than one 'exception' annotation found");
                                return;
                            }
                            this.exceptionCommands = bpExecStr;
                        }
                        else {
                            var bpId = WScript.InsertBreakpoint(srcId, bpLine, bpColumnOffset, isLocationBreakpoint ? BREAKPOINT_DISABLED : BREAKPOINT_ENABLED);


                            //WScript.Echo("NAME: " + bpName + ", EXEC: " + bpExecStr.replace(/[\n\r]/g, " "));

                            // Check whether the breakpoint name already exists.  Note - we can't use ES5 builtins
                            // here because the engine may be run in a legacy language version.
                            var duplicateName = false;
                            for (var idx in this.bpMap) {
                                if (this.bpMap[idx] && this.bpMap[idx].name === bpName) {
                                    duplicateName = true;
                                    break;
                                }
                            }

                            if (bpName != undefined && duplicateName) {
                                throw "duplicate breakpoints named " + bpName;
                            }

                            // Save the breakpoint.
                            if (this.bpMap[bpId]) {
                                if (this.bpMap[bpId].name || bpName) {
                                    throw "duplicate named breakpoints on the same line not supported (names: " + bpName + ", " + this.bpMap[bpId].name + ")";
                                }
                                else {
                                    this.bpMap[bpId].exec = bpExecStr;
                                }
                            }
                            else {
                                this.bpMap[bpId] = { name: bpName, line: bpLine, exec: bpExecStr };
                            }
                        }
                    }
                }
            };

        }
        catch (ex) {
            WScript.Echo("ERROR: " + ex);
        }
    },
    handleException: function () {
        return this.handleBreakpoint("exception");
    },
    handleMutationBreakpoint: function () {
        return this.handleBreakpoint("mutation");
    },
    handleBreakpoint: function (id) {
        this.wasResumed = false;

        if (id != -1) {
            try {
                var execStr = "";

                if (id === "mutation") {
                    execStr = this.mutationCommands;
                    if (execStr && execStr.toString().search("removeExpr()") != -1) {
                        this.mutationCommands = null;
                    }
                }
                else if (id === "exception") {
                    execStr = this.exceptionCommands;
                    if (execStr && execStr.toString().search("removeExpr()") != -1) {
                        this.exceptionCommands = null;
                    }
                } else {
                    // Retrieve this breakpoint's execution string
                    if (this.bpMap[id] && this.bpMap[id].exec) {
                        execStr = this.bpMap[id].exec;
                        if (this.bpMap[id].exec.toString().search("removeExpr()") != -1) {
                            // A hack to remove entire expression, so that it will not run again.
                            this.bpMap[id].exec = null;
                        }
                    }
                }

                // Run the commands.
                eval(execStr);
            }
            catch (ex) {
                WScript.Echo("ERROR: " + ex);
            }
        }
        // Continue processing the command list.

        function getCmdStr(cmd) {
            function cmdName(fn) {
                var cmds = controllerObj.debuggerCommands;
                if (cmds.resume === fn) {
                    return "resume";
                }
                for (var p in cmds) {
                    if (cmds[p] === fn) {
                        return p;
                    }
                }
                return "unknown";
            }
            return cmdName(cmd.fn) + ": " + JSON.stringify(cmd.args)
        }
        function traceCmds(label) {
            if (controllerObj.isTracing(TRACE_COMMANDS)) {
                controllerObj.outputTrace(TRACE_COMMANDS, label + ", commands " + controllerObj.commandList.length);
                controllerObj.outputTrace(TRACE_COMMANDS, controllerObj.commandList.map(function (cmd) {
                    return "      " + getCmdStr(cmd);
                }).join('\n'));
            }
        }

        traceCmds("======== handleBreakpoint");
        while (this.commandList.length > 0 && !this.wasResumed) {
            var cmd = this.commandList.shift();
            var completion = cmd.fn.apply(this, cmd.args);
            if (controllerObj.isTracing(TRACE_COMMANDS)) {
                traceCmds("--- after " + getCmdStr(cmd));
            }

            if (typeof completion === "function") {
                this.commandCompletions.push(completion);
            }
        }

        while (this.commandCompletions.length > 0) {
            var completion = this.commandCompletions.shift();
            completion();
        }

        if (!this.wasResumed) {
            WScript.ResumeFromBreakpoint("continue");
            this.wasResumed = true;
        }
    },

    getBpIdFromName: function (name) {

        for (var i = 0; i < this.bpMap.length; ++i) {
            if (this.bpMap[i].name === name)
                return i;
        }

        WScript.Echo("ERROR: breakpoint named '" + name + "' was not found");
    },

    compareObjects: function (a, b, obj_namespace) {
        var objectsEqual = true;

        // This is a basic object comparison function, primarily to be used for JSON data.
        // It doesn't handle cyclical objects.

        function fail(step, message) {
            if (message == undefined) {
                message = "diff baselines for details";
            }
            WScript.Echo("ERROR: Step " + step + "; on: " + obj_namespace + ": " + message);
            objectsEqual = false;
        }

        function failNonObj(step, a, b, message) {
            if (message == undefined) {
                message = "";
            }
            WScript.Echo("ERROR: Step " + step + "; Local Diff on: " + obj_namespace + ": " + message);
            WScript.Echo("Value 1:" + JSON.stringify(a));
            WScript.Echo("Value 2:" + JSON.stringify(b));
            WScript.Echo("");
            objectsEqual = false;
        }

        // (1) Check strict equality.
        if (a === b)
            return true;

        // (2) non-Objects must have passed the strict equality comparison in (1)
        if (!(a instanceof Object) || !(b instanceof Object)) {
            failNonObj(2, a, b);
            return false;
        }

        // (3) check all properties
        for (var p in a) {

            // (4) check the property
            if (a[p] === b[p])
                continue;

            // (5) non-Objects must have passed the strict equality comparison in (4)
            if (typeof (a[p]) != "object") {
                failNonObj(5, a[p], b[p], "Property " + p);
                continue;
            }

            // (6) recursively check objects or arrays
            if (!this.compareObjects(a[p], b[p], obj_namespace + "." + p)) {
                // Don't need to report error message as it'll be reported inside nested call
                objectsEqual = false;
                continue;
            }
        }

        // (7) check any properties not in the previous enumeration
        var hasOwnProperty = Object.prototype.hasOwnProperty;
        for (var p in b) {
            if (hasOwnProperty.call(b, p) && !hasOwnProperty.call(a, p)) {
                fail(7, "Property missing: " + p + ", value: " + JSON.stringify(b[p]));
                continue;
            }
        }
        return objectsEqual;
    },
    pushCommand: function (fn, args) {
        this.commandList.push({ fn: fn, args: args });
    },
    debuggerCommands: {
        log: function (str) {
            WScript.Echo("LOG: " + str);
        },
        logJson: function (str) {
            WScript.LogJson(str);
        },
        resume: function (kind) {
            if (controllerObj.wasResumed) {
                WScript.Echo("ERROR: breakpoint resumed twice");
            }
            else {
                WScript.ResumeFromBreakpoint(kind);
                controllerObj.wasResumed = true;
            }
        },
        locals: function (expandLevel, flags) {
            WScript.DumpLocals(expandLevel || 0, flags || LOCALS_DEFAULT);
        },
        stack: function (flags) {
            if (flags === undefined) {
                flags = 0;
            }

            WScript.DumpCallstack(flags);
        },
        setnext: function (target, column) {
            var line = target;
            if (typeof (target) === "string") {
                // If the user passer a string, they want to set a location.
                if (column !== undefined) {
                    WScript.Echo("ERROR: if passed a string, setnext() cannot also take a column number");
                    return;
                }
                else {
                    column = 1;
                    for (var idx in controllerObj.bpMap) {
                        if (controllerObj.bpMap[idx].name === target) {
                            line = controllerObj.bpMap[idx].line;
                            break;
                        }
                    }
                    if (line === target) {
                        WScript.Echo("ERROR: location '" + target + "' not found");
                        return;
                    }
                }
            }
            if (WScript.SetNextStatement(line, column))
            {
                // SetNextStatement has implicit resume if success
                controllerObj.wasResumed = true;
            }
        },
        evaluate: function (expression, expandLevel, flags) {
            if (expression != undefined) {
                WScript.EvaluateExpression(expression, expandLevel || 0, flags || LOCALS_DEFAULT);
            }
        },
        evaluateAsync: function(expression, expandLevel, flags) {
            if (expression != undefined) {
                return WScript.EvaluateExpressionAsync(expression, expandLevel || 0, flags || LOCALS_DEFAULT);
            }
        },
        enableBp: function (name) {
            var bpId = controllerObj.getBpIdFromName(name);
            WScript.ModifyBreakpoint(bpId, BREAKPOINT_ENABLED);
        },
        disableBp: function (name) {
            var bpId = controllerObj.getBpIdFromName(name);

            WScript.ModifyBreakpoint(bpId, BREAKPOINT_DISABLED);
        },
        deleteBp: function (name) {
            var bpId = controllerObj.getBpIdFromName(name);

            WScript.ModifyBreakpoint(bpId, BREAKPOINT_DELETED);
        },
        setFrame: function (depth) {
            WScript.SetFrame(depth);
        },
        setExceptionResume: function (kind) {
            WScript.SetExceptionResume(kind);
        },
        trackProjectionCall: function (msg) {
            if (WScript.TrackProjectionCall) {
                WScript.TrackProjectionCall(msg || "JS projection call");
            }
        },
        dumpBreak: function () {
            WScript.DumpBreakpoint();
        },
        dumpSourceList: function () {
            WScript.DumpSourceList();
        },
        mbp: function (name, setOnObject, type, strId) {
            if (!WScript.SetMutationBreakpoint
                || typeof setOnObject !== "string"
                || typeof name !== "string"
                || typeof type !== "string"
                || typeof strId !== "string")
            {
                WScript.Echo("ERROR: mbp: invalid parameters");
                return;
            }
            // Assumption: only use dot notation
            WScript.SetMutationBreakpoint(name, setOnObject, type, strId);
        },
        deleteMbp: function (strId) {
            if (!WScript.DeleteMutationBreakpoint
                || strId === undefined) {
                WScript.Echo("ERROR: deleteMbp: must pass a valid Id string");
                return;
            }
            WScript.DeleteMutationBreakpoint(strId);
        },
        // Start internal tracing
        trace: function (traceFlag) {
            controllerObj.trace(traceFlag);
        }
    }
};

//
// APIs for use from the breakpoint execution string
//
function log(str) {
    controllerObj.pushCommand(controllerObj.debuggerCommands.log, arguments);
}
function logJson(str) {
    controllerObj.pushCommand(controllerObj.debuggerCommands.logJson, arguments);
}
function resume(kind) {
    controllerObj.pushCommand(controllerObj.debuggerCommands.resume, arguments);
}
function locals(expandLevel, flags) {
    controllerObj.pushCommand(controllerObj.debuggerCommands.locals, arguments);
}
function stack() {
    controllerObj.pushCommand(controllerObj.debuggerCommands.stack, arguments);
}
function removeExpr(bpId) {
    // A workaround to remove the current expression
}
function setnext(line, column) {
    controllerObj.pushCommand(controllerObj.debuggerCommands.setnext, arguments);
}
function evaluate(expression, expandLevel, flags) {
    controllerObj.pushCommand(controllerObj.debuggerCommands.evaluate, arguments);
}
function evaluateAsync(expression, expandLevel, flags) {
    controllerObj.pushCommand(controllerObj.debuggerCommands.evaluateAsync, arguments);
}
function enableBp(name) {
    controllerObj.pushCommand(controllerObj.debuggerCommands.enableBp, arguments);
}
function disableBp(name) {
    controllerObj.pushCommand(controllerObj.debuggerCommands.disableBp, arguments);
}
function deleteBp(name) {
    controllerObj.pushCommand(controllerObj.debuggerCommands.deleteBp, arguments);
}
function setFrame(name) {
    controllerObj.pushCommand(controllerObj.debuggerCommands.setFrame, arguments);
}
function setExceptionResume(kind) {
    controllerObj.pushCommand(controllerObj.debuggerCommands.setExceptionResume, arguments);
}
function trackProjectionCall(msg) {
    controllerObj.pushCommand(controllerObj.debuggerCommands.trackProjectionCall, arguments);
}
function dumpBreak() {
    controllerObj.pushCommand(controllerObj.debuggerCommands.dumpBreak, arguments);
}
function dumpSourceList() {
    controllerObj.pushCommand(controllerObj.debuggerCommands.dumpSourceList, arguments);
}
function mbp(name, setOnObject, type, strId) {
    controllerObj.pushCommand(controllerObj.debuggerCommands.mbp, arguments);
}
function deleteMbp(strId) {
    controllerObj.pushCommand(controllerObj.debuggerCommands.deleteMbp, arguments)
}

// Start internal tracing.
// E.g.: /**bp:trace(TRACE_COMMANDS)**/
function trace() {
    controllerObj.pushCommand(controllerObj.debuggerCommands.trace, arguments);
}

//
// APIs exposed to DebuggerController.cpp
//
function RecordEvent() {
    return controllerObj.recordEvent.apply(controllerObj, arguments);
}
function DumpLog() {
    return controllerObj.dumpLog.apply(controllerObj, arguments);
}
function GetOutputJson() {
    return controllerObj.getOutputJson.apply(controllerObj, arguments);
}
function SetBaseline() {
    return controllerObj.setBaseline.apply(controllerObj, arguments);
}
function Verify() {
    return controllerObj.verify.apply(controllerObj, arguments);
}
function AddSourceFile() {
    return controllerObj.addSourceFile.apply(controllerObj, arguments);
}
function ResetBpMap() {
    return controllerObj.resetBpMap.apply(controllerObj, arguments);
}
function HandleBreakpoint() {
    return controllerObj.handleBreakpoint.apply(controllerObj, arguments);
}
function HandleException() {
    return controllerObj.handleException.apply(controllerObj, arguments);
}
function HandleMutationBreakpoint() {
    return controllerObj.handleMutationBreakpoint.apply(controllerObj, arguments);
}
