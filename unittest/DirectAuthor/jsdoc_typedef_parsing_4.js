/**
 * SampleOptions are the options for the sampleFunction
 * @typedef {Object} SampleOptions
 * @property {String} name  - name 
 * to print
 * can span multiple line
 * @property {Number} count times 
 * to print
 * can span multiple line too
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