/* IntelliSense support for the Cordova library and common plugins  */
var Cordova = (function () {
    /**
    * @property {String} platformId Gets the operating system name.
    * @property {String} version Gets Cordova framework version
    */
    function Cordova() {
        this.platformId = "";
        this.version = "";
    }

    /**
    * Invokes native functionality by specifying corresponding service name, action and optional parameters.
    * @param {Function} success A success callback function.
    * @param {Function} fail An error callback function.
    * @param {String} service  The service name to call on the native side (corresponds to a native class).
    * @param {String} action The action name to call on the native side (generally corresponds to the native class method).
    * @param {String[]} [args] An array of arguments to pass into the native environment.
    */
    Cordova.prototype.exec = function (success, fail, service, action, args) {
    };

    /**
    * Defines custom logic as a Cordova module. Other modules can later access it using module name provided.
    * @param {String} moduleName
    * @param {Function(require, exports, module)} factory 
    */
    Cordova.prototype.define = function (moduleName, factory) {
        intellisense.setCallContext(factory, { thisArg: {}, args: [{}, {}, {}] });
    };

    /**
    * Access a Cordova module by name.
    * @param {String} moduleName
    */
    Cordova.prototype.require = function (moduleName) {
        return null;
    };
    Cordova.prototype.__proto__ = null;
    return Cordova;
})();

// cordova/argscheck module
var ArgsCheck = (function () {
    function ArgsCheck() {
        this.enableChecks = false;
    }
    ArgsCheck.prototype.checkArgs = function (argsSpec, functionName, args, callee) {
    };
    ArgsCheck.prototype.getValue = function (value, defaultValue) {
        return null;
    };
    return ArgsCheck;
})();

// cordova/urlutil module
var UrlUtil = (function () {
    function UrlUtil() {
    }
    UrlUtil.prototype.makeAbsolute = function (url) {
        return "";
    };
    return UrlUtil;
})();

// Apache Cordova instance
var cordova = new Cordova();

/* Azure Mobile Services */
var WindowsAzure;
(function (WindowsAzure) {
    WindowsAzure.__proto__ = null;

    var MobileServiceClient = (function () {
        /**
        * Initializes a new instance of the MobileServiceClient class.
        * @param {String} applicationUrl The URL to the Mobile Services application
        * @param {String} applicationKey The application key of the mobile service
        */
        function MobileServiceClient(applicationUrl, applicationKey) {
            this.applicationUrl = applicationUrl;
            this.applicationKey = applicationKey;
            this.currentUser = '';
        }
        /**
        * Logs a user into the mobile service using an access token
        * @param {String} provider The name of the identity provider
        * @param {String} [token] JSON representation of an authentication token
        */
        MobileServiceClient.prototype.login = function (provider, token) {
        };
        /**
        * Logs a user out of the mobile service
        */
        MobileServiceClient.prototype.logout = function () {
        };
        /**
            * Gets a reference to a table and its data operations
            * @param {String} tableName The name of the table
            * @returns {MobileServiceTable} 
            */
        MobileServiceClient.prototype.getTable = function (tableName) {

            return new MobileServiceTable();
        };

        // Intellisense free from JavaScript constructor/prototype properties
        MobileServiceClient.prototype.__proto__ = null;
        delete MobileServiceClient.prototype.constructor;

        return MobileServiceClient;
    })();
    WindowsAzure.MobileServiceClient = MobileServiceClient;

    // Platform.async(func) => Platform.Promise based on code MobileServices.Web-1.0.0.js
    var asyncPromise = (function () {
        function asyncPromise() {
        }
        asyncPromise.prototype.then = function (onSuccess, onError) {
            return asyncPromise;
        };
        asyncPromise.prototype.done = function (onSuccess, onError) {
        };

        // Intellisense free from JavaScript constructor/prototype properties
        asyncPromise.prototype.__proto__ = null;
        delete asyncPromise.prototype.constructor;

        return asyncPromise;
    })();

    // Query object fluent creation based on Microsoft Azure documentation: http://msdn.microsoft.com/en-us/library/windowsazure/jj613353.aspx
    var IQuery = (function () {
        function IQuery() {
        }
        IQuery.prototype.read = function (paramsQS) {
            return asyncPromise;
        };
        IQuery.prototype.orderBy = function () {
            var propName = [];
            for (var _i = 0; _i < (arguments.length - 0) ; _i++) {
                propName[_i] = arguments[_i + 0];
            }
            return IQuery;
        };
        IQuery.prototype.orderByDescending = function () {
            var propName = [];
            for (var _i = 0; _i < (arguments.length - 0) ; _i++) {
                propName[_i] = arguments[_i + 0];
            }
            return IQuery;
        };
        IQuery.prototype.select = function () {
            var propNameSelected = [];
            for (var _i = 0; _i < (arguments.length - 0) ; _i++) {
                propNameSelected[_i] = arguments[_i + 0];
            }
            return IQuery;
        };
        IQuery.prototype.where = function (mapObjFilterCriteria) {
            return IQuery;
        };
        IQuery.prototype.skip = function (n) {
            return IQuery;
        };
        IQuery.prototype.take = function (n) {
            return IQuery;
        };
        IQuery.prototype.includeTotalCount = function () {
            return IQuery;
        };

        // Intellisense free from JavaScript constructor/prototype properties
        IQuery.prototype.__proto__ = null;
        delete IQuery.prototype.constructor;

        return IQuery;
    })();

    var MobileServiceTable = (function () {
        function MobileServiceTable() {
        }

        /*
        * Deletes an object from a given table
        * @param {Object} instance The instance to delete from the table
        */
        MobileServiceTable.prototype.del = function (instance) {

        };

        /**
        * Gets the MobileServiceClient object associated with this table
        */
        MobileServiceTable.prototype.getMobileServiceClient = function () {
        };

        /**
        * Returns the name of the table
        * @returns {String} The name of the table
        */
        MobileServiceTable.prototype.getTableName = function () {
            return "";
        };

        /**
        * Requests that the query also return a total count of all the records that would have been returned without any paging implemented by either the client or the server
        * @returns {Object} A query that can be further composed
        */
        MobileServiceTable.prototype.includeTotalCount = function () {
            return new IQuery();
        };

        /**
        * Inserts data from the supplied JSON object into the table
        * @param {Object} instance The instance to insert into the table
        * @returns {Object} A Promise object that returns a new instance of the inserted object when it completes
        */
        MobileServiceTable.prototype.insert = function (instance) {
            return new asyncPromise();
        };

        /**
        * Requests an existing instance from the table by its ID value
        * @param {Number} id An integer value that is the ID of the instance
        * @returns {Object} A Promise object that returns the requested instance when it completes
        */
        MobileServiceTable.prototype.lookup = function (id) {
            return new asyncPromise();
        };

        /**
        * Sorts a query against the table by the selected columns, in ascending order
        * @param {String} col1 The name of the first column to use for ordering
        * @param {String} col2 The name of the second column to use for ordering
        * @param {String} coln The name of the nth column to use for ordering
        * @returns {Object} A query that can be further composed
        */
        MobileServiceTable.prototype.orderBy = function (col1, col2) {
            return new IQuery();
        };

        /**
        * Sorts a query against the table by the selected columns, in descending order.
        * @param {String} col1 The name of the first column to use for ordering
        * @param {String} col2 The name of the second column to use for ordering
        * @param {String} coln The name of the nth column to use for ordering
        * @returns {Object} A query that can be further composed
        */
        MobileServiceTable.prototype.orderByDescending = function (col1, col2) {
            return new IQuery();
        };

        /**
        * Executes a query against the table
        * @param {Object} query The query to execute. When null or undefined, all rows are returned
        * @returns {Undefined} 
        */
        MobileServiceTable.prototype.read = function (query) {
            return asyncPromise;
        };

        /**
        * Gets the latest property values from the table in the mobile service
        * @param {Object} instance 
        * @returns {Object} A Promise object that returns the requested instance when it completes.
        */
        MobileServiceTable.prototype.refresh = function (instance) {
            return new asyncPromise();
        };

        /**
        * Applies the specific column projection to the query against the table
        * @param {Function} func Function that defines the projection
        * @returns {Object} A query that can be further composed
        */
        MobileServiceTable.prototype.select = function (func) {
            return new IQuery();
        };

        /**
        * Skips the specified number of rows in the query
        * @param {Number} count The number of rows to skip when returning the result
        */
        MobileServiceTable.prototype.skip = function (count) {
        };

        /**
        * Returns the specified number of rows in the query
        * @param {Number} count The number of rows in the query to return
        * @returns {Object} A query that can be further composed
        */
        MobileServiceTable.prototype.take = function (count) {
            return new IQuery();
        };

        /**
        * Updates an object in a given table
        * @param {Object} instance The instance to update in the table, as a JSON object
        * @returns {Object} A Promise object that returns the updated object when it completes.
        */
        MobileServiceTable.prototype.update = function (instance) {
            return new asyncPromise();
        };

        /**
        * Applies a row filtering predicate to the query against the table
        * @param {Object} object JSON object that defines the row filter
        * @returns {Object} A query that can be further composed
        */
        MobileServiceTable.prototype.where = function (object) {
            return new IQuery();
        };

        // Intellisense free from JavaScript constructor/prototype properties
        MobileServiceTable.prototype.__proto__ = null;
        delete MobileServiceTable.prototype.constructor;

        return MobileServiceTable;
    })();
    // WindowsAzure.MobileServiceTable = MobileServiceTable;
})(WindowsAzure || (WindowsAzure = {}));
/* Battery Status */
(function () {
    window.onbatterystatus = Object.create(null);
    window.onbatterycritical = Object.create(null);
    window.onbatterylow = Object.create(null);

    var __addEventListenerOrig = window.addEventListener;
    var __removeEventListenerOrig = window.removeEventListener;

    var _handleListner = function (handlerType, type, listener, useCapture) {
        if (/batterystatus|batterycritical|batterylow/.test(type)) {
            var ev = Object.create(null, {
                level: { value: 0 },
                isPlugged: { value: true }
            });
            if (handlerType == "add") {
                __addEventListenerOrig(type, listener(ev), useCapture);
            } else if (handlerType == "remove") {
                __removeEventListenerOrig(type, listener(ev), useCapture);
            }
        } else {
            if (handlerType == "add") {
                __addEventListenerOrig(type, listener, useCapture);
            } else if (handlerType == "remove") {
                __removeEventListenerOrig(type, listener, useCapture);
            }
        }
    };

    /**
    * @param {String} type 
    * @param {EventListener} listener 
    * @param {Boolean} [useCapture] 
    */
    window.addEventListener = function (type, listener, useCapture) {
        _handleListner("add", type, listener, useCapture);
    };

    /**
    * @param {String} type 
    * @param {EventListener} listener 
    * @param {Boolean} [useCapture] 
    */
    window.removeEventListener = function (type, listener, useCapture) {
        _handleListner("remove", type, listener, useCapture);
    };
})();
/* Camera */
var Camera;
(function (Camera) {
    Camera.__proto__ = null;

    /**
    * This plugin provides an API for taking pictures and for choosing images from the system's image library.
    * @property {number} DATA_URL Return image as base64-encoded string
    * @property {number} FILE_URI Return image file URI
    * @property {number} NATIVE_URI Return image native URI (e.g., assets-library:// on iOS or content:// on Android)
    */
    Camera.DestinationType = {
        __proto__: null,
        DATA_URL: 0,
        FILE_URI: 1,
        NATIVE_URI: 2
    }
    Camera.Direction = {
        __proto__: null,
        BACK: 0,
        FRONT: 1
    }
    Camera.EncodingType = {
        __proto__: null,
        JPEG: 0,
        PNG: 1
    }
    /**
    * @property {number} PICTURE Allow selection of still pictures only. (DEFAULT) Will return format specified via DestinationType
    * @property {number} VIDEO Allow selection of video only, WILL ALWAYS RETURN FILE_URI
    * @property {number} ALLMEDIA Allow selection from all media types
    */
    Camera.MediaType = {
        __proto__: null,
        PICTURE: 0,
        VIDEO: 1,
        ALLMEDIA: 2
    }
    Camera.PictureSourceType = {
        __proto__: null,
        PHOTOLIBRARY: 0,
        CAMERA: 1,
        SAVEDPHOTOALBUM: 2
    }
    Camera.PopoverArrowDirection = {
        __proto__: null,
        ARROW_UP: 1,
        ARROW_DOWN: 2,
        ARROW_LEFT: 4,
        ARROW_RIGHT: 8,
        ARROW_ANY: 15
    }
})(Camera || (Camera = {}));

