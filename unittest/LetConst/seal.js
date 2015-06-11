let x = "let x";

Object.preventExtensions(this);
Object.getOwnPropertyNames(this).forEach(function (p) {
    Object.defineProperty(this, p, { configurable: false });
});

if (Object.isSealed(this)) {
   WScript.Echo("PASS");
}

