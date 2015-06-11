/// <reference path='base.js' />
/// <reference path='wwaapp.js' />
/// <reference winrt='true'  />
/// <reference winrt='true'  />
 

////////////////////////////////////////////////////////////
////   © Microsoft. All rights reserved.                ////
////                                                    ////
////   This library is intended for use in WWAs only.   ////
////////////////////////////////////////////////////////////
// x86fre.fbl_pac_dev 
 



(function (global, Win, undefined) {
    var pendingDrain;

    function dispatchEvent(eventRecord) {
        var promise = Win.Promise.as();
        eventRecord.setPromise = function (p) { promise = p; };
        eventRecord.detail = eventRecord.detail || {};
        if (typeof(eventRecord.detail) === "object") {
            eventRecord.detail.setPromise = eventRecord.setPromise;
        }

        try {
            var l = builtInListeners[eventRecord.type];
            if (l) {
                l.forEach(function dispatchOne(e) { e(eventRecord); });
            }

            l = listeners._listeners[eventRecord.type];
            if (l) {
                l.forEach(function dispatchOne(e) { e.listener(eventRecord); });
            }
        }
        catch (err) {
            queueEvent({type:"error", detail:err});
        }
        
        return promise;
    }

    function drainQueue(queue) {
        pendingDrain = true;
        if (queue.length === 0) {
            if (eventQueue.length > 0) {
                return drainQueue(copyAndClearQueue());
            }
            pendingDrain = false;
            return Win.Promise.as(queue);
        }
        function drainNext() {
            return drainQueue(queue.slice(1));
        }
        function drainError(err) {
            queueEvent({type:"error", detail:err});
            return drainNext();
        }
        return dispatchEvent(queue[0]).
            then(drainNext, drainError);
    }

    function queueEvent(eventRecord) {
        eventQueue.push(eventRecord);
        if (running && !pendingDrain) {
            drainQueue(copyAndClearQueue());
        }
    }

    function copyAndClearQueue() {
        var queue = eventQueue;
        eventQueue = [];
        return queue;
    }
    
    var ListenerType = Win.Class.mix(Win.Class.define(null), Win.Utilities.eventMixin);
    var listeners = new ListenerType();
    var sawActivated = false;
    var sawLoaded = false;
    var queuedReady = false;
    function checkReady() {
        if (sawActivated && sawLoaded) {
            queuedReady = true;
            queueEvent({ type: "ready" });
        }
    }
    var builtInListeners = { 
        mainwindowactivated: [
            function () { 
                sawActivated = true; 
                checkReady(); 
            }
        ],
        loaded: [
            function () { 
                sawLoaded = true; 
                checkReady(); 
            }
        ],
        checkpoint: [
            function(e) {
                // comes from state.js
                Win.Application._oncheckpoint(e);
            }
        ]
    };
    // UNDONE: wire these up to real WinRT events
    //
    var eventQueue = [ ];
    var running = false;
    
    // ideally... loaded == DOMContentLoaded
    //            mainwindowactivated == after WinRT Activated
    //            ready == after all of the above
    //

    var useWinRT = false;
    if (window.Windows && Windows.UI) {
        useWinRT = true;
        var wui = Windows.UI.WebUI.WebUIApplication;
        wui.addEventListener("activated", function (e) {
            Win.Application._loadState(e).then(function () {
                // UNDONE: this is temporary until we get through stabalization
                //
                if (Win.validation) {
                    if (!e.contractId) {
                        throw "contractId not found, there must have been a breaking change that we haven't accounted for yet!";
                    }
                }

                queueEvent({ 
                    type: "mainwindowactivated", 
                    detail: e
                        /* UNDONE: this is here for debugging
                        { 
                            activationContext: e.activationContext,
                            contractId: e.contractId, 
                            handled: e.handled,
                            reason: e.reason,
                            sourceId: e.sourceId,
                            splashScreenInfo: e.splashScreenInfo,
                            _original: e 
                        } 
                        */
                });
            });
        });
    }
    
    document.addEventListener("DOMContentLoaded", function (e) {
        // UNDONE: do we really want to expose an app.loaded
        // event... previously we had said people should just
        // listen to DOMContentLoaded directly
        //
        queueEvent({ type: "loaded" });
        if (!useWinRT) {
            var activatedArgs = { 
                activationContext: "",
                contractId: "Microsoft.Windows.Tile", 
                handled: true,
                reason: 0
                // UNDONE: sourceId: e.sourceId,
                // UNDONE: splashScreenInfo: e.splashScreenInfo,
            };
            Win.Application._loadState(activatedArgs).then(function () {
                queueEvent({ type: "mainwindowactivated", detail:activatedArgs});
            });
        }
    }, false);

    // UNDONE: these  will be replaced with PLM notifications
    //
    window.addEventListener("beforeunload", function (e) {
        queueEvent({type:"checkpoint"});
        queueEvent({type:"unload"});
    }, false);
    var checkPointTimer;
    function ensureCheckPoint() {
        if (!checkPointTimer) {
            checkPointTimer = setInterval(
                function() {
                    queueEvent({type:"checkpoint"});
                }, 10000);
        }
    }
    function clearCheckPoint() {
        if (checkPointTimer) {
            clearInterval(checkPointTimer);
        }
    }
    
    Win.Namespace.defineWithParent(Win, "Application", {
        stop: function() {
            listeners = new ListenerType();
            running = false;
            sawActivated = false;
            sawLoaded = false;
            queuedReady = false;
            clearCheckPoint();
            copyAndClearQueue();
        },
        addEventListener: function (eventType, listener, capture) {
            listeners.addEventListener(eventType, listener, capture);
        },
        removeEventListener: function (eventType, listener, capture) {
            listeners.removeEventListener(eventType, listener, capture);
        },
        checkpoint: function() {
            queueEvent({type:"checkpoint"});
        },
        
        start: function () {
            var queue = copyAndClearQueue();
            running = true;
            ensureCheckPoint();
            drainQueue(queue);
        },

        queueEvent : queueEvent
    });
    
    Object.defineProperties(Win.Application, Win.Utilities.createEventProperties("checkpoint", "unload", "mainwindowactivated", "loaded", "ready"));
})(this, Win);