window.navigator.camera = {
    __proto__: null,

    /**
    * Removes intermediate photos taken by the camera from temporary storage.
    * @param {Function} onSuccess Success callback, that called when cleanup succeeds.
    * @param {Function} onError Error callback, that get an error message.
    */
    cleanup: function (onSuccess, onError) {
        intellisense.setCallContext(onError, { thisArg: {}, args: [''] });
    },

    // TODO: Request on JSLS to support this syntax for Object Literals: * @param {number} cameraOptions.quality ...

    /**
    * Takes a photo using the camera, or retrieves a photo from the device's image gallery.
    * @param {Function} cameraSuccess Success callback, that get the image as a base64-encoded String, or as the URI for the image file.
    * @param {Function} cameraError Error callback, that get an error message.
    * @param {CameraOptions} [cameraOptions] Optional parameters to customize the camera settings.
    * @returns {CameraPopoverHandle} For use on iOS only - A handle to the popover dialog created by calling this function.
    *    
    * @typedef {Object} CameraOptions
    *   @property {number} quality                Quality of the saved image, expressed as a range of 0-100, where 100 is typically full resolution with no loss from file compression. Default is 50.
    *   @property {number} destinationType        Choose the format of the return value. Defined in Camera.DestinationType. Default is FILE_URI.
    *   @property {number} sourceType             Set the source of the picture. Defined in navigator.camera.PictureSourceType. Default is CAMERA.
    *   @property {boolean} allowEdit             Allow simple editing of image before selection.
    *   @property {number} encodingType           Choose the returned image file's encoding. Defined in Camera.EncodingType. Default is JPEG.
    *   @property {number} targetWidth            Width in pixels to scale image. Must be used with targetHeight. Aspect ratio remains constant.
    *   @property {number} mediaTypeSet           The type of media to select from. Only works when PictureSourceType is PHOTOLIBRARY or SAVEDPHOTOALBUM. Defined in Camera.MediaType.
    *   @property {boolean} correctOrientation    Rotate the image to correct for the orientation of the device during capture.
    *   @property {boolean} saveToPhotoAlbum      Save the image to the photo album on the device after capture.
    *   @property {number} cameraDirection        Choose the camera to use (front- or back-facing). Defined in Camera.Direction. Default is BACK.
    *   @property {CameraPopoverOptions} popoverOptions   iOS-only options that specify popover location in iPad. Defined in CameraPopoverOptions.
    */
    getPicture: function (cameraSuccess, cameraError, cameraOptions) {
        intellisense.setCallContext(cameraSuccess, { thisArg: {}, args: [''] });
        intellisense.setCallContext(cameraError, { thisArg: {}, args: [''] });
    }
};

var CameraPopoverHandle = (function () {
    /**
    * A handle to the popover dialog created by navigator.camera.getPicture. Used on iOS only.
    */
    function CameraPopoverHandle() {
    }
    CameraPopoverHandle.prototype.__proto__ = null;
    /**
    * Set the position of the popover.
    * @param {CameraPopoverOptions} popoverOptions The CameraPopoverOptions that specify the new position.
    */
    CameraPopoverHandle.prototype.setPosition = function (popoverOptions) {
    };
    return CameraPopoverHandle;
})();

