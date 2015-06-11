(function () {
    var _eventManager = _$createEventManager(
    function getEventObject(type, attach, obj, ignoreCase) {
        function _eventTypeToObject(type, attach) {
            if(attach) return Event;
			switch (type) {
				case 'close':
					return CloseEvent;
				case 'error':
					return ErrorEvent;
				case 'upgradeneeded':
					return IDBVersionChangeEvent;
				case 'message':
					return MessageEvent;
				case 'loadend':
				case 'progress':
					return ProgressEvent;
			};
            return Event;
        }
        var e = _eventTypeToObject(type, attach);
        var eventObject = Object.create(e);
        eventObject.target = obj;
        eventObject.currentTarget = obj;
        eventObject.type = type;
        if (eventObject.relatedTarget)
            eventObject.relatedTarget = obj;
        return eventObject;
    });
    var _events = _eventManager.createEventProperties;

	var WindowBase64 = {};
	var WorkerUtils = {};
	var Event = {};
	var IDBVersionChangeEvent = _$inherit(Event);
	var EventTarget = {};
	var AbstractWorker = {};
	var Worker = {};
	var WorkerCtor = function() { return Object.create(Worker); };
	var WindowConsole = {};
	var IDBKeyRange = {};
	var Blob = {};
	var BlobCtor = function() { return Object.create(Blob); };
	var MSApp = {};
	var MSBaseReader = {};
	var FileReader = {};
	var FileReaderCtor = function() { return Object.create(FileReader); };
	var ImageData = {};
	var MessagePort = {};
	var IDBRequest = {};
	var MSAppView = {};
	var IDBObjectStore = {};
	var WorkerLocation = {};
	var ProgressEvent = _$inherit(Event);
	var CloseEvent = _$inherit(Event);
	var WebSocket = {};
	var WebSocketCtor = function() { return Object.create(WebSocket); };
	var DOMError = {};
	var MessageEvent = _$inherit(Event);
	var DOMException = {};
	var FileReaderSync = {};
	var FileReaderSyncCtor = function() { return Object.create(FileReaderSync); };
	var MSUnsafeFunctionCallback = {};
	var MessageChannel = {};
	var MessageChannelCtor = function() { return Object.create(MessageChannel); };
	var DedicatedWorkerGlobalScope = {};
	var WorkerGlobalScope = this;
	var IDBOpenDBRequest = _$inherit(IDBRequest);
	var XMLHttpRequestEventTarget = {};
	var DOMStringList = {};
	var IDBDatabase = {};
	var IDBFactory = {};
	var NavigatorOnLine = {};
	var CanvasPixelArray = {};
	var IDBCursor = {};
	var IDBCursorWithValue = _$inherit(IDBCursor);
	var EventException = {};
	var Console = {};
	var EventListener = {};
	var MSBlobBuilder = {};
	var MSBlobBuilderCtor = function() { return Object.create(MSBlobBuilder); };
	var MSStreamReader = {};
	var MSStreamReaderCtor = function() { return Object.create(MSStreamReader); };
	var File = _$inherit(Blob);
	var MSStream = {};
	var ErrorEvent = _$inherit(Event);
	var NavigatorID = {};
	var WorkerNavigator = {};
	var IDBTransaction = {};
	var FileList = {};
	var MSExecAtPriorityFunctionCallback = {};
	var IDBIndex = {};
	var XMLHttpRequest = {};
	var XMLHttpRequestCtor = function() { return Object.create(XMLHttpRequest); };

	/* -- type: WindowBase64 -- */

	WindowBase64.btoa = function(rawString) { 
		/// <signature>
		/// <param name='rawString' type='String' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	WindowBase64.atob = function(encodedString) { 
		/// <signature>
		/// <param name='encodedString' type='String' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};


	/* -- type: WorkerUtils -- */
	_$implement(WorkerUtils, WindowBase64);

	WorkerUtils.navigator = WorkerNavigator;
	WorkerUtils.msIndexedDB = IDBFactory;
	WorkerUtils.indexedDB = IDBFactory;
	WorkerUtils.clearImmediate = function(handle) { 
		/// <signature>
		/// <param name='handle' type='Number' />
		/// </signature>
		_$clearTimeout(handle);
	};
	WorkerUtils.importScripts = function(urls) { 
		/// <signature>
		/// <param name='urls' type='String' />
		/// </signature>
		for (var i = 0; i < arguments.length; i++) _$asyncRequests.add({ src: arguments[i] });
	};
	WorkerUtils.clearTimeout = function(handle) { 
		/// <signature>
		/// <param name='handle' type='Number' />
		/// </signature>
		_$clearTimeout(handle);
	};
	WorkerUtils.setImmediate = function(handler, args) { 
		/// <signature>
		/// <param name='handler' type='Object' />
		/// <param name='args' type='Object' optional='true' />
		/// <returns type='Number'/>
		/// </signature>
		return _$setTimeout(handler, 0, args);
	};
	WorkerUtils.setTimeout = function(handler, timeout, args) { 
		/// <signature>
		/// <param name='handler' type='Object' />
		/// <param name='timeout' type='Object' optional='true' />
		/// <param name='args' type='Object' />
		/// <returns type='Number'/>
		/// </signature>
		return _$setTimeout(handler, timeout, args);
	};
	WorkerUtils.clearInterval = function(handle) { 
		/// <signature>
		/// <param name='handle' type='Number' />
		/// </signature>
		_$clearTimeout(handle);
	};
	WorkerUtils.setInterval = function(handler, timeout, args) { 
		/// <signature>
		/// <param name='handler' type='Object' />
		/// <param name='timeout' type='Object' optional='true' />
		/// <param name='args' type='Object' />
		/// <returns type='Number'/>
		/// </signature>
		return _$setTimeout(handler, timeout, args);
	};


	/* -- type: Event -- */

	Event.timeStamp = 0;
	Event.defaultPrevented = false;
	Event.isTrusted = false;
	Event.currentTarget = EventTarget;
	Event.cancelBubble = false;
	Event.target = EventTarget;
	Event.eventPhase = 0;
	Event.cancelable = false;
	Event.type = '';
	Event.srcElement = {};
	Event.bubbles = false;
	Event.CAPTURING_PHASE = 1;
	Event.AT_TARGET = 2;
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


	/* -- type: IDBVersionChangeEvent -- */

	IDBVersionChangeEvent.newVersion = 0;
	IDBVersionChangeEvent.oldVersion = 0;


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
		_eventManager.add(this, type, listener);
	};
	EventTarget.dispatchEvent = function(evt) { 
		/// <signature>
		/// <param name='evt' type='Event' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};


	/* -- type: AbstractWorker -- */
	_$implement(AbstractWorker, EventTarget);

	_events(AbstractWorker, "onerror");


	/* -- type: Worker -- */
	_$implement(Worker, AbstractWorker);

	Worker.postMessage = function(message, ports) { 
		/// <signature>
		/// <param name='message' type='Object' />
		/// <param name='ports' type='Object' optional='true' />
		/// </signature>
	};
	Worker.terminate = function() { };
	_events(Worker, "onmessage");


	/* -- type: WindowConsole -- */

	WindowConsole.console = Console;


	/* -- type: IDBKeyRange -- */

	IDBKeyRange.upperOpen = false;
	IDBKeyRange.upper = {};
	IDBKeyRange.lowerOpen = false;
	IDBKeyRange.lower = {};
	IDBKeyRange.bound = function(lower, upper, lowerOpen, upperOpen) { 
		/// <signature>
		/// <param name='lower' type='Object' />
		/// <param name='upper' type='Object' />
		/// <param name='lowerOpen' type='Boolean' optional='true' />
		/// <param name='upperOpen' type='Boolean' optional='true' />
		/// <returns type='IDBKeyRange'/>
		/// </signature>
		return IDBKeyRange; 
	};
	IDBKeyRange.only = function(value) { 
		/// <signature>
		/// <param name='value' type='Object' />
		/// <returns type='IDBKeyRange'/>
		/// </signature>
		return IDBKeyRange; 
	};
	IDBKeyRange.upperBound = function(bound, open) { 
		/// <signature>
		/// <param name='bound' type='Object' />
		/// <param name='open' type='Boolean' optional='true' />
		/// <returns type='IDBKeyRange'/>
		/// </signature>
		return IDBKeyRange; 
	};
	IDBKeyRange.lowerBound = function(bound, open) { 
		/// <signature>
		/// <param name='bound' type='Object' />
		/// <param name='open' type='Boolean' optional='true' />
		/// <returns type='IDBKeyRange'/>
		/// </signature>
		return IDBKeyRange; 
	};


	/* -- type: Blob -- */

	Blob.type = '';
	Blob.size = 0;
	Blob.msDetachStream = function() { 
		/// <signature>
		/// <returns type='Object'/>
		/// </signature>
		return {}; 
	};
	Blob.msClose = function() { };
	Blob.slice = function(start, end, contentType) { 
		/// <signature>
		/// <param name='start' type='Number' optional='true' />
		/// <param name='end' type='Number' optional='true' />
		/// <param name='contentType' type='String' optional='true' />
		/// <returns type='Blob'/>
		/// </signature>
		return Blob; 
	};


	/* -- type: MSApp -- */

	MSApp.NORMAL = "normal";
	MSApp.HIGH = "high";
	MSApp.IDLE = "idle";
	MSApp.CURRENT = "current";
	MSApp.createFileFromStorageFile = function(storageFile) { 
		/// <signature>
		/// <param name='storageFile' type='Object' />
		/// <returns type='File'/>
		/// </signature>
		return File; 
	};
	MSApp.createDataPackage = function(object) { 
		/// <signature>
		/// <param name='object' type='Object' />
		/// <returns type='Object'/>
		/// </signature>
		return {}; 
	};
	MSApp.terminateApp = function(exceptionObject) { 
		/// <signature>
		/// <param name='exceptionObject' type='Object' />
		/// </signature>
	};
	MSApp.createStreamFromInputStream = function(type, inputStream) { 
		/// <signature>
		/// <param name='type' type='String' />
		/// <param name='inputStream' type='Object' />
		/// <returns type='MSStream'/>
		/// </signature>
		return MSStream; 
	};
	MSApp.createBlobFromRandomAccessStream = function(type, seeker) { 
		/// <signature>
		/// <param name='type' type='String' />
		/// <param name='seeker' type='Object' />
		/// <returns type='Blob'/>
		/// </signature>
		return Blob; 
	};
	MSApp.addPublicLocalApplicationUri = function(uri) { 
		/// <signature>
		/// <param name='uri' type='String' />
		/// </signature>
	};
	MSApp.execAsyncAtPriority = function(asynchronousCallback, priority, args) { 
		/// <signature>
		/// <param name='asynchronousCallback' type='MSExecAtPriorityFunctionCallback' />
		/// <param name='priority' type='String' />
		/// <param name='args' type='Object' />
		/// </signature>
	};
	MSApp.isTaskScheduledAtPriorityOrHigher = function(priority) { 
		/// <signature>
		/// <param name='priority' type='String' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	MSApp.execUnsafeLocalFunction = function(unsafeFunction) { 
		/// <signature>
		/// <param name='unsafeFunction' type='MSUnsafeFunctionCallback' />
		/// <returns type='Object'/>
		/// </signature>
		return {}; 
	};
	MSApp.getHtmlPrintDocumentSource = function(htmlDoc) { 
		/// <signature>
		/// <param name='htmlDoc' type='Object' />
		/// <returns type='Object'/>
		/// </signature>
		return {}; 
	};
	MSApp.getViewOpener = function() { 
		/// <signature>
		/// <returns type='MSAppView'/>
		/// </signature>
		return MSAppView; 
	};
	MSApp.suppressSubdownloadCredentialPrompts = function(suppress) { 
		/// <signature>
		/// <param name='suppress' type='Boolean' />
		/// </signature>
	};
	MSApp.execAtPriority = function(synchronousCallback, priority, args) { 
		/// <signature>
		/// <param name='synchronousCallback' type='MSExecAtPriorityFunctionCallback' />
		/// <param name='priority' type='String' />
		/// <param name='args' type='Object' />
		/// <returns type='Object'/>
		/// </signature>
		return {}; 
	};
	MSApp.createDataPackageFromSelection = function() { 
		/// <signature>
		/// <returns type='Object'/>
		/// </signature>
		return {}; 
	};
	MSApp.createNewView = function(uri) { 
		/// <signature>
		/// <param name='uri' type='String' />
		/// <returns type='MSAppView'/>
		/// </signature>
		return MSAppView; 
	};
	MSApp.getCurrentPriority = function() { 
		/// <signature>
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};


	/* -- type: MSBaseReader -- */
	_$implement(MSBaseReader, EventTarget);

	MSBaseReader.readyState = 0;
	MSBaseReader.result = {};
	MSBaseReader.LOADING = 1;
	MSBaseReader.DONE = 2;
	MSBaseReader.EMPTY = 0;
	MSBaseReader.abort = function() { };
	_events(MSBaseReader, "onprogress", "onabort", "onloadend", "onerror", "onload", "onloadstart");


	/* -- type: FileReader -- */
	_$implement(FileReader, MSBaseReader);

	FileReader.error = DOMError;
	FileReader.readAsArrayBuffer = function(blob) { 
		/// <signature>
		/// <param name='blob' type='Blob' />
		/// </signature>
	};
	FileReader.readAsDataURL = function(blob) { 
		/// <signature>
		/// <param name='blob' type='Blob' />
		/// </signature>
	};
	FileReader.readAsText = function(blob, encoding) { 
		/// <signature>
		/// <param name='blob' type='Blob' />
		/// <param name='encoding' type='String' optional='true' />
		/// </signature>
	};


	/* -- type: ImageData -- */

	ImageData.width = 0;
	ImageData.data = CanvasPixelArray;
	ImageData.height = 0;


	/* -- type: MessagePort -- */
	_$implement(MessagePort, EventTarget);

	MessagePort.close = function() { };
	MessagePort.postMessage = function(message, ports) { 
		/// <signature>
		/// <param name='message' type='Object' optional='true' />
		/// <param name='ports' type='Object' optional='true' />
		/// </signature>
	};
	MessagePort.start = function() { };
	_events(MessagePort, "onmessage");


	/* -- type: IDBRequest -- */
	_$implement(IDBRequest, EventTarget);

	IDBRequest.source = {};
	IDBRequest.transaction = IDBTransaction;
	IDBRequest.error = DOMError;
	IDBRequest.readyState = '';
	IDBRequest.result = {};
	_events(IDBRequest, "onsuccess", "onerror");


	/* -- type: MSAppView -- */

	MSAppView.viewId = 0;
	MSAppView.close = function() { };
	MSAppView.postMessage = function(message, targetOrigin, ports) { 
		/// <signature>
		/// <param name='message' type='Object' />
		/// <param name='targetOrigin' type='String' />
		/// <param name='ports' type='Object' optional='true' />
		/// </signature>
	};


	/* -- type: IDBObjectStore -- */

	IDBObjectStore.indexNames = DOMStringList;
	IDBObjectStore.transaction = IDBTransaction;
	IDBObjectStore.name = '';
	IDBObjectStore.keyPath = '';
	IDBObjectStore.count = function(key) { 
		/// <signature>
		/// <param name='key' type='Object' optional='true' />
		/// <returns type='IDBRequest'/>
		/// </signature>
		return _createIDBRequest(IDBRequest, this, 0);
	};
	IDBObjectStore.add = function(value, key) { 
		/// <signature>
		/// <param name='value' type='Object' />
		/// <param name='key' type='Object' optional='true' />
		/// <returns type='IDBRequest'/>
		/// </signature>
		return _createIDBRequest(IDBRequest, this, key);
	};
	IDBObjectStore.createIndex = function(name, keyPath, optionalParameters) { 
		/// <signature>
		/// <param name='name' type='String' />
		/// <param name='keyPath' type='String' />
		/// <param name='optionalParameters' type='Object' optional='true' />
		/// <returns type='IDBIndex'/>
		/// </signature>
		return IDBIndex; 
	};
	IDBObjectStore.clear = function() { 
		/// <signature>
		/// <returns type='IDBRequest'/>
		/// </signature>
		return _createIDBRequest(IDBRequest, this, undefined);
	};
	IDBObjectStore.put = function(value, key) { 
		/// <signature>
		/// <param name='value' type='Object' />
		/// <param name='key' type='Object' optional='true' />
		/// <returns type='IDBRequest'/>
		/// </signature>
		return _createIDBRequest(IDBRequest, this, key);
	};
	IDBObjectStore.deleteIndex = function(indexName) { 
		/// <signature>
		/// <param name='indexName' type='String' />
		/// </signature>
	};
	IDBObjectStore.openCursor = function(range, direction) { 
		/// <signature>
		/// <param name='range' type='Object' optional='true' />
		/// <param name='direction' type='String' optional='true' />
		/// <returns type='IDBRequest'/>
		/// </signature>
		var cursor = Object.create(IDBCursorWithValue); cursor.source = this; return _createIDBRequest(IDBRequest, this, cursor);
	};
	IDBObjectStore.index = function(name) { 
		/// <signature>
		/// <param name='name' type='String' />
		/// <returns type='IDBIndex'/>
		/// </signature>
		return IDBIndex; 
	};
	IDBObjectStore.delete = function(key) { 
		/// <signature>
		/// <param name='key' type='Object' />
		/// <returns type='IDBRequest'/>
		/// </signature>
		return _createIDBRequest(IDBRequest, this, undefined);
	};
	IDBObjectStore.get = function(key) { 
		/// <signature>
		/// <param name='key' type='Object' />
		/// <returns type='IDBRequest'/>
		/// </signature>
		return _createIDBRequest(IDBRequest, this, {});
	};


	/* -- type: WorkerLocation -- */

	WorkerLocation.protocol = '';
	WorkerLocation.hash = '';
	WorkerLocation.search = '';
	WorkerLocation.href = '';
	WorkerLocation.hostname = '';
	WorkerLocation.pathname = '';
	WorkerLocation.port = '';
	WorkerLocation.host = '';
	WorkerLocation.toString = function() { 
		/// <signature>
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};


	/* -- type: ProgressEvent -- */

	ProgressEvent.loaded = 0;
	ProgressEvent.lengthComputable = false;
	ProgressEvent.total = 0;
	ProgressEvent.initProgressEvent = function(typeArg, canBubbleArg, cancelableArg, lengthComputableArg, loadedArg, totalArg) { 
		/// <signature>
		/// <param name='typeArg' type='String' />
		/// <param name='canBubbleArg' type='Boolean' />
		/// <param name='cancelableArg' type='Boolean' />
		/// <param name='lengthComputableArg' type='Boolean' />
		/// <param name='loadedArg' type='Number' />
		/// <param name='totalArg' type='Number' />
		/// </signature>
	};


	/* -- type: CloseEvent -- */

	CloseEvent.wasClean = false;
	CloseEvent.reason = '';
	CloseEvent.code = 0;
	CloseEvent.initCloseEvent = function(typeArg, canBubbleArg, cancelableArg, wasCleanArg, codeArg, reasonArg) { 
		/// <signature>
		/// <param name='typeArg' type='String' />
		/// <param name='canBubbleArg' type='Boolean' />
		/// <param name='cancelableArg' type='Boolean' />
		/// <param name='wasCleanArg' type='Boolean' />
		/// <param name='codeArg' type='Number' />
		/// <param name='reasonArg' type='String' />
		/// </signature>
	};


	/* -- type: WebSocket -- */
	_$implement(WebSocket, EventTarget);

	WebSocketCtor.CLOSING = 2;
	WebSocketCtor.OPEN = 1;
	WebSocketCtor.CLOSED = 3;
	WebSocketCtor.CONNECTING = 0;
	WebSocket.protocol = '';
	WebSocket.bufferedAmount = 0;
	WebSocket.readyState = 0;
	WebSocket.extensions = '';
	WebSocket.binaryType = '';
	WebSocket.url = '';
	WebSocket.CLOSING = 2;
	WebSocket.OPEN = 1;
	WebSocket.CLOSED = 3;
	WebSocket.CONNECTING = 0;
	WebSocket.close = function(code, reason) { 
		/// <signature>
		/// <param name='code' type='Number' optional='true' />
		/// <param name='reason' type='String' optional='true' />
		/// </signature>
	};
	WebSocket.send = function(data) { 
		/// <signature>
		/// <param name='data' type='Object' />
		/// </signature>
	};
	_events(WebSocket, "onopen", "onmessage", "onclose", "onerror");


	/* -- type: DOMError -- */

	DOMError.name = '';
	DOMError.toString = function() { 
		/// <signature>
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};


	/* -- type: MessageEvent -- */

	MessageEvent.source = {};
	MessageEvent.ports = {};
	MessageEvent.origin = '';
	MessageEvent.data = {};
	MessageEvent.initMessageEvent = function(typeArg, canBubbleArg, cancelableArg, dataArg, originArg, lastEventIdArg, sourceArg) { 
		/// <signature>
		/// <param name='typeArg' type='String' />
		/// <param name='canBubbleArg' type='Boolean' />
		/// <param name='cancelableArg' type='Boolean' />
		/// <param name='dataArg' type='Object' />
		/// <param name='originArg' type='String' />
		/// <param name='lastEventIdArg' type='String' />
		/// <param name='sourceArg' type='Object' />
		/// </signature>
	};


	/* -- type: DOMException -- */

	DOMException.name = '';
	DOMException.code = 0;
	DOMException.message = '';
	DOMException.HIERARCHY_REQUEST_ERR = 3;
	DOMException.NO_MODIFICATION_ALLOWED_ERR = 7;
	DOMException.DATA_CLONE_ERR = 25;
	DOMException.INVALID_MODIFICATION_ERR = 13;
	DOMException.NAMESPACE_ERR = 14;
	DOMException.INVALID_CHARACTER_ERR = 5;
	DOMException.TYPE_MISMATCH_ERR = 17;
	DOMException.ABORT_ERR = 20;
	DOMException.INVALID_STATE_ERR = 11;
	DOMException.SECURITY_ERR = 18;
	DOMException.NETWORK_ERR = 19;
	DOMException.WRONG_DOCUMENT_ERR = 4;
	DOMException.INVALID_NODE_TYPE_ERR = 24;
	DOMException.QUOTA_EXCEEDED_ERR = 22;
	DOMException.INDEX_SIZE_ERR = 1;
	DOMException.SYNTAX_ERR = 12;
	DOMException.DOMSTRING_SIZE_ERR = 2;
	DOMException.SERIALIZE_ERR = 82;
	DOMException.VALIDATION_ERR = 16;
	DOMException.NOT_FOUND_ERR = 8;
	DOMException.URL_MISMATCH_ERR = 21;
	DOMException.PARSE_ERR = 81;
	DOMException.NO_DATA_ALLOWED_ERR = 6;
	DOMException.NOT_SUPPORTED_ERR = 9;
	DOMException.TIMEOUT_ERR = 23;
	DOMException.INVALID_ACCESS_ERR = 15;
	DOMException.INUSE_ATTRIBUTE_ERR = 10;
	DOMException.toString = function() { 
		/// <signature>
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};


	/* -- type: FileReaderSync -- */

	FileReaderSync.readAsArrayBuffer = function(blob) { 
		/// <signature>
		/// <param name='blob' type='Blob' />
		/// <returns type='Object'/>
		/// </signature>
		return {}; 
	};
	FileReaderSync.readAsDataURL = function(blob) { 
		/// <signature>
		/// <param name='blob' type='Blob' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	FileReaderSync.readAsText = function(blob, encoding) { 
		/// <signature>
		/// <param name='blob' type='Blob' />
		/// <param name='encoding' type='String' optional='true' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};


	/* -- type: MSUnsafeFunctionCallback -- */

	MSUnsafeFunctionCallback.callback = function() { 
		/// <signature>
		/// <returns type='Object'/>
		/// </signature>
		return {}; 
	};


	/* -- type: MessageChannel -- */

	MessageChannel.port2 = MessagePort;
	MessageChannel.port1 = MessagePort;


	/* -- type: DedicatedWorkerGlobalScope -- */

	DedicatedWorkerGlobalScope.onmessage = function() {};
	DedicatedWorkerGlobalScope.postMessage = function(data) { 
		/// <signature>
		/// <param name='data' type='Object' />
		/// </signature>
	};


	/* -- type: WorkerGlobalScope -- */
	_$implement(WorkerGlobalScope, EventTarget);
	_$implement(WorkerGlobalScope, WindowConsole);
	_$implement(WorkerGlobalScope, DedicatedWorkerGlobalScope);
	_$implement(WorkerGlobalScope, WorkerUtils);

	WorkerGlobalScope.location = WorkerLocation;
	WorkerGlobalScope.onerror = function() {};
	WorkerGlobalScope.self = _$getTrackingNull(Object.create(WorkerGlobalScope));
	WorkerGlobalScope.msWriteProfilerMark = function(profilerMarkName) { 
		/// <signature>
		/// <param name='profilerMarkName' type='String' />
		/// </signature>
	};
	WorkerGlobalScope.close = function() { };
	WorkerGlobalScope.toString = function() { 
		/// <signature>
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};


	/* -- type: IDBOpenDBRequest -- */

	_events(IDBOpenDBRequest, "onupgradeneeded", "onblocked");


	/* -- type: XMLHttpRequestEventTarget -- */
	_$implement(XMLHttpRequestEventTarget, EventTarget);

	_events(XMLHttpRequestEventTarget, "onload", "onerror", "onprogress", "ontimeout", "onabort", "onloadend", "onloadstart");


	/* -- type: DOMStringList -- */

	DOMStringList.length = 0;
	DOMStringList.contains = function(str) { 
		/// <signature>
		/// <param name='str' type='String' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	DOMStringList.item = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='String'/>
		/// </signature>
		return this[index] || _$getTrackingNull(''); 
	};
	/* Add a single array element */
	DOMStringList[0] = _$getTrackingNull('');


	/* -- type: IDBDatabase -- */
	_$implement(IDBDatabase, EventTarget);

	IDBDatabase.version = '';
	IDBDatabase.objectStoreNames = DOMStringList;
	IDBDatabase.name = '';
	IDBDatabase.close = function() { };
	IDBDatabase.createObjectStore = function(name, optionalParameters) { 
		/// <signature>
		/// <param name='name' type='String' />
		/// <param name='optionalParameters' type='Object' optional='true' />
		/// <returns type='IDBObjectStore'/>
		/// </signature>
		return IDBObjectStore; 
	};
	IDBDatabase.transaction = function(storeNames, mode) { 
		/// <signature>
		/// <param name='storeNames' type='Object' />
		/// <param name='mode' type='String' optional='true' />
		/// <returns type='IDBTransaction'/>
		/// </signature>
		return IDBTransaction; 
	};
	IDBDatabase.deleteObjectStore = function(name) { 
		/// <signature>
		/// <param name='name' type='String' />
		/// </signature>
	};
	_events(IDBDatabase, "onerror", "onabort");


	/* -- type: IDBFactory -- */

	IDBFactory.open = function(name, version) { 
		/// <signature>
		/// <param name='name' type='String' />
		/// <param name='version' type='Number' optional='true' />
		/// <returns type='IDBOpenDBRequest'/>
		/// </signature>
		return _createIDBRequest(IDBOpenDBRequest, null, Object.create(IDBDatabase));
	};
	IDBFactory.cmp = function(first, second) { 
		/// <signature>
		/// <param name='first' type='Object' />
		/// <param name='second' type='Object' />
		/// <returns type='Number'/>
		/// </signature>
		return 0; 
	};
	IDBFactory.deleteDatabase = function(name) { 
		/// <signature>
		/// <param name='name' type='String' />
		/// <returns type='IDBOpenDBRequest'/>
		/// </signature>
		return _createIDBRequest(IDBOpenDBRequest, null, null);
	};


	/* -- type: NavigatorOnLine -- */

	NavigatorOnLine.onLine = false;


	/* -- type: CanvasPixelArray -- */

	CanvasPixelArray.length = 0;


	/* -- type: IDBCursor -- */

	IDBCursor.direction = '';
	IDBCursor.source = {};
	IDBCursor.primaryKey = {};
	IDBCursor.key = {};
	IDBCursor.PREV = "prev";
	IDBCursor.PREV_NO_DUPLICATE = "prevunique";
	IDBCursor.NEXT = "next";
	IDBCursor.NEXT_NO_DUPLICATE = "nextunique";
	IDBCursor.advance = function(count) { 
		/// <signature>
		/// <param name='count' type='Number' />
		/// </signature>
	};
	IDBCursor.delete = function() { 
		/// <signature>
		/// <returns type='IDBRequest'/>
		/// </signature>
		return _createIDBRequest(IDBRequest, this, undefined);
	};
	IDBCursor.continue = function(key) { 
		/// <signature>
		/// <param name='key' type='Object' optional='true' />
		/// </signature>
	};
	IDBCursor.update = function(value) { 
		/// <signature>
		/// <param name='value' type='Object' />
		/// <returns type='IDBRequest'/>
		/// </signature>
		return _createIDBRequest(IDBRequest, this, value);
	};


	/* -- type: IDBCursorWithValue -- */

	IDBCursorWithValue.value = {};


	/* -- type: EventException -- */

	EventException.name = '';
	EventException.code = 0;
	EventException.message = '';
	EventException.DISPATCH_REQUEST_ERR = 1;
	EventException.UNSPECIFIED_EVENT_TYPE_ERR = 0;
	EventException.toString = function() { 
		/// <signature>
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};


	/* -- type: Console -- */

	Console.profile = function(reportName) { 
		/// <signature>
		/// <param name='reportName' type='String' optional='true' />
		/// </signature>
	};
	Console.groupEnd = function() { };
	Console.assert = function(test, message, optionalParams) { 
		/// <signature>
		/// <param name='test' type='Boolean' optional='true' />
		/// <param name='message' type='String' optional='true' />
		/// <param name='optionalParams' type='Object' />
		/// </signature>
	};
	Console.time = function(timerName) { 
		/// <signature>
		/// <param name='timerName' type='String' optional='true' />
		/// </signature>
	};
	Console.timeEnd = function(timerName) { 
		/// <signature>
		/// <param name='timerName' type='String' optional='true' />
		/// </signature>
	};
	Console.clear = function() { };
	Console.dir = function(value, optionalParams) { 
		/// <signature>
		/// <param name='value' type='Object' optional='true' />
		/// <param name='optionalParams' type='Object' />
		/// </signature>
	};
	Console.trace = function() { };
	Console.group = function(groupTitle) { 
		/// <signature>
		/// <param name='groupTitle' type='String' optional='true' />
		/// </signature>
	};
	Console.warn = function(message, optionalParams) { 
		/// <signature>
		/// <param name='message' type='String' optional='true' />
		/// <param name='optionalParams' type='Object' />
		/// </signature>
	};
	Console.error = function(message, optionalParams) { 
		/// <signature>
		/// <param name='message' type='String' optional='true' />
		/// <param name='optionalParams' type='Object' />
		/// </signature>
	};
	Console.debug = function(message, optionalParams) { 
		/// <signature>
		/// <param name='message' type='String' optional='true' />
		/// <param name='optionalParams' type='Object' />
		/// </signature>
	};
	Console.dirxml = function(value) { 
		/// <signature>
		/// <param name='value' type='Object' />
		/// </signature>
	};
	Console.log = function(message, optionalParams) { 
		/// <signature>
		/// <param name='message' type='String' optional='true' />
		/// <param name='optionalParams' type='Object' />
		/// </signature>
	};
	Console.profileEnd = function() { };
	Console.select = function(element) { 
		/// <signature>
		/// <param name='element' type='Object' />
		/// </signature>
	};
	Console.info = function(message, optionalParams) { 
		/// <signature>
		/// <param name='message' type='String' optional='true' />
		/// <param name='optionalParams' type='Object' />
		/// </signature>
	};
	Console.count = function(countTitle) { 
		/// <signature>
		/// <param name='countTitle' type='String' optional='true' />
		/// </signature>
	};
	Console.msIsIndependentlyComposed = function(element) { 
		/// <signature>
		/// <param name='element' type='Object' />
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	Console.groupCollapsed = function(groupTitle) { 
		/// <signature>
		/// <param name='groupTitle' type='String' optional='true' />
		/// </signature>
	};


	/* -- type: EventListener -- */

	EventListener.handleEvent = function(evt) { 
		/// <signature>
		/// <param name='evt' type='Event' />
		/// </signature>
	};


	/* -- type: MSBlobBuilder -- */

	MSBlobBuilder.append = function(data, endings) { 
		/// <signature>
		/// <param name='data' type='Object' />
		/// <param name='endings' type='String' optional='true' />
		/// </signature>
	};
	MSBlobBuilder.getBlob = function(contentType) { 
		/// <signature>
		/// <param name='contentType' type='String' optional='true' />
		/// <returns type='Blob'/>
		/// </signature>
		return Blob; 
	};


	/* -- type: MSStreamReader -- */
	_$implement(MSStreamReader, MSBaseReader);

	MSStreamReader.error = DOMError;
	MSStreamReader.readAsArrayBuffer = function(stream, size) { 
		/// <signature>
		/// <param name='stream' type='MSStream' />
		/// <param name='size' type='Number' optional='true' />
		/// </signature>
	};
	MSStreamReader.readAsBlob = function(stream, size) { 
		/// <signature>
		/// <param name='stream' type='MSStream' />
		/// <param name='size' type='Number' optional='true' />
		/// </signature>
	};
	MSStreamReader.readAsDataURL = function(stream, size) { 
		/// <signature>
		/// <param name='stream' type='MSStream' />
		/// <param name='size' type='Number' optional='true' />
		/// </signature>
	};
	MSStreamReader.readAsText = function(stream, encoding, size) { 
		/// <signature>
		/// <param name='stream' type='MSStream' />
		/// <param name='encoding' type='String' optional='true' />
		/// <param name='size' type='Number' optional='true' />
		/// </signature>
	};


	/* -- type: File -- */

	File.lastModifiedDate = {};
	File.name = '';


	/* -- type: MSStream -- */

	MSStream.type = '';
	MSStream.msDetachStream = function() { 
		/// <signature>
		/// <returns type='Object'/>
		/// </signature>
		return {}; 
	};
	MSStream.msClose = function() { };


	/* -- type: ErrorEvent -- */

	ErrorEvent.colno = 0;
	ErrorEvent.filename = '';
	ErrorEvent.error = {};
	ErrorEvent.lineno = 0;
	ErrorEvent.message = '';
	ErrorEvent.initErrorEvent = function(typeArg, canBubbleArg, cancelableArg, messageArg, filenameArg, linenoArg) { 
		/// <signature>
		/// <param name='typeArg' type='String' />
		/// <param name='canBubbleArg' type='Boolean' />
		/// <param name='cancelableArg' type='Boolean' />
		/// <param name='messageArg' type='String' />
		/// <param name='filenameArg' type='String' />
		/// <param name='linenoArg' type='Number' />
		/// </signature>
	};


	/* -- type: NavigatorID -- */

	NavigatorID.appVersion = '';
	NavigatorID.appName = '';
	NavigatorID.userAgent = '';
	NavigatorID.platform = '';
	NavigatorID.product = '';
	NavigatorID.vendor = '';


	/* -- type: WorkerNavigator -- */
	_$implement(WorkerNavigator, NavigatorOnLine);
	_$implement(WorkerNavigator, NavigatorID);



	/* -- type: IDBTransaction -- */
	_$implement(IDBTransaction, EventTarget);

	IDBTransaction.db = IDBDatabase;
	IDBTransaction.mode = '';
	IDBTransaction.error = DOMError;
	IDBTransaction.READ_ONLY = "readonly";
	IDBTransaction.VERSION_CHANGE = "versionchange";
	IDBTransaction.READ_WRITE = "readwrite";
	IDBTransaction.abort = function() { };
	IDBTransaction.objectStore = function(name) { 
		/// <signature>
		/// <param name='name' type='String' />
		/// <returns type='IDBObjectStore'/>
		/// </signature>
		return IDBObjectStore; 
	};
	_events(IDBTransaction, "oncomplete", "onerror", "onabort");


	/* -- type: FileList -- */

	FileList.length = 0;
	FileList.item = function(index) { 
		/// <signature>
		/// <param name='index' type='Number' />
		/// <returns type='File'/>
		/// </signature>
		return this[index] || _$getTrackingNull(Object.create(File)); 
	};
	/* Add a single array element */
	FileList[0] = _$getTrackingNull(Object.create(File));


	/* -- type: MSExecAtPriorityFunctionCallback -- */

	MSExecAtPriorityFunctionCallback.callback = function(args) { 
		/// <signature>
		/// <param name='args' type='Object' />
		/// <returns type='Object'/>
		/// </signature>
		return {}; 
	};


	/* -- type: IDBIndex -- */

	IDBIndex.unique = false;
	IDBIndex.name = '';
	IDBIndex.keyPath = '';
	IDBIndex.objectStore = IDBObjectStore;
	IDBIndex.count = function(key) { 
		/// <signature>
		/// <param name='key' type='Object' optional='true' />
		/// <returns type='IDBRequest'/>
		/// </signature>
		return _createIDBRequest(IDBRequest, this, 0);
	};
	IDBIndex.getKey = function(key) { 
		/// <signature>
		/// <param name='key' type='Object' />
		/// <returns type='IDBRequest'/>
		/// </signature>
		return _createIDBRequest(IDBRequest, this.objectStore, {});
	};
	IDBIndex.get = function(key) { 
		/// <signature>
		/// <param name='key' type='Object' />
		/// <returns type='IDBRequest'/>
		/// </signature>
		return _createIDBRequest(IDBRequest, this.objectStore, {});
	};
	IDBIndex.openKeyCursor = function(range, direction) { 
		/// <signature>
		/// <param name='range' type='IDBKeyRange' optional='true' />
		/// <param name='direction' type='String' optional='true' />
		/// <returns type='IDBRequest'/>
		/// </signature>
		var cursor = Object.create(IDBCursor); cursor.source = this; return _createIDBRequest(IDBRequest, this.objectStore, cursor);
	};
	IDBIndex.openCursor = function(range, direction) { 
		/// <signature>
		/// <param name='range' type='IDBKeyRange' optional='true' />
		/// <param name='direction' type='String' optional='true' />
		/// <returns type='IDBRequest'/>
		/// </signature>
		var cursor = Object.create(IDBCursorWithValue); cursor.source = this; return _createIDBRequest(IDBRequest, this, cursor);
	};


	/* -- type: XMLHttpRequest -- */
	_$implement(XMLHttpRequest, EventTarget);

	XMLHttpRequestCtor.LOADING = 3;
	XMLHttpRequestCtor.DONE = 4;
	XMLHttpRequestCtor.UNSENT = 0;
	XMLHttpRequestCtor.OPENED = 1;
	XMLHttpRequestCtor.HEADERS_RECEIVED = 2;
	XMLHttpRequest.status = 0;
	XMLHttpRequest.msCaching = '';
	XMLHttpRequest.readyState = 0;
	XMLHttpRequest.responseXML = {};
	XMLHttpRequest.responseType = '';
	XMLHttpRequest.timeout = 0;
	XMLHttpRequest.upload = XMLHttpRequestEventTarget;
	XMLHttpRequest.responseBody = {};
	XMLHttpRequest.response = {};
	XMLHttpRequest.withCredentials = false;
	XMLHttpRequest.responseText = '';
	XMLHttpRequest.statusText = '';
	XMLHttpRequest.LOADING = 3;
	XMLHttpRequest.DONE = 4;
	XMLHttpRequest.UNSENT = 0;
	XMLHttpRequest.OPENED = 1;
	XMLHttpRequest.HEADERS_RECEIVED = 2;
	XMLHttpRequest.create = function() { 
		/// <signature>
		/// <returns type='XMLHttpRequest'/>
		/// </signature>
		return XMLHttpRequest; 
	};
	XMLHttpRequest.msCachingEnabled = function() { 
		/// <signature>
		/// <returns type='Boolean'/>
		/// </signature>
		return false; 
	};
	XMLHttpRequest.send = function(data) { 
		/// <signature>
		/// <param name='data' type='Object' optional='true' />
		/// </signature>
		/// <signature>
		/// <param name='data' type='String' optional='true' />
		/// </signature>
		this.status = 200; this.readyState = XMLHttpRequest.DONE; this.status = 4; this.statusText = "OK";
	};
	XMLHttpRequest.abort = function() { };
	XMLHttpRequest.getAllResponseHeaders = function() { 
		/// <signature>
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
	XMLHttpRequest.getResponseHeader = function(header) { 
		/// <signature>
		/// <param name='header' type='String' />
		/// <returns type='String'/>
		/// </signature>
		return ''; 
	};
	XMLHttpRequest.open = function(method, url, async, user, password) { 
		/// <signature>
		/// <param name='method' type='String' />
		/// <param name='url' type='String' />
		/// <param name='async' type='Boolean' optional='true' />
		/// <param name='user' type='String' optional='true' />
		/// <param name='password' type='String' optional='true' />
		/// </signature>
	};
	XMLHttpRequest.overrideMimeType = function(mime) { 
		/// <signature>
		/// <param name='mime' type='String' />
		/// </signature>
	};
	_events(XMLHttpRequest, "onprogress", "onloadend", "onerror", "ontimeout", "onabort", "onreadystatechange", "onload", "onloadstart");



    function _publicInterface(name, interface, interfacePrototype) {
        _$nonRemovable(interface);
        WorkerGlobalScope[name] = interface;
        WorkerGlobalScope[name].prototype = interfacePrototype;
    }

    function _publicObject(name, obj) {
        _$nonRemovable(obj);
        WorkerGlobalScope[name] = obj;
    }
    
	_publicInterface('IDBIndex', {}, IDBIndex);
	_publicInterface('FileList', {}, FileList);
	_publicInterface('IDBTransaction', {
		'READ_ONLY' : "readonly",
		'VERSION_CHANGE' : "versionchange",
		'READ_WRITE' : "readwrite"
	}, IDBTransaction);
	_publicInterface('WorkerNavigator', {}, WorkerNavigator);
	_publicInterface('IDBCursor', {
		'PREV' : "prev",
		'PREV_NO_DUPLICATE' : "prevunique",
		'NEXT' : "next",
		'NEXT_NO_DUPLICATE' : "nextunique"
	}, IDBCursor);
	_publicInterface('ErrorEvent', {}, ErrorEvent);
	_publicInterface('MSStream', {}, MSStream);
	_publicInterface('File', {}, File);
	_publicInterface('Console', {}, Console);
	_publicInterface('EventException', {
		'DISPATCH_REQUEST_ERR' : 1,
		'UNSPECIFIED_EVENT_TYPE_ERR' : 0
	}, EventException);
	_publicInterface('IDBCursorWithValue', {}, IDBCursorWithValue);
	_publicInterface('CanvasPixelArray', {}, CanvasPixelArray);
	_publicInterface('IDBFactory', {}, IDBFactory);
	_publicInterface('IDBDatabase', {}, IDBDatabase);
	_publicInterface('DOMStringList', {}, DOMStringList);
	_publicInterface('XMLHttpRequestEventTarget', {}, XMLHttpRequestEventTarget);
	_publicInterface('IDBOpenDBRequest', {}, IDBOpenDBRequest);
	_publicInterface('WorkerGlobalScope', {}, WorkerGlobalScope);
	_publicInterface('DOMException', {
		'HIERARCHY_REQUEST_ERR' : 3,
		'NO_MODIFICATION_ALLOWED_ERR' : 7,
		'DATA_CLONE_ERR' : 25,
		'INVALID_MODIFICATION_ERR' : 13,
		'NAMESPACE_ERR' : 14,
		'INVALID_CHARACTER_ERR' : 5,
		'TYPE_MISMATCH_ERR' : 17,
		'ABORT_ERR' : 20,
		'INVALID_STATE_ERR' : 11,
		'SECURITY_ERR' : 18,
		'NETWORK_ERR' : 19,
		'WRONG_DOCUMENT_ERR' : 4,
		'INVALID_NODE_TYPE_ERR' : 24,
		'QUOTA_EXCEEDED_ERR' : 22,
		'INDEX_SIZE_ERR' : 1,
		'SYNTAX_ERR' : 12,
		'DOMSTRING_SIZE_ERR' : 2,
		'SERIALIZE_ERR' : 82,
		'VALIDATION_ERR' : 16,
		'NOT_FOUND_ERR' : 8,
		'URL_MISMATCH_ERR' : 21,
		'PARSE_ERR' : 81,
		'NO_DATA_ALLOWED_ERR' : 6,
		'NOT_SUPPORTED_ERR' : 9,
		'TIMEOUT_ERR' : 23,
		'INVALID_ACCESS_ERR' : 15,
		'INUSE_ATTRIBUTE_ERR' : 10
	}, DOMException);
	_publicInterface('MessageEvent', {}, MessageEvent);
	_publicInterface('DOMError', {}, DOMError);
	_publicInterface('CloseEvent', {}, CloseEvent);
	_publicInterface('ProgressEvent', {}, ProgressEvent);
	_publicInterface('WorkerLocation', {}, WorkerLocation);
	_publicInterface('IDBObjectStore', {}, IDBObjectStore);
	_publicInterface('MSAppView', {}, MSAppView);
	_publicInterface('IDBRequest', {}, IDBRequest);
	_publicInterface('MessagePort', {}, MessagePort);
	_publicInterface('Event', {
		'CAPTURING_PHASE' : 1,
		'AT_TARGET' : 2,
		'BUBBLING_PHASE' : 3
	}, Event);
	_publicInterface('ImageData', {}, ImageData);
	_publicObject('MSApp', MSApp);
	_publicInterface('IDBKeyRange', {
		'bound' : IDBKeyRange ['bound'],
		'only' : IDBKeyRange ['only'],
		'upperBound' : IDBKeyRange ['upperBound'],
		'lowerBound' : IDBKeyRange ['lowerBound']
	}, IDBKeyRange);
	_publicInterface('IDBVersionChangeEvent', {}, IDBVersionChangeEvent);

	_publicInterface('XMLHttpRequest', XMLHttpRequestCtor , XMLHttpRequest);
	_publicInterface('MSStreamReader', MSStreamReaderCtor , MSStreamReader);
	_publicInterface('MSBlobBuilder', MSBlobBuilderCtor , MSBlobBuilder);
	_publicInterface('MessageChannel', MessageChannelCtor , MessageChannel);
	_publicInterface('FileReaderSync', FileReaderSyncCtor , FileReaderSync);
	_publicInterface('WebSocket', WebSocketCtor , WebSocket);
	_publicInterface('FileReader', FileReaderCtor , FileReader);
	_publicInterface('Blob', BlobCtor , Blob);
	_publicInterface('Worker', WorkerCtor , Worker);

    this['XMLHttpRequest'].create = this['XMLHttpRequest'];
})();

function _$getActiveXObject(className, location) {
    if ((/XMLHTTP/i).test(className))
        return new window.XMLHttpRequest();
}
