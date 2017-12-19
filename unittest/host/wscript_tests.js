function throws(fn) {
  try {
    fn();
    console.log(`Failed: Expected an error from ${fn}`);
  } catch (e) {
    console.log(`${fn}: ${e}`);
  }
}

throws(() => WScript.LoadModule(Symbol()));
throws(() => WScript.LoadModule({toString() {throw new Error("Exception when calling toString");}}));
throws(() => WScript.LoadModule());
throws(() => WScript.LoadScript(Symbol()));
throws(() => WScript.LoadScript({toString() {throw new Error("Exception when calling toString");}}));
throws(() => WScript.LoadScript());
throws(() => WScript.LoadScriptFile(Symbol()));
throws(() => WScript.LoadScriptFile({toString() {throw new Error("Exception when calling toString");}}));
throws(() => WScript.LoadScriptFile());
