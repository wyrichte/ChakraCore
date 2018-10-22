let nSegments = 100;

let arrayUaf = [{}];
for (var i = 1; i < nSegments; i++) {
    arrayUaf[i * 1000] = i;
}

function rebuildSegmentMap() {
    arrayUaf[(nSegments - 2) * 1000] = 1;
}

Object.defineProperty(Array, Symbol.species, { get: function() { rebuildSegmentMap(); return Array; } });

let segmentSize1 = 17;
parms = new Array();
parms.push(segmentSize1);
parms.push(1000 * nSegments + segmentSize1 + 1);
for (i = 0; i < 2001; i++) {
    parms.push(i);
}
Array.prototype.splice.apply(arrayUaf, parms);

if (arrayUaf[2000].toString() == "1983") {
    print('pass');
} else {
    print('fail');
}
