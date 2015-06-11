function everyCallback1(value, index, array) {
    WScript.Echo("everyCallback1: this.p1=" + this.p1 + ", value=" + value + ", index=" + index + ", array=" + array);
    return true;
}

function everyCallback2(value, index, array) {
    WScript.Echo("everyCallback2: this.p1=" + this.p1 + ", value=" + value + ", index=" + index + ", array=" + array);
    return false;
}

function someCallback1(value, index, array) {
    WScript.Echo("someCallback1: this.p1=" + this.p1 + ", value=" + value + ", index=" + index + ", array=" + array);
    return true;
}

function someCallback2(value, index, array) {
    WScript.Echo("someCallback2: this.p1=" + this.p1 + ", value=" + value + ", index=" + index + ", array=" + array);
    return false;
}

function forEachCallback(value, index, array) {
    WScript.Echo("forEachCallback: this.p1=" + this.p1 + ", value=" + value + ", index=" + index + ", array=" + array);
}

function mapCallback(value, index, array) {
    WScript.Echo("mapCallback: this.p1=" + this.p1 + ", value=" + value + ", index=" + index + ", array=" + array);
    return value * 10;
}

function filterCallback(value, index, array) {
    WScript.Echo("filterCallback: this.p1=" + this.p1 + ", value=" + value + ", index=" + index + ", array=" + array);
    return (value % 2 == 1);
}

function reduceCallback(previousValue, currentValue, index, array) {
    WScript.Echo("reduceCallback: previousValue=" + previousValue + ", currentValue=" + currentValue + ", index=" + index + ", array=" + array);
    return previousValue + currentValue;
}

function reduceRightCallback(previousValue, currentValue, index, array) {
    WScript.Echo("reduceRightCallback: previousValue=" + previousValue + ", currentValue=" + currentValue + ", index=" + index + ", array=" + array);
    return previousValue + currentValue;
}
