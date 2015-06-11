var toBeCopied = (function()
{
var objectWithSetter =
{
};
Object.defineProperty(objectWithSetter, 'Bug', {
    get: function() {
        return 10086;
    },
    set: function(name) {
    },
    enumerable: true,
});
var result = { Bug : 12580 };
result.__proto__ = objectWithSetter;
var map = new WeakMap();
map.set(result, 30624700);
return map
}());
