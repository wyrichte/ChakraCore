// Copyright (c) Microsoft Corporation
// All rights reserved

/// <reference path="../base/_es3.js" />
/// <reference path="../base/base.js" />

(function (undefined) {
    function createXHR() {
        if (window.location.protocol !== "file:") {
            return new XMLHttpRequest();
        }
        else {
            return new ActiveXObject("Microsoft.XMLHTTP");
        }
    }

    Win.Namespace.define("Win", {
        xhr: function (options) {
            var req = createXHR();
            req.onreadystatechange = function () {
                if (options.readystatechange) {
                    options.readystatechange(req);
                }
                if (req.readyState == 4) {
                    if (req.status >= 200 && req.status < 300) {
                        if (options.success) {
                            options.success(req);
                        }
                    }
                    else {
                        if (options.error) {
                            options.error(req);
                        }
                    }
                }
            };
            req.open(options.type || "GET", options.url, options.async || true, options.user, options.password);
            if (options.headers) {
                Object.keys(options.headers).forEach(function (k) {
                    req.setRequestHeader(k, options.headers[k]);
                });
            }
            req.send(options.data);
            return { abort: function () { req.abort(); } };
        }
    });
})();