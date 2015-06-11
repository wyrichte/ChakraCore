/// <reference path='base.js' />
 

////////////////////////////////////////////////////////////
////   © Microsoft. All rights reserved.                ////
////                                                    ////
////   This library is intended for use in WWAs only.   ////
////////////////////////////////////////////////////////////
// x86fre.fbl_pac_dev 
 



(function (global, undefined) {

    var expandProperties = function (properties, isStatic) {
        var expandedProperties = {};
        if (properties) {
            var keys = Object.keys(properties);
            for (var i = 0, len = keys.length; i < len; i++) {
                var name = keys[i],
                    property = properties[name],
                    propertyValue;

                // If the property name starts with an underscore, make it non-enumerable
                var isEnumerable = (name[0] !== '_');
                switch (typeof (property)) {
                    case "object":
                        if (property && (property.value !== undefined || typeof (property.get) === "function" || typeof (property.set) === "function")) {
                            if (property.enumerable === undefined) {
                                property.enumerable = isEnumerable;
                            }
                            propertyValue = property;
                        } else {
                            propertyValue = {
                                value: property, 
                                writable: !isStatic, 
                                enumerable: isEnumerable, 
                                configurable: false 
                            };
                        }
                        break;

                    case "function":
                        propertyValue = { 
                            value: property, 
                            writable: false, 
                            enumerable: isEnumerable, 
                            configurable: false 
                        };
                        break;

                    default:
                        propertyValue = { 
                            value: property, 
                            writable: !isStatic, 
                            enumerable: isEnumerable, 
                            configurable: false 
                        };
                        break;
                }

                expandedProperties[name] = propertyValue;
            }
        }

        return expandedProperties;
    };
    var constant = function (value) {
        return { value: value };
    };

    (function (rootNamespace) {

        // Create the rootNamespace in the global namespace
        if (!global[rootNamespace]) {
            global[rootNamespace] = Object.create(Object.prototype);
        }

        // Cache the rootNamespace we just created in a local variable
        var _rootNamespace = global[rootNamespace];
        if (!_rootNamespace.Namespace) {
            _rootNamespace.Namespace = Object.create(Object.prototype);
        }

        function defineWithParent(parentNamespace, name, members) {
            /// <summary locid="1">
            /// Defines a new namespace with the specified name, under the specified parent namespace.
            /// </summary>
            /// <param name="parentNamespace" locid="2">
            /// The parent namespace which will contain the new namespace.
            /// </param>
            /// <param name="name" locid="3">
            /// Name of the new namespace.
            /// </param>
            /// <param name="parentNamespace" locid="4">
            /// Members in the new namespace.
            /// </param>
            /// <returns locid="5">
            /// The newly defined namespace.
            /// </returns>
            var currentNamespace = parentNamespace,
                namespaceFragments = name.split(".");

            for (var i = 0, len = namespaceFragments.length; i < len; i++) {
                var namespaceName = namespaceFragments[i];
                if (!currentNamespace[namespaceName]) {
                    Object.defineProperty(currentNamespace, namespaceName, { value: Object.create(Object.prototype), writable: false, enumerable: true, configurable: true });
                }
                currentNamespace = currentNamespace[namespaceName];
            }

            if (members) {
                var newProperties = expandProperties(members, true);
                Object.defineProperties(currentNamespace, newProperties);
            }

            return currentNamespace;
        };

        function define(name, members) {
            /// <summary locid="6">
            /// Defines a new namespace with the specified name.
            /// </summary>
            /// <param name="name" locid="7">
            /// Name of the namespace.  This could be a dot-separated nested name.
            /// </param>
            /// <param name="parentNamespace" locid="4">
            /// Members in the new namespace.
            /// </param>
            /// <returns locid="5">
            /// The newly defined namespace.
            /// </returns>
            return defineWithParent(global, name, members);
        }

        // Establish members of the "Win.Namespace" namespace
        Object.defineProperties(_rootNamespace.Namespace, {

            defineWithParent: { value: defineWithParent, enumerable: true },

            define: { value: define, enumerable: true }

        });

    })("Win");

    (function (Win) {

        function createPrototype(constructor, baseClass) {
            constructor.prototype = Object.create(baseClass.prototype);
        }
        function createInstanceMembers(constructor, instanceMembers) {
            var members = expandProperties(instanceMembers, false);
            members._super = { value: Object.getPrototypeOf(constructor.prototype), writable: false };
            members.constructor = { value: constructor, writable: false };
            Object.defineProperties(constructor.prototype, members);
        }
        function createStaticMembers(constructor, staticMembers) {
            Object.defineProperties(constructor, expandProperties(staticMembers, true));
        }

        function define(constructor, instanceMembers, staticMembers) {
            /// <summary locid="8">
            /// Defines a class using the given constructor and with the specified instance members.
            /// </summary>
            /// <param name="constructor" locid="9">
            /// A constructor function that will be used to instantiate this class.
            /// </param>
            /// <param name="instanceMembers" locid="10">
            /// The set of instance fields, properties and methods to be made available on the class.
            /// </param>
            /// <returns locid="11">
            /// The newly defined class.
            /// </returns>
            constructor = constructor || function () { };
            createPrototype(constructor, Win.Class);
            createInstanceMembers(constructor, instanceMembers);
            createStaticMembers(constructor, staticMembers);
            return constructor;
        }

        function derive(baseClass, constructor, instanceMembers, staticMembers) {
            /// <summary locid="12">
            /// Uses prototypal inheritance to create a sub-class based on the supplied baseClass parameter.
            /// </summary>
            /// <param name="baseClass" locid="13">
            /// The class to inherit from.
            /// </param>
            /// <param name="constructor" locid="9">
            /// A constructor function that will be used to instantiate this class.
            /// </param>
            /// <param name="instanceMembers" locid="10">
            /// The set of instance fields, properties and methods to be made available on the class.
            /// </param>
            /// <returns locid="11">
            /// The newly defined class.
            /// </returns>
            constructor = constructor || function () { };
            createPrototype(constructor, baseClass || Win.Class);
            createInstanceMembers(constructor, instanceMembers);
            createStaticMembers(constructor, staticMembers);
            return constructor;
        }

        function mix(constructor, mixin1, mixin2) {
            /// <summary locid="14">
            /// Defines a class using the given constructor and the union of the set of instance members
            /// specified by all the mixin objects.  The mixin parameter list can be of variable length.
            /// </summary>
            /// <param name="constructor" locid="9">
            /// A constructor function that will be used to instantiate this class.
            /// </param>
            /// <param name="mixin1" locid="15">
            /// A mixin that contributes to the instance member set of this class.
            /// </param>
            /// <param name="mixin2" locid="16">
            /// Another mixin that contributes to the instance member set of this class.
            /// </param>
            /// <returns locid="11">
            /// The newly defined class.
            /// </returns>
            var instanceMembers = {};
            for (var i = 1; i < arguments.length; i++) {
                var mixin = arguments[i];
                Object.keys(mixin).forEach(function (k) {
                    instanceMembers[k] = mixin[k];
                });
            }
            constructor = constructor || define();
            createInstanceMembers(constructor, instanceMembers);
            return constructor;
        }

        // Establish members of "Win.Class" namespace
        Win.Namespace.define("Win.Class", {

            define: define,

            derive: derive,

            mix: mix,

            prototype: constant(Object.prototype)

        });

    })(Win);

})(this);




