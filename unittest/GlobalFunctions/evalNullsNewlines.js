var output = "";

function write(str) {
    if (typeof (WScript) == "undefined") {
        output += str + "\n";
        document.getElementById("writer").innerText = output; // .replace("\0", '\\0');
    } else {
        WScript.Echo(str);
    }
}

write("--- 1 ---");                                                                   // CHROME          IE8
try { write(eval('1+//\0\n1')); } catch (e) { write(e); }                             // 2               !
try { write(eval('"a\0b"').length); } catch (e) { write(e); }                         // 3               !
try { write(eval('\'a\0b\'').length); } catch (e) { write(e); }                       // 3               !
try { write(eval('\0 = 1')); } catch (e) { write(e); }                                // !               undefined
try { write(eval('/*\0*/1')); } catch (e) { write(e); }                               // 1               !
try { write(eval('1//\0')); } catch (e) { write(e); }                                 // 1               1
