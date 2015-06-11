// Setting OMBP on Setter property

/**onmbp:locals(1);stack()**/

function Product(name, price) {
    this.name = name;
    this.price = price;
    var _discount = 0;
    Object.defineProperty(this, "discount", {
        get : function () {
            return _discount;
        },
        set : function (value) {
            _discount = value;
        }
    });
}
var sneakers = new Product("Sneakers", 20);

var obj = {};
Object.defineProperty(obj, "newDataProperty", {
    value: 1,
    writable: true,
    enumerable: true,
    configurable: true
});

function attach() {
    var x = 1; /**bp:mbp("sneakers", 'properties', 'all', "MB1");**/
    sneakers.discount = 50;
    delete sneakers.discount;
    sneakers.discount = 10; // No OMBP hit as getter/setter is gone

    x = 1; /**bp:mbp("obj", 'properties', 'all', "MB2");**/
    obj.newDataProperty = 2;
}
WScript.Attach(attach);
WScript.Echo("pass");