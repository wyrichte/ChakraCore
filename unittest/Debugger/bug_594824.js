// Validating bug 594824, where the eval happens after shutdown

eval('');/**bp:evaluate('WScript.Shutdown()');**/