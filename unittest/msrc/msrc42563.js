
function main() {
    RegExp.input = {toString: f};
    if (RegExp.lastMatch != "bbbbbbbbbb") {
        print('Fail')
    } else {
        print('Pass');
    }
}
    
var input = [Array(100).join("a"), Array(11).join("b"), Array(100).join("a")].join("");
    
function f() {
    String.prototype.match.call(input, "bbbbbbbbbb");
}
    
main();
