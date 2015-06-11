//
// SCA tests for builtin objects
//
var tests_builtin = [
    {
        get: function() {
            return new Boolean(true);
        }
    },

    {
        get: function() {
            return new Boolean(false);
        }
    },

    {
        get: function() {
            return new Date("2011-04-15T21:47:42.658Z");
        }
    },

    {
        get: function() {
            return new Number(12);
        }
    },

    {
        get: function() {
            return new String("String object");
        }
    },

    /./,
    /./i,
    /./g,
    /./m,
    /.*/i,
    /\s+/gim,
    /^.+abc$/ig,
    {
        get: function() {
            return new RegExp("^\s+regex.*$","gm");
        }
    },
];
