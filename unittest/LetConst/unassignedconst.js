function print(x) { WScript.Echo(x); }

try { eval('const x;'); } catch (e) { print(e); }
try { eval('function a() { const x; }'); } catch (e) { print(e); }
try { WScript.LoadScriptFile('unassignedconst_noneval_global.js'); } catch (e) { }
try { WScript.LoadScriptFile('unassignedconst_noneval_function.js'); } catch (e) { }
