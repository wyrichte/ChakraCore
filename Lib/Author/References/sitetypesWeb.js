ActiveXObject = _$getTrackingUndefined(
function ActiveXObject(className, location) {
    /// <signature>
    ///   <param name="className" type="String" />
    ///   <param name="location" type="String" />
    /// </signature>
    if (typeof _$getActiveXObject === 'function')
        return _$getActiveXObject(className, location);
});

function VBArray(safeArray) {
    /// <signature>
    ///   <param name="safeArray" type"VBArray" />
    /// </signature>
}
VBArray.prototype.getItem = function () {
    /// <signature>
    ///   <param name="dimension1" type="Number" />
    ///   <param name="[dimension2]" type="Number" />
    ///   <param name="..." />
    ///   <returns type="Object" />
    /// </signature>
    return {};
};
VBArray.prototype.lbound = function () {
    /// <signature>
    ///   <param name="[dimension]" type="Number" />
    ///   <return type="Number" />
    /// </signature>
};
VBArray.prototype.ubound = function () {
    /// <signature>
    ///   <param name="[dimension]" type="Number" />
    ///   <return type="Number" />
    /// </signature>
};
VBArray.prototype.toArray = function () {
    /// <signature>
    ///   <return type="Array" />
    /// </signature>
};

Debug = {}

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

function Enumerator(collection) {
    /// <signature>
    ///   <param name="collection" />
    /// </signature>
    /// <signature>
    /// </signature>
}

Enumerator.prototype.atEnd = function () {
    /// <signature>
    ///   <returns type="Boolean" />
    /// </signature>
    return true;
};

Enumerator.prototype.item = function () {
    /// <signature>
    ///   <returns type="Object" />
    /// </signature>
    return {};
};

Enumerator.prototype.moveFirst = function () {
    /// <signature>
    /// </signature>
};

Enumerator.prototype.moveNext = function () {
    /// <signature>
    /// </signature>
};
