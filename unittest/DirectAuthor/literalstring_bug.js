/**ref:..\\..\\Lib\\Author\\References\\domWeb.js**/
/**ref:..\\..\\Lib\\Author\\References\\libhelp.js**/
/**ref:EzeTestReference.js**/

/*
	Test case for the new Object functions on IE 9's HTML DOM elements.
 */
 
/*
 * This is a list of objects that will be iterated over to test the functionality
 * of the Object.defineProperty/ies functions.
 */

 var htmlElements = undefined;
 if (getHOSTMode() < IE9STANDARDSMODE) {
	//can be empty as all test cases exit immediately in legacy modes
	htmlElements = new Array();
    
} else {
	//this is required as may of these objects do not exist in IE7
	//mode and earlier
	htmlElements = [
		HTMLAnchorElement,
		HTMLAreaElement,
		HTMLAudioElement,
		HTMLBaseElement,
		HTMLBaseFontElement,
		HTMLBGSoundElement,
		HTMLBlockElement,
		HTMLBodyElement,
		HTMLBRElement,
		HTMLButtonElement,
		HTMLCanvasElement,
		//HTMLCommentElement,
		HTMLDDElement,
		HTMLDivElement,
		HTMLDListElement,
		HTMLDTElement,
		HTMLEmbedElement,
		HTMLFieldSetElement,
		HTMLFontElement,
		HTMLFormElement,
		HTMLFrameElement,
		HTMLFrameSetElement,
		//HTMLGenericElement,
		HTMLHeadElement,
		HTMLHeadingElement,
		HTMLHRElement,
		HTMLHtmlElement,
		HTMLIFrameElement,
		HTMLImageElement,
		HTMLInputElement,
		HTMLIsIndexElement,
		HTMLLabelElement,
		HTMLLegendElement,
		HTMLLIElement,
		HTMLLinkElement,
		HTMLMapElement,
		HTMLMarqueeElement,
		HTMLMetaElement,
		HTMLNextIdElement,
		HTMLObjectElement,
		HTMLOListElement,
		HTMLOptionElement,
		HTMLParagraphElement,
		HTMLParamElement,
		HTMLPhraseElement,
		HTMLScriptElement,
		HTMLSelectElement,
		HTMLSpanElement,
		HTMLStyleElement,
		HTMLTableCaptionElement,
		HTMLTableCellElement,
		HTMLTableColElement,
		HTMLTableElement,
		HTMLTableRowElement,
		HTMLTableSectionElement,
		HTMLTextAreaElement,
		HTMLTitleElement,
		HTMLUListElement,
		HTMLVideoElement,
	];
}

var options = [ true, false ];

function generatePropertyDescriptors() {
	var descs = {};
	var propCount = 0;
	for (var a in options)										// accessor
		for (var w in options)									// writable
			for (var c in options)								// configurable
				for (var e in options) {						// enumerable
					var prop = 'prop' + propCount++;
					descs[prop] = { configurable: options[c], enumerable: options[e] };
					if (options[a]) {
						if (options[w]) {
							descs[prop].get = function() { return this._prop; }
							descs[prop].set = function(v) { this._prop = v; }
						}
						else descs[prop].get = function() { return "get"; }
					}
					else {
						/**ml:-**/descs[prop].value = 'value';
						descs[prop].writable = options[w];
					}
				}
	return descs;
}
_$trace("at global\n");
var descs = generatePropertyDescriptors();		// global property bag