var CameraPopoverOptions = (function () {
    /**
    * iOS-only parameters that specify the anchor element location and arrow direction of the popover when selecting images from an iPad's library or album.
    * @param {Number} x x pixel coordinate of screen element onto which to anchor the popover.
    * @param {Number} y y pixel coordinate of screen element onto which to anchor the popover.
    * @param {Number} width width, in pixels, of the screen element onto which to anchor the popover.
    * @param {Number} height height, in pixels, of the screen element onto which to anchor the popover.
    * @param {Number} arrowDir Direction the arrow on the popover should point. Defined in Camera.PopoverArrowDirection.
    * @property {Number} x x pixel coordinate of screen element onto which to anchor the popover.
    * @property {Number} y y pixel coordinate of screen element onto which to anchor the popover.
    * @property {Number} width width, in pixels, of the screen element onto which to anchor the popover.
    * @property {Number} height height, in pixels, of the screen element onto which to anchor the popover.
    * @property {Number} arrowDir Direction the arrow on the popover should point. Defined in Camera.PopoverArrowDirection.
    */
    function CameraPopoverOptions(x, y, width, height, arrowDir) {

    }
    CameraPopoverOptions.prototype.__proto__ = null;
    return CameraPopoverOptions;
})();
/* Contacts */
(function (contacts) {
    window.navigator.contacts = {
        __proto__: null,

        /**
        * The navigator.contacts.create method is synchronous, and returns a new Contact object.
        * This method does not retain the Contact object in the device contacts database,
        * for which you need to invoke the Contact.save method.
        * @param {Object} [properties] Object with contact fields
        */
        create: function (properties) {
            return new Contact();
        },

        /**
        * The navigator.contacts.find method executes asynchronously, querying the device contacts database
        * and returning an array of Contact objects. The resulting objects are passed to the onSuccess
        * callback function specified by the onSuccess parameter.
        * @param {String[]} fields The fields parameter specifies the fields to be used as a search qualifier
        * and only those results are passed to the onSuccess callback function. A zero-length fields parameter
        * is invalid and results in ContactError.INVALID_ARGUMENT_ERROR. A contactFields value of "*" returns all contact fields.
        * @param {Function} onSuccess Success callback function invoked with the array of Contact objects returned from the database
        * @param {Function} onError Error callback function, invoked when an error occurs.
        * @param {Object} [optionsSearch options to filter navigator.contacts. Keys Include:
        * filter: The search string used to find navigator.contacts. (DOMString) (Default:  "" )
        * multiple: Determines if the find operation returns multiple navigator.contacts. (Boolean) (Default:  false )
        */
        find: function (fields, onSuccess, onError, options) {
            intellisense.setCallContext(onSuccess, { thisArg: {}, args: [[new Contact()]] });
            intellisense.setCallContext(onError, { thisArg: {}, args: [ContactError] });
        },

        /**
        * The navigator.contacts.pickContact method launches the Contact Picker to select a single contact.
        * The resulting object is passed to the contactSuccess callback function specified by the contactSuccess parameter.
        * 
        * @param {Function} onSuccess Callback function, invoked with the Contact selected by the user.
        * @param {Function} onError Callback function, invoked when an error occurs.
        */
        pickContact: function (onSuccess, onError) {
            intellisense.setCallContext(onSuccess, { thisArg: {}, args: [new Contact()] });
            intellisense.setCallContext(onError, { thisArg: {}, args: [ContactError] });
        }
    };
    /**
    * The Contact object represents a user's contact. Contacts can be created, stored, or removed
    * from the device contacts database. Contacts can also be retrieved (individually or in bulk)
    * from the database by invoking the navigator.contacts.find method.
    * @param {String} [id] A globally unique identifier.
    * @param {String} [displayName] The name of this Contact, suitable for display to end users.
    * @param {ContactName} [name] An object containing all components of a persons name.
    * @param {String} [nickname] A casual name by which to address the contact.
    * @param {ContactField[]} [phoneNumbers] An array of all the contact's phone numbers.
    * @param {ContactField[]} [emails] An array of all the contact's email addresses.
    * @param {ContactAddress[]} [addresses] An array of all the contact's addresses.
    * @param {ContactField[]} [ims] An array of all the contact's IM addresses.
    * @param {ContactOrganization[]} [organizations] An array of all the contact's organizations.
    * @param {Date} [birthday] The birthday of the contact.
    * @param {String} [note] A note about the contact.
    * @param {ContactField[]} [photos] An array of the contact's photos.
    * @param {ContactField[]} [categories] An array of all the user-defined categories associated with the contact.
    * @param {ContactField[]} [urls] An array of web pages associated with the contact.
    * @property {String} id A globally unique identifier.
    * @property {String} displayName The name of this Contact, suitable for display to end users.
    * @property {ContactName} name An object containing all components of a persons name.
    * @property {String} nickname A casual name by which to address the contact.
    * @property {ContactField[]} phoneNumbers An array of all the contact's phone numbers.
    * @property {ContactField[]} emails An array of all the contact's email addresses.
    * @property {ContactAddress[]} addresses An array of all the contact's addresses.
    * @property {ContactField[]} ims An array of all the contact's IM addresses.
    * @property {ContactOrganization[]} organizations An array of all the contact's organizations.
    * @property {Date} birthday The birthday of the contact.
    * @property {String} note A note about the contact.
    * @property {ContactField[]} photos An array of the contact's photos.
    * @property {ContactField[]} categories An array of all the user-defined categories associated with the contact.
    * @property {ContactField[]} urls An array of web pages associated with the contact.
*/
    window.Contact = function (id, displayName, name, nickname, phoneNumbers, emails,
        addresses, ims, organizations, birthday, note, photos, categories, urls) {
    };

    window.Contact.prototype = {
        __proto__: null,

        // Returns a new Contact object that is a deep copy of the calling object, with the id property set to null
        clone: function () {
            return new Contact();
        },

        /**
        * Removes the contact from the device contacts database, otherwise executes an error callback with a ContactError object.
        * @param {Function} onSuccess Success callback function invoked on success operation.
        * @param {Function} onError Error callback function, invoked when an error occurs.
        */
        remove: function (onSuccess, onError) {
            intellisense.setCallContext(onSuccess, { thisArg: {}, args: [] });
            intellisense.setCallContext(onError, { thisArg: {}, args: [new Error()] });
        },

        /**
        * Saves a new contact to the device contacts database, or updates an existing contact if a contact with the same id already exists.
        * @param {Function} onSuccess Success callback function invoked on success operation with che Contact object.
        * @param {Function} onError Error callback function, invoked when an error occurs.
        */
        save: function (onSuccess, onError) {
            intellisense.setCallContext(onSuccess, { thisArg: {}, args: [] });
            intellisense.setCallContext(onError, { thisArg: {}, args: [new Error()] });
        }
    };

    window.ContactError = function (code) {
        this.code = 0;
        this.message = '';
    };
    ContactError.prototype.__proto__ = null;
    ContactError.UNKNOWN_ERROR = 0;
    ContactError.INVALID_ARGUMENT_ERROR = 0;
    ContactError.TIMEOUT_ERROR = 0;
    ContactError.PENDING_OPERATION_ERROR = 0;
    ContactError.IO_ERROR = 0;
    ContactError.NOT_SUPPORTED_ERROR = 0;
    ContactError.PERMISSION_DENIED_ERROR = 0;

    /**
    * @param {String} [formatted] The complete name of the contact.
    * @param {String} [familyName] The contact's family name.
    * @param {String} [givenName] The contact's given name.
    * @param {String} [middleName] The contact's middle name.
    * @param {String} [honorificPrefix] The contact's prefix (example Mr. or Dr.).
    * @param {String} [honorificSuffix] The contact's suffix (example Esq).
    * @property {String} formatted The complete name of the contact.
    * @property {String} familyName The contact's family name.
    * @property {String} givenName The contact's given name.
    * @property {String} middleName The contact's middle name.
    * @property {String} honorificPrefix The contact's prefix (example Mr. or Dr.).
    * @property {String} honorificSuffix The contact's suffix (example Esq).
    */
    window.ContactName = function (formatted, familyName, givenName, middleName, honorificPrefix, honorificSuffix) {
    };

    window.ContactName.prototype = {
        __proto__: null
    };

    /**
    *  The ContactField object is a reusable component that represents contact fields generically.
    *  Each ContactField object contains a value, type, and pref property. A Contact object stores
    *  several properties in ContactField[] arrays, such as phone numbers and email addresses.
    *  
    *  In most instances, there are no pre-determined values for a ContactField object's type attribute.
    *  For example, a phone number can specify type values of home, work, mobile, iPhone,
    *  or any other value that is supported by a particular device platform's contact database.
    *  However, for the Contact photos field, the type field indicates the format of the returned image:
    *  url when the value attribute contains a URL to the photo image, or base64 when the value
    *  contains a base64-encoded image string.
    * 
    * @param {String} [type] A string that indicates what type of field this is, home for example.
    * @param {String} [value] The value of the field, such as a phone number or email address.
    * @param {boolean} [pref] Set to true if this ContactField contains the user's preferred value.
    * @property {String} type A string that indicates what type of field this is, home for example.
    * @property {String} value The value of the field, such as a phone number or email address.
    * @property {boolean} pref Set to true if this ContactField contains the user's preferred value.
    */
    window.ContactField = function (type, value, pref) {

    };

    window.ContactField.prototype = {
        __proto__: null
    };

    /**
    * The ContactAddress object stores the properties of a single address of a contact.
    * A Contact object may include more than one address in a ContactAddress[] array.
    * @param {boolean} [pref] Set to true if this ContactAddress contains the user's preferred value.
    * @param {String} [type] A string indicating what type of field this is, home for example.
    * @param {String} [formatted] The full address formatted for display.
    * @param {String} [streetAddress] The full street address.
    * @param {String} [locality] The city or locality.
    * @param {String} [region] The state or region.
    * @param {String} [postalCode] The zip code or postal code.
    * @param {String} [country] The country name.
    * @property {boolean} pref Set to true if this ContactAddress contains the user's preferred value.
    * @property {String} type A string indicating what type of field this is, home for example.
    * @property {String} formatted The full address formatted for display.
    * @property {String} streetAddress The full street address.
    * @property {String} locality The city or locality.
    * @property {String} region The state or region.
    * @property {String} postalCode The zip code or postal code.
    * @property {String} country The country name.
    */
    window.ContactAddress = function () {

    };
    window.ContactAddress.prototype = {
        __proto__: null
    };

    /**
    * The ContactOrganization object stores a contact's organization properties. A Contact object stores
    * one or more ContactOrganization objects in an array.
    * @param {boolean} [pref] Set to true if this ContactOrganization contains the user's preferred value.
    * @param {String} [type] A string that indicates what type of field this is, home for example.
    * @param {String} [name] The name of the organization.
    * @param {String} [department] The department the contract works for.
    * @param {String} [title] The contact's title at the organization.
    * @property {boolean} pref Set to true if this ContactOrganization contains the user's preferred value.
    * @property {String} type A string that indicates what type of field this is, home for example.
    * @property {String} name The name of the organization.
    * @property {String} department The department the contract works for.
    * @property {String} title The contact's title at the organization.
    */
    window.ContactOrganization = function () {

    };
    window.ContactOrganization.prototype = {
        __proto__: null
    };

    /**
    * Search options to filter navigator.contacts.
    * @param {String} [filter] The search string used to find navigator.contacts.
    * @param {boolean} [multiple] Determines if the find operation returns multiple navigator.contacts.
    * @param {String[]} [desiredFields] Contact fields to be returned back. If specified, the resulting Contact object only features values for these fields.
    * @property {String} filter The search string used to find navigator.contacts.
    * @property {boolean} multiple Determines if the find operation returns multiple navigator.contacts.
    * @property {String[]} desiredFields Contact fields to be returned back. If specified, the resulting Contact object only features values for these fields.
    */
    window.ContactFindOptions = function (filter, multiple, desiredFields) {
    };
    window.ContactFindOptions.prototype = {
        __proto__: null
    };

})(window.navigator.contacts = navigator.contacts || {});
/* Device */
/**
*
* @property {String} cordova Get the version of Cordova running on the device.
* @property {String} model Returns the name of the device's model or product
* @property {String} name device.name is deprecated as of version 2.3.0. Use device.model instead
* @property {String} platform Get the device's operating system name
* @property {String} uuid Get the device's Universally Unique Identifier (UUID)
* @property {String} version Get the operating system version
    */