(function (global, Win) {

    // Establish members of "Win.Utilities" namespace
    Win.Namespace.defineWithParent(Win, "Utilities", {
        /// <summary locid="17">
        /// Gets the leaf-level type or namespace as specified by the name.
        /// </summary>
        /// <param name="name" locid="18">
        /// The name of the member.
        /// </param>
        /// <param name="root" locid="19">
        /// The root to start in, defaults to the global object.
        /// </param>
        /// <returns locid="20">
        /// The leaf-level type of namespace inside the specified parent namespace.
        /// </returns>
        getMember: function (name, root) {
            root = root || global;
            if (!name) {
                return null;
            }

            return name.split(".").reduce(function (currentNamespace, name) {
                if (currentNamespace) {
                    return currentNamespace[name];
                }
                return null;
            }, root);
        },
        
        /// <summary locid="21">
        /// Ensures the given function only executes after the DOMContentLoaded event has fired
        /// for the current page.
        /// </summary>
        /// <returns locid="22">Promise which completes after DOMContentLoaded has fired.</returns>
        /// <param name="callback" optional="true" locid="23">
        /// A JS Function to execute after DOMContentLoaded has fired.
        /// </param>
        /// <param name="async" optional="true" locid="24">
        /// If true then the callback should be asynchronously executed.
        /// </param>
        ready: function(callback, async) {
            return new Win.Promise(function (c, e) {
                function complete() {
                    if (callback) { 
                        try {
                            callback(); 
                            c();
                        }
                        catch (err) {
                            e(err);
                        }
                    }
                    else {
                        c();
                    }
                };

                var readyState = this.testReadyState || document.readyState;
                if(readyState === "complete" || document.body !== null) {
                    if(async) {
                        msQueueCallback(complete);
                    }
                    else {
                        complete();
                    }
                }
                else {
                  window.addEventListener("DOMContentLoaded", complete, false);
                }
            });
        }
    });

    Win.Namespace.define("Win", {
        validation: { value: false, writable: true, enumerable: true, configurable: false }
    });
})(this, Win);




