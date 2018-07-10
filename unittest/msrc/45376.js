let x = 1;

this.x = 0x1234;  // IsShadowed

// Convert to BigDictionaryTypeHandler, CopyFrom will be used in the process.
for (let i = 0; i < 0x10000; i++) {
    this['a' + i] = 1;
}

// Set IsAccessor
this.__defineSetter__('x', () => {});

// Type confusion
this.x;

print("PASS");
