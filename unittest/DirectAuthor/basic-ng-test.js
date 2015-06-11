/**ref:..\\..\\Lib\\Author\\References\\domWeb.js**/
/**ref:angular.js**/
/**ref:angular.intellisense.js**/

(function (angular) {
    // Create a test module.
    var testApp = angular.module('tests', ['ng'], ['$logProvider', function (logProvider) {
        // TEST: Providers can be injected into module config functions in module declaration.
        logProvider./**ml:$get**/;
    }]).config(['$locationProvider', function ($locationProvider) {
        $locationProvider.html5Mode = true;
    }]).factory('chainedFactory', function ($location) {
        // TEST: Components can be injected into chained provider functions.
        // TEST: Components can be injected by function parameter name.
        $location./**ml:absUrl,url**/
    });

    testApp.constant('testConstant', { foo: 1, bar: 2 });

    // Create a test provider.
})(angular);
