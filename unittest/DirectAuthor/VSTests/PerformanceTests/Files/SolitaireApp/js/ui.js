// Copyright (c) Microsoft Corporation
// All rights reserved

/// <reference path="../base/_es3.js" />
/// <reference path="../base/base.js" />

(function (Win, undefined) {
    Win.Namespace.defineWithParent(Win, "Controls", {
        Control: Win.Class.define(null, {
            _domElement: null,

            addEventListener: function (type, listener, useCapture) {
                /// <summary>
                /// Adds an event listener to the control.
                /// </summary>
                /// <param name='type'>
                /// The type (name) of the event.
                /// </param>
                /// <param name='listener'>
                /// The listener to invoke when the event gets raised.
                /// </param>
                /// <param name='useCapture'>
                /// Specifies whether or not to initiate capture.
                /// </param>
                if (this._domElement) {
                    this._domElement.addEventListener(type, listener, useCapture);
                }
            },

            raiseEvent: function (type, eventProperties) {
                /// <summary>
                /// Raises an event of the specified type and with additional properties.
                /// </summary>
                /// <param name='type'>
                /// The type (name) of the event.
                /// </param>
                /// <param name='eventProperties'>
                /// The set of additional properties to be attached to the event object when the event is raised.
                /// </param>
                if (this._domElement) {
                    var customEvent = document.createEvent("Event");
                    customEvent.initEvent(type, false, false);

                    if (eventProperties) {
                        var keys = Object.keys(eventProperties);
                        for (var i = 0; i < keys.length; i++) {
                            var name = keys[i];
                            var value = eventProperties[name];

                            customEvent[name] = value;
                        }
                    }
                    this._domElement.dispatchEvent(customEvent);
                }
            },

            removeEventListener: function (type, listener, useCapture) {
                /// <summary>
                /// Removes an event listener from the control.
                /// </summary>
                /// <param name='type'>
                /// The type (name) of the event.
                /// </param>
                /// <param name='listener'>
                /// The listener to remove from the invoke list.
                /// </param>
                /// <param name='useCapture'>
                /// Specifies whether or not to initiate capture.
                /// </param>
                if (this._domElement) {
                    this._domElement.removeEventListener(type, listener, useCapture);
                }
            },

            setOptions: function (options) {
                /// <summary>
                /// Applies the set of declaratively specified options (properties and events) on the specified control.
                /// </summary>
                /// <param name='control' domElement='false'>
                /// The control on which the properties and events are to be applied.
                /// </param>
                /// <param name='options' domElement='false'>
                /// The set of options that were specified declaratively.
                /// </param>
                if (options) {
                    var keys = Object.keys(options);
                    for (var i = 0; i < keys.length; i++) {
                        var name = keys[i];
                        var value = options[name];

                        // Look for an event
                        if (this._domElement &&
                            name.length > 2 && name.substr(0, 2).toUpperCase() == "ON" &&
                            typeof (value) === "function") {

                            this.addEventListener(name.substr(2), value);
                        }
                        else {
                            this[name] = value;
                        }
                    }
                }
            },
        })
    });
})(Win);
// Copyright (c) Microsoft Corporation
// All rights reserved

/// <reference path="../level1/es3.js" />
/// <reference path="../level1/base.js" />

