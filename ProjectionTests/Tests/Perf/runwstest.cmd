echo off
del merged.js
type projectionsGlue.js >> merged.js
type wsPlugin.js >> merged.js
type wsTests.js >> merged.js
echo runner.run^(^) >> merged.js
jshost merged.js