$AdditionalDefinitions = {
	'INTERFACES' => {
		'WorkerUtils' => {
		    'METHOD' => {
		        'importScripts' => {
		        	'BODY' => 'for (var i = 0; i < arguments.length; i++) _$asyncRequests.add({ src: arguments[i] });'
		        },
		        'setTimeout' => {
		        	'BODY' => 'return _$setTimeout(handler, timeout, args);'
		        },
		        'clearTimeout' => {
		        	'BODY' => '_$clearTimeout(handle);'
		        },
		        'setInterval' => {
		        	'BODY' => 'return _$setTimeout(handler, timeout, args);'
		        },
		        'clearInterval' => {
		        	'BODY' => '_$clearTimeout(handle);'
		        },
		        'setImmediate' => {
		        	'BODY' => 'return _$setTimeout(handler, 0, args);'
		        },
		        'clearImmediate' => {
		        	'BODY' => '_$clearTimeout(handle);'
		        }
		    }
		}
	}
};