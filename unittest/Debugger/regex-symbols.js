RegExp.prototype[Symbol.search] = function search() {
    return undefined; /**bp:stack()**/
}
'string'.search(/./);

RegExp.prototype[Symbol.match] = function match() {
    return undefined; /**bp:stack()**/
}
'string'.match(/./);

RegExp.prototype[Symbol.split] = function split() {
    return undefined; /**bp:stack()**/
}
'string'.split(/./);

print("Pass");
