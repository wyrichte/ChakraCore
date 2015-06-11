/// <disable>JS2085,JS2038,JS2015,JS2064,JS2003,JS2043,JS2055,JS2087</disable>
// Copyright (C) Microsoft Corporation.  All rights reserved.

/// <reference path="Debug.js"/>
/// <reference path="dom.ref.js"/>
/// <disable>JS2026.CapitalizeComments,JS2058.DoNotUseIncrementAndDecrementOperators,JS2008.DoNotUseCookies</disable>
/// <dictionary>reg,attr,canonicalization,rect</dictionary>

///#DEBUG
(function () {

    /* @constructor*/function DebugConsole() {

        this._registers = [];
        this._createUI();
        this._catchErrors();
        this._loadCommandHistory();

        var that = this;
        window.addEventListener("keydown", function (/* @dynamic*/ev) {
            // Handles the ~ key press that will toggle the console's visibility.
            if (ev.keyCode === 192 && ev.shiftKey) {
                ev.preventDefault();
                that._toggleLayerVisibility();
            } else if (that._isVisible()) {
                // If the console is open and the user starts typing, put focus in the text
                // input.  But don't mess with keyboard navigation, copy-paste, etc.
                var controlKeys = [ "Up", "Down", "PageUp", "PageDown", "Home", "End" ];
                if (controlKeys.indexOf(ev.key) === -1 &&
                    !ev.ctrlKey && !ev.altKey) {
                    that._textInputElement.focus();
                }

                // Block all other key down handlers.
                ev.stopPropagation();
                ev.stopImmediatePropagation();

                // Because we do this, we need to manually call our text input handler.
                if (document.activeElement === that._textInputElement) {
                    that._onTextInput(ev);
                }
            }
        }, true);

        // If the user clicks on the > next to the input area, switch to multi-line input.
        this._textInputFlipElement.addEventListener("click", function () {
            if (that._isSingleLineMode()) {
                this.innerText = "<";
                that._textInputElement.className = "debugConsoleMultiLine";
            } else {
                this.innerText = ">";
                that._textInputElement.className = "debugConsoleSingleLine";
            }
            that._setCommand(that._textInputElement.value);
            that._textInputElement.focus();
        }, false);
    };
    DebugConsole.prototype._onTextInput = function (/* @dynamic*/ev) {
        this._textInputElement.scrollIntoView();
        if (ev.key === "Enter" && (ev.ctrlKey || this._isSingleLineMode())) {
            // Enter (Ctrl+Enter in multi-line mode) runs the command
            ev.preventDefault();
            var command = this._textInputElement.value.trim();
            this._textInputElement.value = "";
            if (command !== "") {

                // Dump the command to the output
                this._appendText("", "> " + toSingleLine(command));

                // Update the command history
                var index = -1;
                this._commandHistory.forEach(function (previousCommand, i) {
                    if (toSingleLine(command) === toSingleLine(previousCommand)) {
                        index = i;
                    }
                });
                if (index !== -1) {
                    var other = this._commandHistory.splice(index, 1)[0];
                    if (this._isSingleLineMode()) {
                        command = other;
                    }
                }
                this._commandHistory.unshift(command);
                this._commandHistory.length = Math.min(this._commandHistory.length, 20);
                this._commandHistoryPosition = -1;
                this._saveCommandHistory();

                // Execute the command
                var scriptElement = document.createElement("script");
                scriptElement.type = "text/javascript";
                scriptElement.innerText =
                    "with (Debug.console._getCommands()) {\n" +
                    (this._safety ? "    try {\n" : "") +
                    "        dump(stash(eval(" + JSON.stringify(command) + ")));\n" +
                    (this._safety ? "    } catch (ex) {\n" : "") +
                    (this._safety ? "        err(ex);\n" : "") +
                    (this._safety ? "    }\n" : "") +
                    "}";
                document.getElementsByTagName("head")[0].appendChild(scriptElement);
                document.getElementsByTagName("head")[0].removeChild(scriptElement);
                this._textInputElement.scrollIntoView();
            }
        } else if (ev.key === "Up" && (ev.ctrlKey || this._isSingleLineMode())) {
            ev.preventDefault();
            if (this._commandHistoryPosition < this._commandHistory.length - 1) {
                this._setCommand(this._commandHistory[++this._commandHistoryPosition]);
            }
        } else if (ev.key === "Down" && (ev.ctrlKey || this._isSingleLineMode())) {
            ev.preventDefault();
            if (this._commandHistoryPosition > 0) {
                this._setCommand(this._commandHistory[--this._commandHistoryPosition]);
            } else {
                this._commandHistoryPosition = -1;
                this._setCommand("");
            }
        } else if (ev.key === "Esc") {
            this._textInputElement.value = "";
        } else if (ev.key === "Tab") {
            ev.preventDefault();
            this._tabComplete();
        }
    };
    DebugConsole.prototype._tabComplete = function () {
        var fullText = this._textInputElement.value;

        // Tab completion does not include this set of unsavory characters.  It starts immediately after them.
        // This allows us to properly tab complete something like var x = document.b<TAB>, or the inner block of a 
        // while loop.  Note that cursor position is not currently considered.
        var /* @dynamic*/match = fullText.match(/[^=\s\(\){}\+\-\*\/,;&|!"']+$/);
        if (match !== null && match.length === 1 && match[0] !== ".") {
            var /* @type(String)*/candidate = match[0].trim();

            // Now we are looking for realPart.queryPart<TAB>
            var lastDot = candidate.lastIndexOf(".");
            var isGlobal = false;
            var realPart = "";
            var queryPart = "";
            if (lastDot === -1) {
                isGlobal = true;
                realPart = "window";
                queryPart = candidate;
            } else {
                isGlobal = false;
                realPart = candidate.substr(0,lastDot);
                queryPart = candidate.substr(lastDot + 1);
            }

            var realValue = undefined;
            try {
                // Evaluate the part of the expression before the dot
                ///<disable>JS2002.DoNotUseWithStatements,JS2001.DoNotUseEval</disable>
                with (this._getCommands()) {
                    realValue = eval(realPart);
                }
            } catch (ex) { }

            // Collect all of the field names into an object literal
            var matchingFields = {};
            for (var field in realValue) {
                if (field.substr(0, queryPart.length).toLowerCase() === queryPart.toLowerCase()) {
                    matchingFields[field] = null;
                }
            }

            var matchingFieldsArray = /* @static_cast(Array)*/Object.keys(matchingFields);
            if (matchingFieldsArray.length === 1) {
                // If we had only one match, drop it into the text input
                this._textInputElement.value = fullText.substr(0, match.index) + (isGlobal ? "" : (realPart + ".")) + matchingFieldsArray[0];
            } else if (matchingFieldsArray.length > 1) {
                // Otherwise, dump the set of matches 
                matchingFields.toString = function () { return matchingFieldsArray.length.toString() + " matches"; };
                this._appendTreeView(/* @static_cast(TreeView)*/new DataTreeView(candidate + "*", realValue, matchingFields, ["toString"]), true);
            }
        }
    };
    DebugConsole.prototype._createUI = function () {
        ///<summary>Creates all of the UI elements for the console, but does not add them to the view</summary>
        this._styleElement = document.createElement("style");
        this._styleElement.id = "debugConsoleStyles";
        this._styleElement.type = "text/css";
        this._styleElement.innerText =
            ".debugConsoleRoot {" +
                "width:0;" +
                "height:0;" +
            "}" +
            "#debugConsoleBackground {" +
                "text-align:left;" +
                "text-overflow:clip;" +
                "position:fixed;" +
                "top:0;" +
                "left:0;" +
                "width:100%;" +
                "height:100%;" +
                "z-index:100;" +
                "background-color:rgba(0,0,0,0.8);" +
                "overflow:hidden;" +
            "}" +
            "#debugConsoleLayer {" +
                "width:98%;" +
                "height:100%;" +
                "padding:0 1%;" +
                "color:white;" +
                "font-family:Segoe UI;" +
                "font-size:10pt;" +
                "overflow-x:hidden;" +
                "overflow-y:scroll;" +
            "}" +
            "#debugConsoleOutput {" +
                "width:100%;" +
            "}" +
            "#debugConsole div {" +
                "width:100%;" +
                "overflow:hidden;" +
            "}" +
            ".debugConsoleVerticalOverlay {" +
                "background-color:blue;" +
                "position:absolute;" +
                "z-index:100000;" +
            "}" +
            ".debugConsoleHorizontalOverlay {" +
                "background-color:blue;" +
                "position:absolute;" +
                "z-index:100000;" +
            "}" +
            ".debugConsoleLogText {" +
                "color:lightgreen;" +
                "font-style:italic;" +
            "}" +
            ".debugConsoleExceptionText {" +
                "color:red;" +
                "font-weight:bold;" +
            "}" +
            ".debugConsoleOutputText {" +
                "color:rgba(200,200,200,1);" +
            "}" +
            ".debugConsoleErrorText {" +
                "color:red;" +
            "}" +
            ".debugConsolePropertyExpander { " +
                "cursor:pointer;" +
                "margin-right:5px;" +
                "font-family:Consolas;" +
            "}" +
            ".debugConsolePropertyName {" +
                "color:rgba(200,100,200,1);" +
            "}" +
            ".debugConsoleObjectProperties {" +
                "margin-left:15px;" +
            "}" +
            "#debugConsoleInput {" +
                "font-family:inherit;" +
                "font-size:inherit;" +
                "border:none;" +
                "width:95%;" +
                "color:inherit;" +
                "padding:0px;" +
                "margin:0px;" +
            "}" +
            "#debugConsoleInput.debugConsoleSingleLine {" +
                "height: 1.5em;" +
                "overflow: hidden;" +
                "background:none;" +
            "}" +
            "#debugConsoleInput.debugConsoleMultiLine {" +
                "height: 10em;" +
                "overflow: auto;" +
                "background-color: rgba(0,0,0,0.4);" +
            "}" +
            "#debugConsoleInputFlip { " +
                "vertical-align:top;" +
                "cursor:pointer;" +
            "}" +
            ".debugConsoleRegister {" +
                "margin-left:5px;" +
                "color:cyan;" +
                "text-decoration:none;" +
            "}" +
            ".debugConsoleHtmlComment {" +
                "color:#d0d090;" +
                "font-style:italic;" +
            "}" +
            ".debugConsoleHtmlAttrName {" +
                "color:#b0d0f0;" +
            "}" +
            ".debugConsoleHtmlAttrValue {" +
                "color:#80c0e0;" +
            "}" +
            ".debugConsoleHtmlHref {" +
                "color:#98a8f0;" +
                "text-decoration: underline;" +
            "}" +
            ".debugConsoleHtmlBracket {" +
                "color:#f0c0f0;" +
            "}" +
            ".debugConsoleHtmlText {" +
                "color:#d0d0d0;" +
                "font-style:italic;" +
            "}" +
            ".debugConsoleCssOverriden {" +
                "color:red;" +
                "text-decoration:line-through;" +
            "}";

        this._rootElement = createElement("div", "debugConsoleRoot", null);
            this._rootElement.innerHTML =
                "<div id=debugConsoleBackground>" +
                    "<div id=debugConsoleLayer>" +
                        "<div id=debugConsoleOutput></div>" +
                        "<div>" +
                            "<span id=debugConsoleInputFlip title='Click here to switch between single and multi-line input'>&gt;</span> " +
                            "<textarea id=debugConsoleInput class=debugConsoleSingleLine></textarea>" +
                        "</div>" +
                    "</div>" +
                "</div>";
        this._layerElement = /* @static_cast(HTMLElement)*/this._rootElement.querySelector("#debugConsoleLayer");
        this._outputElement = /* @static_cast(HTMLElement)*/this._rootElement.querySelector("#debugConsoleOutput");
        this._textInputElement = /* @static_cast(HTMLElement)*/this._rootElement.querySelector("#debugConsoleInput");
        this._textInputElement.spellcheck = false;
        this._textInputFlipElement = /* @static_cast(HTMLElement)*/this._rootElement.querySelector("#debugConsoleInputFlip");
    };
    DebugConsole.prototype._appendOutput = function (div, parentElement) {
        ///<summary>Appends an element to the console</summary>
        ///<param name="div" type="HTMLElement">The element to append</param>
        ///<param name="parentElement" type="HTMLElement" optional="true">The parent element to append to</param>
        if (parentElement !== this._outputElement && parentElement !== undefined) {
            parentElement.appendChild(div);
        } else {
            parentElement = this._outputElement;

            var maintainScrollPosition = false;
            var stuckToBottom = false;
            if (this._rootElement.parentElement !== null) {
                maintainScrollPosition = true;
                var scrollTop = this._layerElement.scrollTop;
                var scrollHeight = this._layerElement.scrollHeight;
                var clientHeight = this._layerElement.clientHeight;
                stuckToBottom = (scrollHeight - scrollTop <= clientHeight + 10);
            }

            parentElement.appendChild(div);

            if (maintainScrollPosition && !stuckToBottom) {
                var newHeight = this._layerElement.scrollHeight;
            }

            while (parentElement.children.length > 1000) {
                parentElement.removeChild(parentElement.children[0]);
            }

            if (stuckToBottom) {
                this._layerElement.scrollTop = this._layerElement.scrollHeight - this._layerElement.clientHeight;
            } else if (maintainScrollPosition) {
                this._layerElement.scrollTop = scrollTop - (newHeight - this._layerElement.scrollHeight);
            }
        }
    };
    DebugConsole.prototype._clearOutput = function () {
        ///<summary>Clears the output area</summary>
        this._outputElement.innerHTML = "";
    };
    DebugConsole.prototype._appendText = function (className, text, parentElement) {
        ///<summary>Appends plaintext output with a specified class</summary>
        ///<param name="className" type="String"/>
        ///<param name="text" type="String"/>
        ///<param name="parentElement" type="HTMLElement" optional="true"/>
        this._appendOutput(createElement("div", className, text), parentElement);
    };
    DebugConsole.prototype._appendData = function (label, /* @dynamic*/data, shouldExpand) {
        ///<summary>Appends some JavaScript data to the output: could be a string/number/object/function/etc.</summary>
        ///<param name="label" type="String">A label for the data being appended</param>
        ///<param name="data">The object containing the data to append</param>
        ///<param name="shouldExpand" type="Boolean">Indicates whether child nodes should be shown by default</param>
        this._appendTreeView(/* @static_cast(TreeView)*/new DataTreeView(label, data, data, []), shouldExpand);
    };
    DebugConsole.prototype._appendHTML = function (node, shouldExpand) {
        ///<summary>Appends an HTML tree to the output.</summary>
        ///<param name="node" type="Node">The HTML node to append</param>
        ///<param name="shouldExpand" type="Boolean">Indicates whether child nodes should be shown by default</param>
        this._appendTreeView(/* @static_cast(TreeView)*/new HtmlTreeView(node), shouldExpand);
    };
    DebugConsole.prototype._appendCSS = function (element) {
        ///<summary>Finds the matching CSS rules for a specific element.</summary>
        ///<param name="element" type="HTMLElement">The html element to trace</param>
        this._appendTreeView(/* @static_cast(TreeView)*/new CssTraceTreeView(element), true);
    };
    DebugConsole.prototype._appendSheet = function (sheet, shouldExpand) {
        ///<summary>Appends a style sheet to the output.</summary>
        ///<param name="style" type="CSSStyleSheet">The sheet to append</param>
        ///<param name="shouldExpand" type="Boolean">Indicates whether child nodes should be shown by default</param>
        this._appendTreeView(/* @static_cast(TreeView)*/new CssTreeView(sheet), shouldExpand);
    };
    DebugConsole.prototype._appendRule = function (rule, shouldExpand) {
        ///<summary>Appends a CSS rule to the output.</summary>
        ///<param name="style" type="CSSSRule">The rule to append</param>
        ///<param name="shouldExpand" type="Boolean">Indicates whether child nodes should be shown by default</param>
        this._appendTreeView(/* @static_cast(TreeView)*/new CssRuleTreeView(rule, null), shouldExpand);
    };
    DebugConsole.prototype._appendStyle = function (style, shouldExpand) {
        ///<summary>Appends a specific CSS style to the output.</summary>
        ///<param name="style" type="Style">The style to append</param>
        ///<param name="shouldExpand" type="Boolean">Indicates whether child nodes should be shown by default</param>
        this._appendTreeView(/* @static_cast(TreeView)*/new CssStyleTreeView(style, null), shouldExpand);
    };
    DebugConsole.prototype._appendTreeView = function (treeView, shouldExpand, parentElement) {
        ///<summary>Appends a tree of data to the output</summary>
        ///<param name="treeView" type="TreeView">The output tree to append</param>
        ///<param name="shouldExpand" type="Boolean">Whether or not to expand the members the view. This function will make the final determination</param>
        ///<param name="parentElement" type="HTMLElement" optional="true">The container in which to append the tree</param>
        var canExpand = treeView.length > 0;
        if (treeView.isExpanded === undefined) {
            treeView.isExpanded = canExpand && shouldExpand && treeView.length <= 20;
        }

        var div = createElement("div", "debugConsoleOutputText", null);
        var expanderRegion = createElement("span", "debugConsolePropertyName", null, div);
        var expanderElement = createElement("span", "debugConsolePropertyExpander debugConsoleOutputText", "+", expanderRegion);
        if (isNonEmptyString(treeView.label)) {
            createElement("span", null, treeView.label + ": ", expanderRegion);
        }

        var valueElement = createElement("span", null, treeView.value, div);
        if (isObject(treeView.html)) {
            valueElement.innerHTML = treeView.html.outerHTML;
        } else if (isNonEmptyString(treeView.className)) {
            valueElement.className = treeView.className;
        }

        var that = this;
        var register = createElement("span", "debugConsoleRegister",
            (treeView.label !== "prototype") ? this._getRegisterName(treeView.register) : null, valueElement);
        valueElement.addEventListener("click", function (ev) {
            that._toggleRegister(treeView.register);
        }, false);
        register.onRegisterChange = function (registerValue, registerName) {
            if (registerValue === treeView.register && treeView.label !== "prototype") {
                this.innerText = registerName;
            }
        };

        var detailsElement = createElement("div", "debugConsoleObjectProperties", null, div);
        if (canExpand) {
            expanderRegion.addEventListener("click", function (ev) {
                if (detailsElement.innerHTML !== "") {
                    detailsElement.innerHTML = "";
                    expanderElement.innerText = "+";
                    treeView.isExpanded = false;
                } else {
                    expanderElement.innerText = "-";
                    that._expandTreeView(treeView, detailsElement);
                }
            }, false);
            if (treeView.isExpanded) {
                expanderElement.innerText = "-";
                that._expandTreeView(treeView, detailsElement);
            }
        } else {
            expanderElement.style.visibility = "hidden";
        }

        this._appendOutput(div, parentElement);
    };
    DebugConsole.prototype._expandTreeView = function (treeView, detailsElement) {
        ///<summary>Expands display of a tree view's children</summary>
        ///<param name="treeView" type="TreeView">The tree view whose children are being expanded</param>
        ///<param name="detailsElement" type="HTMLElement">The element that will contain members of this rule</param>
        if (treeView.children === undefined) {
            treeView.children = [];
            for (var i = 0, len = treeView.length; i < len; i++) {
                treeView.children.push(treeView.createChild(i));
            }
        }
        treeView.isExpanded = true;
        treeView.children.forEach(/* @bind(DebugConsole)*/function (child) {
            this._appendTreeView(child, false, detailsElement);
        }, this);
    };

    function TreeView() { };
    TreeView.prototype.label = "";
    TreeView.prototype.value = "";
    TreeView.prototype.className = "";
    TreeView.prototype.html = /*@static_cast(HTMLElement)*/null;
    TreeView.prototype.register = null;
    TreeView.prototype.length = 0;
    TreeView.prototype.isExpanded = false;
    TreeView.prototype.children = [];
    TreeView.prototype.createChild = function (index) { return null; };

    function DataTreeView(label, /* @dynamic*/data, /* @dynamic*/proto, excluded) {
        ///<summary>Appends some JavaScript data to the output: could be a string/number/object/function/etc.</summary>
        ///<param name="label" type="String">A label for the data being appended</param>
        ///<param name="data">The object containing the data to append</param>
        ///<param name="proto">The object in the prototype chain of data whose members should be displayed</param>
        ///<param name="excluded type="Object">An array of property names that should not be displayed, either because they have already been displayed or are otherwise unsavory</param>
        this.label = label;
        this._data = this.register = data;

        // Truncate the value
        this.value = valueToString(proto);
        if (isNonEmptyString(this.value)) {
            var maxLength = 1000;
            if (isNonEmptyString(label)) {
                this.value = this.value.replace(/\s+/g," ");
                maxLength = 100;
            }
            if (this.value.length > maxLength) {
                this.value = this.value.substr(0, maxLength) + "...";
            }
        }

        this._fields = getProperties(proto, true, excluded).sort(function (/* @type(String)*/a, /* @type(String)*/b) {
            // Functions go at the end
            if ((typeof data[a] === "function") !== (typeof data[b] === "function")) {
                return (typeof data[a] === "function") ? 1 : -1;
            }
            // When we dump arrays (or types like arrays), we want to avoid sorting numerical values using a string comparison
            var aAsNumber = Number(a);
            var bAsNumber = Number(b);
            var aIsNumber = aAsNumber !== null && aAsNumber !== NaN;
            var bIsNumber = bAsNumber !== null && bAsNumber !== NaN;
            if (aIsNumber && bIsNumber) {
                return aAsNumber - bAsNumber;
            }
            if (aIsNumber !== bIsNumber) {
                return aIsNumber ? 1 : -1;
            }
            // Privates after publics
            if ((a[0] === "_") !== (b[0] === "_")) {
                return (a[0] === "_") ? 1 : -1;
            }
            // Otherwise, sort alphabetically
            return a.localeCompare(b);
        });

        if (typeof data === "function" && !isEmpty(data.prototype, [])) {
            this._prototype = { data: data.prototype, proto: data.prototype, excluded: [] };
            this._fields.splice(0, 0, this._prototype);
        } else if (!isEmpty(proto, excluded)) {
            var nextPrototype = Object.getPrototypeOf(proto);
            var nextExcluded = excluded.concat(this._fields);
            if (!isEmpty(nextPrototype, nextExcluded)) {
                this._prototype = { data: data, proto: nextPrototype, excluded: nextExcluded };
                this._fields.splice(0, 0, this._prototype);
            }
        }

        this.length = this._fields.length;
    };
    DataTreeView.prototype.createChild = function (index) {
        if (this._prototype !== undefined && index === 0) {
            return new DataTreeView("prototype", this._prototype.data, this._prototype.proto, this._prototype.excluded);
        } else {
            var field = this._fields[index], value, className;
            try {
                value = this._data[field];
                className = "debugConsoleOutputText";
            } catch (ex) {
                value = ex;
                className = "debugConsoleErrorText";
            }
            var treeView = new DataTreeView(field, value, value, []);
            treeView.className = className;
            return treeView;
        }
    };

    function HtmlTreeView(node) {
        this._element = this.register = node;

        if (node instanceof HTMLStyleElement) {
            var style = /*@static_cast(HTMLStyleElement)*/node;
            this._displayable = style.styleSheet.cssRules;
        } else if (node instanceof HTMLLinkElement && node.type === "text/css") {
            var link = /*@static_cast(HTMLLinkElement)*/node;
            this._displayable = link.styleSheet.cssRules;
        } else {
            this._displayable = [];
            for (var i = 0, len = node.childNodes.length; i < len; i++) {
                if (isDisplayableHtmlNode(node.childNodes[i])) {
                    this._displayable.push(node.childNodes[i]);
                }
            }
        }
        this.length = this._displayable.length;

        if (node.nodeType === Node.TEXT_NODE) {
            // Simple text node
            this.className = "debugConsoleHtmlText";
            var textNode = /* @static_cast(TextNode)*/node;
            this.value = toSingleLine(textNode.textContent.trim());
        } else if (node.nodeType === Node.COMMENT_NODE) {
            this.className = "debugConsoleHtmlComment";
            this.value = toSingleLine(node.text);
        } else {
            var htmlNode = /*@static_cast(HTMLElement)*/node;
            this.html = createElement("span", null, null);
            createElement("span", "debugConsoleHtmlBracket", "<", this.html);
            createElement("span", "debugConsolePropertyName", htmlNode.nodeName.toLowerCase(), this.html);

            // Attributes - id first, then class, everything else last
            if (isNonEmptyString(htmlNode.id)) {
                this._appendHtmlAttribute("id", htmlNode.id);
            }
            if (isNonEmptyString(htmlNode.className)) {
                this._appendHtmlAttribute("class", htmlNode.className);
            }
            for (i = 0, len = htmlNode.attributes.length; i < len; i++) {
                var attribute = /* @static_cast(Attr)*/htmlNode.attributes[i];
                if (attribute.name !== "id" && attribute.name !== "class") {
                    this._appendHtmlAttribute(attribute.name, attribute.value);
                }
            }

            createElement("span", "debugConsoleHtmlBracket", ">", this.html);
        }
    };
    HtmlTreeView.prototype.createChild = function (index) {
        var child = this._displayable[index];
        if (child instanceof CSSRule) {
            return new CssRuleTreeView(this._displayable[index]);
        } else {
            return new HtmlTreeView(this._displayable[index]);
        }
    };
    HtmlTreeView.prototype._appendHtmlAttribute = function (attrName, attrValue) {
        ///<summary>Displays an attribute node when expanding a tree of html elements</summary>
        ///<param name="attrName" type="String">The name of the attribute</param>
        ///<param name="attrValue" type="String">The value of the attribute</param>
        createElement("span", null, " ", this.html);
        createElement("span", "debugConsoleHtmlAttrName", attrName, this.html);
        createElement("span", null, "=\"", this.html);
        if (attrName === "href") {
            var link = createElement("a", "debugConsoleHtmlHref", attrValue, this.html);
            link.href = attrValue;
            link.target = "_new";
        } else {
            createElement("span", "debugConsoleHtmlAttrValue", attrValue, this.html);
        }
        createElement("span", null, "\"", this.html);
    };

    function CssTraceTreeView(element) {
        ///<summary>Appends CSS rules that match the element to the outptut.</summary>
        ///<param name="element" type="HTMLElement">The HTML element to match</param>
        this._element = element;
        this._matches = [];

        // Include inline styles by creating an object that looks like a CSS rule
        if (element.style.length > 0) {
            this._matches.push(new CssStyleTreeView(element.style, "<inline-styles>", element));
        }

        var parentElement = isObject(element.parentElement) ? element.parentElement : element;
        var sheets = document.styleSheets;
        for (var i = 0, iLen = sheets.length; i < iLen; i++) {
            var rules = /* @static_cast(Array)*/sheets[i].cssRules;
            for (var j = 0, jLen = rules.length; j < jLen; j++) {
                var rule = /* @static_cast(CSSRule)*/rules[j],
                    elements = parentElement.querySelectorAll(rule.selectorText);
                for (var k = 0, kLen = elements.length; k < kLen; k++) {
                    if (elements[k] === element) {
                        this._matches.push(new CssRuleTreeView(rule, element));
                        break;
                    }
                }
            }
        }

        this.label = "trace";
        this.value = this.register = element;
        this.length = this._matches.length;
    };
    CssTraceTreeView.prototype.createChild = function (index) {
        return this._matches[index];
    };

    function CssTreeView(sheet) {
        ///<summary>Builds a tree of CSS rules from a style sheet</summary>
        ///<param name="sheet" type="CSSStyleSheet">The sheet to consider</param>
        if (isNonEmptyString(sheet.href)) {
            this.value = sheet.href;
        } else if (isNonEmptyString(sheet.id)) {
            this.value = sheet.id;
        } else if (isNonEmptyString(sheet.title)) {
            this.value = sheet.title;
        }

        this.register = this._sheet = sheet;
        this.label = "css";
        this.length = sheet.cssRules.length;
    };
    CssTreeView.prototype.createChild = function (index) {
        return new CssRuleTreeView(this._sheet.cssRules[index]);
    };

    function CssRuleTreeView(rule, match) {
        ///<summary>Builds a tree of CSS styles from a rule</summary>
        ///<param name="rule" type="CSSRule">The rule to consider</param>
        ///<param name="match" type="HTMLElement" optional="true">An element that matches this rule</param>
        if (rule.type === rule.STYLE_RULE) {
            mixIn(this, new CssStyleTreeView(rule.style, rule.selectorText, match));
            this.createChild = CssStyleTreeView.prototype.createChild;
            this.label = isObject(match) ? "match" : "rule";
        } else if (rule.type === rule.IMPORT_RULE) {
            this.label = "@import";
            this.value = rule.href;
            this.length = rule.styleSheet.cssRules.length;
        } else {
            this.label = "rule";
            this.value = rule.cssText;
            this.length = 0;
        }

        this._rule = this.register = rule;
        this._match = match;
    };
    CssRuleTreeView.prototype.createChild = function (index) {
        var rule = this._rule;
        if (rule.type === rule.IMPORT_RULE) {
            return new CssRuleTreeView(rule.styleSheet.cssRules[index], this._match);
        }
        return null;
    };

    function CssStyleTreeView(style, value, match) {
        ///<summary>Builds a tree of CSS properties from a style</summary>
        ///<param name="rule" type="Style">The style to consider</param>
        ///<param name="value" type="String">The value to display</param>
        ///<param name="match" type="HTMLElement" optional="true">An element that matches this style</param>
        this._style = this.register = style;
        this._match = match;
        this.label = "style";
        this.value = value;

        this._children = [];
        for (var i = 0, len = style.length; i < len; i++) {
            if (this.cssIgnoreName[style[i]] === undefined) {
                var parts = /* @static_cast(Array)*/style[i].split("-");
                if (/^-ms-/.test(style[i])) {
                    parts.splice(0, 3, "-ms-" + parts[2]);
                }
                this._insertChild(parts);
            }
        }

        this.length = this._children.length;
    };
    CssStyleTreeView.prototype._insertChild = function (parts) {
        for (var i = 0, len = this._children.length; i < len; i++) {
            if (this._children[i].insert(parts)) {
                return;
            }
        }
        this._children.push(new CssPropertyTreeView(this._style, null, parts, this._match));
    };
    CssStyleTreeView.prototype.createChild = function (index) {
        return this._children[index].getLeaf();
    };
    // CSS property names that just don't exist in JS
    CssStyleTreeView.prototype.cssIgnoreName = {};
    CssStyleTreeView.prototype.cssIgnoreName["-ms-text-size-adjust"] = true;

    function CssPropertyTreeView(style, base, remaining, match) {
        this._style = style;
        this._match = match;

        this.part = remaining.splice(0, 1)[0];
        this.label = isNonEmptyString(base) ? base + "-" + this.part : this.part;

        this._children = [];
        if (remaining.length > 0) {
            this._children.push(new CssPropertyTreeView(style, this.label, remaining, match));
        }

        this.value = this._style[this.cssFixedName[this.label]];
        if (this.value === undefined) {
            // Standard conversion, e.g. padding-left=>paddingLeft, -ms-flex-box=>msFlexBox
            var indexer = this.label.replace(/^-ms/, "ms");
            indexer = indexer.replace(/-\w/g, function (m) { return m[1].toUpperCase(); });
            this.value = this._style[indexer];

            if (this.value === undefined) {
                // Some -ms-* properties don't use the 'ms' prefix in their JS field name
                indexer = indexer.replace(/^ms\w/, function (m) { return m[2].toLowerCase(); });
                this.value = this._style[indexer];
            }
        }

        if (this.value !== undefined) {
            this.register = this.value;
            if (isObject(this._match) && this._children.length === 0 && !areCssValuesEqual(this.value, this._match.currentStyle[indexer])) {
                // Show a red strike through properties that don't match
                this.html = createElement("span", "debugConsoleCssOverriden", null);
                createElement("span", "debugConsoleOutputText", this.value, this.html);
            }
        }
    };
    CssPropertyTreeView.prototype.createChild = function (index) {
        return this._children[index].getLeaf();
    };
    Object.defineProperty(CssPropertyTreeView.prototype, "length", { get: /* @bind(CssPropertyTreeView)*/function () {
        return this._children.length;
    } });
    CssPropertyTreeView.prototype.insert = function (/*@type(Array)*/parts) {
        if (this.part === parts[0]) {
            parts.splice(0, 1);
            if (parts.length > 0) {
                for (var i = 0, len = this._children.length; i < len; i++) {
                    if (this._children[i].insert(parts)) {
                        return true;
                    }
                }
                this._children.push(new CssPropertyTreeView(this._style, this.label, parts, this._match));
            }
            return true;
        } else {
            return false;
        }
    };
    CssPropertyTreeView.prototype.getLeaf = function () {
        if (this._children.length === 1) {
            return this._children[0].getLeaf();
        } else {
            return this;
        }
    };
    CssPropertyTreeView.prototype.cssFixedName = {
        /// <disable>JS2074.IdentifierNameIsMisspelled</disable>
        "-ms-scrollbar-3dlight-color": "scrollbar3dLightColor",
        "scrollbar-3dlight-color": "scrollbar3dLightColor",
        "-ms-scrollbar-darkshadow-color": "scrollbarDarkShadowColor",
        "scrollbar-darkshadow-color": "scrollbarDarkShadowColor",
        "-ms-transform-origin-x": "msTransformOrigin",
        "-ms-transform-origin-y": "msTransformOrigin" };

    function ElementEvents(/* @type(function)*/callback, /* @dynamic*/context) {
        var domEvents = {}, body = document.body, selection = body, border = 2,
            root = createElement("div", "debugConsoleRoot", null, body),
            overlayLeft = createElement("div", "debugConsoleVerticalOverlay", null, root).style,
            overlayRight = createElement("div", "debugConsoleVerticalOverlay", null, root).style,
            overlayTop = createElement("div", "debugConsoleHorizontalOverlay", null, root).style,
            overlayBottom = createElement("div", "debugConsoleHorizontalOverlay", null, root).style;
            overlayLeft.width = overlayRight.width = overlayTop.height = overlayBottom.height = String(border) + "px";

        function restoreConsole() {
            for (var evName in domEvents) {
                document.removeEventListener(evName, domEvents[evName], true);
            }
            body.removeChild(root);
            callback.call(context, selection);
        }

        function positionOverlay() {
            var rect = selection.getBoundingClientRect(),
                offsetLeft = rect.left + body.scrollLeft - body.clientLeft,
                offsetTop = rect.top + body.scrollTop - body.clientTop,
                selectionWidth = selection.offsetWidth,
                selectionHeight = selection.offsetHeight;

            overlayLeft.left = overlayTop.left = overlayBottom.left = String(offsetLeft) + "px";
            overlayRight.left = String(offsetLeft + selectionWidth - border) + "px";
            overlayLeft.top = overlayRight.top = overlayTop.top = String(offsetTop) + "px";
            overlayBottom.top = String(offsetTop + selectionHeight - border) + "px";
            overlayLeft.height = overlayRight.height = String(selectionHeight) + "px";
            overlayTop.width = overlayBottom.width = String(selectionWidth) + "px";
        };

        domEvents.mousemove = function (/*@dynamic*/ev) {
            var target = document.elementFromPoint(ev.clientX, ev.clientY);
            if (/^debugConsole/.test(target.className)) {
                return;
            }
            if (target !== selection) {
                selection = target;
                positionOverlay();
            }
        };
        domEvents.click = function (/*@dynamic*/ev) {
            ev.preventDefault();
            ev.stopPropagation();
            restoreConsole();
        };
        domEvents.keydown = function (/*@dynamic*/ev) {
            if (ev.keyCode === 192 && ev.shiftKey) {
                // tilde key - stop selection before the console goes away
                restoreConsole();
            }
            if (ev.key === "Esc") {
                ev.preventDefault();
                ev.stopPropagation();
                selection = null;
                restoreConsole();
            }
        };

        for (var eventName in domEvents) {
            document.addEventListener(eventName, domEvents[eventName], true);
        }

        // Initial position on the body
        positionOverlay();
    }
    DebugConsole.prototype.stashHtml = function (element) {
        ///<summary>Called with the resulting element of a _selectHtml call</summary>
        ///<param name="element" type="HTMLElement">The element to stash in a register</param>
        this._rootElement.style.display = "";
        if (isObject(element)) {
            this._toggleRegister(element);
            this._appendHTML(/*@static_cast(Node)*/element, true);
        }
        this._textInputElement.focus();
    };
    DebugConsole.prototype._selectHtml = function () {
        ///<summary>Temporarily hides the console to allow selection of HTML elements via the UI</summary>
        this._rootElement.style.display = "none";
        new ElementEvents(this.stashHtml, this);
    };
    DebugConsole.prototype.log = function (level, text) {
        ///<summary>Logs text to the console, appends timetamped output</summary>
        ///<param name="level">Unused</param>
        ///<param name="text" type="String"/>
        var secondsSinceStart = (Date.now() - this._startTime) / 1000;
        this._appendText("debugConsoleLogText", "Log@" + secondsSinceStart.toString() + ": " + text);
    };
    DebugConsole.prototype._isVisible = function () {
        ///<summary>Checks whether the console is currently visible</summary>
        ///<returns type="Boolean"/>
        return (this._rootElement.parentElement !== null);
    };
    DebugConsole.prototype._toggleLayerVisibility = function () {
        ///<summary>Shows/hides the console.  Actually tears the elements out of the tree so that the console is invisible to debuggers and other code until called upon</summary>
        if (!this._isVisible()) {
            document.getElementsByTagName("head")[0].appendChild(this._styleElement);
            document.body.appendChild(this._rootElement);
            this._textInputElement.focus();
        } else {
            document.getElementsByTagName("head")[0].removeChild(this._styleElement);
            document.body.removeChild(this._rootElement);
        }
    };
    DebugConsole.prototype._catchErrors = function () {
        ///<summary>Hooks window.onerror to output exception information</summary>
        var that = this;
        window["onerror"] = function (msg, file, line) {
            that._appendText("debugConsoleExceptionText", "Unhandled exception: " + msg + "\n  file: " + file + "\n  line: " + line);
            return false;
        };
    };
    DebugConsole.prototype._loadCommandHistory = function () {
        ///<summary>Loads stored command history from a cookie</summary>
        this._commandHistory = [];
        try {
            document.cookie.split(";").forEach(function (/* @type(String)*/cookie) {
                var split = cookie.split("=");
                var cookieName = split[0].trim();
                if (cookieName === "consoleCommandHistory") {
                    this._commandHistory = JSON.parse(unescape(split[1]));
                }
            }, /* @static_cast(Array)*/this);
        } catch (ex) { }
    };
    DebugConsole.prototype._saveCommandHistory = function () {
        ///<summary>Saves command history to the cookie</summary>
        var date = new Date();
        date.setFullYear(date.getFullYear() + 1);
        document.cookie = "consoleCommandHistory=" + escape(JSON.stringify(this._commandHistory)) + "; expires=" + date.toUTCString();
    };
    DebugConsole.prototype._setCommand = function (value) {
        ///<summary>Sets a value into the text input.  Handles single-line/multi-line cases</summary>
        ///<param name="value" type="String"/>
        if (this._isSingleLineMode()) {
            value = toSingleLine(value);
        } else if (this._commandHistoryPosition >= 0 && this._commandHistoryPosition < this._commandHistory.length &&
                   toSingleLine(this._textInputElement.value) === toSingleLine(this._commandHistory[this._commandHistoryPosition])) {
            value = this._commandHistory[this._commandHistoryPosition];
        }
        this._textInputElement.value = value;
    };
    DebugConsole.prototype._isSingleLineMode = function () {
        ///<summary>Checks whether the input element is in single-line or multi-line input mode</summary>
        ///<returns type="Boolean"/>
        return this._textInputElement.className === "debugConsoleSingleLine";
    };
    DebugConsole.prototype._setSafety = function (enableSafety) {
        ///<summary>Enables or disables the "safety": the try/catch around evaluation of command line input</summary>
        ///<param name="enableSafety" type="Boolean"/>
        this._safety = enableSafety;
        this._appendText("debugConsoleOutputText", "Safety " + (enableSafety ? "enabled" : "disabled"));
    };
    DebugConsole.prototype._stash = function (/* @dynamic*/value) {
        ///<summary>Stores a value into the stash register:  $</summary>
        ///<param name="value"/>
        this._registers[0] = value;
    };
    DebugConsole.prototype._toggleRegister = function (/* @dynamic*/value) {
        ///<summary>Stores the specified value into a register, or clears it from one if it is already stored</summary>
        ///<param name="value">The value to save or clear</param>

        // If this value is already in a register, clear it
        for (var i = 1, len = this._registers.length; i < len; i++) {
            if (this._registers[i] === value) {
                this._registers[i] = undefined;
                this._updateRegisterElements(value, "");
                return;
            }
        }

        // Otherwise, set it to the first available register
        for (i = 1, len = this._registers.length; i < len; i++) {
            if (this._registers[i] === undefined) {
                break;
            }
        }
        this._registers[i] = value;
        this._updateRegisterElements(value, "$" + i.toString());
    };
    DebugConsole.prototype._getRegisterName = function (/* @dynamic*/value) {
        ///<summary>Returns the name of a register that contains this value, or an empty string</summary>
        ///<param name="value">The value to find</param>
        var registerName = "";
        if (value !== undefined) {
            var registerIndex = this._registers.indexOf(value, 1);
            if (registerIndex !== -1) {
                registerName = "$" + registerIndex.toString();
            }
        }
        return registerName;
    };
    DebugConsole.prototype._updateRegisterElements = function (value, registerName) {
        ///<summary>Updates all of the register elements based on a register change</summary>
        ///<param name="value">The value that has been stored or cleared</param>
        ///<param name="registerName" type="String">The name of the register containing this value, or empty string if none</param>
        var registerElements = document.querySelectorAll(".debugConsoleRegister");
        for (var i = 0, len = registerElements.length; i < len; i++) {
            registerElements[i].onRegisterChange(value, registerName);
        }
    };
    DebugConsole.prototype._loadScriptFile = function (path) {
        var scriptElement = document.createElement("script");
        var that = this;
        scriptElement.onload = function () {
            that._appendText("debugConsoleOutputText", "Script file loaded: " + path);
        };
        scriptElement.onerror = function () {
            that._appendText("debugConsoleErrorText", "Error loading script file: " + path);
        };
        scriptElement.type = "text/javascript";
        scriptElement.src = path;
        document.getElementsByTagName("head")[0].appendChild(scriptElement);
    };
    DebugConsole.prototype._getCommands = function () {
        ///<returns type="ConsoleCommands">A ConsoleCommands object to be used as a context for command-line evaluation</returns>
        return new ConsoleCommands(this, this._registers);
    };
    DebugConsole.prototype._startTime = Date.now();
    DebugConsole.prototype._styleElement = /* @static_cast(HTMLElement)*/null;
    DebugConsole.prototype._rootElement = /* @static_cast(HTMLElement)*/null;
    DebugConsole.prototype._layerElement = /* @static_cast(HTMLElement)*/null;
    DebugConsole.prototype._outputElement = /* @static_cast(HTMLElement)*/null;
    DebugConsole.prototype._textInputElement = /* @static_cast(HTMLElement)*/null;
    DebugConsole.prototype._textInputFlipElement = /* @static_cast(HTMLElement)*/null;
    DebugConsole.prototype._commandHistory = /* @static_cast(Array)*/null;
    DebugConsole.prototype._commandHistoryPosition = -1;
    DebugConsole.prototype._safety = true;

    function ConsoleCommands(debugConsole, registers) {
        // These commands are exposed to the console using a "with" statement.
        // This enables them to be run without any context, and without polluting the window
        // object.
        this._console = debugConsole;
        Object.defineProperty(/* @static_cast(Object)*/this, "$", { value: registers[0] });
        for (var i = 1, len = registers.length; i < len; i++) {
            Object.defineProperty(/* @static_cast(Object)*/this, "$" + i.toString(), { value: registers[i] });
        }
    };
    ConsoleCommands.prototype.dump = function (data, label) {
        if (data !== undefined || label !== undefined) {
            this._console._appendData(label, data, true);
        }
    };
    ConsoleCommands.prototype.log = function (data) {
        if (data !== undefined) {
           this._console._appendData(null, data, true);
        }
    };
    ConsoleCommands.prototype.html = function (data) {
        if (data === undefined) {
            data = document.documentElement;
        }
        if (data instanceof Node) {
            this._console._appendHTML(/* @static_cast(Node)*/data, true);
        } else {
            this.err("Not an HTML node");
        }
    };
    ConsoleCommands.prototype.css = function (/* @dynamic*/data) {
        if (data instanceof HTMLElement) {
            this._console._appendCSS(data);
        } else if (data instanceof CSSStyleDeclaration) {
            this._console._appendStyle(data, true);
        } else if (data instanceof CSSRule) {
            this._console._appendRule(data, true);
        } else if (data instanceof CSSStyleSheet) {
            this._console._appendSheet(data, true);
        } else if (data === undefined) {
            var sheets = document.styleSheets;
            for (var i = 0, len = sheets.length; i < len; i++) {
                var sheet = /* @static_cast(CSSStyleSheet)*/sheets[i];
                if (sheet.id !== "debugConsoleStyles") {
                    this._console._appendSheet(sheet, false);
                }
            }
        } else {
            this.err("Not a valid object type");
        }
    };
    ConsoleCommands.prototype.err = function (msg) {
         this._console._appendText("debugConsoleErrorText", msg);
    };
    ConsoleCommands.prototype.load = function (path) {
        this._console._loadScriptFile(path);
    };
    function EventDetails(source, eventName, eventArguments) {
        this.source = source;
        this.event = eventName;
        this.arguments = eventArguments;
    }
    ConsoleCommands.prototype.hook = function (/* @dynamic*/source, eventName) {
        var that = this;
        function listener() {
            that.dump(new EventDetails(source, eventName, arguments), "Event fired");
        }
        if (isObject(source)) {
            if (source.addEventListener !== undefined) {
                source.addEventListener(eventName, listener, false);
            } else if (source.addListener !== undefined) {
                source.addListener(eventName, listener);
            } else if (source.on !== undefined) {
                source.on(eventName, listener);
            }
        }
    };
    ConsoleCommands.prototype.safety = function (enableSafety) {
         this._console._setSafety(enableSafety);
    };
    Object.defineProperty(ConsoleCommands.prototype, "cls", { get: /* @bind(ConsoleCommands)*/function () {
         this._console._clearOutput();
    } });
    Object.defineProperty(ConsoleCommands.prototype, "select", { get: /* @bind(ConsoleCommands)*/function () {
         this._console._selectHtml();
    } });
    Object.defineProperty(ConsoleCommands.prototype, "help", { get: /* @bind(ConsoleCommands)*/function () {
        var helpText = createElement("div", "debugConsoleOutputText",
            "The console command-line accepts any JavaScript code and dumps the result. There are a number of built-in commands available for use:\n" +
            "\n" +
            "   help - displays the help text in the output area\n" +
            "   cls - clears the output area\n" +
            "   log(string) - outputs a string\n" +
            "   dump(obj [, label]) - dumps an object with an optional label\n" +
            "   html([node]) - dumps the HTML tree, optionally rooted to a specific node\n" +
            "   css([element|sheet|rule|style]) - dumps CSS properties, optionally matching a provided object\n" +
            "   select - temporarily hides the console to allow selection of an HTML element from the page\n" +
            "   hook(obj, event) - hooks an event on an object and dumps when it fires\n" +
            "   load(string) - loads the specified javascript file\n" +
            "   safety(bool) - enables/disables catching errors thrown from console evaluation\n" +
            "\n" +
            "You can click the > prompt next to the input area to toggle between single-line and multi-line input modes. In multi-line mode, use Ctrl+Enter to execute commands.\n" +
            "\n" +
            "The console uses cookies to persist a cross-session command history. Use the arrow keys (or Ctrl+arrow in multi-line mode) to repeat past commands.\n" +
            "\n" +
            "The result of the last evaluation is stored in a stash register which is accessed via the $ symbol. Clicking on any value in the output will store that value into a numbered register, like $1 or $2. All of these registers can be used anywhere in an expression. Note that a register is a value, not an alias to a property. If the property that the registers was created from changes, the register will not.\n" +
            "\n" +
            "Uncaught exceptions are displayed in the output area when the JavaScript engine isn’t running in debug mode.\n" +
            "\n" +
            "In depth usage instructions can be found at ");

        var link = createElement("a", "debugConsoleHtmlHref", "http://codebox/jsconsole/wiki/documentation", helpText);
        link.href = "http://codebox/jsconsole/wiki/documentation";
        link.target = "_new";

        this._console._appendOutput(helpText);
    } });
    ConsoleCommands.prototype.stash = function (value) {
        this._console._stash(value);
        return value;
    };

    function getProperties(/* @dynamic*/data, own, excluded) {
        ///<param name="data">The object whose properties to enumerate</param>
        ///<param name="own" type="Boolean">Controls whether only the properties of this object are returned, or properties from the entire prototype chain</param>
        ///<param name="excluded" type="Array">Names of properties that should not be returned</param>
        var props = [];
        if (data) {

            var hasProps = false;
            if (typeof data === "function") {
                hasProps = true;
                Array.prototype.push.apply(excluded, Object.getOwnPropertyNames(/* @static_cast(Object)*/Function));
            } else if (typeof data === "object") {
                hasProps = true;
            }

            if (hasProps) {
                var /* @dynamic*/prototype = data;
                do {
                    if (prototype === Object.prototype || prototype === Array.prototype || prototype === Function.prototype) {
                        break;
                    }
                    var prototypeProperties = /* @static_cast(Array)*/Object.getOwnPropertyNames(prototype);
                    for (var i = 0, len = prototypeProperties.length; i < len; i++) {
                        var propertyName = prototypeProperties[i];
                        if (props.indexOf(propertyName) === -1 &&
                            excluded.indexOf(propertyName) === -1 &&
                            propertyName !== "constructor") {
                            props.push(propertyName);
                        }
                    }
                    prototype = Object.getPrototypeOf(prototype);
                } while (!own && prototype);
            }
        }
        return props;
    }

    function isEmpty(/* @dynamic*/data, excluded) {
        ///<summary>Returns whether any properties would be shown when expanding this object</summary>
        ///<param name="data">The object to examine</param>
        ///<param name="excluded" type="Array">A set of property names that should not be output</param>
        var empty = getProperties(data, false, excluded).length === 0;
        if (typeof data === "function") {
            empty = empty && isEmpty(data.prototype, []);
        }
        return empty;
    }

    function getConstructorName(constructorFunction) {
        ///<summary>Tries to determine the name of a given constructor function.  Used to identify object types.</summary>
        ///<param name="constructorFunction" type="Function"/>

        // Look for the constructor by searching the namespaces
        var constructorName = searchForConstructor(constructorFunction, /* @static_cast(Object)*/window, [], 0);

        // Otherwise, see if the constructor function has a name
        if (constructorName === null) {
            var code = constructorFunction.toString();
            if (code.substr(0, 9) === "function ") {
                var parenthesisIndex = code.indexOf("(");
                if (parenthesisIndex !== -1) {
                    var possibleName = code.substr(9, parenthesisIndex - 9).trim();
                    if (possibleName.match(/^[\w$]+$/) !== null) {
                        constructorName = possibleName;
                    }
                }
            }
        }
        return constructorName;
    }

    function searchForConstructor(constructorFunction, namespace, alreadyChecked, depth) {
        ///<summary>Searches the namespace hierarchy recursively, trying to find the given constructor</summary>
        ///<param name="constructorFunction" type="Function">The constructor we are trying to locate</param>
        ///<param name="namespace" type="Object">The namespace to searching</param>
        ///<param name="alreadyChecked" type="Array">A set of namespaces that have already been examined, used as a guard against infinite recursion or wastefulness</param>
        ///<param name="depth" type="Number">The recursion depth, used to avoid excessive cost in fruitless lookups</param>
        if (depth < 10 && alreadyChecked.length < 1000) {
            for (var field in namespace) {
                ///<disable>JS3057.AvoidImplicitTypeCoercion</disable> JSCop doesn't like the string comparisons here
                if ((field[0] >= 'A' && field[0] <= 'Z') || field[0] === '$') { // namespaces and constructors start with a capital letter
                    var /* @dynamic*/objectToCheck = namespace[field];
                    var isNamespace = typeof objectToCheck === "object" && objectToCheck !== null && objectToCheck.constructor === Object;
                    var isFunction = typeof objectToCheck === "function";
                    if (isFunction && objectToCheck === constructorFunction) {
                        return field;
                    }
                    if ((isNamespace || isFunction) && alreadyChecked.indexOf(objectToCheck) === -1) {
                        alreadyChecked.push(objectToCheck);
                        var childField = searchForConstructor(constructorFunction, objectToCheck, alreadyChecked, depth + 1);
                        if (childField !== null) {
                            return field + "." + childField;
                        }
                    }
                }
            }
        }
        return null;
    }

    function toSingleLine(value) {
        ///<summary>Converts the given string to a single line</summary>
        ///<param name="value" type="String"/>
        ///<returns type="String"/>
        return value.replace(/\s*[\r\n]\s*/g, " ");
    }

    function valueToString(/* @dynamic*/data, /* @optional*/isRecursive) {
        ///<summary>Converts the provided value (object/function/array/string/number/etc) to a string representation</summary>
        ///<param name="data">The object to convert to a string</param>
        ///<param name="isRecursive" type="Boolean" optional="true">The function will recurse when printing an array, but should only do so once</param>
        ///<returns type="String"/>
        var string = "{...}";
        try {
            switch (typeof data) {
                case "undefined":
                case "string":
                case "number":
                case "boolean":
                    string = JSON.stringify(data);
                    break;
                default:
                    if (data === null) {
                        string = "null";
                    } else if (Array.isArray(data)) {
                        string = "length=" + data.length.toString();
                        if (data.length < 20 && !isRecursive) {
                            string += " [" + data.map(function (item) { return valueToString(item, true); }).join(", ") + "]";
                        }
                    } else {
                        if (data.toString) {
                            string = data.toString();
                        } else {
                            string = "" + data;
                        }
                    }

                    if (string === "[object Object]") {
                       var constructorName = getConstructorName(data.constructor);
                       if (constructorName !== null) {
                           string = "[object " + constructorName + "]";
                       }
                    }

                    // Decorate the string with unique identifiers
                    if ("name" in data && data.name) {
                       string = string + " " + data.name;
                    }
                    if ("id" in data && data.id) {
                       string = string + " #" + data.id;
                    }
                    if ("className" in data && data.className) {
                       string = string + data.className.trim().split(/\s+/).map(function (className) { return "." + className; }).join(" ");
                    }
                    if ("objectId" in data && data.objectId) {
                        string = string + " " + data.objectId;
                    }
                    break;
            }
        } catch (ex) { }
        return string;
    }

    function isDisplayableHtmlNode(node) {
        ///<summary>Determines if a specific HTML element should displayed when dumping the tree</summary>
        ///<param name="node" type="Node">The element to test</param>
        switch (node.nodeType) {
            case Node.TEXT_NODE:
            case Node.COMMENT_NODE:
                var textNode = /* @static_cast(TextNode)*/node;
                return textNode.textContent.trim().length > 0;
        }
        return true;
    }

    function createElement(nodeType, className, innerText, parentElement) {
        ///<summary>Creates an element of the specified type, setting the inner text and class name and optionally appends
        ///it to the provided parent element.</summary>
        ///<param name="nodeType" type="String">The type of HTML element to create</param>
        ///<param name="className" type="String">The class name value to set</param>
        ///<param name="innerText" type="String">The text value to set</param>
        ///<param name="parentElement" type="HTMLElement" optional="true">The parent element to append to</param>
        var element = document.createElement(nodeType);
        if (isNonEmptyString(className)) {
            element.className = className;
        }
        // ConsoleCommands.err calls this with an exception that we want to be sure to display
        if (isNonEmptyString(innerText) || isObject(innerText)) {
            element.innerText = innerText;
        }
        if (isObject(parentElement)) {
            parentElement.appendChild(element);
        }
        return element;
    }

    function areCssValuesEqual(valueA, valueB) {
        ///<param name="valueA" type="String"/>
        ///<param name="valueB" type="String"/>

        // basic string compare
        if (valueA.trim() === valueB.trim()) {
            return true;
        }

        // try comparing as numbers, for floating point canonicalization
        var aAsNumber = Number(valueA);
        var bAsNumber = Number(valueB);
        return aAsNumber !== null && aAsNumber !== NaN && aAsNumber === bAsNumber;
    }

    function isNonEmptyString(/*@dynamic*/v) {
        /// <summary>Check if the given argument is a non-empty string.</summary>
        /// <param name="v">Argument to check.</param>
        return typeof v === "string" && Boolean(v);
    };

    function isObject(/*@dynamic*/v) {
        /// <summary>Check if the given argument is a valid object.</summary>
        /// <param name="v">Argument to check.</param>
        return v !== undefined && v !== null && typeof v === "object";
    };

    function mixIn(/*@dynamic*/dest, /*@dynamic*/src) {
        /// <summary>Copy all properties from src to dest</summary>
        /// <param name="dest">Destination object</param>
        /// <param name="src">Source object</param>
        for (var i in src) {
            // Don't copy properties inherited from prototype. Object.prototype might be augmented.
            if (src.hasOwnProperty(i)) {
                dest[i] = src[i];
            }
        }
        return dest;
    };

    Debug.console = new DebugConsole();

})();
///#ENDDEBUG
