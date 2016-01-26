/*
  Copyright (C) 2013

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
WScript.LoadScriptFile("SIMDTestUtil.js");

var skipValueTests = false; //Fails if using pollyfill
var currentName = '<global>';
var numFails = 0;

function test(name, func) {
    
  if(name.indexOf(bool32x4.name) !== 0) return;
  if(name.indexOf('Bool32x4 value semantics') > -1) return; //unsupported function
  
  
  /*
  if(name.indexOf('Bool32x4 operators') > -1) return; //unsupported function
  if(name.indexOf('Bool32x4 allTrue') > -1) return; //unsupported function
  if(name.indexOf('Bool32x4 anyTrue') > -1) return; //unsupported function
  if(name.indexOf('Bool32x4 not') > -1) return; //unsupported function
  if(name.indexOf('Bool32x4 xor') > -1) return; //unsupported function
  if(name.indexOf('Bool32x4 or') > -1) return; //unsupported function
  if(name.indexOf('Bool32x4 and') > -1) return; //unsupported function
  if(name.indexOf('Bool32x4 replaceLane') > -1) return; //unsupported function
  if(name.indexOf('Bool32x4 check') > -1) return; //unsupported function
  if(name.indexOf('Bool32x4 constructor') > -1) return; //unsupported function
  */
    
  if (skipValueTests && name.indexOf("value semantics") != -1) return;

  currentName = name;
  WScript.Echo('\n******* ' + name + ' *******')
  try {
    func();
    WScript.Echo('passed')
  } catch (e) {
	if (e.stack)
      WScript.Echo(e.stack + '\n');
	else
	  WScript.Echo(e.toString() + '\n')
	
    numFails++;
  }
}
