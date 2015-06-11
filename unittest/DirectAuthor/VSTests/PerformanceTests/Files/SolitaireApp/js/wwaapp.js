// Copyright (c) Microsoft Corporation
// All rights reserved

/// <reference path="../base/_es3.js" />
/// <reference path="../base/base.js" />

(function (global, Win, undefined) {
    var windowSetTimeout = global.setTimeout;
    var windowClearInterval = global.clearInterval;
    var windowSetInterval = global.setInterval;
    var dispatcherInitialized = false;
    var dispatcherInterval;

    var ApplicationClass = Win.Class.define(null, {
        localStorage: { get: function () { return window.localStorage; } },
        totalTime: { get: function () { return Win.Application._totalTime; } },
        state: { get: function () { return Win.Application._applicationInstance.state; } },
        recycle: function () {
            Win.Application.recycle();
        },
        run: function () {
            /// <summary>
            ///     run method
            ///</summary>
            Win.Application.run();
        },
        dispatch: function (callback) {
            /// <summary>
            ///     dispatch method
            ///</summary>
            Win.Application.dispatch(callback);
        },
        addEventListener: function (eventType, listener, capture) {
            Win.Application._listeners[eventType] = listener;
        }
    });

    var applicationStateMachineDefinition = Object.freeze({
        stateOnLoad: "loaded",
        stateOnReload: "startup",
        loaded: { next: "startup" },
        startup: { next: "running" },
        running: {}
    });

    var currentState = function () {
        return Win.Application._applicationInstance[Win.Application._applicationInstance.state];
    };

    var wrapStringCallback = function (callback) {
        if (typeof (callback) === "string") {
            return function () { return eval(callback) };
        }
        else {
            return callback;
        }
    }
    var dispatchFrame = function (heartbeat, fps, timeAvailableForUserCode, elapsedFrames, elapsedTime, totalTime) {
        Win.Application._totalTime = totalTime;
        var start = new Date();
        heartbeat();

        var f = Win.Application._listeners.frame;
        if (f) {
            f(fps, elapsedFrames, elapsedTime, totalTime);
        }

        var end = new Date();

        var queues = Win.Application._workqueues;
        for (var qi = 0, ql = queues.length; qi < ql; qi++) {
            var q = queues[qi];

            // don't process any work from the background queue if we are out of time
            //
            if (qi == ql - 1) { if ((end - start) >= timeAvailableForUserCode) { break; } }

            while (q.length > 0) {
                if (q[0].timeStamp < totalTime) {
                    q.shift().callback();
                    // only process at most 1 item from the background queue
                    //
                    if (qi == ql - 1) { break; }
                    end = new Date();
                    if ((end - start) >= timeAvailableForUserCode * .9) {
                        break;
                    }
                }
                else {
                    break;
                }
            }
        }
    };

    var initializeDispatcher = function () {
        if (dispatcherInitialized) { return; }
        dispatcherInitialized = true;
        var heartbeat = initializeHeartbeat();

        // IE will (it's a bug if this behavior changes) sync 16.7ms interval to VBlank of the 
        // monitor, so we avoid tearing by locking at 60fps 16.7ms wait.
        var fps = 60;
        var delay = 16.7;

        // UNDONE: reserve 30% of the frame for rendering... this should be dynamically tuned
        var userCodeTime = delay * .7;

        var start = new Date();
        var lastFrame = 0;
        var lastTime = 0;

        dispatcherInterval = windowSetInterval(function () {
            var now = new Date();
            var totalTime = now - start;
            var elapsedTime = totalTime - lastTime;
            var frame = ((totalTime / 1000) * fps) >> 0;
            if (frame != lastFrame) {
                dispatchFrame(heartbeat, fps, userCodeTime, frame - lastFrame, elapsedTime, totalTime);
                lastFrame = frame;
                lastTime = totalTime;
            }
        }, delay);
    };
    var unloadDispatcher = function () {
        if (dispatcherInitialized) {
            dispatcherInitialized = false;
            windowClearInterval(dispatcherInterval);
        }
    };

    var initializeHeartbeat = function () {
        var maxDelay = 2000;
        var skipDelay = 500;
        var skipCount = 6;

        var lastBeat = new Date();
        var skipped = 0;
        return function () {
            /*
            var beat = new Date();
            var elapsed = beat - lastBeat;
            lastBeat = beat;
            if (elapsed > maxDelay) {
                Win.Application._applicationClass.recycle();
            }
            else if (elapsed > skipDelay) {
                skipped++;
                if (skipped > skipCount) {
                    Win.Application._applicationClass.recycle();
                }
            }
            else {
                skipped = 0;
            }
            */
        };
    };

    var initializeMessage = function () {
        window.onmessage = function (e) {
            Win.Application.dispatchUrgent(function () {
                var listener = Win.Application._listeners["message"];
                if (listener) {
                    listener(e);
                }

                if (e.data && e.data.length > 9 && e.data.substring(0, 8) == "wwahost.") {
                    var name = e.data.substring(8);
                    var listener = Win.Application._listeners[name];
                    if (listener) {
                        listener(e);
                    }
                }
            });
        };
    };
    var unloadMessage = function () {
        window.onmessage = null;
    };

    var processCurrentState = function (listener, curState) {
        var complete = function (skipProcessing, nextState) {
            var next = nextState || curState.next;
            if (next) {
                Win.Application._applicationInstance.state = nextState || curState.next;
                Win.Application._saveState();
                if (!skipProcessing) {
                    Win.Application._processState();
                }
            }
        };
        if (listener) {
            listener(Win.Application._applicationClass, complete);
        }
        else {
            complete();
        }
    };

    Win.Namespace.defineWithParent(Win, "Application", {
        _applicationClass: new ApplicationClass(),
        _applicationInstance: { value: null, writable: true, enumerable: false },
        _listeners: { value: {
            startup: function (app, complete) { 
                window.addEventListener("DOMContentLoaded", function () { complete(); }, true);
            } 
        }, writable: true, enumerable: false },
        _nextIntervalId: { value: 1, writable: true, enumerable: false },
        _intervals: { value: [], writable: true, enumerable: false },
        _workqueues: { value: [[], [], []], writable: true, enumerable: false },
        _totalTime: { value: 0, writable: true, enumerable: false },

        totalTime: { get: function () { return Win.Application._totalTime; } },

        _processState: function () {
            var curState = currentState();
            if (curState) {
                var handleState = function () {
                    var listener = Win.Application._listeners[Win.Application._applicationInstance.state];
                    processCurrentState(listener, curState);
                };
                if (!curState.async) {
                    handleState();
                }
                else {
                    Win.Application.dispatch(handleState);
                }
            }
        },

        _saveState: function () {
            if (global.sessionStorage) {
                global.sessionStorage.applicationInstance = JSON.stringify(Win.Application._applicationInstance);
            }
        },

        connect: function () {
            var loaded = false;
            initializeDispatcher();
            initializeMessage();

            if (global.sessionStorage && global.sessionStorage.applicationInstance) {
                var temp = JSON.parse(global.sessionStorage.applicationInstance);
                if (temp) {
                    Win.Application._applicationInstance = Object.create(applicationStateMachineDefinition);
                    Win.Application._applicationInstance.state = temp.state;
                    loaded = true;
                }
            }
            if (!loaded) {
                Win.Application._applicationInstance = Object.create(applicationStateMachineDefinition);
                Win.Application._applicationInstance.state = Win.Application._applicationInstance.stateOnLoad || "loaded";
                Win.Application._saveState();
            }
            else {
                Win.Application._applicationInstance.state = Win.Application._applicationInstance.stateOnReload || Win.Application._applicationInstance.state;
                Win.Application._saveState();
            }

            window.setInterval = Win.Application.setInterval;
            window.clearInterval = Win.Application.clearInterval;
            window.setTimeout = Win.Application.setTimeout;

            return Win.Application._applicationClass;
        },

        disconnect: function () {
            Win.Application._listeners = {};
            Win.Application._intervals = [];
            Win.Application._workqueues = [[], [], []];
            unloadDispatcher();
            unloadMessage();
            window.setInterval = windowSetInterval;
            window.clearInterval = windowClearInterval;
            window.setTimeout = windowSetTimeout;
        },

        dispatchBackground: function (callback) {
            initializeDispatcher();
            Win.Application._workqueues[2].push({ timeStamp: Win.Application._totalTime, callback: wrapStringCallback(callback) });
        },

        dispatch: function (callback) {
            initializeDispatcher();
            Win.Application._workqueues[1].push({ timeStamp: Win.Application._totalTime, callback: wrapStringCallback(callback) });
        },

        dispatchUrgent: function (callback) {
            initializeDispatcher();
            Win.Application._workqueues[0].push({ timeStamp: Win.Application._totalTime, callback: wrapStringCallback(callback) });
        },

        recycle: function () {
            if (global.sessionStorage) {
                delete global.sessionStorage.applicationInstance;
            }
            window.location = window.location;
        },

        run: function () {
            Win.Application._processState();
        },

        setTimeout: function (callback, delay) {
            var start = new Date();
            var worker = function () {
                var elapsed = (new Date()) - start;
                if (elapsed >= delay) {
                    callback();
                }
                else {
                    Win.Application.dispatch(worker);
                }
            };
            Win.Application.dispatch(worker);
        },
        setInterval: function (callback, delay) {
            var id = Win.Application._nextIntervalId;
            Win.Application._nextIntervalId++;
            Win.Application._intervals[id] = new Date();
            var worker = function () {
                var last = Win.Application._intervals[id];
                if (!last) { return; }

                var cur = new Date();
                var elapsed = cur - last;
                if (elapsed >= delay) {
                    Win.Application._intervals[id] = cur;
                    callback();
                }
                Win.Application.dispatch(worker);
            };
            Win.Application.dispatch(worker);
            return id;
        },
        clearInterval: function (intervalId) {
            delete Win.Application._intervals[intervalId];
        }
    })
})(this, Win);﻿// Copyright (c) Microsoft Corporation
// All rights reserved

