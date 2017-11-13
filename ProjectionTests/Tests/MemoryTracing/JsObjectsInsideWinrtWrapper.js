if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
Loader42_FileName = 'Javascript Object wrapped to pass on as Winrt Objects';

function getBackArrayAsVector() {
    logger.comment("Get Same Vector");
    var myVector = (new Animals.Animal(1)).sendBackSameVector([1, 2, 3, 4]);

    return myVector;
}

function createDuplicateVector() {
    logger.comment("Duplicate Array as Vector");
    var myVector = (new Animals.Animal(1)).duplicateVector([1, 2, 3, 4]);

    return myVector;
}

function testIterableIteratorOnSameArrayAsVector() {
    var myVector = getBackArrayAsVector();

    logger.comment("Perform gc");
    CollectGarbage();
    logger.comment("GC complete");
    
    logger.comment("Call iterable's first method");
    var first = myVector.first();
    for (var index = 1; first.hasCurrent; index++) {
        verify(first.current, index, "first.current at (" + index + ")");
        first.moveNext();
    }
}

function getBackArrayAsVectorView() {
    logger.comment("Get Same VectorView");
    var myVectorView = (new Animals.Animal(1)).sendBackSameVectorView([1, 2, 3, 4]);

    return myVectorView;
}

function createDuplicateVectorView() {
    logger.comment("Duplicate Array as VectorView");
    var myVectorView = (new Animals.Animal(1)).duplicateVectorView([1, 2, 3, 4]);

    return myVectorView;
}

function getBackArrayAsIterable() {
    logger.comment("Get Same Iterable");
    var myIterable = (new Animals.Animal(1)).sendBackSameIterable([1, 2, 3, 4]);
    return myIterable;
}

function createDuplicateIterable() {
    logger.comment("Duplicate Array as Iterable");
    var myIterable = (new Animals.Animal(1)).duplicateIterable([1, 2, 3, 4]);

    return myIterable;
}

function testIterableIteratorOnSameArrayAsIterable() {
    var myIterable = getBackArrayAsIterable();

    logger.comment("Perform gc");
    CollectGarbage();
    logger.comment("GC complete");

    logger.comment("Call iterable's first method");
    var first = myIterable.first();
    for (var index = 1; first.hasCurrent; index++) {
        verify(first.current, index, "first.current at (" + index + ")");
        first.moveNext();
    }
}

function getBackArrayAsIterator() {
    logger.comment("Get Same Iterator");
    var myIterator = (new Animals.Animal(1)).sendBackSameIterator([1, 2, 3, 4]);
    return myIterator;
}

function createDuplicateIterator() {
    logger.comment("Duplicate Array as Iterator");
    var myIterator = (new Animals.Animal(1)).duplicateIterator([1, 2, 3, 4]);
    return myIterator;
}

function passDelegateAsInParam() {
    var delegateCalled = 0;
    function callback(animal) {
        logger.comment("*** Delegate called");
        var animalNames = animal.getNames();
        delegateCalled++;
        return animalNames.common;
    }

    logger.comment("Pass the function as delegate and then invoke it from ABI");
    var myAnimalDelegateStringOut = (new Animals.Animal(1)).callDelegateWithOutParam_HSTRING(callback);
    verify(delegateCalled, 1, "Delegate Called");
    verify(myAnimalDelegateStringOut, 'Wolverine', "(new Animals.Animal(1)).callDelegateWithOutParam_HSTRING(callback)");
}

function getBackSameDelegate() {
    var delegateCalled = 0;
    function callback(animal) {
        logger.comment("*** Delegate called");
        var animalNames = animal.getNames();
        delegateCalled++;
        return animalNames.common;
    }

    logger.comment("Pass the function as delegate and get it back as the delegate");
    var myAnimal = new Animals.Animal(1);
    var myNewCallback = myAnimal.methodDelegateAsOutParam(callback);
    verify(delegateCalled, 1, "Delegate Called");
    verify(myNewCallback, callback, "(new Animals.Animal(1)).methodDelegateAsOutParam(callback)");

    logger.comment("Verify the callback can be called");
    var myAnimalDelegateStringOut = myNewCallback(myAnimal);
    verify(delegateCalled, 2, "Delegate Called");
    verify(myAnimalDelegateStringOut, 'Wolverine', "myNewCallback(myAnimal)");
}