window['device'] = {
    cordova: '',

    model: string,

    name: string,

    platform: string,

    uuid: string,

    version: '',
    __proto__: null
};
/* Device Motion */
(function () {
    /**
    * This plugin provides access to the device's accelerometer. The accelerometer is a motion sensor
    * that detects the change (delta) in movement relative to the current device orientation,
    * in three dimensions along the x, y, and z axis.
    */

    /**
        * Contains Accelerometer data captured at a specific point in time. Acceleration values include
        * the effect of gravity (9.81 m/s^2), so that when a device lies flat and facing up, x, y, and z
        * values returned should be 0, 0, and 9.81.
    * @property {Number} x Amount of acceleration on the x-axis. (in m/s^2)
    * @property {Number} y Amount of acceleration on the y-axis. (in m/s^2)
    * @property {Number} z Amount of acceleration on the z-axis. (in m/s^2)
    * @property {Number} timestamp Creation timestamp in milliseconds.
        */
    var _acceleration = {
        __proto__: null,
        x: 0,
        y: 0,
        z: 0,
        timestamp: 0
    }

    /**
        * This plugin provides access to the device's accelerometer. The accelerometer is a motion sensor
        * that detects the change (delta) in movement relative to the current device orientation,
        * in three dimensions along the x, y, and z axis.
        */
    var _accelerometer = {
        __proto__: null,
        /**
        * Stop watching the Acceleration referenced by the watchID parameter.
        * @param {Number} watchID The ID returned by navigator.accelerometer.watchAcceleration.
        */
        clearWatch: function (watchID) {

        },

        /**
        * Get the current acceleration along the x, y, and z axes.
        * These acceleration values are returned to the accelerometerSuccess callback function.
        * @param {Function} accelerometerSuccess Success callback that gets the Acceleration object.
        * @param {Function} accelerometerError Error callback
        */
        getCurrentAcceleration: function (accelerometerSuccess, accelerometerError) {
            intellisense.setCallContext(accelerometerSuccess, { thisArg: {}, args: [Object.create(_acceleration)] });
            intellisense.setCallContext(accelerometerError, { thisArg: {}, args: [] });
        },

        /**
        * Retrieves the device's current Acceleration at a regular interval, executing the
        * accelerometerSuccess callback function each time. Specify the interval in milliseconds
        * via the acceleratorOptions object's frequency parameter.
        * The returned watch ID references the accelerometer's watch interval, and can be used
        * with navigator.accelerometer.clearWatch to stop watching the accelerometer.
        * @param {Function} accelerometerSuccess Callback, that called at every time interval and passes an Acceleration object.
        * @param {Function} accelerometerError Error callback.
        * @param {Object} [accelerometerOptions] Object with options for watchAcceleration
        *   frequency (number): How often to retrieve the Acceleration in milliseconds. (Default: 10000)
        */
        watchAcceleration: function (accelerometerSuccess, accelerometerError, accelerometerOptions) {
            intellisense.setCallContext(accelerometerSuccess, { thisArg: {}, args: [Object.create(_acceleration)] });
            intellisense.setCallContext(accelerometerError, { thisArg: {}, args: [] });
            return 0;
        }
    }

    window.navigator.accelerometer = Object.create(_accelerometer);
})();
/* Device Orientation */
(function (navigator) {
    // TODO: TS Typings - getCurrentHeading doesn't have CompassOptions

    /** A CompassHeading object is returned to the compassSuccess callback function. 
    * @property {Number} magneticHeading The heading in degrees from 0-359.99 at a single moment in time.
    * @property {Number} trueHeading The heading relative to the geographic North Pole in degrees 0-359.99 at a single moment in time. A negative value indicates that the true heading can't be determined.
    * @property {Number} headingAccuracy The deviation in degrees between the reported heading and the true heading.
    * @property {Number} timestamp The time at which this heading was determined.
    */
    var _compassHeading = {
        __proto__: null,
        magneticHeading: 0,
        trueHeading: 0,
        headingAccuracy: 0,
        timestamp: 0
    };

    var _compassOptions = {
        __proto__: null,
        filter: 0,
        frequency: 0
    };

    /** A CompassError object is returned to the onError callback function when an error occurs.
    * @property {Number} code One of the predefined error codes
    *     CompassError.COMPASS_INTERNAL_ERR
    *     CompassError.COMPASS_NOT_SUPPORTED
    */
    window.CompassError = function () {
        this.code = 0;
    };
    CompassError.__proto__ = null;
    CompassError.prototype.__proto__ = null;
    CompassError.COMPASS_INTERNAL_ERR = 0;
    CompassError.COMPASS_NOT_SUPPORTED = 0;

    /** 
        * This plugin provides access to the device's compass. The compass is a sensor that detects
        * the direction or heading that the device is pointed, typically from the top of the device.
        * It measures the heading in degrees from 0 to 359.99, where 0 is north.
        */
    var _compass = {
        __proto__: null,

        /**
        * Get the current compass heading. The compass heading is returned via a CompassHeading
        * object using the onSuccess callback function.
        * @param {Function} onSuccess Success callback that passes CompassHeading object.
        * @param {Function} onError Error callback that passes CompassError object.
        */
        getCurrentHeading: function (onSuccess, onError) {
            intellisense.setCallContext(onSuccess, { thisArg: {}, args: [Object.create(_compassHeading)] });
            intellisense.setCallContext(onError, { thisArg: {}, args: [new CompassError()] });
        },

        /**
        * Gets the device's current heading at a regular interval. Each time the heading is retrieved,
        * the headingSuccess callback function is executed. The returned watch ID references the compass
        * watch interval. The watch ID can be used with navigator.compass.clearWatch to stop watching
        * the navigator.compass.
        * @param {Function} onSuccess Success callback that passes CompassHeading object.
        * @param {Function} onError Error callback that passes CompassError object.
        * @param {CompassOptions} [options] CompassOptions object
        *
        * @typedef {Object} CompassOptions
        *   @property {number} frequency How often to retrieve the compass heading in milliseconds. (Default: 100)
        *   @property {number} filter The change in degrees required to initiate a watchHeading success callback. When this value is set, frequency is ignored.
        */
        watchHeading: function (onSuccess, onError, options) {
            intellisense.setCallContext(onSuccess, { thisArg: {}, args: [Object.create(_compassHeading)] });
            intellisense.setCallContext(onError, { thisArg: {}, args: [new CompassError()] });

            return 0;
        },

        /**
        * Stop watching the compass referenced by the watch ID parameter.
        * @param {Number} id The ID returned by navigator.compass.watchHeading.
        */
        clearWatch: function (id) {
        }
    };
    navigator.compass = Object.create(_compass);
})(window.navigator = window.navigator || {});
/* File System */
(function () {
    cordova.file = {
        __proto__: null,
        // Read-only directory where the application is installed.
        applicationDirectory: null,
        // Root of app's private writable storage
        applicationStorageDirectory: null,
        // Where to put app-specific data files.
        dataDirectory: null,
        // Cached files that should survive app restarts.
        // Apps should not rely on the OS to delete files in here.
        cacheDirectory: null,
        // Android: the application space on external storage.
        externalApplicationStorageDirectory: null,
        // Android: Where to put app-specific data files on external storage.
        externalDataDirectory: null,
        // Android: the application cache on external storage.
        externalCacheDirectory: null,
        // Android: the external storage (SD card) root.
        externalRootDirectory: null,
        // iOS: Temp directory that the OS can clear at will.
        tempDirectory: null,
        // iOS: Holds app-specific files that should be synced (e.g. to iCloud).
        syncedDataDirectory: null,
        // iOS: Files private to the app, but that are meaningful to other applciations (e.g. Office files)
        documentsDirectory: null,
        // BlackBerry10: Files globally available to all apps
        sharedDirectory: null
    };

    /**
    * A FileError object is returned to the onError callback function when an error occurs.
    * @param {Number} code One of the predefined error codes
    * FileError.NOT_FOUND_ERR  
    * FileError.SECURITY_ERR  
    * FileError.ABORT_ERR  
    * FileError.NOT_READABLE_ERR  
    * FileError.ENCODING_ERR  
    * FileError.NO_MODIFICATION_ALLOWED_ERR  
    * FileError.INVALID_STATE_ERR  
    * FileError.SYNTAX_ERR  
    * FileError.INVALID_MODIFICATION_ERR  
    * FileError.QUOTA_EXCEEDED_ERR  
    * FileError.TYPE_MISMATCH_ERR  
    * FileError.PATH_EXISTS_ERR
    * @property {Number} code One of the predefined error codes
    * FileError.NOT_FOUND_ERR  
    * FileError.SECURITY_ERR  
    * FileError.ABORT_ERR  
    * FileError.NOT_READABLE_ERR  
    * FileError.ENCODING_ERR  
    * FileError.NO_MODIFICATION_ALLOWED_ERR  
    * FileError.INVALID_STATE_ERR  
    * FileError.SYNTAX_ERR  
    * FileError.INVALID_MODIFICATION_ERR  
    * FileError.QUOTA_EXCEEDED_ERR  
    * FileError.TYPE_MISMATCH_ERR  
    * FileError.PATH_EXISTS_ERR
    */
    window.FileError = function (code) {
        this.code = 0;
    };
    FileError.__proto__ = null;
    FileError.prototype.__proto__ = null;
    FileError.NOT_FOUND_ERR = 1;
    FileError.SECURITY_ERR = 2;
    FileError.ABORT_ERR = 3;
    FileError.NOT_READABLE_ERR = 4;
    FileError.ENCODING_ERR = 5;
    FileError.NO_MODIFICATION_ALLOWED_ERR = 6;
    FileError.INVALID_STATE_ERR = 7;
    FileError.SYNTAX_ERR = 8;
    FileError.INVALID_MODIFICATION_ERR = 9;
    FileError.QUOTA_EXCEEDED_ERR = 10;
    FileError.TYPE_MISMATCH_ERR = 11;
    FileError.PATH_EXISTS_ERR = 12;

    /** Interfaces **/

    /* This interface supplies information about the state of a file or directory.
    * @property {Date} modificationTime This is the time at which the file or directory was last modified.
    * @property {Number} size The size of the file, in bytes. This must return 0 for directories.
    */
    var _metadata = {
        __proto__: null,

        modificationTime: new Date(),
        size: 0
    };

    /**
    * @property {String} name name of the file, without path information
    * @property {String} fullPath the full path of the file, including the name
    * @property {String} type mime type
    * @property {Date} lastModifiedDate last modified date
    * @property {Number} size size of the file in bytes
    */
    window.File = function () {
    };
    window.File.prototype = {
        __proto__: null,

        /**
        * Returns a "slice" of the file. Since Cordova Files don't contain the actual 
        * content, this really returns a File with adjusted start and end.
        * Slices of slices are supported.
        * @param {Number} [start] The index at which to start the slice (inclusive).
        * @param {Number} [end] The index at which to end the slice (exclusive).
        */
        slice: function (start, end) {
            return new _File();
        }
    };

    /**
    * An abstract interface representing entries in a file system, each of which may be a File or DirectoryEntry.
    * @param {boolean} isFile
    * @param {boolean} isDirectory
    * @param {String} name The name of the entry, excluding the path leading to it.
    * @param {String} fullPath The full absolute path from the root to the entry.
    * @param {FileSystem} fileSystem The file system on which the entry resides.
    * @param {String} nativeURL
    * @property {boolean} isFile
    * @property {boolean} isDirectory
    * @property {String} name The name of the entry, excluding the path leading to it.
    * @property {String} fullPath The full absolute path from the root to the entry.
    * @property {FileSystem} fileSystem The file system on which the entry resides.
    * @property {String} nativeURL
    */
    function _Entry(isFile, isDirectory, name, fullPath, fileSystem, nativeURL) {
    };

    _Entry.prototype = {
        __proto__: null,

        /**
        * Look up metadata about this entry.
        * @param {Function} successCallback A callback that is called with the time of the last modification.
        * @param {Function} [errorCallback] A callback that is called when errors happen.
        */
        getMetadata: function (successCallback, errorCallback) {

            intellisense.setCallContext(successCallback, { thisArg: {}, args: [Object.create(_metadata)] });
            intellisense.setCallContext(errorCallback, { thisArg: {}, args: [new FileError()] });

        },

        /**
        * Move an entry to a different location on the file system. It is an error to try to:
        *     move a directory inside itself or to any child at any depth;move an entry into its parent if a name different from its current one isn't provided;
        *     move a file to a path occupied by a directory;
        *     move a directory to a path occupied by a file;
        *     move any element to a path occupied by a directory which is not empty.
        * A move of a file on top of an existing file must attempt to delete and replace that file.
        * A move of a directory on top of an existing empty directory must attempt to delete and replace that directory.
        * @param {DirectoryEntry} parent The directory to which to move the entry.
        * @param {String} [newName] The new name of the entry. Defaults to the Entry's current name if unspecified.
        * @param {Function} [successCallback] A callback that is called with the Entry for the new location.
        * @param {Function} [errorCallback] A callback that is called when errors happen.
        */
        moveTo: function (parent, newName, successCallback, errorCallback) {
            intellisense.setCallContext(successCallback, { thisArg: {}, args: [new _Entry()] });
            intellisense.setCallContext(errorCallback, { thisArg: {}, args: [new FileError()] });
        },

        /**
        * Copy an entry to a different location on the file system. It is an error to try to:
        *     copy a directory inside itself or to any child at any depth;
        *     copy an entry into its parent if a name different from its current one isn't provided;
        *     copy a file to a path occupied by a directory;
        *     copy a directory to a path occupied by a file;
        *     copy any element to a path occupied by a directory which is not empty.
        *     A copy of a file on top of an existing file must attempt to delete and replace that file.
        *     A copy of a directory on top of an existing empty directory must attempt to delete and replace that directory.
        * Directory copies are always recursive--that is, they copy all contents of the directory.
        * @param {DirectoryEntry} parent The directory to which to move the entry.
        * @param {String} [newName] The new name of the entry. Defaults to the Entry's current name if unspecified.
        * @param {Function} [successCallback] A callback that is called with the Entry for the new object.
        * @param {Function} [errorCallback] A callback that is called when errors happen.
        */
        copyTo: function (parent, newName, successCallback, errorCallback) {
            intellisense.setCallContext(successCallback, { thisArg: {}, args: [new _Entry()] });
            intellisense.setCallContext(errorCallback, { thisArg: {}, args: [new FileError()] });
        },

        /**
        * @returns {string} Returns a URL that can be used as the src attribute of a &lt;video&gt; or &lt;audio&gt; tag. If that is not possible, construct a cdvfile:// URL.
        */
        toURL: function () {
            return "";
        },

        /**
        * @returns {string} Return a URL that can be passed across the bridge to identify this entry.
        */
        toInternalURL: function () {
            return "";
        },

        /**
        * Deletes a file or directory. It is an error to attempt to delete a directory that is not empty. It is an error to attempt to delete the root directory of a filesystem.
        * @param {Function} successCallback A callback that is called on success.
        * @param {Function} [errorCallback] A callback that is called when errors happen.
        */
        remove: function (successCallback, errorCallback) {
            intellisense.setCallContext(successCallback, { thisArg: {} });
            intellisense.setCallContext(errorCallback, { thisArg: {}, args: [new FileError()] });
        },

        /**
        * Look up the parent DirectoryEntry containing this Entry. If this Entry is the root of its filesystem, its parent is itself.
        * @param {Function} successCallback A callback that is called with the time of the last modification.
        * @param {Function} [errorCallback] A callback that is called when errors happen.
        */
        getParent: function (successCallback, errorCallback) {
            intellisense.setCallContext(successCallback, { thisArg: {}, args: [new _Entry()] });
            intellisense.setCallContext(errorCallback, { thisArg: {}, args: [new FileError()] });
        }
    };

    /**
    * This interface represents a file on a file system.
    */
    function _FileEntry() {
    }

    _FileEntry.prototype = {
        __proto__: new _Entry(),

        /**
        * Creates a new FileWriter associated with the file that this FileEntry represents.
        * @param {Function} successCallback A callback that is called with the new FileWriter.
        * @param {Function} [errorCallback] A callback that is called when errors happen.
        */
        createWriter: function (successCallback, errorCallback) {
            intellisense.setCallContext(successCallback, { thisArg: {}, args: [new _FileWriter] });
            intellisense.setCallContext(errorCallback, { thisArg: {}, args: [new FileError()] });
        },

        /**
        * Returns a File that represents the current state of the file that this FileEntry represents.
        * @param {Function} successCallback A callback that is called with the File.
        * @param {Function} [errorCallback] A callback that is called when errors happen.
        */
        file: function (successCallback, errorCallback) {
            intellisense.setCallContext(successCallback, { thisArg: {}, args: [new _File()] });
            intellisense.setCallContext(errorCallback, { thisArg: {}, args: [new FileError()] });
        }
    };

    /**
    * This interface represents a directory on a file system.
    */
    function _DirectoryEntry() {
    }

    _DirectoryEntry.prototype = {
        __proto__: new _Entry(),

        /**
        * Creates a new DirectoryReader to read Entries from this Directory.
        */
        createReader: function () {
            return new _DirectoryReader();
        },

        /**
        * Creates or looks up a file.
        * @param {String} path Either an absolute path or a relative path from this DirectoryEntry
        *                to the file to be looked up or created.
        *                It is an error to attempt to create a file whose immediate parent does not yet exist.
        * @param {Flags} [options] create (boolean): Used to indicate that the user wants to create a file or directory if it was not previously there.
        * exclusive (boolean): By itself, exclusive must have no effect. Used with create, it must cause getFile and getDirectory to fail if the target path already exists.
        * If create and exclusive are both true, and the path already exists, getFile must fail.
        *                If create is true, the path doesn't exist, and no other error occurs, getFile must create it as a zero-length file and return a corresponding FileEntry.
        *                If create is not true and the path doesn't exist, getFile must fail.
        *                If create is not true and the path exists, but is a directory, getFile must fail.
        *                Otherwise, if no other error occurs, getFile must return a FileEntry corresponding to path.
        * @param {Function} [successCallback] A callback that is called to return the File selected or created.
        * @param {Function} [errorCallback] A callback that is called when errors happen.
        */
        getFile: function (path, options, successCallback, errorCallback) {
            intellisense.setCallContext(successCallback, { thisArg: {}, args: [new _FileEntry()] });
            intellisense.setCallContext(errorCallback, { thisArg: {}, args: [new FileError()] });
        },

        /**
        * Creates or looks up a directory.
        * @param {String} path Either an absolute path or a relative path from this DirectoryEntry
        *                to the directory to be looked up or created.
        *                It is an error to attempt to create a directory whose immediate parent does not yet exist.
        * @param {Flags} [options] create (boolean): Used to indicate that the user wants to create a file or directory if it was not previously there.
        * exclusive (boolean): By itself, exclusive must have no effect. Used with create, it must cause getFile and getDirectory to fail if the target path already exists.
        * If create and exclusive are both true and the path already exists, getDirectory must fail.
        *                If create is true, the path doesn't exist, and no other error occurs, getDirectory must create and return a corresponding DirectoryEntry.
        *                If create is not true and the path doesn't exist, getDirectory must fail.
        *                If create is not true and the path exists, but is a file, getDirectory must fail.
        *                Otherwise, if no other error occurs, getDirectory must return a DirectoryEntry corresponding to path.
        * @param {Function} [successCallback] A callback that is called to return the Directory selected or created.
        * @param {Function} [errorCallback] A callback that is called when errors happen.
        */
        getDirectory: function (path, options, successCallback, errorCallback) {

            intellisense.setCallContext(successCallback, { thisArg: {}, args: [new _DirectoryEntry()] });
            intellisense.setCallContext(errorCallback, { thisArg: {}, args: [new FileError()] });
        },

        /**
        *  Deletes a directory and all of its contents, if any. In the event of an error (e.g. trying
        * to delete a directory that contains a file that cannot be removed), some of the contents
        * of the directory may be deleted. It is an error to attempt to delete the root directory of a filesystem.
        * @param {Function} successCallback A callback that is called on success.
        * @param {Function} [errorCallback] A callback that is called when errors happen.
        */
        removeRecursively: function (successCallback, errorCallback) {
            intellisense.setCallContext(successCallback, { thisArg: {}, args: [new _DirectoryEntry()] });
            intellisense.setCallContext(errorCallback, { thisArg: {}, args: [new FileError()] });
        }
    };

    /**
    * This dictionary is used to supply arguments to methods
    * that look up or create files or directories.
    * @property {boolean} create Used to indicate that the user wants to create a file or directory if it was not previously there. 
    * @property {boolean} exclusive By itself, exclusive must have no effect. Used with create, it must cause getFile and getDirectory to fail if the target path already exists.
    */
    var _Flags = {
        __proto__: null,

        create: false,
        exclusive: false
    };

    /**
    *  This interface lets a user list files and directories in a directory. If there are
    * no additions to or deletions from a directory between the first and last call to
    * readEntries, and no errors occur, then:
    *     A series of calls to readEntries must return each entry in the directory exactly once.
    *     Once all entries have been returned, the next call to readEntries must produce an empty array.
    *     If not all entries have been returned, the array produced by readEntries must not be empty.
    *     The entries produced by readEntries must not include the directory itself ["."] or its parent [".."].
    * 
    */
    var _DirectoryReader = function () {
    };
    _DirectoryReader.prototype = {
        __proto__: null,

        /**
        * Read the next block of entries from this directory.
        * @param {Function} successCallback Called once per successful call to readEntries to deliver the next
        * previously-unreported set of Entries in the associated Directory.
        *                       If all Entries have already been returned from previous invocations
        *                        of readEntries, successCallback must be called with a zero-length array as an argument.
        * @param {Function} [errorCallback] A callback indicating that there was an error reading from the Directory.
        */
        readEntries: function (successCallback, errorCallback) {
            intellisense.setCallContext(successCallback, { thisArg: {}, args: [[new _Entry()]] });
            intellisense.setCallContext(errorCallback, { thisArg: {}, args: [new FileError()] });
        }
    };

    /**
    * This interface represents a file system.
    * @param {String} name This is the name of the file system. The specifics of naming filesystems
    * is unspecified, but a name must be unique across the list of exposed file systems.
    * @param {DirectoryEntry} root The root directory of the file system.
    * @property {String} name This is the name of the file system. The specifics of naming filesystems
    * is unspecified, but a name must be unique across the list of exposed file systems.
    * @property {DirectoryEntry} root The root directory of the file system.
    */
    window.FileSystem = function (name, root) {
    };
    window.FileSystem.prototype = {
        __proto__: null,

        root: new _DirectoryEntry()
    };

    /**
    * Requests a filesystem in which to store application data.
    * @param {Number} type Whether the filesystem requested should be persistent, as defined above. Use one of TEMPORARY or PERSISTENT.
    * @param {Number} size This is an indicator of how much storage space, in bytes, the application expects to need.
    * @param {Function} successCallback The callback that is called when the user agent provides a filesystem.
    * @param {Function} [errorCallback] A callback that is called when errors happen, or when the request to obtain the filesystem is denied.
    */
    window.requestFileSystem = function (type, size, successCallback, errorCallback) {
        intellisense.setCallContext(successCallback, { thisArg: {}, args: [new FileSystem()] });
        intellisense.setCallContext(errorCallback, { thisArg: {}, args: [new FileError()] });
    };

    /**
    * Look up file system Entry referred to by local URI. Note: Supported only on Blackberry devices.
    * @param {String} uri URI referring to a local file or directory.
    * @param {Function} successCallback Invoked with Entry object corresponding to URI
    * @param {Function} [errorCallback] Invoked if error occurs retrieving file system entry
    */
    window.resolveLocalFileSystemURI = function (uri, successCallback, errorCallback) {
        intellisense.setCallContext(successCallback, { thisArg: {}, args: [new _Entry()] });
        intellisense.setCallContext(errorCallback, { thisArg: {}, args: [new FileError()] });
    };

    window.TEMPORARY = 0;
    window.PERSISTENT = 0;

    /**
    * This class writes to the mobile device file system
    * @param {File} file
    * @property {Number} readyState The FileSaver object can be in one of 3 states. The readyState attribute, on getting,
    * must return the current state, which must be one of the following values:
    *     INIT
    *     WRITING
    *     DONE
    * @property {Function} onwritestart Handler for writestart events.
    * @property {Function} onprogress Handler for progress events.
    * @property {Function} onwrite Handler for write events.
    * @property {Function} onabort Handler for abort events.
    * @property {Function} onerror Handler for error events.
    * @property {Function} onwriteend Handler for writeend events.
    * @property {Error} error The last error that occurred on the FileSaver.
    * @property {Number} position The byte offset at which the next write to the file will occur. This must be no greater than length.
    * A newly-created FileWriter must have position set to 0.
    * @property {Number} length The length of the file.
    */
    window.FileWriter = function (file) {
        this.onabort = null;
        this.onerror = null;
        this.onprogress = null;
        this.onwrite = null;
        this.onwriteend = null;
        this.onwritestart = null;

        var progressEvent = {
            type: "",
            bubbles: false,
            cancelBubble: false,
            cancelable: false,
            lengthComputable: false,
            loaded: 0,
            total: 0,
            target: null

        };
        intellisense.setCallContext(this.onabort, { args: [progressEvent] });
        intellisense.setCallContext(this.onerror, { args: [progressEvent] });
        intellisense.setCallContext(this.onprogress, { args: [progressEvent] });
        intellisense.setCallContext(this.onwrite, { args: [progressEvent] });
        intellisense.setCallContext(this.onwriteend, { args: [progressEvent] });
        intellisense.setCallContext(this.onwritestart, { args: [progressEvent] });
    };
    // FileWriter states
    FileWriter.INIT = 0;
    FileWriter.WRITING = 1;
    FileWriter.DONE = 2;

    window.FileWriter.prototype = {
        __proto__: null,

        /**Terminate file operation*/
        abort: function () {
        },

        /**
        * Write the supplied data to the file at position.
        * @param {Blob} data The blob to write.
        */
        write: function (data) {
        },

        /**
        * The file position at which the next write will occur.
        * @param {Number} offset If nonnegative, an absolute byte offset into the file.
        * If negative, an offset back from the end of the file.
        */
        seek: function (offset) {
        },

        /**
        * Changes the length of the file to that specified. If shortening the file, data beyond the new length
        * will be discarded. If extending the file, the existing data will be zero-padded up to the new length.
        * @param {Number} size The size to which the length of the file is to be adjusted, measured in bytes.
        */
        truncate: function (size) {

        }
    };
})();
/* File Transfer */
(function () {
    /**    
    * @property {Number} bytesSent The number of bytes sent to the server as part of the upload.
    * @property {Number} responseCode The HTTP response code returned by the server.
    * @property {String} response The HTTP response returned by the server.
    * @property {Object[]} headers The HTTP response headers by the server. Currently supported on iOS only.
    */
    var _fileUploadResult = {
        __proto__: null,
        bytesSent: 0,
        responseCode: 0,
        response: "",
        headers: [object]
    };

    /**
    * The FileTransfer object provides a way to upload files using an HTTP multi-part POST request,
    * and to download files as well.
    */
    window.FileTransfer = function () {
    };
    FileTransfer.prototype.__proto__ = null;
    FileTransfer.prototype.onprogress = function () {
        // Called with a ProgressEvent whenever a new chunk of data is transferred.
    };

    /**
    * Sends a file to a server.
    * @param {String} fileURL Filesystem URL representing the file on the device. For backwards compatibility,
    *                                this can also be the full path of the file on the device.
    * @param {String} server URL of the server to receive the file, as encoded by encodeURI().
    * @param {Function} successCallback A callback that is passed a FileUploadResult object.
    * @param {Function} errorCallback A callback that executes if an error occurs retrieving the FileUploadResult. 
    *                               Invoked with a FileTransferError object.
    * @param {FileDownloadOptions} ['options] Optional parameters.
    * fileKey (string): The name of the form element. Defaults to file.
    * fileName (string): The file name to use when saving the file on the server. Defaults to image.jpg.
    * mimeType (string): The mime type of the data to upload. Defaults to image/jpeg.
    * params (Object): A set of optional key/value pairs to pass in the HTTP request.
    * chunkedMode (boolean): Whether to upload the data in chunked streaming mode. Defaults to true.
    * headers (Object[]): A map of header name/header values. Use an array to specify more than one value.
    * @param {boolean} [trustAllHosts] Optional parameter, defaults to false. If set to true, it accepts all security certificates.
    *                               This is useful since Android rejects self-signed security certificates.
    *                               Not recommended for production use. Supported on Android and iOS.
    */
    FileTransfer.prototype.upload = function (fileURL, server, successCallback, errorCallback, options, trustAllHosts) {
        intellisense.setCallContext(successCallback, { thisArg: {}, args: [Object.create(_fileUploadResult)] });
        intellisense.setCallContext(errorCallback, { thisArg: {}, args: [new FileTransferError()] });
    };

    /**
    * downloads a file from server.
    * @param {String} source URL of the server to download the file, as encoded by encodeURI().
    * @param {String} target Filesystem url representing the file on the device. For backwards compatibility,
    *                               this can also be the full path of the file on the device.
    * @param {Function} successCallback A callback that is passed a FileEntry object. (Function)
    * @param {Function} errorCallback A callback that executes if an error occurs when retrieving the fileEntry.
    *                               Invoked with a FileTransferError object.
    * @param {FileDownloadOptions} [options] Optional parameters.
    * headers (Object[]): A map of header name/header values. Use an array to specify more than one value.
        
    * @param {boolean} [trustAllHosts] Optional parameter, defaults to false. If set to true, it accepts all security certificates.
    *                               This is useful since Android rejects self-signed security certificates.
    *                               Not recommended for production use. Supported on Android and iOS.
    */
    FileTransfer.prototype.download = function (source, target, successCallback, errorCallback, options, trustAllHosts) {
        intellisense.setCallContext(successCallback, { thisArg: {}, args: [Object.create(_fileUploadResult)] });
        intellisense.setCallContext(errorCallback, { thisArg: {}, args: [new FileTransferError()] });
    };

    /**
    *  Aborts an in-progress transfer. The onerror callback is passed a FileTransferError object
    * which has an error code of FileTransferError.ABORT_ERR.
    */
    FileTransfer.prototype.abort = function () {

    };

    /**
    * A FileTransferError object is passed to an error callback when an error occurs.
    * @param {Number} [code] One of the predefined error codes listed below.
    *     FileTransferError.FILE_NOT_FOUND_ERR
    *     FileTransferError.INVALID_URL_ERR
    *     FileTransferError.CONNECTION_ERR
    *     FileTransferError.ABORT_ERR
    * @param {String} [source] URL to the source.
    * @param {String} [target] URL to the target.
    * @param {Number} [status] HTTP status code. This attribute is only available when a response code is received from the HTTP connection.
    * @param {Object} [body] Request body
    * @param {Object} [exception] Exception that is thrown by native code 
    * @property {Number} [code] One of the predefined error codes listed below.
    *     FileTransferError.FILE_NOT_FOUND_ERR
    *     FileTransferError.INVALID_URL_ERR
    *     FileTransferError.CONNECTION_ERR
    *     FileTransferError.ABORT_ERR
    * @property {String} source URL to the source.
    * @property {String} target URL to the target.
    * @property {Number} http_status HTTP status code. This attribute is only available when a response code is received from the HTTP connection.
    * @property {Object} body Request body
    * @property {Object} exception Exception that is thrown by native code 
    */
    window.FileTransferError = function (code, source, target, status, body, exception) {

    };
    FileTransferError.FILE_NOT_FOUND_ERR = 1;
    FileTransferError.INVALID_URL_ERR = 2;
    FileTransferError.CONNECTION_ERR = 3;
    FileTransferError.ABORT_ERR = 4;
})();
/* Globalizaton */
(function (navigator) {
    /**This plugin obtains information and performs operations specific to the user's locale and timezone.*/
    function Globalization() {
    }
    /**
    * Get the string identifier for the client's current language.
    * @param {Function} onSuccess Called on success getting the language with a properties object,
    *                  that should have a value property with a String value.
    * @param {Function} onError Called on error getting the language with a GlobalizationError object.
    *                  The error's expected code is GlobalizationError.UNKNOWN_ERROR.
    */
    Globalization.prototype.getPreferredLanguage = function (onSuccess, onError) {
        intellisense.setCallContext(onSuccess, { thisArg: {}, args: [{ value: '' }] });
        intellisense.setCallContext(onError, { thisArg: {}, args: [new GlobalizationError()] });
    };

    /**
    * Get the string identifier for the client's current locale setting.
    * @param {Function} onSuccess Called on success getting the locale identifier with a properties object,
    *                  that should have a value property with a String value.
    * @param {Function} onError Called on error getting the locale identifier with a GlobalizationError object.
    *                  The error's expected code is GlobalizationError.UNKNOWN_ERROR.
    */
    Globalization.prototype.getLocaleName = function (onSuccess, onError) {
        intellisense.setCallContext(onSuccess, { thisArg: {}, args: [{ value: '' }] });
        intellisense.setCallContext(onError, { thisArg: {}, args: [new GlobalizationError()] });
    };

    /**
    * Returns a date formatted as a string according to the client's locale and timezone.
    * @param {Date} date Date to format.
    * @param {Function} onSuccess Called on success with a properties object,
    *                  that should have a value property with a String value.
    * @param {Function} onError Called on error with a GlobalizationError object.
    *                  The error's expected code is GlobalizationError.FORMATTING_ERROR.
    * @param {Object} [options] Optional format parameters. Default {formatLength:'short', selector:'date and time'}
    * formatLength (string): can be 'short', 'medium', 'long', or 'full'.
    * selector (string): can be 'date', 'time' or 'date and time'
    */
    Globalization.prototype.dateToString = function (date, onSuccess, onError, options) {
        intellisense.setCallContext(onSuccess, { thisArg: {}, args: [{ value: '' }] });
        intellisense.setCallContext(onError, { thisArg: {}, args: [new GlobalizationError()] });
    };

    /**
    * Date returned by stringToDate
    * @property {Number} year The four digit year.
    * @property {Number} month The month from (0-11).
    * @property {Number} day The day from (1-31).
    * @property {Number} hour The hour from (0-23).
    * @property {Number} minute The minute from (0-59).
    * @property {Number} second The second from (0-59).
    * @property {Number} millisecond The milliseconds (from 0-999), not available on all platforms.
    */
    var _globalizationDate = {
        __proto__: null,
        year: 0,
        month: 0,
        day: 0,
        hour: 0,
        minute: 0,
        second: 0,
        millisecond: 0
    };

    /**
    * Pattern to format and parse dates according to the client's user preferences.
    * @property {String} pattern The date and time pattern to format and parse dates. The patterns follow Unicode Technical Standard #35.
    * @property {String} timezone The abbreviated name of the time zone on the client.
    * @property {Number} utc_offset The current difference in seconds between the client's time zone and coordinated universal time.
    * @property {Number} dst_offset The current daylight saving time offset in seconds between the client's non-daylight saving's time zone and the client's daylight saving's time zone.
    */
    var _globalizationDatePattern = {
        __proto__: null,
        pattern: '',
        timezone: '',
        utc_offset: 0,
        dst_offset: 0
    };

    /**
    * Pattern to format and parse numbers according to the client's user preferences.
    * @property {String} pattern The number pattern to format and parse numbers. The patterns follow Unicode Technical Standard #35.
    * @property {String} symbol The symbol to use when formatting and parsing, such as a percent or currency symbol.
    * @property {Number} fraction The number of fractional digits to use when parsing and formatting numbers.
    * @property {Number} rounding The rounding increment to use when parsing and formatting.
    * @property {String} positive The symbol to use for positive numbers when parsing and formatting.
    * @property {String} negative The symbol to use for negative numbers when parsing and formatting.
    * @property {String} decimal The decimal symbol to use for parsing and formatting.
    * @property {String} grouping The grouping symbol to use for parsing and formatting.
    */
    var _globalizationNumberPattern = {
        __proto__: null,
        pattern: '',
        symbol: '',
        fraction: 0,
        rounding: 0,
        positive: '',
        negative: '',
        decimal: '',
        grouping: ''
    };

    /**
    * Pattern to format and parse currency values according
    * to the client's user preferences and ISO 4217 currency code.
    * @property {String} pattern The currency pattern to format and parse currency values. The patterns follow Unicode Technical Standard #35.
    * @property {String} code The ISO 4217 currency code for the pattern.
    * @property {Number} fraction The number of fractional digits to use when parsing and formatting currency.
    * @property {Number} rounding The rounding increment to use when parsing and formatting.
    * @property {String} decimal The decimal symbol to use for parsing and formatting.
    * @property {String} grouping The grouping symbol to use for parsing and formatting.
    */
    var _globalizationCurrencyPattern = {
        __proto__: null,
        pattern: '',
        code: '',
        fraction: 0,
        rounding: 0,
        decimal: '',
        grouping: ''
    };

    /**
    * An object representing a error from the Globalization API.
    * @property {Number} code One of the following codes representing the error type:
    * GlobalizationError.UNKNOWN_ERROR: 0
    * GlobalizationError.FORMATTING_ERROR: 1
    * GlobalizationError.PARSING_ERROR: 2
    * GlobalizationError.PATTERN_ERROR: 3
    * @property {String} message A text message that includes the error's explanation and/or details
    */
    var _globalizationError = {
        __proto__: null,
        code: 0,
        message: ''
    };

    /**
    * Parses a date formatted as a string, according to the client's user preferences
    * and calendar using the time zone of the client, and returns the corresponding date object.
    * @param {String} dateString String to parse
    * @param {Function} onSuccess Called on success with GlobalizationDate object
    * @param {Function} onError Called on error getting the language with a GlobalizationError object.
    *                    The error's expected code is GlobalizationError.PARSING_ERROR.
    * @param {Object} [options] Optional format parameters. Default {formatLength:'short', selector:'date and time'}
    * formatLength (string): can be 'short', 'medium', 'long', or 'full'.
    * selector (string): can be 'date', 'time' or 'date and time'
    */
    Globalization.prototype.stringToDate = function (dateString, onSuccess, onError, options) {
        intellisense.setCallContext(onSuccess, { thisArg: {}, args: [Object.create(_globalizationDate)] });
        intellisense.setCallContext(onError, { thisArg: {}, args: [Object.create(_globalizationError)] });
    };

    /**
    * Returns a pattern string to format and parse dates according to the client's user preferences.
    * @param {Function} onSuccess Called on success getting pattern with a GlobalizationDatePattern object
    * @param {Function} onError Called on error getting pattern with a GlobalizationError object.
    *                  The error's expected code is GlobalizationError.PATTERN_ERROR.
    * @param {Object} [options] Optional format parameters. Default {formatLength:'short', selector:'date and time'}
    * formatLength (string): can be 'short', 'medium', 'long', or 'full'.
    * selector (string): can be 'date', 'time' or 'date and time'
    */
    Globalization.prototype.getDatePattern = function (onSuccess, onError, options) {
        intellisense.setCallContext(onSuccess, { thisArg: {}, args: [Object.create(_globalizationDatePattern)] });
        intellisense.setCallContext(onError, { thisArg: {}, args: [Object.create(_globalizationError)] });
    };

    /**
    *  Returns an array of the names of the months or days of the week, depending on the client's user preferences and calendar.
    * @param {Function} onSuccess Called on success getting names with a properties object,
    *                  that should have a value property with a String[] value.
    * @param {Function} onError Called on error getting the language with a GlobalizationError object.
    *                  The error's expected code is GlobalizationError.UNKNOWN_ERROR.
    * @param {Function} [options] Optional parameters. Default: {type:'wide', item:'months'}
    * type (string): can be 'narrow' or 'wide'.
    * item (string): can be 'months' or 'days'
    */
    Globalization.prototype.getDateNames = function (onSuccess, onError, options) {
        intellisense.setCallContext(onSuccess, { thisArg: {}, args: [{ value: [''] }] });
        intellisense.setCallContext(onError, { thisArg: {}, args: [Object.create(_globalizationError)] });
    };

    /**
    * Indicates whether daylight savings time is in effect for a given date using the client's time zone and calendar.
    * @param {Date} date Date to check
    * @param {Function} onSuccess Called on success with a properties object,
    *                  that should have a dst property with a boolean value.
    * @param {Function} onError Called on error with a GlobalizationError object.
    *                  The error's expected code is GlobalizationError.UNKNOWN_ERROR.
    */
    Globalization.prototype.isDaylightSavingsTime = function (date, onSuccess, onError) {
        intellisense.setCallContext(onSuccess, { thisArg: {}, args: [{ dst: false }] });
        intellisense.setCallContext(onError, { thisArg: {}, args: [Object.create(_globalizationError)] });
    };


    /**
    * Returns the first day of the week according to the client's user preferences and calendar.
    * @param {Function} onSuccess Called on success with a day object,
    *                  that should have a value property with a number value.
    * @param {Function} onError Called on error with a GlobalizationError object.
    *                  The error's expected code is GlobalizationError.UNKNOWN_ERROR.
    */
    Globalization.prototype.getFirstDayOfWeek = function (onSuccess, onError) {
        intellisense.setCallContext(onSuccess, { thisArg: {}, args: [{ value: 0 }] });
        intellisense.setCallContext(onError, { thisArg: {}, args: [Object.create(_globalizationError)] });
    };

    /**
    * Returns a number formatted as a string according to the client's user preferences.
    * @param {Number} value Number to format
    * @param {Function} onSuccess Called on success with a result object,
    *                  that should have a value property with a String value.
    * @param {Function} onError Called on error with a GlobalizationError object.
    *                  The error's expected code is GlobalizationError.FORMATTING_ERROR.
    * @param {Object} [format] Optional format parameters. Default: {type:'decimal'}
    * type (string): can be 'decimal', 'percent', or 'currency'.
    */
    Globalization.prototype.numberToString = function (value, onSuccess, onError, format) {
        intellisense.setCallContext(onSuccess, { thisArg: {}, args: [{ value: '' }] });
        intellisense.setCallContext(onError, { thisArg: {}, args: [Object.create(_globalizationError)] });
    };

    /**
    * Parses a number formatted as a string according to the client's user preferences and returns the corresponding number.
    * @param {String} value String to parse
    * @param {Function} onSuccess Called on success with a result object,
    *                  that should have a value property with a number value.
    * @param {Function} onError Called on error with a GlobalizationError object.
    *                  The error's expected code is GlobalizationError.FORMATTING_ERROR.
    * @param {Object} [format] Optional format parameters. Default: {type:'decimal'}
    * type (string): can be 'decimal', 'percent', or 'currency'.
    */
    Globalization.prototype.stringToNumber = function (value, onSuccess, onError, format) {
        intellisense.setCallContext(onSuccess, { thisArg: {}, args: [{ value: 0 }] });
        intellisense.setCallContext(onError, { thisArg: {}, args: [Object.create(_globalizationError)] });
    };

    /**
    * Returns a pattern string to format and parse numbers according to the client's user preferences.
    * @param {Function} onSuccess Called on success getting pattern with a GlobalizationNumberPattern object
    * @param {Function} onError Called on error getting the language with a GlobalizationError object.
    *                  The error's expected code is GlobalizationError.PATTERN_ERROR.
    * @param {Object} [options] Optional format parameters. Default {type:'decimal'}.
    * type (string): can be 'decimal', 'percent', or 'currency'.
    */
    Globalization.prototype.getNumberPattern = function (onSuccess, onError, format) {
        intellisense.setCallContext(onSuccess, { thisArg: {}, args: [Object.create(_globalizationNumberPattern)] });
        intellisense.setCallContext(onError, { thisArg: {}, args: [Object.create(_globalizationError)] });
    };

    /**
    *  Returns a pattern string to format and parse currency values according to the client's user preferences and ISO 4217 currency code.
    * @param {String} currencyCode Should be a String of one of the ISO 4217 currency codes, for example 'USD'.
    * @param {Function} onSuccess Called on success getting pattern with a GlobalizatioCurrencyPattern object
    * @param {Function} onError       Called on error getting pattern with a GlobalizationError object.
    *                      The error's expected code is GlobalizationError.FORMATTING_ERROR.
    */
    Globalization.prototype.getCurrencyPattern = function (currencyCode, onSuccess, onError) {
        intellisense.setCallContext(onSuccess, { thisArg: {}, args: [Object.create(_globalizationCurrencyPattern)] });
        intellisense.setCallContext(onError, { thisArg: {}, args: [Object.create(_globalizationError)] });
    };

    navigator.globalization = new Globalization();
})(window.navigator = window.navigator || {});
/* Media */
/**
* @property {Number} code Error code
*/
window["CaptureError"] = function (c) {
    this.code = c || null;
    this.__proto__ = null;
};
window["CaptureError"].CAPTURE_INTERNAL_ERR = 0;
window["CaptureError"].CAPTURE_APPLICATION_BUSY = 1;
window["CaptureError"].CAPTURE_INVALID_ARGUMENT = 2;
window["CaptureError"].CAPTURE_NO_MEDIA_FILES = 3;
window["CaptureError"].CAPTURE_NOT_SUPPORTED = 4;
window["CaptureError"].__proto__ = null;

