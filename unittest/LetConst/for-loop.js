function print(x) { WScript.Echo(x) }

let a = 'global';

for (let a = 'for', f = function() { a += ' loop' }; ; ) {
    f();
    print(a);
    break;
}
print(a);

for (let a in this) {
    let f = function() { a = 'for-in loop'; };
    f();
    print(a);
    break;
}
print(a);

try { eval('for (let a = 123 in this) { }'); print('fail'); } catch (e) { print(e); }
try { eval('for (const a = 123 in this) { }'); print('fail'); } catch (e) { print(e); }
try { eval('function foo() { for (let a = 123 in this) { } } foo();'); print('fail'); } catch (e) { print(e); }
try { eval('function foo() { for (const a = 123 in this) { } } foo();'); print('fail'); } catch (e) { print(e); }
try { eval('function foo() { { for (var a = 123 in []) { } let a; } } foo();'); print('fail'); } catch (e) { print(e); }

function test3() {
    eval('');

    v2;
    let v1;
    for (let v2; false;) {
        // this var should get a slot before v1 via EnsureScopeSlot
        // to ensure that the slot order is the same as the SlotArray
        var v2 = 0;
    }
}
test3();
