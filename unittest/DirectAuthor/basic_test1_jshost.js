/**ref:..\\..\\Lib\\Author\\References\\domWeb.js**/
/**ref:..\\..\\Lib\\Author\\References\\libhelp.js**/
/**ref:test1_sub.js**/

const c = 1;
let l = 2;
var v = 3;

/**ml:c,l,v**/;
objLit1./**ml:xx,yy**/

function myFunc() {
	this./**ml:v,!c,!l**/;
}
myFunc();


function f2() {
	/// <returns type='HTMLElement' />
}
f2()./**ml:appendChild**/;
function f3(a) {
	/// <param name='a' type='HTMLElement' />
	a./**ml:appendChild**/;
}
function f4() {
	/// <field name='a' type='HTMLElement' />
	this.a./**ml:appendChild**/;
}
new f4();
new f4().a./**ml:appendChild**/;
var x = {
	/// <field type='HTMLElement' />
	field1: undefined
};
x.field1./**ml:appendChild**/;

intellisense.addEventListener('statementcompletion', function() {});
function DragScroll() {
	/// <field name='scrollZones' type='Array' elementType='Number'></field>
	/// <field name='scrollZones1' type='Array' elementType='Number'></field>
	/// <field name='scrollZones2' type='Array' elementType='Number'></field>
	/// <field name='scrollZones3' type='Array' elementType='Number'></field>
	/// <field name='scrollZones5' type='Array' elementType='Number'></field>

	this.scrollZones = [];
	this.scrollZones1 = [ undefined ];
	this.scrollZones2 = [ null ];
	this.scrollZones4 = [ '' ];
	this.scrollZones5 = [ '' ];
}

DragScroll.prototype.pointerEvent = function (evt) {
	this.scrollZones.forEach(function(zone) {
		zone./**ml:toFixed**/;
	});
	this.scrollZones.forEach(function(zone) {
		this./**ml:num**/;
	}, { num: 0 } /* thisArg */);
	// Verify that the patched forEach behaves correctly. 
	// Push an item into the array and verify that the change takes effect.
	this.scrollZones.push('');
	this.scrollZones.forEach(function (zone) {
		zone./**ml:concat**/;
	});
	this.scrollZones1.forEach(function (zone) {
		zone./**ml:!toFixed**/;
	});
	this.scrollZones2.forEach(function (zone) {
		zone./**ml:!toFixed**/;
	});
	this.scrollZones3.forEach(function (zone) {
		zone./**ml:toFixed**/;
	});
	this.scrollZones4.forEach(function (zone) {
		zone./**ml:concat**/;
	});
	this.scrollZones5.forEach(function (zone) {
		zone./**ml:concat**/;
	});
};