(function (window) {
    /**
    * @property {Number} code Error code
    * @property {String} message Error message
    */
    var mediaErrorObj = {
        code: 0,
        message: '',
        __proto__: null
    };

    var media = (function () {
        /**
        * Constructor for Media object
        * @param {String} src A URI containing the audio content
        * @param {Function} [mediaSuccess] The callback that executes after a Media object has completed
        * @param {Function} [mediaError] The callback that executes if an error occurs
        * @param {Function} [mediaStatus] The callback that executes to indicate status changes
        */
        function Media(src, mediaSuccess, mediaError, mediaStatus) {
            intellisense.setCallContext(mediaSuccess, { thisArg: {}, args: [''] });
            intellisense.setCallContext(mediaError, { thisArg: {}, args: [mediaErrorObj] });
            intellisense.setCallContext(mediaStatus, { thisArg: {}, args: [''] });
        };

        /**
        * Returns the current position within an audio file. Also updates the Media object's position parameter
        * @param {Function} mediaSuccess The callback that is passed the current position in seconds
        * @param {Function} [mediaError] The callback to execute if an error occurs
        */
        Media.prototype.getCurrentPosition = function (mediaSuccess, mediaError) {
            intellisense.setCallContext(mediaSuccess, { thisArg: {}, args: [''] });
            intellisense.setCallContext(mediaError, { thisArg: {}, args: [mediaErrorObj] });
        };

        /**
        * Returns the duration of an audio file in seconds. If the duration is unknown, it returns a value of -1
        * @returns {number}
        */
        Media.prototype.getDuration = function () {
            return 0;
        };

        /**
        * Starts or resumes playing an audio file
        */
        Media.prototype.play = function () {
        };

        /**
        * Pauses playing an audio file
        */
        Media.prototype.pause = function () {
        };

        /**
        * Releases the underlying operating system's audio resources
        */
        Media.prototype.release = function () {
        };

        /**
        * Sets the current position within an audio file
        * @param {Number} position Position in milliseconds
        */
        Media.prototype.seekTo = function (position) {
        };

        /**
        * Set the volume for an audio file
        * @param {Number} volume The volume to set for playback. The value must be within the range of 0.0 to 1.0
        */
        Media.prototype.setVolume = function (volume) {
        };

        /**
        * Starts recording an audio file
        */
        Media.prototype.startRecord = function () {
        };

        /**
        * Stops recording an audio file
        */
        Media.prototype.stopRecord = function () {
        };

        /**
        * Stops playing an audio file
        */
        Media.prototype.stop = function () {
        };

        Media.MEDIA_NONE = 0;
        Media.MEDIA_STARTING = 1;
        Media.MEDIA_RUNNING = 2;
        Media.MEDIA_PAUSED = 3;
        Media.MEDIA_STOPPED = 4;

        Media.prototype.__proto__ = null;
        delete Media.prototype.constructor;
        Media.__proto__ = null;

        return Media;
    })();

    window['Media'] = media;
})(window);

