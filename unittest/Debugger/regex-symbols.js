RegExp.prototype[Symbol.match] = function match() {
    return undefined; /**bp:stack()**/
}

'string'.match(/./);
print("Pass");
