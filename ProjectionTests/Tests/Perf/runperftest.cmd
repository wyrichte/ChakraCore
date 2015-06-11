echo off
del merged.js
type projectionsGlue.js >> merged.js
type perfplugin.js >> merged.js
type tests.js >> merged.js
echo runner.run^(^) >> merged.js
jshost -forcenative merged.js
