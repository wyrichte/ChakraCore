var /**target:OBJ**/obj = {
    a : 10
};

// Creation of the gf1's instance should happen implicitly and call next on it to execute the body
function* /**target:GF1**/gf1() {
    try {
        obj./**ml:a**/a = 20;
        yield /**GD:OBJ**/obj./**ml:a**/a;
        obj = { b : 30 };
        yield obj./**ml:!a,b**/b;
    } finally {
        obj = { c : 40 };
        yield obj./**ml:!a,!b,c**/c;
    }
}

// For yield* the next() call should happen implicitly on the iterator
function* gf2() {
    yield* /**GD:GF1**/gf1();
    yield obj./**ml:!a,!b,c**/c;
}

// Scenario in which the generator function is a member function with this reference
var gfObj = {
    a : 10,
    gf : function* () {
        obj = { a : 50 };
        yield this./**ml:a**/a + obj./**ml:a**/a;
        obj = { b : 60 };
        yield this./**ml:a**/a + obj./**ml:!a,b**/b;
    }
}

// This scenario requires a new called on func1 implicitly to execute the generator function
function func1() {
    this.a = 10;
    this.gf = function* () {
        obj = { a : 70 };
        yield this./**ml:a**/a + obj./**ml:a**/a;
        obj = { b : 80 };
        yield this./**ml:a**/a + obj./**ml:!a,b**/b;
    }
}

// Here new is called on the generator
function* func2() {
    this.a = 10;
    this.gf = function () {
        obj = { a : 90 };
        yield this./**ml:a**/a + obj./**ml:a**/a;
        obj = { b : 80 };
        yield this./**ml:a**/a + obj./**ml:!a,b**/b;
    };
    obj./**ml:a**/a;
}
/**gs:***/