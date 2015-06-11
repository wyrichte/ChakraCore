(function () {

    function makeAllEnumerable(v) {
        /// <param name="v" type="Object" />
        if (v && typeof v === "object")
            Object.getOwnPropertyNames(v).forEach(function (name) {
                var pd = Object.getOwnPropertyDescriptor(v, name);
                if (!pd.enumerable && pd.configurable) {
                    pd.enumerable = true;
                    Object.defineProperty(v, name, pd);
                }
            });
        return v;
    }

    function wrap(old) {
        /// <param name="old" type="Function" />
        return function() {
            var args = [];
            for (var i = 0, len = arguments.length; i < len; i++)
                args.push(makeAllEnumerable(arguments[i]));
            old.apply(this, args);
        };
    }

    function wrapAllMethods(v) {
        /// <param name="v" type="Object" />
        if (v)
            Object.getOwnPropertyNames(v).forEach(function (name) {
                var value = v[name];
                if (typeof value === "function")
                    v[name] = wrap(value);
            });
        return v;
    }

    if (this.WinJS) {
        wrapAllMethods(WinJS.Namespace);
        wrapAllMethods(WinJS.Class);
    }

})();