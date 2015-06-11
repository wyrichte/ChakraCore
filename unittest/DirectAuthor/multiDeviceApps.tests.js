/**ref:..\\..\\Lib\\Author\\References\\domWeb.js**/
/**ref:multiDeviceApps.js**/

// TEST: Make sure we're not polluting the global window object without members that shouldn't be there
//window.

//////////// Compass tests
// TEST
//window.navigator.compass.
window.navigator.compass.clearWatch(/*TEST*/);
window.navigator.compass.getCurrentHeading(/*TEST*/);
window.navigator.compass.getCurrentHeading(
    function (compassHeading) {
        // TEST
        compassHeading./**ml:trueHeading**/