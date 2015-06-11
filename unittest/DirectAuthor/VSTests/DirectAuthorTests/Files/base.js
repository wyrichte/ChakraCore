(function (global, rootNamespace, _undefined) {
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
                        if (property !== null && (property.value !== _undefined || typeof (property.get) === "function" || typeof (property.set) === "function")) {
                            if (property.enumerable === _undefined) {
                                property.enumerable = isEnumerable;
                            }
                            propertyValue = property;
                        } else {
                            propertyValue = { value: property, writable: !isStatic, enumerable: isEnumerable, configurable: false };
                        }
                        break;

                    case "function":
                        propertyValue = { value: property, writable: false, enumerable: isEnumerable, configurable: false };
                        break;

                    default:
                        propertyValue = { value: property, writable: !isStatic, enumerable: isEnumerable, configurable: false };
                        break;
                }

                expandedProperties[name] = propertyValue;
            }
        }

        return expandedProperties;
    };

    var constant = function (value) {
        return { value: value, writable: false /* WOOB: 1126722, this shouldn't be needed */ };
    };

    // Create the rootNamespace in the global namespace
    if (!global[rootNamespace]) {
        global[rootNamespace] = Object.create(null);
    }

    // Cache the rootNamespace we just created in a local variable
    var _rootNamespace = global[rootNamespace];
    if (!_rootNamespace.Namespace) {
        _rootNamespace.Namespace = Object.create(null);
    }

    // Establish members of the "Win.Namespace" namespace
    Object.defineProperties(_rootNamespace.Namespace, {
        defineWithParent: constant(
            function (parentNamespace, name, members) {
                /// <summary>
                /// Defines a new namespace with the specified name, under the specified parent namespace. 
                /// </summary>
                /// <param name='parentNamespace'>
                /// The parent namespace which will contain the new namespace.
                /// </param>
                /// <param name='name'>
                /// Name of the new namespace.
                /// </param>
                /// <param name='parentNamespace'>
                /// Members in the new namespace.
                /// </param>
                /// <returns>
                /// The newly defined namespace.
                /// </returns>
                var currentNamespace = parentNamespace,
                    namespaceFragments = name.split(".");
                for (var i = 0, len = namespaceFragments.length; i < len; i++) {
                    var namespaceName = namespaceFragments[i];
                    if (!currentNamespace[namespaceName]) {
                        Object.defineProperty(currentNamespace, namespaceName, { value: Object.create(null), writable: false, enumerable: true });
                    }
                    currentNamespace = currentNamespace[namespaceName];
                }

                if (members) {
                    var newProperties = expandProperties(members, true);
                    Object.defineProperties(currentNamespace, newProperties);
                }


		// Start Debugger Test Code
		var i = 0;
		i++;
		i++;
		i++;
		i++;
		i++;
		i++;
		i++;
		i++;
		i++;
		// End Debugger Test Code

                return currentNamespace;
            }
        ),

        define: constant(
            function (name, members) {
                /// <summary>
                /// Defines a new namespace with the specified name.
                /// </summary>
                /// <param name='name'>
                /// Name of the namespace.  This could be a dot-separated nested name.
                /// </param>
                /// <param name='parentNamespace'>
                /// Members in the new namespace.
                /// </param>
                /// <returns>
                /// The newly defined namespace.
                /// </returns>
                return this.defineWithParent(global, name, members);
            }
        )
    });

    // Establish members of "Win.Class" namespace
    _rootNamespace.Namespace.defineWithParent(_rootNamespace, "Class", {
        _objectFromProperties: function (baseClass, properties, constructor, statics) {
            if (typeof (constructor) !== "function") {
                throw "Constructors have to be functions.";
            }

            var outerObj = constructor,
                expandedProperties = expandProperties(properties, false);
            expandedProperties._super = { value: baseClass.prototype, writable: false };

            Object.defineProperty(outerObj, "prototype", { value : Object.create(baseClass.prototype, expandedProperties) });
            Object.defineProperties(outerObj, expandProperties(statics, true));

            return outerObj;
        },

        define: function (baseClass, properties, constructor, statics) {
            /// <summary>
            /// Defines a new class derived from the baseClass, with the specified properties and constructors.  
            /// The statics will be available as top-level members on the Class object.
            /// </summary>
            /// <param name='baseClass'>
            /// The class to inherit from.
            /// </param>
            /// <param name='properties'>
            /// The set of new properties on the new class.
            /// </param>
            /// <param name='constructor'>
            /// A constructor function that can instantiate this class.
            /// </param>
            /// <param name='statics'>
            /// A set of static members to be attached to the top-level class object.
            /// </param>
            /// <returns>
            /// The newly defined class.
            /// </returns>
            return this._objectFromProperties(
                baseClass || _rootNamespace.Class,
                properties,
                constructor || function () { },
                statics);
        },

        prototype: {}
    });
})(this, "Win");

(function (global, Win, _undefined) {

    // Establish members of "Win.Utilities" namespace
    Win.Namespace.defineWithParent(Win, "Utilities", {
        /// <summary>
        /// Gets the leaf-level type or namespace as specified by the name.
        /// </summary>
        /// <param name='name'>
        /// The name of the member.
        /// </param>
        /// <returns>
        /// The leaf-level type of namespace inside the specified parent namespace.
        /// </returns>
        getMember: function (name) {
            if (!name) {
                return null;
            }

            return name.split(".").reduce(function (currentNamespace, name) {
                if (currentNamespace) {
                    return currentNamespace[name];
                }
                return null;
            }, global);
        },

        /// <summary>
        /// Returns a merged namespace object from all the namespaces passed in.
        /// </summary>
        /// <returns>
        /// A merged namespace object.
        /// </returns>
        // UNDONE: this would be implemented by a native construct in Eze to avoid
        // the performance implications of this eager model.
        merge: function () {
            var merged = {};
            var addProperty = function (namespaceObject) {
                Object.keys(namespaceObject).forEach(function (memberName) {
                    Object.defineProperty(merged, memberName, {
                        get: function () { return namespaceObject[memberName]; },
                        set: function (value) { namespaceObject[memberName] = value; }
                    });
                });
            };

            // arguments is an "array-like" structure, you can't use forEach on it,
            // so we simulate it here to ensure we get the right closure semantics.
            for (var i = 0, len = arguments.length; i < len; i++) {
                addProperty(arguments[i]);
            }

            return merged;
        },
        
        /// <summary>
        /// Ensures the given function only executes after the DOMContentLoaded event has fired
        /// for the current page.
        /// </summary>
        /// <param name="callback">
        /// A JS Function to execute after DOMContentLoaded has fired.
        /// </param>
        /// <param name="async">
        /// If true then the callback should be asynchronously executed.
        /// </param>
        executeAfterDomLoaded: function(callback, async) {
        
            var readyState = this.testReadyState || document.readyState;
            
            if(readyState === "complete" || readyState === "interactive") {
              if(async) {
                  window.setTimeout(function () { callback(); }, 0);
              }
              else {
                  callback();
              }
            }
            else {
              window.addEventListener("DOMContentLoaded", callback, false);
            }
        }
    });

    // Promote "merge".  This is one place where we modify the global namespace.
    if (!global.merge) {
        global.merge = Win.Utilities.merge;
    }
})(this, Win);

