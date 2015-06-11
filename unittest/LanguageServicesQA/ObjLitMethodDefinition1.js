var /**target:A**/a = 10;
var obj = {
	/**target:B**/b : 20,
	/**target:C**/c() {
		this./**ml:b**/b/**gd:B**/ = 30;
		/**ml:a**/a = 40;
	}
};
obj./**ml:b,c**/c/**gd:C**/();
