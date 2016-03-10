print("pass");

WScript.LoadScript(`
  Object.defineProperty(this, "prop", { value: 1, configurable: false });
`);

print("pass");

try {
    WScript.LoadScript(`
      let prop = 2;
      print("Prop is " + prop);
    `);

    print("failed -- didn't throw exception");
} catch (e) {
    if (e.constructor === ReferenceError && e.message === "Let/Const redeclaration") {
        print("pass");
    } else {
        print("failed -- threw exception: " + e);
    }
}

print("pass");
