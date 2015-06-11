var harnessInitStart = new Date();
// String extensions
String.format = function (formatString) {
    for (var i = 1; i < arguments.length; i++) {
        formatString = formatString.replace(new RegExp("\\{" + (i - 1) + "\\}", "g"), arguments[i]);
    }
    return formatString;
};
String.formatGivenArray = function (formatString, arr) {
    for (var i = 0; i < arr.length; i++) {
        formatString = formatString.replace(new RegExp("\\{" + i + "\\}", "g"), arr[i]);
    }

    return formatString;
};
String.prototype.format = function () {
    var formatString = this;
    for (var i = 0; i < arguments.length; i++) {
        formatString = formatString.replace(new RegExp("\\{" + (i) + "\\}", "g"), arguments[i]);
    }
    return formatString;
};
String.prototype.formatGivenArray = function (arr) {
    return String.formatGivenArray(this, arr);
};
String.removeDuplicateSpaces = function (str) {
    var seenSpace = true;//trim first space too
    var subArray = new Array(str.length);
    for (var i = 0, j = 0; i < str.length; i++) {
        var c = str.charAt(i);
        if (c === ' ') {
            if (!seenSpace) {
                subArray[j++] = c;
            }
            seenSpace = true;
        }
        else {
            subArray[j++] = c;
            seenSpace = false;
        }
    }
    return subArray.join('');
}
String.prototype.contains = function (value) {
    return this.indexOf(value) !== -1;
}
String.prototype.toFirstUpper = function () {
    return "{0}{1}".format(this.charAt(0).toUpperCase(), this.substring(1));
}

//Array extensions
Array.prototype.map = function (func, inPlace) {
    var toReturn = inPlace ? this : new Array(this.length);
    for (var i = 0; i < this.length; i++) {
        toReturn[i] = func(this[i]);
    }
    return toReturn;
};

Array.prototype.forEach = function (func, thisArg) {
    for (var i = 0; i < this.length; i++) {
        func.call(thisArg, this[i]);
    }
};

Array.prototype.contains = function (item) {
    for (var i = 0; i < this.length; i++) {
        if (this[i] === item) {
            return true;
        }
    }
    return false;
}

//Date Extensions
Date.prototype.toFileNameString = function () {
    return "{0}{1}{2}_{3}{4}{5}_{6}".format(this.getYear(), this.getMonth(),
        this.getDay(), this.getHours() < 10 ? "0" + this.getHours() : this.getHours(),
        this.getMinutes() < 10 ? "0" + this.getMinutes() : this.getMinutes(),
        this.getSeconds() < 10 ? "0" + this.getSeconds() : this.getSeconds(),
        this.getMilliseconds());
};