
function _$InitializeAsyncRequests() {
		_$asyncRequests = [{ src: '' }];
}

var document = { };
(function () {
	function _getEventObject(type, attach) {
		if(attach) return MSEventObj;
		switch(type) {
			case 'activate':
			case 'afterprint':
			case 'afterupdate':
			case 'beforeactivate':
			case 'beforecopy':
			case 'beforecut':
			case 'beforedeactivate':
			case 'beforeeditfocus':
			case 'beforepaste':
			case 'beforeprint':
			case 'beforeupdate':
			case 'bounce':
			case 'cellchange':
			case 'controlselect':
			case 'copy':
			case 'cut':
			case 'dataavailable':
			case 'datasetchanged':
			case 'datasetcomplete':
			case 'deactivate':
			case 'errorupdate':
			case 'filterchange':
			case 'finish':
			case 'hashchange':
			case 'help':
			case 'layoutcomplete':
			case 'losecapture':
			case 'mouseenter':
			case 'mouseleave':
			case 'move':
			case 'moveend':
			case 'movestart':
			case 'page':
			case 'paste':
			case 'propertychange':
			case 'reset':
			case 'resizeend':
			case 'resizestart':
			case 'rowenter':
			case 'rowexit':
			case 'rowsdelete':
			case 'rowsinserted':
			case 'selectionchange':
			case 'selectstart':
			case 'start':
			case 'stop':
			case 'storagecommit':
			case 'unload':
				// general event object
				break;
			case 'message':
				return MessageEvent;
			case 'beforeunload':
				return BeforeUnloadEvent;
			case 'abort': 
			case 'error':
			case 'resize':
			case 'scroll':
			case 'select':
				return UIEvent;	
			case 'canplay':
			case 'canplaythrough':
			case 'change':
			case 'contextmenu':
			case 'durationchange':
			case 'emptied':
			case 'ended':
			case 'input':
			case 'load':
			case 'loadeddata':
			case 'loadedmetadata':
			case 'loadstart':
			case 'offline':
			case 'online':
			case 'pause':
			case 'play':
			case 'playing':
			case 'progress':
			case 'ratechange':
			case 'readystatechange':
			case 'seeked':
			case 'seeking':
			case 'stalled':
			case 'submit':
			case 'suspend':
			case 'timeupdate':
			case 'volumechange':
			case 'waiting':
			case 'DOMContentLoaded':
				return Event;
			case 'click':
			case 'dblclick':
			case 'mousedown':
			case 'mousemove':
			case 'mouseout':
			case 'mouseover':
			case 'mouseup':
				return MouseEvent;
			case 'drag':
			case 'dragend':
			case 'dragenter':
			case 'dragleave':
			case 'dragover':
			case 'dragstart':
			case 'drop':
				return DragEvent;
			case 'blur':
			case 'focus':
			case 'focusin':
			case 'focusout':
				return FocusEvent;
			case 'keydown':
			case 'keypress':
			case 'keyup':
				return KeyboardEvent;
			case 'mousewheel':
				return MouseWheelEvent;
			case 'mssitemodejumplistitemremoved':
			case 'msthumbnailclick':
				return MSSiteModeEvent;
			case 'storage':
				return StorageEvent;
			case 'timeout':
				return undefined;
			break;
		}
		return MSEventObj;
	}

	function _getTagNameMap() {
		return {
			"ol" : HTMLOListElement,
			"select" : HTMLSelectElement,
			"address" : HTMLBlockElement,
			"blockquote" : HTMLBlockElement,
			"center" : HTMLBlockElement,
			"listing" : HTMLBlockElement,
			"plaintext" : HTMLBlockElement,
			"pre" : HTMLBlockElement,
			"xmp" : HTMLBlockElement,
			"dd" : HTMLDDElement,
			"font" : HTMLFontElement,
			"caption" : HTMLTableCaptionElement,
			"optgroup" : HTMLOptionElement,
			"option" : HTMLOptionElement,
			"map" : HTMLMapElement,
			"menu" : HTMLMenuElement,
			"img" : HTMLImageElement,
			"area" : HTMLAreaElement,
			"button" : HTMLButtonElement,
			"source" : HTMLSourceElement,
			"script" : HTMLScriptElement,
			"tr" : HTMLTableRowElement,
			"html" : HTMLHtmlElement,
			"frame" : HTMLFrameElement,
			"quote" : HTMLQuoteElement,
			"th" : HTMLTableHeaderCellElement,
			"dl" : HTMLDListElement,
			"frameset" : HTMLFrameSetElement,
			"label" : HTMLLabelElement,
			"legend" : HTMLLegendElement,
			"dir" : HTMLDirectoryElement,
			"li" : HTMLLIElement,
			"iframe" : HTMLIFrameElement,
			"body" : HTMLBodyElement,
			"tbody" : HTMLTableSectionElement,
			"tfoot" : HTMLTableSectionElement,
			"thead" : HTMLTableSectionElement,
			"input" : HTMLInputElement,
			"a" : HTMLAnchorElement,
			"param" : HTMLParamElement,
			"pre" : HTMLPreElement,
			"abbr" : HTMLPhraseElement,  
			"acronym" : HTMLPhraseElement,  
			"b" : HTMLPhraseElement,  
			"bdo" : HTMLPhraseElement,  
			"big" : HTMLPhraseElement,  
			"cite" : HTMLPhraseElement,  
			"code" : HTMLPhraseElement,  
			"del" : HTMLPhraseElement,  
			"dfn" : HTMLPhraseElement,  
			"em" : HTMLPhraseElement,  
			"i" : HTMLPhraseElement,  
			"ins" : HTMLPhraseElement,  
			"kbd" : HTMLPhraseElement,  
			"nobr" : HTMLPhraseElement,  
			"q" : HTMLPhraseElement,  
			"rt" : HTMLPhraseElement,  
			"ruby" : HTMLPhraseElement,  
			"s" : HTMLPhraseElement,  
			"samp" : HTMLPhraseElement,  
			"small" : HTMLPhraseElement,  
			"strike" : HTMLPhraseElement,  
			"strong" : HTMLPhraseElement,  
			"sub" : HTMLPhraseElement,  
			"sup" : HTMLPhraseElement,  
			"tt" : HTMLPhraseElement,  
			"u" : HTMLPhraseElement,  
			"var" : HTMLPhraseElement, 
			"canvas" : HTMLCanvasElement,
			"title" : HTMLTitleElement,
			"style" : HTMLStyleElement,
			"unknown" : HTMLUnknownElement,
			"audio" : HTMLAudioElement,
			"td" : HTMLTableCellElement,
			"th" : HTMLTableCellElement,
			"basefont" : HTMLBaseFontElement,
			"textarea" : HTMLTextAreaElement,
			"marquee" : HTMLMarqueeElement,
			"modification" : HTMLModElement,
			"col" : HTMLTableColElement,
			"colgroup" : HTMLTableColElement,
			"ul" : HTMLUListElement,
			"div" : HTMLDivElement,
			"br" : HTMLBRElement,
			"span" : HTMLSpanElement,
			"head" : HTMLHeadElement,
			"hn" : HTMLHeadingElement,
			"form" : HTMLFormElement,
			"dt" : HTMLDTElement,
			"fieldset" : HTMLFieldSetElement,
			"bgsound" : HTMLBGSoundElement,
			"hr" : HTMLHRElement,
			"applet" : HTMLObjectElement,
			"object" : HTMLObjectElement,
			"embed" : HTMLEmbedElement,
			"optgroup" : HTMLOptGroupElement,
			"isindex" : HTMLIsIndexElement,
			"video" : HTMLVideoElement,
			'article': HTMLElement, // HTML5, not implemented 
			'aside': HTMLElement, // HTML5, not implemented  
			'base': HTMLBaseElement,
			'command': HTMLElement, // HTML5, not implemented 
			'details': HTMLElement, // HTML5, not implemented 
			'figcaption': HTMLElement, // HTML5, not implemented 
			'figure': HTMLElement, // HTML5, not implemented 
			'footer': HTMLElement, // HTML5, not implemented
			'h1': HTMLHeadingElement,
			'h2': HTMLHeadingElement,
			'h3': HTMLHeadingElement,
			'h4': HTMLHeadingElement,
			'h5': HTMLHeadingElement,
			'h6': HTMLHeadingElement,
			'header': HTMLElement, // HTML5, not implemented 
			'hgroup': HTMLElement, // HTML5, not implemented 
			'link': HTMLLinkElement, 
			'mark': HTMLElement, // HTML5, not implemented 
			'meta': HTMLMetaElement, 
			'nav': HTMLElement, // HTML5, not implemented 
			'p': HTMLParagraphElement, 
			'section': HTMLElement, // HTML5, not implemented 
			'summary': HTMLElement, // HTML5, not implemented
			'table': HTMLTableElement, 
			'time': HTMLElement // HTML5, not implemented
		};
	}

	function _createDomObject(name) {
		switch (name) {
			case 'Document': return Document;
			case 'HTMLAnchorElement': return HTMLAnchorElement;
			case 'HTMLAreaElement': return HTMLAreaElement;
			case 'HTMLAudioElement': return HTMLAudioElement;
			case 'HTMLBaseFontElement': return HTMLBaseFontElement;
			case 'HTMLBGSoundElement': return HTMLBGSoundElement;
			case 'HTMLBlockElement': return HTMLBlockElement;
			case 'HTMLBodyElement': return HTMLBodyElement;
			case 'HTMLBRElement': return HTMLBRElement;
			case 'HTMLButtonElement': return HTMLButtonElement;
			case 'HTMLCanvasElement': return HTMLCanvasElement;
			case 'HTMLDDElement': return HTMLDDElement;
			case 'HTMLDirectoryElement': return HTMLDirectoryElement;
			case 'HTMLDivElement': return HTMLDivElement;
			case 'HTMLDListElement': return HTMLDListElement;
			case 'HTMLDTElement': return HTMLDTElement;
			case 'HTMLEmbedElement': return HTMLEmbedElement;
			case 'HTMLElement': return HTMLElement;
			case 'HTMLFieldSetElement': return HTMLFieldSetElement;
			case 'HTMLFontElement': return HTMLFontElement;
			case 'HTMLFormElement': return HTMLFormElement;
			case 'HTMLFrameElement': return HTMLFrameElement;
			case 'HTMLFrameSetElement': return HTMLFrameSetElement;
			case 'HTMLHeadElement': return HTMLHeadElement;
			case 'HTMLHeadingElement': return HTMLHeadingElement;
			case 'HTMLHRElement': return HTMLHRElement;
			case 'HTMLHtmlElement': return HTMLHtmlElement;
			case 'HTMLIFrameElement': return HTMLIFrameElement;
			case 'HTMLImageElement': return HTMLImageElement;
			case 'HTMLInputElement': return HTMLInputElement;
			case 'HTMLIsIndexElement': return HTMLIsIndexElement;
			case 'HTMLLabelElement': return HTMLLabelElement;
			case 'HTMLLegendElement': return HTMLLegendElement;
			case 'HTMLLIElement': return HTMLLIElement;
			case 'HTMLMapElement': return HTMLMapElement;
			case 'HTMLMarqueeElement': return HTMLMarqueeElement;
			case 'HTMLMenuElement': return HTMLMenuElement;
			case 'HTMLModElement': return HTMLModElement;
			case 'HTMLObjectElement': return HTMLObjectElement;
			case 'HTMLOListElement': return HTMLOListElement;
			case 'HTMLOptGroupElement': return HTMLOptGroupElement;
			case 'HTMLOptionElement': return HTMLOptionElement;
			case 'HTMLParamElement': return HTMLParamElement;
			case 'HTMLPhraseElement ': return HTMLPhraseElement ;
			case 'HTMLPhraseElement  ': return HTMLPhraseElement  ;
			case 'HTMLPreElement': return HTMLPreElement;
			case 'HTMLProgressElement': return HTMLProgressElement;
			case 'HTMLQuoteElement': return HTMLQuoteElement;
			case 'HTMLScriptElement': return HTMLScriptElement;
			case 'HTMLSelectElement': return HTMLSelectElement;
			case 'HTMLSourceElement': return HTMLSourceElement;
			case 'HTMLSpanElement': return HTMLSpanElement;
			case 'HTMLStyleElement': return HTMLStyleElement;
			case 'HTMLTableCaptionElement': return HTMLTableCaptionElement;
			case 'HTMLTableCellElement': return HTMLTableCellElement;
			case 'HTMLTableColElement': return HTMLTableColElement;
			case 'HTMLTableHeaderCellElement': return HTMLTableHeaderCellElement;
			case 'HTMLTableRowElement': return HTMLTableRowElement;
			case 'HTMLTableSectionElement': return HTMLTableSectionElement;
			case 'HTMLTextAreaElement': return HTMLTextAreaElement;
			case 'HTMLTitleElement': return HTMLTitleElement;
			case 'HTMLUListElement': return HTMLUListElement;
			case 'HTMLUnknownElement': return HTMLUnknownElement;
			case 'HTMLVideoElement': return HTMLVideoElement;
			case 'HTMLBaseElement': return HTMLBaseElement;
			case 'HTMLLinkElement': return HTMLLinkElement;
			case 'HTMLMetaElement': return HTMLMetaElement;
			case 'HTMLParagraphElement': return HTMLParagraphElement;
			case 'HTMLTableElement': return HTMLTableElement;
			case 'Node': return Node;
			case 'Window': return Window;
		}
		return undefined;
	};

	function _getAsyncRequests() {
		_$asyncRequests = (typeof(_$asyncRequests) !== 'undefined' && _$asyncRequests) ? _$asyncRequests : [{src:''}];
		return _$asyncRequests;
	}

	function _markAsAsyncScript(object) {
		object._$isAsyncScript = true;
		return object;
	}

	function _isAsyncScript(object) {
		return object && object._$isAsyncScript;
	}

	function _createElementByTagName(tagName) {
		var element = HTMLElement;
		var tagNameMap = _getTagNameMap();
		if(tagNameMap[tagName]) 
			element = tagNameMap[tagName];
		var newObject = Object.create(element);
		if (tagName == 'script')
			_markAsAsyncScript(newObject);
		return newObject;
	}

	function _getElementsByTagName(tagName) {
		if (tagName == "script") 
			return _getAsyncRequests();
		var element = HTMLElement;
		var tagNameMap = _getTagNameMap();
		if(tagNameMap[tagName]) 
			element = tagNameMap[tagName];
		return new Array(Object.create(element));
	}

	function _firstChild() {
		return _getAsyncRequests()[0];
	}

	function _lastChild() {
		var asyncRequests = _getAsyncRequests();
		return asyncRequests[asyncRequests.length - 1];
	}

	function _childCount() {
		return _getAsyncRequests().length;
	}

	function _insertBefore(newChild, refChild) {
		if (_isAsyncScript(newChild)) {
			var index = 0;
			for(var request in _getAsyncRequests()) {
				if (_$asyncRequests[request] == refChild)
					break;
				index ++;
			}
			_$asyncRequests = _$asyncRequests.slice(0,index).concat(newChild, _$asyncRequests.slice(index));
		}
		return newChild;
	}

	function _replaceChild(newChild, oldChild) {
		if (_isAsyncScript(newChild)) {
			for(var request in _getAsyncRequests()) {
				if (_$asyncRequests[request] == oldChild) {
					_$asyncRequests[request] = newChild;
					break;
				}
			}
		}
		return oldChild;
	}

	function _appendChild(newChild) {
		if (_isAsyncScript(newChild))
			_getAsyncRequests().push(newChild);
		return newChild;
	}

	function _applyElement(apply, where) {
		if (where == "outside")
			return _insertBefore(apply, this);
		else
			return _appendChild(apply);
	}

	function _hasChildNodes() {
		return true;
	}

	function _clearTimeout(handle){
	}

	function _setTimeout(expression, msec, language) {
		if (typeof expression == 'function') {
			if (arguments && arguments.length > 2) {
				var args = [];
				for (var i = 2; i < arguments.length; i++)
					args.push(arguments[i]);
				expression.apply(this, args);
			}
			else {
				expression.apply(this);
			}
		}
		else if (typeof expression == 'string') {
			eval(expression);
		}
		return 1;
	}

	function _hasAttribute(object, name) {
		if (object)
			return object.hasOwnProperty(name);
		else
			return false;
	}

	function _setAttribute(object, name, value) {
		if (object)
			object[name] = value;
	}

	function _getAttribute(object, name) {
		if (_hasAttribute(object, name))
			return object[name];
		else 
			return null;
	}

	var sending = false;
	var listeners = [];

	function _removeOnPrefix(name) {
		return (name.length > 2 && name[0] == 'o' && name[1] == 'n') ? name.substring(2, name.length) : name;
	}
	
	function _addEventListener(object, type, handler, attach) {
		if(!object) return;
		type = _removeOnPrefix(type);
		object['_$' + type] = handler;
		if(handler) {
			if (sending)
				handler(_getEventObject(type, attach));
			else
				listeners.push(function() { handler(_getEventObject(type, attach)); });
		}
	}
	
	function _getEventListener(object, type) {
		type = _removeOnPrefix(type);
		return object ? object['_$' + type] : undefined;
	}

	function _invokeListeners() {
		sending = true;
		for (var i = listeners.length - 1; i >= 0; i--)
			listeners[i]();
	}

	function _getElementById(elementId) {
		var element = document._$documentElements ? document._$documentElements[elementId] : undefined;
		if(element) 
			return element;
		return HTMLElement;
	}

	function _openXMLHttpRequest (request, method, url) { 
		request._$url = url;
		request._$method = method;
		_markAsAsyncScript(request);
	};

	function _sendXMLHttpRequest(request, data) { 
		request._$requestBody = data;
		_getAsyncRequests().push(request);
	};
	
	function _defineEventAttribute(object, name) {
		if(!object.hasOwnProperty(name)) {
			Object.defineProperty(object, name, 
				{ 
					set: function (handler) { 
						_addEventListener(this, name, handler); 
					}, 
					get: function () { 
						return _getEventListener(this, name); 
					}
				});		
			}
	}

	function _inherit(o) {
		return Object.create(o);
	}
	
	function _implement(o, type) {
		if(!o || !type) return;
		var props = Object.getOwnPropertyNames(type);
		for(var i=0; i<props.length; i++) {
            var prop = props[i];
			Object.defineProperty(o, prop, Object.getOwnPropertyDescriptor(type, prop));
		}
	}
	
	function _events(o) {
		if(!o) return;
		for(var i=1; i<arguments.length; i++) {
			_defineEventAttribute(o, arguments[i]);
		}
	}

	var ElementTraversal = {};
	var SVGElementEventHandlers = {};
	var MSElementExtensions = {};
	var SVGLangSpace = {};
	var SVGStylable = {};
	var EventTarget = {};
	var Node = {};
	var NodeSelector = {};
	var Element = _inherit(Node);
	var SVGElement = _inherit(Element);
	var SVGTests = {};
	var SVGUnitTypes = {};
	var SVGMaskElement = _inherit(SVGElement);
	var ClientRectList = new Array();
	var DOML2DeprecatedAlignmentStyle_HTMLTableCellElement = {};
	var MSXMLHttpRequestExtensions = {};
	var MSElementCSSInlineStyleExtensions = {};
	var ElementCSSInlineStyle = {};
	var MSHTMLElementExtensions = {};
	var MSNodeExtensions = {};
	var MSEventAttachmentTarget = {};
	var MSHTMLElementRangeExtensions = {};
	var HTMLElement = _inherit(Element);
	var HTMLMediaElement = _inherit(HTMLElement);
	var HTMLVideoElement = _inherit(HTMLMediaElement);
	var LinkStyle = {};
	var MSDataBindingRecordSetExtensions = {};
	var CSSRuleList = new Array();
	var SVGFitToViewBox = {};
	var SVGSymbolElement = _inherit(SVGElement);
	var SVGElementInstanceList = new Array();
	var SVGStopElement = _inherit(SVGElement);
	var PositionCallback = {};
	var HTMLBodyElementDOML2Deprecated = {};
	var DOML2DeprecatedAlignmentStyle_HTMLTableCaptionElement = {};
	var PerformanceNavigation = {};
	var MSAttrExtensions = {};
	var Attr = _inherit(Node);
	var MSBorderColorStyle_HTMLTableRowElement = {};
	var SVGPreserveAspectRatio = {};
	var DOML2DeprecatedMarginStyle_MSHTMLIFrameElementExtensions = {};
	var DOML2DeprecatedBorderStyle_MSHTMLIFrameElementExtensions = {};
	var MSHTMLIFrameElementExtensions = {};
	var SVGLocatable = {};
	var SVGTransformable = _inherit(SVGLocatable);
	var SVGSwitchElement = _inherit(SVGElement);
	var MSCompatibleInfoCollection = new Array();
	var MSHTMLIsIndexElementExtensions = {};
	var SVGAnimatedBoolean = {};
	var MSHTMLDocumentSelection = {};
	var DOMException = {};
	var HTMLIsIndexElement = _inherit(HTMLElement);
	var SVGPathSeg = {};
	var SVGPathSegLinetoRel = _inherit(SVGPathSeg);
	var MSHTMLOptGroupElementExtensions = {};
	var MSDataBindingExtensions = {};
	var HTMLOptGroupElement = _inherit(HTMLElement);
	var CharacterData = _inherit(Node);
	var DOML2DeprecatedAlignmentStyle_HTMLTableSectionElement = {};
	var DocumentView = {};
	var MSEventExtensions = {};
	var Event = {};
	var StorageEvent = _inherit(Event);
	var MSHTMLEmbedElementExtensions = {};
	var GetSVGDocument = {};
	var HTMLEmbedElement = _inherit(HTMLElement);
	var DOML2DeprecatedMarginStyle_HTMLObjectElement = {};
	var DOML2DeprecatedAlignmentStyle_HTMLObjectElement = {};
	var MSHTMLObjectElementExtensions = {};
	var DOML2DeprecatedBorderStyle_HTMLObjectElement = {};
	var HTMLObjectElement = _inherit(HTMLElement);
	var HTMLHRElementDOML2Deprecated = {};
	var DOML2DeprecatedColorProperty = {};
	var MSHTMLHRElementExtensions = {};
	var DOML2DeprecatedWidthStyle_HTMLHRElement = {};
	var DOML2DeprecatedSizeProperty = {};
	var DOML2DeprecatedAlignmentStyle_HTMLHRElement = {};
	var HTMLHRElement = _inherit(HTMLElement);
	var MSHTMLFrameSetElementExtensions = {};
	var DOML2DeprecatedTextFlowControl_HTMLBlockElement = {};
	var PositionOptions = {};
	var CanvasPattern = {};
	var MSCommentExtensions = {};
	var Comment = _inherit(CharacterData);
	var HTMLBGSoundElement = _inherit(HTMLElement);
	var DOML2DeprecatedAlignmentStyle_HTMLFieldSetElement = {};
	var MSHTMLFieldSetElementExtensions = {};
	var HTMLFieldSetElement = _inherit(HTMLElement);
	var MediaError = {};
	var SVGNumberList = {};
	var DOML2DeprecatedMarginStyle_HTMLInputElement = {};
	var DOML2DeprecatedBackgroundColorStyle = {};
	var MSHTMLTableSectionElementExtensions = {};
	var AbstractView = {};
	var ScreenView = _inherit(AbstractView);
	var DOML2DeprecatedWordWrapSuppression_HTMLDTElement = {};
	var MSBorderColorStyle_HTMLFrameElement = {};
	var NodeFilter = {};
	var DOML2DeprecatedTextFlowControl_HTMLBRElement = {};
	var MSHTMLParagraphElementExtensions = {};
	var SVGURIReference = {};
	var SVGGradientElement = _inherit(SVGElement);
	var MSHTMLTableRowElementExtensions = {};
	var DOML2DeprecatedWordWrapSuppression_HTMLDDElement = {};
	var StyleSheetPage = {};
	var XMLSerializer = {};
	var NodeList = new Array();
	var HTMLDTElement = _inherit(HTMLElement);
	var SVGTextContentElement = _inherit(SVGElement);
	var SVGTextPathElement = _inherit(SVGTextContentElement);
	var DOML2DeprecatedWidthStyle_HTMLAppletElement = {};
	var DOML2DeprecatedBorderStyle_HTMLTableElement = {};
	var StyleSheet = {};
	var MSMimeTypesCollection = {};
	var DOMParser = {};
	var MSHTMLFormElementExtensions = {};
	var MSHTMLCollectionExtensions = {};
	var HTMLFormElement = _inherit(HTMLElement);
	var SVGZoomAndPan = {};
	var DOML2DeprecatedAlignmentStyle_HTMLHeadingElement = {};
	var MSHTMLHeadingElementExtensions = {};
	var HTMLHeadingElement = _inherit(HTMLElement);
	var NodeFilterCallback = {};
	var HTMLHeadElement = _inherit(HTMLElement);
	var HTMLSpanElement = _inherit(HTMLElement);
	var DOML2DeprecatedWordWrapSuppression_HTMLDivElement = {};
	var MSHTMLDivElementExtensions = {};
	var DOML2DeprecatedBorderStyle_HTMLInputElement = {};
	var HTMLBRElement = _inherit(HTMLElement);
	var CSSRule = {};
	var CSSPageRule = _inherit(CSSRule);
	var WindowPerformance = {};
	var BookmarkCollection = new Array();
	var SVGAnimatedPathData = {};
	var Position = {};
	var DOML2DeprecatedWidthStyle = {};
	var SVGAnimatedPoints = {};
	var SVGPolylineElement = _inherit(SVGElement);
	var DocumentFragment = _inherit(Node);
	var UIEvent = _inherit(Event);
	var TextEvent = _inherit(UIEvent);
	var DOML2DeprecatedBackgroundStyle = {};
	var CSSFontFaceRule = _inherit(CSSRule);
	var MSBehaviorUrnsCollection = new Array();
	var MSWindowExtensions = {};
	var ProcessingInstruction = _inherit(Node);
	var SVGLengthList = {};
	var SVGPathSegCurvetoCubicSmoothRel = _inherit(SVGPathSeg);
	var SVGPathSegCurvetoQuadraticSmoothAbs = _inherit(SVGPathSeg);
	var MediaList = new Array();
	var SVG1_1Properties = {};
	var NamedNodeMap = {};
	var DOML2DeprecatedBorderStyle = {};
	var DOML2DeprecatedAlignmentStyle_HTMLDivElement = {};
	var HTMLDivElement = _inherit(HTMLElement);
	var NavigatorDoNotTrack = {};
	var SVGRectElement = _inherit(SVGElement);
	var DOML2DeprecatedListNumberingAndBulletStyle = {};
	var DOML2DeprecatedListSpaceReduction = {};
	var HTMLUListElement = _inherit(HTMLElement);
	var MSBorderColorStyle_HTMLTableCellElement = {};
	var HTMLTableAlignment = {};
	var SVGAnimatedEnumeration = {};
	var SVGLinearGradientElement = _inherit(SVGGradientElement);
	var MSHTMLDocumentEventExtensions = {};
	var MSHTMLDocumentViewExtensions = {};
	var MSResourceMetadata = {};
	var MSHTMLDocumentExtensions = {};
	var HTMLDocument = {};
	var SVGException = {};
	var DOML2DeprecatedTableCellHeight = {};
	var DOML2DeprecatedAlignmentStyle_HTMLTableColElement = {};
	var HTMLTableColElement = _inherit(HTMLElement);
	var ImageData = {};
	var SVGUseElement = _inherit(SVGElement);
	var BeforeUnloadEvent = _inherit(Event);
	var MSPopupWindow = {};
	var SVGMatrix = {};
	var HTMLModElement = _inherit(HTMLElement);
	var DOML2DeprecatedWordWrapSuppression = {};
	var MSMouseEventExtensions = {};
	var SVGPathSegLinetoAbs = _inherit(SVGPathSeg);
	var TimeRanges = {};
	var SVGPathSegCurvetoQuadraticAbs = _inherit(SVGPathSeg);
	var SVGPathSegCurvetoCubicAbs = _inherit(SVGPathSeg);
	var DocumentStyle = {};
	var History = {};
	var KeyboardEventExtensions = {};
	var DOML2DeprecatedMarginStyle_HTMLMarqueeElement = {};
	var HTMLMarqueeElement = _inherit(HTMLElement);
	var SVGRect = {};
	var MSWindowModeless = {};
	var DOML2DeprecatedMarginStyle = {};
	var Geolocation = {};
	var MSHTMLTextAreaElementExtensions = {};
	var HTMLTextAreaElement = _inherit(HTMLElement);
	var DOML2DeprecatedSizeProperty_HTMLBaseFontElement = {};
	var HTMLBaseFontElement = _inherit(HTMLElement);
	var CustomEvent = _inherit(Event);
	var CSSImportRule = _inherit(CSSRule);
	var StyleSheetList = new Array();
	var SVGCircleElement = _inherit(SVGElement);
	var MSNamespaceInfoCollection = new Array();
	var SVGElementInstance = {};
	var BrowserPublic = {};
	var MSBorderColorHighlightStyle_HTMLTableCellElement = {};
	var DOML2DeprecatedWidthStyle_HTMLTableCellElement = {};
	var HTMLTableHeaderCellScope = {};
	var HTMLTableCellElement = _inherit(HTMLElement);
	var MSBorderColorHighlightStyle_HTMLTableRowElement = {};
	var PositionError = {};
	var MSImageResourceExtensions = {};
	var HTMLAudioElement = _inherit(HTMLMediaElement);
	var MSDataBindingRecordSetReadonlyExtensions = {};
	var HTMLUnknownElement = _inherit(HTMLElement);
	var SVGPathSegList = {};
	var CSSNamespaceRule = _inherit(CSSRule);
	var Text = _inherit(CharacterData);
	var SVGAnimatedRect = {};
	var MSCompatibleInfo = {};
	var SVGPathElement = _inherit(SVGElement);
	var SVGNumber = {};
	var MouseEvent = _inherit(UIEvent);
	var WheelEvent = _inherit(MouseEvent);
	var ViewCSS_SVGSVGElement = {};
	var MSCSSFilter = {};
	var SVGTransform = {};
	var MSBorderColorHighlightStyle = {};
	var MSLinkStyleExtensions = {};
	var HTMLStyleElement = _inherit(HTMLElement);
	var HTMLTitleElement = _inherit(HTMLElement);
	var Location = {};
	var HTMLCanvasElement = _inherit(HTMLElement);
	var MSEventObj = {};
	var SVGPathSegCurvetoCubicRel = _inherit(SVGPathSeg);
	var HTMLPhraseElement = _inherit(HTMLElement);
	var SVGPolygonElement = _inherit(SVGElement);
	var SVGLength = {};
	var XDomainRequest = {};
	var SVGStringList = {};
	var SVGPathSegMovetoAbs = _inherit(SVGPathSeg);
	var SVGPathSegArcRel = _inherit(SVGPathSeg);
	var SVGMetadataElement = _inherit(SVGElement);
	var WindowLocalStorage = {};
	var NavigatorOnLine = {};
	var DOMHTMLImplementation = {};
	var EventException = {};
	var MSHTMLPreElementExtensions = {};
	var HTMLPreElement = _inherit(HTMLElement);
	var DOML2DeprecatedAlignmentStyle_HTMLInputElement = {};
	var PerformanceTiming = {};
	var SVGAnimatedNumber = {};
	var HTMLParamElement = _inherit(HTMLElement);
	var SVGImageElement = _inherit(SVGElement);
	var MSHTMLAnchorElementExtensions = {};
	var HTMLAnchorElement = _inherit(HTMLElement);
	var MSHTMLInputElementExtensions = {};
	var MSImageResourceExtensions_HTMLInputElement = {};
	var HTMLInputElement = _inherit(HTMLElement);
	var HTMLTableSectionElement = _inherit(HTMLElement);
	var DragEvent = _inherit(MouseEvent);
	var MutationEvent = _inherit(Event);
	var SVGRadialGradientElement = _inherit(SVGGradientElement);
	var DOML2DeprecatedAlignmentStyle_HTMLLegendElement = {};
	var DocumentType = _inherit(Node);
	var DOML2DeprecatedWordWrapSuppression_HTMLBodyElement = {};
	var MSHTMLBodyElementExtensions = {};
	var HTMLBodyElement = _inherit(HTMLElement);
	var MSNavigatorAbilities = {};
	var TextRangeCollection = new Array();
	var DOML2DeprecatedAlignmentStyle_HTMLIFrameElement = {};
	var HTMLIFrameElement = _inherit(HTMLElement);
	var MSStorageExtensions = {};
	var Storage = {};
	var DocumentTraversal = {};
	var CSS3Properties = {};
	var MSCSSStyleDeclarationExtensions = {};
	var CSS2Properties = {};
	var CSSStyleDeclaration = {};
	var MSCSSProperties = _inherit(CSSStyleDeclaration);
	var MSCurrentStyleCSSProperties = _inherit(MSCSSProperties);
	var SVGStyleElement = _inherit(SVGElement);
	var ViewCSS = {};
	var HTMLLIElement = _inherit(HTMLElement);
	var SVGPathSegLinetoVerticalAbs = _inherit(SVGPathSeg);
	var SVGTextPositioningElement = _inherit(SVGTextContentElement);
	var SVGTSpanElement = _inherit(SVGTextPositioningElement);
	var SVGTextElement = _inherit(SVGTextPositioningElement);
	var SVGAnimatedInteger = {};
	var NavigatorAbilities = {};
	var MSHTMLImageElementExtensions = {};
	var HTMLLegendElement = _inherit(HTMLElement);
	var MSHTMLDirectoryElementExtensions = {};
	var HTMLDirectoryElement = _inherit(HTMLElement);
	var MSHTMLQuoteElementExtensions = {};
	var HTMLLabelElement = _inherit(HTMLElement);
	var DocumentEvent = {};
	var SVGSVGElementEventHandlers = {};
	var SVGSVGElement = _inherit(SVGElement);
	var MSPluginsCollection = {};
	var SVGAnimatedNumberList = {};
	var SVGPoint = {};
	var Range = {};
	var FocusEvent = _inherit(UIEvent);
	var DataTransfer = {};
	var EventListener = {};
	var NavigatorGeolocation = {};
	var Coordinates = {};
	var MSScreenExtensions = {};
	var Screen = {};
	var MSBorderColorStyle_HTMLFrameSetElement = {};
	var HTMLFrameSetElement = _inherit(HTMLElement);
	var MSHTMLMetaElementExtensions = {};
	var SVGAElement = _inherit(SVGElement);
	var SVGEllipseElement = _inherit(SVGElement);
	var SVGPathSegLinetoHorizontalRel = _inherit(SVGPathSeg);
	var HTMLDListElement = _inherit(HTMLElement);
	var HTMLTableHeaderCellElement = _inherit(HTMLTableCellElement);
	var XMLHttpRequest = {};
	var WindowModal = {};
	var MSHTMLButtonElementExtensions = {};
	var CSSMediaRule = _inherit(CSSRule);
	var HTMLQuoteElement = _inherit(HTMLElement);
	var SVGDefsElement = _inherit(SVGElement);
	var SVGAnimatedLength = {};
	var MSHTMLFrameElementExtensions = {};
	var HTMLFrameElement = _inherit(HTMLElement);
	var SVGPathSegClosePath = _inherit(SVGPathSeg);
	var HTMLHtmlElementDOML2Deprecated = {};
	var HTMLHtmlElement = _inherit(HTMLElement);
	var MSBorderColorStyle = {};
	var SVGTransformList = {};
	var SVGPathSegArcAbs = _inherit(SVGPathSeg);
	var SVGPathSegLinetoHorizontalAbs = _inherit(SVGPathSeg);
	var MSCSSRuleList = new Array();
	var CanvasRenderingContext2D = {};
	var DOML2DeprecatedAlignmentStyle_HTMLTableRowElement = {};
	var HTMLTableRowElement = _inherit(HTMLElement);
	var HTMLScriptElement = _inherit(HTMLElement);
	var MessageEvent = _inherit(Event);
	var SVGDocument = {};
	var DocumentRange = {};
	var Document = _inherit(Node);
	var KeyboardEvent = _inherit(UIEvent);
	var CanvasGradient = {};
	var HTMLSourceElement = _inherit(HTMLElement);
	var HTMLButtonElement = _inherit(HTMLElement);
	var SVGAngle = {};
	var HTMLAreaElement = _inherit(HTMLElement);
	var DOML2DeprecatedAlignmentStyle_HTMLImageElement = {};
	var HTMLImageElement = _inherit(HTMLElement);
	var HTMLCollection = new Array();
	var StyleSheetPageList = new Array();
	var MSCSSStyleRuleExtensions = {};
	var MSSiteModeEvent = _inherit(Event);
	var SVGAnimatedPreserveAspectRatio = {};
	var WindowSessionStorage = {};
	var WindowTimers = {};
	var Window = this;
	var SVGAnimatedLengthList = {};
	var SVGPointList = {};
	var MouseWheelEvent = _inherit(MouseEvent);
	var HTMLMenuElement = _inherit(HTMLElement);
	var HTMLMapElement = _inherit(HTMLElement);
	var HTMLOptionElement = _inherit(HTMLElement);
	var MSHTMLTableCaptionElementExtensions = {};
	var HTMLTableCaptionElement = _inherit(HTMLElement);
	var SVGAnimatedTransformList = {};
	var MSNamespaceInfo = {};
	var ControlRangeCollection = new Array();
	var SVGTitleElement = _inherit(SVGElement);
	var HTMLFontElement = _inherit(HTMLElement);
	var MSHTMLTableElementExtensions = {};
	var MSHTMLAppletElementExtensions = {};
	var HTMLLinkElement = _inherit(HTMLElement);
	var SVGViewElement = _inherit(SVGElement);
	var NodeIterator = {};
	var CSSStyleRule = _inherit(CSSRule);
	var HTMLDDElement = _inherit(HTMLElement);
	var SVGScriptElement = _inherit(SVGElement);
	var Selection = {};
	var SVGAnimatedAngle = {};
	var SVGPatternElement = _inherit(SVGElement);
	var HTMLMetaElement = _inherit(HTMLElement);
	var MSSelection = {};
	var MSCSSStyleSheetExtensions = {};
	var CSSStyleSheet = _inherit(StyleSheet);
	var DOML2DeprecatedWidthStyle_HTMLBlockElement = {};
	var HTMLBlockElement = _inherit(HTMLElement);
	var TextRange = {};
	var HTMLSelectElement = _inherit(HTMLElement);
	var StyleMedia = {};
	var CDATASection = _inherit(Text);
	var SVGAnimatedString = {};
	var SVGPathSegLinetoVerticalRel = _inherit(SVGPathSeg);
	var HTMLOListElement = _inherit(HTMLElement);
	var TextMetrics = {};
	var HTMLAppletElement = _inherit(HTMLElement);
	var RangeException = {};
	var DOML2DeprecatedAlignmentStyle_HTMLTableElement = {};
	var SVGClipPathElement = _inherit(SVGElement);
	var MSScriptHost = {};
	var SVGPathSegCurvetoQuadraticSmoothRel = _inherit(SVGPathSeg);
	var SVGDescElement = _inherit(SVGElement);
	var HTMLAreasCollection = _inherit(HTMLCollection);
	var ErrorFunction = {};
	var DOML2DeprecatedAlignmentStyle_HTMLParagraphElement = {};
	var HTMLParagraphElement = _inherit(HTMLElement);
	var SVGLineElement = _inherit(SVGElement);
	var SVGPathSegMovetoRel = _inherit(SVGPathSeg);
	var HTMLNextIdElement = _inherit(HTMLElement);
	var DOMImplementation = {};
	var PositionErrorCallback = {};
	var ClientRect = {};
	var HTMLBaseElement = _inherit(HTMLElement);
	var HTMLTableDataCellElement = _inherit(HTMLTableCellElement);
	var SVGZoomEvent = _inherit(UIEvent);
	var SVGPathSegCurvetoCubicSmoothAbs = _inherit(SVGPathSeg);
	var NavigatorID = {};
	var Navigator = {};
	var MSStyleCSSProperties = _inherit(MSCSSProperties);
	var SVGGElement = _inherit(SVGElement);
	var SVGMarkerElement = _inherit(SVGElement);
	var CompositionEvent = _inherit(UIEvent);
	var MSDataBindingTableExtensions = {};
	var Performance = {};
	var CanvasPixelArray = {};
	var SVGPathSegCurvetoQuadraticRel = _inherit(SVGPathSeg);
	var TreeWalker = {};
	var HTMLTableElement = _inherit(HTMLElement);

	/* -- type: ElementTraversal -- */

	ElementTraversal.previousElementSibling = HTMLElement;
	Object.defineProperty(ElementTraversal,"childElementCount", { get: function () { return _childCount(); } });
	ElementTraversal.nextElementSibling = HTMLElement;
	Object.defineProperty(ElementTraversal,"lastElementChild", { get: function () { return _lastChild(); } });
	Object.defineProperty(ElementTraversal,"firstElementChild", { get: function () { return _firstChild(); } });


	/* -- type: SVGElementEventHandlers -- */

	_events(SVGElementEventHandlers, "onmouseover", "onmouseout", "onmousemove", "ondblclick", "onfocusout", "onfocusin", "onmousedown", "onload", "onmouseup", "onclick");


	/* -- type: MSElementExtensions -- */

	MSElementExtensions.msMatchesSelector = function(selectors) { 
		/// <signature>
		/// <param name='selectors' type='String' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	MSElementExtensions.fireEvent = function(eventName, eventObj) { 
		/// <signature>
		/// <param name='eventName' type='String' />
		/// <param name='eventObj' type='Object' optional='true' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};


	/* -- type: SVGLangSpace -- */

	SVGLangSpace.xmllang = '';
	SVGLangSpace.xmlspace = '';


	/* -- type: SVGStylable -- */

	SVGStylable.className = SVGAnimatedString;
	SVGStylable.style = CSSStyleDeclaration;


	/* -- type: EventTarget -- */

	EventTarget.removeEventListener = function(type, listener, useCapture) { 
		/// <signature>
		/// <param name='type' type='String' />
		/// <param name='listener' type='EventListener' />
		/// <param name='useCapture' type='Boolean' optional='true' />
		/// </signature>
	};
	EventTarget.addEventListener = function(type, listener, useCapture) { 
		/// <signature>
		/// <param name='type' type='String' />
		/// <param name='listener' type='EventListener' />
		/// <param name='useCapture' type='Boolean' optional='true' />
		/// </signature>
		_addEventListener(this, type, listener);
	};
	EventTarget.dispatchEvent = function(evt) { 
		/// <signature>
		/// <param name='evt' type='Event' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};


	/* -- type: Node -- */
	_implement(Node, EventTarget);

	Node.nodeType = 0;
	Node.localName = '';
	Node.previousSibling = Node;
	Node.textContent = '';
	Node.namespaceURI = '';
	Node.parentNode = Node;
	Object.defineProperty(Node,"lastChild", { get: function () { return _lastChild(); } });
	Node.nextSibling = Node;
	Node.nodeValue = '';
	Node.childNodes = NodeList;
	Node.ownerDocument = Document;
	Node.nodeName = '';
	Node.attributes = NamedNodeMap;
	Node.prefix = '';
	Object.defineProperty(Node,"firstChild", { get: function () { return _firstChild(); } });
	Node.ENTITY_REFERENCE_NODE = 5;
	Node.ATTRIBUTE_NODE = 2;
	Node.DOCUMENT_FRAGMENT_NODE = 11;
	Node.TEXT_NODE = 3;
	Node.ELEMENT_NODE = 1;
	Node.COMMENT_NODE = 8;
	Node.DOCUMENT_POSITION_DISCONNECTED = 0x01;
	Node.DOCUMENT_POSITION_CONTAINED_BY = 0x10;
	Node.DOCUMENT_POSITION_CONTAINS = 0x08;
	Node.DOCUMENT_TYPE_NODE = 10;
	Node.DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC = 0x20;
	Node.DOCUMENT_NODE = 9;
	Node.ENTITY_NODE = 6;
	Node.PROCESSING_INSTRUCTION_NODE = 7;
	Node.CDATA_SECTION_NODE = 4;
	Node.NOTATION_NODE = 12;
	Node.DOCUMENT_POSITION_PRECEDING = 0x02;
	Node.DOCUMENT_POSITION_FOLLOWING = 0x04;
	Node.removeChild = function(oldChild) { 
		/// <signature>
		/// <param name='oldChild' type='Node' />
		/// <returns type='Node'/>
		/// </signature>
		return Node; 
	};
	Node.appendChild = function(newChild) { 
		/// <signature>
		/// <param name='newChild' type='Node' />
		/// <returns type='Node'/>
		/// </signature>
		return _appendChild(newChild);
	};
	Node.isSupported = function(feature, version) { 
		/// <signature>
		/// <param name='feature' type='String' />
		/// <param name='version' type='String' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	Node.isEqualNode = function(arg) { 
		/// <signature>
		/// <param name='arg' type='Node' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	Node.lookupPrefix = function(namespaceURI) { 
		/// <signature>
		/// <param name='namespaceURI' type='String' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	Node.isDefaultNamespace = function(namespaceURI) { 
		/// <signature>
		/// <param name='namespaceURI' type='String' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	Node.compareDocumentPosition = function(other) { 
		/// <signature>
		/// <param name='other' type='Node' />
		/// <returns type='Number'/>
		/// </signature>
		return 0; 
	};
	Node.isSameNode = function(other) { 
		/// <signature>
		/// <param name='other' type='Node' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	Node.normalize = function() { };
	Node.lookupNamespaceURI = function(prefix) { 
		/// <signature>
		/// <param name='prefix' type='String' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	Node.hasAttributes = function() { 
		/// <signature>
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	Node.hasChildNodes = function() { 
		/// <signature>
		/// <returns type='Boolean'/>
		/// </signature>
		return _hasChildNodes();
	};
	Node.cloneNode = function(deep) { 
		/// <signature>
		/// <param name='deep' type='Boolean' optional='true' />
		/// <returns type='Node'/>
		/// </signature>
		return Node; 
	};
	Node.replaceChild = function(newChild, oldChild) { 
		/// <signature>
		/// <param name='newChild' type='Node' />
		/// <param name='oldChild' type='Node' />
		/// <returns type='Node'/>
		/// </signature>
		return _replaceChild(newChild, oldChild);
	};
	Node.insertBefore = function(newChild, refChild) { 
		/// <signature>
		/// <param name='newChild' type='Node' />
		/// <param name='refChild' type='Node' optional='true' />
		/// <returns type='Node'/>
		/// </signature>
		return _insertBefore(newChild, refChild);
	};


	/* -- type: NodeSelector -- */

	NodeSelector.querySelectorAll = function(selectors) { 
		/// <signature>
		/// <param name='selectors' type='String' />
		/// <returns type='NodeList'/>
		/// </signature>
		return NodeList; 
	};
	NodeSelector.querySelector = function(selectors) { 
		/// <signature>
		/// <param name='selectors' type='String' />
		/// <returns type='Element'/>
		/// </signature>
		return HTMLElement; 
	};


	/* -- type: Element -- */
	_implement(Element, NodeSelector);
	_implement(Element, ElementTraversal);
	_implement(Element, MSElementExtensions);

	Element.scrollTop = 0;
	Element.clientLeft = 0;
	Element.scrollLeft = 0;
	Element.clientWidth = 0;
	Element.tagName = '';
	Element.scrollWidth = 0;
	Element.clientHeight = 0;
	Element.clientTop = 0;
	Element.scrollHeight = 0;
	Element.getAttribute = function(name) { 
		/// <signature>
		/// <param name='name' type='String' optional='true' />
		/// <returns type='String'/>
		/// </signature>
		return _getAttribute(this, name);
	};
	Element.hasAttributeNS = function(namespaceURI, localName) { 
		/// <signature>
		/// <param name='namespaceURI' type='String' />
		/// <param name='localName' type='String' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	Element.getElementsByTagNameNS = function(namespaceURI, localName) { 
		/// <signature>
		/// <param name='namespaceURI' type='String' />
		/// <param name='localName' type='String' />
		/// <returns type='NodeList'/>
		/// </signature>
		return NodeList; 
	};
	Element.getBoundingClientRect = function() { 
		/// <signature>
		/// <returns type='ClientRect'/>
		/// </signature>
		return ClientRect; 
	};
	Element.getAttributeNS = function(namespaceURI, localName) { 
		/// <signature>
		/// <param name='namespaceURI' type='String' />
		/// <param name='localName' type='String' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	Element.getAttributeNodeNS = function(namespaceURI, localName) { 
		/// <signature>
		/// <param name='namespaceURI' type='String' />
		/// <param name='localName' type='String' />
		/// <returns type='Attr'/>
		/// </signature>
		return Attr; 
	};
	Element.setAttributeNodeNS = function(newAttr) { 
		/// <signature>
		/// <param name='newAttr' type='Attr' />
		/// <returns type='Attr'/>
		/// </signature>
		return Attr; 
	};
	Element.hasAttribute = function(name) { 
		/// <signature>
		/// <param name='name' type='String' />
		/// <returns type='Boolean'/>
		/// </signature>
		return _hasAttribute(this, name);
	};
	Element.removeAttribute = function(name) { 
		/// <signature>
		/// <param name='name' type='String' optional='true' />
		/// </signature>
	};
	Element.setAttributeNS = function(namespaceURI, qualifiedName, value) { 
		/// <signature>
		/// <param name='namespaceURI' type='String' />
		/// <param name='qualifiedName' type='String' />
		/// <param name='value' type='String' />
		/// </signature>
	};
	Element.getAttributeNode = function(name) { 
		/// <signature>
		/// <param name='name' type='String' />
		/// <returns type='Attr'/>
		/// </signature>
		return Attr; 
	};
	Element.getElementsByTagName = function(name) { 
		/// <signature>
		/// <param name='name' type='String' />
		/// <returns type='NodeList'/>
		/// </signature>
		return _getElementsByTagName(name);
	};
	Element.setAttributeNode = function(newAttr) { 
		/// <signature>
		/// <param name='newAttr' type='Attr' />
		/// <returns type='Attr'/>
		/// </signature>
		return Attr; 
	};
	Element.getClientRects = function() { 
		/// <signature>
		/// <returns type='ClientRectList'/>
		/// </signature>
		return ClientRectList; 
	};
	Element.removeAttributeNode = function(oldAttr) { 
		/// <signature>
		/// <param name='oldAttr' type='Attr' />
		/// <returns type='Attr'/>
		/// </signature>
		return Attr; 
	};
	Element.setAttribute = function(name, value) { 
		/// <signature>
		/// <param name='name' type='String' optional='true' />
		/// <param name='value' type='String' optional='true' />
		/// </signature>
		_setAttribute(this, name, value);
	};
	Element.removeAttributeNS = function(namespaceURI, localName) { 
		/// <signature>
		/// <param name='namespaceURI' type='String' />
		/// <param name='localName' type='String' />
		/// </signature>
	};


	/* -- type: SVGElement -- */
	_implement(SVGElement, SVGElementEventHandlers);

	SVGElement.viewportElement = SVGElement;
	SVGElement.xmlbase = '';
	SVGElement.ownerSVGElement = SVGSVGElement;
	SVGElement.id = '';


	/* -- type: SVGTests -- */

	SVGTests.requiredExtensions = SVGStringList;
	SVGTests.requiredFeatures = SVGStringList;
	SVGTests.systemLanguage = SVGStringList;
	SVGTests.hasExtension = function(extension) { 
		/// <signature>
		/// <param name='extension' type='String' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};


	/* -- type: SVGUnitTypes -- */

	SVGUnitTypes.SVG_UNIT_TYPE_OBJECTBOUNDINGBOX = 2;
	SVGUnitTypes.SVG_UNIT_TYPE_UNKNOWN = 0;
	SVGUnitTypes.SVG_UNIT_TYPE_USERSPACEONUSE = 1;


	/* -- type: SVGMaskElement -- */
	_implement(SVGMaskElement, SVGStylable);
	_implement(SVGMaskElement, SVGUnitTypes);
	_implement(SVGMaskElement, SVGLangSpace);
	_implement(SVGMaskElement, SVGTests);

	SVGMaskElement.width = SVGAnimatedLength;
	SVGMaskElement.y = SVGAnimatedLength;
	SVGMaskElement.maskUnits = SVGAnimatedEnumeration;
	SVGMaskElement.maskContentUnits = SVGAnimatedEnumeration;
	SVGMaskElement.x = SVGAnimatedLength;
	SVGMaskElement.height = SVGAnimatedLength;


	/* -- type: ClientRectList -- */

	ClientRectList.length = 0;
	ClientRectList.item = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='ClientRect'/>
		/// </signature>
		return ClientRect; 
	};
	/* Add a single array element */
	ClientRectList.push(ClientRect);


	/* -- type: DOML2DeprecatedAlignmentStyle_HTMLTableCellElement -- */

	DOML2DeprecatedAlignmentStyle_HTMLTableCellElement.align = '';


	/* -- type: MSXMLHttpRequestExtensions -- */

	MSXMLHttpRequestExtensions.responseBody = new Object();
	MSXMLHttpRequestExtensions.timeout = 0;
	_events(MSXMLHttpRequestExtensions, "ontimeout");


	/* -- type: MSElementCSSInlineStyleExtensions -- */

	MSElementCSSInlineStyleExtensions.doScroll = function(component) { 
		/// <signature>
		/// <param name='component' type='Object' optional='true' />
		/// </signature>
	};
	MSElementCSSInlineStyleExtensions.componentFromPoint = function(x, y) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};


	/* -- type: ElementCSSInlineStyle -- */
	_implement(ElementCSSInlineStyle, MSElementCSSInlineStyleExtensions);

	ElementCSSInlineStyle.runtimeStyle = MSStyleCSSProperties;
	ElementCSSInlineStyle.currentStyle = MSCurrentStyleCSSProperties;


	/* -- type: MSHTMLElementExtensions -- */

	MSHTMLElementExtensions.behaviorUrns = MSBehaviorUrnsCollection;
	MSHTMLElementExtensions.document = HTMLDocument;
	MSHTMLElementExtensions.filters = new Object();
	MSHTMLElementExtensions.children = HTMLCollection;
	MSHTMLElementExtensions.scopeName = '';
	MSHTMLElementExtensions.uniqueID = '';
	MSHTMLElementExtensions.isMultiLine = false;
	MSHTMLElementExtensions.uniqueNumber = 0;
	MSHTMLElementExtensions.tagUrn = '';
	MSHTMLElementExtensions.hideFocus = false;
	MSHTMLElementExtensions.recordNumber = new Object();
	MSHTMLElementExtensions.parentTextEdit = HTMLElement;
	MSHTMLElementExtensions.outerText = '';
	MSHTMLElementExtensions.readyState = '';
	MSHTMLElementExtensions.isDisabled = false;
	MSHTMLElementExtensions.isTextEdit = false;
	MSHTMLElementExtensions.all = HTMLCollection;
	MSHTMLElementExtensions.canHaveHTML = false;
	MSHTMLElementExtensions.innerText = '';
	MSHTMLElementExtensions.language = '';
	MSHTMLElementExtensions.parentElement = HTMLElement;
	MSHTMLElementExtensions.canHaveChildren = false;
	MSHTMLElementExtensions.sourceIndex = 0;
	MSHTMLElementExtensions.dragDrop = function() { 
		/// <signature>
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	MSHTMLElementExtensions.setCapture = function(containerCapture) { 
		/// <signature>
		/// <param name='containerCapture' type='Boolean' optional='true' />
		/// </signature>
	};
	MSHTMLElementExtensions.addFilter = function(filter) { 
		/// <signature>
		/// <param name='filter' type='Object' />
		/// </signature>
	};
	MSHTMLElementExtensions.releaseCapture = function() { };
	MSHTMLElementExtensions.removeBehavior = function(cookie) { 
		/// <signature>
		/// <param name='cookie' type='Number' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	MSHTMLElementExtensions.contains = function(child) { 
		/// <signature>
		/// <param name='child' type='HTMLElement' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	MSHTMLElementExtensions.getAdjacentText = function(where) { 
		/// <signature>
		/// <param name='where' type='String' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	MSHTMLElementExtensions.insertAdjacentText = function(where, text) { 
		/// <signature>
		/// <param name='where' type='String' />
		/// <param name='text' type='String' />
		/// </signature>
	};
	MSHTMLElementExtensions.insertAdjacentElement = function(position, insertedElement) { 
		/// <signature>
		/// <param name='position' type='String' />
		/// <param name='insertedElement' type='Element' />
		/// <returns type='Element'/>
		/// </signature>
		return HTMLElement; 
	};
	MSHTMLElementExtensions.mergeAttributes = function(source, preserveIdentity) { 
		/// <signature>
		/// <param name='source' type='HTMLElement' />
		/// <param name='preserveIdentity' type='Boolean' optional='true' />
		/// </signature>
	};
	MSHTMLElementExtensions.applyElement = function(apply, where) { 
		/// <signature>
		/// <param name='apply' type='Element' />
		/// <param name='where' type='String' optional='true' />
		/// <returns type='Element'/>
		/// </signature>
		return _applyElement(apply, where);
	};
	MSHTMLElementExtensions.replaceAdjacentText = function(where, newText) { 
		/// <signature>
		/// <param name='where' type='String' />
		/// <param name='newText' type='String' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	MSHTMLElementExtensions.setActive = function() { };
	MSHTMLElementExtensions.removeFilter = function(filter) { 
		/// <signature>
		/// <param name='filter' type='Object' />
		/// </signature>
	};
	MSHTMLElementExtensions.addBehavior = function(bstrUrl, factory) { 
		/// <signature>
		/// <param name='bstrUrl' type='String' />
		/// <param name='factory' type='Object' optional='true' />
		/// <returns type='Number'/>
		/// </signature>
		return 0; 
	};
	MSHTMLElementExtensions.clearAttributes = function() { };
	_events(MSHTMLElementExtensions, "onrowexit", "onlosecapture", "onrowsinserted", "oncontrolselect", "onmouseleave", "onpropertychange", "onbeforecut", "onbeforepaste", "onmove", "onafterupdate", "onresizeend", "onlayoutcomplete", "onbeforecopy", "onhelp", "onbeforeactivate", "onfocusout", "ondataavailable", "onfocusin", "onfilterchange", "onbeforeupdate", "ondatasetcomplete", "onresizestart", "onbeforedeactivate", "onactivate", "onmovestart", "onmouseenter", "onselectstart", "onpaste", "onerrorupdate", "ondeactivate", "onresize", "onmoveend", "oncut", "ondatasetchanged", "onrowsdelete", "oncopy", "onrowenter", "onbeforeeditfocus", "oncellchange");


	/* -- type: MSNodeExtensions -- */

	MSNodeExtensions.swapNode = function(otherNode) { 
		/// <signature>
		/// <param name='otherNode' type='Node' />
		/// <returns type='Node'/>
		/// </signature>
		return Node; 
	};
	MSNodeExtensions.removeNode = function(deep) { 
		/// <signature>
		/// <param name='deep' type='Boolean' optional='true' />
		/// <returns type='Node'/>
		/// </signature>
		return Node; 
	};
	MSNodeExtensions.replaceNode = function(replacement) { 
		/// <signature>
		/// <param name='replacement' type='Node' />
		/// <returns type='Node'/>
		/// </signature>
		return _replaceChild(replacement, this);
	};


	/* -- type: MSEventAttachmentTarget -- */

	MSEventAttachmentTarget.attachEvent = function(event, listener) { 
		/// <signature>
		/// <param name='event' type='String' />
		/// <param name='listener' type='EventListener' />
		/// <returns type='Boolean'/>
		/// </signature>
		_addEventListener(this, event, listener, true);
		return false; 
	};
	MSEventAttachmentTarget.detachEvent = function(event, listener) { 
		/// <signature>
		/// <param name='event' type='String' />
		/// <param name='listener' type='EventListener' />
		/// </signature>
	};


	/* -- type: MSHTMLElementRangeExtensions -- */

	MSHTMLElementRangeExtensions.createControlRange = function() { 
		/// <signature>
		/// <returns type='ControlRangeCollection'/>
		/// </signature>
		return ControlRangeCollection; 
	};


	/* -- type: HTMLElement -- */
	_implement(HTMLElement, MSHTMLElementRangeExtensions);
	_implement(HTMLElement, MSEventAttachmentTarget);
	_implement(HTMLElement, ElementCSSInlineStyle);
	_implement(HTMLElement, MSHTMLElementExtensions);
	_implement(HTMLElement, MSNodeExtensions);

	HTMLElement.offsetTop = 0;
	HTMLElement.innerHTML = '';
	HTMLElement.lang = '';
	HTMLElement.className = '';
	HTMLElement.title = '';
	HTMLElement.outerHTML = '';
	HTMLElement.offsetLeft = 0;
	HTMLElement.offsetHeight = 0;
	HTMLElement.dir = '';
	HTMLElement.style = MSStyleCSSProperties;
	HTMLElement.isContentEditable = false;
	HTMLElement.contentEditable = '';
	HTMLElement.tabIndex = 0;
	HTMLElement.id = '';
	HTMLElement.offsetParent = HTMLElement;
	HTMLElement.disabled = false;
	HTMLElement.accessKey = '';
	HTMLElement.offsetWidth = 0;
	HTMLElement.click = function() { };
	HTMLElement.scrollIntoView = function(top) { 
		/// <signature>
		/// <param name='top' type='Boolean' optional='true' />
		/// </signature>
	};
	HTMLElement.getElementsByClassName = function(classNames) { 
		/// <signature>
		/// <param name='classNames' type='String' />
		/// <returns type='NodeList'/>
		/// </signature>
		return NodeList; 
	};
	HTMLElement.blur = function() { };
	HTMLElement.focus = function() { };
	HTMLElement.insertAdjacentHTML = function(where, html) { 
		/// <signature>
		/// <param name='where' type='String' />
		/// <param name='html' type='String' />
		/// </signature>
	};
	_events(HTMLElement, "ondragend", "onkeydown", "ondragover", "onkeyup", "onreset", "onmouseup", "ondragstart", "ondrag", "onmouseover", "ondragleave", "onpause", "onseeked", "onmousedown", "onclick", "onwaiting", "ondurationchange", "onblur", "onemptied", "onseeking", "oncanplay", "onstalled", "onmousemove", "onratechange", "onloadstart", "ondragenter", "onsubmit", "onprogress", "ondblclick", "oncontextmenu", "onchange", "onloadedmetadata", "onerror", "onplay", "onplaying", "oncanplaythrough", "onabort", "onreadystatechange", "onkeypress", "onloadeddata", "onsuspend", "onfocus", "ontimeupdate", "onselect", "ondrop", "onmouseout", "onended", "onscroll", "onmousewheel", "onvolumechange", "onload", "oninput");


	/* -- type: HTMLMediaElement -- */

	HTMLMediaElement.played = TimeRanges;
	HTMLMediaElement.initialTime = 0;
	HTMLMediaElement.currentSrc = '';
	HTMLMediaElement.readyState = new Object();
	HTMLMediaElement.loop = false;
	HTMLMediaElement.autobuffer = false;
	HTMLMediaElement.ended = false;
	HTMLMediaElement.error = MediaError;
	HTMLMediaElement.buffered = TimeRanges;
	HTMLMediaElement.controls = false;
	HTMLMediaElement.autoplay = false;
	HTMLMediaElement.seekable = TimeRanges;
	HTMLMediaElement.volume = 0;
	HTMLMediaElement.src = '';
	HTMLMediaElement.playbackRate = 0;
	HTMLMediaElement.muted = false;
	HTMLMediaElement.duration = 0;
	HTMLMediaElement.paused = false;
	HTMLMediaElement.defaultPlaybackRate = 0;
	HTMLMediaElement.seeking = false;
	HTMLMediaElement.currentTime = 0;
	HTMLMediaElement.preload = '';
	HTMLMediaElement.networkState = 0;
	HTMLMediaElement.HAVE_CURRENT_DATA = 2;
	HTMLMediaElement.HAVE_METADATA = 1;
	HTMLMediaElement.HAVE_NOTHING = 0;
	HTMLMediaElement.NETWORK_NO_SOURCE = 3;
	HTMLMediaElement.HAVE_ENOUGH_DATA = 4;
	HTMLMediaElement.NETWORK_EMPTY = 0;
	HTMLMediaElement.NETWORK_LOADING = 2;
	HTMLMediaElement.NETWORK_IDLE = 1;
	HTMLMediaElement.HAVE_FUTURE_DATA = 3;
	HTMLMediaElement.pause = function() { };
	HTMLMediaElement.play = function() { };
	HTMLMediaElement.canPlayType = function(type) { 
		/// <signature>
		/// <param name='type' type='String' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	HTMLMediaElement.load = function() { };


	/* -- type: HTMLVideoElement -- */

	HTMLVideoElement.videoWidth = 0;
	HTMLVideoElement.width = 0;
	HTMLVideoElement.videoHeight = 0;
	HTMLVideoElement.height = 0;
	HTMLVideoElement.poster = '';


	/* -- type: LinkStyle -- */

	LinkStyle.sheet = StyleSheet;


	/* -- type: MSDataBindingRecordSetExtensions -- */

	MSDataBindingRecordSetExtensions.recordset = new Object();
	MSDataBindingRecordSetExtensions.namedRecordset = function(dataMember, hierarchy) { 
		/// <signature>
		/// <param name='dataMember' type='String' />
		/// <param name='hierarchy' type='Object' optional='true' />
		/// <returns type='Object'/>
		/// </signature>
		return new Object(); 
	};


	/* -- type: CSSRuleList -- */

	CSSRuleList.length = 0;
	CSSRuleList.item = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='CSSRule'/>
		/// </signature>
		return CSSRule; 
	};
	/* Add a single array element */
	CSSRuleList.push(CSSRule);


	/* -- type: SVGFitToViewBox -- */

	SVGFitToViewBox.viewBox = SVGAnimatedRect;
	SVGFitToViewBox.preserveAspectRatio = SVGAnimatedPreserveAspectRatio;


	/* -- type: SVGSymbolElement -- */
	_implement(SVGSymbolElement, SVGStylable);
	_implement(SVGSymbolElement, SVGLangSpace);
	_implement(SVGSymbolElement, SVGFitToViewBox);



	/* -- type: SVGElementInstanceList -- */

	SVGElementInstanceList.length = 0;
	SVGElementInstanceList.item = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='SVGElementInstance'/>
		/// </signature>
		return SVGElementInstance; 
	};
	/* Add a single array element */
	SVGElementInstanceList.push(SVGElementInstance);


	/* -- type: SVGStopElement -- */
	_implement(SVGStopElement, SVGStylable);

	SVGStopElement.offset = SVGAnimatedNumber;


	/* -- type: PositionCallback -- */

	PositionCallback.handleEvent = function(position) { 
		/// <signature>
		/// <param name='position' type='Position' />
		/// </signature>
	};


	/* -- type: HTMLBodyElementDOML2Deprecated -- */

	HTMLBodyElementDOML2Deprecated.link = new Object();
	HTMLBodyElementDOML2Deprecated.aLink = new Object();
	HTMLBodyElementDOML2Deprecated.text = new Object();
	HTMLBodyElementDOML2Deprecated.vLink = new Object();


	/* -- type: DOML2DeprecatedAlignmentStyle_HTMLTableCaptionElement -- */

	DOML2DeprecatedAlignmentStyle_HTMLTableCaptionElement.align = '';


	/* -- type: PerformanceNavigation -- */

	PerformanceNavigation.redirectCount = 0;
	PerformanceNavigation.type = 0;
	PerformanceNavigation.TYPE_RELOAD = 1;
	PerformanceNavigation.TYPE_RESERVED = 255;
	PerformanceNavigation.TYPE_BACK_FORWARD = 2;
	PerformanceNavigation.TYPE_NAVIGATE = 0;
	PerformanceNavigation.toJSON = function() { 
		/// <signature>
		/// <returns type='Object'/>
		/// </signature>
		return new Object(); 
	};


	/* -- type: MSAttrExtensions -- */

	MSAttrExtensions.expando = false;


	/* -- type: Attr -- */
	_implement(Attr, MSAttrExtensions);

	Attr.ownerElement = HTMLElement;
	Attr.specified = false;
	Attr.value = '';
	Attr.name = '';


	/* -- type: MSBorderColorStyle_HTMLTableRowElement -- */

	MSBorderColorStyle_HTMLTableRowElement.borderColor = new Object();


	/* -- type: SVGPreserveAspectRatio -- */

	SVGPreserveAspectRatio.align = 0;
	SVGPreserveAspectRatio.meetOrSlice = 0;
	SVGPreserveAspectRatio.SVG_PRESERVEASPECTRATIO_XMINYMID = 5;
	SVGPreserveAspectRatio.SVG_PRESERVEASPECTRATIO_NONE = 1;
	SVGPreserveAspectRatio.SVG_PRESERVEASPECTRATIO_XMAXYMIN = 4;
	SVGPreserveAspectRatio.SVG_PRESERVEASPECTRATIO_XMAXYMAX = 10;
	SVGPreserveAspectRatio.SVG_PRESERVEASPECTRATIO_XMINYMAX = 8;
	SVGPreserveAspectRatio.SVG_MEETORSLICE_UNKNOWN = 0;
	SVGPreserveAspectRatio.SVG_PRESERVEASPECTRATIO_XMINYMIN = 2;
	SVGPreserveAspectRatio.SVG_PRESERVEASPECTRATIO_XMAXYMID = 7;
	SVGPreserveAspectRatio.SVG_PRESERVEASPECTRATIO_XMIDYMAX = 9;
	SVGPreserveAspectRatio.SVG_MEETORSLICE_MEET = 1;
	SVGPreserveAspectRatio.SVG_PRESERVEASPECTRATIO_XMIDYMIN = 3;
	SVGPreserveAspectRatio.SVG_PRESERVEASPECTRATIO_XMIDYMID = 6;
	SVGPreserveAspectRatio.SVG_MEETORSLICE_SLICE = 2;
	SVGPreserveAspectRatio.SVG_PRESERVEASPECTRATIO_UNKNOWN = 0;


	/* -- type: DOML2DeprecatedMarginStyle_MSHTMLIFrameElementExtensions -- */

	DOML2DeprecatedMarginStyle_MSHTMLIFrameElementExtensions.vspace = 0;
	DOML2DeprecatedMarginStyle_MSHTMLIFrameElementExtensions.hspace = 0;


	/* -- type: DOML2DeprecatedBorderStyle_MSHTMLIFrameElementExtensions -- */

	DOML2DeprecatedBorderStyle_MSHTMLIFrameElementExtensions.border = '';


	/* -- type: MSHTMLIFrameElementExtensions -- */
	_implement(MSHTMLIFrameElementExtensions, DOML2DeprecatedBorderStyle_MSHTMLIFrameElementExtensions);
	_implement(MSHTMLIFrameElementExtensions, DOML2DeprecatedMarginStyle_MSHTMLIFrameElementExtensions);

	MSHTMLIFrameElementExtensions.noResize = false;
	MSHTMLIFrameElementExtensions.frameSpacing = new Object();
	_events(MSHTMLIFrameElementExtensions, "onload");


	/* -- type: SVGLocatable -- */

	SVGLocatable.farthestViewportElement = SVGElement;
	SVGLocatable.nearestViewportElement = SVGElement;
	SVGLocatable.getBBox = function() { 
		/// <signature>
		/// <returns type='SVGRect'/>
		/// </signature>
		return SVGRect; 
	};
	SVGLocatable.getTransformToElement = function(element) { 
		/// <signature>
		/// <param name='element' type='SVGElement' />
		/// <returns type='SVGMatrix'/>
		/// </signature>
		return SVGMatrix; 
	};
	SVGLocatable.getScreenCTM = function() { 
		/// <signature>
		/// <returns type='SVGMatrix'/>
		/// </signature>
		return SVGMatrix; 
	};
	SVGLocatable.getCTM = function() { 
		/// <signature>
		/// <returns type='SVGMatrix'/>
		/// </signature>
		return SVGMatrix; 
	};


	/* -- type: SVGTransformable -- */

	SVGTransformable.transform = SVGAnimatedTransformList;


	/* -- type: SVGSwitchElement -- */
	_implement(SVGSwitchElement, SVGStylable);
	_implement(SVGSwitchElement, SVGTransformable);
	_implement(SVGSwitchElement, SVGLangSpace);
	_implement(SVGSwitchElement, SVGTests);



	/* -- type: MSCompatibleInfoCollection -- */

	MSCompatibleInfoCollection.length = 0;
	MSCompatibleInfoCollection.item = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='MSCompatibleInfo'/>
		/// </signature>
		return MSCompatibleInfo; 
	};
	/* Add a single array element */
	MSCompatibleInfoCollection.push(MSCompatibleInfo);


	/* -- type: MSHTMLIsIndexElementExtensions -- */

	MSHTMLIsIndexElementExtensions.action = '';


	/* -- type: SVGAnimatedBoolean -- */

	SVGAnimatedBoolean.animVal = false;
	SVGAnimatedBoolean.baseVal = false;


	/* -- type: MSHTMLDocumentSelection -- */

	MSHTMLDocumentSelection.selection = MSSelection;


	/* -- type: DOMException -- */

	DOMException.message = '';
	DOMException.code = 0;
	DOMException.NO_MODIFICATION_ALLOWED_ERR = 7;
	DOMException.HIERARCHY_REQUEST_ERR = 3;
	DOMException.INVALID_MODIFICATION_ERR = 13;
	DOMException.NAMESPACE_ERR = 14;
	DOMException.INVALID_CHARACTER_ERR = 5;
	DOMException.TYPE_MISMATCH_ERR = 17;
	DOMException.ABORT_ERR = 20;
	DOMException.INVALID_STATE_ERR = 11;
	DOMException.SECURITY_ERR = 18;
	DOMException.NETWORK_ERR = 19;
	DOMException.WRONG_DOCUMENT_ERR = 4;
	DOMException.QUOTA_EXCEEDED_ERR = 22;
	DOMException.INDEX_SIZE_ERR = 1;
	DOMException.DOMSTRING_SIZE_ERR = 2;
	DOMException.SYNTAX_ERR = 12;
	DOMException.SERIALIZE_ERR = 82;
	DOMException.NOT_FOUND_ERR = 8;
	DOMException.VALIDATION_ERR = 16;
	DOMException.URL_MISMATCH_ERR = 21;
	DOMException.PARSE_ERR = 81;
	DOMException.NO_DATA_ALLOWED_ERR = 6;
	DOMException.NOT_SUPPORTED_ERR = 9;
	DOMException.INVALID_ACCESS_ERR = 15;
	DOMException.INUSE_ATTRIBUTE_ERR = 10;
	DOMException.toString = function() { 
		/// <signature>
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};


	/* -- type: HTMLIsIndexElement -- */
	_implement(HTMLIsIndexElement, MSHTMLIsIndexElementExtensions);

	HTMLIsIndexElement.form = HTMLFormElement;
	HTMLIsIndexElement.prompt = '';


	/* -- type: SVGPathSeg -- */

	SVGPathSeg.pathSegType = 0;
	SVGPathSeg.pathSegTypeAsLetter = '';
	SVGPathSeg.PATHSEG_CURVETO_CUBIC_SMOOTH_ABS = 16;
	SVGPathSeg.PATHSEG_LINETO_VERTICAL_REL = 15;
	SVGPathSeg.PATHSEG_MOVETO_REL = 3;
	SVGPathSeg.PATHSEG_CURVETO_QUADRATIC_REL = 9;
	SVGPathSeg.PATHSEG_CURVETO_CUBIC_ABS = 6;
	SVGPathSeg.PATHSEG_LINETO_HORIZONTAL_ABS = 12;
	SVGPathSeg.PATHSEG_CURVETO_QUADRATIC_ABS = 8;
	SVGPathSeg.PATHSEG_LINETO_ABS = 4;
	SVGPathSeg.PATHSEG_CLOSEPATH = 1;
	SVGPathSeg.PATHSEG_LINETO_HORIZONTAL_REL = 13;
	SVGPathSeg.PATHSEG_CURVETO_CUBIC_SMOOTH_REL = 17;
	SVGPathSeg.PATHSEG_LINETO_REL = 5;
	SVGPathSeg.PATHSEG_CURVETO_QUADRATIC_SMOOTH_ABS = 18;
	SVGPathSeg.PATHSEG_ARC_REL = 11;
	SVGPathSeg.PATHSEG_CURVETO_CUBIC_REL = 7;
	SVGPathSeg.PATHSEG_UNKNOWN = 0;
	SVGPathSeg.PATHSEG_LINETO_VERTICAL_ABS = 14;
	SVGPathSeg.PATHSEG_ARC_ABS = 10;
	SVGPathSeg.PATHSEG_MOVETO_ABS = 2;
	SVGPathSeg.PATHSEG_CURVETO_QUADRATIC_SMOOTH_REL = 19;


	/* -- type: SVGPathSegLinetoRel -- */

	SVGPathSegLinetoRel.y = 0;
	SVGPathSegLinetoRel.x = 0;


	/* -- type: MSHTMLOptGroupElementExtensions -- */

	MSHTMLOptGroupElementExtensions.index = 0;
	MSHTMLOptGroupElementExtensions.value = '';
	MSHTMLOptGroupElementExtensions.text = '';
	MSHTMLOptGroupElementExtensions.defaultSelected = false;
	MSHTMLOptGroupElementExtensions.form = HTMLFormElement;
	MSHTMLOptGroupElementExtensions.label = '';
	MSHTMLOptGroupElementExtensions.selected = false;


	/* -- type: MSDataBindingExtensions -- */

	MSDataBindingExtensions.dataSrc = '';
	MSDataBindingExtensions.dataFormatAs = '';
	MSDataBindingExtensions.dataFld = '';


	/* -- type: HTMLOptGroupElement -- */
	_implement(HTMLOptGroupElement, MSDataBindingExtensions);
	_implement(HTMLOptGroupElement, MSHTMLOptGroupElementExtensions);



	/* -- type: CharacterData -- */

	CharacterData.length = 0;
	CharacterData.data = '';
	CharacterData.replaceData = function(offset, count, arg) { 
		/// <signature>
		/// <param name='offset' type='Number' />
		/// <param name='count' type='Number' />
		/// <param name='arg' type='String' />
		/// </signature>
	};
	CharacterData.deleteData = function(offset, count) { 
		/// <signature>
		/// <param name='offset' type='Number' />
		/// <param name='count' type='Number' />
		/// </signature>
	};
	CharacterData.appendData = function(arg) { 
		/// <signature>
		/// <param name='arg' type='String' />
		/// </signature>
	};
	CharacterData.insertData = function(offset, arg) { 
		/// <signature>
		/// <param name='offset' type='Number' />
		/// <param name='arg' type='String' />
		/// </signature>
	};
	CharacterData.substringData = function(offset, count) { 
		/// <signature>
		/// <param name='offset' type='Number' />
		/// <param name='count' type='Number' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};


	/* -- type: DOML2DeprecatedAlignmentStyle_HTMLTableSectionElement -- */

	DOML2DeprecatedAlignmentStyle_HTMLTableSectionElement.align = '';


	/* -- type: DocumentView -- */

	DocumentView.defaultView = AbstractView;
	DocumentView.elementFromPoint = function(x, y) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// <returns type='Element'/>
		/// </signature>
		return HTMLElement; 
	};


	/* -- type: MSEventExtensions -- */

	MSEventExtensions.cancelBubble = false;
	MSEventExtensions.srcElement = HTMLElement;


	/* -- type: Event -- */
	_implement(Event, MSEventExtensions);

	Event.timeStamp = 0;
	Event.isTrusted = false;
	Event.defaultPrevented = false;
	Event.currentTarget = EventTarget;
	Event.target = EventTarget;
	Event.eventPhase = 0;
	Event.type = '';
	Event.cancelable = false;
	Event.bubbles = false;
	Event.AT_TARGET = 2;
	Event.CAPTURING_PHASE = 1;
	Event.BUBBLING_PHASE = 3;
	Event.initEvent = function(eventTypeArg, canBubbleArg, cancelableArg) { 
		/// <signature>
		/// <param name='eventTypeArg' type='String' />
		/// <param name='canBubbleArg' type='Boolean' />
		/// <param name='cancelableArg' type='Boolean' />
		/// </signature>
	};
	Event.stopPropagation = function() { };
	Event.stopImmediatePropagation = function() { };
	Event.preventDefault = function() { };


	/* -- type: StorageEvent -- */

	StorageEvent.newValue = new Object();
	StorageEvent.oldValue = new Object();
	StorageEvent.url = '';
	StorageEvent.storageArea = Storage;
	StorageEvent.key = '';
	StorageEvent.initStorageEvent = function(typeArg, canBubbleArg, cancelableArg, keyArg, oldValueArg, newValueArg, urlArg, storageAreaArg) { 
		/// <signature>
		/// <param name='typeArg' type='String' />
		/// <param name='canBubbleArg' type='Boolean' />
		/// <param name='cancelableArg' type='Boolean' />
		/// <param name='keyArg' type='String' />
		/// <param name='oldValueArg' type='Object' />
		/// <param name='newValueArg' type='Object' />
		/// <param name='urlArg' type='String' />
		/// <param name='storageAreaArg' type='Storage' />
		/// </signature>
	};


	/* -- type: MSHTMLEmbedElementExtensions -- */

	MSHTMLEmbedElementExtensions.palette = '';
	MSHTMLEmbedElementExtensions.pluginspage = '';
	MSHTMLEmbedElementExtensions.hidden = '';
	MSHTMLEmbedElementExtensions.units = '';


	/* -- type: GetSVGDocument -- */

	GetSVGDocument.getSVGDocument = function() { 
		/// <signature>
		/// <returns type='SVGDocument'/>
		/// </signature>
		return SVGDocument; 
	};


	/* -- type: HTMLEmbedElement -- */
	_implement(HTMLEmbedElement, GetSVGDocument);
	_implement(HTMLEmbedElement, MSHTMLEmbedElementExtensions);

	HTMLEmbedElement.width = '';
	HTMLEmbedElement.src = '';
	HTMLEmbedElement.name = '';
	HTMLEmbedElement.height = '';


	/* -- type: DOML2DeprecatedMarginStyle_HTMLObjectElement -- */

	DOML2DeprecatedMarginStyle_HTMLObjectElement.vspace = 0;
	DOML2DeprecatedMarginStyle_HTMLObjectElement.hspace = 0;


	/* -- type: DOML2DeprecatedAlignmentStyle_HTMLObjectElement -- */

	DOML2DeprecatedAlignmentStyle_HTMLObjectElement.align = '';


	/* -- type: MSHTMLObjectElementExtensions -- */

	MSHTMLObjectElementExtensions.object = new Object();
	MSHTMLObjectElementExtensions.alt = '';
	MSHTMLObjectElementExtensions.classid = '';
	MSHTMLObjectElementExtensions.BaseHref = '';
	MSHTMLObjectElementExtensions.altHtml = '';


	/* -- type: DOML2DeprecatedBorderStyle_HTMLObjectElement -- */

	DOML2DeprecatedBorderStyle_HTMLObjectElement.border = '';


	/* -- type: HTMLObjectElement -- */
	_implement(HTMLObjectElement, GetSVGDocument);
	_implement(HTMLObjectElement, MSHTMLObjectElementExtensions);
	_implement(HTMLObjectElement, DOML2DeprecatedMarginStyle_HTMLObjectElement);
	_implement(HTMLObjectElement, MSDataBindingRecordSetExtensions);
	_implement(HTMLObjectElement, MSDataBindingExtensions);
	_implement(HTMLObjectElement, DOML2DeprecatedBorderStyle_HTMLObjectElement);
	_implement(HTMLObjectElement, DOML2DeprecatedAlignmentStyle_HTMLObjectElement);

	HTMLObjectElement.codeType = '';
	HTMLObjectElement.width = '';
	HTMLObjectElement.standby = '';
	HTMLObjectElement.archive = '';
	HTMLObjectElement.form = HTMLFormElement;
	HTMLObjectElement.useMap = '';
	HTMLObjectElement.name = '';
	HTMLObjectElement.data = '';
	HTMLObjectElement.height = '';
	HTMLObjectElement.contentDocument = Document;
	HTMLObjectElement.declare = false;
	HTMLObjectElement.codeBase = '';
	HTMLObjectElement.type = '';
	HTMLObjectElement.code = '';


	/* -- type: HTMLHRElementDOML2Deprecated -- */

	HTMLHRElementDOML2Deprecated.noShade = false;


	/* -- type: DOML2DeprecatedColorProperty -- */

	DOML2DeprecatedColorProperty.color = '';


	/* -- type: MSHTMLHRElementExtensions -- */
	_implement(MSHTMLHRElementExtensions, DOML2DeprecatedColorProperty);



	/* -- type: DOML2DeprecatedWidthStyle_HTMLHRElement -- */

	DOML2DeprecatedWidthStyle_HTMLHRElement.width = 0;


	/* -- type: DOML2DeprecatedSizeProperty -- */

	DOML2DeprecatedSizeProperty.size = 0;


	/* -- type: DOML2DeprecatedAlignmentStyle_HTMLHRElement -- */

	DOML2DeprecatedAlignmentStyle_HTMLHRElement.align = '';


	/* -- type: HTMLHRElement -- */
	_implement(HTMLHRElement, DOML2DeprecatedWidthStyle_HTMLHRElement);
	_implement(HTMLHRElement, MSHTMLHRElementExtensions);
	_implement(HTMLHRElement, HTMLHRElementDOML2Deprecated);
	_implement(HTMLHRElement, DOML2DeprecatedSizeProperty);
	_implement(HTMLHRElement, DOML2DeprecatedAlignmentStyle_HTMLHRElement);



	/* -- type: MSHTMLFrameSetElementExtensions -- */

	MSHTMLFrameSetElementExtensions.name = '';
	MSHTMLFrameSetElementExtensions.frameBorder = '';
	MSHTMLFrameSetElementExtensions.frameSpacing = new Object();
	MSHTMLFrameSetElementExtensions.border = '';


	/* -- type: DOML2DeprecatedTextFlowControl_HTMLBlockElement -- */

	DOML2DeprecatedTextFlowControl_HTMLBlockElement.clear = '';


	/* -- type: PositionOptions -- */

	PositionOptions.enableHighAccuracy = false;
	PositionOptions.timeout = 0;
	PositionOptions.maximumAge = 0;


	/* -- type: CanvasPattern -- */



	/* -- type: MSCommentExtensions -- */

	MSCommentExtensions.text = '';


	/* -- type: Comment -- */
	_implement(Comment, MSCommentExtensions);



	/* -- type: HTMLBGSoundElement -- */

	HTMLBGSoundElement.volume = new Object();
	HTMLBGSoundElement.balance = new Object();
	HTMLBGSoundElement.src = '';
	HTMLBGSoundElement.loop = 0;


	/* -- type: DOML2DeprecatedAlignmentStyle_HTMLFieldSetElement -- */

	DOML2DeprecatedAlignmentStyle_HTMLFieldSetElement.align = '';


	/* -- type: MSHTMLFieldSetElementExtensions -- */
	_implement(MSHTMLFieldSetElementExtensions, DOML2DeprecatedAlignmentStyle_HTMLFieldSetElement);



	/* -- type: HTMLFieldSetElement -- */
	_implement(HTMLFieldSetElement, MSHTMLFieldSetElementExtensions);

	HTMLFieldSetElement.form = HTMLFormElement;


	/* -- type: MediaError -- */

	MediaError.code = 0;
	MediaError.MEDIA_ERR_SRC_NOT_SUPPORTED = 4;
	MediaError.MEDIA_ERR_NETWORK = 2;
	MediaError.MEDIA_ERR_ABORTED = 1;
	MediaError.MEDIA_ERR_DECODE = 3;


	/* -- type: SVGNumberList -- */

	SVGNumberList.numberOfItems = 0;
	SVGNumberList.replaceItem = function(newItem, index) { 
		/// <signature>
		/// <param name='newItem' type='SVGNumber' />
		/// <param name='index' type='Number' />
		/// <returns type='SVGNumber'/>
		/// </signature>
		return SVGNumber; 
	};
	SVGNumberList.getItem = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='SVGNumber'/>
		/// </signature>
		return SVGNumber; 
	};
	SVGNumberList.appendItem = function(newItem) { 
		/// <signature>
		/// <param name='newItem' type='SVGNumber' />
		/// <returns type='SVGNumber'/>
		/// </signature>
		return SVGNumber; 
	};
	SVGNumberList.clear = function() { };
	SVGNumberList.removeItem = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='SVGNumber'/>
		/// </signature>
		return SVGNumber; 
	};
	SVGNumberList.initialize = function(newItem) { 
		/// <signature>
		/// <param name='newItem' type='SVGNumber' />
		/// <returns type='SVGNumber'/>
		/// </signature>
		return SVGNumber; 
	};
	SVGNumberList.insertItemBefore = function(newItem, index) { 
		/// <signature>
		/// <param name='newItem' type='SVGNumber' />
		/// <param name='index' type='Number' />
		/// <returns type='SVGNumber'/>
		/// </signature>
		return SVGNumber; 
	};


	/* -- type: DOML2DeprecatedMarginStyle_HTMLInputElement -- */

	DOML2DeprecatedMarginStyle_HTMLInputElement.vspace = 0;
	DOML2DeprecatedMarginStyle_HTMLInputElement.hspace = 0;


	/* -- type: DOML2DeprecatedBackgroundColorStyle -- */

	DOML2DeprecatedBackgroundColorStyle.bgColor = new Object();


	/* -- type: MSHTMLTableSectionElementExtensions -- */
	_implement(MSHTMLTableSectionElementExtensions, DOML2DeprecatedBackgroundColorStyle);

	MSHTMLTableSectionElementExtensions.moveRow = function(indexFrom, indexTo) { 
		/// <signature>
		/// <param name='indexFrom' type='Number' optional='true' />
		/// <param name='indexTo' type='Number' optional='true' />
		/// <returns type='Object'/>
		/// </signature>
		return new Object(); 
	};


	/* -- type: AbstractView -- */

	AbstractView.styleMedia = StyleMedia;
	AbstractView.document = DocumentView;


	/* -- type: ScreenView -- */

	ScreenView.outerWidth = 0;
	ScreenView.pageXOffset = 0;
	ScreenView.innerWidth = 0;
	ScreenView.pageYOffset = 0;
	ScreenView.screenY = 0;
	ScreenView.outerHeight = 0;
	ScreenView.screen = Screen;
	ScreenView.innerHeight = 0;
	ScreenView.screenX = 0;
	ScreenView.scrollBy = function(x, y) { 
		/// <signature>
		/// <param name='x' type='Number' optional='true' />
		/// <param name='y' type='Number' optional='true' />
		/// </signature>
	};
	ScreenView.scroll = function(x, y) { 
		/// <signature>
		/// <param name='x' type='Number' optional='true' />
		/// <param name='y' type='Number' optional='true' />
		/// </signature>
	};
	ScreenView.scrollTo = function(x, y) { 
		/// <signature>
		/// <param name='x' type='Number' optional='true' />
		/// <param name='y' type='Number' optional='true' />
		/// </signature>
	};


	/* -- type: DOML2DeprecatedWordWrapSuppression_HTMLDTElement -- */

	DOML2DeprecatedWordWrapSuppression_HTMLDTElement.noWrap = false;


	/* -- type: MSBorderColorStyle_HTMLFrameElement -- */

	MSBorderColorStyle_HTMLFrameElement.borderColor = new Object();


	/* -- type: NodeFilter -- */

	NodeFilter.SHOW_NOTATION = 0x00000800;
	NodeFilter.SHOW_ENTITY_REFERENCE = 0x00000010;
	NodeFilter.SHOW_DOCUMENT = 0x00000100;
	NodeFilter.SHOW_ENTITY = 0x00000020;
	NodeFilter.SHOW_PROCESSING_INSTRUCTION = 0x00000040;
	NodeFilter.FILTER_REJECT = 2;
	NodeFilter.SHOW_CDATA_SECTION = 0x00000008;
	NodeFilter.SHOW_DOCUMENT_TYPE = 0x00000200;
	NodeFilter.SHOW_ALL = 0xFFFFFFFF;
	NodeFilter.FILTER_ACCEPT = 1;
	NodeFilter.SHOW_TEXT = 0x00000004;
	NodeFilter.SHOW_ELEMENT = 0x00000001;
	NodeFilter.SHOW_COMMENT = 0x00000080;
	NodeFilter.SHOW_ATTRIBUTE = 0x00000002;
	NodeFilter.FILTER_SKIP = 3;
	NodeFilter.SHOW_DOCUMENT_FRAGMENT = 0x00000400;
	NodeFilter.acceptNode = function(n) { 
		/// <signature>
		/// <param name='n' type='Node' />
		/// <returns type='Number'/>
		/// </signature>
		return 0; 
	};


	/* -- type: DOML2DeprecatedTextFlowControl_HTMLBRElement -- */

	DOML2DeprecatedTextFlowControl_HTMLBRElement.clear = '';


	/* -- type: MSHTMLParagraphElementExtensions -- */
	_implement(MSHTMLParagraphElementExtensions, DOML2DeprecatedTextFlowControl_HTMLBlockElement);



	/* -- type: SVGURIReference -- */

	SVGURIReference.href = SVGAnimatedString;


	/* -- type: SVGGradientElement -- */
	_implement(SVGGradientElement, SVGStylable);
	_implement(SVGGradientElement, SVGUnitTypes);
	_implement(SVGGradientElement, SVGURIReference);

	SVGGradientElement.spreadMethod = SVGAnimatedEnumeration;
	SVGGradientElement.gradientTransform = SVGAnimatedTransformList;
	SVGGradientElement.gradientUnits = SVGAnimatedEnumeration;
	SVGGradientElement.SVG_SPREADMETHOD_REFLECT = 2;
	SVGGradientElement.SVG_SPREADMETHOD_PAD = 1;
	SVGGradientElement.SVG_SPREADMETHOD_UNKNOWN = 0;
	SVGGradientElement.SVG_SPREADMETHOD_REPEAT = 3;


	/* -- type: MSHTMLTableRowElementExtensions -- */

	MSHTMLTableRowElementExtensions.height = new Object();


	/* -- type: DOML2DeprecatedWordWrapSuppression_HTMLDDElement -- */

	DOML2DeprecatedWordWrapSuppression_HTMLDDElement.noWrap = false;


	/* -- type: StyleSheetPage -- */

	StyleSheetPage.pseudoClass = '';
	StyleSheetPage.selector = '';


	/* -- type: XMLSerializer -- */

	XMLSerializer.serializeToString = function(target) { 
		/// <signature>
		/// <param name='target' type='Node' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};


	/* -- type: NodeList -- */

	NodeList.length = 0;
	NodeList.item = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='Node'/>
		/// </signature>
		return Node; 
	};
	/* Add a single array element */
	NodeList.push(Node);


	/* -- type: HTMLDTElement -- */
	_implement(HTMLDTElement, DOML2DeprecatedWordWrapSuppression_HTMLDTElement);



	/* -- type: SVGTextContentElement -- */
	_implement(SVGTextContentElement, SVGStylable);
	_implement(SVGTextContentElement, SVGLangSpace);
	_implement(SVGTextContentElement, SVGTests);

	SVGTextContentElement.textLength = SVGAnimatedLength;
	SVGTextContentElement.lengthAdjust = SVGAnimatedEnumeration;
	SVGTextContentElement.LENGTHADJUST_SPACING = 1;
	SVGTextContentElement.LENGTHADJUST_SPACINGANDGLYPHS = 2;
	SVGTextContentElement.LENGTHADJUST_UNKNOWN = 0;
	SVGTextContentElement.getCharNumAtPosition = function(point) { 
		/// <signature>
		/// <param name='point' type='SVGPoint' />
		/// <returns type='Number'/>
		/// </signature>
		return 0; 
	};
	SVGTextContentElement.getStartPositionOfChar = function(charnum) { 
		/// <signature>
		/// <param name='charnum' type='Number' />
		/// <returns type='SVGPoint'/>
		/// </signature>
		return SVGPoint; 
	};
	SVGTextContentElement.getExtentOfChar = function(charnum) { 
		/// <signature>
		/// <param name='charnum' type='Number' />
		/// <returns type='SVGRect'/>
		/// </signature>
		return SVGRect; 
	};
	SVGTextContentElement.selectSubString = function(charnum, nchars) { 
		/// <signature>
		/// <param name='charnum' type='Number' />
		/// <param name='nchars' type='Number' />
		/// </signature>
	};
	SVGTextContentElement.getSubStringLength = function(charnum, nchars) { 
		/// <signature>
		/// <param name='charnum' type='Number' />
		/// <param name='nchars' type='Number' />
		/// <returns type='Number'/>
		/// </signature>
		return 0; 
	};
	SVGTextContentElement.getComputedTextLength = function() { 
		/// <signature>
		/// <returns type='Number'/>
		/// </signature>
		return 0; 
	};
	SVGTextContentElement.getNumberOfChars = function() { 
		/// <signature>
		/// <returns type='Number'/>
		/// </signature>
		return 0; 
	};
	SVGTextContentElement.getRotationOfChar = function(charnum) { 
		/// <signature>
		/// <param name='charnum' type='Number' />
		/// <returns type='Number'/>
		/// </signature>
		return 0; 
	};
	SVGTextContentElement.getEndPositionOfChar = function(charnum) { 
		/// <signature>
		/// <param name='charnum' type='Number' />
		/// <returns type='SVGPoint'/>
		/// </signature>
		return SVGPoint; 
	};


	/* -- type: SVGTextPathElement -- */
	_implement(SVGTextPathElement, SVGURIReference);

	SVGTextPathElement.startOffset = SVGAnimatedLength;
	SVGTextPathElement.method = SVGAnimatedEnumeration;
	SVGTextPathElement.spacing = SVGAnimatedEnumeration;
	SVGTextPathElement.TEXTPATH_SPACINGTYPE_EXACT = 2;
	SVGTextPathElement.TEXTPATH_SPACINGTYPE_AUTO = 1;
	SVGTextPathElement.TEXTPATH_METHODTYPE_STRETCH = 2;
	SVGTextPathElement.TEXTPATH_SPACINGTYPE_UNKNOWN = 0;
	SVGTextPathElement.TEXTPATH_METHODTYPE_ALIGN = 1;
	SVGTextPathElement.TEXTPATH_METHODTYPE_UNKNOWN = 0;


	/* -- type: DOML2DeprecatedWidthStyle_HTMLAppletElement -- */

	DOML2DeprecatedWidthStyle_HTMLAppletElement.width = 0;


	/* -- type: DOML2DeprecatedBorderStyle_HTMLTableElement -- */

	DOML2DeprecatedBorderStyle_HTMLTableElement.border = '';


	/* -- type: StyleSheet -- */

	StyleSheet.disabled = false;
	StyleSheet.ownerNode = Node;
	StyleSheet.media = MediaList;
	StyleSheet.href = '';
	StyleSheet.parentStyleSheet = StyleSheet;
	StyleSheet.title = '';
	StyleSheet.type = '';


	/* -- type: MSMimeTypesCollection -- */

	MSMimeTypesCollection.length = 0;


	/* -- type: DOMParser -- */

	DOMParser.parseFromString = function(source, mimeType) { 
		/// <signature>
		/// <param name='source' type='String' />
		/// <param name='mimeType' type='String' />
		/// <returns type='Document'/>
		/// </signature>
		return Document; 
	};


	/* -- type: MSHTMLFormElementExtensions -- */

	MSHTMLFormElementExtensions.encoding = '';


	/* -- type: MSHTMLCollectionExtensions -- */



	/* -- type: HTMLFormElement -- */
	_implement(HTMLFormElement, MSHTMLFormElementExtensions);
	_implement(HTMLFormElement, MSHTMLCollectionExtensions);

	HTMLFormElement.acceptCharset = '';
	HTMLFormElement.target = '';
	HTMLFormElement.length = 0;
	HTMLFormElement.elements = HTMLCollection;
	HTMLFormElement.enctype = '';
	HTMLFormElement.name = '';
	HTMLFormElement.action = '';
	HTMLFormElement.method = '';
	HTMLFormElement.reset = function() { };
	HTMLFormElement.submit = function() { };
	HTMLFormElement.item = function(name, index) { 
		/// <signature>
		/// <param name='name' type='Object' optional='true' />
		/// <param name='index' type='Object' optional='true' />
		/// <returns type='Object'/>
		/// </signature>
		return new Object(); 
	};
	HTMLFormElement.namedItem = function(name) { 
		/// <signature>
		/// <param name='name' type='String' />
		/// <returns type='Object'/>
		/// </signature>
		return new Object(); 
	};


	/* -- type: SVGZoomAndPan -- */

	SVGZoomAndPan.zoomAndPan = 0;
	SVGZoomAndPan.SVG_ZOOMANDPAN_MAGNIFY = 2;
	SVGZoomAndPan.SVG_ZOOMANDPAN_DISABLE = 1;
	SVGZoomAndPan.SVG_ZOOMANDPAN_UNKNOWN = 0;


	/* -- type: DOML2DeprecatedAlignmentStyle_HTMLHeadingElement -- */

	DOML2DeprecatedAlignmentStyle_HTMLHeadingElement.align = '';


	/* -- type: MSHTMLHeadingElementExtensions -- */
	_implement(MSHTMLHeadingElementExtensions, DOML2DeprecatedTextFlowControl_HTMLBlockElement);



	/* -- type: HTMLHeadingElement -- */
	_implement(HTMLHeadingElement, DOML2DeprecatedAlignmentStyle_HTMLHeadingElement);
	_implement(HTMLHeadingElement, MSHTMLHeadingElementExtensions);



	/* -- type: NodeFilterCallback -- */



	/* -- type: HTMLHeadElement -- */

	HTMLHeadElement.profile = '';


	/* -- type: HTMLSpanElement -- */
	_implement(HTMLSpanElement, MSDataBindingExtensions);



	/* -- type: DOML2DeprecatedWordWrapSuppression_HTMLDivElement -- */

	DOML2DeprecatedWordWrapSuppression_HTMLDivElement.noWrap = false;


	/* -- type: MSHTMLDivElementExtensions -- */
	_implement(MSHTMLDivElementExtensions, DOML2DeprecatedWordWrapSuppression_HTMLDivElement);



	/* -- type: DOML2DeprecatedBorderStyle_HTMLInputElement -- */

	DOML2DeprecatedBorderStyle_HTMLInputElement.border = '';


	/* -- type: HTMLBRElement -- */
	_implement(HTMLBRElement, DOML2DeprecatedTextFlowControl_HTMLBRElement);



	/* -- type: CSSRule -- */

	CSSRule.cssText = '';
	CSSRule.parentStyleSheet = CSSStyleSheet;
	CSSRule.parentRule = CSSRule;
	CSSRule.type = 0;
	CSSRule.MSKEYFRAME_RULE = 9;
	CSSRule.FONT_FACE_RULE = 5;
	CSSRule.IMPORT_RULE = 3;
	CSSRule.MSKEYFRAMES_RULE = 8;
	CSSRule.STYLE_RULE = 1;
	CSSRule.MEDIA_RULE = 4;
	CSSRule.UNKNOWN_RULE = 0;
	CSSRule.PAGE_RULE = 6;
	CSSRule.NAMESPACE_RULE = 10;
	CSSRule.CHARSET_RULE = 2;


	/* -- type: CSSPageRule -- */
	_implement(CSSPageRule, StyleSheetPage);

	CSSPageRule.selectorText = '';
	CSSPageRule.style = CSSStyleDeclaration;


	/* -- type: WindowPerformance -- */

	WindowPerformance.performance = new Object();


	/* -- type: BookmarkCollection -- */

	BookmarkCollection.length = 0;
	BookmarkCollection.item = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='Object'/>
		/// </signature>
		return new Object(); 
	};
	/* Add a single array element */
	BookmarkCollection.push(new Object());


	/* -- type: SVGAnimatedPathData -- */

	SVGAnimatedPathData.pathSegList = SVGPathSegList;


	/* -- type: Position -- */

	Position.timestamp = 0;
	Position.coords = Coordinates;


	/* -- type: DOML2DeprecatedWidthStyle -- */

	DOML2DeprecatedWidthStyle.width = 0;


	/* -- type: SVGAnimatedPoints -- */

	SVGAnimatedPoints.animatedPoints = SVGPointList;
	SVGAnimatedPoints.points = SVGPointList;


	/* -- type: SVGPolylineElement -- */
	_implement(SVGPolylineElement, SVGStylable);
	_implement(SVGPolylineElement, SVGTransformable);
	_implement(SVGPolylineElement, SVGLangSpace);
	_implement(SVGPolylineElement, SVGAnimatedPoints);
	_implement(SVGPolylineElement, SVGTests);



	/* -- type: DocumentFragment -- */
	_implement(DocumentFragment, NodeSelector);
	_implement(DocumentFragment, MSEventAttachmentTarget);
	_implement(DocumentFragment, MSNodeExtensions);



	/* -- type: UIEvent -- */

	UIEvent.detail = 0;
	UIEvent.view = AbstractView;
	UIEvent.initUIEvent = function(typeArg, canBubbleArg, cancelableArg, viewArg, detailArg) { 
		/// <signature>
		/// <param name='typeArg' type='String' />
		/// <param name='canBubbleArg' type='Boolean' />
		/// <param name='cancelableArg' type='Boolean' />
		/// <param name='viewArg' type='AbstractView' />
		/// <param name='detailArg' type='Number' />
		/// </signature>
	};


	/* -- type: TextEvent -- */

	TextEvent.inputMethod = 0;
	TextEvent.locale = '';
	TextEvent.data = '';
	TextEvent.DOM_INPUT_METHOD_DROP = 0x03;
	TextEvent.DOM_INPUT_METHOD_KEYBOARD = 0x01;
	TextEvent.DOM_INPUT_METHOD_IME = 0x04;
	TextEvent.DOM_INPUT_METHOD_SCRIPT = 0x09;
	TextEvent.DOM_INPUT_METHOD_VOICE = 0x07;
	TextEvent.DOM_INPUT_METHOD_UNKNOWN = 0x00;
	TextEvent.DOM_INPUT_METHOD_PASTE = 0x02;
	TextEvent.DOM_INPUT_METHOD_HANDWRITING = 0x06;
	TextEvent.DOM_INPUT_METHOD_OPTION = 0x05;
	TextEvent.DOM_INPUT_METHOD_MULTIMODAL = 0x08;
	TextEvent.initTextEvent = function(typeArg, canBubbleArg, cancelableArg, viewArg, dataArg, inputMethod, locale) { 
		/// <signature>
		/// <param name='typeArg' type='String' />
		/// <param name='canBubbleArg' type='Boolean' />
		/// <param name='cancelableArg' type='Boolean' />
		/// <param name='viewArg' type='AbstractView' />
		/// <param name='dataArg' type='String' />
		/// <param name='inputMethod' type='Number' />
		/// <param name='locale' type='String' />
		/// </signature>
	};


	/* -- type: DOML2DeprecatedBackgroundStyle -- */

	DOML2DeprecatedBackgroundStyle.background = '';


	/* -- type: CSSFontFaceRule -- */

	CSSFontFaceRule.style = CSSStyleDeclaration;


	/* -- type: MSBehaviorUrnsCollection -- */

	MSBehaviorUrnsCollection.length = 0;
	MSBehaviorUrnsCollection.item = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	/* Add a single array element */
	MSBehaviorUrnsCollection.push('');


	/* -- type: MSWindowExtensions -- */

	MSWindowExtensions.status = '';
	MSWindowExtensions.screenLeft = 0;
	MSWindowExtensions.offscreenBuffering = new Object();
	MSWindowExtensions.maxConnectionsPerServer = 0;
	MSWindowExtensions.clipboardData = DataTransfer;
	MSWindowExtensions.defaultStatus = '';
	MSWindowExtensions.clientInformation = Navigator;
	MSWindowExtensions.closed = false;
	MSWindowExtensions.external = BrowserPublic;
	MSWindowExtensions.event = MSEventObj;
	MSWindowExtensions.screenTop = 0;
	MSWindowExtensions.showModelessDialog = function(url, argument, options) { 
		/// <signature>
		/// <param name='url' type='String' optional='true' />
		/// <param name='argument' type='Object' optional='true' />
		/// <param name='options' type='Object' optional='true' />
		/// <returns type='Window'/>
		/// </signature>
		return Window; 
	};
	MSWindowExtensions.resizeBy = function(x, y) { 
		/// <signature>
		/// <param name='x' type='Number' optional='true' />
		/// <param name='y' type='Number' optional='true' />
		/// </signature>
	};
	MSWindowExtensions.navigate = function(url) { 
		/// <signature>
		/// <param name='url' type='String' />
		/// </signature>
	};
	MSWindowExtensions.item = function(index) { 
		/// <signature>
		/// <param name='index' type='Object' />
		/// <returns type='Object'/>
		/// </signature>
		return new Object(); 
	};
	MSWindowExtensions.resizeTo = function(x, y) { 
		/// <signature>
		/// <param name='x' type='Number' optional='true' />
		/// <param name='y' type='Number' optional='true' />
		/// </signature>
	};
	MSWindowExtensions.toStaticHTML = function(html) { 
		/// <signature>
		/// <param name='html' type='String' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	MSWindowExtensions.createPopup = function(arguments) { 
		/// <signature>
		/// <param name='arguments' type='Object' optional='true' />
		/// <returns type='MSPopupWindow'/>
		/// </signature>
		return MSPopupWindow; 
	};
	MSWindowExtensions.msWriteProfilerMark = function(profilerMarkName) { 
		/// <signature>
		/// <param name='profilerMarkName' type='String' />
		/// </signature>
	};
	MSWindowExtensions.execScript = function(code, language) { 
		/// <signature>
		/// <param name='code' type='String' />
		/// <param name='language' type='String' optional='true' />
		/// <returns type='Object'/>
		/// </signature>
		return new Object(); 
	};
	MSWindowExtensions.moveTo = function(x, y) { 
		/// <signature>
		/// <param name='x' type='Number' optional='true' />
		/// <param name='y' type='Number' optional='true' />
		/// </signature>
	};
	MSWindowExtensions.showHelp = function(url, helpArg, features) { 
		/// <signature>
		/// <param name='url' type='String' />
		/// <param name='helpArg' type='Object' optional='true' />
		/// <param name='features' type='String' optional='true' />
		/// </signature>
	};
	MSWindowExtensions.moveBy = function(x, y) { 
		/// <signature>
		/// <param name='x' type='Number' optional='true' />
		/// <param name='y' type='Number' optional='true' />
		/// </signature>
	};
	_events(MSWindowExtensions, "onmouseleave", "onmouseenter", "onhelp", "onfocusout", "onfocusin");


	/* -- type: ProcessingInstruction -- */

	ProcessingInstruction.target = '';
	ProcessingInstruction.data = '';


	/* -- type: SVGLengthList -- */

	SVGLengthList.numberOfItems = 0;
	SVGLengthList.replaceItem = function(newItem, index) { 
		/// <signature>
		/// <param name='newItem' type='SVGLength' />
		/// <param name='index' type='Number' />
		/// <returns type='SVGLength'/>
		/// </signature>
		return SVGLength; 
	};
	SVGLengthList.getItem = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='SVGLength'/>
		/// </signature>
		return SVGLength; 
	};
	SVGLengthList.appendItem = function(newItem) { 
		/// <signature>
		/// <param name='newItem' type='SVGLength' />
		/// <returns type='SVGLength'/>
		/// </signature>
		return SVGLength; 
	};
	SVGLengthList.clear = function() { };
	SVGLengthList.removeItem = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='SVGLength'/>
		/// </signature>
		return SVGLength; 
	};
	SVGLengthList.initialize = function(newItem) { 
		/// <signature>
		/// <param name='newItem' type='SVGLength' />
		/// <returns type='SVGLength'/>
		/// </signature>
		return SVGLength; 
	};
	SVGLengthList.insertItemBefore = function(newItem, index) { 
		/// <signature>
		/// <param name='newItem' type='SVGLength' />
		/// <param name='index' type='Number' />
		/// <returns type='SVGLength'/>
		/// </signature>
		return SVGLength; 
	};


	/* -- type: SVGPathSegCurvetoCubicSmoothRel -- */

	SVGPathSegCurvetoCubicSmoothRel.y = 0;
	SVGPathSegCurvetoCubicSmoothRel.x2 = 0;
	SVGPathSegCurvetoCubicSmoothRel.x = 0;
	SVGPathSegCurvetoCubicSmoothRel.y2 = 0;


	/* -- type: SVGPathSegCurvetoQuadraticSmoothAbs -- */

	SVGPathSegCurvetoQuadraticSmoothAbs.y = 0;
	SVGPathSegCurvetoQuadraticSmoothAbs.x = 0;


	/* -- type: MediaList -- */

	MediaList.length = 0;
	MediaList.mediaText = '';
	MediaList.deleteMedium = function(oldMedium) { 
		/// <signature>
		/// <param name='oldMedium' type='String' />
		/// </signature>
	};
	MediaList.appendMedium = function(newMedium) { 
		/// <signature>
		/// <param name='newMedium' type='String' />
		/// </signature>
	};
	MediaList.item = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	MediaList.toString = function() { 
		/// <signature>
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	/* Add a single array element */
	MediaList.push('');


	/* -- type: SVG1_1Properties -- */

	SVG1_1Properties.strokeLinecap = '';
	SVG1_1Properties.fillRule = '';
	SVG1_1Properties.stopColor = '';
	SVG1_1Properties.glyphOrientationHorizontal = '';
	SVG1_1Properties.kerning = '';
	SVG1_1Properties.alignmentBaseline = '';
	SVG1_1Properties.dominantBaseline = '';
	SVG1_1Properties.marker = '';
	SVG1_1Properties.fill = '';
	SVG1_1Properties.strokeMiterlimit = '';
	SVG1_1Properties.glyphOrientationVertical = '';
	SVG1_1Properties.markerMid = '';
	SVG1_1Properties.textAnchor = '';
	SVG1_1Properties.fillOpacity = '';
	SVG1_1Properties.strokeDasharray = '';
	SVG1_1Properties.mask = '';
	SVG1_1Properties.strokeDashoffset = '';
	SVG1_1Properties.stroke = '';
	SVG1_1Properties.stopOpacity = '';
	SVG1_1Properties.strokeOpacity = '';
	SVG1_1Properties.markerStart = '';
	SVG1_1Properties.pointerEvents = '';
	SVG1_1Properties.baselineShift = '';
	SVG1_1Properties.markerEnd = '';
	SVG1_1Properties.clipRule = '';
	SVG1_1Properties.strokeLinejoin = '';
	SVG1_1Properties.clipPath = '';
	SVG1_1Properties.strokeWidth = '';


	/* -- type: NamedNodeMap -- */

	NamedNodeMap.length = 0;
	NamedNodeMap.removeNamedItemNS = function(namespaceURI, localName) { 
		/// <signature>
		/// <param name='namespaceURI' type='String' />
		/// <param name='localName' type='String' />
		/// <returns type='Node'/>
		/// </signature>
		return Node; 
	};
	NamedNodeMap.item = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='Node'/>
		/// </signature>
		return Node; 
	};
	NamedNodeMap.removeNamedItem = function(name) { 
		/// <signature>
		/// <param name='name' type='String' />
		/// <returns type='Node'/>
		/// </signature>
		return Node; 
	};
	NamedNodeMap.getNamedItem = function(name) { 
		/// <signature>
		/// <param name='name' type='String' />
		/// <returns type='Node'/>
		/// </signature>
		return Node; 
	};
	NamedNodeMap.setNamedItemNS = function(arg) { 
		/// <signature>
		/// <param name='arg' type='Node' />
		/// <returns type='Node'/>
		/// </signature>
		return Node; 
	};
	NamedNodeMap.getNamedItemNS = function(namespaceURI, localName) { 
		/// <signature>
		/// <param name='namespaceURI' type='String' />
		/// <param name='localName' type='String' />
		/// <returns type='Node'/>
		/// </signature>
		return Node; 
	};
	NamedNodeMap.setNamedItem = function(arg) { 
		/// <signature>
		/// <param name='arg' type='Node' />
		/// <returns type='Node'/>
		/// </signature>
		return Node; 
	};


	/* -- type: DOML2DeprecatedBorderStyle -- */

	DOML2DeprecatedBorderStyle.border = '';


	/* -- type: DOML2DeprecatedAlignmentStyle_HTMLDivElement -- */

	DOML2DeprecatedAlignmentStyle_HTMLDivElement.align = '';


	/* -- type: HTMLDivElement -- */
	_implement(HTMLDivElement, DOML2DeprecatedAlignmentStyle_HTMLDivElement);
	_implement(HTMLDivElement, MSHTMLDivElementExtensions);
	_implement(HTMLDivElement, MSDataBindingExtensions);



	/* -- type: NavigatorDoNotTrack -- */

	NavigatorDoNotTrack.msDoNotTrack = '';


	/* -- type: SVGRectElement -- */
	_implement(SVGRectElement, SVGStylable);
	_implement(SVGRectElement, SVGTransformable);
	_implement(SVGRectElement, SVGLangSpace);
	_implement(SVGRectElement, SVGTests);

	SVGRectElement.ry = SVGAnimatedLength;
	SVGRectElement.width = SVGAnimatedLength;
	SVGRectElement.y = SVGAnimatedLength;
	SVGRectElement.rx = SVGAnimatedLength;
	SVGRectElement.x = SVGAnimatedLength;
	SVGRectElement.height = SVGAnimatedLength;


	/* -- type: DOML2DeprecatedListNumberingAndBulletStyle -- */

	DOML2DeprecatedListNumberingAndBulletStyle.type = '';


	/* -- type: DOML2DeprecatedListSpaceReduction -- */

	DOML2DeprecatedListSpaceReduction.compact = false;


	/* -- type: HTMLUListElement -- */
	_implement(HTMLUListElement, DOML2DeprecatedListNumberingAndBulletStyle);
	_implement(HTMLUListElement, DOML2DeprecatedListSpaceReduction);



	/* -- type: MSBorderColorStyle_HTMLTableCellElement -- */

	MSBorderColorStyle_HTMLTableCellElement.borderColor = new Object();


	/* -- type: HTMLTableAlignment -- */

	HTMLTableAlignment.ch = '';
	HTMLTableAlignment.vAlign = '';
	HTMLTableAlignment.chOff = '';


	/* -- type: SVGAnimatedEnumeration -- */

	SVGAnimatedEnumeration.animVal = 0;
	SVGAnimatedEnumeration.baseVal = 0;


	/* -- type: SVGLinearGradientElement -- */

	SVGLinearGradientElement.y1 = SVGAnimatedLength;
	SVGLinearGradientElement.x2 = SVGAnimatedLength;
	SVGLinearGradientElement.y2 = SVGAnimatedLength;
	SVGLinearGradientElement.x1 = SVGAnimatedLength;


	/* -- type: MSHTMLDocumentEventExtensions -- */

	MSHTMLDocumentEventExtensions.fireEvent = function(eventName, eventObj) { 
		/// <signature>
		/// <param name='eventName' type='String' />
		/// <param name='eventObj' type='Object' optional='true' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	MSHTMLDocumentEventExtensions.createEventObject = function(eventObj) { 
		/// <signature>
		/// <param name='eventObj' type='Object' optional='true' />
		/// <returns type='MSEventObj'/>
		/// </signature>
		return MSEventObj; 
	};


	/* -- type: MSHTMLDocumentViewExtensions -- */

	MSHTMLDocumentViewExtensions.createStyleSheet = function(href, index) { 
		/// <signature>
		/// <param name='href' type='String' optional='true' />
		/// <param name='index' type='Number' optional='true' />
		/// <returns type='CSSStyleSheet'/>
		/// </signature>
		return CSSStyleSheet; 
	};


	/* -- type: MSResourceMetadata -- */



	/* -- type: MSHTMLDocumentExtensions -- */

	MSHTMLDocumentExtensions.compatible = MSCompatibleInfoCollection;
	MSHTMLDocumentExtensions.media = '';
	MSHTMLDocumentExtensions.uniqueID = '';
	MSHTMLDocumentExtensions.documentMode = 0;
	MSHTMLDocumentExtensions.security = '';
	MSHTMLDocumentExtensions.namespaces = MSNamespaceInfoCollection;
	MSHTMLDocumentExtensions.frames = Window;
	MSHTMLDocumentExtensions.parentWindow = Window;
	MSHTMLDocumentExtensions.URLUnencoded = '';
	MSHTMLDocumentExtensions.updateSettings = function() { };
	MSHTMLDocumentExtensions.execCommandShowHelp = function(commandId) { 
		/// <signature>
		/// <param name='commandId' type='String' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	MSHTMLDocumentExtensions.releaseCapture = function() { };
	MSHTMLDocumentExtensions.focus = function() { };
	_events(MSHTMLDocumentExtensions, "onrowexit", "onrowsinserted", "oncontrolselect", "onpropertychange", "onafterupdate", "onhelp", "onbeforeactivate", "onstoragecommit", "onselectionchange", "onfocusout", "ondataavailable", "onbeforeupdate", "onfocusin", "ondatasetcomplete", "onbeforedeactivate", "onstop", "onactivate", "onmssitemodejumplistitemremoved", "onselectstart", "onerrorupdate", "ondeactivate", "ondatasetchanged", "onrowsdelete", "onmsthumbnailclick", "onrowenter", "onbeforeeditfocus", "oncellchange");


	/* -- type: HTMLDocument -- */
	_implement(HTMLDocument, MSEventAttachmentTarget);
	_implement(HTMLDocument, MSHTMLDocumentSelection);
	_implement(HTMLDocument, MSHTMLDocumentViewExtensions);
	_implement(HTMLDocument, MSHTMLDocumentEventExtensions);
	_implement(HTMLDocument, MSResourceMetadata);
	_implement(HTMLDocument, MSNodeExtensions);
	_implement(HTMLDocument, MSHTMLDocumentExtensions);

	HTMLDocument.bgColor = '';
	HTMLDocument.scripts = HTMLCollection;
	HTMLDocument.linkColor = '';
	HTMLDocument.charset = '';
	HTMLDocument.vlinkColor = '';
	HTMLDocument.title = '';
	HTMLDocument.defaultCharset = '';
	HTMLDocument.embeds = HTMLCollection;
	HTMLDocument.all = HTMLCollection;
	HTMLDocument.applets = HTMLCollection;
	HTMLDocument.forms = HTMLCollection;
	HTMLDocument.dir = '';
	HTMLDocument.body = HTMLElement;
	HTMLDocument.domain = '';
	HTMLDocument.designMode = '';
	HTMLDocument.activeElement = HTMLElement;
	HTMLDocument.links = HTMLCollection;
	HTMLDocument.URL = '';
	HTMLDocument.images = HTMLCollection;
	HTMLDocument.head = HTMLHeadElement;
	HTMLDocument.location = Location;
	HTMLDocument.cookie = '';
	HTMLDocument.characterSet = '';
	HTMLDocument.anchors = HTMLCollection;
	HTMLDocument.lastModified = '';
	HTMLDocument.plugins = HTMLCollection;
	HTMLDocument.readyState = '';
	HTMLDocument.referrer = '';
	HTMLDocument.alinkColor = '';
	HTMLDocument.fgColor = '';
	HTMLDocument.compatMode = '';
	HTMLDocument.queryCommandValue = function(commandId) { 
		/// <signature>
		/// <param name='commandId' type='String' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	HTMLDocument.queryCommandIndeterm = function(commandId) { 
		/// <signature>
		/// <param name='commandId' type='String' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	HTMLDocument.execCommand = function(commandId, showUI, value) { 
		/// <signature>
		/// <param name='commandId' type='String' />
		/// <param name='showUI' type='Boolean' optional='true' />
		/// <param name='value' type='Object' optional='true' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	HTMLDocument.writeln = function(content) { 
		/// <signature>
		/// <param name='content' type='String' />
		/// </signature>
	};
	HTMLDocument.getElementsByName = function(elementName) { 
		/// <signature>
		/// <param name='elementName' type='String' />
		/// <returns type='NodeList'/>
		/// </signature>
		return NodeList; 
	};
	HTMLDocument.open = function(url, name, features, replace) { 
		/// <signature>
		/// <param name='url' type='String' optional='true' />
		/// <param name='name' type='String' optional='true' />
		/// <param name='features' type='String' optional='true' />
		/// <param name='replace' type='Boolean' optional='true' />
		/// <returns type='Object'/>
		/// </signature>
		return new Object(); 
	};
	HTMLDocument.queryCommandState = function(commandId) { 
		/// <signature>
		/// <param name='commandId' type='String' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	HTMLDocument.close = function() { };
	HTMLDocument.getElementsByClassName = function(classNames) { 
		/// <signature>
		/// <param name='classNames' type='String' />
		/// <returns type='NodeList'/>
		/// </signature>
		return NodeList; 
	};
	HTMLDocument.hasFocus = function() { 
		/// <signature>
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	HTMLDocument.queryCommandSupported = function(commandId) { 
		/// <signature>
		/// <param name='commandId' type='String' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	HTMLDocument.getSelection = function() { 
		/// <signature>
		/// <returns type='Selection'/>
		/// </signature>
		return Selection; 
	};
	HTMLDocument.queryCommandEnabled = function(commandId) { 
		/// <signature>
		/// <param name='commandId' type='String' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	HTMLDocument.queryCommandText = function(commandId) { 
		/// <signature>
		/// <param name='commandId' type='String' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	HTMLDocument.write = function(content) { 
		/// <signature>
		/// <param name='content' type='String' />
		/// </signature>
	};
	_events(HTMLDocument, "ondragend", "ondragover", "onkeydown", "onkeyup", "onreset", "onmouseup", "ondragstart", "ondrag", "ondragleave", "onmouseover", "onpause", "onseeked", "onmousedown", "onclick", "onwaiting", "ondurationchange", "onblur", "onemptied", "onseeking", "oncanplay", "onstalled", "onmousemove", "onratechange", "onloadstart", "ondragenter", "onsubmit", "onprogress", "ondblclick", "oncontextmenu", "onchange", "onloadedmetadata", "onerror", "onplay", "onplaying", "oncanplaythrough", "onabort", "onreadystatechange", "onkeypress", "onloadeddata", "onsuspend", "onfocus", "ontimeupdate", "onselect", "ondrop", "onmouseout", "onended", "onscroll", "onmousewheel", "onvolumechange", "onload", "oninput");


	/* -- type: SVGException -- */

	SVGException.message = '';
	SVGException.code = 0;
	SVGException.SVG_MATRIX_NOT_INVERTABLE = 2;
	SVGException.SVG_WRONG_TYPE_ERR = 0;
	SVGException.SVG_INVALID_VALUE_ERR = 1;
	SVGException.toString = function() { 
		/// <signature>
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};


	/* -- type: DOML2DeprecatedTableCellHeight -- */

	DOML2DeprecatedTableCellHeight.height = new Object();


	/* -- type: DOML2DeprecatedAlignmentStyle_HTMLTableColElement -- */

	DOML2DeprecatedAlignmentStyle_HTMLTableColElement.align = '';


	/* -- type: HTMLTableColElement -- */
	_implement(HTMLTableColElement, HTMLTableAlignment);
	_implement(HTMLTableColElement, DOML2DeprecatedAlignmentStyle_HTMLTableColElement);

	HTMLTableColElement.width = new Object();
	HTMLTableColElement.span = 0;


	/* -- type: ImageData -- */

	ImageData.width = 0;
	ImageData.data = CanvasPixelArray;
	ImageData.height = 0;


	/* -- type: SVGUseElement -- */
	_implement(SVGUseElement, SVGStylable);
	_implement(SVGUseElement, SVGTransformable);
	_implement(SVGUseElement, SVGLangSpace);
	_implement(SVGUseElement, SVGTests);
	_implement(SVGUseElement, SVGURIReference);

	SVGUseElement.width = SVGAnimatedLength;
	SVGUseElement.y = SVGAnimatedLength;
	SVGUseElement.animatedInstanceRoot = SVGElementInstance;
	SVGUseElement.instanceRoot = SVGElementInstance;
	SVGUseElement.x = SVGAnimatedLength;
	SVGUseElement.height = SVGAnimatedLength;


	/* -- type: BeforeUnloadEvent -- */

	BeforeUnloadEvent.returnValue = '';


	/* -- type: MSPopupWindow -- */

	MSPopupWindow.document = HTMLDocument;
	MSPopupWindow.isOpen = false;
	MSPopupWindow.show = function(x, y, w, h, element) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// <param name='w' type='Number' />
		/// <param name='h' type='Number' />
		/// <param name='element' type='Object' optional='true' />
		/// </signature>
	};
	MSPopupWindow.hide = function() { };


	/* -- type: SVGMatrix -- */

	SVGMatrix.e = 0;
	SVGMatrix.c = 0;
	SVGMatrix.a = 0;
	SVGMatrix.b = 0;
	SVGMatrix.d = 0;
	SVGMatrix.f = 0;
	SVGMatrix.skewY = function(angle) { 
		/// <signature>
		/// <param name='angle' type='Number' />
		/// <returns type='SVGMatrix'/>
		/// </signature>
		return SVGMatrix; 
	};
	SVGMatrix.flipY = function() { 
		/// <signature>
		/// <returns type='SVGMatrix'/>
		/// </signature>
		return SVGMatrix; 
	};
	SVGMatrix.multiply = function(secondMatrix) { 
		/// <signature>
		/// <param name='secondMatrix' type='SVGMatrix' />
		/// <returns type='SVGMatrix'/>
		/// </signature>
		return SVGMatrix; 
	};
	SVGMatrix.inverse = function() { 
		/// <signature>
		/// <returns type='SVGMatrix'/>
		/// </signature>
		return SVGMatrix; 
	};
	SVGMatrix.scaleNonUniform = function(scaleFactorX, scaleFactorY) { 
		/// <signature>
		/// <param name='scaleFactorX' type='Number' />
		/// <param name='scaleFactorY' type='Number' />
		/// <returns type='SVGMatrix'/>
		/// </signature>
		return SVGMatrix; 
	};
	SVGMatrix.rotate = function(angle) { 
		/// <signature>
		/// <param name='angle' type='Number' />
		/// <returns type='SVGMatrix'/>
		/// </signature>
		return SVGMatrix; 
	};
	SVGMatrix.flipX = function() { 
		/// <signature>
		/// <returns type='SVGMatrix'/>
		/// </signature>
		return SVGMatrix; 
	};
	SVGMatrix.translate = function(x, y) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// <returns type='SVGMatrix'/>
		/// </signature>
		return SVGMatrix; 
	};
	SVGMatrix.scale = function(scaleFactor) { 
		/// <signature>
		/// <param name='scaleFactor' type='Number' />
		/// <returns type='SVGMatrix'/>
		/// </signature>
		return SVGMatrix; 
	};
	SVGMatrix.rotateFromVector = function(x, y) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// <returns type='SVGMatrix'/>
		/// </signature>
		return SVGMatrix; 
	};
	SVGMatrix.skewX = function(angle) { 
		/// <signature>
		/// <param name='angle' type='Number' />
		/// <returns type='SVGMatrix'/>
		/// </signature>
		return SVGMatrix; 
	};


	/* -- type: HTMLModElement -- */

	HTMLModElement.dateTime = '';
	HTMLModElement.cite = '';


	/* -- type: DOML2DeprecatedWordWrapSuppression -- */

	DOML2DeprecatedWordWrapSuppression.noWrap = false;


	/* -- type: MSMouseEventExtensions -- */

	MSMouseEventExtensions.toElement = HTMLElement;
	MSMouseEventExtensions.which = 0;
	MSMouseEventExtensions.fromElement = HTMLElement;
	MSMouseEventExtensions.layerY = 0;
	MSMouseEventExtensions.layerX = 0;


	/* -- type: SVGPathSegLinetoAbs -- */

	SVGPathSegLinetoAbs.y = 0;
	SVGPathSegLinetoAbs.x = 0;


	/* -- type: TimeRanges -- */

	TimeRanges.length = 0;
	TimeRanges.end = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='Number'/>
		/// </signature>
		return 0; 
	};
	TimeRanges.start = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='Number'/>
		/// </signature>
		return 0; 
	};


	/* -- type: SVGPathSegCurvetoQuadraticAbs -- */

	SVGPathSegCurvetoQuadraticAbs.y1 = 0;
	SVGPathSegCurvetoQuadraticAbs.y = 0;
	SVGPathSegCurvetoQuadraticAbs.x = 0;
	SVGPathSegCurvetoQuadraticAbs.x1 = 0;


	/* -- type: SVGPathSegCurvetoCubicAbs -- */

	SVGPathSegCurvetoCubicAbs.y1 = 0;
	SVGPathSegCurvetoCubicAbs.y = 0;
	SVGPathSegCurvetoCubicAbs.x2 = 0;
	SVGPathSegCurvetoCubicAbs.x = 0;
	SVGPathSegCurvetoCubicAbs.y2 = 0;
	SVGPathSegCurvetoCubicAbs.x1 = 0;


	/* -- type: DocumentStyle -- */

	DocumentStyle.styleSheets = StyleSheetList;


	/* -- type: History -- */

	History.length = 0;
	History.forward = function(distance) { 
		/// <signature>
		/// <param name='distance' type='Object' optional='true' />
		/// </signature>
	};
	History.back = function(distance) { 
		/// <signature>
		/// <param name='distance' type='Object' optional='true' />
		/// </signature>
	};
	History.go = function(delta) { 
		/// <signature>
		/// <param name='delta' type='Object' optional='true' />
		/// </signature>
	};


	/* -- type: KeyboardEventExtensions -- */

	KeyboardEventExtensions.keyCode = 0;
	KeyboardEventExtensions.which = 0;
	KeyboardEventExtensions.charCode = 0;


	/* -- type: DOML2DeprecatedMarginStyle_HTMLMarqueeElement -- */

	DOML2DeprecatedMarginStyle_HTMLMarqueeElement.vspace = 0;
	DOML2DeprecatedMarginStyle_HTMLMarqueeElement.hspace = 0;


	/* -- type: HTMLMarqueeElement -- */
	_implement(HTMLMarqueeElement, DOML2DeprecatedMarginStyle_HTMLMarqueeElement);
	_implement(HTMLMarqueeElement, MSDataBindingExtensions);
	_implement(HTMLMarqueeElement, DOML2DeprecatedBackgroundColorStyle);

	HTMLMarqueeElement.width = '';
	HTMLMarqueeElement.trueSpeed = false;
	HTMLMarqueeElement.scrollDelay = 0;
	HTMLMarqueeElement.scrollAmount = 0;
	HTMLMarqueeElement.height = '';
	HTMLMarqueeElement.behavior = '';
	HTMLMarqueeElement.loop = 0;
	HTMLMarqueeElement.direction = '';
	HTMLMarqueeElement.stop = function() { };
	HTMLMarqueeElement.start = function() { };
	_events(HTMLMarqueeElement, "onbounce", "onstart", "onfinish");


	/* -- type: SVGRect -- */

	SVGRect.width = 0;
	SVGRect.y = 0;
	SVGRect.x = 0;
	SVGRect.height = 0;


	/* -- type: MSWindowModeless -- */

	MSWindowModeless.dialogTop = new Object();
	MSWindowModeless.dialogLeft = new Object();
	MSWindowModeless.dialogWidth = new Object();
	MSWindowModeless.dialogHeight = new Object();
	MSWindowModeless.menuArguments = new Object();


	/* -- type: DOML2DeprecatedMarginStyle -- */

	DOML2DeprecatedMarginStyle.vspace = 0;
	DOML2DeprecatedMarginStyle.hspace = 0;


	/* -- type: Geolocation -- */

	Geolocation.clearWatch = function(watchId) { 
		/// <signature>
		/// <param name='watchId' type='Number' />
		/// </signature>
	};
	Geolocation.getCurrentPosition = function(successCallback, errorCallback, options) { 
		/// <signature>
		/// <param name='successCallback' type='PositionCallback' />
		/// <param name='errorCallback' type='PositionErrorCallback' optional='true' />
		/// <param name='options' type='PositionOptions' optional='true' />
		/// </signature>
	};
	Geolocation.watchPosition = function(successCallback, errorCallback, options) { 
		/// <signature>
		/// <param name='successCallback' type='PositionCallback' />
		/// <param name='errorCallback' type='PositionErrorCallback' optional='true' />
		/// <param name='options' type='PositionOptions' optional='true' />
		/// <returns type='Number'/>
		/// </signature>
		return 0; 
	};


	/* -- type: MSHTMLTextAreaElementExtensions -- */

	MSHTMLTextAreaElementExtensions.status = new Object();
	MSHTMLTextAreaElementExtensions.createTextRange = function() { 
		/// <signature>
		/// <returns type='TextRange'/>
		/// </signature>
		return TextRange; 
	};


	/* -- type: HTMLTextAreaElement -- */
	_implement(HTMLTextAreaElement, MSDataBindingExtensions);
	_implement(HTMLTextAreaElement, MSHTMLTextAreaElementExtensions);

	HTMLTextAreaElement.value = '';
	HTMLTextAreaElement.name = '';
	HTMLTextAreaElement.form = HTMLFormElement;
	HTMLTextAreaElement.selectionStart = 0;
	HTMLTextAreaElement.rows = 0;
	HTMLTextAreaElement.readOnly = false;
	HTMLTextAreaElement.cols = 0;
	HTMLTextAreaElement.selectionEnd = 0;
	HTMLTextAreaElement.wrap = '';
	HTMLTextAreaElement.type = '';
	HTMLTextAreaElement.defaultValue = '';
	HTMLTextAreaElement.setSelectionRange = function(start, end) { 
		/// <signature>
		/// <param name='start' type='Number' />
		/// <param name='end' type='Number' />
		/// </signature>
	};
	HTMLTextAreaElement.select = function() { };


	/* -- type: DOML2DeprecatedSizeProperty_HTMLBaseFontElement -- */

	DOML2DeprecatedSizeProperty_HTMLBaseFontElement.size = 0;


	/* -- type: HTMLBaseFontElement -- */
	_implement(HTMLBaseFontElement, DOML2DeprecatedSizeProperty_HTMLBaseFontElement);
	_implement(HTMLBaseFontElement, DOML2DeprecatedColorProperty);

	HTMLBaseFontElement.face = '';


	/* -- type: CustomEvent -- */

	CustomEvent.detail = new Object();
	CustomEvent.initCustomEvent = function(typeArg, canBubbleArg, cancelableArg, detailArg) { 
		/// <signature>
		/// <param name='typeArg' type='String' />
		/// <param name='canBubbleArg' type='Boolean' />
		/// <param name='cancelableArg' type='Boolean' />
		/// <param name='detailArg' type='Object' />
		/// </signature>
	};


	/* -- type: CSSImportRule -- */

	CSSImportRule.styleSheet = CSSStyleSheet;
	CSSImportRule.media = MediaList;
	CSSImportRule.href = '';


	/* -- type: StyleSheetList -- */

	StyleSheetList.length = 0;
	StyleSheetList.item = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' optional='true' />
		/// <returns type='StyleSheet'/>
		/// </signature>
		return StyleSheet; 
	};
	/* Add a single array element */
	StyleSheetList.push(StyleSheet);


	/* -- type: SVGCircleElement -- */
	_implement(SVGCircleElement, SVGStylable);
	_implement(SVGCircleElement, SVGTransformable);
	_implement(SVGCircleElement, SVGLangSpace);
	_implement(SVGCircleElement, SVGTests);

	SVGCircleElement.cx = SVGAnimatedLength;
	SVGCircleElement.r = SVGAnimatedLength;
	SVGCircleElement.cy = SVGAnimatedLength;


	/* -- type: MSNamespaceInfoCollection -- */

	MSNamespaceInfoCollection.length = 0;
	MSNamespaceInfoCollection.add = function(namespace, urn, implementationUrl) { 
		/// <signature>
		/// <param name='namespace' type='String' optional='true' />
		/// <param name='urn' type='String' optional='true' />
		/// <param name='implementationUrl' type='Object' optional='true' />
		/// <returns type='Object'/>
		/// </signature>
		return new Object(); 
	};
	MSNamespaceInfoCollection.item = function(index) { 
		/// <signature>
		/// <param name='index' type='Object' />
		/// <returns type='Object'/>
		/// </signature>
		return new Object(); 
	};
	/* Add a single array element */
	MSNamespaceInfoCollection.push(new Object());


	/* -- type: SVGElementInstance -- */
	_implement(SVGElementInstance, EventTarget);

	SVGElementInstance.previousSibling = SVGElementInstance;
	SVGElementInstance.parentNode = SVGElementInstance;
	Object.defineProperty(SVGElementInstance,"lastChild", { get: function () { return _lastChild(); } });
	SVGElementInstance.nextSibling = SVGElementInstance;
	SVGElementInstance.childNodes = SVGElementInstanceList;
	SVGElementInstance.correspondingUseElement = SVGUseElement;
	SVGElementInstance.correspondingElement = SVGElement;
	Object.defineProperty(SVGElementInstance,"firstChild", { get: function () { return _firstChild(); } });


	/* -- type: BrowserPublic -- */



	/* -- type: MSBorderColorHighlightStyle_HTMLTableCellElement -- */

	MSBorderColorHighlightStyle_HTMLTableCellElement.borderColorLight = new Object();
	MSBorderColorHighlightStyle_HTMLTableCellElement.borderColorDark = new Object();


	/* -- type: DOML2DeprecatedWidthStyle_HTMLTableCellElement -- */

	DOML2DeprecatedWidthStyle_HTMLTableCellElement.width = 0;


	/* -- type: HTMLTableHeaderCellScope -- */

	HTMLTableHeaderCellScope.scope = '';


	/* -- type: HTMLTableCellElement -- */
	_implement(HTMLTableCellElement, DOML2DeprecatedTableCellHeight);
	_implement(HTMLTableCellElement, MSBorderColorHighlightStyle_HTMLTableCellElement);
	_implement(HTMLTableCellElement, HTMLTableAlignment);
	_implement(HTMLTableCellElement, MSBorderColorStyle_HTMLTableCellElement);
	_implement(HTMLTableCellElement, DOML2DeprecatedBackgroundStyle);
	_implement(HTMLTableCellElement, DOML2DeprecatedWidthStyle_HTMLTableCellElement);
	_implement(HTMLTableCellElement, DOML2DeprecatedAlignmentStyle_HTMLTableCellElement);
	_implement(HTMLTableCellElement, HTMLTableHeaderCellScope);
	_implement(HTMLTableCellElement, DOML2DeprecatedWordWrapSuppression);
	_implement(HTMLTableCellElement, DOML2DeprecatedBackgroundColorStyle);

	HTMLTableCellElement.abbr = '';
	HTMLTableCellElement.headers = '';
	HTMLTableCellElement.cellIndex = 0;
	HTMLTableCellElement.rowSpan = 0;
	HTMLTableCellElement.colSpan = 0;
	HTMLTableCellElement.axis = '';


	/* -- type: MSBorderColorHighlightStyle_HTMLTableRowElement -- */

	MSBorderColorHighlightStyle_HTMLTableRowElement.borderColorLight = new Object();
	MSBorderColorHighlightStyle_HTMLTableRowElement.borderColorDark = new Object();


	/* -- type: PositionError -- */

	PositionError.message = '';
	PositionError.code = 0;
	PositionError.POSITION_UNAVAILABLE = 2;
	PositionError.TIMEOUT = 3;
	PositionError.PERMISSION_DENIED = 1;
	PositionError.toString = function() { 
		/// <signature>
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};


	/* -- type: MSImageResourceExtensions -- */

	MSImageResourceExtensions.dynsrc = '';
	MSImageResourceExtensions.vrml = '';
	MSImageResourceExtensions.loop = 0;
	MSImageResourceExtensions.start = '';
	MSImageResourceExtensions.lowsrc = '';


	/* -- type: HTMLAudioElement -- */



	/* -- type: MSDataBindingRecordSetReadonlyExtensions -- */

	MSDataBindingRecordSetReadonlyExtensions.recordset = new Object();
	MSDataBindingRecordSetReadonlyExtensions.namedRecordset = function(dataMember, hierarchy) { 
		/// <signature>
		/// <param name='dataMember' type='String' />
		/// <param name='hierarchy' type='Object' optional='true' />
		/// <returns type='Object'/>
		/// </signature>
		return new Object(); 
	};


	/* -- type: HTMLUnknownElement -- */
	_implement(HTMLUnknownElement, MSDataBindingRecordSetReadonlyExtensions);



	/* -- type: SVGPathSegList -- */

	SVGPathSegList.numberOfItems = 0;
	SVGPathSegList.replaceItem = function(newItem, index) { 
		/// <signature>
		/// <param name='newItem' type='SVGPathSeg' />
		/// <param name='index' type='Number' />
		/// <returns type='SVGPathSeg'/>
		/// </signature>
		return SVGPathSeg; 
	};
	SVGPathSegList.getItem = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='SVGPathSeg'/>
		/// </signature>
		return SVGPathSeg; 
	};
	SVGPathSegList.appendItem = function(newItem) { 
		/// <signature>
		/// <param name='newItem' type='SVGPathSeg' />
		/// <returns type='SVGPathSeg'/>
		/// </signature>
		return SVGPathSeg; 
	};
	SVGPathSegList.clear = function() { };
	SVGPathSegList.removeItem = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='SVGPathSeg'/>
		/// </signature>
		return SVGPathSeg; 
	};
	SVGPathSegList.initialize = function(newItem) { 
		/// <signature>
		/// <param name='newItem' type='SVGPathSeg' />
		/// <returns type='SVGPathSeg'/>
		/// </signature>
		return SVGPathSeg; 
	};
	SVGPathSegList.insertItemBefore = function(newItem, index) { 
		/// <signature>
		/// <param name='newItem' type='SVGPathSeg' />
		/// <param name='index' type='Number' />
		/// <returns type='SVGPathSeg'/>
		/// </signature>
		return SVGPathSeg; 
	};


	/* -- type: CSSNamespaceRule -- */

	CSSNamespaceRule.namespaceURI = '';
	CSSNamespaceRule.prefix = '';


	/* -- type: Text -- */
	_implement(Text, MSNodeExtensions);

	Text.wholeText = '';
	Text.splitText = function(offset) { 
		/// <signature>
		/// <param name='offset' type='Number' />
		/// <returns type='Text'/>
		/// </signature>
		return Text; 
	};
	Text.replaceWholeText = function(content) { 
		/// <signature>
		/// <param name='content' type='String' />
		/// <returns type='Text'/>
		/// </signature>
		return Text; 
	};


	/* -- type: SVGAnimatedRect -- */

	SVGAnimatedRect.animVal = SVGRect;
	SVGAnimatedRect.baseVal = SVGRect;


	/* -- type: MSCompatibleInfo -- */

	MSCompatibleInfo.version = '';
	MSCompatibleInfo.userAgent = '';


	/* -- type: SVGPathElement -- */
	_implement(SVGPathElement, SVGAnimatedPathData);
	_implement(SVGPathElement, SVGStylable);
	_implement(SVGPathElement, SVGTransformable);
	_implement(SVGPathElement, SVGLangSpace);
	_implement(SVGPathElement, SVGTests);

	SVGPathElement.getPathSegAtLength = function(distance) { 
		/// <signature>
		/// <param name='distance' type='Number' />
		/// <returns type='Number'/>
		/// </signature>
		return 0; 
	};
	SVGPathElement.createSVGPathSegCurvetoQuadraticAbs = function(x, y, x1, y1) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// <param name='x1' type='Number' />
		/// <param name='y1' type='Number' />
		/// <returns type='SVGPathSegCurvetoQuadraticAbs'/>
		/// </signature>
		return SVGPathSegCurvetoQuadraticAbs; 
	};
	SVGPathElement.getPointAtLength = function(distance) { 
		/// <signature>
		/// <param name='distance' type='Number' />
		/// <returns type='SVGPoint'/>
		/// </signature>
		return SVGPoint; 
	};
	SVGPathElement.createSVGPathSegLinetoRel = function(x, y) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// <returns type='SVGPathSegLinetoRel'/>
		/// </signature>
		return SVGPathSegLinetoRel; 
	};
	SVGPathElement.createSVGPathSegCurvetoQuadraticRel = function(x, y, x1, y1) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// <param name='x1' type='Number' />
		/// <param name='y1' type='Number' />
		/// <returns type='SVGPathSegCurvetoQuadraticRel'/>
		/// </signature>
		return SVGPathSegCurvetoQuadraticRel; 
	};
	SVGPathElement.createSVGPathSegCurvetoCubicAbs = function(x, y, x1, y1, x2, y2) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// <param name='x1' type='Number' />
		/// <param name='y1' type='Number' />
		/// <param name='x2' type='Number' />
		/// <param name='y2' type='Number' />
		/// <returns type='SVGPathSegCurvetoCubicAbs'/>
		/// </signature>
		return SVGPathSegCurvetoCubicAbs; 
	};
	SVGPathElement.createSVGPathSegLinetoAbs = function(x, y) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// <returns type='SVGPathSegLinetoAbs'/>
		/// </signature>
		return SVGPathSegLinetoAbs; 
	};
	SVGPathElement.createSVGPathSegClosePath = function() { 
		/// <signature>
		/// <returns type='SVGPathSegClosePath'/>
		/// </signature>
		return SVGPathSegClosePath; 
	};
	SVGPathElement.createSVGPathSegCurvetoCubicRel = function(x, y, x1, y1, x2, y2) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// <param name='x1' type='Number' />
		/// <param name='y1' type='Number' />
		/// <param name='x2' type='Number' />
		/// <param name='y2' type='Number' />
		/// <returns type='SVGPathSegCurvetoCubicRel'/>
		/// </signature>
		return SVGPathSegCurvetoCubicRel; 
	};
	SVGPathElement.createSVGPathSegCurvetoQuadraticSmoothRel = function(x, y) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// <returns type='SVGPathSegCurvetoQuadraticSmoothRel'/>
		/// </signature>
		return SVGPathSegCurvetoQuadraticSmoothRel; 
	};
	SVGPathElement.createSVGPathSegMovetoRel = function(x, y) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// <returns type='SVGPathSegMovetoRel'/>
		/// </signature>
		return SVGPathSegMovetoRel; 
	};
	SVGPathElement.createSVGPathSegCurvetoCubicSmoothAbs = function(x, y, x2, y2) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// <param name='x2' type='Number' />
		/// <param name='y2' type='Number' />
		/// <returns type='SVGPathSegCurvetoCubicSmoothAbs'/>
		/// </signature>
		return SVGPathSegCurvetoCubicSmoothAbs; 
	};
	SVGPathElement.createSVGPathSegMovetoAbs = function(x, y) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// <returns type='SVGPathSegMovetoAbs'/>
		/// </signature>
		return SVGPathSegMovetoAbs; 
	};
	SVGPathElement.createSVGPathSegLinetoVerticalRel = function(y) { 
		/// <signature>
		/// <param name='y' type='Number' />
		/// <returns type='SVGPathSegLinetoVerticalRel'/>
		/// </signature>
		return SVGPathSegLinetoVerticalRel; 
	};
	SVGPathElement.createSVGPathSegArcRel = function(x, y, r1, r2, angle, largeArcFlag, sweepFlag) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// <param name='r1' type='Number' />
		/// <param name='r2' type='Number' />
		/// <param name='angle' type='Number' />
		/// <param name='largeArcFlag' type='Boolean' />
		/// <param name='sweepFlag' type='Boolean' />
		/// <returns type='SVGPathSegArcRel'/>
		/// </signature>
		return SVGPathSegArcRel; 
	};
	SVGPathElement.createSVGPathSegCurvetoQuadraticSmoothAbs = function(x, y) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// <returns type='SVGPathSegCurvetoQuadraticSmoothAbs'/>
		/// </signature>
		return SVGPathSegCurvetoQuadraticSmoothAbs; 
	};
	SVGPathElement.getTotalLength = function() { 
		/// <signature>
		/// <returns type='Number'/>
		/// </signature>
		return 0; 
	};
	SVGPathElement.createSVGPathSegLinetoHorizontalRel = function(x) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <returns type='SVGPathSegLinetoHorizontalRel'/>
		/// </signature>
		return SVGPathSegLinetoHorizontalRel; 
	};
	SVGPathElement.createSVGPathSegCurvetoCubicSmoothRel = function(x, y, x2, y2) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// <param name='x2' type='Number' />
		/// <param name='y2' type='Number' />
		/// <returns type='SVGPathSegCurvetoCubicSmoothRel'/>
		/// </signature>
		return SVGPathSegCurvetoCubicSmoothRel; 
	};
	SVGPathElement.createSVGPathSegLinetoVerticalAbs = function(y) { 
		/// <signature>
		/// <param name='y' type='Number' />
		/// <returns type='SVGPathSegLinetoVerticalAbs'/>
		/// </signature>
		return SVGPathSegLinetoVerticalAbs; 
	};
	SVGPathElement.createSVGPathSegLinetoHorizontalAbs = function(x) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <returns type='SVGPathSegLinetoHorizontalAbs'/>
		/// </signature>
		return SVGPathSegLinetoHorizontalAbs; 
	};
	SVGPathElement.createSVGPathSegArcAbs = function(x, y, r1, r2, angle, largeArcFlag, sweepFlag) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// <param name='r1' type='Number' />
		/// <param name='r2' type='Number' />
		/// <param name='angle' type='Number' />
		/// <param name='largeArcFlag' type='Boolean' />
		/// <param name='sweepFlag' type='Boolean' />
		/// <returns type='SVGPathSegArcAbs'/>
		/// </signature>
		return SVGPathSegArcAbs; 
	};


	/* -- type: SVGNumber -- */

	SVGNumber.value = 0;


	/* -- type: MouseEvent -- */
	_implement(MouseEvent, MSMouseEventExtensions);

	MouseEvent.pageX = 0;
	MouseEvent.offsetY = 0;
	MouseEvent.x = 0;
	MouseEvent.y = 0;
	MouseEvent.altKey = false;
	MouseEvent.metaKey = false;
	MouseEvent.ctrlKey = false;
	MouseEvent.offsetX = 0;
	MouseEvent.screenX = 0;
	MouseEvent.clientY = 0;
	MouseEvent.shiftKey = false;
	MouseEvent.screenY = 0;
	MouseEvent.relatedTarget = EventTarget;
	MouseEvent.button = 0;
	MouseEvent.pageY = 0;
	MouseEvent.clientX = 0;
	MouseEvent.buttons = 0;
	MouseEvent.getModifierState = function(keyArg) { 
		/// <signature>
		/// <param name='keyArg' type='String' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	MouseEvent.initMouseEvent = function(typeArg, canBubbleArg, cancelableArg, viewArg, detailArg, screenXArg, screenYArg, clientXArg, clientYArg, ctrlKeyArg, altKeyArg, shiftKeyArg, metaKeyArg, buttonArg, relatedTargetArg) { 
		/// <signature>
		/// <param name='typeArg' type='String' />
		/// <param name='canBubbleArg' type='Boolean' />
		/// <param name='cancelableArg' type='Boolean' />
		/// <param name='viewArg' type='AbstractView' />
		/// <param name='detailArg' type='Number' />
		/// <param name='screenXArg' type='Number' />
		/// <param name='screenYArg' type='Number' />
		/// <param name='clientXArg' type='Number' />
		/// <param name='clientYArg' type='Number' />
		/// <param name='ctrlKeyArg' type='Boolean' />
		/// <param name='altKeyArg' type='Boolean' />
		/// <param name='shiftKeyArg' type='Boolean' />
		/// <param name='metaKeyArg' type='Boolean' />
		/// <param name='buttonArg' type='Number' />
		/// <param name='relatedTargetArg' type='EventTarget' />
		/// </signature>
	};


	/* -- type: WheelEvent -- */

	WheelEvent.deltaZ = 0;
	WheelEvent.deltaX = 0;
	WheelEvent.deltaMode = 0;
	WheelEvent.deltaY = 0;
	WheelEvent.DOM_DELTA_PIXEL = 0x00;
	WheelEvent.DOM_DELTA_LINE = 0x01;
	WheelEvent.DOM_DELTA_PAGE = 0x02;
	WheelEvent.initWheelEvent = function(typeArg, canBubbleArg, cancelableArg, viewArg, detailArg, screenXArg, screenYArg, clientXArg, clientYArg, buttonArg, relatedTargetArg, modifiersListArg, deltaXArg, deltaYArg, deltaZArg, deltaMode) { 
		/// <signature>
		/// <param name='typeArg' type='String' />
		/// <param name='canBubbleArg' type='Boolean' />
		/// <param name='cancelableArg' type='Boolean' />
		/// <param name='viewArg' type='AbstractView' />
		/// <param name='detailArg' type='Number' />
		/// <param name='screenXArg' type='Number' />
		/// <param name='screenYArg' type='Number' />
		/// <param name='clientXArg' type='Number' />
		/// <param name='clientYArg' type='Number' />
		/// <param name='buttonArg' type='Number' />
		/// <param name='relatedTargetArg' type='EventTarget' />
		/// <param name='modifiersListArg' type='String' />
		/// <param name='deltaXArg' type='Number' />
		/// <param name='deltaYArg' type='Number' />
		/// <param name='deltaZArg' type='Number' />
		/// <param name='deltaMode' type='Number' />
		/// </signature>
	};


	/* -- type: ViewCSS_SVGSVGElement -- */

	ViewCSS_SVGSVGElement.getComputedStyle = function(elt, pseudoElt) { 
		/// <signature>
		/// <param name='elt' type='Element' />
		/// <param name='pseudoElt' type='String' optional='true' />
		/// <returns type='CSSStyleDeclaration'/>
		/// </signature>
		return CSSStyleDeclaration; 
	};


	/* -- type: MSCSSFilter -- */

	MSCSSFilter.Percent = 0;
	MSCSSFilter.Enabled = false;
	MSCSSFilter.Duration = 0;
	MSCSSFilter.Play = function(Duration) { 
		/// <signature>
		/// <param name='Duration' type='Number' />
		/// </signature>
	};
	MSCSSFilter.Apply = function() { };
	MSCSSFilter.Stop = function() { };


	/* -- type: SVGTransform -- */

	SVGTransform.angle = 0;
	SVGTransform.type = 0;
	SVGTransform.matrix = SVGMatrix;
	SVGTransform.SVG_TRANSFORM_SKEWX = 5;
	SVGTransform.SVG_TRANSFORM_SCALE = 3;
	SVGTransform.SVG_TRANSFORM_UNKNOWN = 0;
	SVGTransform.SVG_TRANSFORM_TRANSLATE = 2;
	SVGTransform.SVG_TRANSFORM_MATRIX = 1;
	SVGTransform.SVG_TRANSFORM_ROTATE = 4;
	SVGTransform.SVG_TRANSFORM_SKEWY = 6;
	SVGTransform.setScale = function(sx, sy) { 
		/// <signature>
		/// <param name='sx' type='Number' />
		/// <param name='sy' type='Number' />
		/// </signature>
	};
	SVGTransform.setTranslate = function(tx, ty) { 
		/// <signature>
		/// <param name='tx' type='Number' />
		/// <param name='ty' type='Number' />
		/// </signature>
	};
	SVGTransform.setMatrix = function(matrix) { 
		/// <signature>
		/// <param name='matrix' type='SVGMatrix' />
		/// </signature>
	};
	SVGTransform.setSkewY = function(angle) { 
		/// <signature>
		/// <param name='angle' type='Number' />
		/// </signature>
	};
	SVGTransform.setRotate = function(angle, cx, cy) { 
		/// <signature>
		/// <param name='angle' type='Number' />
		/// <param name='cx' type='Number' />
		/// <param name='cy' type='Number' />
		/// </signature>
	};
	SVGTransform.setSkewX = function(angle) { 
		/// <signature>
		/// <param name='angle' type='Number' />
		/// </signature>
	};


	/* -- type: MSBorderColorHighlightStyle -- */

	MSBorderColorHighlightStyle.borderColorLight = new Object();
	MSBorderColorHighlightStyle.borderColorDark = new Object();


	/* -- type: MSLinkStyleExtensions -- */

	MSLinkStyleExtensions.styleSheet = StyleSheet;


	/* -- type: HTMLStyleElement -- */
	_implement(HTMLStyleElement, MSLinkStyleExtensions);
	_implement(HTMLStyleElement, LinkStyle);

	HTMLStyleElement.media = '';
	HTMLStyleElement.type = '';


	/* -- type: HTMLTitleElement -- */

	HTMLTitleElement.text = '';


	/* -- type: Location -- */

	Location.protocol = '';
	Location.hash = '';
	Location.search = '';
	Location.href = '';
	Location.hostname = '';
	Location.pathname = '';
	Location.port = '';
	Location.host = '';
	Location.reload = function(flag) { 
		/// <signature>
		/// <param name='flag' type='Boolean' optional='true' />
		/// </signature>
	};
	Location.replace = function(url) { 
		/// <signature>
		/// <param name='url' type='String' />
		/// </signature>
	};
	Location.assign = function(url) { 
		/// <signature>
		/// <param name='url' type='String' />
		/// </signature>
	};
	Location.toString = function() { 
		/// <signature>
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};


	/* -- type: HTMLCanvasElement -- */

	HTMLCanvasElement.width = 0;
	HTMLCanvasElement.height = 0;
	HTMLCanvasElement.toDataURL = function(type, args) { 
		/// <signature>
		/// <param name='type' type='String' optional='true' />
		/// <param name='args' type='Object' optional='true' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	HTMLCanvasElement.getContext = function(contextId) { 
		/// <signature>
		/// <param name='contextId' type='String' />
		/// <returns type='CanvasRenderingContext2D'/>
		/// </signature>
		return CanvasRenderingContext2D; 
	};


	/* -- type: MSEventObj -- */

	MSEventObj.nextPage = '';
	MSEventObj.toElement = HTMLElement;
	MSEventObj.keyCode = 0;
	MSEventObj.returnValue = new Object();
	MSEventObj.dataFld = '';
	MSEventObj.y = 0;
	MSEventObj.dataTransfer = DataTransfer;
	MSEventObj.propertyName = '';
	MSEventObj.url = '';
	MSEventObj.recordset = new Object();
	MSEventObj.offsetX = 0;
	MSEventObj.screenX = 0;
	MSEventObj.buttonID = 0;
	MSEventObj.wheelDelta = 0;
	MSEventObj.reason = 0;
	MSEventObj.origin = '';
	MSEventObj.srcFilter = new Object();
	MSEventObj.data = '';
	MSEventObj.boundElements = HTMLCollection;
	MSEventObj.cancelBubble = false;
	MSEventObj.behaviorCookie = 0;
	MSEventObj.altLeft = false;
	MSEventObj.bookmarks = BookmarkCollection;
	MSEventObj.srcElement = HTMLElement;
	MSEventObj.repeat = false;
	MSEventObj.type = '';
	MSEventObj.source = Window;
	MSEventObj.fromElement = HTMLElement;
	MSEventObj.offsetY = 0;
	MSEventObj.x = 0;
	MSEventObj.behaviorPart = 0;
	MSEventObj.qualifier = '';
	MSEventObj.altKey = false;
	MSEventObj.ctrlKey = false;
	MSEventObj.clientY = 0;
	MSEventObj.shiftKey = false;
	MSEventObj.shiftLeft = false;
	MSEventObj.contentOverflow = false;
	MSEventObj.screenY = 0;
	MSEventObj.ctrlLeft = false;
	MSEventObj.button = 0;
	MSEventObj.srcUrn = '';
	MSEventObj.actionURL = '';
	MSEventObj.clientX = 0;
	MSEventObj.setAttribute = function(strAttributeName, AttributeValue, lFlags) { 
		/// <signature>
		/// <param name='strAttributeName' type='String' />
		/// <param name='AttributeValue' type='Object' />
		/// <param name='lFlags' type='Number' optional='true' />
		/// </signature>
		_setAttribute(this, strAttributeName, AttributeValue);
	};
	MSEventObj.getAttribute = function(strAttributeName, lFlags) { 
		/// <signature>
		/// <param name='strAttributeName' type='String' />
		/// <param name='lFlags' type='Number' optional='true' />
		/// <returns type='Object'/>
		/// </signature>
		return _getAttribute(this, strAttributeName);
	};
	MSEventObj.removeAttribute = function(strAttributeName, lFlags) { 
		/// <signature>
		/// <param name='strAttributeName' type='String' />
		/// <param name='lFlags' type='Number' optional='true' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};


	/* -- type: SVGPathSegCurvetoCubicRel -- */

	SVGPathSegCurvetoCubicRel.y1 = 0;
	SVGPathSegCurvetoCubicRel.y = 0;
	SVGPathSegCurvetoCubicRel.x2 = 0;
	SVGPathSegCurvetoCubicRel.x = 0;
	SVGPathSegCurvetoCubicRel.y2 = 0;
	SVGPathSegCurvetoCubicRel.x1 = 0;


	/* -- type: HTMLPhraseElement -- */

	HTMLPhraseElement.dateTime = '';
	HTMLPhraseElement.cite = '';


	/* -- type: SVGPolygonElement -- */
	_implement(SVGPolygonElement, SVGStylable);
	_implement(SVGPolygonElement, SVGTransformable);
	_implement(SVGPolygonElement, SVGLangSpace);
	_implement(SVGPolygonElement, SVGAnimatedPoints);
	_implement(SVGPolygonElement, SVGTests);



	/* -- type: SVGLength -- */

	SVGLength.valueAsString = '';
	SVGLength.valueInSpecifiedUnits = 0;
	SVGLength.value = 0;
	SVGLength.unitType = 0;
	SVGLength.SVG_LENGTHTYPE_NUMBER = 1;
	SVGLength.SVG_LENGTHTYPE_PC = 10;
	SVGLength.SVG_LENGTHTYPE_CM = 6;
	SVGLength.SVG_LENGTHTYPE_PERCENTAGE = 2;
	SVGLength.SVG_LENGTHTYPE_MM = 7;
	SVGLength.SVG_LENGTHTYPE_PT = 9;
	SVGLength.SVG_LENGTHTYPE_IN = 8;
	SVGLength.SVG_LENGTHTYPE_EMS = 3;
	SVGLength.SVG_LENGTHTYPE_UNKNOWN = 0;
	SVGLength.SVG_LENGTHTYPE_PX = 5;
	SVGLength.SVG_LENGTHTYPE_EXS = 4;
	SVGLength.newValueSpecifiedUnits = function(unitType, valueInSpecifiedUnits) { 
		/// <signature>
		/// <param name='unitType' type='Number' />
		/// <param name='valueInSpecifiedUnits' type='Number' />
		/// </signature>
	};
	SVGLength.convertToSpecifiedUnits = function(unitType) { 
		/// <signature>
		/// <param name='unitType' type='Number' />
		/// </signature>
	};


	/* -- type: XDomainRequest -- */

	XDomainRequest.timeout = 0;
	XDomainRequest.responseText = '';
	XDomainRequest.contentType = '';
	XDomainRequest.open = function(method, url) { 
		/// <signature>
		/// <param name='method' type='String' />
		/// <param name='url' type='String' />
		/// </signature>
	};
	XDomainRequest.send = function(data) { 
		/// <signature>
		/// <param name='data' type='Object' optional='true' />
		/// </signature>
	};
	XDomainRequest.abort = function() { };
	_events(XDomainRequest, "onprogress", "onload", "onerror", "ontimeout");


	/* -- type: SVGStringList -- */

	SVGStringList.numberOfItems = 0;
	SVGStringList.replaceItem = function(newItem, index) { 
		/// <signature>
		/// <param name='newItem' type='String' />
		/// <param name='index' type='Number' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	SVGStringList.getItem = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	SVGStringList.appendItem = function(newItem) { 
		/// <signature>
		/// <param name='newItem' type='String' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	SVGStringList.clear = function() { };
	SVGStringList.removeItem = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	SVGStringList.initialize = function(newItem) { 
		/// <signature>
		/// <param name='newItem' type='String' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	SVGStringList.insertItemBefore = function(newItem, index) { 
		/// <signature>
		/// <param name='newItem' type='String' />
		/// <param name='index' type='Number' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};


	/* -- type: SVGPathSegMovetoAbs -- */

	SVGPathSegMovetoAbs.y = 0;
	SVGPathSegMovetoAbs.x = 0;


	/* -- type: SVGPathSegArcRel -- */

	SVGPathSegArcRel.y = 0;
	SVGPathSegArcRel.sweepFlag = false;
	SVGPathSegArcRel.r2 = 0;
	SVGPathSegArcRel.angle = 0;
	SVGPathSegArcRel.x = 0;
	SVGPathSegArcRel.largeArcFlag = false;
	SVGPathSegArcRel.r1 = 0;


	/* -- type: SVGMetadataElement -- */



	/* -- type: WindowLocalStorage -- */

	WindowLocalStorage.localStorage = Storage;


	/* -- type: NavigatorOnLine -- */

	NavigatorOnLine.onLine = false;


	/* -- type: DOMHTMLImplementation -- */

	DOMHTMLImplementation.createHTMLDocument = function(title) { 
		/// <signature>
		/// <param name='title' type='String' />
		/// <returns type='Document'/>
		/// </signature>
		return Document; 
	};


	/* -- type: EventException -- */

	EventException.message = '';
	EventException.code = 0;
	EventException.DISPATCH_REQUEST_ERR = 1;
	EventException.UNSPECIFIED_EVENT_TYPE_ERR = 0;
	EventException.toString = function() { 
		/// <signature>
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};


	/* -- type: MSHTMLPreElementExtensions -- */
	_implement(MSHTMLPreElementExtensions, DOML2DeprecatedTextFlowControl_HTMLBlockElement);

	MSHTMLPreElementExtensions.cite = '';


	/* -- type: HTMLPreElement -- */
	_implement(HTMLPreElement, DOML2DeprecatedWidthStyle);
	_implement(HTMLPreElement, MSHTMLPreElementExtensions);



	/* -- type: DOML2DeprecatedAlignmentStyle_HTMLInputElement -- */

	DOML2DeprecatedAlignmentStyle_HTMLInputElement.align = '';


	/* -- type: PerformanceTiming -- */

	PerformanceTiming.responseStart = 0;
	PerformanceTiming.domainLookupEnd = 0;
	PerformanceTiming.redirectStart = 0;
	PerformanceTiming.domComplete = 0;
	PerformanceTiming.msFirstPaint = 0;
	PerformanceTiming.loadEventStart = 0;
	PerformanceTiming.domainLookupStart = 0;
	PerformanceTiming.domInteractive = 0;
	PerformanceTiming.requestStart = 0;
	PerformanceTiming.fetchStart = 0;
	PerformanceTiming.unloadEventEnd = 0;
	PerformanceTiming.navigationStart = 0;
	PerformanceTiming.loadEventEnd = 0;
	PerformanceTiming.connectEnd = 0;
	PerformanceTiming.connectStart = 0;
	PerformanceTiming.domLoading = 0;
	PerformanceTiming.responseEnd = 0;
	PerformanceTiming.redirectEnd = 0;
	PerformanceTiming.unloadEventStart = 0;
	PerformanceTiming.domContentLoadedEventEnd = 0;
	PerformanceTiming.domContentLoadedEventStart = 0;
	PerformanceTiming.toJSON = function() { 
		/// <signature>
		/// <returns type='Object'/>
		/// </signature>
		return new Object(); 
	};


	/* -- type: SVGAnimatedNumber -- */

	SVGAnimatedNumber.animVal = 0;
	SVGAnimatedNumber.baseVal = 0;


	/* -- type: HTMLParamElement -- */

	HTMLParamElement.value = '';
	HTMLParamElement.name = '';
	HTMLParamElement.type = '';
	HTMLParamElement.valueType = '';


	/* -- type: SVGImageElement -- */
	_implement(SVGImageElement, SVGStylable);
	_implement(SVGImageElement, SVGTransformable);
	_implement(SVGImageElement, SVGLangSpace);
	_implement(SVGImageElement, SVGTests);
	_implement(SVGImageElement, SVGURIReference);

	SVGImageElement.width = SVGAnimatedLength;
	SVGImageElement.y = SVGAnimatedLength;
	SVGImageElement.preserveAspectRatio = SVGAnimatedPreserveAspectRatio;
	SVGImageElement.x = SVGAnimatedLength;
	SVGImageElement.height = SVGAnimatedLength;


	/* -- type: MSHTMLAnchorElementExtensions -- */

	MSHTMLAnchorElementExtensions.urn = '';
	MSHTMLAnchorElementExtensions.protocolLong = '';
	MSHTMLAnchorElementExtensions.nameProp = '';
	MSHTMLAnchorElementExtensions.mimeType = '';
	MSHTMLAnchorElementExtensions.Methods = '';


	/* -- type: HTMLAnchorElement -- */
	_implement(HTMLAnchorElement, MSDataBindingExtensions);
	_implement(HTMLAnchorElement, MSHTMLAnchorElementExtensions);

	HTMLAnchorElement.protocol = '';
	HTMLAnchorElement.rel = '';
	HTMLAnchorElement.search = '';
	HTMLAnchorElement.coords = '';
	HTMLAnchorElement.hostname = '';
	HTMLAnchorElement.pathname = '';
	HTMLAnchorElement.target = '';
	HTMLAnchorElement.href = '';
	HTMLAnchorElement.name = '';
	HTMLAnchorElement.charset = '';
	HTMLAnchorElement.hreflang = '';
	HTMLAnchorElement.port = '';
	HTMLAnchorElement.host = '';
	HTMLAnchorElement.hash = '';
	HTMLAnchorElement.rev = '';
	HTMLAnchorElement.type = '';
	HTMLAnchorElement.shape = '';
	HTMLAnchorElement.toString = function() { 
		/// <signature>
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};


	/* -- type: MSHTMLInputElementExtensions -- */
	_implement(MSHTMLInputElementExtensions, DOML2DeprecatedBorderStyle_HTMLInputElement);
	_implement(MSHTMLInputElementExtensions, DOML2DeprecatedMarginStyle_HTMLInputElement);

	MSHTMLInputElementExtensions.status = false;
	MSHTMLInputElementExtensions.complete = false;
	MSHTMLInputElementExtensions.createTextRange = function() { 
		/// <signature>
		/// <returns type='TextRange'/>
		/// </signature>
		return TextRange; 
	};


	/* -- type: MSImageResourceExtensions_HTMLInputElement -- */

	MSImageResourceExtensions_HTMLInputElement.dynsrc = '';
	MSImageResourceExtensions_HTMLInputElement.vrml = '';
	MSImageResourceExtensions_HTMLInputElement.loop = 0;
	MSImageResourceExtensions_HTMLInputElement.start = '';
	MSImageResourceExtensions_HTMLInputElement.lowsrc = '';


	/* -- type: HTMLInputElement -- */
	_implement(HTMLInputElement, MSImageResourceExtensions_HTMLInputElement);
	_implement(HTMLInputElement, DOML2DeprecatedAlignmentStyle_HTMLInputElement);
	_implement(HTMLInputElement, MSHTMLInputElementExtensions);
	_implement(HTMLInputElement, MSDataBindingExtensions);

	HTMLInputElement.width = '';
	HTMLInputElement.accept = '';
	HTMLInputElement.alt = '';
	HTMLInputElement.defaultChecked = false;
	HTMLInputElement.src = '';
	HTMLInputElement.value = '';
	HTMLInputElement.form = HTMLFormElement;
	HTMLInputElement.name = '';
	HTMLInputElement.useMap = '';
	HTMLInputElement.selectionStart = 0;
	HTMLInputElement.height = '';
	HTMLInputElement.size = 0;
	HTMLInputElement.readOnly = false;
	HTMLInputElement.indeterminate = false;
	HTMLInputElement.checked = false;
	HTMLInputElement.maxLength = 0;
	HTMLInputElement.selectionEnd = 0;
	HTMLInputElement.type = '';
	HTMLInputElement.defaultValue = '';
	HTMLInputElement.setSelectionRange = function(start, end) { 
		/// <signature>
		/// <param name='start' type='Number' />
		/// <param name='end' type='Number' />
		/// </signature>
	};
	HTMLInputElement.select = function() { };


	/* -- type: HTMLTableSectionElement -- */
	_implement(HTMLTableSectionElement, MSHTMLTableSectionElementExtensions);
	_implement(HTMLTableSectionElement, HTMLTableAlignment);
	_implement(HTMLTableSectionElement, DOML2DeprecatedAlignmentStyle_HTMLTableSectionElement);

	HTMLTableSectionElement.rows = HTMLCollection;
	HTMLTableSectionElement.deleteRow = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' optional='true' />
		/// </signature>
	};
	HTMLTableSectionElement.insertRow = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' optional='true' />
		/// <returns type='HTMLElement'/>
		/// </signature>
		return HTMLElement; 
	};


	/* -- type: DragEvent -- */

	DragEvent.dataTransfer = DataTransfer;
	DragEvent.initDragEvent = function(typeArg, canBubbleArg, cancelableArg, viewArg, detailArg, screenXArg, screenYArg, clientXArg, clientYArg, ctrlKeyArg, altKeyArg, shiftKeyArg, metaKeyArg, buttonArg, relatedTargetArg, dataTransferArg) { 
		/// <signature>
		/// <param name='typeArg' type='String' />
		/// <param name='canBubbleArg' type='Boolean' />
		/// <param name='cancelableArg' type='Boolean' />
		/// <param name='viewArg' type='AbstractView' />
		/// <param name='detailArg' type='Number' />
		/// <param name='screenXArg' type='Number' />
		/// <param name='screenYArg' type='Number' />
		/// <param name='clientXArg' type='Number' />
		/// <param name='clientYArg' type='Number' />
		/// <param name='ctrlKeyArg' type='Boolean' />
		/// <param name='altKeyArg' type='Boolean' />
		/// <param name='shiftKeyArg' type='Boolean' />
		/// <param name='metaKeyArg' type='Boolean' />
		/// <param name='buttonArg' type='Number' />
		/// <param name='relatedTargetArg' type='EventTarget' />
		/// <param name='dataTransferArg' type='DataTransfer' />
		/// </signature>
	};


	/* -- type: MutationEvent -- */

	MutationEvent.attrChange = 0;
	MutationEvent.newValue = '';
	MutationEvent.attrName = '';
	MutationEvent.prevValue = '';
	MutationEvent.relatedNode = Node;
	MutationEvent.MODIFICATION = 1;
	MutationEvent.REMOVAL = 3;
	MutationEvent.ADDITION = 2;
	MutationEvent.initMutationEvent = function(typeArg, canBubbleArg, cancelableArg, relatedNodeArg, prevValueArg, newValueArg, attrNameArg, attrChangeArg) { 
		/// <signature>
		/// <param name='typeArg' type='String' />
		/// <param name='canBubbleArg' type='Boolean' />
		/// <param name='cancelableArg' type='Boolean' />
		/// <param name='relatedNodeArg' type='Node' />
		/// <param name='prevValueArg' type='String' />
		/// <param name='newValueArg' type='String' />
		/// <param name='attrNameArg' type='String' />
		/// <param name='attrChangeArg' type='Number' />
		/// </signature>
	};


	/* -- type: SVGRadialGradientElement -- */

	SVGRadialGradientElement.cx = SVGAnimatedLength;
	SVGRadialGradientElement.r = SVGAnimatedLength;
	SVGRadialGradientElement.fx = SVGAnimatedLength;
	SVGRadialGradientElement.cy = SVGAnimatedLength;
	SVGRadialGradientElement.fy = SVGAnimatedLength;


	/* -- type: DOML2DeprecatedAlignmentStyle_HTMLLegendElement -- */

	DOML2DeprecatedAlignmentStyle_HTMLLegendElement.align = '';


	/* -- type: DocumentType -- */

	DocumentType.name = '';
	DocumentType.internalSubset = '';
	DocumentType.systemId = '';
	DocumentType.notations = NamedNodeMap;
	DocumentType.publicId = '';
	DocumentType.entities = NamedNodeMap;


	/* -- type: DOML2DeprecatedWordWrapSuppression_HTMLBodyElement -- */

	DOML2DeprecatedWordWrapSuppression_HTMLBodyElement.noWrap = false;


	/* -- type: MSHTMLBodyElementExtensions -- */
	_implement(MSHTMLBodyElementExtensions, DOML2DeprecatedWordWrapSuppression_HTMLBodyElement);

	MSHTMLBodyElementExtensions.scroll = '';
	MSHTMLBodyElementExtensions.bottomMargin = new Object();
	MSHTMLBodyElementExtensions.topMargin = new Object();
	MSHTMLBodyElementExtensions.rightMargin = new Object();
	MSHTMLBodyElementExtensions.leftMargin = new Object();
	MSHTMLBodyElementExtensions.bgProperties = '';
	MSHTMLBodyElementExtensions.createTextRange = function() { 
		/// <signature>
		/// <returns type='TextRange'/>
		/// </signature>
		return TextRange; 
	};


	/* -- type: HTMLBodyElement -- */
	_implement(HTMLBodyElement, MSHTMLBodyElementExtensions);
	_implement(HTMLBodyElement, HTMLBodyElementDOML2Deprecated);
	_implement(HTMLBodyElement, DOML2DeprecatedBackgroundColorStyle);
	_implement(HTMLBodyElement, DOML2DeprecatedBackgroundStyle);

	_events(HTMLBodyElement, "onresize", "ononline", "onafterprint", "onbeforeprint", "onoffline", "onblur", "onfocus", "onhashchange", "onunload", "onmessage", "onerror", "onload", "onbeforeunload", "onstorage");


	/* -- type: MSNavigatorAbilities -- */

	MSNavigatorAbilities.userLanguage = '';
	MSNavigatorAbilities.cookieEnabled = false;
	MSNavigatorAbilities.plugins = MSPluginsCollection;
	MSNavigatorAbilities.cpuClass = '';
	MSNavigatorAbilities.appCodeName = '';
	MSNavigatorAbilities.appMinorVersion = '';
	MSNavigatorAbilities.connectionSpeed = 0;
	MSNavigatorAbilities.browserLanguage = '';
	MSNavigatorAbilities.mimeTypes = MSMimeTypesCollection;
	MSNavigatorAbilities.systemLanguage = '';
	MSNavigatorAbilities.javaEnabled = function() { 
		/// <signature>
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	MSNavigatorAbilities.taintEnabled = function() { 
		/// <signature>
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};


	/* -- type: TextRangeCollection -- */

	TextRangeCollection.length = 0;
	TextRangeCollection.item = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='TextRange'/>
		/// </signature>
		return TextRange; 
	};
	/* Add a single array element */
	TextRangeCollection.push(TextRange);


	/* -- type: DOML2DeprecatedAlignmentStyle_HTMLIFrameElement -- */

	DOML2DeprecatedAlignmentStyle_HTMLIFrameElement.align = '';


	/* -- type: HTMLIFrameElement -- */
	_implement(HTMLIFrameElement, GetSVGDocument);
	_implement(HTMLIFrameElement, MSHTMLIFrameElementExtensions);
	_implement(HTMLIFrameElement, MSDataBindingExtensions);
	_implement(HTMLIFrameElement, DOML2DeprecatedAlignmentStyle_HTMLIFrameElement);

	HTMLIFrameElement.width = '';
	HTMLIFrameElement.scrolling = '';
	HTMLIFrameElement.contentWindow = Window;
	HTMLIFrameElement.marginHeight = '';
	HTMLIFrameElement.src = '';
	HTMLIFrameElement.name = '';
	HTMLIFrameElement.marginWidth = '';
	HTMLIFrameElement.height = '';
	HTMLIFrameElement.contentDocument = Document;
	HTMLIFrameElement.longDesc = '';
	HTMLIFrameElement.frameBorder = '';


	/* -- type: MSStorageExtensions -- */



	/* -- type: Storage -- */
	_implement(Storage, MSStorageExtensions);

	Storage.length = 0;
	Storage.getItem = function(key) { 
		/// <signature>
		/// <param name='key' type='String' />
		/// <returns type='Object'/>
		/// </signature>
		return new Object(); 
	};
	Storage.setItem = function(key, data) { 
		/// <signature>
		/// <param name='key' type='String' />
		/// <param name='data' type='String' />
		/// </signature>
	};
	Storage.clear = function() { };
	Storage.removeItem = function(key) { 
		/// <signature>
		/// <param name='key' type='String' />
		/// </signature>
	};
	Storage.key = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};


	/* -- type: DocumentTraversal -- */

	DocumentTraversal.createNodeIterator = function(root, whatToShow, filter, entityReferenceExpansion) { 
		/// <signature>
		/// <param name='root' type='Node' />
		/// <param name='whatToShow' type='Number' />
		/// <param name='filter' type='NodeFilterCallback' />
		/// <param name='entityReferenceExpansion' type='Boolean' />
		/// <returns type='NodeIterator'/>
		/// </signature>
		return NodeIterator; 
	};
	DocumentTraversal.createTreeWalker = function(root, whatToShow, filter, entityReferenceExpansion) { 
		/// <signature>
		/// <param name='root' type='Node' />
		/// <param name='whatToShow' type='Number' />
		/// <param name='filter' type='NodeFilterCallback' />
		/// <param name='entityReferenceExpansion' type='Boolean' />
		/// <returns type='TreeWalker'/>
		/// </signature>
		return TreeWalker; 
	};


	/* -- type: CSS3Properties -- */

	CSS3Properties.textAlignLast = '';
	CSS3Properties.textUnderlinePosition = '';
	CSS3Properties.borderTopLeftRadius = '';
	CSS3Properties.wordWrap = '';
	CSS3Properties.backgroundClip = '';
	CSS3Properties.msTransformOrigin = '';
	CSS3Properties.opacity = '';
	CSS3Properties.overflowY = '';
	CSS3Properties.boxShadow = '';
	CSS3Properties.backgroundSize = '';
	CSS3Properties.wordBreak = '';
	CSS3Properties.boxSizing = '';
	CSS3Properties.rubyOverhang = '';
	CSS3Properties.rubyAlign = '';
	CSS3Properties.textJustify = '';
	CSS3Properties.borderRadius = '';
	CSS3Properties.msTransform = '';
	CSS3Properties.borderTopRightRadius = '';
	CSS3Properties.overflowX = '';
	CSS3Properties.borderBottomLeftRadius = '';
	CSS3Properties.borderBottomRightRadius = '';
	CSS3Properties.rubyPosition = '';
	CSS3Properties.textOverflow = '';
	CSS3Properties.backgroundOrigin = '';


	/* -- type: MSCSSStyleDeclarationExtensions -- */

	MSCSSStyleDeclarationExtensions.setAttribute = function(attributeName, AttributeValue, flags) { 
		/// <signature>
		/// <param name='attributeName' type='String' />
		/// <param name='AttributeValue' type='Object' />
		/// <param name='flags' type='Number' optional='true' />
		/// </signature>
		_setAttribute(this, attributeName, AttributeValue);
	};
	MSCSSStyleDeclarationExtensions.getAttribute = function(attributeName, flags) { 
		/// <signature>
		/// <param name='attributeName' type='String' />
		/// <param name='flags' type='Number' optional='true' />
		/// <returns type='Object'/>
		/// </signature>
		return _getAttribute(this, attributeName);
	};
	MSCSSStyleDeclarationExtensions.removeAttribute = function(attributeName, flags) { 
		/// <signature>
		/// <param name='attributeName' type='String' />
		/// <param name='flags' type='Number' optional='true' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};


	/* -- type: CSS2Properties -- */

	CSS2Properties.visibility = '';
	CSS2Properties.backgroundAttachment = '';
	CSS2Properties.fontFamily = '';
	CSS2Properties.borderRightStyle = '';
	CSS2Properties.content = '';
	CSS2Properties.clear = '';
	CSS2Properties.orphans = '';
	CSS2Properties.counterIncrement = '';
	CSS2Properties.marginBottom = '';
	CSS2Properties.counterReset = '';
	CSS2Properties.borderStyle = '';
	CSS2Properties.outlineWidth = '';
	CSS2Properties.marginRight = '';
	CSS2Properties.paddingLeft = '';
	CSS2Properties.borderBottom = '';
	CSS2Properties.marginTop = '';
	CSS2Properties.borderTopColor = '';
	CSS2Properties.top = '';
	CSS2Properties.fontWeight = '';
	CSS2Properties.textIndent = '';
	CSS2Properties.borderRight = '';
	CSS2Properties.width = '';
	CSS2Properties.listStyleImage = '';
	CSS2Properties.cursor = '';
	CSS2Properties.listStylePosition = '';
	CSS2Properties.borderTopStyle = '';
	CSS2Properties.direction = '';
	CSS2Properties.maxWidth = '';
	CSS2Properties.color = '';
	CSS2Properties.clip = '';
	CSS2Properties.borderRightWidth = '';
	CSS2Properties.verticalAlign = '';
	CSS2Properties.pageBreakAfter = '';
	CSS2Properties.overflow = '';
	CSS2Properties.fontStretch = '';
	CSS2Properties.borderLeftStyle = '';
	CSS2Properties.borderBottomStyle = '';
	CSS2Properties.emptyCells = '';
	CSS2Properties.padding = '';
	CSS2Properties.paddingRight = '';
	CSS2Properties.background = '';
	CSS2Properties.height = '';
	CSS2Properties.bottom = '';
	CSS2Properties.paddingTop = '';
	CSS2Properties.right = '';
	CSS2Properties.borderLeft = '';
	CSS2Properties.borderLeftWidth = '';
	CSS2Properties.backgroundPosition = '';
	CSS2Properties.widows = '';
	CSS2Properties.backgroundColor = '';
	CSS2Properties.pageBreakInside = '';
	CSS2Properties.lineHeight = '';
	CSS2Properties.borderTopWidth = '';
	CSS2Properties.left = '';
	CSS2Properties.outlineStyle = '';
	CSS2Properties.borderTop = '';
	CSS2Properties.paddingBottom = '';
	CSS2Properties.outlineColor = '';
	CSS2Properties.wordSpacing = '';
	CSS2Properties.outline = '';
	CSS2Properties.font = '';
	CSS2Properties.marginLeft = '';
	CSS2Properties.display = '';
	CSS2Properties.maxHeight = '';
	CSS2Properties.cssFloat = '';
	CSS2Properties.letterSpacing = '';
	CSS2Properties.borderSpacing = '';
	CSS2Properties.backgroundRepeat = '';
	CSS2Properties.fontSizeAdjust = '';
	CSS2Properties.borderLeftColor = '';
	CSS2Properties.borderWidth = '';
	CSS2Properties.backgroundImage = '';
	CSS2Properties.whiteSpace = '';
	CSS2Properties.listStyleType = '';
	CSS2Properties.fontStyle = '';
	CSS2Properties.minWidth = '';
	CSS2Properties.borderBottomColor = '';
	CSS2Properties.zIndex = '';
	CSS2Properties.position = '';
	CSS2Properties.borderColor = '';
	CSS2Properties.listStyle = '';
	CSS2Properties.captionSide = '';
	CSS2Properties.borderCollapse = '';
	CSS2Properties.tableLayout = '';
	CSS2Properties.quotes = '';
	CSS2Properties.fontVariant = '';
	CSS2Properties.unicodeBidi = '';
	CSS2Properties.minHeight = '';
	CSS2Properties.borderBottomWidth = '';
	CSS2Properties.textDecoration = '';
	CSS2Properties.fontSize = '';
	CSS2Properties.pageBreakBefore = '';
	CSS2Properties.border = '';
	CSS2Properties.textAlign = '';
	CSS2Properties.textTransform = '';
	CSS2Properties.margin = '';
	CSS2Properties.borderRightColor = '';


	/* -- type: CSSStyleDeclaration -- */
	_implement(CSSStyleDeclaration, CSS3Properties);
	_implement(CSSStyleDeclaration, SVG1_1Properties);
	_implement(CSSStyleDeclaration, CSS2Properties);

	CSSStyleDeclaration.length = 0;
	CSSStyleDeclaration.cssText = '';
	CSSStyleDeclaration.parentRule = CSSRule;
	CSSStyleDeclaration.getPropertyPriority = function(propertyName) { 
		/// <signature>
		/// <param name='propertyName' type='String' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	CSSStyleDeclaration.removeProperty = function(propertyName) { 
		/// <signature>
		/// <param name='propertyName' type='String' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	CSSStyleDeclaration.getPropertyValue = function(propertyName) { 
		/// <signature>
		/// <param name='propertyName' type='String' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	CSSStyleDeclaration.item = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	CSSStyleDeclaration.setProperty = function(propertyName, value, priority) { 
		/// <signature>
		/// <param name='propertyName' type='String' />
		/// <param name='value' type='String' />
		/// <param name='priority' type='String' optional='true' />
		/// </signature>
	};


	/* -- type: MSCSSProperties -- */
	_implement(MSCSSProperties, MSCSSStyleDeclarationExtensions);

	MSCSSProperties.scrollbarShadowColor = '';
	MSCSSProperties.scrollbarHighlightColor = '';
	MSCSSProperties.layoutGridChar = '';
	MSCSSProperties.layoutGridType = '';
	MSCSSProperties.textKashidaSpace = '';
	MSCSSProperties.textAutospace = '';
	MSCSSProperties.writingMode = '';
	MSCSSProperties.scrollbarFaceColor = '';
	MSCSSProperties.backgroundPositionY = '';
	MSCSSProperties.lineBreak = '';
	MSCSSProperties.msBlockProgression = '';
	MSCSSProperties.imeMode = '';
	MSCSSProperties.scrollbarBaseColor = '';
	MSCSSProperties.layoutGridLine = '';
	MSCSSProperties.layoutFlow = '';
	MSCSSProperties.layoutGrid = '';
	MSCSSProperties.textKashida = '';
	MSCSSProperties.zoom = '';
	MSCSSProperties.filter = '';
	MSCSSProperties.scrollbarArrowColor = '';
	MSCSSProperties.accelerator = '';
	MSCSSProperties.backgroundPositionX = '';
	MSCSSProperties.behavior = '';
	MSCSSProperties.textJustifyTrim = '';
	MSCSSProperties.layoutGridMode = '';
	MSCSSProperties.scrollbar3dLightColor = '';
	MSCSSProperties.msInterpolationMode = '';
	MSCSSProperties.scrollbarTrackColor = '';
	MSCSSProperties.styleFloat = '';
	MSCSSProperties.scrollbarDarkShadowColor = '';


	/* -- type: MSCurrentStyleCSSProperties -- */

	MSCurrentStyleCSSProperties.blockDirection = '';
	MSCurrentStyleCSSProperties.clipBottom = '';
	MSCurrentStyleCSSProperties.clipTop = '';
	MSCurrentStyleCSSProperties.clipRight = '';
	MSCurrentStyleCSSProperties.clipLeft = '';
	MSCurrentStyleCSSProperties.hasLayout = '';


	/* -- type: SVGStyleElement -- */
	_implement(SVGStyleElement, SVGLangSpace);

	SVGStyleElement.media = '';
	SVGStyleElement.title = '';
	SVGStyleElement.type = '';


	/* -- type: ViewCSS -- */

	ViewCSS.getComputedStyle = function(elt, pseudoElt) { 
		/// <signature>
		/// <param name='elt' type='Element' />
		/// <param name='pseudoElt' type='String' optional='true' />
		/// <returns type='CSSStyleDeclaration'/>
		/// </signature>
		return CSSStyleDeclaration; 
	};


	/* -- type: HTMLLIElement -- */
	_implement(HTMLLIElement, DOML2DeprecatedListNumberingAndBulletStyle);

	HTMLLIElement.value = 0;


	/* -- type: SVGPathSegLinetoVerticalAbs -- */

	SVGPathSegLinetoVerticalAbs.y = 0;


	/* -- type: SVGTextPositioningElement -- */

	SVGTextPositioningElement.y = SVGAnimatedLengthList;
	SVGTextPositioningElement.rotate = SVGAnimatedNumberList;
	SVGTextPositioningElement.dy = SVGAnimatedLengthList;
	SVGTextPositioningElement.dx = SVGAnimatedLengthList;
	SVGTextPositioningElement.x = SVGAnimatedLengthList;


	/* -- type: SVGTSpanElement -- */



	/* -- type: SVGTextElement -- */
	_implement(SVGTextElement, SVGTransformable);



	/* -- type: SVGAnimatedInteger -- */

	SVGAnimatedInteger.animVal = 0;
	SVGAnimatedInteger.baseVal = 0;


	/* -- type: NavigatorAbilities -- */



	/* -- type: MSHTMLImageElementExtensions -- */

	MSHTMLImageElementExtensions.href = '';


	/* -- type: HTMLLegendElement -- */
	_implement(HTMLLegendElement, DOML2DeprecatedAlignmentStyle_HTMLLegendElement);
	_implement(HTMLLegendElement, MSDataBindingExtensions);

	HTMLLegendElement.form = HTMLFormElement;


	/* -- type: MSHTMLDirectoryElementExtensions -- */
	_implement(MSHTMLDirectoryElementExtensions, DOML2DeprecatedListNumberingAndBulletStyle);



	/* -- type: HTMLDirectoryElement -- */
	_implement(HTMLDirectoryElement, DOML2DeprecatedListSpaceReduction);
	_implement(HTMLDirectoryElement, MSHTMLDirectoryElementExtensions);



	/* -- type: MSHTMLQuoteElementExtensions -- */

	MSHTMLQuoteElementExtensions.dateTime = '';


	/* -- type: HTMLLabelElement -- */
	_implement(HTMLLabelElement, MSDataBindingExtensions);

	HTMLLabelElement.htmlFor = '';
	HTMLLabelElement.form = HTMLFormElement;


	/* -- type: DocumentEvent -- */

	DocumentEvent.createEvent = function(eventInterface) { 
		/// <signature>
		/// <param name='eventInterface' type='String' />
		/// <returns type='Event'/>
		/// </signature>
		return Event; 
	};


	/* -- type: SVGSVGElementEventHandlers -- */

	_events(SVGSVGElementEventHandlers, "onresize", "onunload", "onscroll", "onerror", "onzoom", "onabort");


	/* -- type: SVGSVGElement -- */
	_implement(SVGSVGElement, SVGZoomAndPan);
	_implement(SVGSVGElement, SVGLangSpace);
	_implement(SVGSVGElement, SVGFitToViewBox);
	_implement(SVGSVGElement, SVGTests);
	_implement(SVGSVGElement, SVGLocatable);
	_implement(SVGSVGElement, SVGSVGElementEventHandlers);
	_implement(SVGSVGElement, SVGStylable);
	_implement(SVGSVGElement, DocumentEvent);
	_implement(SVGSVGElement, ViewCSS_SVGSVGElement);

	SVGSVGElement.width = SVGAnimatedLength;
	SVGSVGElement.screenPixelToMillimeterY = 0;
	SVGSVGElement.contentStyleType = '';
	SVGSVGElement.x = SVGAnimatedLength;
	SVGSVGElement.height = SVGAnimatedLength;
	SVGSVGElement.contentScriptType = '';
	SVGSVGElement.y = SVGAnimatedLength;
	SVGSVGElement.pixelUnitToMillimeterX = 0;
	SVGSVGElement.currentTranslate = SVGPoint;
	SVGSVGElement.viewport = SVGRect;
	SVGSVGElement.currentScale = 0;
	SVGSVGElement.screenPixelToMillimeterX = 0;
	SVGSVGElement.pixelUnitToMillimeterY = 0;
	SVGSVGElement.createSVGLength = function() { 
		/// <signature>
		/// <returns type='SVGLength'/>
		/// </signature>
		return SVGLength; 
	};
	SVGSVGElement.setCurrentTime = function(seconds) { 
		/// <signature>
		/// <param name='seconds' type='Number' />
		/// </signature>
	};
	SVGSVGElement.getIntersectionList = function(rect, referenceElement) { 
		/// <signature>
		/// <param name='rect' type='SVGRect' />
		/// <param name='referenceElement' type='SVGElement' />
		/// <returns type='NodeList'/>
		/// </signature>
		return NodeList; 
	};
	SVGSVGElement.unpauseAnimations = function() { };
	SVGSVGElement.createSVGRect = function() { 
		/// <signature>
		/// <returns type='SVGRect'/>
		/// </signature>
		return SVGRect; 
	};
	SVGSVGElement.checkIntersection = function(element, rect) { 
		/// <signature>
		/// <param name='element' type='SVGElement' />
		/// <param name='rect' type='SVGRect' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	SVGSVGElement.unsuspendRedrawAll = function() { };
	SVGSVGElement.pauseAnimations = function() { };
	SVGSVGElement.deselectAll = function() { };
	SVGSVGElement.suspendRedraw = function(maxWaitMilliseconds) { 
		/// <signature>
		/// <param name='maxWaitMilliseconds' type='Number' />
		/// <returns type='Number'/>
		/// </signature>
		return 0; 
	};
	SVGSVGElement.createSVGAngle = function() { 
		/// <signature>
		/// <returns type='SVGAngle'/>
		/// </signature>
		return SVGAngle; 
	};
	SVGSVGElement.getEnclosureList = function(rect, referenceElement) { 
		/// <signature>
		/// <param name='rect' type='SVGRect' />
		/// <param name='referenceElement' type='SVGElement' />
		/// <returns type='NodeList'/>
		/// </signature>
		return NodeList; 
	};
	SVGSVGElement.createSVGTransform = function() { 
		/// <signature>
		/// <returns type='SVGTransform'/>
		/// </signature>
		return SVGTransform; 
	};
	SVGSVGElement.unsuspendRedraw = function(suspendHandleID) { 
		/// <signature>
		/// <param name='suspendHandleID' type='Number' />
		/// </signature>
	};
	SVGSVGElement.forceRedraw = function() { };
	SVGSVGElement.getCurrentTime = function() { 
		/// <signature>
		/// <returns type='Number'/>
		/// </signature>
		return 0; 
	};
	SVGSVGElement.checkEnclosure = function(element, rect) { 
		/// <signature>
		/// <param name='element' type='SVGElement' />
		/// <param name='rect' type='SVGRect' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	SVGSVGElement.createSVGMatrix = function() { 
		/// <signature>
		/// <returns type='SVGMatrix'/>
		/// </signature>
		return SVGMatrix; 
	};
	SVGSVGElement.createSVGPoint = function() { 
		/// <signature>
		/// <returns type='SVGPoint'/>
		/// </signature>
		return SVGPoint; 
	};
	SVGSVGElement.createSVGNumber = function() { 
		/// <signature>
		/// <returns type='SVGNumber'/>
		/// </signature>
		return SVGNumber; 
	};
	SVGSVGElement.createSVGTransformFromMatrix = function(matrix) { 
		/// <signature>
		/// <param name='matrix' type='SVGMatrix' />
		/// <returns type='SVGTransform'/>
		/// </signature>
		return SVGTransform; 
	};
	SVGSVGElement.getElementById = function(elementId) { 
		/// <signature>
		/// <param name='elementId' type='String' />
		/// <returns type='Element'/>
		/// </signature>
		return _getElementById(elementId);
	};


	/* -- type: MSPluginsCollection -- */

	MSPluginsCollection.length = 0;
	MSPluginsCollection.refresh = function(reload) { 
		/// <signature>
		/// <param name='reload' type='Boolean' optional='true' />
		/// </signature>
	};


	/* -- type: SVGAnimatedNumberList -- */

	SVGAnimatedNumberList.animVal = SVGNumberList;
	SVGAnimatedNumberList.baseVal = SVGNumberList;


	/* -- type: SVGPoint -- */

	SVGPoint.y = 0;
	SVGPoint.x = 0;
	SVGPoint.matrixTransform = function(matrix) { 
		/// <signature>
		/// <param name='matrix' type='SVGMatrix' />
		/// <returns type='SVGPoint'/>
		/// </signature>
		return SVGPoint; 
	};


	/* -- type: Range -- */

	Range.collapsed = false;
	Range.startOffset = 0;
	Range.endOffset = 0;
	Range.startContainer = Node;
	Range.commonAncestorContainer = Node;
	Range.endContainer = Node;
	Range.END_TO_END = 2;
	Range.END_TO_START = 3;
	Range.START_TO_END = 1;
	Range.START_TO_START = 0;
	Range.setEndBefore = function(refNode) { 
		/// <signature>
		/// <param name='refNode' type='Node' />
		/// </signature>
	};
	Range.setStart = function(refNode, offset) { 
		/// <signature>
		/// <param name='refNode' type='Node' />
		/// <param name='offset' type='Number' />
		/// </signature>
	};
	Range.setStartBefore = function(refNode) { 
		/// <signature>
		/// <param name='refNode' type='Node' />
		/// </signature>
	};
	Range.detach = function() { };
	Range.selectNode = function(refNode) { 
		/// <signature>
		/// <param name='refNode' type='Node' />
		/// </signature>
	};
	Range.getBoundingClientRect = function() { 
		/// <signature>
		/// <returns type='ClientRect'/>
		/// </signature>
		return ClientRect; 
	};
	Range.toString = function() { 
		/// <signature>
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	Range.compareBoundaryPoints = function(how, sourceRange) { 
		/// <signature>
		/// <param name='how' type='Number' />
		/// <param name='sourceRange' type='Range' />
		/// <returns type='Number'/>
		/// </signature>
		return 0; 
	};
	Range.insertNode = function(newNode) { 
		/// <signature>
		/// <param name='newNode' type='Node' />
		/// </signature>
	};
	Range.collapse = function(toStart) { 
		/// <signature>
		/// <param name='toStart' type='Boolean' />
		/// </signature>
	};
	Range.selectNodeContents = function(refNode) { 
		/// <signature>
		/// <param name='refNode' type='Node' />
		/// </signature>
	};
	Range.cloneContents = function() { 
		/// <signature>
		/// <returns type='DocumentFragment'/>
		/// </signature>
		return DocumentFragment; 
	};
	Range.setEnd = function(refNode, offset) { 
		/// <signature>
		/// <param name='refNode' type='Node' />
		/// <param name='offset' type='Number' />
		/// </signature>
	};
	Range.cloneRange = function() { 
		/// <signature>
		/// <returns type='Range'/>
		/// </signature>
		return Range; 
	};
	Range.getClientRects = function() { 
		/// <signature>
		/// <returns type='ClientRectList'/>
		/// </signature>
		return ClientRectList; 
	};
	Range.surroundContents = function(newParent) { 
		/// <signature>
		/// <param name='newParent' type='Node' />
		/// </signature>
	};
	Range.setStartAfter = function(refNode) { 
		/// <signature>
		/// <param name='refNode' type='Node' />
		/// </signature>
	};
	Range.deleteContents = function() { };
	Range.extractContents = function() { 
		/// <signature>
		/// <returns type='DocumentFragment'/>
		/// </signature>
		return DocumentFragment; 
	};
	Range.setEndAfter = function(refNode) { 
		/// <signature>
		/// <param name='refNode' type='Node' />
		/// </signature>
	};


	/* -- type: FocusEvent -- */

	FocusEvent.relatedTarget = EventTarget;
	FocusEvent.initFocusEvent = function(typeArg, canBubbleArg, cancelableArg, viewArg, detailArg, relatedTargetArg) { 
		/// <signature>
		/// <param name='typeArg' type='String' />
		/// <param name='canBubbleArg' type='Boolean' />
		/// <param name='cancelableArg' type='Boolean' />
		/// <param name='viewArg' type='AbstractView' />
		/// <param name='detailArg' type='Number' />
		/// <param name='relatedTargetArg' type='EventTarget' />
		/// </signature>
	};


	/* -- type: DataTransfer -- */

	DataTransfer.effectAllowed = '';
	DataTransfer.dropEffect = '';
	DataTransfer.clearData = function(format) { 
		/// <signature>
		/// <param name='format' type='String' optional='true' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	DataTransfer.getData = function(format) { 
		/// <signature>
		/// <param name='format' type='String' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	DataTransfer.setData = function(format, data) { 
		/// <signature>
		/// <param name='format' type='String' />
		/// <param name='data' type='String' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};


	/* -- type: EventListener -- */

	EventListener.handleEvent = function(evt) { 
		/// <signature>
		/// <param name='evt' type='Event' />
		/// </signature>
	};


	/* -- type: NavigatorGeolocation -- */

	NavigatorGeolocation.geolocation = Geolocation;


	/* -- type: Coordinates -- */

	Coordinates.altitudeAccuracy = 0;
	Coordinates.longitude = 0;
	Coordinates.speed = 0;
	Coordinates.latitude = 0;
	Coordinates.heading = 0;
	Coordinates.accuracy = 0;
	Coordinates.altitude = 0;


	/* -- type: MSScreenExtensions -- */

	MSScreenExtensions.fontSmoothingEnabled = false;
	MSScreenExtensions.deviceXDPI = 0;
	MSScreenExtensions.bufferDepth = 0;
	MSScreenExtensions.systemXDPI = 0;
	MSScreenExtensions.logicalXDPI = 0;
	MSScreenExtensions.updateInterval = 0;
	MSScreenExtensions.systemYDPI = 0;
	MSScreenExtensions.logicalYDPI = 0;
	MSScreenExtensions.deviceYDPI = 0;


	/* -- type: Screen -- */
	_implement(Screen, MSScreenExtensions);

	Screen.colorDepth = 0;
	Screen.width = 0;
	Screen.availWidth = 0;
	Screen.pixelDepth = 0;
	Screen.height = 0;
	Screen.availHeight = 0;


	/* -- type: MSBorderColorStyle_HTMLFrameSetElement -- */

	MSBorderColorStyle_HTMLFrameSetElement.borderColor = new Object();


	/* -- type: HTMLFrameSetElement -- */
	_implement(HTMLFrameSetElement, MSHTMLFrameSetElementExtensions);
	_implement(HTMLFrameSetElement, MSBorderColorStyle_HTMLFrameSetElement);

	HTMLFrameSetElement.rows = '';
	HTMLFrameSetElement.cols = '';
	_events(HTMLFrameSetElement, "onresize", "ononline", "onafterprint", "onbeforeprint", "onoffline", "onblur", "onfocus", "onhashchange", "onunload", "onmessage", "onerror", "onload", "onbeforeunload", "onstorage");


	/* -- type: MSHTMLMetaElementExtensions -- */

	MSHTMLMetaElementExtensions.url = '';
	MSHTMLMetaElementExtensions.charset = '';


	/* -- type: SVGAElement -- */
	_implement(SVGAElement, SVGStylable);
	_implement(SVGAElement, SVGTransformable);
	_implement(SVGAElement, SVGLangSpace);
	_implement(SVGAElement, SVGTests);
	_implement(SVGAElement, SVGURIReference);

	SVGAElement.target = SVGAnimatedString;


	/* -- type: SVGEllipseElement -- */
	_implement(SVGEllipseElement, SVGStylable);
	_implement(SVGEllipseElement, SVGTransformable);
	_implement(SVGEllipseElement, SVGLangSpace);
	_implement(SVGEllipseElement, SVGTests);

	SVGEllipseElement.ry = SVGAnimatedLength;
	SVGEllipseElement.cx = SVGAnimatedLength;
	SVGEllipseElement.rx = SVGAnimatedLength;
	SVGEllipseElement.cy = SVGAnimatedLength;


	/* -- type: SVGPathSegLinetoHorizontalRel -- */

	SVGPathSegLinetoHorizontalRel.x = 0;


	/* -- type: HTMLDListElement -- */
	_implement(HTMLDListElement, DOML2DeprecatedListSpaceReduction);



	/* -- type: HTMLTableHeaderCellElement -- */
	_implement(HTMLTableHeaderCellElement, HTMLTableHeaderCellScope);



	/* -- type: XMLHttpRequest -- */
	_implement(XMLHttpRequest, MSXMLHttpRequestExtensions);
	_implement(XMLHttpRequest, EventTarget);

	XMLHttpRequest.status = 0;
	XMLHttpRequest.responseXML = new Object();
	XMLHttpRequest.responseText = '';
	XMLHttpRequest.readyState = 0;
	XMLHttpRequest.statusText = '';
	XMLHttpRequest.LOADING = 3;
	XMLHttpRequest.DONE = 4;
	XMLHttpRequest.HEADERS_RECEIVED = 2;
	XMLHttpRequest.OPENED = 1;
	XMLHttpRequest.UNSENT = 0;
	XMLHttpRequest.open = function(method, url, async, user, password) { 
		/// <signature>
		/// <param name='method' type='String' />
		/// <param name='url' type='String' />
		/// <param name='async' type='Boolean' optional='true' />
		/// <param name='user' type='String' optional='true' />
		/// <param name='password' type='String' optional='true' />
		/// </signature>
		_openXMLHttpRequest(this, method, url, async);
	};
	XMLHttpRequest.abort = function() { };
	XMLHttpRequest.send = function(data) { 
		/// <signature>
		/// <param name='data' type='Object' optional='true' />
		/// </signature>
		_sendXMLHttpRequest(this, data);
	};
	XMLHttpRequest.getAllResponseHeaders = function() { 
		/// <signature>
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	XMLHttpRequest.getResponseHeader = function(header) { 
		/// <signature>
		/// <param name='header' type='String' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	XMLHttpRequest.setRequestHeader = function(header, value) { 
		/// <signature>
		/// <param name='header' type='String' />
		/// <param name='value' type='String' />
		/// </signature>
	};
	_events(XMLHttpRequest, "onreadystatechange", "onload");


	/* -- type: WindowModal -- */

	WindowModal.dialogArguments = new Object();
	WindowModal.returnValue = new Object();


	/* -- type: MSHTMLButtonElementExtensions -- */

	MSHTMLButtonElementExtensions.status = new Object();
	MSHTMLButtonElementExtensions.createTextRange = function() { 
		/// <signature>
		/// <returns type='TextRange'/>
		/// </signature>
		return TextRange; 
	};


	/* -- type: CSSMediaRule -- */

	CSSMediaRule.media = MediaList;
	CSSMediaRule.cssRules = CSSRuleList;
	CSSMediaRule.insertRule = function(rule, index) { 
		/// <signature>
		/// <param name='rule' type='String' />
		/// <param name='index' type='Number' optional='true' />
		/// <returns type='Number'/>
		/// </signature>
		return 0; 
	};
	CSSMediaRule.deleteRule = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' optional='true' />
		/// </signature>
	};


	/* -- type: HTMLQuoteElement -- */
	_implement(HTMLQuoteElement, MSHTMLQuoteElementExtensions);

	HTMLQuoteElement.cite = '';


	/* -- type: SVGDefsElement -- */
	_implement(SVGDefsElement, SVGStylable);
	_implement(SVGDefsElement, SVGTransformable);
	_implement(SVGDefsElement, SVGLangSpace);
	_implement(SVGDefsElement, SVGTests);



	/* -- type: SVGAnimatedLength -- */

	SVGAnimatedLength.animVal = SVGLength;
	SVGAnimatedLength.baseVal = SVGLength;


	/* -- type: MSHTMLFrameElementExtensions -- */

	MSHTMLFrameElementExtensions.width = new Object();
	MSHTMLFrameElementExtensions.contentWindow = Window;
	MSHTMLFrameElementExtensions.height = new Object();
	MSHTMLFrameElementExtensions.frameBorder = '';
	MSHTMLFrameElementExtensions.frameSpacing = new Object();
	MSHTMLFrameElementExtensions.border = '';
	_events(MSHTMLFrameElementExtensions, "onload");


	/* -- type: HTMLFrameElement -- */
	_implement(HTMLFrameElement, GetSVGDocument);
	_implement(HTMLFrameElement, MSHTMLFrameElementExtensions);
	_implement(HTMLFrameElement, MSBorderColorStyle_HTMLFrameElement);
	_implement(HTMLFrameElement, MSDataBindingExtensions);

	HTMLFrameElement.scrolling = '';
	HTMLFrameElement.src = '';
	HTMLFrameElement.marginHeight = '';
	HTMLFrameElement.name = '';
	HTMLFrameElement.marginWidth = '';
	HTMLFrameElement.contentDocument = Document;
	HTMLFrameElement.longDesc = '';
	HTMLFrameElement.noResize = false;


	/* -- type: SVGPathSegClosePath -- */



	/* -- type: HTMLHtmlElementDOML2Deprecated -- */

	HTMLHtmlElementDOML2Deprecated.version = '';


	/* -- type: HTMLHtmlElement -- */
	_implement(HTMLHtmlElement, HTMLHtmlElementDOML2Deprecated);



	/* -- type: MSBorderColorStyle -- */

	MSBorderColorStyle.borderColor = new Object();


	/* -- type: SVGTransformList -- */

	SVGTransformList.numberOfItems = 0;
	SVGTransformList.getItem = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='SVGTransform'/>
		/// </signature>
		return SVGTransform; 
	};
	SVGTransformList.consolidate = function() { 
		/// <signature>
		/// <returns type='SVGTransform'/>
		/// </signature>
		return SVGTransform; 
	};
	SVGTransformList.appendItem = function(newItem) { 
		/// <signature>
		/// <param name='newItem' type='SVGTransform' />
		/// <returns type='SVGTransform'/>
		/// </signature>
		return SVGTransform; 
	};
	SVGTransformList.clear = function() { };
	SVGTransformList.removeItem = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='SVGTransform'/>
		/// </signature>
		return SVGTransform; 
	};
	SVGTransformList.initialize = function(newItem) { 
		/// <signature>
		/// <param name='newItem' type='SVGTransform' />
		/// <returns type='SVGTransform'/>
		/// </signature>
		return SVGTransform; 
	};
	SVGTransformList.insertItemBefore = function(newItem, index) { 
		/// <signature>
		/// <param name='newItem' type='SVGTransform' />
		/// <param name='index' type='Number' />
		/// <returns type='SVGTransform'/>
		/// </signature>
		return SVGTransform; 
	};
	SVGTransformList.replaceItem = function(newItem, index) { 
		/// <signature>
		/// <param name='newItem' type='SVGTransform' />
		/// <param name='index' type='Number' />
		/// <returns type='SVGTransform'/>
		/// </signature>
		return SVGTransform; 
	};
	SVGTransformList.createSVGTransformFromMatrix = function(matrix) { 
		/// <signature>
		/// <param name='matrix' type='SVGMatrix' />
		/// <returns type='SVGTransform'/>
		/// </signature>
		return SVGTransform; 
	};


	/* -- type: SVGPathSegArcAbs -- */

	SVGPathSegArcAbs.y = 0;
	SVGPathSegArcAbs.sweepFlag = false;
	SVGPathSegArcAbs.r2 = 0;
	SVGPathSegArcAbs.angle = 0;
	SVGPathSegArcAbs.x = 0;
	SVGPathSegArcAbs.largeArcFlag = false;
	SVGPathSegArcAbs.r1 = 0;


	/* -- type: SVGPathSegLinetoHorizontalAbs -- */

	SVGPathSegLinetoHorizontalAbs.x = 0;


	/* -- type: MSCSSRuleList -- */

	MSCSSRuleList.length = 0;
	MSCSSRuleList.item = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' optional='true' />
		/// <returns type='CSSStyleRule'/>
		/// </signature>
		return CSSStyleRule; 
	};
	/* Add a single array element */
	MSCSSRuleList.push(CSSStyleRule);


	/* -- type: CanvasRenderingContext2D -- */

	CanvasRenderingContext2D.shadowOffsetX = 0;
	CanvasRenderingContext2D.miterLimit = 0;
	CanvasRenderingContext2D.lineWidth = 0;
	CanvasRenderingContext2D.font = '';
	CanvasRenderingContext2D.strokeStyle = new Object();
	CanvasRenderingContext2D.canvas = HTMLCanvasElement;
	CanvasRenderingContext2D.shadowOffsetY = 0;
	CanvasRenderingContext2D.globalCompositeOperation = '';
	CanvasRenderingContext2D.globalAlpha = 0;
	CanvasRenderingContext2D.fillStyle = new Object();
	CanvasRenderingContext2D.textAlign = '';
	CanvasRenderingContext2D.shadowBlur = 0;
	CanvasRenderingContext2D.lineCap = '';
	CanvasRenderingContext2D.textBaseline = '';
	CanvasRenderingContext2D.shadowColor = '';
	CanvasRenderingContext2D.lineJoin = '';
	CanvasRenderingContext2D.restore = function() { };
	CanvasRenderingContext2D.setTransform = function(m11, m12, m21, m22, dx, dy) { 
		/// <signature>
		/// <param name='m11' type='Number' />
		/// <param name='m12' type='Number' />
		/// <param name='m21' type='Number' />
		/// <param name='m22' type='Number' />
		/// <param name='dx' type='Number' />
		/// <param name='dy' type='Number' />
		/// </signature>
	};
	CanvasRenderingContext2D.save = function() { };
	CanvasRenderingContext2D.arc = function(x, y, radius, startAngle, endAngle, anticlockwise) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// <param name='radius' type='Number' />
		/// <param name='startAngle' type='Number' />
		/// <param name='endAngle' type='Number' />
		/// <param name='anticlockwise' type='Boolean' optional='true' />
		/// </signature>
	};
	CanvasRenderingContext2D.measureText = function(text) { 
		/// <signature>
		/// <param name='text' type='String' />
		/// <returns type='TextMetrics'/>
		/// </signature>
		return TextMetrics; 
	};
	CanvasRenderingContext2D.isPointInPath = function(x, y) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	CanvasRenderingContext2D.quadraticCurveTo = function(cpx, cpy, x, y) { 
		/// <signature>
		/// <param name='cpx' type='Number' />
		/// <param name='cpy' type='Number' />
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// </signature>
	};
	CanvasRenderingContext2D.putImageData = function(imagedata, dx, dy, dirtyX, dirtyY, dirtyWidth, dirtyHeight) { 
		/// <signature>
		/// <param name='imagedata' type='ImageData' />
		/// <param name='dx' type='Number' />
		/// <param name='dy' type='Number' />
		/// <param name='dirtyX' type='Number' optional='true' />
		/// <param name='dirtyY' type='Number' optional='true' />
		/// <param name='dirtyWidth' type='Number' optional='true' />
		/// <param name='dirtyHeight' type='Number' optional='true' />
		/// </signature>
	};
	CanvasRenderingContext2D.rotate = function(angle) { 
		/// <signature>
		/// <param name='angle' type='Number' />
		/// </signature>
	};
	CanvasRenderingContext2D.fillText = function(text, x, y, maxWidth) { 
		/// <signature>
		/// <param name='text' type='String' />
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// <param name='maxWidth' type='Number' optional='true' />
		/// </signature>
	};
	CanvasRenderingContext2D.translate = function(x, y) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// </signature>
	};
	CanvasRenderingContext2D.scale = function(x, y) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// </signature>
	};
	CanvasRenderingContext2D.createRadialGradient = function(x0, y0, r0, x1, y1, r1) { 
		/// <signature>
		/// <param name='x0' type='Number' />
		/// <param name='y0' type='Number' />
		/// <param name='r0' type='Number' />
		/// <param name='x1' type='Number' />
		/// <param name='y1' type='Number' />
		/// <param name='r1' type='Number' />
		/// <returns type='CanvasGradient'/>
		/// </signature>
		return CanvasGradient; 
	};
	CanvasRenderingContext2D.lineTo = function(x, y) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// </signature>
	};
	CanvasRenderingContext2D.fill = function() { };
	CanvasRenderingContext2D.createPattern = function(image, repetition) { 
		/// <signature>
		/// <param name='image' type='HTMLElement' />
		/// <param name='repetition' type='String' />
		/// <returns type='CanvasPattern'/>
		/// </signature>
		return CanvasPattern; 
	};
	CanvasRenderingContext2D.closePath = function() { };
	CanvasRenderingContext2D.rect = function(x, y, w, h) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// <param name='w' type='Number' />
		/// <param name='h' type='Number' />
		/// </signature>
	};
	CanvasRenderingContext2D.clip = function() { };
	CanvasRenderingContext2D.createImageData = function(imageDataOrSw, sh) { 
		/// <signature>
		/// <param name='imageDataOrSw' type='Object' />
		/// <param name='sh' type='Number' optional='true' />
		/// <returns type='ImageData'/>
		/// </signature>
		return ImageData; 
	};
	CanvasRenderingContext2D.clearRect = function(x, y, w, h) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// <param name='w' type='Number' />
		/// <param name='h' type='Number' />
		/// </signature>
	};
	CanvasRenderingContext2D.moveTo = function(x, y) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// </signature>
	};
	CanvasRenderingContext2D.getImageData = function(sx, sy, sw, sh) { 
		/// <signature>
		/// <param name='sx' type='Number' />
		/// <param name='sy' type='Number' />
		/// <param name='sw' type='Number' />
		/// <param name='sh' type='Number' />
		/// <returns type='ImageData'/>
		/// </signature>
		return ImageData; 
	};
	CanvasRenderingContext2D.fillRect = function(x, y, w, h) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// <param name='w' type='Number' />
		/// <param name='h' type='Number' />
		/// </signature>
	};
	CanvasRenderingContext2D.bezierCurveTo = function(cp1x, cp1y, cp2x, cp2y, x, y) { 
		/// <signature>
		/// <param name='cp1x' type='Number' />
		/// <param name='cp1y' type='Number' />
		/// <param name='cp2x' type='Number' />
		/// <param name='cp2y' type='Number' />
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// </signature>
	};
	CanvasRenderingContext2D.drawImage = function(image, offsetX, offsetY, width, height, canvasOffsetX, canvasOffsetY, canvasImageWidth, canvasImageHeight) { 
		/// <signature>
		/// <param name='image' type='HTMLElement' />
		/// <param name='offsetX' type='Number' />
		/// <param name='offsetY' type='Number' />
		/// <param name='width' type='Number' optional='true' />
		/// <param name='height' type='Number' optional='true' />
		/// <param name='canvasOffsetX' type='Number' optional='true' />
		/// <param name='canvasOffsetY' type='Number' optional='true' />
		/// <param name='canvasImageWidth' type='Number' optional='true' />
		/// <param name='canvasImageHeight' type='Number' optional='true' />
		/// </signature>
	};
	CanvasRenderingContext2D.transform = function(m11, m12, m21, m22, dx, dy) { 
		/// <signature>
		/// <param name='m11' type='Number' />
		/// <param name='m12' type='Number' />
		/// <param name='m21' type='Number' />
		/// <param name='m22' type='Number' />
		/// <param name='dx' type='Number' />
		/// <param name='dy' type='Number' />
		/// </signature>
	};
	CanvasRenderingContext2D.stroke = function() { };
	CanvasRenderingContext2D.strokeRect = function(x, y, w, h) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// <param name='w' type='Number' />
		/// <param name='h' type='Number' />
		/// </signature>
	};
	CanvasRenderingContext2D.strokeText = function(text, x, y, maxWidth) { 
		/// <signature>
		/// <param name='text' type='String' />
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// <param name='maxWidth' type='Number' optional='true' />
		/// </signature>
	};
	CanvasRenderingContext2D.beginPath = function() { };
	CanvasRenderingContext2D.arcTo = function(x1, y1, x2, y2, radius) { 
		/// <signature>
		/// <param name='x1' type='Number' />
		/// <param name='y1' type='Number' />
		/// <param name='x2' type='Number' />
		/// <param name='y2' type='Number' />
		/// <param name='radius' type='Number' />
		/// </signature>
	};
	CanvasRenderingContext2D.createLinearGradient = function(x0, y0, x1, y1) { 
		/// <signature>
		/// <param name='x0' type='Number' />
		/// <param name='y0' type='Number' />
		/// <param name='x1' type='Number' />
		/// <param name='y1' type='Number' />
		/// <returns type='CanvasGradient'/>
		/// </signature>
		return CanvasGradient; 
	};


	/* -- type: DOML2DeprecatedAlignmentStyle_HTMLTableRowElement -- */

	DOML2DeprecatedAlignmentStyle_HTMLTableRowElement.align = '';


	/* -- type: HTMLTableRowElement -- */
	_implement(HTMLTableRowElement, MSBorderColorHighlightStyle_HTMLTableRowElement);
	_implement(HTMLTableRowElement, MSBorderColorStyle_HTMLTableRowElement);
	_implement(HTMLTableRowElement, HTMLTableAlignment);
	_implement(HTMLTableRowElement, MSHTMLTableRowElementExtensions);
	_implement(HTMLTableRowElement, DOML2DeprecatedBackgroundColorStyle);
	_implement(HTMLTableRowElement, DOML2DeprecatedAlignmentStyle_HTMLTableRowElement);

	HTMLTableRowElement.rowIndex = 0;
	HTMLTableRowElement.cells = HTMLCollection;
	HTMLTableRowElement.sectionRowIndex = 0;
	HTMLTableRowElement.deleteCell = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' optional='true' />
		/// </signature>
	};
	HTMLTableRowElement.insertCell = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' optional='true' />
		/// <returns type='HTMLElement'/>
		/// </signature>
		return HTMLElement; 
	};


	/* -- type: HTMLScriptElement -- */

	HTMLScriptElement.defer = false;
	HTMLScriptElement.htmlFor = '';
	HTMLScriptElement.src = '';
	HTMLScriptElement.text = '';
	HTMLScriptElement.type = '';
	HTMLScriptElement.charset = '';
	HTMLScriptElement.event = '';


	/* -- type: MessageEvent -- */

	MessageEvent.source = Window;
	MessageEvent.origin = '';
	MessageEvent.data = new Object();
	MessageEvent.initMessageEvent = function(typeArg, canBubbleArg, cancelableArg, dataArg, originArg, lastEventIdArg, sourceArg) { 
		/// <signature>
		/// <param name='typeArg' type='String' />
		/// <param name='canBubbleArg' type='Boolean' />
		/// <param name='cancelableArg' type='Boolean' />
		/// <param name='dataArg' type='Object' />
		/// <param name='originArg' type='String' />
		/// <param name='lastEventIdArg' type='String' />
		/// <param name='sourceArg' type='Window' />
		/// </signature>
	};


	/* -- type: SVGDocument -- */

	SVGDocument.rootElement = SVGSVGElement;


	/* -- type: DocumentRange -- */

	DocumentRange.createRange = function() { 
		/// <signature>
		/// <returns type='Range'/>
		/// </signature>
		return Range; 
	};


	/* -- type: Document -- */
	_implement(Document, DocumentStyle);
	_implement(Document, DocumentRange);
	_implement(Document, HTMLDocument);
	_implement(Document, NodeSelector);
	_implement(Document, DocumentEvent);
	_implement(Document, DocumentTraversal);
	_implement(Document, DocumentView);
	_implement(Document, SVGDocument);

	Document.doctype = DocumentType;
	Document.xmlVersion = '';
	Document.implementation = DOMImplementation;
	Document.xmlStandalone = false;
	Document.xmlEncoding = '';
	Document.documentElement = HTMLElement;
	Document.inputEncoding = '';
	Document.adoptNode = function(source) { 
		/// <signature>
		/// <param name='source' type='Node' />
		/// <returns type='Node'/>
		/// </signature>
		return Node; 
	};
	Document.createElement = function(tagName) { 
		/// <signature>
		/// <param name='tagName' type='String' />
		/// <returns type='Element'/>
		/// </signature>
		return _createElementByTagName(tagName);
	};
	Document.createComment = function(data) { 
		/// <signature>
		/// <param name='data' type='String' />
		/// <returns type='Comment'/>
		/// </signature>
		return Comment; 
	};
	Document.getElementsByTagNameNS = function(namespaceURI, localName) { 
		/// <signature>
		/// <param name='namespaceURI' type='String' />
		/// <param name='localName' type='String' />
		/// <returns type='NodeList'/>
		/// </signature>
		return NodeList; 
	};
	Document.getElementsByTagName = function(tagname) { 
		/// <signature>
		/// <param name='tagname' type='String' />
		/// <returns type='NodeList'/>
		/// </signature>
		return _getElementsByTagName(tagname);
	};
	Document.createDocumentFragment = function() { 
		/// <signature>
		/// <returns type='DocumentFragment'/>
		/// </signature>
		return DocumentFragment; 
	};
	Document.createProcessingInstruction = function(target, data) { 
		/// <signature>
		/// <param name='target' type='String' />
		/// <param name='data' type='String' />
		/// <returns type='ProcessingInstruction'/>
		/// </signature>
		return ProcessingInstruction; 
	};
	Document.createElementNS = function(namespaceURI, qualifiedName) { 
		/// <signature>
		/// <param name='namespaceURI' type='String' />
		/// <param name='qualifiedName' type='String' />
		/// <returns type='Element'/>
		/// </signature>
		return HTMLElement; 
	};
	Document.createAttribute = function(name) { 
		/// <signature>
		/// <param name='name' type='String' />
		/// <returns type='Attr'/>
		/// </signature>
		return Attr; 
	};
	Document.createTextNode = function(data) { 
		/// <signature>
		/// <param name='data' type='String' />
		/// <returns type='Text'/>
		/// </signature>
		return Text; 
	};
	Document.importNode = function(importedNode, deep) { 
		/// <signature>
		/// <param name='importedNode' type='Node' />
		/// <param name='deep' type='Boolean' />
		/// <returns type='Node'/>
		/// </signature>
		return Node; 
	};
	Document.createAttributeNS = function(namespaceURI, qualifiedName) { 
		/// <signature>
		/// <param name='namespaceURI' type='String' />
		/// <param name='qualifiedName' type='String' />
		/// <returns type='Attr'/>
		/// </signature>
		return Attr; 
	};
	Document.createCDATASection = function(data) { 
		/// <signature>
		/// <param name='data' type='String' />
		/// <returns type='CDATASection'/>
		/// </signature>
		return CDATASection; 
	};
	Document.getElementById = function(elementId) { 
		/// <signature>
		/// <param name='elementId' type='String' />
		/// <returns type='Element'/>
		/// </signature>
		return _getElementById(elementId);
	};


	/* -- type: KeyboardEvent -- */
	_implement(KeyboardEvent, KeyboardEventExtensions);

	KeyboardEvent.location = 0;
	KeyboardEvent.shiftKey = false;
	KeyboardEvent.locale = '';
	KeyboardEvent.key = '';
	KeyboardEvent.altKey = false;
	KeyboardEvent.metaKey = false;
	KeyboardEvent.char = '';
	KeyboardEvent.ctrlKey = false;
	KeyboardEvent.repeat = false;
	KeyboardEvent.DOM_KEY_LOCATION_RIGHT = 0x02;
	KeyboardEvent.DOM_KEY_LOCATION_LEFT = 0x01;
	KeyboardEvent.DOM_KEY_LOCATION_STANDARD = 0x00;
	KeyboardEvent.DOM_KEY_LOCATION_NUMPAD = 0x03;
	KeyboardEvent.DOM_KEY_LOCATION_JOYSTICK = 0x05;
	KeyboardEvent.DOM_KEY_LOCATION_MOBILE = 0x04;
	KeyboardEvent.getModifierState = function(keyArg) { 
		/// <signature>
		/// <param name='keyArg' type='String' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	KeyboardEvent.initKeyboardEvent = function(typeArg, canBubbleArg, cancelableArg, viewArg, keyArg, locationArg, modifiersListArg, repeat, locale) { 
		/// <signature>
		/// <param name='typeArg' type='String' />
		/// <param name='canBubbleArg' type='Boolean' />
		/// <param name='cancelableArg' type='Boolean' />
		/// <param name='viewArg' type='AbstractView' />
		/// <param name='keyArg' type='String' />
		/// <param name='locationArg' type='Number' />
		/// <param name='modifiersListArg' type='String' />
		/// <param name='repeat' type='Boolean' />
		/// <param name='locale' type='String' />
		/// </signature>
	};


	/* -- type: CanvasGradient -- */

	CanvasGradient.addColorStop = function(offset, color) { 
		/// <signature>
		/// <param name='offset' type='Number' />
		/// <param name='color' type='String' />
		/// </signature>
	};


	/* -- type: HTMLSourceElement -- */

	HTMLSourceElement.media = '';
	HTMLSourceElement.src = '';
	HTMLSourceElement.type = '';


	/* -- type: HTMLButtonElement -- */
	_implement(HTMLButtonElement, MSDataBindingExtensions);
	_implement(HTMLButtonElement, MSHTMLButtonElementExtensions);

	HTMLButtonElement.value = '';
	HTMLButtonElement.name = '';
	HTMLButtonElement.form = HTMLFormElement;
	HTMLButtonElement.type = '';


	/* -- type: SVGAngle -- */

	SVGAngle.valueAsString = '';
	SVGAngle.valueInSpecifiedUnits = 0;
	SVGAngle.value = 0;
	SVGAngle.unitType = 0;
	SVGAngle.SVG_ANGLETYPE_RAD = 3;
	SVGAngle.SVG_ANGLETYPE_UNSPECIFIED = 1;
	SVGAngle.SVG_ANGLETYPE_UNKNOWN = 0;
	SVGAngle.SVG_ANGLETYPE_GRAD = 4;
	SVGAngle.SVG_ANGLETYPE_DEG = 2;
	SVGAngle.newValueSpecifiedUnits = function(unitType, valueInSpecifiedUnits) { 
		/// <signature>
		/// <param name='unitType' type='Number' />
		/// <param name='valueInSpecifiedUnits' type='Number' />
		/// </signature>
	};
	SVGAngle.convertToSpecifiedUnits = function(unitType) { 
		/// <signature>
		/// <param name='unitType' type='Number' />
		/// </signature>
	};


	/* -- type: HTMLAreaElement -- */

	HTMLAreaElement.protocol = '';
	HTMLAreaElement.search = '';
	HTMLAreaElement.alt = '';
	HTMLAreaElement.coords = '';
	HTMLAreaElement.hostname = '';
	HTMLAreaElement.pathname = '';
	HTMLAreaElement.port = '';
	HTMLAreaElement.host = '';
	HTMLAreaElement.hash = '';
	HTMLAreaElement.target = '';
	HTMLAreaElement.noHref = false;
	HTMLAreaElement.href = '';
	HTMLAreaElement.shape = '';
	HTMLAreaElement.toString = function() { 
		/// <signature>
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};


	/* -- type: DOML2DeprecatedAlignmentStyle_HTMLImageElement -- */

	DOML2DeprecatedAlignmentStyle_HTMLImageElement.align = '';


	/* -- type: HTMLImageElement -- */
	_implement(HTMLImageElement, DOML2DeprecatedMarginStyle);
	_implement(HTMLImageElement, DOML2DeprecatedBorderStyle);
	_implement(HTMLImageElement, MSHTMLImageElementExtensions);
	_implement(HTMLImageElement, MSImageResourceExtensions);
	_implement(HTMLImageElement, DOML2DeprecatedAlignmentStyle_HTMLImageElement);
	_implement(HTMLImageElement, MSDataBindingExtensions);
	_implement(HTMLImageElement, MSResourceMetadata);

	HTMLImageElement.width = 0;
	HTMLImageElement.naturalHeight = 0;
	HTMLImageElement.alt = '';
	HTMLImageElement.src = '';
	HTMLImageElement.name = '';
	HTMLImageElement.naturalWidth = 0;
	HTMLImageElement.useMap = '';
	HTMLImageElement.height = 0;
	HTMLImageElement.longDesc = '';
	HTMLImageElement.isMap = false;
	HTMLImageElement.complete = false;


	/* -- type: HTMLCollection -- */
	_implement(HTMLCollection, MSHTMLCollectionExtensions);

	HTMLCollection.length = 0;
	HTMLCollection.item = function(nameOrIndex, optionalIndex) { 
		/// <signature>
		/// <param name='nameOrIndex' type='Object' optional='true' />
		/// <param name='optionalIndex' type='Object' optional='true' />
		/// <returns type='Element'/>
		/// </signature>
		return HTMLElement; 
	};
	HTMLCollection.namedItem = function(name) { 
		/// <signature>
		/// <param name='name' type='String' />
		/// <returns type='Element'/>
		/// </signature>
		return HTMLElement; 
	};
	/* Add a single array element */
	HTMLCollection.push(HTMLElement);


	/* -- type: StyleSheetPageList -- */

	StyleSheetPageList.length = 0;
	StyleSheetPageList.item = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='StyleSheetPage'/>
		/// </signature>
		return StyleSheetPage; 
	};
	/* Add a single array element */
	StyleSheetPageList.push(StyleSheetPage);


	/* -- type: MSCSSStyleRuleExtensions -- */

	MSCSSStyleRuleExtensions.readOnly = false;


	/* -- type: MSSiteModeEvent -- */

	MSSiteModeEvent.buttonID = 0;
	MSSiteModeEvent.actionURL = '';


	/* -- type: SVGAnimatedPreserveAspectRatio -- */

	SVGAnimatedPreserveAspectRatio.animVal = SVGPreserveAspectRatio;
	SVGAnimatedPreserveAspectRatio.baseVal = SVGPreserveAspectRatio;


	/* -- type: WindowSessionStorage -- */

	WindowSessionStorage.sessionStorage = Storage;


	/* -- type: WindowTimers -- */

	WindowTimers.clearTimeout = function(handle) { 
		/// <signature>
		/// <param name='handle' type='Number' />
		/// </signature>
		_clearTimeout(handle);
	};
	WindowTimers.setTimeout = function(expression, msec, language) { 
		/// <signature>
		/// <param name='expression' type='Object' />
		/// <param name='msec' type='Number' optional='true' />
		/// <param name='language' type='Object' optional='true' />
		/// <returns type='Number'/>
		/// </signature>
		return _setTimeout(expression, msec, language);
	};
	WindowTimers.clearInterval = function(handle) { 
		/// <signature>
		/// <param name='handle' type='Number' />
		/// </signature>
		_clearTimeout(handle);
	};
	WindowTimers.setInterval = function(expression, msec, language) { 
		/// <signature>
		/// <param name='expression' type='Object' />
		/// <param name='msec' type='Number' optional='true' />
		/// <param name='language' type='Object' optional='true' />
		/// <returns type='Number'/>
		/// </signature>
		return _setTimeout(expression, msec, language);
	};


	/* -- type: Window -- */
	_implement(Window, ViewCSS);
	_implement(Window, MSEventAttachmentTarget);
	_implement(Window, MSWindowExtensions);
	_implement(Window, WindowPerformance);
	_implement(Window, ScreenView);
	_implement(Window, EventTarget);
	_implement(Window, WindowLocalStorage);
	_implement(Window, WindowSessionStorage);
	_implement(Window, WindowTimers);

	Window.history = History;
	Window.name = '';
	Window.top = Window;
	Window.opener = Window;
	Window.frames = Window;
	Window.length = 0;
	Window.self = Window;
	Window.onerror = ErrorFunction;
	Window.parent = Window;
	Window.location = Location;
	Window.frameElement = HTMLElement;
	Window.window = Window;
	Window.navigator = Navigator;
	Window.alert = function(message) { 
		/// <signature>
		/// <param name='message' type='String' optional='true' />
		/// </signature>
	};
	Window.focus = function() { };
	Window.print = function() { };
	Window.toString = function() { 
		/// <signature>
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	Window.prompt = function(message, defaultValue) { 
		/// <signature>
		/// <param name='message' type='String' optional='true' />
		/// <param name='defaultValue' type='String' optional='true' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	Window.open = function(url, target, features, replace) { 
		/// <signature>
		/// <param name='url' type='String' optional='true' />
		/// <param name='target' type='String' optional='true' />
		/// <param name='features' type='String' optional='true' />
		/// <param name='replace' type='Boolean' optional='true' />
		/// <returns type='Window'/>
		/// </signature>
		return Window; 
	};
	Window.close = function() { };
	Window.confirm = function(message) { 
		/// <signature>
		/// <param name='message' type='String' optional='true' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	Window.postMessage = function(message, targetOrigin) { 
		/// <signature>
		/// <param name='message' type='Object' />
		/// <param name='targetOrigin' type='String' optional='true' />
		/// </signature>
	};
	Window.showModalDialog = function(url, argument, options) { 
		/// <signature>
		/// <param name='url' type='String' optional='true' />
		/// <param name='argument' type='Object' optional='true' />
		/// <param name='options' type='Object' optional='true' />
		/// <returns type='Object'/>
		/// </signature>
		return new Object(); 
	};
	Window.getSelection = function() { 
		/// <signature>
		/// <returns type='Selection'/>
		/// </signature>
		return Selection; 
	};
	Window.blur = function() { };
	_events(Window, "ondragend", "onkeydown", "ondragover", "onkeyup", "onreset", "onmouseup", "ondragstart", "ondrag", "onmouseover", "ondragleave", "onafterprint", "onpause", "onbeforeprint", "onseeked", "onmousedown", "onclick", "onwaiting", "ononline", "ondurationchange", "onblur", "onemptied", "onseeking", "oncanplay", "onstalled", "onmousemove", "onoffline", "onbeforeunload", "onstorage", "onratechange", "onloadstart", "ondragenter", "onsubmit", "onprogress", "ondblclick", "oncontextmenu", "onchange", "onloadedmetadata", "onplay", "onplaying", "oncanplaythrough", "onabort", "onreadystatechange", "onkeypress", "onloadeddata", "onsuspend", "onfocus", "onmessage", "ontimeupdate", "onresize", "onselect", "ondrop", "onmouseout", "onended", "onunload", "onhashchange", "onscroll", "onmousewheel", "onvolumechange", "onload", "oninput");


	/* -- type: SVGAnimatedLengthList -- */

	SVGAnimatedLengthList.animVal = SVGLengthList;
	SVGAnimatedLengthList.baseVal = SVGLengthList;


	/* -- type: SVGPointList -- */

	SVGPointList.numberOfItems = 0;
	SVGPointList.replaceItem = function(newItem, index) { 
		/// <signature>
		/// <param name='newItem' type='SVGPoint' />
		/// <param name='index' type='Number' />
		/// <returns type='SVGPoint'/>
		/// </signature>
		return SVGPoint; 
	};
	SVGPointList.getItem = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='SVGPoint'/>
		/// </signature>
		return SVGPoint; 
	};
	SVGPointList.appendItem = function(newItem) { 
		/// <signature>
		/// <param name='newItem' type='SVGPoint' />
		/// <returns type='SVGPoint'/>
		/// </signature>
		return SVGPoint; 
	};
	SVGPointList.clear = function() { };
	SVGPointList.removeItem = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='SVGPoint'/>
		/// </signature>
		return SVGPoint; 
	};
	SVGPointList.initialize = function(newItem) { 
		/// <signature>
		/// <param name='newItem' type='SVGPoint' />
		/// <returns type='SVGPoint'/>
		/// </signature>
		return SVGPoint; 
	};
	SVGPointList.insertItemBefore = function(newItem, index) { 
		/// <signature>
		/// <param name='newItem' type='SVGPoint' />
		/// <param name='index' type='Number' />
		/// <returns type='SVGPoint'/>
		/// </signature>
		return SVGPoint; 
	};


	/* -- type: MouseWheelEvent -- */

	MouseWheelEvent.wheelDelta = 0;
	MouseWheelEvent.initMouseWheelEvent = function(typeArg, canBubbleArg, cancelableArg, viewArg, detailArg, screenXArg, screenYArg, clientXArg, clientYArg, buttonArg, relatedTargetArg, modifiersListArg, wheelDeltaArg) { 
		/// <signature>
		/// <param name='typeArg' type='String' />
		/// <param name='canBubbleArg' type='Boolean' />
		/// <param name='cancelableArg' type='Boolean' />
		/// <param name='viewArg' type='AbstractView' />
		/// <param name='detailArg' type='Number' />
		/// <param name='screenXArg' type='Number' />
		/// <param name='screenYArg' type='Number' />
		/// <param name='clientXArg' type='Number' />
		/// <param name='clientYArg' type='Number' />
		/// <param name='buttonArg' type='Number' />
		/// <param name='relatedTargetArg' type='EventTarget' />
		/// <param name='modifiersListArg' type='String' />
		/// <param name='wheelDeltaArg' type='Number' />
		/// </signature>
	};


	/* -- type: HTMLMenuElement -- */
	_implement(HTMLMenuElement, DOML2DeprecatedListSpaceReduction);

	HTMLMenuElement.type = '';


	/* -- type: HTMLMapElement -- */

	HTMLMapElement.name = '';
	HTMLMapElement.areas = HTMLAreasCollection;


	/* -- type: HTMLOptionElement -- */
	_implement(HTMLOptionElement, MSDataBindingExtensions);

	HTMLOptionElement.index = 0;
	HTMLOptionElement.text = '';
	HTMLOptionElement.value = '';
	HTMLOptionElement.defaultSelected = false;
	HTMLOptionElement.form = HTMLFormElement;
	HTMLOptionElement.label = '';
	HTMLOptionElement.selected = false;


	/* -- type: MSHTMLTableCaptionElementExtensions -- */

	MSHTMLTableCaptionElementExtensions.vAlign = '';


	/* -- type: HTMLTableCaptionElement -- */
	_implement(HTMLTableCaptionElement, MSHTMLTableCaptionElementExtensions);
	_implement(HTMLTableCaptionElement, DOML2DeprecatedAlignmentStyle_HTMLTableCaptionElement);



	/* -- type: SVGAnimatedTransformList -- */

	SVGAnimatedTransformList.animVal = SVGTransformList;
	SVGAnimatedTransformList.baseVal = SVGTransformList;


	/* -- type: MSNamespaceInfo -- */
	_implement(MSNamespaceInfo, MSEventAttachmentTarget);

	MSNamespaceInfo.urn = '';
	MSNamespaceInfo.name = '';
	MSNamespaceInfo.readyState = '';
	MSNamespaceInfo.doImport = function(implementationUrl) { 
		/// <signature>
		/// <param name='implementationUrl' type='String' />
		/// </signature>
	};
	_events(MSNamespaceInfo, "onreadystatechange");


	/* -- type: ControlRangeCollection -- */

	ControlRangeCollection.length = 0;
	ControlRangeCollection.queryCommandValue = function(cmdID) { 
		/// <signature>
		/// <param name='cmdID' type='String' />
		/// <returns type='Object'/>
		/// </signature>
		return new Object(); 
	};
	ControlRangeCollection.remove = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// </signature>
	};
	ControlRangeCollection.scrollIntoView = function(varargStart) { 
		/// <signature>
		/// <param name='varargStart' type='Object' optional='true' />
		/// </signature>
	};
	ControlRangeCollection.queryCommandIndeterm = function(cmdID) { 
		/// <signature>
		/// <param name='cmdID' type='String' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	ControlRangeCollection.add = function(item) { 
		/// <signature>
		/// <param name='item' type='Element' />
		/// </signature>
	};
	ControlRangeCollection.item = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='Element'/>
		/// </signature>
		return HTMLElement; 
	};
	ControlRangeCollection.execCommand = function(cmdID, showUI, value) { 
		/// <signature>
		/// <param name='cmdID' type='String' />
		/// <param name='showUI' type='Boolean' optional='true' />
		/// <param name='value' type='Object' optional='true' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	ControlRangeCollection.addElement = function(item) { 
		/// <signature>
		/// <param name='item' type='Element' />
		/// </signature>
	};
	ControlRangeCollection.queryCommandState = function(cmdID) { 
		/// <signature>
		/// <param name='cmdID' type='String' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	ControlRangeCollection.queryCommandSupported = function(cmdID) { 
		/// <signature>
		/// <param name='cmdID' type='String' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	ControlRangeCollection.queryCommandEnabled = function(cmdID) { 
		/// <signature>
		/// <param name='cmdID' type='String' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	ControlRangeCollection.select = function() { };
	ControlRangeCollection.queryCommandText = function(cmdID) { 
		/// <signature>
		/// <param name='cmdID' type='String' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	/* Add a single array element */
	ControlRangeCollection.push(HTMLElement);


	/* -- type: SVGTitleElement -- */
	_implement(SVGTitleElement, SVGStylable);
	_implement(SVGTitleElement, SVGLangSpace);



	/* -- type: HTMLFontElement -- */
	_implement(HTMLFontElement, DOML2DeprecatedColorProperty);
	_implement(HTMLFontElement, DOML2DeprecatedSizeProperty);

	HTMLFontElement.face = '';


	/* -- type: MSHTMLTableElementExtensions -- */

	MSHTMLTableElementExtensions.cells = HTMLCollection;
	MSHTMLTableElementExtensions.height = new Object();
	MSHTMLTableElementExtensions.cols = 0;
	MSHTMLTableElementExtensions.moveRow = function(indexFrom, indexTo) { 
		/// <signature>
		/// <param name='indexFrom' type='Number' optional='true' />
		/// <param name='indexTo' type='Number' optional='true' />
		/// <returns type='Object'/>
		/// </signature>
		return new Object(); 
	};


	/* -- type: MSHTMLAppletElementExtensions -- */
	_implement(MSHTMLAppletElementExtensions, DOML2DeprecatedBorderStyle_HTMLObjectElement);

	MSHTMLAppletElementExtensions.codeType = '';
	MSHTMLAppletElementExtensions.standby = '';
	MSHTMLAppletElementExtensions.classid = '';
	MSHTMLAppletElementExtensions.form = HTMLFormElement;
	MSHTMLAppletElementExtensions.useMap = '';
	MSHTMLAppletElementExtensions.data = '';
	MSHTMLAppletElementExtensions.altHtml = '';
	MSHTMLAppletElementExtensions.contentDocument = Document;
	MSHTMLAppletElementExtensions.declare = false;
	MSHTMLAppletElementExtensions.type = '';
	MSHTMLAppletElementExtensions.BaseHref = '';


	/* -- type: HTMLLinkElement -- */
	_implement(HTMLLinkElement, MSLinkStyleExtensions);
	_implement(HTMLLinkElement, LinkStyle);

	HTMLLinkElement.rel = '';
	HTMLLinkElement.target = '';
	HTMLLinkElement.media = '';
	HTMLLinkElement.href = '';
	HTMLLinkElement.rev = '';
	HTMLLinkElement.charset = '';
	HTMLLinkElement.type = '';
	HTMLLinkElement.hreflang = '';


	/* -- type: SVGViewElement -- */
	_implement(SVGViewElement, SVGZoomAndPan);
	_implement(SVGViewElement, SVGFitToViewBox);

	SVGViewElement.viewTarget = SVGStringList;


	/* -- type: NodeIterator -- */

	NodeIterator.whatToShow = 0;
	NodeIterator.filter = NodeFilterCallback;
	NodeIterator.root = Node;
	NodeIterator.expandEntityReferences = false;
	NodeIterator.nextNode = function() { 
		/// <signature>
		/// <returns type='Node'/>
		/// </signature>
		return Node; 
	};
	NodeIterator.detach = function() { };
	NodeIterator.previousNode = function() { 
		/// <signature>
		/// <returns type='Node'/>
		/// </signature>
		return Node; 
	};


	/* -- type: CSSStyleRule -- */
	_implement(CSSStyleRule, MSCSSStyleRuleExtensions);

	CSSStyleRule.selectorText = '';
	CSSStyleRule.style = MSStyleCSSProperties;


	/* -- type: HTMLDDElement -- */
	_implement(HTMLDDElement, DOML2DeprecatedWordWrapSuppression_HTMLDDElement);



	/* -- type: SVGScriptElement -- */
	_implement(SVGScriptElement, SVGURIReference);

	SVGScriptElement.type = '';


	/* -- type: Selection -- */

	Selection.isCollapsed = false;
	Selection.anchorNode = Node;
	Selection.focusNode = Node;
	Selection.focusOffset = 0;
	Selection.anchorOffset = 0;
	Selection.rangeCount = 0;
	Selection.addRange = function(range) { 
		/// <signature>
		/// <param name='range' type='Range' />
		/// </signature>
	};
	Selection.collapseToEnd = function() { };
	Selection.toString = function() { 
		/// <signature>
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	Selection.selectAllChildren = function(parentNode) { 
		/// <signature>
		/// <param name='parentNode' type='Node' />
		/// </signature>
	};
	Selection.getRangeAt = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='Range'/>
		/// </signature>
		return Range; 
	};
	Selection.collapse = function(parentNode, offset) { 
		/// <signature>
		/// <param name='parentNode' type='Node' />
		/// <param name='offset' type='Number' />
		/// </signature>
	};
	Selection.removeAllRanges = function() { };
	Selection.collapseToStart = function() { };
	Selection.deleteFromDocument = function() { };
	Selection.removeRange = function(range) { 
		/// <signature>
		/// <param name='range' type='Range' />
		/// </signature>
	};


	/* -- type: SVGAnimatedAngle -- */

	SVGAnimatedAngle.animVal = SVGAngle;
	SVGAnimatedAngle.baseVal = SVGAngle;


	/* -- type: SVGPatternElement -- */
	_implement(SVGPatternElement, SVGStylable);
	_implement(SVGPatternElement, SVGUnitTypes);
	_implement(SVGPatternElement, SVGLangSpace);
	_implement(SVGPatternElement, SVGFitToViewBox);
	_implement(SVGPatternElement, SVGTests);
	_implement(SVGPatternElement, SVGURIReference);

	SVGPatternElement.width = SVGAnimatedLength;
	SVGPatternElement.y = SVGAnimatedLength;
	SVGPatternElement.patternUnits = SVGAnimatedEnumeration;
	SVGPatternElement.x = SVGAnimatedLength;
	SVGPatternElement.height = SVGAnimatedLength;
	SVGPatternElement.patternTransform = SVGAnimatedTransformList;
	SVGPatternElement.patternContentUnits = SVGAnimatedEnumeration;


	/* -- type: HTMLMetaElement -- */
	_implement(HTMLMetaElement, MSHTMLMetaElementExtensions);

	HTMLMetaElement.httpEquiv = '';
	HTMLMetaElement.content = '';
	HTMLMetaElement.name = '';
	HTMLMetaElement.scheme = '';


	/* -- type: MSSelection -- */

	MSSelection.type = '';
	MSSelection.typeDetail = '';
	MSSelection.createRange = function() { 
		/// <signature>
		/// <returns type='TextRange'/>
		/// </signature>
		return TextRange; 
	};
	MSSelection.clear = function() { };
	MSSelection.empty = function() { };
	MSSelection.createRangeCollection = function() { 
		/// <signature>
		/// <returns type='TextRangeCollection'/>
		/// </signature>
		return TextRangeCollection; 
	};


	/* -- type: MSCSSStyleSheetExtensions -- */

	MSCSSStyleSheetExtensions.owningElement = HTMLElement;
	MSCSSStyleSheetExtensions.imports = StyleSheetList;
	MSCSSStyleSheetExtensions.rules = MSCSSRuleList;
	MSCSSStyleSheetExtensions.isAlternate = false;
	MSCSSStyleSheetExtensions.readOnly = false;
	MSCSSStyleSheetExtensions.isPrefAlternate = false;
	MSCSSStyleSheetExtensions.cssText = '';
	MSCSSStyleSheetExtensions.href = '';
	MSCSSStyleSheetExtensions.id = '';
	MSCSSStyleSheetExtensions.pages = StyleSheetPageList;
	MSCSSStyleSheetExtensions.addPageRule = function(bstrSelector, bstrStyle, lIndex) { 
		/// <signature>
		/// <param name='bstrSelector' type='String' />
		/// <param name='bstrStyle' type='String' />
		/// <param name='lIndex' type='Number' optional='true' />
		/// <returns type='Number'/>
		/// </signature>
		return 0; 
	};
	MSCSSStyleSheetExtensions.addImport = function(bstrURL, lIndex) { 
		/// <signature>
		/// <param name='bstrURL' type='String' />
		/// <param name='lIndex' type='Number' optional='true' />
		/// <returns type='Number'/>
		/// </signature>
		return 0; 
	};
	MSCSSStyleSheetExtensions.removeRule = function(lIndex) { 
		/// <signature>
		/// <param name='lIndex' type='Number' />
		/// </signature>
	};
	MSCSSStyleSheetExtensions.removeImport = function(lIndex) { 
		/// <signature>
		/// <param name='lIndex' type='Number' />
		/// </signature>
	};
	MSCSSStyleSheetExtensions.addRule = function(bstrSelector, bstrStyle, lIndex) { 
		/// <signature>
		/// <param name='bstrSelector' type='String' />
		/// <param name='bstrStyle' type='String' optional='true' />
		/// <param name='lIndex' type='Number' optional='true' />
		/// <returns type='Number'/>
		/// </signature>
		return 0; 
	};


	/* -- type: CSSStyleSheet -- */
	_implement(CSSStyleSheet, MSCSSStyleSheetExtensions);

	CSSStyleSheet.ownerRule = CSSRule;
	CSSStyleSheet.cssRules = CSSRuleList;
	CSSStyleSheet.insertRule = function(rule, index) { 
		/// <signature>
		/// <param name='rule' type='String' />
		/// <param name='index' type='Number' optional='true' />
		/// <returns type='Number'/>
		/// </signature>
		return 0; 
	};
	CSSStyleSheet.deleteRule = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' optional='true' />
		/// </signature>
	};


	/* -- type: DOML2DeprecatedWidthStyle_HTMLBlockElement -- */

	DOML2DeprecatedWidthStyle_HTMLBlockElement.width = 0;


	/* -- type: HTMLBlockElement -- */
	_implement(HTMLBlockElement, DOML2DeprecatedTextFlowControl_HTMLBlockElement);
	_implement(HTMLBlockElement, DOML2DeprecatedWidthStyle_HTMLBlockElement);

	HTMLBlockElement.cite = '';


	/* -- type: TextRange -- */

	TextRange.boundingLeft = 0;
	TextRange.offsetLeft = 0;
	TextRange.htmlText = '';
	TextRange.boundingWidth = 0;
	TextRange.boundingHeight = 0;
	TextRange.boundingTop = 0;
	TextRange.text = '';
	TextRange.offsetTop = 0;
	TextRange.queryCommandValue = function(cmdID) { 
		/// <signature>
		/// <param name='cmdID' type='String' />
		/// <returns type='Object'/>
		/// </signature>
		return new Object(); 
	};
	TextRange.moveToPoint = function(x, y) { 
		/// <signature>
		/// <param name='x' type='Number' />
		/// <param name='y' type='Number' />
		/// </signature>
	};
	TextRange.scrollIntoView = function(fStart) { 
		/// <signature>
		/// <param name='fStart' type='Boolean' optional='true' />
		/// </signature>
	};
	TextRange.queryCommandIndeterm = function(cmdID) { 
		/// <signature>
		/// <param name='cmdID' type='String' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	TextRange.move = function(Unit, Count) { 
		/// <signature>
		/// <param name='Unit' type='String' />
		/// <param name='Count' type='Number' optional='true' />
		/// <returns type='Number'/>
		/// </signature>
		return 0; 
	};
	TextRange.getBookmark = function() { 
		/// <signature>
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	TextRange.findText = function(string, count, flags) { 
		/// <signature>
		/// <param name='string' type='String' />
		/// <param name='count' type='Number' optional='true' />
		/// <param name='flags' type='Number' optional='true' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	TextRange.execCommand = function(cmdID, showUI, value) { 
		/// <signature>
		/// <param name='cmdID' type='String' />
		/// <param name='showUI' type='Boolean' optional='true' />
		/// <param name='value' type='Object' optional='true' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	TextRange.moveToBookmark = function(Bookmark) { 
		/// <signature>
		/// <param name='Bookmark' type='String' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	TextRange.getBoundingClientRect = function() { 
		/// <signature>
		/// <returns type='ClientRect'/>
		/// </signature>
		return ClientRect; 
	};
	TextRange.isEqual = function(range) { 
		/// <signature>
		/// <param name='range' type='TextRange' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	TextRange.duplicate = function() { 
		/// <signature>
		/// <returns type='TextRange'/>
		/// </signature>
		return TextRange; 
	};
	TextRange.collapse = function(Start) { 
		/// <signature>
		/// <param name='Start' type='Boolean' optional='true' />
		/// </signature>
	};
	TextRange.select = function() { };
	TextRange.queryCommandText = function(cmdID) { 
		/// <signature>
		/// <param name='cmdID' type='String' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	TextRange.pasteHTML = function(html) { 
		/// <signature>
		/// <param name='html' type='String' />
		/// </signature>
	};
	TextRange.inRange = function(range) { 
		/// <signature>
		/// <param name='range' type='TextRange' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	TextRange.moveEnd = function(Unit, Count) { 
		/// <signature>
		/// <param name='Unit' type='String' />
		/// <param name='Count' type='Number' optional='true' />
		/// <returns type='Number'/>
		/// </signature>
		return 0; 
	};
	TextRange.getClientRects = function() { 
		/// <signature>
		/// <returns type='ClientRectList'/>
		/// </signature>
		return ClientRectList; 
	};
	TextRange.queryCommandState = function(cmdID) { 
		/// <signature>
		/// <param name='cmdID' type='String' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	TextRange.parentElement = function() { 
		/// <signature>
		/// <returns type='Element'/>
		/// </signature>
		return HTMLElement; 
	};
	TextRange.moveStart = function(Unit, Count) { 
		/// <signature>
		/// <param name='Unit' type='String' />
		/// <param name='Count' type='Number' optional='true' />
		/// <returns type='Number'/>
		/// </signature>
		return 0; 
	};
	TextRange.moveToElementText = function(element) { 
		/// <signature>
		/// <param name='element' type='Element' />
		/// </signature>
	};
	TextRange.execCommandShowHelp = function(cmdID) { 
		/// <signature>
		/// <param name='cmdID' type='String' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	TextRange.compareEndPoints = function(how, sourceRange) { 
		/// <signature>
		/// <param name='how' type='String' />
		/// <param name='sourceRange' type='TextRange' />
		/// <returns type='Number'/>
		/// </signature>
		return 0; 
	};
	TextRange.expand = function(Unit) { 
		/// <signature>
		/// <param name='Unit' type='String' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	TextRange.queryCommandSupported = function(cmdID) { 
		/// <signature>
		/// <param name='cmdID' type='String' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	TextRange.setEndPoint = function(how, SourceRange) { 
		/// <signature>
		/// <param name='how' type='String' />
		/// <param name='SourceRange' type='TextRange' />
		/// </signature>
	};
	TextRange.queryCommandEnabled = function(cmdID) { 
		/// <signature>
		/// <param name='cmdID' type='String' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};


	/* -- type: HTMLSelectElement -- */
	_implement(HTMLSelectElement, MSDataBindingExtensions);
	_implement(HTMLSelectElement, MSHTMLCollectionExtensions);

	HTMLSelectElement.options = HTMLSelectElement;
	HTMLSelectElement.value = '';
	HTMLSelectElement.name = '';
	HTMLSelectElement.form = HTMLFormElement;
	HTMLSelectElement.size = 0;
	HTMLSelectElement.length = 0;
	HTMLSelectElement.selectedIndex = 0;
	HTMLSelectElement.multiple = false;
	HTMLSelectElement.type = '';
	HTMLSelectElement.remove = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' optional='true' />
		/// </signature>
	};
	HTMLSelectElement.add = function(element, before) { 
		/// <signature>
		/// <param name='element' type='HTMLElement' />
		/// <param name='before' type='Object' optional='true' />
		/// </signature>
	};
	HTMLSelectElement.item = function(name, index) { 
		/// <signature>
		/// <param name='name' type='Object' optional='true' />
		/// <param name='index' type='Object' optional='true' />
		/// <returns type='Object'/>
		/// </signature>
		return new Object(); 
	};
	HTMLSelectElement.namedItem = function(name) { 
		/// <signature>
		/// <param name='name' type='String' />
		/// <returns type='Object'/>
		/// </signature>
		return new Object(); 
	};


	/* -- type: StyleMedia -- */

	StyleMedia.type = '';
	StyleMedia.matchMedium = function(mediaquery) { 
		/// <signature>
		/// <param name='mediaquery' type='String' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};


	/* -- type: CDATASection -- */



	/* -- type: SVGAnimatedString -- */

	SVGAnimatedString.animVal = '';
	SVGAnimatedString.baseVal = '';


	/* -- type: SVGPathSegLinetoVerticalRel -- */

	SVGPathSegLinetoVerticalRel.y = 0;


	/* -- type: HTMLOListElement -- */
	_implement(HTMLOListElement, DOML2DeprecatedListSpaceReduction);
	_implement(HTMLOListElement, DOML2DeprecatedListNumberingAndBulletStyle);

	HTMLOListElement.start = 0;


	/* -- type: TextMetrics -- */

	TextMetrics.width = 0;


	/* -- type: HTMLAppletElement -- */
	_implement(HTMLAppletElement, MSHTMLAppletElementExtensions);
	_implement(HTMLAppletElement, DOML2DeprecatedMarginStyle_HTMLObjectElement);
	_implement(HTMLAppletElement, DOML2DeprecatedWidthStyle_HTMLAppletElement);
	_implement(HTMLAppletElement, MSDataBindingRecordSetExtensions);
	_implement(HTMLAppletElement, MSDataBindingExtensions);
	_implement(HTMLAppletElement, DOML2DeprecatedAlignmentStyle_HTMLObjectElement);

	HTMLAppletElement.object = '';
	HTMLAppletElement.alt = '';
	HTMLAppletElement.codeBase = '';
	HTMLAppletElement.archive = '';
	HTMLAppletElement.name = '';
	HTMLAppletElement.height = '';
	HTMLAppletElement.code = '';


	/* -- type: RangeException -- */

	RangeException.message = '';
	RangeException.code = 0;
	RangeException.INVALID_NODE_TYPE_ERR = 2;
	RangeException.BAD_BOUNDARYPOINTS_ERR = 1;
	RangeException.toString = function() { 
		/// <signature>
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};


	/* -- type: DOML2DeprecatedAlignmentStyle_HTMLTableElement -- */

	DOML2DeprecatedAlignmentStyle_HTMLTableElement.align = '';


	/* -- type: SVGClipPathElement -- */
	_implement(SVGClipPathElement, SVGStylable);
	_implement(SVGClipPathElement, SVGUnitTypes);
	_implement(SVGClipPathElement, SVGTransformable);
	_implement(SVGClipPathElement, SVGLangSpace);
	_implement(SVGClipPathElement, SVGTests);

	SVGClipPathElement.clipPathUnits = SVGAnimatedEnumeration;


	/* -- type: MSScriptHost -- */



	/* -- type: SVGPathSegCurvetoQuadraticSmoothRel -- */

	SVGPathSegCurvetoQuadraticSmoothRel.y = 0;
	SVGPathSegCurvetoQuadraticSmoothRel.x = 0;


	/* -- type: SVGDescElement -- */
	_implement(SVGDescElement, SVGStylable);
	_implement(SVGDescElement, SVGLangSpace);



	/* -- type: HTMLAreasCollection -- */

	HTMLAreasCollection.remove = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' optional='true' />
		/// </signature>
	};
	HTMLAreasCollection.add = function(element, before) { 
		/// <signature>
		/// <param name='element' type='HTMLElement' />
		/// <param name='before' type='Object' optional='true' />
		/// </signature>
	};


	/* -- type: ErrorFunction -- */

	ErrorFunction.handleWindowError = function(event, source, fileno) { 
		/// <signature>
		/// <param name='event' type='Event' />
		/// <param name='source' type='String' />
		/// <param name='fileno' type='Number' />
		/// </signature>
	};


	/* -- type: DOML2DeprecatedAlignmentStyle_HTMLParagraphElement -- */

	DOML2DeprecatedAlignmentStyle_HTMLParagraphElement.align = '';


	/* -- type: HTMLParagraphElement -- */
	_implement(HTMLParagraphElement, DOML2DeprecatedAlignmentStyle_HTMLParagraphElement);
	_implement(HTMLParagraphElement, MSHTMLParagraphElementExtensions);



	/* -- type: SVGLineElement -- */
	_implement(SVGLineElement, SVGStylable);
	_implement(SVGLineElement, SVGTransformable);
	_implement(SVGLineElement, SVGLangSpace);
	_implement(SVGLineElement, SVGTests);

	SVGLineElement.y1 = SVGAnimatedLength;
	SVGLineElement.x2 = SVGAnimatedLength;
	SVGLineElement.y2 = SVGAnimatedLength;
	SVGLineElement.x1 = SVGAnimatedLength;


	/* -- type: SVGPathSegMovetoRel -- */

	SVGPathSegMovetoRel.y = 0;
	SVGPathSegMovetoRel.x = 0;


	/* -- type: HTMLNextIdElement -- */

	HTMLNextIdElement.n = '';


	/* -- type: DOMImplementation -- */
	_implement(DOMImplementation, DOMHTMLImplementation);

	DOMImplementation.createDocumentType = function(qualifiedName, publicId, systemId) { 
		/// <signature>
		/// <param name='qualifiedName' type='String' />
		/// <param name='publicId' type='String' />
		/// <param name='systemId' type='String' />
		/// <returns type='DocumentType'/>
		/// </signature>
		return DocumentType; 
	};
	DOMImplementation.createDocument = function(namespaceURI, qualifiedName, doctype) { 
		/// <signature>
		/// <param name='namespaceURI' type='String' />
		/// <param name='qualifiedName' type='String' />
		/// <param name='doctype' type='DocumentType' />
		/// <returns type='Document'/>
		/// </signature>
		return Document; 
	};
	DOMImplementation.hasFeature = function(feature, version) { 
		/// <signature>
		/// <param name='feature' type='String' />
		/// <param name='version' type='String' optional='true' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};


	/* -- type: PositionErrorCallback -- */

	PositionErrorCallback.handleEvent = function(error) { 
		/// <signature>
		/// <param name='error' type='PositionError' />
		/// </signature>
	};


	/* -- type: ClientRect -- */

	ClientRect.width = 0;
	ClientRect.left = 0;
	ClientRect.right = 0;
	ClientRect.top = 0;
	ClientRect.height = 0;
	ClientRect.bottom = 0;


	/* -- type: HTMLBaseElement -- */

	HTMLBaseElement.target = '';
	HTMLBaseElement.href = '';


	/* -- type: HTMLTableDataCellElement -- */



	/* -- type: SVGZoomEvent -- */

	SVGZoomEvent.zoomRectScreen = SVGRect;
	SVGZoomEvent.previousScale = 0;
	SVGZoomEvent.newScale = 0;
	SVGZoomEvent.newTranslate = SVGPoint;
	SVGZoomEvent.previousTranslate = SVGPoint;


	/* -- type: SVGPathSegCurvetoCubicSmoothAbs -- */

	SVGPathSegCurvetoCubicSmoothAbs.y = 0;
	SVGPathSegCurvetoCubicSmoothAbs.x2 = 0;
	SVGPathSegCurvetoCubicSmoothAbs.x = 0;
	SVGPathSegCurvetoCubicSmoothAbs.y2 = 0;


	/* -- type: NavigatorID -- */

	NavigatorID.appVersion = '';
	NavigatorID.appName = '';
	NavigatorID.userAgent = '';
	NavigatorID.platform = '';


	/* -- type: Navigator -- */
	_implement(Navigator, NavigatorOnLine);
	_implement(Navigator, NavigatorID);
	_implement(Navigator, NavigatorDoNotTrack);
	_implement(Navigator, NavigatorAbilities);
	_implement(Navigator, NavigatorGeolocation);
	_implement(Navigator, MSNavigatorAbilities);



	/* -- type: MSStyleCSSProperties -- */

	MSStyleCSSProperties.posHeight = 0;
	MSStyleCSSProperties.pixelWidth = 0;
	MSStyleCSSProperties.textDecorationNone = false;
	MSStyleCSSProperties.pixelBottom = 0;
	MSStyleCSSProperties.pixelLeft = 0;
	MSStyleCSSProperties.textDecorationOverline = false;
	MSStyleCSSProperties.posWidth = 0;
	MSStyleCSSProperties.textDecorationLineThrough = false;
	MSStyleCSSProperties.pixelHeight = 0;
	MSStyleCSSProperties.textDecorationBlink = false;
	MSStyleCSSProperties.textDecorationUnderline = false;
	MSStyleCSSProperties.pixelRight = 0;
	MSStyleCSSProperties.posLeft = 0;
	MSStyleCSSProperties.pixelTop = 0;
	MSStyleCSSProperties.posTop = 0;
	MSStyleCSSProperties.posBottom = 0;
	MSStyleCSSProperties.posRight = 0;


	/* -- type: SVGGElement -- */
	_implement(SVGGElement, SVGStylable);
	_implement(SVGGElement, SVGTransformable);
	_implement(SVGGElement, SVGLangSpace);
	_implement(SVGGElement, SVGTests);



	/* -- type: SVGMarkerElement -- */
	_implement(SVGMarkerElement, SVGStylable);
	_implement(SVGMarkerElement, SVGLangSpace);
	_implement(SVGMarkerElement, SVGFitToViewBox);

	SVGMarkerElement.orientType = SVGAnimatedEnumeration;
	SVGMarkerElement.markerUnits = SVGAnimatedEnumeration;
	SVGMarkerElement.orientAngle = SVGAnimatedAngle;
	SVGMarkerElement.markerHeight = SVGAnimatedLength;
	SVGMarkerElement.markerWidth = SVGAnimatedLength;
	SVGMarkerElement.refY = SVGAnimatedLength;
	SVGMarkerElement.refX = SVGAnimatedLength;
	SVGMarkerElement.SVG_MARKER_ORIENT_ANGLE = 2;
	SVGMarkerElement.SVG_MARKER_ORIENT_UNKNOWN = 0;
	SVGMarkerElement.SVG_MARKERUNITS_STROKEWIDTH = 2;
	SVGMarkerElement.SVG_MARKERUNITS_UNKNOWN = 0;
	SVGMarkerElement.SVG_MARKER_ORIENT_AUTO = 1;
	SVGMarkerElement.SVG_MARKERUNITS_USERSPACEONUSE = 1;
	SVGMarkerElement.setOrientToAngle = function(angle) { 
		/// <signature>
		/// <param name='angle' type='SVGAngle' />
		/// </signature>
	};
	SVGMarkerElement.setOrientToAuto = function() { };


	/* -- type: CompositionEvent -- */

	CompositionEvent.locale = '';
	CompositionEvent.data = '';
	CompositionEvent.initCompositionEvent = function(typeArg, canBubbleArg, cancelableArg, viewArg, dataArg, locale) { 
		/// <signature>
		/// <param name='typeArg' type='String' />
		/// <param name='canBubbleArg' type='Boolean' />
		/// <param name='cancelableArg' type='Boolean' />
		/// <param name='viewArg' type='AbstractView' />
		/// <param name='dataArg' type='String' />
		/// <param name='locale' type='String' />
		/// </signature>
	};


	/* -- type: MSDataBindingTableExtensions -- */

	MSDataBindingTableExtensions.dataPageSize = 0;
	MSDataBindingTableExtensions.nextPage = function() { };
	MSDataBindingTableExtensions.refresh = function() { };
	MSDataBindingTableExtensions.firstPage = function() { };
	MSDataBindingTableExtensions.previousPage = function() { };
	MSDataBindingTableExtensions.lastPage = function() { };


	/* -- type: Performance -- */

	Performance.navigation = PerformanceNavigation;
	Performance.timing = PerformanceTiming;
	Performance.toJSON = function() { 
		/// <signature>
		/// <returns type='Object'/>
		/// </signature>
		return new Object(); 
	};


	/* -- type: CanvasPixelArray -- */

	CanvasPixelArray.length = 0;


	/* -- type: SVGPathSegCurvetoQuadraticRel -- */

	SVGPathSegCurvetoQuadraticRel.y1 = 0;
	SVGPathSegCurvetoQuadraticRel.y = 0;
	SVGPathSegCurvetoQuadraticRel.x = 0;
	SVGPathSegCurvetoQuadraticRel.x1 = 0;


	/* -- type: TreeWalker -- */

	TreeWalker.whatToShow = 0;
	TreeWalker.filter = NodeFilterCallback;
	TreeWalker.currentNode = Node;
	TreeWalker.root = Node;
	TreeWalker.expandEntityReferences = false;
	TreeWalker.previousSibling = function() { 
		/// <signature>
		/// <returns type='Node'/>
		/// </signature>
		return Node; 
	};
	TreeWalker.nextSibling = function() { 
		/// <signature>
		/// <returns type='Node'/>
		/// </signature>
		return Node; 
	};
	TreeWalker.lastChild = function() { 
		/// <signature>
		/// <returns type='Node'/>
		/// </signature>
		return Node; 
	};
	TreeWalker.nextNode = function() { 
		/// <signature>
		/// <returns type='Node'/>
		/// </signature>
		return Node; 
	};
	TreeWalker.previousNode = function() { 
		/// <signature>
		/// <returns type='Node'/>
		/// </signature>
		return Node; 
	};
	TreeWalker.firstChild = function() { 
		/// <signature>
		/// <returns type='Node'/>
		/// </signature>
		return Node; 
	};
	TreeWalker.parentNode = function() { 
		/// <signature>
		/// <returns type='Node'/>
		/// </signature>
		return Node; 
	};


	/* -- type: HTMLTableElement -- */
	_implement(HTMLTableElement, DOML2DeprecatedBorderStyle_HTMLTableElement);
	_implement(HTMLTableElement, DOML2DeprecatedAlignmentStyle_HTMLTableElement);
	_implement(HTMLTableElement, MSDataBindingExtensions);
	_implement(HTMLTableElement, MSBorderColorStyle);
	_implement(HTMLTableElement, MSHTMLTableElementExtensions);
	_implement(HTMLTableElement, MSBorderColorHighlightStyle);
	_implement(HTMLTableElement, DOML2DeprecatedBackgroundStyle);
	_implement(HTMLTableElement, MSDataBindingTableExtensions);
	_implement(HTMLTableElement, DOML2DeprecatedBackgroundColorStyle);

	HTMLTableElement.width = '';
	HTMLTableElement.tBodies = HTMLCollection;
	HTMLTableElement.tHead = HTMLTableSectionElement;
	HTMLTableElement.cellSpacing = '';
	HTMLTableElement.frame = '';
	HTMLTableElement.tFoot = HTMLTableSectionElement;
	HTMLTableElement.rules = '';
	HTMLTableElement.rows = HTMLCollection;
	HTMLTableElement.cellPadding = '';
	HTMLTableElement.caption = HTMLTableCaptionElement;
	HTMLTableElement.summary = '';
	HTMLTableElement.deleteRow = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' optional='true' />
		/// </signature>
	};
	HTMLTableElement.createTBody = function() { 
		/// <signature>
		/// <returns type='HTMLElement'/>
		/// </signature>
		return HTMLElement; 
	};
	HTMLTableElement.insertRow = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' optional='true' />
		/// <returns type='HTMLElement'/>
		/// </signature>
		return HTMLElement; 
	};
	HTMLTableElement.deleteCaption = function() { };
	HTMLTableElement.deleteTFoot = function() { };
	HTMLTableElement.createTHead = function() { 
		/// <signature>
		/// <returns type='HTMLElement'/>
		/// </signature>
		return HTMLElement; 
	};
	HTMLTableElement.deleteTHead = function() { };
	HTMLTableElement.createCaption = function() { 
		/// <signature>
		/// <returns type='HTMLElement'/>
		/// </signature>
		return HTMLElement; 
	};
	HTMLTableElement.createTFoot = function() { 
		/// <signature>
		/// <returns type='HTMLElement'/>
		/// </signature>
		return HTMLElement; 
	};


	
	// Assign variables to emulate browser host
	Document._$createDomObject = _createDomObject;
	this.window = Window;
	document = Document;
	window.document = Document;
	window.navigator.userAgent = 'Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; WOW64; Trident/5.0; SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; .NET CLR 3.0.30729; .NET4.0C; .NET4.0E; MS-RTC LM 8; InfoPath.3; Override:IE9_DEFAULT_20091014';
	window.location.href = 'about:blank';
	window.location.pathname = '/blank';
	window.location.protocol = 'about:';
	window.location.toString = function() { return this.href; }
	Window.XMLHttpRequest = function() { return Object.create(XMLHttpRequest);};
	Document._$ls = _invokeListeners;
})();
