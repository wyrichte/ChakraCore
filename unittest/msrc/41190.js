function foo() {
    var str = 'sssssssssssssss';
    function bar() {
       str = ('sssssssssssssss' + str) + str;
    }
    for (var i = 0; i < 30; i++) {
        bar();
        print(str.length);
    }
    print(str);
}
try
{
    foo();
}
catch(e){}
