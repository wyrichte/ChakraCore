obj = {}
obj.a = 13.37;
obj.b = 1;
function main0(o) { 
    var l3 = function() {
        o.b = o.b;
        o.e = 0x41414141; // [[ 1 ]]
    };
    o.a = "HELLO";
    for (var i = 0; i < 100; i++) {
        l3(); 
        o.a = obj;
    }
}
for (var i = 0; i < 160; i++) {
    main0({a:1.1, b:2.2, c:3.3});
} 
main0({a:1.1, b:12.2, c:0,d:3.3,}); 

obj.a += 12.26;

WScript.Echo('pass');
