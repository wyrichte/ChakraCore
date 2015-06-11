var f = function() {};
(function () { /*sStart*/ ;
    f(
        (function x() {
            return { iterate: function () { } };
        })(
            (function() {
                return function () { };
            })()
        )
    );
})();
