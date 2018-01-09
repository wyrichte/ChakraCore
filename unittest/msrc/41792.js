try {
  eval('import("");');
  console.log('FAILED');
} catch(e) {
  console.log('PASSED');
}

try {
  eval('var a = import("");');
  console.log('FAILED');
} catch(e) {
  console.log('PASSED');
}
