var /**target:A1**/a = 10;
var obj = {
	/**target:A2**/a
};
with (obj) {
	/**ml:a**/a/**gd:A2**/ = 20;
}
a/**target:A1**/++;