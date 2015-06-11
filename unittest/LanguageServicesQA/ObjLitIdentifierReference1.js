var /**target:A1**/a = 10;
var obj = {
	/**ml:a**//**target:A2**/a,
	b() {
		/**ml:a**/a/**gd:A1**/ = 10;
	}
};
obj./**ml:a**/a/**gd:A2**/ = 20;
a/**gd:A1**/++;
obj.b.call();