(function (Win, undefined) {
    Win.Namespace.defineWithParent(Win, "Data", {
        bindOneWay: function (source, sourceProperty, destination, destinationProperty) {
            var destinationObservable = Win.Data.Observable.get(destination);
            destinationObservable.setValue(destinationProperty, source[sourceProperty]);

            if (source instanceof HTMLInputElement) {
                source.addEventListener("change", function (e) {
                    var newValue = source["value"];
                    destinationObservable.setValue(destinationProperty, newValue);
                }, false);
            }
            else {
                var sourceObservable = Win.Data.Observable.get(source);
                sourceObservable.addEventListener("propertychange", function (e) {
                    if (e.name === sourceProperty) {
                        destinationObservable.setValue(destinationProperty, e.newValue);
                    }
                });
            }
        },

        Observable: Win.Class.define(null, {
            _listeners: [],
            backingData: null,

            addEventListener: function (type, listener) {
                /// <param name='type'>Can only be propertychange.</param>
                if (typeof listener !== "function")
                    throw "Listener needs to be a function.";

                if (type === "propertychange") {
                    this._listeners.push(listener);
                } 
                else {
                    throw "Invalid event type";
                }
            },

            getkeys: function () {
                return Object.keys(this.backingData);  
            },

            _getObservable: function () {
                return this;
            },

            getValue: function (name) {
                if (!this.backingData)
                    return null;

                return Win.Data.Observable.get(this.backingData[name]);
            },

            _notifyListeners: function (name, oldValue, newValue) {
                this._listeners.forEach(function (listener) {
                    if (listener !== null) {
                        listener({ name: name, oldValue: oldValue, newValue: newValue });
                    }
                });
            },

            removeEventListener: function (type, listener) {
                if (typeof listener !== "function")
                    throw "Listener needs to be a function.";

                for (var i = 0, len = this._listeners.length; i < len; i++) {
                    if (listener === this._listeners[i]) {
                        this._listeners[i] = null;
                        break;
                    }
                }
            },

            setValue: function (name, value) {
                if (!this.backingData)
                    return;

                var oldValue = this.backingData[name];
                var newValue = Win.Data.Observable.unwrap(value);
                this.backingData[name] = newValue;
                
                this._notifyListeners(name, oldValue, newValue);
            },
        },
        function (data) {
            this.backingData = data;
        },
        {
            get: function (data) {
                if (!data)
                    return null;

                var type = typeof data;
                if (type === "object") {
                    if (data._getObservable)
                        return data._getObservable();

                    var observable = new Win.Data.Observable(data);
                    return observable;
                }
                else {
                    return data;
                }
            },

            unwrap: function (data) {
                if (!data)
                    return null;
                else if (data.backingData)
                    return data.backingData;
                else
                    return data;
            }
        }),
    });
})(Win);
// Copyright (c) Microsoft Corporation
// All rights reserved

/// <reference path="../base/_es3.js" />
/// <reference path="../base/base.js" />
/// <reference path="elementUtilities.js" />

var InvalidHandler = "Invalid data-ms-control attribute";

