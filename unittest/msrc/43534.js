// stack->heap object
function inline() {
    return inline.arguments[0];
}

// code that will be jitted
function opt() {
    // Create a RegExp Object, so we can abuse the JavascriptRegExp::BoxStackInstance
    var obj = /ab+c/;
    // Add indexed value to create objectArray on RegExp
    obj[1] = 0xccc;

    // Initialize first auxSlot to tagged int
    obj.a=0x111;

    // This is where we box and have separate objects pointing to the same auxSlots
    heap_obj=inline(obj);

    // Set second auxSlot to keymap, from boxed instance
    wm.set(heap_obj,{});

    // Set (overwrite) second auxSlot to string
    obj["aaaa"]=0xaaaa;

    // Set third auxSlot to string
    obj["bbb"]=0xbbb;
}

// WeakMap modifies the auxSlot of its keys
var wm = new WeakMap();
// Cache boxed instance so that it can be used outside of opt
var heap_obj;

// JIT opt. JIT needed to force boxing from stack
for (var i = 0; i < 150; i++) {
    opt();
}

// The type confusion will be over the 2nd auxSlot-- expecting keymap but getting string
wm.set(heap_obj,0x123);

print("PASSED");