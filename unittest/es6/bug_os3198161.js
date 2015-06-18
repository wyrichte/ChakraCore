obj = [1,2,3,4,5];
obj.constructor = function() { return {}; }
Object.freeze(obj);
if ('{"0":2,"1":3,"length":2}' == JSON.stringify(Array.prototype.slice.call(obj,1,3)))
{
    WScript.Echo('Pass');
}
else
{
    WScript.Echo('Fail');
}