var object_literal = {
    /**
     * @property {String} name - wrong description for name - should NOT be parsed as JSDoc
     */
    name: "abc"
};

object_literal./**ml:name**/