var tc = new TestCase();
tc.id = 3;
tc.desc = "15.2.3.2-7 Object.*property* and Object.create tests";
tc.test = function() {
	// The easiest way to test getOwnPropertyDescriptor() and getOwnPropertyNames()
	// is to add them. This also tests defineProperty() and defineProperties(), so yay!
	// As an added bonus, we need to use create() to create an instance of the item.
	
	function verifyPropertyDescriptors(d1, d2, o) {
		var items = [ 'writable', 'configurable', 'get', 'set', 'enumerable'];
		for (var i in items) {
			var item = items[i];
			verify(d1[item], d2[item], item + " should be the same for object: " + o);
		}
	}
	
	function verifyProperties(o, prop, desc) {
		o[prop] = prop;
		if (desc.writable || desc.set) {
			verify(o[prop], prop, "verify that the accessor/value can be set and retrieved.");
			delete o[prop];
			delete o._prop;
		}
		else if (desc.get) {
			verify(o[prop], "get", "verify that the non-writable accessor/value can be retrieved.");
		}
		else {
			verify(o[prop], "value", "verify that the non-writable accessor/value can be retrieved.");
		}
	}

	// Test for Object.create(o) and Object.defineProperty()
	(function() {
        _$trace("here 1\n");
        var counter = 0;
		for (var o in htmlElements) {
			var e = htmlElements[o];
			var i = Object.create(e);
			assert(e.isPrototypeOf(i), "verify that '" + i + "' is an instance of '" + e + "'");
			assert(Object.getPrototypeOf(i).isPrototypeOf(i), "verify that the result of getPrototypeOf is the same prototype of '" + i);
			_$trace(" top loop counter : " + (counter++) + "\n");
			
			for (var pn in descs) {
				var prop = descs[pn];
				Object.defineProperty(i, pn, prop);
				assert(i.hasOwnProperty(pn), "Ensure that property '" + pn + "' was successfully added to " + i);
				_$trace(" inner loop counter : " + (counter++) + "\n");
				var desc = Object.getOwnPropertyDescriptor(i, pn);
				verifyPropertyDescriptors(prop, desc, e);
				verifyProperties(i, pn, prop);
			}
		}
        /**ml:htmlElements**/
	})();
	
	// Test for Object.create(o, props)
	(function() {
        _$trace("here 2\n");
		for (var o in htmlElements) {
			var e = htmlElements[o];
			var i = Object.create(e, descs);
			assert(e.isPrototypeOf(i), "verify that '" + i + "' is an instance of '" + e + "'");
			assert(Object.getPrototypeOf(i).isPrototypeOf(i), "verify that the result of getPrototypeOf is the same prototype of '" + i);
			
			for (var pn in descs) {
				var prop = descs[pn];
				assert(i.hasOwnProperty(pn), "Ensure that property '" + pn + "' was successfully added to " + i);
				
				var desc = Object.getOwnPropertyDescriptor(i, pn);
				verifyPropertyDescriptors(prop, desc, e);
				verifyProperties(i, pn, prop);
			}
		}
	})();
	
	// Test for Object.create(o) and Object.defineProperties()
	(function() {
        _$trace("here 3\n");
		for (var o in htmlElements) {
			var e = htmlElements[o];
			var i = Object.create(e);
			assert(e.isPrototypeOf(i), "verify that '" + i + "' is an instance of '" + e + "'");
			assert(Object.getPrototypeOf(i).isPrototypeOf(i), "verify that the result of getPrototypeOf is the same prototype of '" + i);
			
			Object.defineProperties(i, descs);
			
			for (var pn in descs) {
				var prop = descs[pn];
				assert(i.hasOwnProperty(pn), "Ensure that property '" + pn + "' was successfully added to " + i);
				
				var desc = Object.getOwnPropertyDescriptor(i, pn);
				verifyPropertyDescriptors(prop, desc, e);
				verifyProperties(i, pn, prop);
			}
		}
	})();
	
	// Test for Object.defineProperties() on elements
	(function() {
        _$trace("here 4\n");
		for (var e in htmlElements) {
			var o = htmlElements[e];
			Object.defineProperties(o, descs);
			
			for (var pn in descs) {
				var prop = descs[pn];
				assert(o.hasOwnProperty(pn), "Ensure that property '" + pn + "' was successfully added to " + o);
				
				var desc = Object.getOwnPropertyDescriptor(o, pn);
				verifyPropertyDescriptors(prop, desc, o);
				verifyProperties(o, pn, prop);
			}
		}
	})();
};
tc.AddTest();

var tc = new TestCase();
tc.id = 4;
tc.desc = "15.2.3.4 Object.getOwnPropertyNames";
tc.test = function() {
	(function() {
            _$trace("here 5\n");

		for (var e in htmlElements) {
			var o = Object.create(htmlElements[e], descs);
			var names = Object.getOwnPropertyNames(o).sort();
			
			// Note that getOwnPropertyNames ignores the [[Enumerable]] flag.
			for (var d in descs) {
				assert(names.indexOf(d) != -1, d + " should be in the array on object " + o);
			}
		}
	})();
};
tc.AddTest();

var tc = new TestCase();
tc.id = 14;
tc.desc = "15.2.3.14 Object.keys";
tc.test = function() {
	(function() {
            _$trace("here 6\n");
		for (var e in htmlElements) {
			var o = Object.create(htmlElements[e], descs);
			var keys = Object.keys(o);
			
			// Note that keys does NOT ignore the [[Enumerable]] flag.
			for (var d in descs) {
				verify(descs[d].enumerable, keys.indexOf(d) != -1, "verify proper existance (or non) of property " + d + " for object " + o);
			}
		}
	})();
};
tc.AddTest();

var tc = new TestCase();
tc.id = 15;
tc.desc = "Verify empty document object";
tc.test = function() {
	//with no user expandos, document should have no 'own' properties
	(function() {
            _$trace("here 7\n");

		//test depends on ES5 features in IE9 and greater
		if (getHOSTMode() < IE9STANDARDSMODE)
			return;
	
		var keys = Object.keys(document);
		verify(keys.length, 0, "document has no 'own' enumerable properties");
		
		var names = Object.getOwnPropertyNames(document);
		verify(names.length, 0, "document has no 'own' non-enumerable properties");
		
		for (var x in document) {
			assert(!document.hasOwnProperty(document[x]), "Inherited properties are not 'own' properties: " + document[x]);
		}
	})();
};
tc.AddTest();
