/*!
  trblib core
      Required for all trblib plugins. Contains sub-libraries require, run, util, fn, and plugins.
          
              trblib require
                    Loads JavaScript / CSS files dynamically.
                        
                            trblib run
                                  Event runner consolidating multiple events with a common handler. Allows custom events.
                                      
                                          trlib util
                                                Various utility methods.
                                                    
                                                        trblib fn
                                                              Public API for code common to a distributed sub-library (e.g. plugins).
                                                                    
                                                                        trblib plugins
                                                                              Distributed, dynamically loaded plugin system using evented roles in a publish / subscribe 
                                                                                    pattern with implictly invoked lazy instantiation. Actual plugins defined outside trblib
                                                                                          core. All instances of a plugin use same runner and router.
                                                                                                
                                                                                                      To register (load & enable) a plugin, call trblib.fn.plugins.register(pluginName).
                                                                                                        
                                                                                                          Author: Dan Krecichwost
                                                                                                          */

                                                                                                          (function(window, $) {
                                                                                                              var document = window.document,
                                                                                                                  trblib = window.trblib = window.trblib || {},
                                                                                                                        old,
                                                                                                                              root = $.root = $(document),
                                                                                                                                    find = root.find,
                                                                                                                                          urlMap = {},
                                                                                                                                                rAttributeCleanup = /[\n\t\r]/g;
                                                                                                                                                  
                                                                                                              // temporary - need correct version of jQuery
                                                                                                              trblib.jQuery = $;
                                                                                                                
                                                                                                              // iepp v1.6.2 MIT @jon_neal - http://www.iecss.com/print-protector/
                                                                                                              /*@cc_on(function(k,o){var a="abbr|article|aside|audio|canvas|details|figcaption|figure|footer|header|hgroup|mark|meter|nav|output|progress|section|summary|time|video",f=a.split("|"),d=f.length,b=new RegExp("(^|\\s)("+a+")","gi"),h=new RegExp("<(/*)("+a+")","gi"),m=new RegExp("(^|[^\\n]*?\\s)("+a+")([^\\n]*)({[\\n\\w\\W]*?})","gi"),p=o.createDocumentFragment(),i=o.documentElement,n=i.firstChild,c=o.createElement("body"),g=o.createElement("style"),j;function e(r){var q=-1;while(++q<d){r.createElement(f[q])}}function l(u,s){var r=-1,q=u.length,v,t=[];while(++r<q){v=u[r];if((s=v.media||s)!="screen"){t.push(l(v.imports,s),v.cssText)}}return t.join("")}e(o);e(p);n.insertBefore(g,n.firstChild);g.media="print";k.attachEvent("onbeforeprint",function(){var r=-1,u=l(o.styleSheets,"all"),t=[],w;j=j||o.body;while((w=m.exec(u))!=null){t.push((w[1]+w[2]+w[3]).replace(b,"$1.iepp_$2")+w[4])}g.styleSheet.cssText=t.join("\n");while(++r<d){var s=o.getElementsByTagName(f[r]),v=s.length,q=-1;while(++q<v){if(s[q].className.indexOf("iepp_")<0){s[q].className+=" iepp_"+f[r]}}}p.appendChild(j);i.appendChild(c);c.className=j.className;c.innerHTML=j.innerHTML.replace(h,"<$1font")});k.attachEvent("onafterprint",function(){c.innerHTML="";i.removeChild(c);i.appendChild(j);g.styleSheet.cssText=""})})(this,document);@*/
                                                                                                                      
                                                                                                              trblib.util = $.extend({
                                                                                                                  // TODO add adler32, sharding, and base URL concepts to trblib.require
                                                                                                                  adler32: function adler32(s) {
                                                                                                                      var s1 = 1, s2 = 0, b, i = -1;
                                                                                                                      while ((b = s.charCodeAt(++i))) {
                                                                                                                          s1 = (s1 + b) % 65521;
                                                                                                                          s2 = (s2 + s1) % 65521;
                                                                                                                      }
                                                                                                                      return s2 << 16 | s1;
                                                                                                                  },
                                                                                                                  inject: function inject(t, o) {
                                                                                                                      for (var x in o) if (!t[x] && o.hasOwnProperty(x)) t[x] = o[x];
                                                                                                                  },
                                                                                                                  absolutizeUrl: function absolutizeUrl(url) {
                                                                                                                      var absoluteUrl;
                                                                                                                      if ((absoluteUrl = urlMap[url])) return absoluteUrl;
                                                                                                                                          
                                                                                                                      absoluteUrl = document.createElement('div');
                                                                                                                      absoluteUrl.innerHTML = '<a href="' + url + '">d</a>';
                                                                                                                      return (urlMap[url] = absoluteUrl.firstChild.href);
                                                                                                                  },
                                                                                                                  expandQueryString: function expandQueryString(queryString) {
                                                                                                                      var params = {},
                                                                                                                                              qsArray = queryString.split('&'),
                                                                                                                                                        param,
                                                                                                                                                                  paramName,
                                                                                                                                                                            paramValue,
                                                                                                                                                                                      eqIndex,
                                                                                                                                                                                                hasEq,
                                                                                                                                                                                                          i = -1,
                                                                                                                                                                                                                    qsArrayLength = qsArray.length;

                                                                                                                      while (i++ < qsArrayLength) {
                                                                                                                          if ((param = qsArray[i])) {
                                                                                                                              eqIndex = param.indexOf('=');
                                                                                                                              hasEq = eqIndex > -1;
                                                                                                                              params[param.substring(0, hasEq ? eqIndex : param.length)] = hasEq ? param.substring(eqIndex + 1, param.length) : true;
                                                                                                                          }
                                                                                                                      }
                                                                                                                                          
                                                                                                                      return params;
                                                                                                                  }
                                                                                                              }, trblib.util);
                                                                                                                
                                                                                                              old = trblib.require;
                                                                                                                trblib.require = (function require() {
                                                                                                                    
                                                                                                                        var a = arguments,
                                                                                                                                anchor_ = function() {
                                                                                                                                    fs = document.getElementsByTagName('script')[0];
                                                                                                                                    pn = fs.parentNode;
                                                                                                                                },
                                                                                                                                        fs,
                                                                                                                                                pn,
                                                                                                                                                        loadAsync = document.createElement('script').async === true || 'MozAppearance' in document.documentElement.style || window.opera,
                                                                                                                                                                require = $.extend(function require() {

                                                                                                                                                                    m