function referenceAsInParam() {
    var propertyValueTests = new Animals.PropertyValueTests();

    var myDimensions = {
        length: 10,
        width: 40
    };
    var outVar = propertyValueTests.testDimensions_ReferenceIn(myDimensions);
    verify(outVar.isNull, false, 'testDimensions_ReferenceIn(myDimensions).isNull');
    verify(outVar.isValidType, true, 'testDimensions_ReferenceIn(myDimensions).isValidType');
    verify(outVar.outValue.length, myDimensions.length, 'testDimensions_ReferenceIn(myDimensions).outValue.length');
    verify(outVar.outValue.width, myDimensions.width, 'testDimensions_ReferenceIn(myDimensions).outValue.width');
}

function referenceAsOutParam() {
    var propertyValueTests = new Animals.PropertyValueTests();

    var myDimensions = {
        length: 10,
        width: 40
    };
    var outVar = propertyValueTests.testDimensions_ReferenceOut(myDimensions);
    verify(outVar.length, myDimensions.length, 'testDimensions_ReferenceOut(myDimensions).length');
    verify(outVar.width, myDimensions.width, 'testDimensions_ReferenceOut(myDimensions).width');
}

function referenceArrayAsInParam() {
    var propertyValueTests = new Animals.PropertyValueTests();
    var myArray = propertyValueTests.receiveStructArray();
    var outVar = propertyValueTests.testDimensionsArray_IPropertyValueIn(myArray);
    verify(outVar.isValidType, true, 'testDimensionsArray_IPropertyValueIn(myArray).isValidType');
    verify(outVar.outValue.length, myArray.length, 'testDimensionsArray_IPropertyValueIn(myArray).outValue.length');
    for (var i = 0; i < myArray.length; i++) {
        verify(outVar.outValue[i].length, myArray[i].length, 'testDimensionsArray_IPropertyValueIn(myArray).outValue[' + i + '].length');
        verify(outVar.outValue[i].width, myArray[i].width, 'testDimensionsArray_IPropertyValueIn(myArray).outValue[' + i + '].width');
    }
}

function referenceArrayAsOutParam() {
    var propertyValueTests = new Animals.PropertyValueTests();
    var myArray = propertyValueTests.receiveStructArray();
    var outVar = propertyValueTests.testInspectable_IPropertyValueOut(myArray);
    verify(outVar.length, myArray.length, 'testInspectable_IPropertyValueOut(myArray).length');
    for (var i = 0; i < myArray.length; i++) {
        verify(outVar[i].length, myArray[i].length, 'testInspectable_IPropertyValueOut(myArray)[' + i + '].length');
        verify(outVar[i].width, myArray[i].width, 'testInspectable_IPropertyValueOut(myArray)[' + i + '].width');
    }
}

runner.addTest({
    id: 1,
    desc: 'Array back in JS as IVector',
    pri: '0',
    test: function () {
        getBackArrayAsVector();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 2,
    desc: 'Array back in JS wrapped inside native IVector',
    pri: '0',
    test: function () {
        createDuplicateVector();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 3,
    desc: 'Array back in JS as IVector and then get the iterable and iterator',
    pri: '0',
    test: function () {
        testIterableIteratorOnSameArrayAsVector();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 4,
    desc: 'Array back in JS as IVectorView',
    pri: '0',
    test: function () {
        getBackArrayAsVectorView();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 5,
    desc: 'Array back in JS wrapped inside native IVectorView',
    pri: '0',
    test: function () {
        createDuplicateVectorView();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 6,
    desc: 'Array back in JS as IIterable',
    pri: '0',
    test: function () {
        getBackArrayAsIterable();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 7,
    desc: 'Array back in JS wrapped inside native IIterable',
    pri: '0',
    test: function () {
        createDuplicateIterable();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 8,
    desc: 'Array back in JS as IIteable and then get the iterator',
    pri: '0',
    test: function () {
        testIterableIteratorOnSameArrayAsIterable();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 9,
    desc: 'Array back in JS as IIterator',
    pri: '0',
    test: function () {
        getBackArrayAsIterator();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 10,
    desc: 'Array back in JS wrapped inside native IIterator',
    pri: '0',
    test: function () {
        createDuplicateIterator();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 11,
    desc: 'Delegate passed in',
    pri: '0',
    test: function () {
        passDelegateAsInParam();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 12,
    desc: 'Get Back same delegate',
    pri: '0',
    test: function () {
        getBackSameDelegate();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 13,
    desc: 'IReference as in parameter',
    pri: '0',
    test: function () {
        referenceAsInParam();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 14,
    desc: 'IReference as out parameter',
    pri: '0',
    test: function () {
        referenceAsOutParam();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 15,
    desc: 'IReferenceArray as in parameter',
    pri: '0',
    test: function () {
        referenceArrayAsInParam();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 16,
    desc: 'IReferenceArray as out parameter',
    pri: '0',
    test: function () {
        referenceArrayAsOutParam();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
