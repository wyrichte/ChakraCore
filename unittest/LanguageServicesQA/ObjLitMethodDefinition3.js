var obj = {
	/**target:A**/a() {
		return 10;
	}
};
with (obj) {
	/**ml:a**/a/**gd:A**/();
}