(function (Win, undefined) {
    var navigatedEventName = "navigated";
    var navigatingEventName = "navigating";
    var beforenavigateEventName = "beforenavigate";
    var ListenerType = Win.Class.mix(Win.Class.define(null), Win.Utilities.eventMixin);
    var listeners = new ListenerType();
    var history = {
        backStack: [],
        current: {location:"", initialPlaceholder: true},
        forwardStack: []
    };

    var raiseBeforeNavigate = function (proposed) {
        return Win.Promise.as().
            then(function () {
                var promise = Win.Promise.as();
                var defaultPrevented = listeners.dispatchEvent(beforenavigateEventName, { 
                    setPromise: function(p) {  promise = p; },
                    location: proposed.location,
                    state: proposed.state
                });
                return promise.then(function beforeNavComplete() {
                    return defaultPrevented;
                });
            });
    };
    var raiseNavigating = function (delta) {
        return Win.Promise.as().
            then(function () {
                var promise = Win.Promise.as();
                listeners.dispatchEvent(navigatingEventName, { 
                    setPromise: function(p) {  promise = p; },
                    location: history.current.location,
                    state: history.current.state,
                    delta: delta
                });
                return promise;
            });
    };
    var raiseNavigated = function(value, err) {
        var promise = Win.Promise.as();
        var detail = { 
            value: value,
            location: history.current.location,
            state: history.current.state,
            setPromise: function(p) {  promise = p; }
        };
        if (!value && err) {
            detail.error = err;
        }
        listeners.dispatchEvent(navigatedEventName, detail);
        return promise;
    };

    var go = function (distance, fromStack, toStack, delta) {
        distance = Math.min(distance, fromStack.length);
        if (distance > 0) { 
            return raiseBeforeNavigate(fromStack[fromStack.length-distance]).
                then(function goBeforeCompleted(cancel) {
                    if (!cancel) {
                        toStack.push(history.current);
                        while (distance-1 != 0) {
                            distance--;
                            toStack.push(fromStack.pop());
                        }
                        history.current = fromStack.pop();
                        return raiseNavigating(delta).then(
                            raiseNavigated, 
                            function (err) { 
                                raiseNavigated(undefined, err || true); 
                                throw err;
                            }).then(function() { return true; });
                    }
                    else {
                        return false;
                    }
                });
        }
        return Win.Promise.wrap(false);
    }

    Win.Namespace.defineWithParent(Win, "Navigation", {
        canGoForward: {
            get: function () {
                return history.forwardStack.length > 0;
            }
        },
        canGoBack: {
            get: function () {
                return history.backStack.length > 0;
            }
        },
        location: {
            get: function () {
                return history.current.location;
            }
        },
        state: {
            get: function () {
                return history.current.state;
            },
            set: function (value) {
                history.current.state = value;
            }
        },
        history: {
            get: function() {
                return history;
            },
            set: function(value) {
                var s = history = value;

                // ensure the require fields are present
                //
                s.backStack = s.backStack || [];
                s.forwardStack = s.forwardStack || [];
                s.current = s.current || {location:"", initialPlaceholder:true};
                s.current.location = s.current.location || "";
            }
        },
        forward: function(distance) {
            distance = distance || 1;
            return go(distance, history.forwardStack, history.backStack, distance);
        },
        back: function(distance) {
            distance = distance || 1;
            return go(distance, history.backStack, history.forwardStack, -distance);
        },
        navigate: function (location, initialState) {
            var proposed = { location:location, state: initialState };
            return raiseBeforeNavigate(proposed).
                then(function navBeforeCompleted(cancel) {
                    if (!cancel) {
                        if (!history.current.initialPlaceholder) {
                            history.backStack.push(history.current);
                        }
                        history.forwardStack = [];
                        history.current = proposed;
                
                        // error or no, we go from navigating -> navigated
                        // cancelation should be handled with "beforenavigate"
                        //
                        return raiseNavigating().then(
                            raiseNavigated, 
                            function (err) { 
                                raiseNavigated(undefined, err || true);
                                throw err; 
                            }).then(function () { return true; });
                    }
                    else {
                        return false;
                    }
                });
        },
        addEventListener: function (eventType, listener, capture) {
            listeners.addEventListener(eventType, listener, capture);
        },
        removeEventListener: function (eventType, listener, capture) {
            listeners.removeEventListener(eventType, listener, capture);
        }
    });
    
    Object.defineProperties(Win.Navigation, Win.Utilities.createEventProperties(navigatedEventName, navigatingEventName, beforenavigateEventName));
})(Win);