(function (Win, undefined) {
    Win.Namespace.defineWithParent(Win, "Controls", {
        _keyValueRegEx: /\s*([A-Za-z_\$][\w\$]*|\'[^\']*\'|\"[^\"]*\")\s*\:\s*((-\d*\.\d*|\+\d*\.\d*|\d*\.\d*)|(-\d+|\+\d+|\d+)|\'([^\']*)\'|\"([^\"]*)\"|([A-Za-z_\$][\w\.\$]*|\'[^\']*\'|\"[^\"]*\"))\s*,?/g,
        // -----------------|Identifier                              ||Colon | |Float Number                 | |Integer       | |Sngl & dbl qte string  | |Identifier                                 |Comma | 

        _evaluateSymbol: function (symbol) {
            return Win.Utilities.getMember(symbol);
        },

        _getControlHandler: function (element) {
            var evaluator = element.getAttribute("data-ms-control");
            if (evaluator) {
                var handler = Win.Utilities.getMember(evaluator);
                if (!handler) {
                    throw InvalidHandler;
                }
                return handler;
            }
        },

        _optionsFromElement: function (element) {
            var result = {};
            var optionsAttribute = element.getAttribute("data-ms-options");
            if (optionsAttribute) {
                try {
                    result = this._parseOptionsString(optionsAttribute);
                }
                catch (e) {
                    result = {};
                }
            }
            return result;
        },

        _parseOptionsString: function (optionsString) {
            var obj = {};
            while ((result = this._keyValueRegEx.exec(optionsString))) {
                var key = result[1];
                if (key.length) {
                    var firstChar = key[0];
                    if ((firstChar == '"' || firstChar == "'") && (key[key.length - 1] == firstChar)) {
                        key = key.substring(1, key.length - 1);
                    }
                }

                var value = undefined;
                if (result[3])
                    value = parseFloat(result[3]);
                else if (result[4])
                    value = parseInt(result[4]);
                else if (result[5])
                    value = result[5];
                else if (result[6])
                    value = result[6];
                else if (result[7]) {
                    var miscValue = result[7];
                    if (miscValue == "true")
                        value = true;
                    else if (miscValue == "false")
                        value = false;
                    else if (miscValue == "null")
                        value = null;
                    else 
                        value = this._evaluateSymbol(miscValue);
                }

                if (key == null)
                    break;

                obj[key] = value;
            }

            return obj;
        },

        delayedProcessAll: function (completed) {
            window.addEventListener("DOMContentLoaded", function (event) {
                Win.Controls.processAll(completed);
            }, false);
        },

        getControl: function (element) {
            /// <summary>
            /// Given a DOM element, retrieves the associated Control.
            /// </summary>
            /// <param name='element' domElement='true'>
            /// Element whose associated Control is requested.
            /// </param>
            /// <returns>
            /// The control associated with the dom element.
            /// </returns>
            return Win.Utilities.getData(element, "declControl");
        },

        processAll: function (rootElement, complete, dataContext) {
            /// <summary>
            /// Applies declarative control binding to all elements, starting optionally at rootElement.
            /// </summary>
            /// <param name='rootElement' domElement='true'>
            /// Element to start searching at, if not specified, the entire document is searched.
            /// </param>
            rootElement = rootElement || document.body;
            var pending = 0;
            var any = false;
            var controls = rootElement.querySelectorAll("[data-ms-control]");
            var checkComplete = function () {
                pending = pending - 1;
                if (complete && pending <= 0) {
                    complete();
                }
            };

            pending++;
            this.process(rootElement, checkComplete, dataContext);

            for (var i = 0; i < controls.length; i++) {
                var e = controls[i];
                if (!Win.Controls.getControl(e, "declControl")) {
                    if (!any) { pending++; }
                    any = true;
                    pending = pending + 1;
                    this.process(e, checkComplete, dataContext);
                }
            }
            if (any) {
                checkComplete();
            }
            if (complete && !any) {
                complete();
            }
        },

        process: function (element, complete, dataContext) {
            /// <summary>
            /// Applies declarative control binding to the specified element.
            /// </summary>
            /// <param name='element' domElement='true'>
            /// Element to bind.
            /// </param>
            var handler = this._getControlHandler(element);
            if (!handler)
                return;

            var that = this;
            var optionsGenerator = handler.optionsGenerator ||
                function (element) { return that._optionsFromElement(element); };

            var temp = window["dataContext"];
            window["dataContext"] = dataContext;
            var options = optionsGenerator(element);
            window["dataContext"] = temp;

            var ctl = handler(element, options, complete);
            if (complete && handler.length < 3) {
                complete();
            }

            return Win.Utilities.setData(element, "declControl", ctl);
        }
    });
})(Win);
// Copyright (c) Microsoft Corporation
// All rights reserved

/// <reference path="../base/_es3.js" />
/// <reference path="../base/base.js" />
/// <reference path="elementUtilities.js" />

(function (Win, undefined) {
    var listWrapper = Win.Class.define(null,
        {
            _list: null,
            // UNDONE: really we want to implement the Array contract... can we handle the indexer? [0]
            //
            length: {
                get: function () { return this._list.length; }
            },

            get: function (index) { return this._list[index]; },

            list: {
                get: function () { return this._list; }
            },

            // UNDONE: forEach/reduce/etc should be natively implemented on the result 
            // of querySelectorAll, we may be able to make this whole class be much much 
            // simpler
            //
            forEach: function (callback) {
                if (this._list.forEach) {
                    this._list.forEach(callback);
                }
                else {
                    for (var i = 0, l = this._list.length; i < l; i++) {
                        callback(this._list[i]);
                    }
                }
                return this;
            },

            reduce: function (callback, initial) {
                if (this._list.reduce) {
                    return this._list.reduce(callback);
                }
                else {
                    var last = initial;
                    for (var i = 0, l = this._list.length; i < l; i++) {
                        last = callback(last, this._list[i]);
                    }
                    return last;
                }
            },

            // UNDONE: there are much better ways to implement this... we think we need it
            //
            query: function (query) {
                return wrap(this.reduce(
                    function (r, e) {
                        var chunk = e.querySelectorAll(query);
                        for (var i = 0, l = chunk.length; i < l; i++) {
                            r.push(chunk[i]);
                        }
                        return r;
                    },
                    []
                ));
            },

            addClass: function (name) {
                return this.forEach(function (e) {
                    Win.Utilities.addClass(e, name);
                });
            },

            removeClass: function (name) {
                return this.forEach(function (e) {
                    Win.Utilities.removeClass(e, name);
                });
            },

            toggleClass: function (name) {
                return this.forEach(function (e) {
                    Win.Utilities.toggleClass(e, name);
                });
            },

            addEventListener: function (eventType, listener, capture) {
                return this.forEach(function (e) { e.addEventListener(eventType, listener, capture); });
            },

            removeEventListener: function (eventType, listener, capture) {
                return this.forEach(function (e) { e.removeEventListener(eventType, listener, capture); });
            },

            setStyle: function (name, value) {
                return this.forEach(function (e) {
                    e.style[name] = value;
                });
            },

            clearStyle: function (name) {
                return this.forEach(function (e) {
                    e.style[name] = "";
                });
            }
        }
    );
    var wrap = function (list) {
        var w = new listWrapper();
        w._list = list;
        return w;
    };

    Win.Namespace.defineWithParent(Win, "Utilities", {
        query: function utilities_query(query, element) {
            return wrap((element || document).querySelectorAll(query));
        }
    });
})(Win);// Copyright (c) Microsoft Corporation
// All rights reserved

/// <reference path="../base/_es3.js" />
/// <reference path="../base/base.js" />

(function (Win, undefined) {
    function getClassName(e) {
        var name = e.className;
        if (typeof(name) == "string") {
            return name || "";
        }
        else {
            return name.baseVal || "";
        }
    };
    function setClassName(e, value) {
        var name = e.className;
        if (typeof(name) == "string") {
            e.className = value;
        }
        else {
            e.className.baseVal = value;
        }
        return e;
    };
    Win.Namespace.defineWithParent(Win, "Utilities", {
        _dataKey: "_msDataKey",

        getData: function (element, key) {
            var data = element[Win.Utilities._dataKey] || {};
            return data[key];
        },

        setData: function (element, key, value) {
            var data = element[Win.Utilities._dataKey] || {};
            data[key] = value;
            element[Win.Utilities._dataKey] = data;
            return value;
        },

        hasClass: function (e, name) {
            var className = getClassName(e);
            var names = className.trim().split(" ");
            var l = names.length;
            for (var i = 0; i < l; i++) {
                if (names[i] == name) {
                    return true;
                }
            }
            return false;
        },

        addClass: function (e, name) {
            var className = getClassName(e);
            var names = className.trim().split(" ");
            var l = names.length;
            var found = false;
            for (var i = 0; i < l; i++) {
                if (names[i] == name) {
                    found = true;
                }
            }
            if (!found) {
                if (l > 0 && names[0].length > 0) {
                    setClassName(e, className + " " + name);
                }
                else {
                    setClassName(e, className + name);
                }
            }
        },

        removeClass: function (e, name) {
            var names = getClassName(e).trim().split(" ");
            setClassName(e, names.reduce(function (r, e) {
                if (e == name) {
                    return r;
                }
                else if (r && r.length > 0) {
                    return r + " " + e;
                }
                else {
                    return e;
                }
            }, ""));
        },

        toggleClass: function (e, name) {
            var className = getClassName(e);
            var names = className.trim().split(" ");
            var l = names.length;
            var found = false;
            for (var i = 0; i < l; i++) {
                if (names[i] == name) {
                    found = true;
                }
            }
            if (!found) {
                if (l > 0 && names[0].length > 0) {
                    setClassName(e, className + " " + name);
                }
                else {
                    setClassName(e, className + name);
                }
            }
            else {
                setClassName(e, names.reduce(function (r, e) {
                    if (e == name) {
                        return r;
                    }
                    else if (r && r.length > 0) {
                        return r + " " + e;
                    }
                    else {
                        return e;
                    }
                }, ""));
            }
        },

        getRelativeLeft: function (element, parent) {
            /// <summary>
            /// Gets the left coordinate of the element relative to the specified parent.
            /// </summary>
            /// <param name='element' domElement='true'>
            /// Element whose relative coordinate is needed.
            /// </param>
            /// <param name='parent' domElement='true'>
            /// Element to which the coordinate will be relative to.
            /// </param>
            /// <returns>
            /// Relative left co-ordinate.
            /// </returns>
            if (element === null)
                return 0;

            var left = element.offsetLeft;
            var e = element.parentNode;
            while (e !== null) {
                left -= e.offsetLeft;

                if (e === parent)
                    break;
                e = e.parentNode;
            }

            return left;
        },

        getRelativeTop: function (element, parent) {
            /// <summary>
            /// Gets the top coordinate of the element relative to the specified parent.
            /// </summary>
            /// <param name='element' domElement='true'>
            /// Element whose relative coordinate is needed.
            /// </param>
            /// <param name='parent' domElement='true'>
            /// Element to which the coordinate will be relative to.
            /// </param>
            /// <returns>
            /// Relative top co-ordinate.
            /// </returns>
            if (element === null)
                return 0;

            var top = element.offsetTop;
            var e = element.parentNode;
            while (e !== null) {
                top -= e.offsetTop;

                if (e === parent)
                    break;
                e = e.parentNode;
            }

            return top;
        },

        removeAllChildren: function (element) {
            /// <summary>
            /// Removes all the child nodes from the specified element.
            /// </summary>
            /// <param name='element' domElement='true'>
            /// The element whose child nodes will be removed.
            /// </param>
            for (var i = element.childNodes.length - 1; i >= 0; i--) {
                element.removeChild(element.childNodes.item(i));
            }
        },

        trackDragMove: function (element, mouseDown, mouseMove, mouseUp) {
            /// <summary>
            /// Signs the element for drag events and tracks the drag operation.
            /// </summary>
            /// <param name='element' domElement='true'>
            /// The element to track.
            /// </param>
            /// <param name='mouseDown'>
            /// The listener to call back when the mouseDown event arrives.
            /// </param>
            /// <param name='mouseMove'>
            /// The listener to call back when the mouseMove event arrives.
            /// </param>
            /// <param name='mouseUp'>
            /// The listener to call back when the mouseUp event arrives.
            /// </param>
            element.onmousedown = function (e) {
                if (mouseDown !== undefined)
                    mouseDown(e);

                var moveHandler = function (e) {
                    if (mouseMove !== undefined)
                        mouseMove(e);

                    e.cancelBubble = true;
                    e.stopPropagation();
                };

                var upHandler = function (e) {
                    if (mouseDown !== undefined)
                        mouseUp(e);

                    if (element.releaseCapture) {
                        element.onmousemove = null;
                        element.onmouseup = null;
                        element.releaseCapture();
                    }
                    else {
                        window.removeEventListener("mousemove", moveHandler, true);
                        window.removeEventListener("mouseup", upHandler, true);
                    }
                };

                if (element.setCapture) {
                    element.setCapture();
                    element.onmousemove = moveHandler;
                    element.onmouseup = upHandler;
                }
                else {
                    window.addEventListener("mousemove", moveHandler, true);
                    window.addEventListener("mouseup", upHandler, true);
                }

                e.cancelBubble = true;
                e.stopPropagation();
            };
        },
    });
})(Win);
// Copyright (c) Microsoft Corporation
// All rights reserved

/// <reference path="../base/_es3.js" />
/// <reference path="../base/base.js" />

(function (Win, globalObj, undefined) {
    var loaderStateProp = "-ms-fragmentLoader-state";

    // UNDONE: should we hoist this to a shared location?
    //
    var forEach = function (arrayLikeValue, action) {
        for (var i = 0, l = arrayLikeValue.length; i < l; i++) {
            action(arrayLikeValue[i]);
        }
    };
    var head = document.head || document.getElementsByTagName("head")[0];

    Win.Namespace.defineWithParent(Win, "Controls.FragmentLoader", {
        _scripts: {},
        _styles: {},
        _links: {},
        _states: {},
        _initialized: { value: false, writable: true },

        _idFromHref: function (href) {
            if (typeof (href) == "string") {
                return href;
            }
            else {
                return href.id;
            }
        },

        _addScript: function (scriptTag, fragmentHref, position) {
            /// <summary>
            /// PRIVATE METHOD: Adds a script tag based on the data from the fragment to the host document.
            /// </summary>

            // We synthesize a name for inline scripts because today we put the 
            // inline scripts in the same processing pipeline as src scripts. If
            // we seperated inline scripts into their own logic, we could simplify
            // this somewhat.
            //
            var src = scriptTag.src;
            if (!src) {
                src = fragmentHref + "script[" + position + "]";
            }

            if (!(src in Win.Controls.FragmentLoader._scripts)) {
                Win.Controls.FragmentLoader._scripts[src] = true;
                var n = document.createElement("script");
                if (scriptTag.language) {
                    n.setAttribute("language", "javascript");
                }
                if (scriptTag.type == "ms-documentReference") {
                    n.setAttribute("type", "ms-documentReference");
                }
                else if (scriptTag.type == "ms-deferred/javascript") {
                    n.setAttribute("type", "text/javascript");
                }
                else {
                    n.setAttribute("type", scriptTag.type);
                }
                if (scriptTag.id) {
                    n.setAttribute("id", scriptTag.id);
                }
                if (scriptTag.src) {
                    n.setAttribute("src", scriptTag.src);
                }
                else {
                    n.text = scriptTag.text;
                }
                head.appendChild(n);
            }
        },

        _addStyle: function (styleTag, fragmentHref, position) {
            /// <summary>
            /// PRIVATE METHOD: Adds a CSS link tag based on the data from the fragment to the host document.
            /// </summary>

            var src = fragmentHref + "script[" + position + "]";
            if (!(styleTag.href in Win.Controls.FragmentLoader._styles)) {
                Win.Controls.FragmentLoader._styles[src] = true;
                var n = document.createElement("style");
                n.setAttribute("type", "text/css");
                n.innerText = styleTag.innerText;
                head.appendChild(n);
            }
        },

        _addLink: function (styleTag) {
            /// <summary>
            /// PRIVATE METHOD: Adds a CSS link tag based on the data from the fragment to the host document.
            /// </summary>

            if (!(styleTag.href in Win.Controls.FragmentLoader._links)) {
                Win.Controls.FragmentLoader._links[styleTag.href] = true;
                var n = document.createElement("link");
                n.setAttribute("type", "text/css");
                n.setAttribute("rel", styleTag.rel);
                n.setAttribute("href", styleTag.href);
                head.appendChild(n);
            }
        },

        _controlStaticState: function (href, success) {
            /// <summary>
            /// PRIVATE METHOD: retrieves the static (not per-instance) state for a fragment at the
            /// URL "href". "success" will be called either synchronously (for an already loaded
            /// fragment) or asynchronously when the fragment is loaded and ready to be used.
            /// </summary>
            var fragmentId = Win.Controls.FragmentLoader._idFromHref(href);

            var state = Win.Controls.FragmentLoader._states[fragmentId];

            var intervalId;
            var callback = function () {
                if (state.templateElement) {
                    if (state.loadScript) {
                        var load = globalObj[state.loadScript];
                        if (load) {
                            success(load, Win.Controls.FragmentLoader._states[fragmentId]);
                            if (intervalId) { clearInterval(intervalId); }
                            return true;
                        }
                    }
                    else {
                        success(undefined, Win.Controls.FragmentLoader._states[fragmentId]);
                        if (intervalId) { clearInterval(intervalId); }
                        return true;
                    }
                }
                return false;
            }


            // If the state record was found, then we either are ready to 
            // roll immediately (everything is loaded & parsed) or are in
            // process of loading. If possible, we want to directly invoke
            // to avoid any flickering, however if we are still loading
            // the content, we must wait.
            //
            if (state) {
                if (!callback()) {
                    intervalId = setInterval(callback, 20);
                }

                return;
            }
            else {
                Win.Controls.FragmentLoader._states[fragmentId] = state = {};
            }

            if (typeof (href) === "string") {
                var temp = document.createElement('iframe');
                document[loaderStateProp] = "loading";
                temp.src = href;
                temp.style.display = 'none';
                
                var domContentLoaded = null;

                var complete = function (load) {
                    // This is to work around a weird bug where removing the 
                    // IFrame from the DOM triggers DOMContentLoaded a second time.
                    temp.contentDocument.removeEventListener("DOMContentLoaded", domContentLoaded, false);
                    temp.parentNode.removeChild(temp);
                    delete temp;
                    delete document[loaderStateProp];
                    success(load, state);
                };
                
                domContentLoaded = function() {
                    Win.Controls.FragmentLoader._controlStaticStateLoaded(href, temp, state, complete);
                }
                

                document.body.appendChild(temp);
                temp.contentDocument.addEventListener("DOMContentLoaded", domContentLoaded, false);                
            }
            else {
                state.loadScript = href.getAttribute('data-ms-fragmentLoad') || state.loadScript;
                state.templateElement = href;
                if (!callback()) {
                    intervalId = setInterval(callback, 20);
                }
            }
        },

        _controlStaticStateLoaded: function (href, temp, state, complete) {
            /// <summary>
            /// PRIVATE METHOD: Once the control's static state has been loaded in the temporary iframe,
            /// this method spelunks the iframe's document to retrieve all relevant information. Also,
            /// this performs any needed fixups on the DOM (like adjusting relative URLs).
            /// </summary>

            var cd = temp.contentDocument;

            var links = cd.querySelectorAll('head > link[type="text/css"]');
            state.styles = links;
            forEach(links, function (e) {
                Win.Controls.FragmentLoader._addLink(e);
            });

            // NOTE: no need to cache the style objects, as they are unique per fragment
            //
            forEach(cd.querySelectorAll('head > style'), function (e) {
                Win.Controls.FragmentLoader._addStyle(e);
            });

            var scripts = cd.getElementsByTagName('script');
            state.scripts = scripts;

            var scriptPosition = 0;
            forEach(scripts, function (e) {
                Win.Controls.FragmentLoader._addScript(e, href, scriptPosition);
                scriptPosition++;

                state.loadScript = e.getAttribute('data-ms-fragmentLoad') || state.loadScript;
            });

            state.loadScript = cd.body.getAttribute('data-ms-fragmentLoad') || state.loadScript;

            // UNDONE: figure out all the elements we should do URI fixups for
            //
            forEach(cd.body.getElementsByTagName('img'), function (e) {
                e.src = e.href;
            });
            forEach(cd.body.getElementsByTagName('a'), function (e) {
                // UNDONE: for # only anchor tags, we don't update the href... good design?
                //
                if (e.href !== "") {
                    var href = e.getAttribute("href").value;
                    if (href && href[0] != "#") {
                        e.href = e.href;
                    }
                }
            });

            // strip inline scripts from the body, they got copied to the 
            // host document with the rest of the scripts above... 
            //
            var scripts = cd.body.getElementsByTagName("script");
            while (scripts.length > 0) {
                scripts[0].parentNode.removeChild(scripts[0]);
            }

            // UNDONE: capture a documentfragment with the list of body.children
            //
            state.templateElement = document.importNode(temp.contentDocument.body, true);

            // huge ugly kludge
            if (state.loadScript) {
                var intervalId = setInterval(function () {
                    var load = globalObj[state.loadScript];
                    if (load) {
                        complete(load);
                        clearInterval(intervalId);
                    }
                }, 20);
            }
            else {
                complete();
            }
        },

        _initialize: function () {
            /// <summary>
            /// PRIVATE METHOD: Initializes the fragment loader with the list of scripts and 
            /// styles already present in the host document
            /// </summary>
            if (Win.Controls.FragmentLoader._initialized) { return; }

            Win.Controls.FragmentLoader._initialized = true;

            var scripts = head.querySelectorAll("script");
            for (var i = 0, l = scripts.length; i < l; i++) {
                Win.Controls.FragmentLoader._scripts[scripts[i].src] = true;
            }

            var csss = head.querySelectorAll('link[type="text/css"]');
            for (var i = 0, l = csss.length; i < l; i++) {
                Win.Controls.FragmentLoader._links[csss[i]] = true;
            }
        },

        addFragment: function (element, href, options, complete) {
            /// <summary>
            /// Adds the content of the fragment specified by "href" to the children of "element".
            /// The "options" record is pased (optionaly) to the load handler for the fragment.
            /// If supplied "complete" will be called when the  fragment has been loaded and the 
            /// load handler is complete.
            /// </summary>
            Win.Controls.FragmentLoader._initialize();

            Win.Controls.FragmentLoader._controlStaticState(href, function (load, state) {
                istate = Object.create(state, { element: { value: element} });

                var adopted = istate.templateElement.cloneNode(true);
                var c = adopted.children;
                var generatedElements = [];
                while (c.length > 0) {
                    generatedElements.push(c[0]);
                    element.appendChild(c[0]);
                }
                if (load) {
                    load(generatedElements, options);
                }
                if (complete) {
                    complete();
                }
            });
        },

        createFragment: function (href, options, complete) {
            /// <summary>
            /// Returns the content of the fragment specified by "href" to the children of "element".
            /// The "options" record is pased (optionaly) to the load handler for the fragment.
            /// If supplied "complete" will be called when the  fragment has been loaded and the 
            /// load handler is complete.
            ///
            /// The will be placed in a wrapper "div" element.
            /// </summary>
            var container = document.createElement("div");
            Win.Controls.FragmentLoader.addFragment(container, href, options, complete);
            return container;
        },

        createRenderer: function (href, complete) {
            /// <summary>
            /// Returns a function that matches the signature of an itemRenderer, which is implemented
            /// by calling createFragment.
            /// </summary>
            var renderer = function (renderInfo, key, dataObject, itemID) {
                return Win.Controls.FragmentLoader.createFragment(
                    href,
                    { renderInfo: renderInfo, key: key, dataObject: dataObject, itemID: itemID }
                );
            };

            Win.Controls.FragmentLoader.prepareFragment(href, function () {
                if (complete) {
                    complete(renderer);
                }
            });

            return renderer;
        },

        prepareFragment: function (href, complete) {
            /// <summary>
            /// Starts loading the fragment at the specified location, success will be 
            /// called when the fragment is ready to be used
            /// </summary>
            Win.Controls.FragmentLoader._initialize();

            var callback = function () {
                if (complete) {
                    complete();
                }
            };
            Win.Controls.FragmentLoader._controlStaticState(href, callback);
        },

        unprepareFragment: function (href) {
            /// <summary>
            /// Removes any cached information about the fragment, this will not unload scripts 
            /// or styles referenced by the fragment.
            /// </summary>

            delete this._states[this._idFromHref(href)];
        },

        selfhost: function (load) {
            /// <summary>
            /// This is used in the fragment definition markup to allow a fragment to 
            /// be loaded as a stand alone page.
            /// </summary>
            if (globalObj.parent) {
                if (globalObj.parent.document[loaderStateProp] != "loading") {
                    forEach(globalObj.document.querySelectorAll('head > script[type="ms-deferred/javascript"]'),
                        function (e) {
                            Win.Controls.FragmentLoader._addScript(e);
                        });

                    globalObj.addEventListener("DOMContentLoaded", function (event) {
                        load(globalObj.document.body.children);
                    }, false);
                }
            }
        }
    });
})(Win, this);
// Copyright (c) Microsoft Corporation
// All rights reserved

/// <reference path="../base/_es3.js" />
/// <reference path="../base/base.js" />
/// <reference path="elementUtilities.js" />
/// <reference path="control.js" />

(function (Win, undefined) {
    Win.Namespace.defineWithParent(Win, "Controls", {
        _templateProcessingDataKey: "templateProcessingData",

        ItemTemplate: Win.Class.define(Win.Controls.Control, {
            _templateNode: null,
            _templateData: null,
            _regExp: /\{(\w+)\}/g,
            _regExpTest: /\{(\w+)\}/,

            _applyData: function (templateData, element, data) {
                if (!templateData) {
                    return;
                }

                if (templateData.processTextData) {
                    element.data = this._transformText(element.data, data);
                } else {
                    for (var i = 0; i < templateData.attributes.length; i++) {
                        var attribute = element.attributes[templateData.attributes[i]];
                        attribute.value = this._transformText(attribute.value, data);
                    }

                    var keys = Object.keys(templateData.children);
                    for (var i = 0; i < keys.length; i++) {
                        var childIndex = keys[i];
                        var childTemplateData = templateData.children[childIndex];
                        this._applyData(childTemplateData, element.childNodes[childIndex], data);
                    }
                }
            },

            _constructTemplateData: function (element) {
                var processingData = Win.Utilities.getData(element, Win.Controls._templateProcessingDataKey);
                if (processingData !== undefined) {
                    return processingData;
                }

                // We pre-process the element to detect and mark interesting tree-traversals.
                // These paths are stored as indices into attributes and child nodes.  This 
                // makes applyData fast, but also makes it fragile if people modify the visual
                // tree of the template using, say, innerHTML property.
                var requiresProcessing = false;
                processingData = { processTextData: false, children: {}, attributes: [] };

                if (element instanceof Text && this._regExpTest.test(element.data)) {
                    processingData.processTextData = true;
                    requiresProcessing = true;
                }
                else if (element instanceof HTMLElement) {
                    for (var i = 0, len = element.attributes.length; i < len; i++) {
                        var attribute = element.attributes.item(i);
                        if (this._regExpTest.test(attribute.value)) {
                            processingData.attributes.push(i);
                            requiresProcessing = true;
                        }
                    }

                    for (var i = 0, len = element.childNodes.length; i < len; i++) {
                        var childData = this._constructTemplateData(element.childNodes.item(i));
                        if (childData) {
                            requiresProcessing = true;
                            processingData.children[i] = childData;
                        }
                    }
                }

                if (requiresProcessing) {
                    Win.Utilities.setData(element, Win.Controls._templateProcessingDataKey, processingData);
                    return processingData;
                } else {
                    Win.Utilities.setData(element, Win.Controls._templateProcessingDataKey, null);
                    return null;
                }
            },

            createElement: function (data) {
                var newElement = this._templateNode.cloneNode(true);
                this._applyData(this._templateData, newElement, data);

                Win.Controls.processAll(newElement, function () { }, data);
                newElement.style.display = "";
                return newElement;
            },

            setElement: function (element) {
                this._templateNode = element.cloneNode(true);

                this._templateNode.setAttribute("id", "");
                this._templateData = this._constructTemplateData(element);
                element.style.display = "none";

                // Attach a renderItem function that will be used by Items Control.
                var that = this;
                element.renderItem = function (getIndex, key, data, itemId) {
                    return that.createElement(data);
                };
            },

            _transformText: function (text, data) {
                var keys;
                var newText = text;

                while ((keys = this._regExp.exec(text))) {
                    var keyName = keys[1];
                    newText = newText.replace(keys[0], data[keyName]);
                }

                return newText;
            }
        },
        function (element, options) {
            // <summary>Constructs the ItemTemplate control.</summary>
            if (this === window) {
                return new Win.Controls.ItemTemplate(element, options);
            }

            this.setElement(element);
            this.setOptions(options);
        },
        {
            applyTemplate: function (templateElement, data) {
                var template = Win.Controls.getControl(templateElement);
                if (!template) {
                    template = Win.Controls.process(templateElement);
                }
                if (!template) {
                    template = new Win.Controls.ItemTemplate(templateElement);
                }

                return template.createElement(data);
            },

            applyFragmentTemplate: function (fragmentUrl, data) {
                
            }
        })
    })
})(Win);