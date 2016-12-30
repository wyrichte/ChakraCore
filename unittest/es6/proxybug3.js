//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

var obj0 = {};
var obj1 = {};
obj0.method1 = function () {
  var sc0 = WScript.LoadScript('', 'samethread');
  sc0.obj = obj1;
  sc0.Debug.parseFunction('Object.keys(obj)')();
};
obj1.needMarshal = {};
obj1 = new Proxy(obj1, {});
({ attr : (obj0.method1()) });

print("passed");