(function() {
    function initWithWinRT() {
        function share(content, description, title) {
            var sm = new Windows.UI.SharingManager();
            var p = new Windows.UI.DataPackage();
            var pvf = new Windows.Foundation.PropertyValueFactory();
            description = description || "Text";
            title = title || Win.Application.name;
            p.Properties.Insert("Title", pvf.CreateString(title));
            p.Properties.Insert("Description", pvf.CreateString(description));
            if (typeof(content) === "string") {
                p.Text = content;
            }
            else {
                if (content.text) {
                    p.Text = content.text;
                }
            }
            sm.Share(p);
        }        
        
        Win.Namespace.define("Win.Application", {
            // UNDONE: this should come from the manifest
            //
            name: {value:"DefaultAppName", writable:true, enumerable:true},
            share: share
        });

        Win.Application.addEventListener("loaded", function (e) {
            var share = new Windows.UI.SharingManager();
            share.SharingDataRequest = function (sender, e) {
                Win.Application._notifyShare();
            };
        });    
    };
    
    function initWithStub() {

        Win.Namespace.define("Win.Application", {
            // UNDONE: this should come from the manifest
            //
            name: {value:"DefaultAppName", writable:true, enumerable:true},
            share: function (content, description, title) {
                // no meaningful action
            }
        });
    }

    Win.Namespace.define("Win.Application", {
        _notifyShare: function() {
            var target = document.activeElement || document.body || window;
            var e = document.createEvent("CustomEvent");
            e.initCustomEvent("share", true, false, {});
            target.dispatchEvent(e);
        }
    });
    
    if (window.Windows && Windows.UI) {
        initWithWinRT();
    }
    else {
        initWithStub();
    }
})();