// This feature is intended to be replaced by a native implementation in IE10
//

(function(global) {
    // frameLen would ideally be snapped to the vblank of the monitor
    //
    var frameLen = 16;
    // We reserve 30% of each frame for IE to render, ideally this would
    // be tuned based on actual render time
    //
    var userFrameTime = frameLen * .7;

    var _work = [];
    var dispatchFrame = function () {
        var start = new Date();
        var end;

        var q = _work;
        while (q.length > 0) {
            q.shift()();
            end = new Date();
            var elapsed = end - start;
            if (elapsed >= userFrameTime) {
                break;
            }
        }
    };

    // name is intentionally obscure, we want most people
    // to use the promise implementation until we get a 
    // final design from IE.
    //
    global.msQueueCallback = function(f) { 
        _work.push(f);
    };

    setInterval(dispatchFrame, frameLen);
})(this);



(function (global, Win) {

    function isPromise(value) {
        return typeof value === "object" && value && typeof value.then === "function";
    }

    // Helper to notify listeners on successful fulfillment of a promise
    //
    function notifySuccess(listeners, value) {
        for (var i = 0, len = listeners.length; i < len; i++) {
            var listener = listeners[i];
            //
            // NOTE: promise here is always an instance of ThenPromise and we are OK
            //  reaching into its private members _complete and _error in order to
            //  notify it of fulfillment.
            //
            var promise = listener.promise;
            var onComplete = listener.onComplete;
            try {
                if (onComplete) {
                    // If we have a onComplete handler then the fulfillment value of the 
                    // ThenPromise is the result of calling the onComplete handler.
                    //
                    promise._complete(onComplete(value));
                } else {
                    // If we do not have an onComplete handler the ThenPromise is fulfilled
                    // by the current value.
                    //
                    promise._complete(value);
                }
            } catch (exception) {
                // If an error occurs while executing the users onComplete handler then the
                // ThenPromise is itself in error with the exception value as its fulfillment
                // value.
                //
                promise._error(exception);
            }
        }
    }
    // Helper to notify listeners on error fulfillment of a promise
    //
    function notifyError(listeners, value) {
        for (var i = 0, len = listeners.length; i < len; i++) {
            var listener = listeners[i];
            //
            // NOTE: promise here is always an instance of ThenPromise and we are OK
            //  reaching into its private members _complete and _error in order to
            //  notify it of fulfillment.
            //
            var promise = listener.promise;
            var onError = listener.onError;
            try {
                if (onError) {
                    // If we have a onError handler then the fulfillment value of the 
                    // ThenPromise is the result of calling the onError handler
                    //
                    promise._complete(onError(value));
                } else {
                    // If we do not have an onError handler the ThenPromise is in error
                    // and is fulfilled by the current error value.
                    //
                    promise._error(value);
                }
            } catch (exception) {
                // If an exception occurs while executing the users onError handler then
                // the ThenPromise is in error with the exception value as its fulfillment
                // value.
                //
                promise._error(exception);
            }
        }
    }

    var state_working = 0;
    var state_waiting = 1;
    var state_waiting_canceled = 2;
    var state_fulfilled_error = 3;
    var state_fulfilled_success = 4;

    var state_fulfilled_min = state_fulfilled_error;

    var PromiseBase = Win.Class.define(
        null,
        {
            _listeners: null,
            _state: state_working,
            _value: null,

            _cancel: function () {
                switch (this._state) {
                    case state_fulfilled_error:
                    case state_fulfilled_success:
                    case state_waiting_canceled:
                        return;

                    case state_working:
                        this._error(new Error("Canceled"));
                        break;

                    case state_waiting:
                        this._state = state_waiting_canceled;

                        // If this is waiting on a completed value which is a promise then
                        // request that value to cancel itself.
                        //
                        if (typeof this._value.cancel === "function") {
                            this._value.cancel();
                        }
                        break;
                }

                this._cleanup();
            },

            _complete: function (completeValue) {
                // If we are in any state that isn't state_working then we have already
                // commited to a realized value and any duplicate calls to _complete will
                // be ignored.
                //
                if (this._state !== state_working) {
                    return;
                }

                this._value = completeValue;

                if (isPromise(completeValue)) {

                    // If _complete was called with a value which is itself a promise then
                    // we block on that promise being fulfilled. If that promise is fulfilled
                    // with a success value then this Promise continues to be successful, but
                    // if that promise is fulfilled with an error value this promise will move
                    // to an error state as well.
                    //
                    this._state = state_waiting;

                    var that = this;
                    completeValue.then(
                        function (value) { that._state = state_working; that._complete(value); },
                        function (value) { that._state = state_working; that._error(value); },
                        function (value) { that._progress(value); }
                    );
                } else {

                    this._state = state_fulfilled_success;

                    this._notify();
                }

                this._cleanup();
            },

            _error: function (errorValue) {
                if (this._state !== state_working) {
                    return;
                }

                this._value = errorValue;
                this._state = state_fulfilled_error;

                this._notify();

                // We have now entered a fulfilled state, cleanup anything which was
                // around as part of being async (async operation object, back pointer 
                // used for cancellation, etc).
                //
                this._cleanup();
            },

            _notify: function () {
                // Take ownership of the list of listeners, we will notify them all
                // and then drop them on the floor for garbage collection.
                //
                var listeners = this._listeners;
                this._listeners = null;

                if (listeners) {
                    // If there are listeners and we are in a fulfilled state then we 
                    // notify those listeners of our value.
                    //
                    switch (this._state) {
                        case state_fulfilled_success:
                            notifySuccess(listeners, this._value);
                            break;

                        case state_fulfilled_error:
                            notifyError(listeners, this._value);
                            break;
                    }
                }
            },

            _progress: function (progressValue) {
                if (this._state >= state_fulfilled_min) {
                    return;
                }
                if (this._listeners) {
                    // If there are listeners walk through the list and notify any of them
                    // which are listening for progress with the progress value.
                    // 
                    for (var i = 0, len = this._listeners.length; i < len; i++) {
                        var listeners = this._listeners[i];
                        var onProgress = listeners.onProgress;
                        try {
                            if (onProgress) {
                                onProgress(progressValue);
                            }
                        } catch (e) {
                            //
                            // Swallow exception thrown from user progress handler
                            //
                        }
                        // Progress waterfalls through Promises which do not contain a 
                        // terminating clause (complete or error).
                        //
                        if (!(listeners.onComplete || listeners.onError)) {
                            listeners.promise._progress(progressValue);
                        }
                    }
                }
            },

            then: function (onComplete, onError, onProgress) {
                /// <summary locid="25">
                /// Allows specifying work to be done on the realization of the promised value,
                /// error handling to be performed in the event that the Promise fails to realize
                /// a value and handling of progress notifications along the way.
                /// </summary>
                /// <param name="onComplete" type="Function" locid="26">
                /// Function to be called if the Promise is fulfilled successfully with a value.
                /// The value will be passed as the single argument. If null then the Promise will
                /// provide a default implementation which simply returns the value. The value returned
                /// from the function will become the fulfilled value of the Promise returned by
                /// then(). If an exception is thrown while executing the function the Promise returned
                /// by then() will move into the error state.
                /// </param>
                /// <param name="onError" type="Function" optional="true" locid="27">
                /// Function to be called if the Promise is fulfilled with an error. The error
                /// will be passed as the single argument. If null then the Promise will provide a default
                /// implementation which simply forwards the error. The value returned from the function
                /// will become the fulfilled value of the Promise returned by then().
                /// </param>
                /// <param name="onProgress" type="Function" optional="true" locid="28">
                /// Function to be called if the Promise reports progress. Data about the progress
                /// will be passed as the single argument. Promises are not required to support
                /// progress.
                /// </param>
                /// <return>
                /// Promise whose value will be the result of executing the provided complete or
                /// error function.
                /// </return>

                // We have at least one callback to make, ensure we have a list to store it in
                //
                this._listeners = this._listeners || [];

                // Create the promise that will be the return value of the then() call. Pass it
                // ourselves so that if canceled it notify this instance.
                //
                var p = new ThenPromise(this);

                this._listeners.push({
                    promise: p,
                    onComplete: onComplete,
                    onError: onError,
                    onProgress: onProgress
                });

                // If we are already done then trigger notification immediately.
                //
                if (this._state >= state_fulfilled_min) {
                    this._notify();
                }

                return p;
            }
        }
    );

    // ThenPromise is the type of instances that are returned from calling .then() on a
    // PromiseBase. It is kind of strange because it doesn't setup anything in its 
    // constructor to call _complete or _error. PromiseBase's implementation reaches 
    // around to the private members to call _complete and _error as needed on a 
    // ThenPromise instance.
    //
    var ThenPromise = Win.Class.derive(PromiseBase,
        function (creator) {
            // 'creator' is the promise on which .then() was called resulting in this instance
            //
            this._creator = creator;
        },
        {
            _cleanup: function () {
                this._creator = null;
            },

            cancel: function () {
                if (this._creator) {
                    // When we are canceled we need to propagate that up the chain.
                    //
                    this._creator.cancel();
                }
                this._cancel();
            }
        }
    );

    var CompletePromise = Win.Class.derive(PromiseBase,
        function (value) {
            this._complete(value);
        },
        {
            _cleanup: function () { },
            cancel: function () { }
        }
    );

    var ErrorPromise = Win.Class.derive(PromiseBase,
        function (error) {
            this._error(error);
        },
        {
            _cleanup: function () { },
            cancel: function () { }
        }
    );

    // Promise implements the contract that we have checked into our libraries today.
    //
    var Promise = Win.Class.derive(PromiseBase,
        function (init, cancel) {
            /// <summary locid="29">
            /// A Promise provides a mechanism to schedule work to be done on a value that
            /// has not yet been computed. It is a very convinent abstraction for managing
            /// interactions with asynchronous APIs.
            /// </summary>
            /// <param name="init" type="Function" locid="30">
            /// Function which is called during construction of the  Promise. The function
            /// is given three arguments (complete, error, progress). Inside the function
            /// the creator of the Promise should wire up the notifications supported by
            /// this value.
            /// </param>
            /// <param name="onCancel" optional="true" locid="31">
            /// Function to call if a down-stream consumer of this Promise wants to
            /// attempt to cancel its undone work. Promises are not required to be
            /// cancelable.
            /// </param>


            this._onCancel = cancel;

            try {
                var that = this;
                init(
                    function completeCallback(value) { that._complete(value); },
                    function errorCallback(value) { that._error(value); },
                    function progressCallback(value) { that._progress(value); }
                );
            } catch (e) {
                this._error(e);
            }
        },
        {
            _cleanup: function () {
                this._onCancel = null;
            },

            cancel: function () {
                /// <summary locid="32">
                /// Attempts to cancel the realization of a promised value. If the Promise hasn't
                /// already been fulfilled and cancellation is supported the Promise will enter
                /// the error state with a value of new Error("Canceled").
                /// </summary>
                if (this._onCancel) {
                    this._onCancel();
                }
                this._cancel();
            }
        },
        {
            any: function (values) {
                /// <summary locid="33">
                /// Returns a Promise which is fulfilled when one of the input Promises
                /// has been fulfilled.
                /// </summary>
                /// <param name="value" type="array" locid="34">
                /// Array of values including Promise objects or Object whose property
                /// values include Promise objects.
                /// </param>
                /// <return>
                /// Promise which on fulfillment yields the value of the input which is
                /// complete or in error.
                /// </return>
                var errorCount = 0;
                return new Promise(
                    function (c, e, p) {
                        var keys = Object.keys(values);
                        var len = keys.length;
                        var errors = Array.isArray(values) ? [] : {};
                        keys.forEach(function (key) {
                            Promise.as(values[key]).then(
                                function () { c({ key: key, value: values[key] }); },
                                function () { e({ key: key, value: values[key] }); }
                            );
                        });
                    },
                    function () {
                        var keys = Object.keys(values);
                        keys.forEach(function (key) {
                            var promise = Promise.as(values[key]);
                            if (typeof promise.cancel === "function") {
                                promise.cancel();
                            }
                        });
                    }
                );
            },
            as: function (value) {
                /// <summary locid="35">
                /// Returns a Promise, if the value is already a Promise it is returned otherwise
                /// a CompletePromise is wrapped around the value.
                /// </summary>
                /// <param name="value" locid="36">
                /// Value to be treated as a Promise.
                /// </param>
                /// <return>
                /// Promise.
                /// </return>
                if (isPromise(value)) {
                    return value;
                }
                return new CompletePromise(value);
            },
            is: function (value) {
                return isPromise(value);
            },
            join: function (values) {
                /// <summary locid="37">
                /// Creates a Promise that is fulfilled when all the values are realized.
                /// </summary>
                /// <param name="values" type="Object" locid="38">
                /// Record whose fields contains values, some of which may be Promises.
                /// </param>
                /// <return>
                /// Promise whose value is a record with the same field names as the input where
                /// each field value is a realized value.
                /// </return>
                return new Promise(
                    function (c, e, p) {
                        var keys = Object.keys(values);
                        var errors = Array.isArray(values) ? [] : {};
                        var results = Array.isArray(values) ? [] : {};
                        var pending = keys.length;
                        var argDone = function (key) {
                            if ((--pending) === 0) {
                                if (Object.keys(errors).length === 0) {
                                    c(results);
                                } else {
                                    e(errors);
                                }
                            } else {
                                p({ Key: key, Done: true });
                            }
                        };
                        keys.forEach(function (key) {
                            Promise.then(values[key],
                                function (value) { results[key] = value; argDone(key); },
                                function (value) { errors[key] = value; argDone(key); }
                            );
                        });
                    },
                    function () {
                        Object.keys(values).forEach(function (key) {
                            var promise = Promise.as(values[key]);
                            if (typeof promise.cancel === "function") {
                                promise.cancel();
                            }
                        });
                    });
            },
            then: function (value, complete, error, progress) {
                /// <summary locid="39">
                /// Static forwarder to the Promise instance method then().
                /// </summary>
                /// <param name="value" locid="36">
                /// Value to be treated as a Promise.
                /// </param>
                /// <param name="complete" type="Function" locid="40">
                /// Function to be called if the Promise is fulfilled successfully with a value.
                /// If null then the Promise will provide a default implementation which simply
                /// returns the value. The value will be passed as the single argument.
                /// </param>
                /// <param name="error" type="Function" optional="true" locid="41">
                /// Function to be called if the Promise is fulfilled with an error. The error
                /// will be passed as the single argument.
                /// </param>
                /// <param name="progress" type="Function" optional="true" locid="28">
                /// Function to be called if the Promise reports progress. Data about the progress
                /// will be passed as the single argument. Promises are not required to support
                /// progress.
                /// </param>
                /// <return>
                /// Promise whose value will be the result of executing the provided complete function.
                /// </return>
                return Promise.as(value).then(complete, error, progress);
            },
            thenEach: function (values, complete, error, progress) {
                /// <summary locid="42">
                /// Performs an operation on all of the input promises and returns a Promise
                /// which is in the shape of the input and contains the result of the operation
                /// having been performed on each input.
                /// </summary>
                /// <param name="values" locid="43">
                /// Values (array or record) of which some or all are promises.
                /// </param>
                /// <param name="complete" type="Function" locid="40">
                /// Function to be called if the Promise is fulfilled successfully with a value.
                /// If null then the Promise will provide a default implementation which simply
                /// returns the value. The value will be passed as the single argument.
                /// </param>
                /// <param name="error" type="Function" optional="true" locid="41">
                /// Function to be called if the Promise is fulfilled with an error. The error
                /// will be passed as the single argument.
                /// </param>
                /// <param name="progress" type="Function" optional="true" locid="28">
                /// Function to be called if the Promise reports progress. Data about the progress
                /// will be passed as the single argument. Promises are not required to support
                /// progress.
                /// </param>
                /// <return>
                /// Promise that is the result of calling Promise.join on the parameter 'values'
                /// </return>
                var result = Array.isArray(values) ? [] : {};
                Object.keys(values).forEach(function (key) {
                    result[key] = Promise.as(values[key]).then(complete, error, progress);
                });
                return Promise.join(result);
            },
            timeout: function (timeout) {
                if (!timeout) {
                    return new Win.Promise(
                        function timeoutComplete(c) {
                            msQueueCallback(c);
                        }
                    );
                }
                else {
                    var id = 0;

                    return new Win.Promise(
                        function (c) {
                            id = setTimeout(c, timeout);
                        },
                        function () {
                            if (id) {
                                clearTimeout(id);
                            }
                        }
                    );
                }
            },
            wrap: function (value) {
                return new CompletePromise(value);
            },
            wrapError: function (error) {
                return new ErrorPromise(error);
            }
        }
    );

    // Publish Win.Promise
    //
    Win.Namespace.define("Win", {
        Promise: Promise
    });

    // Wraps a WinRT async operation in a promise, normal usage is:
    //
    //  msWRT(fileInfo.getViewAsync()).
    //      then(function (view) { /* do something */ });
    //
    window.msWRT = function (op) {
        return op;
    };

})(this, Win);




