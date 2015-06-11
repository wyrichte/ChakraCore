ActiveXObject = _$getTrackingUndefined(
function ActiveXObject(className, location) {
    /// <signature>
    ///   <param name="className" type="String" />
    ///   <param name="location" type="String" />
    /// </signature>
    if (typeof _$getActiveXObject === 'function')
        return _$getActiveXObject(className, location);
});

Debug = {};

Debug.write = function (value) {
    /// <signature>
    ///   <param name="value" type="String" />
    ///   <param name="..." />
    /// </signature>
};

Debug.writeln = function (value) {
    /// <signature>
    ///   <param name="value" type="String" />
    ///   <param name="..." />
    /// </signature>
};