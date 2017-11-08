function foo() {
    var arrProto = Array.prototype;
    delete arrProto.entries;
    var obj = CreateDomArrayObject();
    obj.AddObject(101);
    obj.AddObject(102);
    var iter = obj.entries();
}

foo();
print("Passed");


