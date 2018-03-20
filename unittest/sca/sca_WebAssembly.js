// SCA validation for SharedArrayBuffer

this.WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js");

function serialize(rootObject) {
  return SCA.serialize(rootObject, {context: "samethread"}, undefined);
}

function postMessage(rootObject) {
  var blob = serialize(rootObject);
  return SCA.deserialize(blob);
}

/*
(module
  (memory (import "ns" "mem") (shared 1 1))
  (func (export "store") (param i32 i32)
    (i32.atomic.store (get_local 0) (get_local 1))
  )
  (func (export "load") (param i32) (result i32)
    (i32.atomic.load (get_local 0))
  )
)
*/
const sharedBytes = [0x00,0x61,0x73,0x6d,0x01,0x00,0x00,0x00,0x01,0x0b,0x02,0x60,0x02,0x7f,0x7f,0x00,0x60,0x01,0x7f,0x01,0x7f,0x02,0x0c,0x01,0x02,0x6e,0x73,0x03,0x6d,0x65,0x6d,0x02,0x03,0x01,0x01,0x03,0x03,0x02,0x00,0x01,0x07,0x10,0x02,0x05,0x73,0x74,0x6f,0x72,0x65,0x00,0x00,0x04,0x6c,0x6f,0x61,0x64,0x00,0x01,0x0a,0x15,0x02,0x0a,0x00,0x20,0x00,0x20,0x01,0xfe,0x17,0x02,0x00,0x0b,0x08,0x00,0x20,0x00,0xfe,0x10,0x02,0x00,0x0b];
/*
(module
  (memory (import "ns" "mem") 1 1)
  (func (export "store") (param i32 i32)
    (i32.store (get_local 0) (get_local 1))
  )
  (func (export "load") (param i32) (result i32)
    (i32.load (get_local 0))
  )
)
*/
const nonSharedBytes = [0x00,0x61,0x73,0x6d,0x01,0x00,0x00,0x00,0x01,0x0b,0x02,0x60,0x02,0x7f,0x7f,0x00,0x60,0x01,0x7f,0x01,0x7f,0x02,0x0c,0x01,0x02,0x6e,0x73,0x03,0x6d,0x65,0x6d,0x02,0x01,0x01,0x01,0x03,0x03,0x02,0x00,0x01,0x07,0x10,0x02,0x05,0x73,0x74,0x6f,0x72,0x65,0x00,0x00,0x04,0x6c,0x6f,0x61,0x64,0x00,0x01,0x0a,0x13,0x02,0x09,0x00,0x20,0x00,0x20,0x01,0x36,0x02,0x00,0x0b,0x07,0x00,0x20,0x00,0x28,0x02,0x00,0x0b];

function makeBuffer(arr) {
  const buf = new ArrayBuffer(arr.length);
  const view = new Uint8Array(buf);
  for (let i = 0; i < arr.length; ++i) {
    view[i] = arr[i];
  }
  return buf;
}
const sharedBuf = makeBuffer(sharedBytes);
const nonSharedBuf = makeBuffer(nonSharedBytes);

var tests = [{
  name: "Validate that shared WebAssembly.Memory do share the internal buffer after SCA",
  body() {
    const mem1 = new WebAssembly.Memory({initial: 1, maximum: 1, shared: true});
    const mem2 = postMessage(mem1);
    const {exports: {store}} = new WebAssembly.Instance(new WebAssembly.Module(sharedBuf), {ns: {mem: mem1}});
    const {exports: {load}} = new WebAssembly.Instance(new WebAssembly.Module(sharedBuf), {ns: {mem: mem2}});
    const val = 0x12345678;
    store(0, val);
    assert.areEqual(val, load(0), "The value stored in mem1 should be available in mem2");
  }
}, {
  name: "Validate that unshared WebAssembly.Memory don't share the internal buffer after SCA",
  body() {
    const mem1 = new WebAssembly.Memory({initial: 1, maximum: 1, shared: false});
    const {exports: {store}} = new WebAssembly.Instance(new WebAssembly.Module(nonSharedBuf), {ns: {mem: mem1}});
    const mem2 = postMessage(mem1);
    store(0, 0x87654321);
    const {exports: {load}} = new WebAssembly.Instance(new WebAssembly.Module(nonSharedBuf), {ns: {mem: mem2}});
    assert.areEqual(0, load(0), "The new value stored in mem1 should not be available in mem2");
  }
}];

testRunner.runTests(tests, {
  verbose : WScript.Arguments[0] !== "summary"
});
