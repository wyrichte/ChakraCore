// - 'a = 1' makes 'a' only available as an int32
// - 'b = a' makes 'b' only available as an int32
// - The use of 'b' in 'b.length' gets copy-propped with 'a', and so it needs to unspecialize 'a'
(function () {
    var a = 1;
    for(var i = 0; i < 1; i++) {
        var b = a;
        b.length = 1;
        a = 1;
    }
})();