(function() {
    function initWithWinRT() {
        var sto = Windows.Storage;

        var IOHelper = Win.Class.define(
            function (container) {
                this.container = container;
                this._path = container.getProperties().path;
            }, {
                exists: function (fileName) {
                    /// <summary locid="86">Determines if the specified file exists in the container</summary>
                    /// <returns locid="87">
                    /// Promise with either true (file exists) or false.
                    /// </returns>
                    return sto.FileItem.getFileItemFromPathAsync(this._path + "\\" + fileName).
                        then(
                            function() { return true; }, 
                            function() { return false; }
                        );
                },
                remove: function (fileName) {
                    var that = this;
                    return that.exists(fileName).
                        then(function (exists) {
                            if (exists) {
                                return sto.FileItem.getFileItemFromPathAsync(that._path + "\\" + fileName).
                                    then(function (fileItem) {
                                        return fileItem.deleteAsync();
                                    });
                            }
                        });
                },
                writeText: function (fileName, str) {
                    /// <summary locid="88">Writes a file to the container with the specified text</summary>
                    /// <returns locid="89">
                    /// Promise with the count of characters written
                    /// </returns>
                    var that = this;
                    return that.exists(fileName).
                        then(function (exists) {
                            if (exists) {
                                return sto.FileItem.getFileItemFromPathAsync(that._path + "\\" + fileName);
                            }
                            else {
                                return that.container.createFileAsync(fileName, sto.FileCollisionOptions.automaticRename)
                            }
                        }).then(function (fileItem) {
                            return fileItem.getStreamAsync(sto.FileAccessMode.readWrite);
                        }).then(function(byteSeeker) {
                            var writer = byteSeeker.getOutputStreamAt(0);
                            var bbrw = new sto.BasicBinaryReaderWriter();
                            return Win.Promise.join({
                                count:bbrw.writeBinaryStringAsync(writer, str), 
                                writer:writer
                            });
                        }).then(function(countAndWriter) {
                            return countAndWriter.writer.flushAsync().
                                then(function() { return countAndWriter.count; });
                        });
                },
                readText: function (fileName, def) {
                    /// <summary locid="90">
                    /// Reads the contents of a file from the container, if the file
                    /// doesn't exist, def is returned.
                    /// </summary>
                    /// <returns locid="91">
                    /// Promise containing the contents of the file, or def.
                    /// </returns>
                    var that = this;
                    return that.exists(fileName).
                        then(function (exists) {
                            if (exists) {
                                return sto.FileItem.getFileItemFromPathAsync(that._path + "\\" + fileName).
                                    then(function (fileItem) {
                                        return fileItem.getStreamAsync(sto.FileAccessMode.read);
                                    }).then(function (byteSeeker) {
                                        var reader = byteSeeker.getInputStreamAt(0);
                                        var bbrw = new sto.BasicBinaryReaderWriter();
                                        return bbrw.readBinaryStringAsync(reader, byteSeeker.size);
                                    });
                            }
                            else {
                                return def;
                            }
                        });
                }
            }
        );

        Win.Namespace.define("Win.Application", {
            local: new IOHelper(sto.ApplicationData.current.localFolder),
            temp: new IOHelper(sto.ApplicationData.current.temporaryFolder),
            roaming: new IOHelper(sto.ApplicationData.current.roamingFolder)
        });
    };
    
    function initWithStub() {
        var InMemoryHelper = Win.Class.define(
            function () {
                this.storage = {};
            }, {
                exists: function (fileName) {
                    // force conversion to boolean
                    // 
                    return Win.Promise.as(this.storage[fileName] !== undefined);
                },
                remove: function (fileName) {
                    delete this.storage[fileName];
                    return Win.Promise.as();
                },
                writeText: function (fileName, str) {
                    this.storage[fileName] = str;
                    return Win.Promise.as(str.length);
                },
                readText: function (fileName, def) {
                    return Win.Promise.as(this.storage[fileName] || def);
                }
            }
        );

        Win.Namespace.define("Win.Application", {
            local: new InMemoryHelper(),
            temp: new InMemoryHelper(),
            roaming: new InMemoryHelper()
        });
    }
    
    if (window.Windows && Windows.Storage && Windows.Storage.ApplicationData) {
        initWithWinRT();
    }
    else {
        initWithStub();
    }
    

    Win.Namespace.define("Win.Application", {
        sessionState: {value: {}, writable:true, enumerable:true},
        _loadState: function(e) {
            var app = Win.Application;

            return app.local.readText("_sessionState.json", "{}").
                then(function (str) { 
                    app.sessionState = JSON.parse(str);
                });
        },
        _oncheckpoint: function(e) {
            var app = Win.Application;
                
            e.setPromise(Win.Promise.join([
                app.local.writeText("_sessionState.json", JSON.stringify(app.sessionState))
            ]));
        }
    });    
})();
