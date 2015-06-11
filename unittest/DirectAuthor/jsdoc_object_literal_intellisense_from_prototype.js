function Base() { }
function Test() { }

Base.prototype.abc = "123";
Test.prototype = new Base();
Test.prototype.def = "456";

/**
 * @param {Test} op
 */
function CallIt(op)
{

}

CallIt({/**ml:abc,def,!constructor**/