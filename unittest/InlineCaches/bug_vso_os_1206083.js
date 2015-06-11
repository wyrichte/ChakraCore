// VSO OS Bug 1206083
// Accessor inline cache was not invalidated after eval() function definition overwrites global accessor property with same name
function print(x) { WScript.Echo("" + x); }
Object.defineProperty(this, "z", { get: function () { print("getter") }, set: function () { print("setter") }, configurable: true });
print(z);
eval('function z(){}');
print(z);
z = 0;
print(z);