/// <reference path="../base/_es3.js" />
/// <reference path="../base/base.js" />

(function (Win, undefined) {
    var navigateEventName = "navigate";

    Win.Namespace.defineWithParent(Win, "Navigation", {
        _listeners: [],
        _pending: { value: null, writable: true },
        _default: { value: "", writable: true },
        _initialized: { value: false, writable: true },
        _initialize: function () {
            if (!Win.Navigation._initialized) {
                Win.Navigation._initialized = true;
                window.addEventListener("hashchange", Win.Navigation._onhashchange, false);
            }
        },
        _forceUpdateHash: function (e) {
            /// <summary>
            /// Forces the window.location.hash to have the current hash. For some reason I am
            /// seeing IE9 display a blank hash in the address bar, even when window.location.hash is
            /// set to something, when you navigate back.
            /// </summary>
            var hash = window.location.hash;
            var isDefaultHash = (!hash || hash === "" || hash === "#" || hash === "#" + Win.Navigation._default);
            var requestingDefaultHash = e.urlFragment === Win.Navigation._default;

            if (!requestingDefaultHash || (requestingDefaultHash && !isDefaultHash)) {
                if (e.urlFragment) {
                    window.location.hash = "#" + e.urlFragment;
                }
                else {
                    window.location.hash = "";
                }
            }

        },
        _onhashchange: function (event) {
            var e = Win.Navigation._pending;
            Win.Navigation._pending = null;
            if (!e || e.urlFragment !== Win.Navigation.urlFragment) {
                e = { urlFragment: Win.Navigation.urlFragment };
            }

            // UNDONE: why is this needed... it appears that IE (at least for files?) goes 
            // back to a naked URL in the address bar when you press "back" in the browser...?
            //
            Win.Navigation._forceUpdateHash(e);

            Win.Navigation._listeners.forEach(function (l) {
                l(e);
            });

            if (e.succcess) {
                e.success();
            }
        },
        urlFragment: {
            get: function () {
                var hash = window.location.hash;
                if (!hash || hash == "" || hash == "#") {
                    return Win.Navigation._default;
                }
                else {
                    return hash.substring(1);
                }
            }
        },
        _domContentListener: function (event) {
            window.removeEventListener("DOMContentLoaded", Win.Navigation._domContentListener, false);
            Win.Navigation._onhashchange();
        },
        registerDefault: function (urlFragment, navigateOnReady) {
            Win.Navigation._initialize();
            Win.Navigation._default = urlFragment;
            if (navigateOnReady) {
                if(document.readyState === "complete" || document.readyState === "interactive") {
                  window.setTimeout(function () { Win.Navigation._domContentListener(); }, 5);
                }
                else {
                  window.addEventListener("DOMContentLoaded", Win.Navigation._domContentListener, false);
                }
            }
        },
        navigate: function (urlFragment, options) {
            var hash = window.location.hash;

            // If hash isn't right and hash is not "default" when you are navigating to
            var notRightHash = hash !== "#" + urlFragment;
            var isDefaultHash = (!hash || hash === "" || hash === "#" || hash === "#" + Win.Navigation._default);
            var requestingDefaultHash = urlFragment === Win.Navigation._default;

            if (notRightHash && (!requestingDefaultHash || (requestingDefaultHash && !isDefaultHash))) {
                Win.Navigation._initialize();
                if (Win.Navigation._pending) {
                    // UNDONE: only a single navigation at a time is allowed, is that viable?
                    // 
                    throw "Only a single navigation at a time is allowed";
                }
                Win.Navigation._pending = { urlFragment: urlFragment, options: options };

                if (!notRightHash) {
                    setTimeout(Win.Navigation._onhashchange, 1);
                }
                else {
                    window.location.hash = "#" + urlFragment;
                }
            }
        },
        canNavigate: { get: function () { return !Win.Navigation._pending; } },
        addEventListener: function (eventType, listener, capture) {
            Win.Navigation._initialize();
            if (eventType === navigateEventName) {
                Win.Navigation._listeners.push(listener);
            }
        },
        removeEventListener: function (eventType, listener, capture) {
            Win.Navigation._initialize();
            if (eventType === navigateEventName) {
                var listeners = Win.Navigation._listeners;
                for (var i = 0, l = listeners.length; i < l; i++) {
                    if (listeners[i] === listener) {
                        delete listeners[i];
                    }
                }
            }
        }
    });
})(Win);
﻿// Copyright (c) Microsoft Corporation
// All rights reserved