// MediaCapture

(function () {

    window.navigator['device'] = {
        /**
        * @property {ConfigurationData[]} supportedAudioModes The audio recording formats supported by the device.
        * @property {ConfigurationData[]} supportedImageModes The recording image sizes and formats supported by the device.
        * @property {ConfigurationData[]} supportedVideoModes The recording video resolutions and formats supported by the device.
        */
        capture: {
            /**
            * Start the audio recorder application and return information about captured audio clip files
            * @param {Function} onSuccess Executes when the capture operation finishes with an array of MediaFile objects describing each captured audio clip file
            * @param {Function} onError Executes, if the user terminates the operation before an audio clip is captured, with a CaptureError object, featuring the CaptureError.CAPTURE_NO_MEDIA_FILES error code
            * @param {CaptureAudioOptions} [options] Encapsulates audio capture configuration options.
            *
            * @typedef {Object} CaptureAudioOptions
            *   @property {Number} limit The maximum number of audio clips the device user can record in a single capture operation. The value must be greater than or equal to 1 (defaults to 1).
            *   @property {Number} duration The maximum duration of an audio sound clip, in seconds.
            */
            captureAudio: function (onSuccess, onError, options) {
                intellisense.setCallContext(onSuccess, { thisArg: {}, args: [[]] });
                intellisense.setCallContext(onError, { thisArg: {}, args: [new CaptureError()] });
            },
            /**
            * Start the camera application and return information about captured image files
            * @param {Function} onSuccess Executes when the capture operation finishes with an array of MediaFile objects describing each captured audio clip file
            * @param {Function} onError Executes, if the user terminates the operation before an audio clip is captured, with a CaptureError object, featuring the CaptureError.CAPTURE_NO_MEDIA_FILES error code
            * @param {CaptureImageOptions} [options] Encapsulates image capture configuration options. 
            *
            * @typedef {Object} CaptureImageOptions
            *   @property {Number} limit The maximum number of images the user can capture in a single capture operation. The value must be greater than or equal to 1 (defaults to 1).
            */
            captureImage: function (onSuccess, onError, options) {
                intellisense.setCallContext(onSuccess, { thisArg: {}, args: [[]] });
                intellisense.setCallContext(onError, { thisArg: {}, args: [new CaptureError()] });
            },
            /**
            * Start the video recorder application and return information about captured video clip files
            * @param {Function} onSuccess Executes when the capture operation finishes with an array of MediaFile objects describing each captured audio clip file
            * @param {Function} onError Executes, if the user terminates the operation before an audio clip is captured, with a CaptureError object, featuring the CaptureError.CAPTURE_NO_MEDIA_FILES error code
            * @param {CaptureVideoOptions} [options] Encapsulates video capture configuration options.
            *
            * @typedef {Object} CaptureVideoOptions
            *   @property {Number} limit The maximum number of video clips the device's user can capture in a single capture operation. The value must be greater than or equal to 1 (defaults to 1).
            *   @property {Number} duration The maximum duration of a video clip, in seconds.
            */
            captureVideo: function (onSuccess, onError, options) {
                intellisense.setCallContext(onSuccess, { thisArg: {}, args: [[]] });
                intellisense.setCallContext(onError, { thisArg: {}, args: [new CaptureError()] });
            },
            supportedAudioModes: [],
            supportedImageModes: [],
            supportedVideoModes: [],
            __proto__: null
        },
        __proto__: null
    };
})();
/* Network Information */
(function (navigator) {
    /** 
    * The connection object, exposed via navigator.connection, provides information about the device's cellular and wifi connection.
    * @property {Number} type
    * This property offers a fast way to determine the device's network connection state, and type of connection.
    * One of:
    *     Connection.UNKNOWN
    *     Connection.ETHERNET
    *     Connection.WIFI
    *     Connection.CELL_2G
    *     Connection.CELL_3G
    *     Connection.CELL_4G
    *     Connection.CELL
    *     Connection.NONE
    */
    navigator.connection = {
        __proto__: null,
        type: number
    };

    window.Connection = {
        __proto__: null,
        UNKNOWN: 0,
        ETHERNET: 0,
        WIFI: 0,
        CELL_2G: 0,
        CELL_3G: 0,
        CELL_4G: 0,
        CELL: 0,
        NONE: 0,
    };
})(window.navigator = window.navigator || {});
/* Notification */
(function () {
    // TODO: Should we still show vibration APIs in here?
    window.navigator['notification'] = {
        /**
        * Vibrates the device for the specified amount of time
        * @param {Number} time Milliseconds to vibrate the device. Ignored on iOS
        */
        vibrate: function (time) {
        },
        /**
        * Vibrates the device with a given pattern.
        * @param {Number[]} pattern Pattern with which to vibrate the device.
        *                              The first value - number of milliseconds to wait before turning the vibrator on.
        *                         The next value - the number of milliseconds for which to keep the vibrator on before turning it off.
        * @param {Number} [repeat] Optional index into the pattern array at which to start repeating (will repeat until canceled),
        *                         or -1 for no repetition (default).
        */
        vibrateWithPattern: function (pattern, repeat) {
        },
        /**
        * Immediately cancels any currently running vibration.
        */
        cancelVibration: function () {
        },
        /**
        * Shows a custom alert or dialog box
        * @param {String} message Dialog message
        * @param {Function} alertCallback Callback to invoke when alert dialog is dismissed
        * @param {String} [title] Dialog title, defaults to 'Alert'
        * @param {String} [buttonName] Button name, defaults to OK
        */
        alert: function (message, alertCallback, title, buttonName) {
            intellisense.setCallContext(alertCallback, { thisArg: {}, args: [''] });
        },

        /**
        * Displays a customizable confirmation dialog box
        * @param {String} message Dialog message
        * @param {Function} confirmCallback Callback to invoke with index of button pressed (1, 2, or 3) or when the dialog is dismissed without a button press (0)
        * @param {String} [title] Dialog title, defaults to "Confirm"
        * @param {String[]} [buttonLabels] Array of strings specifying button labels, defaults to ["OK","Cancel"]
        */
        confirm: function (message, confirmCallback, title, buttonLabels) {
            intellisense.setCallContext(confirmCallback, { thisArg: {}, args: [''] });
        },
        /**
        * Displays a native dialog box that is more customizable than the browser's prompt function
        * @param {String} message Dialog message
        * @param {Function} promptCallback Callback to invoke when a button is pressed
        * @param {String} [title] Dialog title, defaults to "Prompt"
        * @param {String[]} [buttonLabels] Array of strings specifying button labels, defaults to ["OK","Cancel"]
        * @param {String} [defaultText] Default textbox input value, default: ""
        */
        prompt: function (message, promptCallback, title, buttonLabels, defaultText) {
            var promptCallbackValue = {
                // The index of the pressed button. Note that the index uses one-based indexing, so the value is  1,  2,  3, etc. 0 is the result when the dialog is dismissed without a button press.
                buttonIndex: 0,
                // The text entered in the prompt dialog box
                input1: ''
            };
            intellisense.setCallContext(promptCallback, { thisArg: {}, args: [promptCallbackValue] });
        },
        /**
        * The device plays a beep sound
        * @param {Number} times The number of times to repeat the beep
        */
        beep: function (times) {
        },
        __proto__: null
    };
})();
/* Splashscreen */
window.navigator['splashscreen'] = {
    /**
    * Dismiss the splash screen
    */
    hide: function () {
    },
    /**
    * Displays the splash screen
    */
    show: function () {
    },
    __proto__: null
};
// WebSQL
var openDatabase;
(function () {
    var Database = function () {
    };
    Database.prototype.transaction = function () {
    };
    Database.prototype.readTransaction = function () {
    };
    Database.prototype.__proto__ = null;
    delete Database.prototype.constructor;

    /**
    * Creates (opens, if exist) database with supplied parameters.
    * @param {String} name Database name
    * @param {String} version Database version
    * @param {String} displayname Database display name
    * @param {Number} size Size, in bytes
    * @param {Function} [creationCallback] Callback, executed on database creation. Accepts Database object
    */
    var openDatabase = function (name, version, displayname, size, creationCallback) {
        intellisense.setCallContext(creationCallback, { thisArg: {}, args: [''] });

        return new Database();
    };

    var Database = (function () {
        function Database() {
        }

        /**
        * Starts new transaction
        * @param {Function} callback Function, that will be called when transaction starts
        * @param {Function} [errorCallback] Called, when Transaction fails
        * @param {Function} [successCallback] Called, when transaction committed
        */
        Database.prototype.transaction = function (callback, errorCallback, successCallback) {
            intellisense.setCallContext(callback, { thisArg: {}, args: [new SqlTransaction()] });
            intellisense.setCallContext(errorCallback, { thisArg: {}, args: [SqlError] });
            intellisense.setCallContext(successCallback, { thisArg: {}, args: [''] });
        };

        Database.prototype.__proto__ = null;
        delete Database.prototype.constructor;

        return Database;
    })();

    var SqlTransaction = (function () {
        function SqlTransaction() {
        }
        /**
        * Executes SQL statement via current transaction
        * @param {} sql SQL statement to execute
        * @param {} [arguments] SQL stetement arguments
        * @param {Function} [successCallback] Called in case of query has been successfully done
        * @param {Function} [errorCallback] Called, when query fails
        */
        SqlTransaction.prototype.executeSql = function (sql, arguments, successCallback, errorCallback) {
            intellisense.setCallContext(successCallback, { thisArg: {}, args: [new SqlTransaction(), SqlResultSet] });
            intellisense.setCallContext(errorCallback, { thisArg: {}, args: [new SqlTransaction(), SqlError] });
        };

        SqlTransaction.prototype.__proto__ = null;
        delete SqlTransaction.prototype.constructor;

        return SqlTransaction;
    })();

    /**
    * @property {Number} code Error code constants from http://www.w3.org/TR/webdatabase/#sqlerror
    * @property {String} message Error message
    */
    var SqlError = {
        code: 0,
        message: '',
        __proto__: null
    };

    /**
    * @property {Number} insertId Row ID of the inserted SQL statement
    * @property {Object} rowsAffected Number of rows that were changed by the SQL statement
    * @property {Object} rows Rows returned by the database
    */
    var SqlResultSet = {
        insertId: 0,
        rowsAffected: 0,
        /**
        * @property {String} length Number of rows returned by the database
        * @property {String} message Result message
        */
        rows: {
            length: 0,
            message: '',
            __proto__: null
        },
        __proto__: null
    };

    window["openDatabase"] = openDatabase;
})();