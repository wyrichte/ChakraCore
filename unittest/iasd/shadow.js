var a = Debug.createTypedObject(2000, "mytype", 32, true); // use default operators

Object.defineProperty(a.__proto__, 'z', {value: 777});
Object.defineProperty(a, 'z', {value: 123});

WScript.Echo(a.hasOwnProperty('z'));
WScript.Echo(a.z);