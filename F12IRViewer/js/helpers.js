//
// Copyright (C) Microsoft. All rights reserved.
//

/**
 * A simple formatter routine designed to mimic C# style String.Format().
 *
 * @param A list of arguments to be passed to the formatter.
 * @return The string, formatted as in C#.
 */
if (typeof String.prototype.format === "undefined") {
  String.prototype.format = function() {
    // use the implicit "arguments" parameter to get parameters at the given positions.
    var fmt = String(this);
    for (var i = 0; i < arguments.length; i++) {
      var formatter = '{'+i+'}';
      while (fmt.indexOf(formatter) !== -1) {
        fmt = fmt.replace(formatter, arguments[i]);
      }
    }
    return fmt;
  };
}

/**
 * Take a collection of strings and set them as properties on
 * a JS object with the value corresponding to their position in the
 * collection.
 *
 * @param values The collection of strings to make into Enum values.
 * @return An Enum for which values can be accessed as "EnumName.Value"
 */
function Enum(values) {
  var e = {};
  for (var i = 0; i < values.length; i++) {
    var name = values[i];
    var value = i;
    e[name] = value;
  }
  return e;
}
