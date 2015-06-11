
intellisense.enableMicrosoftAjaxLog = function (logLevel) { 
/// <summary>Enables Microsoft Ajax VSIntellisense extension logging</summary>
/// <param name="logLevel" type="String">Acceptable values are "info", "warning", and "error"</param>

    intellisense.logMessage("Passed in log level is " + logLevel);
    if(logLevel) {
        switch(logLevel.trim().toLowerCase())
        {
            case "info":
                intellisense["MicrosoftAjaxLogLevelInfo"] = true;
                intellisense.logMessage("Setting log level info.");
                // Intentional fallthrough
            case "warning":
                intellisense["MicrosoftAjaxLogLevelWarning"] = true;
                intellisense.logMessage("Setting log level warning.");
                // Intentional fallthrough
            case "error":
                intellisense["MicrosoftAjaxLogLevelError"] = true;
                intellisense.logMessage("Setting log level error.");
        }
    }
};


(function () {


    function logError(message) {
        intellisense.logMessage("MicrosoftAjax.intellisense error: " + message);
    }

    function logWarning(message) {
        intellisense.logMessage("MicrosoftAjax.intellisense warning: " + message);
    }

    function logInfo(message) {
        intellisense.logMessage("MicrosoftAjax.intellisense info: " + message);
    }

    // Override runtime validation functions to improve perf and avoid unnecessary errors
    Function._validateParams = function Function$_validateParams(params, expectedParams, validateParameterCount) {
        return null;
    }

    String._toFormattedString = function String$_toFormattedString(useLocale, args) {
        return "";
    }


    function getMetaFieldNames(functionObject) {
        /// <summary>Returns the names of the doc-comment fields defined in the passed in function</summary>
        /// <param name="functionObject" type="Function">Function to search for doc-comment field definitions</param>
        /// <returns type="Object">Map of meta-field names to "MetaField" marker string.</returns>

        if(!functionObject) {
            return null;
        }

        var functionDocComments = getFunctionDocComments(functionObject);
        if (!functionDocComments) {
            return null;
        }

        var fieldSet = {};

        var fieldNamesRegExp = /<field\s+[^>]*name=[\'\"]{1}([^\'\"]*)[\'\"]{1}/g;
        var fieldNameCaptureIndex = 1;
        var results;
        while (results = fieldNamesRegExp.exec(functionDocComments)) {
            var metaField = results[fieldNameCaptureIndex];
            if (logInfos) logInfo("Found meta-field: " + metaField); 
            fieldSet[metaField] = "MetaField";
        }

        return fieldSet;
    }

    function getFunctionDocComments(functionObject) {
        /// <summary>Returns XML doc-comment text of function</summary>
        /// <param name="functionObject" type="Function">Function to search for doc-comments</param>
        /// <returns type="String">Doc-comment content after stripping the leading ///</returns>

        var functionCode = functionObject.toString();
        var functionDocComments = "";

        var line;
        var nextLineStartPos = functionCode.indexOf('{');

        if (nextLineStartPos < 0)
            return null;
        else
            nextLineStartPos++;

        while ((line = getLine(nextLineStartPos, functionCode)) && (line.length > 0)) {
            var docCommentLine = getDocCommentLine(line);
            if (docCommentLine == null)
                break;
            else if (docCommentLine.length > 0)
                functionDocComments += docCommentLine;

            nextLineStartPos += line.length;
        }

        if (logInfos) logInfo("Got function doc-comments: " + functionDocComments);

        return functionDocComments;
    }

    function getLine(startPosition, multiLineText) {
        /// <summary>First line starting at the specified position within passed in text</summary>
        /// <param name="startPosition" type="Number">Search start position</param>
        /// <param name="multiLineText" type="String">Multi-line text to search for the next line</param>
        /// <returns type="String">Single line starting at the specified position or null at the end of the text.</returns>
   
        if (startPosition === undefined || multiLineText === undefined)
            return null;

        if(startPosition >= multiLineText.length)
            return null;

        for(var position = startPosition; position < multiLineText.length; position++) {
            if(multiLineText[position] == '\r' || multiLineText[position] == '\n') {
                if (multiLineText[position] == '\r' && position + 1 < multiLineText.length && multiLineText[position + 1] == '\n')
                    position++;
                return multiLineText.substring(startPosition, position + 1);
            }
        }

        return multiLineText.substring(startPosition);
    }

    function getDocCommentLine(line) {
        /// <summary>Returns text of the XML doc-comment one the line</summary>
        /// <param name="line" type="String"></param>
        /// <returns type="String">Doc-comment line content stripping leading ///, or null if the line is not a doc-comment line</returns>

        var docComment = "///";

        line = line.trim();

        // Blank lines are fine
        if (line.length == 0)
            return "";

        // Non-blank line that doesn't start with "///" is not considered a doc-comment line
        if (!line.startsWith(docComment))
            return null;
        
        // Don't allow more than three slashes at the start
        if (line.length > docComment.length && line[docComment.length + 1] == '/')
            return null;

        // Empty doc-comment lines are fine
        if (line.length == docComment.length)
            return "";

        return line.substring(docComment.length);
    }


    intellisense.addEventListener('statementcompletion', function (e) {

        function addInheritedItems() {
            
            if(intellisense["MicrosoftAjaxLogLevelInfo"]) logInfo("In addInheritedItems");

            if (contextMsAjaxType == "class") {
                if (e.target.resolveInheritance) {
                    var childMemberCache = new Object();
                    for (var member in e.target) {
                        childMemberCache[member] = member;
                    }
             
                    e.target.resolveInheritance();
             
                    for (var member in e.target) {
                        if (!childMemberCache[member]) {
                            var kind = isFunction(e.target[member]) ? "method" : "field";
                            e.items.push({ name: member, kind: kind, value: e.target[member], parentObject: e.target }); 
                        }
                    }
                }
            }

            if (logInfos) logInfo("Exiting addInheritedItems");
        }


        function isFunction(obj) {
            return Object.prototype.toString.call(obj) === "[object Function]";
        }

        function filterItems() {

            if (logInfos) logInfo("Entering filterItems for " + e.items.length + " items");

            e.items = e.items.filter(filterItem);

            if (logInfos) logInfo("Exiting filterItems");

        }

        function filterItem(item) {

            if (logInfos) logInfo("");
            if (logInfos) logInfo("Filtering item: " + item.name);

            var hidden = false;
            var msAjaxType = null;
        
            hidden = item.name[0] == "_";
            hidden |= item.name.indexOf("$") > 0;

            if (hidden) {
                var contextIsThis = false;
                if (e.targetName && e.targetName == "this") { 
                    contextIsThis = true;
                }
                if (logInfos) logInfo("Context is this: " + contextIsThis + " (it is " + e.targetName + ")");
                if (contextIsThis && !isGlobalScope) {
                    // Don"t hide any members when completion is invoked on "this."
                    // and "this" is not global scope
                    hidden = false;
                }
                else if (isGlobalScope) {
                    if (logInfos) logInfo("In global scope item value for item " + item.name + " is " + item.value);
                    msAjaxType = getMicrosoftAjaxTypeForObject(item.value);
                    if (msAjaxType == null) {
                        hidden = false;
                    }
                }
            }

            // For Enums and Flags, we show fields only
            if (!hidden && isEnumContext) {

                if (item.kind != "field") {
                    hidden = true;
                    if (logInfos) logInfo("Filtering out non-field member " + item.name + " on a " + contextMsAjaxType);
                }
                else if (enumMetaFields && (enumMetaFields[item.name] != "MetaField")) {
                    hidden = true;
                    if (logInfos) logInfo("Filtering out non-meta-field member " + item.name + " on a " + contextMsAjaxType);
                }
            }

            return !hidden;
        }

        function setItemsGlyphs() {
            e.items.forEach(setItemGlyph);
        }

        function setItemGlyph(item) {

            if (logInfos) logInfo("Setting item glyph for: " + item.name);

            if (isEnumContext) {
                item.glyph = "vs:GlyphGroupEnumMember";
            }
            else if (item.name.indexOf("set_") == 0 || item.name.indexOf("get_") == 0) {
                if (item.kind == "method") {
                    item.kind = "property";
                }
            }
            else if (item.name.indexOf("add_") == 0 || item.name.indexOf("remove_") == 0) {
                if (item.kind == "method") {
                    item.glyph = "vs:GlyphGroupEvent";
                }
            }
            else {
                var itemValue = item.value;
                if(itemValue) {
                    var ajaxType = getMicrosoftAjaxType(itemValue);
                    if (ajaxType != null) {
                        switch (ajaxType) {
                            case "class":
                                item.glyph = "vs:GlyphGroupClass";
                                break;
                            case "namespace":
                                item.glyph = "vs:GlyphGroupNamespace";
                                break;
                            case "interface":
                                item.glyph = "vs:GlyphGroupInterface";
                                break;
                            case "enum":
                            case "flags":
                                item.glyph = "vs:GlyphGroupEnum";
                            default:
                                if (logErrors) logError("Unknown Microsoft Ajax type: " + ajaxType);
                        }
                    }
                }
            }
        }

        function isFlagsOrEnumItem(item) {
            if (logInfos) logInfo("Entering isFlagsOrEnumItem for: " + item.name);
            var microsoftAjaxType = getMicrosoftAjaxTypeForObject(item.value);
            return isFlagsOrEnumType(microsoftAjaxType);
        }

        function isFlagsOrEnumType(microsoftAjaxType) {
            return microsoftAjaxType == "enum" || microsoftAjaxType == "flags";
        }

        function getMicrosoftAjaxTypeForObject(object) {
            
            var ajaxType = null;

            if (object) {
                ajaxType = getMicrosoftAjaxType(object);

                if (ajaxType == null) {
                    var objectConstructor = object.constructor;
                    if (objectConstructor) {
                        ajaxType = getMicrosoftAjaxType(objectConstructor);
                        if (ajaxType != null) { 
                            if (logInfos) logInfo("Got MS Ajax type for object constructor: " + ajaxType);
                        }
                    }
                }
                else {
                    if (logInfos) logInfo("Got MS Ajax type for object itself: " + ajaxType);
                }
            }

            return ajaxType;
        }

        function getMicrosoftAjaxType(obj) {

            var ajaxType = null;

            if (obj.__class)
                ajaxType = "class";
            else if (obj.__interface)
                ajaxType = "interface";
            else if (obj.__namespace)
                ajaxType = "namespace";
            else if (obj.__flags)
                ajaxType = "flags";
            else if (obj.__enum)
                ajaxType = "enum";

            return ajaxType;
        }

        function isGlobalScopeCompletionList() {

            var result = false;

            for (var i = 0; i < e.items.length; i++) {
                if (e.items[i].name == "break") {
                    result = true;
                    break;
                }
            }

            return result;
        }

        var logErrors = !!intellisense["MicrosoftAjaxLogLevelError"];
        var logWarnings = !!intellisense["MicrosoftAjaxLogLevelWarning"];
        var logInfos = !!intellisense["MicrosoftAjaxLogLevelInfo"];
        
        if (logInfos) logInfo("In addCompletionHandler\n");
//        if (logInfos) logInfo("e.target is " + e.target);
        if (logInfos) logInfo("e.targetName is " + e.targetName);
        if (logInfos) logInfo("e.offset is " + e.offset);

        var isGlobalScope = isGlobalScopeCompletionList();
        var contextMsAjaxType = isGlobalScope ? null : getMicrosoftAjaxTypeForObject(e.target);
        var isEnumContext = contextMsAjaxType != null ? isFlagsOrEnumType(contextMsAjaxType) : false;
        var enumMetaFields = isEnumContext ? getMetaFieldNames(e.target) : {};

        if (logInfos) logInfo("isGlobalScope is " + isGlobalScope);
        if (logInfos) logInfo("contextMsAjaxType is " + contextMsAjaxType + "\n");
        if (logInfos) logInfo("isEnumContext is " + isEnumContext + "\n");

        addInheritedItems();
        filterItems();
        setItemsGlyphs();
    });
    

    intellisense.addEventListener('signaturehelp', function (e) {

        function getPropertyType(comments) {
            /// <param name="comments" type="String" />

            var propertyType = "";

            var docCommentStart = comments.indexOf("<value");
            if (docCommentStart == -1) {
                if (logWarnings) logWarning("<value> doc comment is not found.");
                return "";
            }

            var docCommentEnd = comments.indexOf("</value>", docCommentStart);
            if (docCommentEnd == -1) {
                docCommentEnd = comments.indexOf("/>", docCommentStart);
                if (docCommentEnd == -1) {
                    if (logWarnings) logWarning("End of <value> doc comment is not found.");
                    return "";
                }
            }

            var typeStart = comments.indexOf("type", docCommentStart);
            if (typeStart == -1 || typeStart > docCommentEnd)
                return "";

            var typeValueStart = comments.indexOf("\"", typeStart);
            if (typeValueStart == -1 || typeValueStart > docCommentEnd)
                return "";

            var typeValueEnd = comments.indexOf("\"", typeValueStart + 1);
            if (typeValueEnd == -1 || typeValueEnd > docCommentEnd)
                return "";

            propertyType = comments.substring(typeValueStart + 1, typeValueEnd);

            return propertyType;
        }

        function fixupSetterSignature(signature) {
            if (logInfos) logInfo("Fixing signature for type " + getterType);
            if (getterType) {                
                if (signature.params.length > 0) {
                    signature.params[0].type = getterType;
                }
            }
            else {
                if(intellisense["MicrosoftAjaxLogLevelError"]) logError("No getter.");
            }
        }

        function fixupSetter() {

            var baseName = e.functionHelp.functionName.substring("set_".length);
            var getterName = "get_" + baseName;

            if (e.parentObject) {
                var getterFunction = e.parentObject[getterName];
                if (getterFunction) {
                    var comments = intellisense.getFunctionComments(getterFunction);
                    if (comments && comments.inside) { 
                        if (logInfos) logInfo("Comments is " + comments.inside);
                        getterType = getPropertyType(comments.inside);
                        if (logInfos) logInfo("Matching getter type is " + getterType);
                        if (e.functionHelp.signatures) {
                            e.functionHelp.signatures.forEach(fixupSetterSignature);
                        }
                    }
                    else { 
                        if (logWarnings) logWarning("No doc-comments in matching getter " + getterName);
                    }
                }
                else {
                    if (logWarnings) logWarning("No matching getter for seter " + e.functionHelp.functionName);
                }
            }
        }

        var logErrors = !!intellisense["MicrosoftAjaxLogLevelError"];
        var logWarnings = !!intellisense["MicrosoftAjaxLogLevelWarning"];
        var logInfos = !!intellisense["MicrosoftAjaxLogLevelInfo"];

        if (logInfos) logInfo("In addEventListener(functionHelp)");

        var getterType = null;

        if (e.functionHelp.functionName.indexOf("set_") == 0 && e.functionHelp.functionName.length > "set_".length) {
            fixupSetter();
        }
    });

})();


