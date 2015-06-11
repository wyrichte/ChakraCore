//
// Win8 867346: Bad continue wasn't detected by deferred parsing.
//

function f() {
    LABEL1:
    switch(0) {
        case 0:
            continue LABEL1;
    }
}

