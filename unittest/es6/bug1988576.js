var parts = ['shoulder', 'knees'];
var allParts = ['head', ...parts, 'and', 'toes'];
if (JSON.stringify(allParts) === '["head","shoulder","knees","and","toes"]') {
    _$trace("Passed\n");
} else {
    _$trace("Failed\n");
}
 allParts[0]./**ml:constructor**/;
