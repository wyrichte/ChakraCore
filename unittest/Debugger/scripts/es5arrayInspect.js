function clickme()
{
    // Normal array
    var arr1 = [11, 88, 2];
    arr1[15] = "15";
    arr1.x = 30;

    // ES5 array
    var arr2 = [8, 5];

    Object.defineProperty(arr2, "7", { value: 37,
        enumerable: true
    });

    arr2.bValue = 0;
    Object.defineProperty(arr2, "11",
            { get: function () { return 99; },
                set: function (newValue) { arr2.bValue = newValue; },
                enumerable: true
            }
            );

    arr2[11] = 50;

    arr2.x = "x";
    return 10;

}

clickme();

