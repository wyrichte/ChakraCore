function testCopyOnWriteObjects() {
	function getP2(o, echo) {
		var v = o.p2;
		if (echo === undefined || echo)
			WScript.Echo(v);
	}

	function getP3(o, echo) {
		var v = o.p3;
		if (echo === undefined || echo)
			WScript.Echo(v);
	}
	
	WScript.Echo("testCopyOnWriteObjects():");
	
	var numberOfObjects = 4;

	// Create a few regular objects with multiple properties.  
	var regularObjects = new Array(numberOfObjects);
	for (var i = 0; i < numberOfObjects; i++) {
		regularObjects[i] = { p0: 10, p1: 11, p2: 12 };
	}

	// Create copy-on-write objects proxying the regular objects.
	var copyOnWriteObjects = new Array(numberOfObjects);
	for (var i = 0; i < numberOfObjects; i++) {
		copyOnWriteObjects[i] = WScript.CopyOnWrite(regularObjects[i]);
	}

	WScript.Echo("Inline cache:");
	
	WScript.Echo("attached copy-on-write objects:");	
	
	// Read the same property from the copy-on-write objects while attached.
	for (var i = 0; i < numberOfObjects; i++) {
		getP2(copyOnWriteObjects[i]);
	}

	// Set a property on the copy-on-write objects to force them to detach.
	// We should not cache this type transition because detaching creates additional
	// properties on the detached copy-on-write object.
	for (var i = 0; i < numberOfObjects; i++) {
		copyOnWriteObjects[i].p3 = 33;
	}

	// Update a property on the proxied objects.
	for (var i = 0; i < numberOfObjects; i++) {
		regularObjects[i].p2 = -12;
	}

	WScript.Echo("detached copy-on-write objects:");	
	
	// Read the same property from the copy-on-write objects to see if we got the value
	// from the detached object or from the proxied one.
	for (var i = 0; i < numberOfObjects; i++) {
		getP2(copyOnWriteObjects[i]);
	}

	// Read the new property from the copy-on-write objects.
	for (var i = 0; i < numberOfObjects; i++) {
		getP3(copyOnWriteObjects[i]);
	}	
	
	WScript.Echo();
}


function testObjectsWithRegularPrototypes() {
	// TODO: Create and use copy-on-write objects that use regular prototypes.
}


function testObjectsWithCopyOnWritePrototypes() {
	function getP2(o, echo) {
		var v = o.p2;
		if (echo === undefined || echo)
			WScript.Echo(v);
	}

	function getP3(o, echo) {
		var v = o.p3;
		if (echo === undefined || echo)
			WScript.Echo(v);
	}
	
	WScript.Echo("testCopyOnWriteObjectsWithCopyOnWritePrototypes():");
	
	var numberOfObjects = 8;
	
	var regularPrototype = { p2: 2 };
	var attachedCopyOnWritePrototype = WScript.CopyOnWrite(regularPrototype);
	var detachedCopyOnWritePrototype = WScript.CopyOnWrite(regularPrototype);

	// Create a few regular objects with multiple properties.  
	var regularObjects = new Array(numberOfObjects);
	for (var i = 0; i < 4; i++) {
		var ro = Object.create(attachedCopyOnWritePrototype);
		ro.p0 = 10;
		ro.p1 = 11;
		regularObjects[i] = ro;
	}
	regularObjects[2].p2 = 12;
	regularObjects[3].p2 = 12;
	
	for (var i = 4; i < 8; i++) {
		var ro = Object.create(detachedCopyOnWritePrototype);
		ro.p0 = 10;
		ro.p1 = 11;
		regularObjects[i] = ro;
	}
	regularObjects[6].p2 = 12;
	regularObjects[7].p2 = 12;

	// Force the copy-on-write prototype to detach.
	detachedCopyOnWritePrototype.p3 = 23;

	// Create copy-on-write objects proxying the regular objects.
	var copyOnWriteObjects = new Array(numberOfObjects);
	for (var i = 0; i < numberOfObjects; i++) {
		copyOnWriteObjects[i] = WScript.CopyOnWrite(regularObjects[i]);
	}

	WScript.Echo("Inline cache:");

	WScript.Echo("attached copy-on-write objects:");

	// Read a property from the copy-on-write objects while attached.
	for (var i = 0; i < numberOfObjects; i++) {
		getP2(copyOnWriteObjects[i]);
	}

	// Set a property on the copy-on-write objects to force them to detach.
	// We should not cache this type transition because detaching creates additional
	// properties on the detached copy-on-write object.
	for (var i = 0; i < numberOfObjects; i++) {
		copyOnWriteObjects[i].p3 = 33;
	}

	// Update a property on the proxied objects.
	for (var i = 0; i < numberOfObjects; i++) {
		regularObjects[i].p2 = -12;
	}

	WScript.Echo("detached copy-on-write objects:");
	
	// Read the same property from the copy-on-write objects to see if we got the value
	// from the detached object or from the proxied one.
	for (var i = 0; i < numberOfObjects; i++) {
		getP2(copyOnWriteObjects[i]);
	}

	// Read the new property from the copy-on-write objects.
	for (var i = 0; i < numberOfObjects; i++) {
		getP3(copyOnWriteObjects[i]);
	}	
	
	WScript.Echo();
}


function testInlineAndTypePropertyCaches() {
	function access(o, echo) {
		var v = o.p;
		if(echo === undefined || echo)
			WScript.Echo(v);
	}

	WScript.Echo("testInlineAndTypePropertyCaches():");

	// Create two objects using one prototype object, where one object hides a property from the prototype
	var proto = { p: 0 };
	var o = [];
	for(var i = 0; i < 2; ++i)
		o.push(Object.create(proto));
	o[1].p = 1;

	// Create copy-on-write clones of the above
	var protoCopy = WScript.CopyOnWrite(proto);
	var oc = [];
	for(var i = 0; i < o.length; ++i)
		oc.push(WScript.CopyOnWrite(o[i]));

	// Force the copy-on-write prototype to detach
	protoCopy.q = 0;

	// Test inline cache usage in accessing 'p' on the copy-on-write objects
	WScript.Echo("Inline cache:");
	for(var i = 0; i < oc.length; ++i)
		access(oc[i]);
	WScript.Echo();

	// Test type property cache usage in accessing 'p' on the copy-on-write objects
	WScript.Echo("Type property cache:");
	var objectWithSomeOtherType = { r: 0 };
	for(var i = 0; i < oc.length; ++i) {
		access(objectWithSomeOtherType, false); // reset the inline cache with some other type so that the type property cache is created/used
		access(oc[i]);
	}
	WScript.Echo();
}


testCopyOnWriteObjects();
testObjectsWithRegularPrototypes();
testObjectsWithCopyOnWritePrototypes();
testInlineAndTypePropertyCaches();
