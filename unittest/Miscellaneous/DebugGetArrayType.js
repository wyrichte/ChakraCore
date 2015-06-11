function write(args) {
    WScript.Echo(args);
}


var arr1 = [];
write(Debug.getArrayType(arr1));

arr1[arr1.length] = 1;
write(Debug.getArrayType(arr1));

arr1.push(1.5);
write(Debug.getArrayType(arr1));

arr1.push("string_value");
write(Debug.getArrayType(arr1));


var arr2 = new Int32Array(3);
arr2[0] = 5;
write(Debug.getArrayType(arr2));

var arr3 = [1, 2, 3, 4, 5];
Object.defineProperty(arr3, "0", { value: 5 });
write(Debug.getArrayType(arr3));


write(Debug.getArrayType({}));
write(Debug.getArrayType({ prop: "string_value" }));
write(Debug.getArrayType(null));
write(Debug.getArrayType(5));
write(Debug.getArrayType("stringvalue"));