(function () {

    Win.Namespace.define("Win", {
        xhr: function (options) {
            var req;
            return new Win.Promise(
                function (c, e, p) {
                    req = new XMLHttpRequest();
                    req.onreadystatechange = function () {
                        if (req.readyState === 4) {
                            if (req.status >= 200 && req.status < 300) {
                                c(req);
                            } else {
                                e(req);
                            }
                            req.onreadystatechange = function () { };
                        } else {
                            p(req);
                        }
                    };

                    req.open(
                        options.type || "GET",
                        options.url,
                        // Promise based XHR does not support sync.
                        //
                        true,
                        options.user,
                        options.password
                    );

                    Object.keys(options.headers || {}).forEach(function (k) {
                        req.setRequestHeader(k, options.headers[k]);
                    });

                    req.send(options.data);
                },
                function () {
                    req.abort();
                }
            );
        }
    });

})();




(function (Win, undefined) {

    function createEventProperties(events) {
        /// <summary locid="44">
        /// Creates a object which has properties for each name passed to the function.
        /// The names are prefixed with 'on'.
        /// </summary>
        /// <param name="events" locid="45">
        /// Variable argument list of property names.
        /// </param>
        /// <returns locid="46">
        /// Object which has properties for each event name passed to the function.
        /// </returns>
        var props = {};
        for (var i = 0, len = arguments.length; i < len; i++) {
            (function (name) {
                var wrapperFunction;
                var userHandler;
            
                props["on" + name] = {
                    get: function () {
                        return userHandler;
                    },
                    set: function (handler) {
                        if (handler) {
                            if (!wrapperFunction) {
                                wrapperFunction = function (evt) {
                                    userHandler(evt);
                                };
                                this.addEventListener(name, wrapperFunction, false);
                            }
                            userHandler = handler;
                        } else {
                            this.removeEventListener(name, wrapperFunction, false);
                            wrapperFunction = null;
                            userHandler = null;
                        }
                    },
                    enumerable: true
                }
            })(arguments[i]);
        }
        return props;
    }

    var EventMixinEvent = Win.Class.define(
        function (type, detail, target) {
            this.detail = detail;
            this.target = target;
            this.timeStamp = Date.now();
            this.type = type;
        },
        {
            bubbles: { value: false, writable: false },
            cancelable: { value: false, writable: false },
            currentTarget: { 
                get: function () { return this.target; } 
            },
            defaultPrevented: {
                get: function () { return this._preventDefaultCalled; }
            },
            trusted: { value: false, writable: false },
            eventPhase: { value: 0, writable: false },
            target: null,
            timeStamp: null,
            type: null,

            preventDefault: function () {
                this._preventDefaultCalled = true;
            },
            stopImmediatePropagation: function () {
                this._stopImmediatePropagationCalled = true;
            },
            stopPropagation: function () {
            }
        }
    );

    var eventMixin = {
        _listeners: null,

        addEventListener: function (type, listener, useCapture) {
            /// <summary locid="47">
            /// Adds an event listener to the control.
            /// </summary>
            /// <param name="type" locid="48">
            /// The type (name) of the event.
            /// </param>
            /// <param name="listener" locid="49">
            /// The listener to invoke when the event gets raised.
            /// </param>
            /// <param name="useCapture" locid="50">
            /// Specifies whether or not to initiate capture.
            /// </param>
            this._listeners = this._listeners || {};
            this._listeners[type] = this._listeners[type] || [];
            this._listeners[type].push({ listener: listener, useCapture: useCapture || false });
        },
        dispatchEvent: function (type, details) {
            /// <summary locid="51">
            /// Raises an event of the specified type and with additional properties.
            /// </summary>
            /// <param name="type" locid="48">
            /// The type (name) of the event.
            /// </param>
            /// <param name="eventProperties" locid="52">
            /// The set of additional properties to be attached to the event object when the event is raised.
            /// </param>
            /// <returns locid="53">
            /// Boolean indicating whether preventDefault was called on the event.
            /// </returns>
            var eventValue = new EventMixinEvent(type, details, this);
            var listeners = this._listeners && this._listeners[type];
            if (listeners) {
                // Need to copy the array to protect against people unregistering while we are dispatching
                listeners = listeners.slice(0, listeners.length);
                for (var i = 0, len = listeners.length; i < len && !eventValue._stopImmediatePropagationCalled; i++) {
                    listeners[i].listener(eventValue);
                }
            }
            return eventValue.defaultPrevented || false;
        },
        removeEventListener: function (type, listener, useCapture) {
            /// <summary locid="54">
            /// Removes an event listener from the control.
            /// </summary>
            /// <param name="type" locid="48">
            /// The type (name) of the event.
            /// </param>
            /// <param name="listener" locid="55">
            /// The listener to remove from the invoke list.
            /// </param>
            /// <param name="useCapture" locid="50">
            /// Specifies whether or not to initiate capture.
            /// </param>
            useCapture = useCapture || false;
            var listeners = this._listeners && this._listeners[type];
            if (listeners) {
                for (var i = 0, len = listeners.length; i < len; i++) {
                    var l = listeners[i];
                    if (l.listener === listener && l.useCapture === useCapture) {
                        listeners.splice(i, 1);
                        if (listeners.length === 0) {
                            delete this._listeners[type];
                        }
                        // Only want to remove one element for each call to removeEventListener
                        break;
                    }
                }
            }
        }
    };

    Win.Namespace.defineWithParent(Win, "Utilities", {
        createEventProperties: createEventProperties,
        eventMixin: eventMixin
    });

})(Win);

