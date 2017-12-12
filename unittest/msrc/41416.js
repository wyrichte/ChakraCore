// First regex should succeed, as number of capturing groups is less than 2^15.
// We create 2^15 - 2 capturing groups, because the entire regex always counts
// as a capturing group.
var re = new RegExp('(.)'.repeat(0x7ffe));
print('haw'.replace(re, function (firstMatch) { return firstMatch; }));

// Number of capturing groups is greater than limit, so expect a fail-fast.
print("Expecting fail fast");
re = new RegExp('(.*)'.repeat(0xfffc));
print("Didn't get fail-fast");
