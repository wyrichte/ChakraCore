(function () { this.window = this; })();
window.document = { 
    createElement: function () { 
        return {
           getElementsByTagName: function() { return []; },
           appendChild: function() { },
           style:{}
        };
    },
    getElementById: function() { },
    createComment: function() { },
    documentElement: { childNodes: [], insertBefore: function() { }, removeChild: function() { } } 
};
window.navigator = { userAgent: "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; WOW64; Trident/5.0; SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; .NET CLR 3.0.30729; .NET4.0C; .NET4.0E; MS-RTC LM 8; InfoPath.3; Override:IE9_DEFAULT_20091014)" };
window.location = {
    hash: "",
    host: "",
    hostname: "",
    href: "about:blank",
    pathname: "/blank",
    port: "",
    protocol: "about:",
    search: "",
    toString: function () { return this.href; }
};
navigator = window.navigator;