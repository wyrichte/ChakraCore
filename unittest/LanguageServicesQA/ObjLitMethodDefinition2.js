var /**target:A**/a = 10;
var obj = {
	/**target:B**/b : 20,
	/**target:C**/c() {
		this./**ml:a,!b**/a/**gd:A**/ = 30;
		/**ml:a**/a/**gd:A**/ = 40;
	}
};
obj.c/**gd:C**/.call();
