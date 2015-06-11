/**
 * SampleOptions are the options for the sampleFunction
 * @typedef {Object} SampleOptions
 * @property {String} name  - name to print
 * @param {String} some parameter - should simply be ignored
 * @property {Number} count times to print.
 */
var x = 0;


/**
 * @returns {SampleOptions} options
 */
function sampleFunction(options)
{
    for (var i = 0; i < options.count; i++)
    {
        WScript.Echo(options.name);
    }
}
/**ml:-**/