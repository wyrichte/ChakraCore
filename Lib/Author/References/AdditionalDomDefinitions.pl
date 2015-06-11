$AdditionalDefinitions = {
    'INTERFACES' => {
        # Define method bodies of select DOM methods to call helpers
        'MSCSSProperties' => {
            'METHOD' => {
                'setAttribute' => {
                    'BODY' => '_setAttribute(this, attributeName, AttributeValue);'
                },
                'getAttribute' => {
                    'BODY' => 'return _getAttribute(this, attributeName);'
                }
            }
        },
        'MSEventObj' => {
            'METHOD' => {
                'setAttribute' => {
                    'BODY' => '_setAttribute(this, strAttributeName, AttributeValue);'
                },
                'getAttribute' => {
                    'BODY' => 'return _getAttribute(this, strAttributeName);'
                }
            }
        },
        'SVGSVGElement' => {
            'METHOD' => {
                'getElementById' => {
                    'BODY' => 'return _getElementById(elementId);'
                }
            }
        },
        'WindowTimers' => {
            'METHOD' => {
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
                }
            }
        },
        'WindowTimersExtension' => {
            'METHOD' => {
                'setImmediate' => {
                    'BODY' => 'return _$setTimeout(expression, null, args);'
                },
                'clearImmediate' => {
                    'BODY' => '_$clearTimeout(handle);'
                },
                'msSetImmediate' => {
                    'BODY' => 'return _$setTimeout(expression, null, args);'
                },
                'msClearImmediate' => {
                    'BODY' => '_$clearTimeout(handle);'
                }
            }
        },
        'Element' => {
            'METHOD' => {
                 'hasAttribute' => {
                     'BODY' => 'return _hasAttribute(this, name);'
                 },
                 'setAttribute' => {
                     'BODY' => '_setAttribute(this, name, value);'
                 },
                 'getAttribute' => {
                     'BODY' => 'return _getAttribute(this, name);'
                 },
                 'getElementsByTagName' => {
                     'BODY' => 'return _getElementsByTagName(this, name);'
                 },
                 'msMatchesSelector' => {
                     'BODY' => 'return true;'
                 }
            }
        },
        'HTMLElement' => {
            'METHOD' => {
                'applyElement' => {
                    'BODY' => 'return _applyElement(this, apply, where);'
                },
                'insertAdjacentHTML' => {
                    'BODY' => '_setInnerHTML(this, html);'
                }
            }
        },
        'MSNodeExtensions' => {
            'METHOD' => {
                'replaceNode' => {
                    'BODY' => 'return _replaceChild(this, replacement, this);'
                }
            }
        },
        'Node' => {
            'METHOD' => {
                'insertBefore' => {
                    'BODY' => 'return _insertBefore(this, newChild, refChild);'
                },
                'replaceChild' => {
                    'BODY' => 'return _replaceChild(this, newChild, oldChild);'
                },
                'appendChild' => {
                    'BODY' => 'return _appendChild(this, newChild);'
                },
                'hasChildNodes' => {
                    'BODY' => 'return _hasChildNodes(this);'
                },
                'removeChild' => {
                    'BODY' => 'return _removeChild(this, oldChild);'
                }
            }
        },
        'NodeSelector' => {
            'METHOD' => {
                'querySelectorAll' => {
                    'BODY' => 'return _querySelectorAll(this, selectors);'
                },
                'querySelector' => {
                    'BODY' => 'return _querySelector(this, selectors);'
                }
            }
        },
        'DocumentEvent' => {
            'METHOD' => {
                'createEvent' => {
                    'BODY' => 'return _createEvent(eventInterface);'
                }
            }
        },
        'Document' => {
            'PROPERTY' => {
                'parentNode' => {
                    'NAME' => 'parentNode',
                    'VALUE' => '_$getTrackingNull(HTMLDocument)'
                },
                'ownerDocument' => {
                    'NAME' => 'ownerDocument',
                    'VALUE' => '_$getTrackingNull(HTMLDocument)'
                }
            },
            'METHOD' => {
                'createElement' => {
                    'BODY' => 'return _createElementByTagName(tagName);'
                },
                'getElementById' => {
                    'BODY' => 'return _getElementById(elementId);'
                },
                'getElementsByTagName' => {
                    'BODY' => 'return _getElementsByTagName(this, tagname);'
                },
                'write' => {
                    'BODY' => '_setInnerHTML(this, content);'
                },
                'writeln' => {
                    'BODY' => '_setInnerHTML(this, content);'
                },
                'msElementsFromPoint' => {
                    'BODY' => 'return _wrapInList([Object.create(HTMLElement)], NodeList);'
                },
                'msElementsFromRect' => {
                    'BODY' => 'return _wrapInList([Object.create(HTMLElement)], NodeList);'
                }
            }
        },
        'MSEventAttachmentTarget' => {
            'METHOD' => {
                'attachEvent' => {
                    'BODY' => '_eventManager.add(this, event, listener, true); return false;'
                }
            }
        },
        'EventTarget' => {
            'METHOD' => {
                'addEventListener' => {
                    'BODY' => '_eventManager.add(this, type, listener);'
                }
            }
        },
        'XMLHttpRequest' => {
            'METHOD' => {
                'send' => {
                    'BODY' => 'this.status = 200; this.readyState = XMLHttpRequest.DONE; this.status = 4; this.statusText = "OK";' 
                }
            }
        },
        'IDBFactory' => {
            'METHOD' => {
                'open' => {
                    'BODY' => 'return _createIDBRequest(IDBOpenDBRequest, null, Object.create(IDBDatabase));'
                },
                'deleteDatabase' => {
                    'BODY' => 'return _createIDBRequest(IDBOpenDBRequest, null, null);'
                }
            }
        },
        'IDBObjectStore' => {
            'METHOD' => {
                'put' => {
                    'BODY' => 'return _createIDBRequest(IDBRequest, this, key);'
                },
                'add' => {
                    'BODY' => 'return _createIDBRequest(IDBRequest, this, key);'
                },
                'delete' => {
                    'BODY' => 'return _createIDBRequest(IDBRequest, this, undefined);'
                },
                'get' => {
                    'BODY' => 'return _createIDBRequest(IDBRequest, this, {});'
                },
                'clear' => {
                    'BODY' => 'return _createIDBRequest(IDBRequest, this, undefined);'
                },
                'openCursor' => {
                    'BODY' => 'var cursor = Object.create(IDBCursorWithValue); cursor.source = this; return _createIDBRequest(IDBRequest, this, cursor);'
                },
                'count' => {
                    'BODY' => 'return _createIDBRequest(IDBRequest, this, 0);'
                }
            }
        },
        'IDBCursor' => {
            'METHOD' => {
                'update' => {
                    'BODY' => 'return _createIDBRequest(IDBRequest, this, value);'
                },
                'delete' => {
                    'BODY' => 'return _createIDBRequest(IDBRequest, this, undefined);'
                }
            }
        },
        'IDBIndex' => {
            'METHOD' => {
                'count' => {
                    'BODY' => 'return _createIDBRequest(IDBRequest, this, 0);'
                },
                'getKey' => {
                    'BODY' => 'return _createIDBRequest(IDBRequest, this.objectStore, {});'
                },
                'openKeyCursor' => {
                    'BODY' => 'var cursor = Object.create(IDBCursor); cursor.source = this; return _createIDBRequest(IDBRequest, this.objectStore, cursor);'
                },
                'get' => {
                    'BODY' => 'return _createIDBRequest(IDBRequest, this.objectStore, {});'
                },
                'openCursor' => {
                    'BODY' => 'var cursor = Object.create(IDBCursorWithValue); cursor.source = this; return _createIDBRequest(IDBRequest, this, cursor);'
                }
            }
        },
        # WebGLContextAttributes is defined in webgl.specidl as a dictionary, which we do not currently support.
        # Add interface definition for WebGLContextAttributes as a workaround.
        'WebGLContextAttributes' => {
            'NAME' => 'WebGLContextAttributes',
            'SUPER' => 'Object',
            'TYPE' => 'interface',
            'PROPERTY' => {
                'alpha' => {
                    'NAME' => 'alpha',
                    'PROPTYPE' => 'boolean'
                },
                'depth' => {
                    'NAME' => 'depth',
                    'PROPTYPE' => 'boolean'
                },
                'stencil' => {
                    'NAME' => 'stencil',
                    'PROPTYPE' => 'boolean'
                },
                'antialias' => {
                    'NAME' => 'antialias',
                    'PROPTYPE' => 'boolean'
                },
                'premultipliedAlpha' => {
                    'NAME' => 'premultipliedAlpha',
                    'PROPTYPE' => 'boolean'
                },
                'preserveDrawingBuffer' => {
                    'NAME' => 'preserveDrawingBuffer',
                    'PROPTYPE' => 'boolean'
                }
            }
        },
        # Add method body for HTMLCanvasElement.getContext to return a WebGLRenderingContext when the context id is 'experimental-webgl'   
        # and a CanvasRenderingContext2D when the context id is '2d'. For any other context id return null.
        'HTMLCanvasElement' => {
            'METHOD' => {
                'getContext' => {
                    'BODY' => 'switch (contextId) { case "2d": return CanvasRenderingContext2D; case "experimental-webgl": return WebGLRenderingContext; default: return null; }'
                }
            }
        },
        # Add CanvasPixelArray since it is not defined in specidl
        'CanvasPixelArray' => {
            'NAME' => 'CanvasPixelArray',
            'SUPER' => 'Object',
            'TYPE' => 'interface',
            'PROPERTY' => {
                'length' => {
                    'NAME' => 'length',
                    'PROPTYPE' => 'long'
                }
            }
        },
        # PositionOptions is defined in ie9.specidl as a dictionary, which we do not currently support.
        # Add interface definition for PositionOptions as a workaround.
        'PositionOptions' => {
            'NAME' => 'PositionOptions',
            'SUPER' => 'Object',
            'TYPE' => 'interface',
            'PROPERTY' => {
                'enableHighAccuracy' => {
                    'NAME' => 'enableHighAccuracy',
                    'PROPTYPE' => 'boolean'
                },
                'timeout' => {
                    'NAME' => 'timeout',
                    'PROPTYPE' => 'long'
                },
                'maximumAge' => {
                    'NAME' => 'maximumAge',
                    'PROPTYPE' => 'long'
                }
            }
        },
        # ObjectURLOptions is defined in ie10.specidl as a dictionary, which we do not currently support.
        # Add interface definition for ObjectURLOptions as a workaround.
        'ObjectURLOptions' => {
            'NAME' => 'ObjectURLOptions',
            'SUPER' => 'Object',
            'TYPE' => 'interface',
            'PROPERTY' => {
                'oneTimeOnly' => {
                    'NAME' => 'oneTimeOnly',
                    'PROPTYPE' => 'boolean'
                }
            }
        },
        # MsZoomToOptions is defined in ie11.specidl as a dictionary, which we do not currently support.
        # Add interface definition for MsZoomToOptions as a workaround.
        'MsZoomToOptions' => {
            'NAME' => 'MsZoomToOptions',
            'SUPER' => 'Object',
            'TYPE' => 'interface',
            'PROPERTY' => {
                'contentX' => {
                    'NAME' => 'contentX',
                    'PROPTYPE' => 'long'
                },
                'contentY' => {
                  'NAME' => 'contentY',
                  'PROPTYPE' => 'long'
                },
                'viewportX' => {
                  'NAME' => 'viewportX',
                  'PROPTYPE' => 'DOMString',
                  'PROPTYPE_NULL' => 1
                },
                'viewportY' => {
                  'NAME' => 'viewportY',
                  'PROPTYPE' => 'DOMString',
                  'PROPTYPE_NULL' => 1
                },
                'scaleFactor' => {
                  'NAME' => 'scaleFactor',
                  'PROPTYPE' => 'float'
                },
                'animate' => {
                  'NAME' => 'animate',
                  'PROPTYPE' => 'DOMString'
                }
            }
        },
        # MutationObserverInit is defined in ie11.specidl as a dictionary, which we do not currently support.
        # Add interface definition for MutationObserverInit as a workaround.
        'MutationObserverInit' => {
            'NAME' => 'MutationObserverInit',
            'SUPER' => 'Object',
            'TYPE' => 'interface',
            'PROPERTY' => {
                'childList' => {
                    'NAME' => 'childList',
                    'PROPTYPE' => 'boolean'
                },
                'attributes' => {
                  'NAME' => 'attributes',
                  'PROPTYPE' => 'boolean'
                },
                'characterData' => {
                  'NAME' => 'characterData',
                  'PROPTYPE' => 'boolean',
                },
                'subtree' => {
                  'NAME' => 'subtree',
                  'PROPTYPE' => 'boolean',
                },
                'attributeOldValue' => {
                  'NAME' => 'attributeOldValue',
                  'PROPTYPE' => 'boolean'
                },
                'characterDataOldValue' => {
                  'NAME' => 'characterDataOldValue',
                  'PROPTYPE' => 'boolean'
                },
                'attributeFilter' => {
                  'NAME' => 'attributeFilter',
                  'PROPTYPE' => 'sequence<DOMString>'
                }
            }
        },
        # AlgorithmParameters is defined in ie11.specidl as a dictionary, which we do not currently support.
        # Add interface definition for AlgorithmParameters as a workaround.
        'AlgorithmParameters' => {
            'NAME' => 'AlgorithmParameters',
            'SUPER' => 'Object',
            'TYPE' => 'interface'
        },
        # Algorithm is defined in ie11.specidl as a dictionary, which we do not currently support.
        # Add interface definition for Algorithm as a workaround.
        'Algorithm' => {
            'NAME' => 'Algorithm',
            'SUPER' => 'Object',
            'TYPE' => 'interface',
            'PROPERTY' => {
                'name' => {
                    'NAME' => 'name',
                    'PROPTYPE' => 'DOMString'
                },
                'params' => {
                  'NAME' => 'params',
                  'PROPTYPE' => 'AlgorithmParameters'
                }
            }
        },
        # DeviceAccelerationDict is defined in ie11.specidl as a dictionary, which we do not currently support.
        # Add interface definition for DeviceAccelerationDict as a workaround.
        'DeviceAccelerationDict' => {
            'NAME' => 'DeviceAccelerationDict',
            'SUPER' => 'Object',
            'TYPE' => 'interface',
            'PROPERTY' => {
                'x' => {
                    'NAME' => 'x',
                    'PROPTYPE' => 'double'
                },
                'y' => {
                    'NAME' => 'y',
                    'PROPTYPE' => 'double'
                },
                'z' => {
                    'NAME' => 'z',
                    'PROPTYPE' => 'double'
                }
            }
        },
        # DeviceRotationRateDict is defined in ie11.specidl as a dictionary, which we do not currently support.
        # Add interface definition for DeviceRotationRateDict as a workaround.
        'DeviceRotationRateDict' => {
            'NAME' => 'DeviceRotationRateDict',
            'SUPER' => 'Object',
            'TYPE' => 'interface',
            'PROPERTY' => {
                'alpha' => {
                    'NAME' => 'alpha',
                    'PROPTYPE' => 'double'
                },
                'beta' => {
                    'NAME' => 'beta',
                    'PROPTYPE' => 'double'
                },
                'gamma' => {
                    'NAME' => 'gamma',
                    'PROPTYPE' => 'double'
                }
            }
        },
        # ExceptionInformation is defined in ie11.specidl as a dictionary, which we do not currently support.
        # Add interface definition for ExceptionInformation as a workaround.
        'ExceptionInformation' => {
            'NAME' => 'ExceptionInformation',
            'SUPER' => 'Object',
            'TYPE' => 'interface',
            'PROPERTY' => {
                'domain' => {
                    'NAME' => 'domain',
                    'PROPTYPE' => 'DOMString'
                }
            }
        },
        # StoreExceptionsInformation is defined in ie11.specidl as a dictionary, which we do not currently support.
        # Add interface definition for StoreExceptionsInformation as a workaround.
        'StoreExceptionsInformation' => {
            'NAME' => 'StoreExceptionsInformation',
            'SUPER' => 'ExceptionInformation',
            'TYPE' => 'interface',
            'PROPERTY' => {
                'siteName' => {
                    'NAME' => 'siteName',
                    'PROPTYPE' => 'DOMString'
                },
                'explanationString' => {
                    'NAME' => 'explanationString',
                    'PROPTYPE' => 'DOMString'
                },
                'detailURI' => {
                    'NAME' => 'detailURI',
                    'PROPTYPE' => 'DOMString'
                }
            }
        },
        # StoreSiteSpecificExceptionsInformation is defined in ie11.specidl as a dictionary, which we do not currently support.
        # Add interface definition for StoreSiteSpecificExceptionsInformation as a workaround.
        'StoreSiteSpecificExceptionsInformation' => {
            'NAME' => 'StoreSiteSpecificExceptionsInformation',
            'SUPER' => 'StoreExceptionsInformation',
            'TYPE' => 'interface',
            'PROPERTY' => {
                'arrayOfDomainStrings' => {
                    'NAME' => 'arrayOfDomainStrings',
                    'PROPTYPE' => 'sequence<DOMString>'
                }
            }
        },
        # ConfirmSiteSpecificExceptionsInformation is defined in ie11.specidl as a dictionary, which we do not currently support.
        # Add interface definition for ConfirmSiteSpecificExceptionsInformation as a workaround.
        'ConfirmSiteSpecificExceptionsInformation' => {
            'NAME' => 'ConfirmSiteSpecificExceptionsInformation',
            'SUPER' => 'ExceptionInformation',
            'TYPE' => 'interface',
            'PROPERTY' => {
                'arrayOfDomainStrings' => {
                    'NAME' => 'arrayOfDomainStrings',
                    'PROPTYPE' => 'sequence<DOMString>'
                }
            }
        },
        # Add overloads for SubtleCrypto members
        'SubtleCrypto' => {
            'METHOD' => {
                'deriveKey' => {
                    'OVERLOADS' => [
                        {
                            'NAME' => 'deriveKey',
                            'RETURN_TYPE' => 'CryptoOperation',
                            'TYPE' => 'method',
                            'PARAMETERS' => [
                                {
                                    'NAME' => 'algorithm',
                                    'PARAMETER_TYPE' => 'DOMString',
                                    'TYPE' => 'parameter',
                                    'DIRECTION' => 'in'
                                },
                                {
                                    'NAME' => 'baseKey',
                                    'PARAMETER_TYPE' => 'Key',
                                    'TYPE' => 'parameter',
                                    'DIRECTION' => 'in'
                                },
                                {
                                    'NAME' => 'derivedKeyType',
                                    'PARAMETER_TYPE' => 'DOMString',
                                    'TYPE' => 'parameter',
                                    'DIRECTION' => 'in'
                                },
                                {
                                    'NAME' => 'extractable',
                                    'PARAMETER_TYPE' => 'bool',
                                    'optional' => 1,
                                    'TYPE' => 'parameter',
                                    'DIRECTION' => 'in'
                                },
                                {
                                    'NAME' => 'keyUsages',
                                    'PARAMETER_TYPE' => 'sequence<DOMString>',
                                    'optional' => 1,
                                    'TYPE' => 'parameter',
                                    'DIRECTION' => 'in'
                                }
                            ]
                        },
                        {
                            'NAME' => 'deriveKey',
                            'RETURN_TYPE' => 'CryptoOperation',
                            'TYPE' => 'method',
                            'PARAMETERS' => [
                                {
                                    'NAME' => 'algorithm',
                                    'PARAMETER_TYPE' => 'Algorithm',
                                    'TYPE' => 'parameter',
                                    'DIRECTION' => 'in'
                                },
                                {
                                    'NAME' => 'baseKey',
                                    'PARAMETER_TYPE' => 'Key',
                                    'TYPE' => 'parameter',
                                    'DIRECTION' => 'in'
                                },
                                {
                                    'NAME' => 'derivedKeyType',
                                    'PARAMETER_TYPE' => 'Algorithm',
                                    'TYPE' => 'parameter',
                                    'DIRECTION' => 'in'
                                },
                                {
                                    'NAME' => 'extractable',
                                    'PARAMETER_TYPE' => 'bool',
                                    'optional' => 1,
                                    'TYPE' => 'parameter',
                                    'DIRECTION' => 'in'
                                },
                                {
                                    'NAME' => 'keyUsages',
                                    'PARAMETER_TYPE' => 'sequence<DOMString>',
                                    'optional' => 1,
                                    'TYPE' => 'parameter',
                                    'DIRECTION' => 'in'
                                }
                            ]
                        }
                    ]
                }
            }
        }
    }
};