var o1,o2;
function main() {
    o1 = Array.prototype.entries.call('');
    o1[0] = 1;
    o1.length = 20;

    o2 = Array.prototype.entries.call('');
    Object.defineProperty(o2, 'tuna', { set: function(){} });
    Array.prototype.sort.call(o1); //pop,push,reduce...
}

main();

WScript.Echo('pass');
