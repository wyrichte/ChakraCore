function /**target:FOO1**/foo() {
}

var obj = {
	/**ml:foo**//**target:FOO2**/foo
};
obj./**ml:foo**/foo/**gd:FOO2**/ = 20;
a/**target:FOO1**/();