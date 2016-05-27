import { modF2, modVar2} from 'module_2.js';

function modF1() {
    modF2();
}
export var modVar1 = modVar2; /**bp:stack();**/
modF1();