/// <reference path="../base/_es3.js" />
/// <reference path="../base/base.js" />
/// <reference path="navigation.js" />

(function (Win, undefined) {
    Win.Namespace.defineWithParent(Win, "Navigation", {
        Frame: Win.Class.define(
            null,
            {
                // UNDONE: do slide left/right instead of just replacing the this._activeContent
                //
                _navigate: function (options) {
                    this._activeContent.innerHTML = "";
                    var that = this;
                    Win.Controls.FragmentLoader.addFragment(
                        this._activeContent,
                        options.href,
                        options,
                        function () {
                            Win.Controls.processAll(that._activeContent.firstChild, function () {
                                if (options.success) {
                                    options.success();
                                }
                            });
                        }
                    );
                }
            },
            function (element, options) {
                var instance = this;
                if (this === window) {
                    instance = Object.create(Win.Navigation.Frame.prototype, {});
                }

                var ac = instance._activeContent = document.createElement("div");
                ac.setAttribute("class", "activeContent");
                element.appendChild(ac);

                var url = options.href;
                if (url) {
                    Win.Navigation.registerDefault(url, true);
                }
                Win.Navigation.addEventListener("navigate",
                    function (e) {
                        instance._navigate({ href: e.urlFragment });
                    }, false);

                return instance;
            }
        )
    });
})(Win);﻿// Copyright (c) Microsoft Corporation
// All rights reserved

/// <reference path="../base/_es3.js" />
/// <reference path="../base/base.js" />
/// <reference path="navigation.js" />

(function (Win, undefined) {
    Win.Namespace.defineWithParent(Win, "Navigation", {
        Link: Win.Class.define(null, null,
            function (element, options) {
                var instance = this;
                if (this == window) {
                    instance = Object.create(Win.Navigation.Link.prototype, {});
                }

                if (options.href) {
                    if (options.href[0] == "#") {
                        element.href = options.href;
                    }
                    else {
                        element.href = "#" + options.href;
                    }
                }

                element.onclick = function (event) {
                    Win.Navigation.navigate(element.getAttribute("href").substring(1), options);
                    return false;
                };

                return instance;
            })
    });
})(Win);
