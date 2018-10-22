
try {
function opt(str) {
  for (let i = 0; i < 2000; i++) {
    let tmp = str.charCodeAt('AAAAAAAAAA' + str + 'BBBBBBBBBB');
  }
}

opt('x');
opt(0x1234);
}
catch(ex) {
print("Passed");
}
