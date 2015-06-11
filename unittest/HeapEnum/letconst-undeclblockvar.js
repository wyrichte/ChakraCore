var func;
function testScopeSlotArray()
{
    func = function() { return blah; };
    throw 1;
    let blah;
}

try { testScopeSlotArray(); } catch (e) { }
Debug.dumpHeap(0, true, true);


function testActivationObject()
{
    // Presence of eval in function below changes the closure from
    // ScopeSlotArray to ActivationObject
    func = function() { eval('blah'); return blah; };
    throw 1;
    let blah;
}

try { testActivationObject(); } catch (e) { }
Debug.dumpHeap(0, true, true);

let globalblah;
