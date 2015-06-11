using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

using Microsoft.BPT.Tests.DirectAuthor;
using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DirectAuthorTests
{
    [TestClass]
    public class CorsicaApplications: CompletionsBase
    {
        [TestMethod]
        [WorkItem(151114)]
        public void BingVideos()
        {
            const string htmlScript = @"
        var videos = [{""test"": {""$"": ""test $1"" }}, {""test"": {""$"": ""test $2""}}, {""test"": {""$"": ""test $3""}}];

        var videoList;
        $(document).ready(function() {
            videoList = Win.UI.Controls.List|c|View($(""#items"").get(0), {
                layout: ""verticalgrid"",
                justified: true,
                dataSource: videos,
                mode: ""singleselection"",
                itemRenderer: function (renderInfo, key, dataObject, itemID) {
                    return $(""#t-videoContent"").tmpl(dataObject).get(0);
                }
            });

            videoList.|v|

            videoList.addEventListener(""selectionchanged"", function (event) {
                var video = videos[videoList.selection()].test.$;
                alert(video);
            });
        });

/* BEGIN EXTERNAL SOURCE */

        <div style=""background-color: Red;"" class=""context"">
            <span>${test.$}</span>
        </div>
    
/* END EXTERNAL SOURCE */
";


            PerformRequests(htmlScript, (context, offset, data, index) =>
            {
                var result = context.GetCompletionsAt(offset);
                switch (data)
                {
                    case "v": result.ToEnumerable().ExpectContains(new[] { "addEventListener" }); break;
                    case "c": result.ToEnumerable().ExpectContains(new[] { "ListView" }); break;
                }
            }, domSim_js, jquery_1_4_2_min_js, jquery_tmpl_min_js, base_js, ui_js, win8ui_js, wwaapp_js, xhr_js);
        }

        #region domSim.js
        const string domSim_js = @"(function () { this.window = this; })();
window.document = { 
    createElement: function () { 
        return {
           getElementsByTagName: function() { return []; },
           appendChild: function() { },
           removeChild: function() { this.hasChildNodes = function () { return false; } },
           style:{}
        };
    },
    getElementById: function() { },
    createComment: function() { },
    documentElement: { childNodes: [], insertBefore: function() { }, removeChild: function() { this.hasChildNodes = function () { return false; } } } 
};
window.navigator = { userAgent: ""Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; WOW64; Trident/5.0; SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; .NET CLR 3.0.30729; .NET4.0C; .NET4.0E; MS-RTC LM 8; InfoPath.3; Override:IE9_DEFAULT_20091014)"" };
window.location = {
    hash: """",
    host: """",
    hostname: """",
    href: ""about:blank"",
    pathname: ""/blank"",
    port: """",
    protocol: ""about:"",
    search: """",
    toString: function () { return this.href; }
};
navigator = window.navigator;";
        #endregion
        #region jQuery
        const string jquery_1_4_2_min_js = @"/*!
 * jQuery JavaScript Library v1.4.2
 * http://jquery.com/
 *
 * Copyright 2010, John Resig
 * Dual licensed under the MIT or GPL Version 2 licenses.
 * http://jquery.org/license
 *
 * Includes Sizzle.js
 * http://sizzlejs.com/
 * Copyright 2010, The Dojo Foundation
 * Released under the MIT, BSD, and GPL Licenses.
 *
 * Date: Sat Feb 13 22:33:48 2010 -0500
 */
(function(A,w){function ma(){if(!c.isReady){try{s.documentElement.doScroll(""left"")}catch(a){setTimeout(ma,1);return}c.ready()}}function Qa(a,b){b.src?c.ajax({url:b.src,async:false,dataType:""script""}):c.globalEval(b.text||b.textContent||b.innerHTML||"""");b.parentNode&&b.parentNode.removeChild(b)}function X(a,b,d,f,e,j){var i=a.length;if(typeof b===""object""){for(var o in b)X(a,o,b[o],f,e,d);return a}if(d!==w){f=!j&&f&&c.isFunction(d);for(o=0;o<i;o++)e(a[o],b,f?d.call(a[o],o,e(a[o],b)):d,j);return a}return i?
e(a[0],b):w}function J(){return(new Date).getTime()}function Y(){return false}function Z(){return true}function na(a,b,d){d[0].type=a;return c.event.handle.apply(b,d)}function oa(a){var b,d=[],f=[],e=arguments,j,i,o,k,n,r;i=c.data(this,""events"");if(!(a.liveFired===this||!i||!i.live||a.button&&a.type===""click"")){a.liveFired=this;var u=i.live.slice(0);for(k=0;k<u.length;k++){i=u[k];i.origType.replace(O,"""")===a.type?f.push(i.selector):u.splice(k--,1)}j=c(a.target).closest(f,a.currentTarget);n=0;for(r=
j.length;n<r;n++)for(k=0;k<u.length;k++){i=u[k];if(j[n].selector===i.selector){o=j[n].elem;f=null;if(i.preType===""mouseenter""||i.preType===""mouseleave"")f=c(a.relatedTarget).closest(i.selector)[0];if(!f||f!==o)d.push({elem:o,handleObj:i})}}n=0;for(r=d.length;n<r;n++){j=d[n];a.currentTarget=j.elem;a.data=j.handleObj.data;a.handleObj=j.handleObj;if(j.handleObj.origHandler.apply(j.elem,e)===false){b=false;break}}return b}}function pa(a,b){return""live.""+(a&&a!==""*""?a+""."":"""")+b.replace(/\./g,""`"").replace(/ /g,
""&"")}function qa(a){return!a||!a.parentNode||a.parentNode.nodeType===11}function ra(a,b){var d=0;b.each(function(){if(this.nodeName===(a[d]&&a[d].nodeName)){var f=c.data(a[d++]),e=c.data(this,f);if(f=f&&f.events){delete e.handle;e.events={};for(var j in f)for(var i in f[j])c.event.add(this,j,f[j][i],f[j][i].data)}}})}function sa(a,b,d){var f,e,j;b=b&&b[0]?b[0].ownerDocument||b[0]:s;if(a.length===1&&typeof a[0]===""string""&&a[0].length<512&&b===s&&!ta.test(a[0])&&(c.support.checkClone||!ua.test(a[0]))){e=
true;if(j=c.fragments[a[0]])if(j!==1)f=j}if(!f){f=b.createDocumentFragment();c.clean(a,b,f,d)}if(e)c.fragments[a[0]]=j?f:1;return{fragment:f,cacheable:e}}function K(a,b){var d={};c.each(va.concat.apply([],va.slice(0,b)),function(){d[this]=a});return d}function wa(a){return""scrollTo""in a&&a.document?a:a.nodeType===9?a.defaultView||a.parentWindow:false}var c=function(a,b){return new c.fn.init(a,b)},Ra=A.jQuery,Sa=A.$,s=A.document,T,Ta=/^[^<]*(<[\w\W]+>)[^>]*$|^#([\w-]+)$/,Ua=/^.[^:#\[\.,]*$/,Va=/\S/,
Wa=/^(\s|\u00A0)+|(\s|\u00A0)+$/g,Xa=/^<(\w+)\s*\/?>(?:<\/\1>)?$/,P=navigator.userAgent,xa=false,Q=[],L,$=Object.prototype.toString,aa=Object.prototype.hasOwnProperty,ba=Array.prototype.push,R=Array.prototype.slice,ya=Array.prototype.indexOf;c.fn=c.prototype={init:function(a,b){var d,f;if(!a)return this;if(a.nodeType){this.context=this[0]=a;this.length=1;return this}if(a===""body""&&!b){this.context=s;this[0]=s.body;this.selector=""body"";this.length=1;return this}if(typeof a===""string"")if((d=Ta.exec(a))&&
(d[1]||!b))if(d[1]){f=b?b.ownerDocument||b:s;if(a=Xa.exec(a))if(c.isPlainObject(b)){a=[s.createElement(a[1])];c.fn.attr.call(a,b,true)}else a=[f.createElement(a[1])];else{a=sa([d[1]],[f]);a=(a.cacheable?a.fragment.cloneNode(true):a.fragment).childNodes}return c.merge(this,a)}else{if(b=s.getElementById(d[2])){if(b.id!==d[2])return T.find(a);this.length=1;this[0]=b}this.context=s;this.selector=a;return this}else if(!b&&/^\w+$/.test(a)){this.selector=a;this.context=s;a=s.getElementsByTagName(a);return c.merge(this,
a)}else return!b||b.jquery?(b||T).find(a):c(b).find(a);else if(c.isFunction(a))return T.ready(a);if(a.selector!==w){this.selector=a.selector;this.context=a.context}return c.makeArray(a,this)},selector:"""",jquery:""1.4.2"",length:0,size:function(){return this.length},toArray:function(){return R.call(this,0)},get:function(a){return a==null?this.toArray():a<0?this.slice(a)[0]:this[a]},pushStack:function(a,b,d){var f=c();c.isArray(a)?ba.apply(f,a):c.merge(f,a);f.prevObject=this;f.context=this.context;if(b===
""find"")f.selector=this.selector+(this.selector?"" "":"""")+d;else if(b)f.selector=this.selector+"".""+b+""(""+d+"")"";return f},each:function(a,b){return c.each(this,a,b)},ready:function(a){c.bindReady();if(c.isReady)a.call(s,c);else Q&&Q.push(a);return this},eq:function(a){return a===-1?this.slice(a):this.slice(a,+a+1)},first:function(){return this.eq(0)},last:function(){return this.eq(-1)},slice:function(){return this.pushStack(R.apply(this,arguments),""slice"",R.call(arguments).join("",""))},map:function(a){return this.pushStack(c.map(this,
function(b,d){return a.call(b,d,b)}))},end:function(){return this.prevObject||c(null)},push:ba,sort:[].sort,splice:[].splice};c.fn.init.prototype=c.fn;c.extend=c.fn.extend=function(){var a=arguments[0]||{},b=1,d=arguments.length,f=false,e,j,i,o;if(typeof a===""boolean""){f=a;a=arguments[1]||{};b=2}if(typeof a!==""object""&&!c.isFunction(a))a={};if(d===b){a=this;--b}for(;b<d;b++)if((e=arguments[b])!=null)for(j in e){i=a[j];o=e[j];if(a!==o)if(f&&o&&(c.isPlainObject(o)||c.isArray(o))){i=i&&(c.isPlainObject(i)||
c.isArray(i))?i:c.isArray(o)?[]:{};a[j]=c.extend(f,i,o)}else if(o!==w)a[j]=o}return a};c.extend({noConflict:function(a){A.$=Sa;if(a)A.jQuery=Ra;return c},isReady:false,ready:function(){if(!c.isReady){if(!s.body)return setTimeout(c.ready,13);c.isReady=true;if(Q){for(var a,b=0;a=Q[b++];)a.call(s,c);Q=null}c.fn.triggerHandler&&c(s).triggerHandler(""ready"")}},bindReady:function(){if(!xa){xa=true;if(s.readyState===""complete"")return c.ready();if(s.addEventListener){s.addEventListener(""DOMContentLoaded"",
L,false);A.addEventListener(""load"",c.ready,false)}else if(s.attachEvent){s.attachEvent(""onreadystatechange"",L);A.attachEvent(""onload"",c.ready);var a=false;try{a=A.frameElement==null}catch(b){}s.documentElement.doScroll&&a&&ma()}}},isFunction:function(a){return $.call(a)===""[object Function]""},isArray:function(a){return $.call(a)===""[object Array]""},isPlainObject:function(a){if(!a||$.call(a)!==""[object Object]""||a.nodeType||a.setInterval)return false;if(a.constructor&&!aa.call(a,""constructor"")&&!aa.call(a.constructor.prototype,
""isPrototypeOf""))return false;var b;for(b in a);return b===w||aa.call(a,b)},isEmptyObject:function(a){for(var b in a)return false;return true},error:function(a){throw a;},parseJSON:function(a){if(typeof a!==""string""||!a)return null;a=c.trim(a);if(/^[\],:{}\s]*$/.test(a.replace(/\\(?:[""\\\/bfnrt]|u[0-9a-fA-F]{4})/g,""@"").replace(/""[^""\\\n\r]*""|true|false|null|-?\d+(?:\.\d*)?(?:[eE][+\-]?\d+)?/g,""]"").replace(/(?:^|:|,)(?:\s*\[)+/g,"""")))return A.JSON&&A.JSON.parse?A.JSON.parse(a):(new Function(""return ""+
a))();else c.error(""Invalid JSON: ""+a)},noop:function(){},globalEval:function(a){if(a&&Va.test(a)){var b=s.getElementsByTagName(""head"")[0]||s.documentElement,d=s.createElement(""script"");d.type=""text/javascript"";if(c.support.scriptEval)d.appendChild(s.createTextNode(a));else d.text=a;b.insertBefore(d,b.firstChild);b.removeChild(d)}},nodeName:function(a,b){return a.nodeName&&a.nodeName.toUpperCase()===b.toUpperCase()},each:function(a,b,d){var f,e=0,j=a.length,i=j===w||c.isFunction(a);if(d)if(i)for(f in a){if(b.apply(a[f],
d)===false)break}else for(;e<j;){if(b.apply(a[e++],d)===false)break}else if(i)for(f in a){if(b.call(a[f],f,a[f])===false)break}else for(d=a[0];e<j&&b.call(d,e,d)!==false;d=a[++e]);return a},trim:function(a){return(a||"""").replace(Wa,"""")},makeArray:function(a,b){b=b||[];if(a!=null)a.length==null||typeof a===""string""||c.isFunction(a)||typeof a!==""function""&&a.setInterval?ba.call(b,a):c.merge(b,a);return b},inArray:function(a,b){if(b.indexOf)return b.indexOf(a);for(var d=0,f=b.length;d<f;d++)if(b[d]===
a)return d;return-1},merge:function(a,b){var d=a.length,f=0;if(typeof b.length===""number"")for(var e=b.length;f<e;f++)a[d++]=b[f];else for(;b[f]!==w;)a[d++]=b[f++];a.length=d;return a},grep:function(a,b,d){for(var f=[],e=0,j=a.length;e<j;e++)!d!==!b(a[e],e)&&f.push(a[e]);return f},map:function(a,b,d){for(var f=[],e,j=0,i=a.length;j<i;j++){e=b(a[j],j,d);if(e!=null)f[f.length]=e}return f.concat.apply([],f)},guid:1,proxy:function(a,b,d){if(arguments.length===2)if(typeof b===""string""){d=a;a=d[b];b=w}else if(b&&
!c.isFunction(b)){d=b;b=w}if(!b&&a)b=function(){return a.apply(d||this,arguments)};if(a)b.guid=a.guid=a.guid||b.guid||c.guid++;return b},uaMatch:function(a){a=a.toLowerCase();a=/(webkit)[ \/]([\w.]+)/.exec(a)||/(opera)(?:.*version)?[ \/]([\w.]+)/.exec(a)||/(msie) ([\w.]+)/.exec(a)||!/compatible/.test(a)&&/(mozilla)(?:.*? rv:([\w.]+))?/.exec(a)||[];return{browser:a[1]||"""",version:a[2]||""0""}},browser:{}});P=c.uaMatch(P);if(P.browser){c.browser[P.browser]=true;c.browser.version=P.version}if(c.browser.webkit)c.browser.safari=
true;if(ya)c.inArray=function(a,b){return ya.call(b,a)};T=c(s);if(s.addEventListener)L=function(){s.removeEventListener(""DOMContentLoaded"",L,false);c.ready()};else if(s.attachEvent)L=function(){if(s.readyState===""complete""){s.detachEvent(""onreadystatechange"",L);c.ready()}};(function(){c.support={};var a=s.documentElement,b=s.createElement(""script""),d=s.createElement(""div""),f=""script""+J();d.style.display=""none"";d.innerHTML=""   <link/><table></table><a href='/a' style='color:red;float:left;opacity:.55;'>a</a><input type='checkbox'/>"";
var e=d.getElementsByTagName(""*""),j=d.getElementsByTagName(""a"")[0];if(!(!e||!e.length||!j)){c.support={leadingWhitespace:d.firstChild.nodeType===3,tbody:!d.getElementsByTagName(""tbody"").length,htmlSerialize:!!d.getElementsByTagName(""link"").length,style:/red/.test(j.getAttribute(""style"")),hrefNormalized:j.getAttribute(""href"")===""/a"",opacity:/^0.55$/.test(j.style.opacity),cssFloat:!!j.style.cssFloat,checkOn:d.getElementsByTagName(""input"")[0].value===""on"",optSelected:s.createElement(""select"").appendChild(s.createElement(""option"")).selected,
parentNode:d.removeChild(d.appendChild(s.createElement(""div""))).parentNode===null,deleteExpando:true,checkClone:false,scriptEval:false,noCloneEvent:true,boxModel:null};b.type=""text/javascript"";try{b.appendChild(s.createTextNode(""window.""+f+""=1;""))}catch(i){}a.insertBefore(b,a.firstChild);if(A[f]){c.support.scriptEval=true;delete A[f]}try{delete b.test}catch(o){c.support.deleteExpando=false}a.removeChild(b);if(d.attachEvent&&d.fireEvent){d.attachEvent(""onclick"",function k(){c.support.noCloneEvent=
false;d.detachEvent(""onclick"",k)});d.cloneNode(true).fireEvent(""onclick"")}d=s.createElement(""div"");d.innerHTML=""<input type='radio' name='radiotest' checked='checked'/>"";a=s.createDocumentFragment();a.appendChild(d.firstChild);c.support.checkClone=a.cloneNode(true).cloneNode(true).lastChild.checked;c(function(){var k=s.createElement(""div"");k.style.width=k.style.paddingLeft=""1px"";s.body.appendChild(k);c.boxModel=c.support.boxModel=k.offsetWidth===2;s.body.removeChild(k).style.display=""none""});a=function(k){var n=
s.createElement(""div"");k=""on""+k;var r=k in n;if(!r){n.setAttribute(k,""return;"");r=typeof n[k]===""function""}return r};c.support.submitBubbles=a(""submit"");c.support.changeBubbles=a(""change"");a=b=d=e=j=null}})();c.props={""for"":""htmlFor"",""class"":""className"",readonly:""readOnly"",maxlength:""maxLength"",cellspacing:""cellSpacing"",rowspan:""rowSpan"",colspan:""colSpan"",tabindex:""tabIndex"",usemap:""useMap"",frameborder:""frameBorder""};var G=""jQuery""+J(),Ya=0,za={};c.extend({cache:{},expando:G,noData:{embed:true,object:true,
applet:true},data:function(a,b,d){if(!(a.nodeName&&c.noData[a.nodeName.toLowerCase()])){a=a==A?za:a;var f=a[G],e=c.cache;if(!f&&typeof b===""string""&&d===w)return null;f||(f=++Ya);if(typeof b===""object""){a[G]=f;e[f]=c.extend(true,{},b)}else if(!e[f]){a[G]=f;e[f]={}}a=e[f];if(d!==w)a[b]=d;return typeof b===""string""?a[b]:a}},removeData:function(a,b){if(!(a.nodeName&&c.noData[a.nodeName.toLowerCase()])){a=a==A?za:a;var d=a[G],f=c.cache,e=f[d];if(b){if(e){delete e[b];c.isEmptyObject(e)&&c.removeData(a)}}else{if(c.support.deleteExpando)delete a[c.expando];
else a.removeAttribute&&a.removeAttribute(c.expando);delete f[d]}}}});c.fn.extend({data:function(a,b){if(typeof a===""undefined""&&this.length)return c.data(this[0]);else if(typeof a===""object"")return this.each(function(){c.data(this,a)});var d=a.split(""."");d[1]=d[1]?"".""+d[1]:"""";if(b===w){var f=this.triggerHandler(""getData""+d[1]+""!"",[d[0]]);if(f===w&&this.length)f=c.data(this[0],a);return f===w&&d[1]?this.data(d[0]):f}else return this.trigger(""setData""+d[1]+""!"",[d[0],b]).each(function(){c.data(this,
a,b)})},removeData:function(a){return this.each(function(){c.removeData(this,a)})}});c.extend({queue:function(a,b,d){if(a){b=(b||""fx"")+""queue"";var f=c.data(a,b);if(!d)return f||[];if(!f||c.isArray(d))f=c.data(a,b,c.makeArray(d));else f.push(d);return f}},dequeue:function(a,b){b=b||""fx"";var d=c.queue(a,b),f=d.shift();if(f===""inprogress"")f=d.shift();if(f){b===""fx""&&d.unshift(""inprogress"");f.call(a,function(){c.dequeue(a,b)})}}});c.fn.extend({queue:function(a,b){if(typeof a!==""string""){b=a;a=""fx""}if(b===
w)return c.queue(this[0],a);return this.each(function(){var d=c.queue(this,a,b);a===""fx""&&d[0]!==""inprogress""&&c.dequeue(this,a)})},dequeue:function(a){return this.each(function(){c.dequeue(this,a)})},delay:function(a,b){a=c.fx?c.fx.speeds[a]||a:a;b=b||""fx"";return this.queue(b,function(){var d=this;setTimeout(function(){c.dequeue(d,b)},a)})},clearQueue:function(a){return this.queue(a||""fx"",[])}});var Aa=/[\n\t]/g,ca=/\s+/,Za=/\r/g,$a=/href|src|style/,ab=/(button|input)/i,bb=/(button|input|object|select|textarea)/i,
cb=/^(a|area)$/i,Ba=/radio|checkbox/;c.fn.extend({attr:function(a,b){return X(this,a,b,true,c.attr)},removeAttr:function(a){return this.each(function(){c.attr(this,a,"""");this.nodeType===1&&this.removeAttribute(a)})},addClass:function(a){if(c.isFunction(a))return this.each(function(n){var r=c(this);r.addClass(a.call(this,n,r.attr(""class"")))});if(a&&typeof a===""string"")for(var b=(a||"""").split(ca),d=0,f=this.length;d<f;d++){var e=this[d];if(e.nodeType===1)if(e.className){for(var j="" ""+e.className+"" "",
i=e.className,o=0,k=b.length;o<k;o++)if(j.indexOf("" ""+b[o]+"" "")<0)i+="" ""+b[o];e.className=c.trim(i)}else e.className=a}return this},removeClass:function(a){if(c.isFunction(a))return this.each(function(k){var n=c(this);n.removeClass(a.call(this,k,n.attr(""class"")))});if(a&&typeof a===""string""||a===w)for(var b=(a||"""").split(ca),d=0,f=this.length;d<f;d++){var e=this[d];if(e.nodeType===1&&e.className)if(a){for(var j=("" ""+e.className+"" "").replace(Aa,"" ""),i=0,o=b.length;i<o;i++)j=j.replace("" ""+b[i]+"" "",
"" "");e.className=c.trim(j)}else e.className=""""}return this},toggleClass:function(a,b){var d=typeof a,f=typeof b===""boolean"";if(c.isFunction(a))return this.each(function(e){var j=c(this);j.toggleClass(a.call(this,e,j.attr(""class""),b),b)});return this.each(function(){if(d===""string"")for(var e,j=0,i=c(this),o=b,k=a.split(ca);e=k[j++];){o=f?o:!i.hasClass(e);i[o?""addClass"":""removeClass""](e)}else if(d===""undefined""||d===""boolean""){this.className&&c.data(this,""__className__"",this.className);this.className=
this.className||a===false?"""":c.data(this,""__className__"")||""""}})},hasClass:function(a){a="" ""+a+"" "";for(var b=0,d=this.length;b<d;b++)if(("" ""+this[b].className+"" "").replace(Aa,"" "").indexOf(a)>-1)return true;return false},val:function(a){if(a===w){var b=this[0];if(b){if(c.nodeName(b,""option""))return(b.attributes.value||{}).specified?b.value:b.text;if(c.nodeName(b,""select"")){var d=b.selectedIndex,f=[],e=b.options;b=b.type===""select-one"";if(d<0)return null;var j=b?d:0;for(d=b?d+1:e.length;j<d;j++){var i=
e[j];if(i.selected){a=c(i).val();if(b)return a;f.push(a)}}return f}if(Ba.test(b.type)&&!c.support.checkOn)return b.getAttribute(""value"")===null?""on"":b.value;return(b.value||"""").replace(Za,"""")}return w}var o=c.isFunction(a);return this.each(function(k){var n=c(this),r=a;if(this.nodeType===1){if(o)r=a.call(this,k,n.val());if(typeof r===""number"")r+="""";if(c.isArray(r)&&Ba.test(this.type))this.checked=c.inArray(n.val(),r)>=0;else if(c.nodeName(this,""select"")){var u=c.makeArray(r);c(""option"",this).each(function(){this.selected=
c.inArray(c(this).val(),u)>=0});if(!u.length)this.selectedIndex=-1}else this.value=r}})}});c.extend({attrFn:{val:true,css:true,html:true,text:true,data:true,width:true,height:true,offset:true},attr:function(a,b,d,f){if(!a||a.nodeType===3||a.nodeType===8)return w;if(f&&b in c.attrFn)return c(a)[b](d);f=a.nodeType!==1||!c.isXMLDoc(a);var e=d!==w;b=f&&c.props[b]||b;if(a.nodeType===1){var j=$a.test(b);if(b in a&&f&&!j){if(e){b===""type""&&ab.test(a.nodeName)&&a.parentNode&&c.error(""type property can't be changed"");
a[b]=d}if(c.nodeName(a,""form"")&&a.getAttributeNode(b))return a.getAttributeNode(b).nodeValue;if(b===""tabIndex"")return(b=a.getAttributeNode(""tabIndex""))&&b.specified?b.value:bb.test(a.nodeName)||cb.test(a.nodeName)&&a.href?0:w;return a[b]}if(!c.support.style&&f&&b===""style""){if(e)a.style.cssText=""""+d;return a.style.cssText}e&&a.setAttribute(b,""""+d);a=!c.support.hrefNormalized&&f&&j?a.getAttribute(b,2):a.getAttribute(b);return a===null?w:a}return c.style(a,b,d)}});var O=/\.(.*)$/,db=function(a){return a.replace(/[^\w\s\.\|`]/g,
function(b){return""\\""+b})};c.event={add:function(a,b,d,f){if(!(a.nodeType===3||a.nodeType===8)){if(a.setInterval&&a!==A&&!a.frameElement)a=A;var e,j;if(d.handler){e=d;d=e.handler}if(!d.guid)d.guid=c.guid++;if(j=c.data(a)){var i=j.events=j.events||{},o=j.handle;if(!o)j.handle=o=function(){return typeof c!==""undefined""&&!c.event.triggered?c.event.handle.apply(o.elem,arguments):w};o.elem=a;b=b.split("" "");for(var k,n=0,r;k=b[n++];){j=e?c.extend({},e):{handler:d,data:f};if(k.indexOf(""."")>-1){r=k.split(""."");
k=r.shift();j.namespace=r.slice(0).sort().join(""."")}else{r=[];j.namespace=""""}j.type=k;j.guid=d.guid;var u=i[k],z=c.event.special[k]||{};if(!u){u=i[k]=[];if(!z.setup||z.setup.call(a,f,r,o)===false)if(a.addEventListener)a.addEventListener(k,o,false);else a.attachEvent&&a.attachEvent(""on""+k,o)}if(z.add){z.add.call(a,j);if(!j.handler.guid)j.handler.guid=d.guid}u.push(j);c.event.global[k]=true}a=null}}},global:{},remove:function(a,b,d,f){if(!(a.nodeType===3||a.nodeType===8)){var e,j=0,i,o,k,n,r,u,z=c.data(a),
C=z&&z.events;if(z&&C){if(b&&b.type){d=b.handler;b=b.type}if(!b||typeof b===""string""&&b.charAt(0)==="".""){b=b||"""";for(e in C)c.event.remove(a,e+b)}else{for(b=b.split("" "");e=b[j++];){n=e;i=e.indexOf(""."")<0;o=[];if(!i){o=e.split(""."");e=o.shift();k=new RegExp(""(^|\\.)""+c.map(o.slice(0).sort(),db).join(""\\.(?:.*\\.)?"")+""(\\.|$)"")}if(r=C[e])if(d){n=c.event.special[e]||{};for(B=f||0;B<r.length;B++){u=r[B];if(d.guid===u.guid){if(i||k.test(u.namespace)){f==null&&r.splice(B--,1);n.remove&&n.remove.call(a,u)}if(f!=
null)break}}if(r.length===0||f!=null&&r.length===1){if(!n.teardown||n.teardown.call(a,o)===false)Ca(a,e,z.handle);delete C[e]}}else for(var B=0;B<r.length;B++){u=r[B];if(i||k.test(u.namespace)){c.event.remove(a,n,u.handler,B);r.splice(B--,1)}}}if(c.isEmptyObject(C)){if(b=z.handle)b.elem=null;delete z.events;delete z.handle;c.isEmptyObject(z)&&c.removeData(a)}}}}},trigger:function(a,b,d,f){var e=a.type||a;if(!f){a=typeof a===""object""?a[G]?a:c.extend(c.Event(e),a):c.Event(e);if(e.indexOf(""!"")>=0){a.type=
e=e.slice(0,-1);a.exclusive=true}if(!d){a.stopPropagation();c.event.global[e]&&c.each(c.cache,function(){this.events&&this.events[e]&&c.event.trigger(a,b,this.handle.elem)})}if(!d||d.nodeType===3||d.nodeType===8)return w;a.result=w;a.target=d;b=c.makeArray(b);b.unshift(a)}a.currentTarget=d;(f=c.data(d,""handle""))&&f.apply(d,b);f=d.parentNode||d.ownerDocument;try{if(!(d&&d.nodeName&&c.noData[d.nodeName.toLowerCase()]))if(d[""on""+e]&&d[""on""+e].apply(d,b)===false)a.result=false}catch(j){}if(!a.isPropagationStopped()&&
f)c.event.trigger(a,b,f,true);else if(!a.isDefaultPrevented()){f=a.target;var i,o=c.nodeName(f,""a"")&&e===""click"",k=c.event.special[e]||{};if((!k._default||k._default.call(d,a)===false)&&!o&&!(f&&f.nodeName&&c.noData[f.nodeName.toLowerCase()])){try{if(f[e]){if(i=f[""on""+e])f[""on""+e]=null;c.event.triggered=true;f[e]()}}catch(n){}if(i)f[""on""+e]=i;c.event.triggered=false}}},handle:function(a){var b,d,f,e;a=arguments[0]=c.event.fix(a||A.event);a.currentTarget=this;b=a.type.indexOf(""."")<0&&!a.exclusive;
if(!b){d=a.type.split(""."");a.type=d.shift();f=new RegExp(""(^|\\.)""+d.slice(0).sort().join(""\\.(?:.*\\.)?"")+""(\\.|$)"")}e=c.data(this,""events"");d=e[a.type];if(e&&d){d=d.slice(0);e=0;for(var j=d.length;e<j;e++){var i=d[e];if(b||f.test(i.namespace)){a.handler=i.handler;a.data=i.data;a.handleObj=i;i=i.handler.apply(this,arguments);if(i!==w){a.result=i;if(i===false){a.preventDefault();a.stopPropagation()}}if(a.isImmediatePropagationStopped())break}}}return a.result},props:""altKey attrChange attrName bubbles button cancelable charCode clientX clientY ctrlKey currentTarget data detail eventPhase fromElement handler keyCode layerX layerY metaKey newValue offsetX offsetY originalTarget pageX pageY prevValue relatedNode relatedTarget screenX screenY shiftKey srcElement target toElement view wheelDelta which"".split("" ""),
fix:function(a){if(a[G])return a;var b=a;a=c.Event(b);for(var d=this.props.length,f;d;){f=this.props[--d];a[f]=b[f]}if(!a.target)a.target=a.srcElement||s;if(a.target.nodeType===3)a.target=a.target.parentNode;if(!a.relatedTarget&&a.fromElement)a.relatedTarget=a.fromElement===a.target?a.toElement:a.fromElement;if(a.pageX==null&&a.clientX!=null){b=s.documentElement;d=s.body;a.pageX=a.clientX+(b&&b.scrollLeft||d&&d.scrollLeft||0)-(b&&b.clientLeft||d&&d.clientLeft||0);a.pageY=a.clientY+(b&&b.scrollTop||
d&&d.scrollTop||0)-(b&&b.clientTop||d&&d.clientTop||0)}if(!a.which&&(a.charCode||a.charCode===0?a.charCode:a.keyCode))a.which=a.charCode||a.keyCode;if(!a.metaKey&&a.ctrlKey)a.metaKey=a.ctrlKey;if(!a.which&&a.button!==w)a.which=a.button&1?1:a.button&2?3:a.button&4?2:0;return a},guid:1E8,proxy:c.proxy,special:{ready:{setup:c.bindReady,teardown:c.noop},live:{add:function(a){c.event.add(this,a.origType,c.extend({},a,{handler:oa}))},remove:function(a){var b=true,d=a.origType.replace(O,"""");c.each(c.data(this,
""events"").live||[],function(){if(d===this.origType.replace(O,""""))return b=false});b&&c.event.remove(this,a.origType,oa)}},beforeunload:{setup:function(a,b,d){if(this.setInterval)this.onbeforeunload=d;return false},teardown:function(a,b){if(this.onbeforeunload===b)this.onbeforeunload=null}}}};var Ca=s.removeEventListener?function(a,b,d){a.removeEventListener(b,d,false)}:function(a,b,d){a.detachEvent(""on""+b,d)};c.Event=function(a){if(!this.preventDefault)return new c.Event(a);if(a&&a.type){this.originalEvent=
a;this.type=a.type}else this.type=a;this.timeStamp=J();this[G]=true};c.Event.prototype={preventDefault:function(){this.isDefaultPrevented=Z;var a=this.originalEvent;if(a){a.preventDefault&&a.preventDefault();a.returnValue=false}},stopPropagation:function(){this.isPropagationStopped=Z;var a=this.originalEvent;if(a){a.stopPropagation&&a.stopPropagation();a.cancelBubble=true}},stopImmediatePropagation:function(){this.isImmediatePropagationStopped=Z;this.stopPropagation()},isDefaultPrevented:Y,isPropagationStopped:Y,
isImmediatePropagationStopped:Y};var Da=function(a){var b=a.relatedTarget;try{for(;b&&b!==this;)b=b.parentNode;if(b!==this){a.type=a.data;c.event.handle.apply(this,arguments)}}catch(d){}},Ea=function(a){a.type=a.data;c.event.handle.apply(this,arguments)};c.each({mouseenter:""mouseover"",mouseleave:""mouseout""},function(a,b){c.event.special[a]={setup:function(d){c.event.add(this,b,d&&d.selector?Ea:Da,a)},teardown:function(d){c.event.remove(this,b,d&&d.selector?Ea:Da)}}});if(!c.support.submitBubbles)c.event.special.submit=
{setup:function(){if(this.nodeName.toLowerCase()!==""form""){c.event.add(this,""click.specialSubmit"",function(a){var b=a.target,d=b.type;if((d===""submit""||d===""image"")&&c(b).closest(""form"").length)return na(""submit"",this,arguments)});c.event.add(this,""keypress.specialSubmit"",function(a){var b=a.target,d=b.type;if((d===""text""||d===""password"")&&c(b).closest(""form"").length&&a.keyCode===13)return na(""submit"",this,arguments)})}else return false},teardown:function(){c.event.remove(this,"".specialSubmit"")}};
if(!c.support.changeBubbles){var da=/textarea|input|select/i,ea,Fa=function(a){var b=a.type,d=a.value;if(b===""radio""||b===""checkbox"")d=a.checked;else if(b===""select-multiple"")d=a.selectedIndex>-1?c.map(a.options,function(f){return f.selected}).join(""-""):"""";else if(a.nodeName.toLowerCase()===""select"")d=a.selectedIndex;return d},fa=function(a,b){var d=a.target,f,e;if(!(!da.test(d.nodeName)||d.readOnly)){f=c.data(d,""_change_data"");e=Fa(d);if(a.type!==""focusout""||d.type!==""radio"")c.data(d,""_change_data"",
e);if(!(f===w||e===f))if(f!=null||e){a.type=""change"";return c.event.trigger(a,b,d)}}};c.event.special.change={filters:{focusout:fa,click:function(a){var b=a.target,d=b.type;if(d===""radio""||d===""checkbox""||b.nodeName.toLowerCase()===""select"")return fa.call(this,a)},keydown:function(a){var b=a.target,d=b.type;if(a.keyCode===13&&b.nodeName.toLowerCase()!==""textarea""||a.keyCode===32&&(d===""checkbox""||d===""radio"")||d===""select-multiple"")return fa.call(this,a)},beforeactivate:function(a){a=a.target;c.data(a,
""_change_data"",Fa(a))}},setup:function(){if(this.type===""file"")return false;for(var a in ea)c.event.add(this,a+"".specialChange"",ea[a]);return da.test(this.nodeName)},teardown:function(){c.event.remove(this,"".specialChange"");return da.test(this.nodeName)}};ea=c.event.special.change.filters}s.addEventListener&&c.each({focus:""focusin"",blur:""focusout""},function(a,b){function d(f){f=c.event.fix(f);f.type=b;return c.event.handle.call(this,f)}c.event.special[b]={setup:function(){this.addEventListener(a,
d,true)},teardown:function(){this.removeEventListener(a,d,true)}}});c.each([""bind"",""one""],function(a,b){c.fn[b]=function(d,f,e){if(typeof d===""object""){for(var j in d)this[b](j,f,d[j],e);return this}if(c.isFunction(f)){e=f;f=w}var i=b===""one""?c.proxy(e,function(k){c(this).unbind(k,i);return e.apply(this,arguments)}):e;if(d===""unload""&&b!==""one"")this.one(d,f,e);else{j=0;for(var o=this.length;j<o;j++)c.event.add(this[j],d,i,f)}return this}});c.fn.extend({unbind:function(a,b){if(typeof a===""object""&&
!a.preventDefault)for(var d in a)this.unbind(d,a[d]);else{d=0;for(var f=this.length;d<f;d++)c.event.remove(this[d],a,b)}return this},delegate:function(a,b,d,f){return this.live(b,d,f,a)},undelegate:function(a,b,d){return arguments.length===0?this.unbind(""live""):this.die(b,null,d,a)},trigger:function(a,b){return this.each(function(){c.event.trigger(a,b,this)})},triggerHandler:function(a,b){if(this[0]){a=c.Event(a);a.preventDefault();a.stopPropagation();c.event.trigger(a,b,this[0]);return a.result}},
toggle:function(a){for(var b=arguments,d=1;d<b.length;)c.proxy(a,b[d++]);return this.click(c.proxy(a,function(f){var e=(c.data(this,""lastToggle""+a.guid)||0)%d;c.data(this,""lastToggle""+a.guid,e+1);f.preventDefault();return b[e].apply(this,arguments)||false}))},hover:function(a,b){return this.mouseenter(a).mouseleave(b||a)}});var Ga={focus:""focusin"",blur:""focusout"",mouseenter:""mouseover"",mouseleave:""mouseout""};c.each([""live"",""die""],function(a,b){c.fn[b]=function(d,f,e,j){var i,o=0,k,n,r=j||this.selector,
u=j?this:c(this.context);if(c.isFunction(f)){e=f;f=w}for(d=(d||"""").split("" "");(i=d[o++])!=null;){j=O.exec(i);k="""";if(j){k=j[0];i=i.replace(O,"""")}if(i===""hover"")d.push(""mouseenter""+k,""mouseleave""+k);else{n=i;if(i===""focus""||i===""blur""){d.push(Ga[i]+k);i+=k}else i=(Ga[i]||i)+k;b===""live""?u.each(function(){c.event.add(this,pa(i,r),{data:f,selector:r,handler:e,origType:i,origHandler:e,preType:n})}):u.unbind(pa(i,r),e)}}return this}});c.each(""blur focus focusin focusout load resize scroll unload click dblclick mousedown mouseup mousemove mouseover mouseout mouseenter mouseleave change select submit keydown keypress keyup error"".split("" ""),
function(a,b){c.fn[b]=function(d){return d?this.bind(b,d):this.trigger(b)};if(c.attrFn)c.attrFn[b]=true});A.attachEvent&&!A.addEventListener&&A.attachEvent(""onunload"",function(){for(var a in c.cache)if(c.cache[a].handle)try{c.event.remove(c.cache[a].handle.elem)}catch(b){}});(function(){function a(g){for(var h="""",l,m=0;g[m];m++){l=g[m];if(l.nodeType===3||l.nodeType===4)h+=l.nodeValue;else if(l.nodeType!==8)h+=a(l.childNodes)}return h}function b(g,h,l,m,q,p){q=0;for(var v=m.length;q<v;q++){var t=m[q];
if(t){t=t[g];for(var y=false;t;){if(t.sizcache===l){y=m[t.sizset];break}if(t.nodeType===1&&!p){t.sizcache=l;t.sizset=q}if(t.nodeName.toLowerCase()===h){y=t;break}t=t[g]}m[q]=y}}}function d(g,h,l,m,q,p){q=0;for(var v=m.length;q<v;q++){var t=m[q];if(t){t=t[g];for(var y=false;t;){if(t.sizcache===l){y=m[t.sizset];break}if(t.nodeType===1){if(!p){t.sizcache=l;t.sizset=q}if(typeof h!==""string""){if(t===h){y=true;break}}else if(k.filter(h,[t]).length>0){y=t;break}}t=t[g]}m[q]=y}}}var f=/((?:\((?:\([^()]+\)|[^()]+)+\)|\[(?:\[[^[\]]*\]|['""][^'""]*['""]|[^[\]'""]+)+\]|\\.|[^ >+~,(\[\\]+)+|[>+~])(\s*,\s*)?((?:.|\r|\n)*)/g,
e=0,j=Object.prototype.toString,i=false,o=true;[0,0].sort(function(){o=false;return 0});var k=function(g,h,l,m){l=l||[];var q=h=h||s;if(h.nodeType!==1&&h.nodeType!==9)return[];if(!g||typeof g!==""string"")return l;for(var p=[],v,t,y,S,H=true,M=x(h),I=g;(f.exec(""""),v=f.exec(I))!==null;){I=v[3];p.push(v[1]);if(v[2]){S=v[3];break}}if(p.length>1&&r.exec(g))if(p.length===2&&n.relative[p[0]])t=ga(p[0]+p[1],h);else for(t=n.relative[p[0]]?[h]:k(p.shift(),h);p.length;){g=p.shift();if(n.relative[g])g+=p.shift();
t=ga(g,t)}else{if(!m&&p.length>1&&h.nodeType===9&&!M&&n.match.ID.test(p[0])&&!n.match.ID.test(p[p.length-1])){v=k.find(p.shift(),h,M);h=v.expr?k.filter(v.expr,v.set)[0]:v.set[0]}if(h){v=m?{expr:p.pop(),set:z(m)}:k.find(p.pop(),p.length===1&&(p[0]===""~""||p[0]===""+"")&&h.parentNode?h.parentNode:h,M);t=v.expr?k.filter(v.expr,v.set):v.set;if(p.length>0)y=z(t);else H=false;for(;p.length;){var D=p.pop();v=D;if(n.relative[D])v=p.pop();else D="""";if(v==null)v=h;n.relative[D](y,v,M)}}else y=[]}y||(y=t);y||k.error(D||
g);if(j.call(y)===""[object Array]"")if(H)if(h&&h.nodeType===1)for(g=0;y[g]!=null;g++){if(y[g]&&(y[g]===true||y[g].nodeType===1&&E(h,y[g])))l.push(t[g])}else for(g=0;y[g]!=null;g++)y[g]&&y[g].nodeType===1&&l.push(t[g]);else l.push.apply(l,y);else z(y,l);if(S){k(S,q,l,m);k.uniqueSort(l)}return l};k.uniqueSort=function(g){if(B){i=o;g.sort(B);if(i)for(var h=1;h<g.length;h++)g[h]===g[h-1]&&g.splice(h--,1)}return g};k.matches=function(g,h){return k(g,null,null,h)};k.find=function(g,h,l){var m,q;if(!g)return[];
for(var p=0,v=n.order.length;p<v;p++){var t=n.order[p];if(q=n.leftMatch[t].exec(g)){var y=q[1];q.splice(1,1);if(y.substr(y.length-1)!==""\\""){q[1]=(q[1]||"""").replace(/\\/g,"""");m=n.find[t](q,h,l);if(m!=null){g=g.replace(n.match[t],"""");break}}}}m||(m=h.getElementsByTagName(""*""));return{set:m,expr:g}};k.filter=function(g,h,l,m){for(var q=g,p=[],v=h,t,y,S=h&&h[0]&&x(h[0]);g&&h.length;){for(var H in n.filter)if((t=n.leftMatch[H].exec(g))!=null&&t[2]){var M=n.filter[H],I,D;D=t[1];y=false;t.splice(1,1);if(D.substr(D.length-
1)!==""\\""){if(v===p)p=[];if(n.preFilter[H])if(t=n.preFilter[H](t,v,l,p,m,S)){if(t===true)continue}else y=I=true;if(t)for(var U=0;(D=v[U])!=null;U++)if(D){I=M(D,t,U,v);var Ha=m^!!I;if(l&&I!=null)if(Ha)y=true;else v[U]=false;else if(Ha){p.push(D);y=true}}if(I!==w){l||(v=p);g=g.replace(n.match[H],"""");if(!y)return[];break}}}if(g===q)if(y==null)k.error(g);else break;q=g}return v};k.error=function(g){throw""Syntax error, unrecognized expression: ""+g;};var n=k.selectors={order:[""ID"",""NAME"",""TAG""],match:{ID:/#((?:[\w\u00c0-\uFFFF-]|\\.)+)/,
CLASS:/\.((?:[\w\u00c0-\uFFFF-]|\\.)+)/,NAME:/\[name=['""]*((?:[\w\u00c0-\uFFFF-]|\\.)+)['""]*\]/,ATTR:/\[\s*((?:[\w\u00c0-\uFFFF-]|\\.)+)\s*(?:(\S?=)\s*(['""]*)(.*?)\3|)\s*\]/,TAG:/^((?:[\w\u00c0-\uFFFF\*-]|\\.)+)/,CHILD:/:(only|nth|last|first)-child(?:\((even|odd|[\dn+-]*)\))?/,POS:/:(nth|eq|gt|lt|first|last|even|odd)(?:\((\d*)\))?(?=[^-]|$)/,PSEUDO:/:((?:[\w\u00c0-\uFFFF-]|\\.)+)(?:\((['""]?)((?:\([^\)]+\)|[^\(\)]*)+)\2\))?/},leftMatch:{},attrMap:{""class"":""className"",""for"":""htmlFor""},attrHandle:{href:function(g){return g.getAttribute(""href"")}},
relative:{""+"":function(g,h){var l=typeof h===""string"",m=l&&!/\W/.test(h);l=l&&!m;if(m)h=h.toLowerCase();m=0;for(var q=g.length,p;m<q;m++)if(p=g[m]){for(;(p=p.previousSibling)&&p.nodeType!==1;);g[m]=l||p&&p.nodeName.toLowerCase()===h?p||false:p===h}l&&k.filter(h,g,true)},"">"":function(g,h){var l=typeof h===""string"";if(l&&!/\W/.test(h)){h=h.toLowerCase();for(var m=0,q=g.length;m<q;m++){var p=g[m];if(p){l=p.parentNode;g[m]=l.nodeName.toLowerCase()===h?l:false}}}else{m=0;for(q=g.length;m<q;m++)if(p=g[m])g[m]=
l?p.parentNode:p.parentNode===h;l&&k.filter(h,g,true)}},"""":function(g,h,l){var m=e++,q=d;if(typeof h===""string""&&!/\W/.test(h)){var p=h=h.toLowerCase();q=b}q(""parentNode"",h,m,g,p,l)},""~"":function(g,h,l){var m=e++,q=d;if(typeof h===""string""&&!/\W/.test(h)){var p=h=h.toLowerCase();q=b}q(""previousSibling"",h,m,g,p,l)}},find:{ID:function(g,h,l){if(typeof h.getElementById!==""undefined""&&!l)return(g=h.getElementById(g[1]))?[g]:[]},NAME:function(g,h){if(typeof h.getElementsByName!==""undefined""){var l=[];
h=h.getElementsByName(g[1]);for(var m=0,q=h.length;m<q;m++)h[m].getAttribute(""name"")===g[1]&&l.push(h[m]);return l.length===0?null:l}},TAG:function(g,h){return h.getElementsByTagName(g[1])}},preFilter:{CLASS:function(g,h,l,m,q,p){g="" ""+g[1].replace(/\\/g,"""")+"" "";if(p)return g;p=0;for(var v;(v=h[p])!=null;p++)if(v)if(q^(v.className&&("" ""+v.className+"" "").replace(/[\t\n]/g,"" "").indexOf(g)>=0))l||m.push(v);else if(l)h[p]=false;return false},ID:function(g){return g[1].replace(/\\/g,"""")},TAG:function(g){return g[1].toLowerCase()},
CHILD:function(g){if(g[1]===""nth""){var h=/(-?)(\d*)n((?:\+|-)?\d*)/.exec(g[2]===""even""&&""2n""||g[2]===""odd""&&""2n+1""||!/\D/.test(g[2])&&""0n+""+g[2]||g[2]);g[2]=h[1]+(h[2]||1)-0;g[3]=h[3]-0}g[0]=e++;return g},ATTR:function(g,h,l,m,q,p){h=g[1].replace(/\\/g,"""");if(!p&&n.attrMap[h])g[1]=n.attrMap[h];if(g[2]===""~="")g[4]="" ""+g[4]+"" "";return g},PSEUDO:function(g,h,l,m,q){if(g[1]===""not"")if((f.exec(g[3])||"""").length>1||/^\w/.test(g[3]))g[3]=k(g[3],null,null,h);else{g=k.filter(g[3],h,l,true^q);l||m.push.apply(m,
g);return false}else if(n.match.POS.test(g[0])||n.match.CHILD.test(g[0]))return true;return g},POS:function(g){g.unshift(true);return g}},filters:{enabled:function(g){return g.disabled===false&&g.type!==""hidden""},disabled:function(g){return g.disabled===true},checked:function(g){return g.checked===true},selected:function(g){return g.selected===true},parent:function(g){return!!g.firstChild},empty:function(g){return!g.firstChild},has:function(g,h,l){return!!k(l[3],g).length},header:function(g){return/h\d/i.test(g.nodeName)},
text:function(g){return""text""===g.type},radio:function(g){return""radio""===g.type},checkbox:function(g){return""checkbox""===g.type},file:function(g){return""file""===g.type},password:function(g){return""password""===g.type},submit:function(g){return""submit""===g.type},image:function(g){return""image""===g.type},reset:function(g){return""reset""===g.type},button:function(g){return""button""===g.type||g.nodeName.toLowerCase()===""button""},input:function(g){return/input|select|textarea|button/i.test(g.nodeName)}},
setFilters:{first:function(g,h){return h===0},last:function(g,h,l,m){return h===m.length-1},even:function(g,h){return h%2===0},odd:function(g,h){return h%2===1},lt:function(g,h,l){return h<l[3]-0},gt:function(g,h,l){return h>l[3]-0},nth:function(g,h,l){return l[3]-0===h},eq:function(g,h,l){return l[3]-0===h}},filter:{PSEUDO:function(g,h,l,m){var q=h[1],p=n.filters[q];if(p)return p(g,l,h,m);else if(q===""contains"")return(g.textContent||g.innerText||a([g])||"""").indexOf(h[3])>=0;else if(q===""not""){h=
h[3];l=0;for(m=h.length;l<m;l++)if(h[l]===g)return false;return true}else k.error(""Syntax error, unrecognized expression: ""+q)},CHILD:function(g,h){var l=h[1],m=g;switch(l){case ""only"":case ""first"":for(;m=m.previousSibling;)if(m.nodeType===1)return false;if(l===""first"")return true;m=g;case ""last"":for(;m=m.nextSibling;)if(m.nodeType===1)return false;return true;case ""nth"":l=h[2];var q=h[3];if(l===1&&q===0)return true;h=h[0];var p=g.parentNode;if(p&&(p.sizcache!==h||!g.nodeIndex)){var v=0;for(m=p.firstChild;m;m=
m.nextSibling)if(m.nodeType===1)m.nodeIndex=++v;p.sizcache=h}g=g.nodeIndex-q;return l===0?g===0:g%l===0&&g/l>=0}},ID:function(g,h){return g.nodeType===1&&g.getAttribute(""id"")===h},TAG:function(g,h){return h===""*""&&g.nodeType===1||g.nodeName.toLowerCase()===h},CLASS:function(g,h){return("" ""+(g.className||g.getAttribute(""class""))+"" "").indexOf(h)>-1},ATTR:function(g,h){var l=h[1];g=n.attrHandle[l]?n.attrHandle[l](g):g[l]!=null?g[l]:g.getAttribute(l);l=g+"""";var m=h[2];h=h[4];return g==null?m===""!="":m===
""=""?l===h:m===""*=""?l.indexOf(h)>=0:m===""~=""?("" ""+l+"" "").indexOf(h)>=0:!h?l&&g!==false:m===""!=""?l!==h:m===""^=""?l.indexOf(h)===0:m===""$=""?l.substr(l.length-h.length)===h:m===""|=""?l===h||l.substr(0,h.length+1)===h+""-"":false},POS:function(g,h,l,m){var q=n.setFilters[h[2]];if(q)return q(g,l,h,m)}}},r=n.match.POS;for(var u in n.match){n.match[u]=new RegExp(n.match[u].source+/(?![^\[]*\])(?![^\(]*\))/.source);n.leftMatch[u]=new RegExp(/(^(?:.|\r|\n)*?)/.source+n.match[u].source.replace(/\\(\d+)/g,function(g,
h){return""\\""+(h-0+1)}))}var z=function(g,h){g=Array.prototype.slice.call(g,0);if(h){h.push.apply(h,g);return h}return g};try{Array.prototype.slice.call(s.documentElement.childNodes,0)}catch(C){z=function(g,h){h=h||[];if(j.call(g)===""[object Array]"")Array.prototype.push.apply(h,g);else if(typeof g.length===""number"")for(var l=0,m=g.length;l<m;l++)h.push(g[l]);else for(l=0;g[l];l++)h.push(g[l]);return h}}var B;if(s.documentElement.compareDocumentPosition)B=function(g,h){if(!g.compareDocumentPosition||
!h.compareDocumentPosition){if(g==h)i=true;return g.compareDocumentPosition?-1:1}g=g.compareDocumentPosition(h)&4?-1:g===h?0:1;if(g===0)i=true;return g};else if(""sourceIndex""in s.documentElement)B=function(g,h){if(!g.sourceIndex||!h.sourceIndex){if(g==h)i=true;return g.sourceIndex?-1:1}g=g.sourceIndex-h.sourceIndex;if(g===0)i=true;return g};else if(s.createRange)B=function(g,h){if(!g.ownerDocument||!h.ownerDocument){if(g==h)i=true;return g.ownerDocument?-1:1}var l=g.ownerDocument.createRange(),m=
h.ownerDocument.createRange();l.setStart(g,0);l.setEnd(g,0);m.setStart(h,0);m.setEnd(h,0);g=l.compareBoundaryPoints(Range.START_TO_END,m);if(g===0)i=true;return g};(function(){var g=s.createElement(""div""),h=""script""+(new Date).getTime();g.innerHTML=""<a name='""+h+""'/>"";var l=s.documentElement;l.insertBefore(g,l.firstChild);if(s.getElementById(h)){n.find.ID=function(m,q,p){if(typeof q.getElementById!==""undefined""&&!p)return(q=q.getElementById(m[1]))?q.id===m[1]||typeof q.getAttributeNode!==""undefined""&&
q.getAttributeNode(""id"").nodeValue===m[1]?[q]:w:[]};n.filter.ID=function(m,q){var p=typeof m.getAttributeNode!==""undefined""&&m.getAttributeNode(""id"");return m.nodeType===1&&p&&p.nodeValue===q}}l.removeChild(g);l=g=null})();(function(){var g=s.createElement(""div"");g.appendChild(s.createComment(""""));if(g.getElementsByTagName(""*"").length>0)n.find.TAG=function(h,l){l=l.getElementsByTagName(h[1]);if(h[1]===""*""){h=[];for(var m=0;l[m];m++)l[m].nodeType===1&&h.push(l[m]);l=h}return l};g.innerHTML=""<a href='#'></a>"";
if(g.firstChild&&typeof g.firstChild.getAttribute!==""undefined""&&g.firstChild.getAttribute(""href"")!==""#"")n.attrHandle.href=function(h){return h.getAttribute(""href"",2)};g=null})();s.querySelectorAll&&function(){var g=k,h=s.createElement(""div"");h.innerHTML=""<p class='TEST'></p>"";if(!(h.querySelectorAll&&h.querySelectorAll("".TEST"").length===0)){k=function(m,q,p,v){q=q||s;if(!v&&q.nodeType===9&&!x(q))try{return z(q.querySelectorAll(m),p)}catch(t){}return g(m,q,p,v)};for(var l in g)k[l]=g[l];h=null}}();
(function(){var g=s.createElement(""div"");g.innerHTML=""<div class='test e'></div><div class='test'></div>"";if(!(!g.getElementsByClassName||g.getElementsByClassName(""e"").length===0)){g.lastChild.className=""e"";if(g.getElementsByClassName(""e"").length!==1){n.order.splice(1,0,""CLASS"");n.find.CLASS=function(h,l,m){if(typeof l.getElementsByClassName!==""undefined""&&!m)return l.getElementsByClassName(h[1])};g=null}}})();var E=s.compareDocumentPosition?function(g,h){return!!(g.compareDocumentPosition(h)&16)}:
function(g,h){return g!==h&&(g.contains?g.contains(h):true)},x=function(g){return(g=(g?g.ownerDocument||g:0).documentElement)?g.nodeName!==""HTML"":false},ga=function(g,h){var l=[],m="""",q;for(h=h.nodeType?[h]:h;q=n.match.PSEUDO.exec(g);){m+=q[0];g=g.replace(n.match.PSEUDO,"""")}g=n.relative[g]?g+""*"":g;q=0;for(var p=h.length;q<p;q++)k(g,h[q],l);return k.filter(m,l)};c.find=k;c.expr=k.selectors;c.expr["":""]=c.expr.filters;c.unique=k.uniqueSort;c.text=a;c.isXMLDoc=x;c.contains=E})();var eb=/Until$/,fb=/^(?:parents|prevUntil|prevAll)/,
gb=/,/;R=Array.prototype.slice;var Ia=function(a,b,d){if(c.isFunction(b))return c.grep(a,function(e,j){return!!b.call(e,j,e)===d});else if(b.nodeType)return c.grep(a,function(e){return e===b===d});else if(typeof b===""string""){var f=c.grep(a,function(e){return e.nodeType===1});if(Ua.test(b))return c.filter(b,f,!d);else b=c.filter(b,f)}return c.grep(a,function(e){return c.inArray(e,b)>=0===d})};c.fn.extend({find:function(a){for(var b=this.pushStack("""",""find"",a),d=0,f=0,e=this.length;f<e;f++){d=b.length;
c.find(a,this[f],b);if(f>0)for(var j=d;j<b.length;j++)for(var i=0;i<d;i++)if(b[i]===b[j]){b.splice(j--,1);break}}return b},has:function(a){var b=c(a);return this.filter(function(){for(var d=0,f=b.length;d<f;d++)if(c.contains(this,b[d]))return true})},not:function(a){return this.pushStack(Ia(this,a,false),""not"",a)},filter:function(a){return this.pushStack(Ia(this,a,true),""filter"",a)},is:function(a){return!!a&&c.filter(a,this).length>0},closest:function(a,b){if(c.isArray(a)){var d=[],f=this[0],e,j=
{},i;if(f&&a.length){e=0;for(var o=a.length;e<o;e++){i=a[e];j[i]||(j[i]=c.expr.match.POS.test(i)?c(i,b||this.context):i)}for(;f&&f.ownerDocument&&f!==b;){for(i in j){e=j[i];if(e.jquery?e.index(f)>-1:c(f).is(e)){d.push({selector:i,elem:f});delete j[i]}}f=f.parentNode}}return d}var k=c.expr.match.POS.test(a)?c(a,b||this.context):null;return this.map(function(n,r){for(;r&&r.ownerDocument&&r!==b;){if(k?k.index(r)>-1:c(r).is(a))return r;r=r.parentNode}return null})},index:function(a){if(!a||typeof a===
""string"")return c.inArray(this[0],a?c(a):this.parent().children());return c.inArray(a.jquery?a[0]:a,this)},add:function(a,b){a=typeof a===""string""?c(a,b||this.context):c.makeArray(a);b=c.merge(this.get(),a);return this.pushStack(qa(a[0])||qa(b[0])?b:c.unique(b))},andSelf:function(){return this.add(this.prevObject)}});c.each({parent:function(a){return(a=a.parentNode)&&a.nodeType!==11?a:null},parents:function(a){return c.dir(a,""parentNode"")},parentsUntil:function(a,b,d){return c.dir(a,""parentNode"",
d)},next:function(a){return c.nth(a,2,""nextSibling"")},prev:function(a){return c.nth(a,2,""previousSibling"")},nextAll:function(a){return c.dir(a,""nextSibling"")},prevAll:function(a){return c.dir(a,""previousSibling"")},nextUntil:function(a,b,d){return c.dir(a,""nextSibling"",d)},prevUntil:function(a,b,d){return c.dir(a,""previousSibling"",d)},siblings:function(a){return c.sibling(a.parentNode.firstChild,a)},children:function(a){return c.sibling(a.firstChild)},contents:function(a){return c.nodeName(a,""iframe"")?
a.contentDocument||a.contentWindow.document:c.makeArray(a.childNodes)}},function(a,b){c.fn[a]=function(d,f){var e=c.map(this,b,d);eb.test(a)||(f=d);if(f&&typeof f===""string"")e=c.filter(f,e);e=this.length>1?c.unique(e):e;if((this.length>1||gb.test(f))&&fb.test(a))e=e.reverse();return this.pushStack(e,a,R.call(arguments).join("",""))}});c.extend({filter:function(a,b,d){if(d)a="":not(""+a+"")"";return c.find.matches(a,b)},dir:function(a,b,d){var f=[];for(a=a[b];a&&a.nodeType!==9&&(d===w||a.nodeType!==1||!c(a).is(d));){a.nodeType===
1&&f.push(a);a=a[b]}return f},nth:function(a,b,d){b=b||1;for(var f=0;a;a=a[d])if(a.nodeType===1&&++f===b)break;return a},sibling:function(a,b){for(var d=[];a;a=a.nextSibling)a.nodeType===1&&a!==b&&d.push(a);return d}});var Ja=/ jQuery\d+=""(?:\d+|null)""/g,V=/^\s+/,Ka=/(<([\w:]+)[^>]*?)\/>/g,hb=/^(?:area|br|col|embed|hr|img|input|link|meta|param)$/i,La=/<([\w:]+)/,ib=/<tbody/i,jb=/<|&#?\w+;/,ta=/<script|<object|<embed|<option|<style/i,ua=/checked\s*(?:[^=]|=\s*.checked.)/i,Ma=function(a,b,d){return hb.test(d)?
a:b+""></""+d+"">""},F={option:[1,""<select multiple='multiple'>"",""</select>""],legend:[1,""<fieldset>"",""</fieldset>""],thead:[1,""<table>"",""</table>""],tr:[2,""<table><tbody>"",""</tbody></table>""],td:[3,""<table><tbody><tr>"",""</tr></tbody></table>""],col:[2,""<table><tbody></tbody><colgroup>"",""</colgroup></table>""],area:[1,""<map>"",""</map>""],_default:[0,"""",""""]};F.optgroup=F.option;F.tbody=F.tfoot=F.colgroup=F.caption=F.thead;F.th=F.td;if(!c.support.htmlSerialize)F._default=[1,""div<div>"",""</div>""];c.fn.extend({text:function(a){if(c.isFunction(a))return this.each(function(b){var d=
c(this);d.text(a.call(this,b,d.text()))});if(typeof a!==""object""&&a!==w)return this.empty().append((this[0]&&this[0].ownerDocument||s).createTextNode(a));return c.text(this)},wrapAll:function(a){if(c.isFunction(a))return this.each(function(d){c(this).wrapAll(a.call(this,d))});if(this[0]){var b=c(a,this[0].ownerDocument).eq(0).clone(true);this[0].parentNode&&b.insertBefore(this[0]);b.map(function(){for(var d=this;d.firstChild&&d.firstChild.nodeType===1;)d=d.firstChild;return d}).append(this)}return this},
wrapInner:function(a){if(c.isFunction(a))return this.each(function(b){c(this).wrapInner(a.call(this,b))});return this.each(function(){var b=c(this),d=b.contents();d.length?d.wrapAll(a):b.append(a)})},wrap:function(a){return this.each(function(){c(this).wrapAll(a)})},unwrap:function(){return this.parent().each(function(){c.nodeName(this,""body"")||c(this).replaceWith(this.childNodes)}).end()},append:function(){return this.domManip(arguments,true,function(a){this.nodeType===1&&this.appendChild(a)})},
prepend:function(){return this.domManip(arguments,true,function(a){this.nodeType===1&&this.insertBefore(a,this.firstChild)})},before:function(){if(this[0]&&this[0].parentNode)return this.domManip(arguments,false,function(b){this.parentNode.insertBefore(b,this)});else if(arguments.length){var a=c(arguments[0]);a.push.apply(a,this.toArray());return this.pushStack(a,""before"",arguments)}},after:function(){if(this[0]&&this[0].parentNode)return this.domManip(arguments,false,function(b){this.parentNode.insertBefore(b,
this.nextSibling)});else if(arguments.length){var a=this.pushStack(this,""after"",arguments);a.push.apply(a,c(arguments[0]).toArray());return a}},remove:function(a,b){for(var d=0,f;(f=this[d])!=null;d++)if(!a||c.filter(a,[f]).length){if(!b&&f.nodeType===1){c.cleanData(f.getElementsByTagName(""*""));c.cleanData([f])}f.parentNode&&f.parentNode.removeChild(f)}return this},empty:function(){for(var a=0,b;(b=this[a])!=null;a++)for(b.nodeType===1&&c.cleanData(b.getElementsByTagName(""*""));b.firstChild;)b.removeChild(b.firstChild);
return this},clone:function(a){var b=this.map(function(){if(!c.support.noCloneEvent&&!c.isXMLDoc(this)){var d=this.outerHTML,f=this.ownerDocument;if(!d){d=f.createElement(""div"");d.appendChild(this.cloneNode(true));d=d.innerHTML}return c.clean([d.replace(Ja,"""").replace(/=([^=""'>\s]+\/)>/g,'=""$1"">').replace(V,"""")],f)[0]}else return this.cloneNode(true)});if(a===true){ra(this,b);ra(this.find(""*""),b.find(""*""))}return b},html:function(a){if(a===w)return this[0]&&this[0].nodeType===1?this[0].innerHTML.replace(Ja,
""""):null;else if(typeof a===""string""&&!ta.test(a)&&(c.support.leadingWhitespace||!V.test(a))&&!F[(La.exec(a)||["""",""""])[1].toLowerCase()]){a=a.replace(Ka,Ma);try{for(var b=0,d=this.length;b<d;b++)if(this[b].nodeType===1){c.cleanData(this[b].getElementsByTagName(""*""));this[b].innerHTML=a}}catch(f){this.empty().append(a)}}else c.isFunction(a)?this.each(function(e){var j=c(this),i=j.html();j.empty().append(function(){return a.call(this,e,i)})}):this.empty().append(a);return this},replaceWith:function(a){if(this[0]&&
this[0].parentNode){if(c.isFunction(a))return this.each(function(b){var d=c(this),f=d.html();d.replaceWith(a.call(this,b,f))});if(typeof a!==""string"")a=c(a).detach();return this.each(function(){var b=this.nextSibling,d=this.parentNode;c(this).remove();b?c(b).before(a):c(d).append(a)})}else return this.pushStack(c(c.isFunction(a)?a():a),""replaceWith"",a)},detach:function(a){return this.remove(a,true)},domManip:function(a,b,d){function f(u){return c.nodeName(u,""table"")?u.getElementsByTagName(""tbody"")[0]||
u.appendChild(u.ownerDocument.createElement(""tbody"")):u}var e,j,i=a[0],o=[],k;if(!c.support.checkClone&&arguments.length===3&&typeof i===""string""&&ua.test(i))return this.each(function(){c(this).domManip(a,b,d,true)});if(c.isFunction(i))return this.each(function(u){var z=c(this);a[0]=i.call(this,u,b?z.html():w);z.domManip(a,b,d)});if(this[0]){e=i&&i.parentNode;e=c.support.parentNode&&e&&e.nodeType===11&&e.childNodes.length===this.length?{fragment:e}:sa(a,this,o);k=e.fragment;if(j=k.childNodes.length===
1?(k=k.firstChild):k.firstChild){b=b&&c.nodeName(j,""tr"");for(var n=0,r=this.length;n<r;n++)d.call(b?f(this[n],j):this[n],n>0||e.cacheable||this.length>1?k.cloneNode(true):k)}o.length&&c.each(o,Qa)}return this}});c.fragments={};c.each({appendTo:""append"",prependTo:""prepend"",insertBefore:""before"",insertAfter:""after"",replaceAll:""replaceWith""},function(a,b){c.fn[a]=function(d){var f=[];d=c(d);var e=this.length===1&&this[0].parentNode;if(e&&e.nodeType===11&&e.childNodes.length===1&&d.length===1){d[b](this[0]);
return this}else{e=0;for(var j=d.length;e<j;e++){var i=(e>0?this.clone(true):this).get();c.fn[b].apply(c(d[e]),i);f=f.concat(i)}return this.pushStack(f,a,d.selector)}}});c.extend({clean:function(a,b,d,f){b=b||s;if(typeof b.createElement===""undefined"")b=b.ownerDocument||b[0]&&b[0].ownerDocument||s;for(var e=[],j=0,i;(i=a[j])!=null;j++){if(typeof i===""number"")i+="""";if(i){if(typeof i===""string""&&!jb.test(i))i=b.createTextNode(i);else if(typeof i===""string""){i=i.replace(Ka,Ma);var o=(La.exec(i)||["""",
""""])[1].toLowerCase(),k=F[o]||F._default,n=k[0],r=b.createElement(""div"");for(r.innerHTML=k[1]+i+k[2];n--;)r=r.lastChild;if(!c.support.tbody){n=ib.test(i);o=o===""table""&&!n?r.firstChild&&r.firstChild.childNodes:k[1]===""<table>""&&!n?r.childNodes:[];for(k=o.length-1;k>=0;--k)c.nodeName(o[k],""tbody"")&&!o[k].childNodes.length&&o[k].parentNode.removeChild(o[k])}!c.support.leadingWhitespace&&V.test(i)&&r.insertBefore(b.createTextNode(V.exec(i)[0]),r.firstChild);i=r.childNodes}if(i.nodeType)e.push(i);else e=
c.merge(e,i)}}if(d)for(j=0;e[j];j++)if(f&&c.nodeName(e[j],""script"")&&(!e[j].type||e[j].type.toLowerCase()===""text/javascript""))f.push(e[j].parentNode?e[j].parentNode.removeChild(e[j]):e[j]);else{e[j].nodeType===1&&e.splice.apply(e,[j+1,0].concat(c.makeArray(e[j].getElementsByTagName(""script""))));d.appendChild(e[j])}return e},cleanData:function(a){for(var b,d,f=c.cache,e=c.event.special,j=c.support.deleteExpando,i=0,o;(o=a[i])!=null;i++)if(d=o[c.expando]){b=f[d];if(b.events)for(var k in b.events)e[k]?
c.event.remove(o,k):Ca(o,k,b.handle);if(j)delete o[c.expando];else o.removeAttribute&&o.removeAttribute(c.expando);delete f[d]}}});var kb=/z-?index|font-?weight|opacity|zoom|line-?height/i,Na=/alpha\([^)]*\)/,Oa=/opacity=([^)]*)/,ha=/float/i,ia=/-([a-z])/ig,lb=/([A-Z])/g,mb=/^-?\d+(?:px)?$/i,nb=/^-?\d/,ob={position:""absolute"",visibility:""hidden"",display:""block""},pb=[""Left"",""Right""],qb=[""Top"",""Bottom""],rb=s.defaultView&&s.defaultView.getComputedStyle,Pa=c.support.cssFloat?""cssFloat"":""styleFloat"",ja=
function(a,b){return b.toUpperCase()};c.fn.css=function(a,b){return X(this,a,b,true,function(d,f,e){if(e===w)return c.curCSS(d,f);if(typeof e===""number""&&!kb.test(f))e+=""px"";c.style(d,f,e)})};c.extend({style:function(a,b,d){if(!a||a.nodeType===3||a.nodeType===8)return w;if((b===""width""||b===""height"")&&parseFloat(d)<0)d=w;var f=a.style||a,e=d!==w;if(!c.support.opacity&&b===""opacity""){if(e){f.zoom=1;b=parseInt(d,10)+""""===""NaN""?"""":""alpha(opacity=""+d*100+"")"";a=f.filter||c.curCSS(a,""filter"")||"""";f.filter=
Na.test(a)?a.replace(Na,b):b}return f.filter&&f.filter.indexOf(""opacity="")>=0?parseFloat(Oa.exec(f.filter)[1])/100+"""":""""}if(ha.test(b))b=Pa;b=b.replace(ia,ja);if(e)f[b]=d;return f[b]},css:function(a,b,d,f){if(b===""width""||b===""height""){var e,j=b===""width""?pb:qb;function i(){e=b===""width""?a.offsetWidth:a.offsetHeight;f!==""border""&&c.each(j,function(){f||(e-=parseFloat(c.curCSS(a,""padding""+this,true))||0);if(f===""margin"")e+=parseFloat(c.curCSS(a,""margin""+this,true))||0;else e-=parseFloat(c.curCSS(a,
""border""+this+""Width"",true))||0})}a.offsetWidth!==0?i():c.swap(a,ob,i);return Math.max(0,Math.round(e))}return c.curCSS(a,b,d)},curCSS:function(a,b,d){var f,e=a.style;if(!c.support.opacity&&b===""opacity""&&a.currentStyle){f=Oa.test(a.currentStyle.filter||"""")?parseFloat(RegExp.$1)/100+"""":"""";return f===""""?""1"":f}if(ha.test(b))b=Pa;if(!d&&e&&e[b])f=e[b];else if(rb){if(ha.test(b))b=""float"";b=b.replace(lb,""-$1"").toLowerCase();e=a.ownerDocument.defaultView;if(!e)return null;if(a=e.getComputedStyle(a,null))f=
a.getPropertyValue(b);if(b===""opacity""&&f==="""")f=""1""}else if(a.currentStyle){d=b.replace(ia,ja);f=a.currentStyle[b]||a.currentStyle[d];if(!mb.test(f)&&nb.test(f)){b=e.left;var j=a.runtimeStyle.left;a.runtimeStyle.left=a.currentStyle.left;e.left=d===""fontSize""?""1em"":f||0;f=e.pixelLeft+""px"";e.left=b;a.runtimeStyle.left=j}}return f},swap:function(a,b,d){var f={};for(var e in b){f[e]=a.style[e];a.style[e]=b[e]}d.call(a);for(e in b)a.style[e]=f[e]}});if(c.expr&&c.expr.filters){c.expr.filters.hidden=function(a){var b=
a.offsetWidth,d=a.offsetHeight,f=a.nodeName.toLowerCase()===""tr"";return b===0&&d===0&&!f?true:b>0&&d>0&&!f?false:c.curCSS(a,""display"")===""none""};c.expr.filters.visible=function(a){return!c.expr.filters.hidden(a)}}var sb=J(),tb=/<script(.|\s)*?\/script>/gi,ub=/select|textarea/i,vb=/color|date|datetime|email|hidden|month|number|password|range|search|tel|text|time|url|week/i,N=/=\?(&|$)/,ka=/\?/,wb=/(\?|&)_=.*?(&|$)/,xb=/^(\w+:)?\/\/([^\/?#]+)/,yb=/%20/g,zb=c.fn.load;c.fn.extend({load:function(a,b,d){if(typeof a!==
""string"")return zb.call(this,a);else if(!this.length)return this;var f=a.indexOf("" "");if(f>=0){var e=a.slice(f,a.length);a=a.slice(0,f)}f=""GET"";if(b)if(c.isFunction(b)){d=b;b=null}else if(typeof b===""object""){b=c.param(b,c.ajaxSettings.traditional);f=""POST""}var j=this;c.ajax({url:a,type:f,dataType:""html"",data:b,complete:function(i,o){if(o===""success""||o===""notmodified"")j.html(e?c(""<div />"").append(i.responseText.replace(tb,"""")).find(e):i.responseText);d&&j.each(d,[i.responseText,o,i])}});return this},
serialize:function(){return c.param(this.serializeArray())},serializeArray:function(){return this.map(function(){return this.elements?c.makeArray(this.elements):this}).filter(function(){return this.name&&!this.disabled&&(this.checked||ub.test(this.nodeName)||vb.test(this.type))}).map(function(a,b){a=c(this).val();return a==null?null:c.isArray(a)?c.map(a,function(d){return{name:b.name,value:d}}):{name:b.name,value:a}}).get()}});c.each(""ajaxStart ajaxStop ajaxComplete ajaxError ajaxSuccess ajaxSend"".split("" ""),
function(a,b){c.fn[b]=function(d){return this.bind(b,d)}});c.extend({get:function(a,b,d,f){if(c.isFunction(b)){f=f||d;d=b;b=null}return c.ajax({type:""GET"",url:a,data:b,success:d,dataType:f})},getScript:function(a,b){return c.get(a,null,b,""script"")},getJSON:function(a,b,d){return c.get(a,b,d,""json"")},post:function(a,b,d,f){if(c.isFunction(b)){f=f||d;d=b;b={}}return c.ajax({type:""POST"",url:a,data:b,success:d,dataType:f})},ajaxSetup:function(a){c.extend(c.ajaxSettings,a)},ajaxSettings:{url:location.href,
global:true,type:""GET"",contentType:""application/x-www-form-urlencoded"",processData:true,async:true,xhr:A.XMLHttpRequest&&(A.location.protocol!==""file:""||!A.ActiveXObject)?function(){return new A.XMLHttpRequest}:function(){try{return new A.ActiveXObject(""Microsoft.XMLHTTP"")}catch(a){}},accepts:{xml:""application/xml, text/xml"",html:""text/html"",script:""text/javascript, application/javascript"",json:""application/json, text/javascript"",text:""text/plain"",_default:""*/*""}},lastModified:{},etag:{},ajax:function(a){function b(){e.success&&
e.success.call(k,o,i,x);e.global&&f(""ajaxSuccess"",[x,e])}function d(){e.complete&&e.complete.call(k,x,i);e.global&&f(""ajaxComplete"",[x,e]);e.global&&!--c.active&&c.event.trigger(""ajaxStop"")}function f(q,p){(e.context?c(e.context):c.event).trigger(q,p)}var e=c.extend(true,{},c.ajaxSettings,a),j,i,o,k=a&&a.context||e,n=e.type.toUpperCase();if(e.data&&e.processData&&typeof e.data!==""string"")e.data=c.param(e.data,e.traditional);if(e.dataType===""jsonp""){if(n===""GET"")N.test(e.url)||(e.url+=(ka.test(e.url)?
""&"":""?"")+(e.jsonp||""callback"")+""=?"");else if(!e.data||!N.test(e.data))e.data=(e.data?e.data+""&"":"""")+(e.jsonp||""callback"")+""=?"";e.dataType=""json""}if(e.dataType===""json""&&(e.data&&N.test(e.data)||N.test(e.url))){j=e.jsonpCallback||""jsonp""+sb++;if(e.data)e.data=(e.data+"""").replace(N,""=""+j+""$1"");e.url=e.url.replace(N,""=""+j+""$1"");e.dataType=""script"";A[j]=A[j]||function(q){o=q;b();d();A[j]=w;try{delete A[j]}catch(p){}z&&z.removeChild(C)}}if(e.dataType===""script""&&e.cache===null)e.cache=false;if(e.cache===
false&&n===""GET""){var r=J(),u=e.url.replace(wb,""$1_=""+r+""$2"");e.url=u+(u===e.url?(ka.test(e.url)?""&"":""?"")+""_=""+r:"""")}if(e.data&&n===""GET"")e.url+=(ka.test(e.url)?""&"":""?"")+e.data;e.global&&!c.active++&&c.event.trigger(""ajaxStart"");r=(r=xb.exec(e.url))&&(r[1]&&r[1]!==location.protocol||r[2]!==location.host);if(e.dataType===""script""&&n===""GET""&&r){var z=s.getElementsByTagName(""head"")[0]||s.documentElement,C=s.createElement(""script"");C.src=e.url;if(e.scriptCharset)C.charset=e.scriptCharset;if(!j){var B=
false;C.onload=C.onreadystatechange=function(){if(!B&&(!this.readyState||this.readyState===""loaded""||this.readyState===""complete"")){B=true;b();d();C.onload=C.onreadystatechange=null;z&&C.parentNode&&z.removeChild(C)}}}z.insertBefore(C,z.firstChild);return w}var E=false,x=e.xhr();if(x){e.username?x.open(n,e.url,e.async,e.username,e.password):x.open(n,e.url,e.async);try{if(e.data||a&&a.contentType)x.setRequestHeader(""Content-Type"",e.contentType);if(e.ifModified){c.lastModified[e.url]&&x.setRequestHeader(""If-Modified-Since"",
c.lastModified[e.url]);c.etag[e.url]&&x.setRequestHeader(""If-None-Match"",c.etag[e.url])}r||x.setRequestHeader(""X-Requested-With"",""XMLHttpRequest"");x.setRequestHeader(""Accept"",e.dataType&&e.accepts[e.dataType]?e.accepts[e.dataType]+"", */*"":e.accepts._default)}catch(ga){}if(e.beforeSend&&e.beforeSend.call(k,x,e)===false){e.global&&!--c.active&&c.event.trigger(""ajaxStop"");x.abort();return false}e.global&&f(""ajaxSend"",[x,e]);var g=x.onreadystatechange=function(q){if(!x||x.readyState===0||q===""abort""){E||
d();E=true;if(x)x.onreadystatechange=c.noop}else if(!E&&x&&(x.readyState===4||q===""timeout"")){E=true;x.onreadystatechange=c.noop;i=q===""timeout""?""timeout"":!c.httpSuccess(x)?""error"":e.ifModified&&c.httpNotModified(x,e.url)?""notmodified"":""success"";var p;if(i===""success"")try{o=c.httpData(x,e.dataType,e)}catch(v){i=""parsererror"";p=v}if(i===""success""||i===""notmodified"")j||b();else c.handleError(e,x,i,p);d();q===""timeout""&&x.abort();if(e.async)x=null}};try{var h=x.abort;x.abort=function(){x&&h.call(x);
g(""abort"")}}catch(l){}e.async&&e.timeout>0&&setTimeout(function(){x&&!E&&g(""timeout"")},e.timeout);try{x.send(n===""POST""||n===""PUT""||n===""DELETE""?e.data:null)}catch(m){c.handleError(e,x,null,m);d()}e.async||g();return x}},handleError:function(a,b,d,f){if(a.error)a.error.call(a.context||a,b,d,f);if(a.global)(a.context?c(a.context):c.event).trigger(""ajaxError"",[b,a,f])},active:0,httpSuccess:function(a){try{return!a.status&&location.protocol===""file:""||a.status>=200&&a.status<300||a.status===304||a.status===
1223||a.status===0}catch(b){}return false},httpNotModified:function(a,b){var d=a.getResponseHeader(""Last-Modified""),f=a.getResponseHeader(""Etag"");if(d)c.lastModified[b]=d;if(f)c.etag[b]=f;return a.status===304||a.status===0},httpData:function(a,b,d){var f=a.getResponseHeader(""content-type"")||"""",e=b===""xml""||!b&&f.indexOf(""xml"")>=0;a=e?a.responseXML:a.responseText;e&&a.documentElement.nodeName===""parsererror""&&c.error(""parsererror"");if(d&&d.dataFilter)a=d.dataFilter(a,b);if(typeof a===""string"")if(b===
""json""||!b&&f.indexOf(""json"")>=0)a=c.parseJSON(a);else if(b===""script""||!b&&f.indexOf(""javascript"")>=0)c.globalEval(a);return a},param:function(a,b){function d(i,o){if(c.isArray(o))c.each(o,function(k,n){b||/\[\]$/.test(i)?f(i,n):d(i+""[""+(typeof n===""object""||c.isArray(n)?k:"""")+""]"",n)});else!b&&o!=null&&typeof o===""object""?c.each(o,function(k,n){d(i+""[""+k+""]"",n)}):f(i,o)}function f(i,o){o=c.isFunction(o)?o():o;e[e.length]=encodeURIComponent(i)+""=""+encodeURIComponent(o)}var e=[];if(b===w)b=c.ajaxSettings.traditional;
if(c.isArray(a)||a.jquery)c.each(a,function(){f(this.name,this.value)});else for(var j in a)d(j,a[j]);return e.join(""&"").replace(yb,""+"")}});var la={},Ab=/toggle|show|hide/,Bb=/^([+-]=)?([\d+-.]+)(.*)$/,W,va=[[""height"",""marginTop"",""marginBottom"",""paddingTop"",""paddingBottom""],[""width"",""marginLeft"",""marginRight"",""paddingLeft"",""paddingRight""],[""opacity""]];c.fn.extend({show:function(a,b){if(a||a===0)return this.animate(K(""show"",3),a,b);else{a=0;for(b=this.length;a<b;a++){var d=c.data(this[a],""olddisplay"");
this[a].style.display=d||"""";if(c.css(this[a],""display"")===""none""){d=this[a].nodeName;var f;if(la[d])f=la[d];else{var e=c(""<""+d+"" />"").appendTo(""body"");f=e.css(""display"");if(f===""none"")f=""block"";e.remove();la[d]=f}c.data(this[a],""olddisplay"",f)}}a=0;for(b=this.length;a<b;a++)this[a].style.display=c.data(this[a],""olddisplay"")||"""";return this}},hide:function(a,b){if(a||a===0)return this.animate(K(""hide"",3),a,b);else{a=0;for(b=this.length;a<b;a++){var d=c.data(this[a],""olddisplay"");!d&&d!==""none""&&c.data(this[a],
""olddisplay"",c.css(this[a],""display""))}a=0;for(b=this.length;a<b;a++)this[a].style.display=""none"";return this}},_toggle:c.fn.toggle,toggle:function(a,b){var d=typeof a===""boolean"";if(c.isFunction(a)&&c.isFunction(b))this._toggle.apply(this,arguments);else a==null||d?this.each(function(){var f=d?a:c(this).is("":hidden"");c(this)[f?""show"":""hide""]()}):this.animate(K(""toggle"",3),a,b);return this},fadeTo:function(a,b,d){return this.filter("":hidden"").css(""opacity"",0).show().end().animate({opacity:b},a,d)},
animate:function(a,b,d,f){var e=c.speed(b,d,f);if(c.isEmptyObject(a))return this.each(e.complete);return this[e.queue===false?""each"":""queue""](function(){var j=c.extend({},e),i,o=this.nodeType===1&&c(this).is("":hidden""),k=this;for(i in a){var n=i.replace(ia,ja);if(i!==n){a[n]=a[i];delete a[i];i=n}if(a[i]===""hide""&&o||a[i]===""show""&&!o)return j.complete.call(this);if((i===""height""||i===""width"")&&this.style){j.display=c.css(this,""display"");j.overflow=this.style.overflow}if(c.isArray(a[i])){(j.specialEasing=
j.specialEasing||{})[i]=a[i][1];a[i]=a[i][0]}}if(j.overflow!=null)this.style.overflow=""hidden"";j.curAnim=c.extend({},a);c.each(a,function(r,u){var z=new c.fx(k,j,r);if(Ab.test(u))z[u===""toggle""?o?""show"":""hide"":u](a);else{var C=Bb.exec(u),B=z.cur(true)||0;if(C){u=parseFloat(C[2]);var E=C[3]||""px"";if(E!==""px""){k.style[r]=(u||1)+E;B=(u||1)/z.cur(true)*B;k.style[r]=B+E}if(C[1])u=(C[1]===""-=""?-1:1)*u+B;z.custom(B,u,E)}else z.custom(B,u,"""")}});return true})},stop:function(a,b){var d=c.timers;a&&this.queue([]);
this.each(function(){for(var f=d.length-1;f>=0;f--)if(d[f].elem===this){b&&d[f](true);d.splice(f,1)}});b||this.dequeue();return this}});c.each({slideDown:K(""show"",1),slideUp:K(""hide"",1),slideToggle:K(""toggle"",1),fadeIn:{opacity:""show""},fadeOut:{opacity:""hide""}},function(a,b){c.fn[a]=function(d,f){return this.animate(b,d,f)}});c.extend({speed:function(a,b,d){var f=a&&typeof a===""object""?a:{complete:d||!d&&b||c.isFunction(a)&&a,duration:a,easing:d&&b||b&&!c.isFunction(b)&&b};f.duration=c.fx.off?0:typeof f.duration===
""number""?f.duration:c.fx.speeds[f.duration]||c.fx.speeds._default;f.old=f.complete;f.complete=function(){f.queue!==false&&c(this).dequeue();c.isFunction(f.old)&&f.old.call(this)};return f},easing:{linear:function(a,b,d,f){return d+f*a},swing:function(a,b,d,f){return(-Math.cos(a*Math.PI)/2+0.5)*f+d}},timers:[],fx:function(a,b,d){this.options=b;this.elem=a;this.prop=d;if(!b.orig)b.orig={}}});c.fx.prototype={update:function(){this.options.step&&this.options.step.call(this.elem,this.now,this);(c.fx.step[this.prop]||
c.fx.step._default)(this);if((this.prop===""height""||this.prop===""width"")&&this.elem.style)this.elem.style.display=""block""},cur:function(a){if(this.elem[this.prop]!=null&&(!this.elem.style||this.elem.style[this.prop]==null))return this.elem[this.prop];return(a=parseFloat(c.css(this.elem,this.prop,a)))&&a>-10000?a:parseFloat(c.curCSS(this.elem,this.prop))||0},custom:function(a,b,d){function f(j){return e.step(j)}this.startTime=J();this.start=a;this.end=b;this.unit=d||this.unit||""px"";this.now=this.start;
this.pos=this.state=0;var e=this;f.elem=this.elem;if(f()&&c.timers.push(f)&&!W)W=setInterval(c.fx.tick,13)},show:function(){this.options.orig[this.prop]=c.style(this.elem,this.prop);this.options.show=true;this.custom(this.prop===""width""||this.prop===""height""?1:0,this.cur());c(this.elem).show()},hide:function(){this.options.orig[this.prop]=c.style(this.elem,this.prop);this.options.hide=true;this.custom(this.cur(),0)},step:function(a){var b=J(),d=true;if(a||b>=this.options.duration+this.startTime){this.now=
this.end;this.pos=this.state=1;this.update();this.options.curAnim[this.prop]=true;for(var f in this.options.curAnim)if(this.options.curAnim[f]!==true)d=false;if(d){if(this.options.display!=null){this.elem.style.overflow=this.options.overflow;a=c.data(this.elem,""olddisplay"");this.elem.style.display=a?a:this.options.display;if(c.css(this.elem,""display"")===""none"")this.elem.style.display=""block""}this.options.hide&&c(this.elem).hide();if(this.options.hide||this.options.show)for(var e in this.options.curAnim)c.style(this.elem,
e,this.options.orig[e]);this.options.complete.call(this.elem)}return false}else{e=b-this.startTime;this.state=e/this.options.duration;a=this.options.easing||(c.easing.swing?""swing"":""linear"");this.pos=c.easing[this.options.specialEasing&&this.options.specialEasing[this.prop]||a](this.state,e,0,1,this.options.duration);this.now=this.start+(this.end-this.start)*this.pos;this.update()}return true}};c.extend(c.fx,{tick:function(){for(var a=c.timers,b=0;b<a.length;b++)a[b]()||a.splice(b--,1);a.length||
c.fx.stop()},stop:function(){clearInterval(W);W=null},speeds:{slow:600,fast:200,_default:400},step:{opacity:function(a){c.style(a.elem,""opacity"",a.now)},_default:function(a){if(a.elem.style&&a.elem.style[a.prop]!=null)a.elem.style[a.prop]=(a.prop===""width""||a.prop===""height""?Math.max(0,a.now):a.now)+a.unit;else a.elem[a.prop]=a.now}}});if(c.expr&&c.expr.filters)c.expr.filters.animated=function(a){return c.grep(c.timers,function(b){return a===b.elem}).length};c.fn.offset=""getBoundingClientRect""in s.documentElement?
function(a){var b=this[0];if(a)return this.each(function(e){c.offset.setOffset(this,a,e)});if(!b||!b.ownerDocument)return null;if(b===b.ownerDocument.body)return c.offset.bodyOffset(b);var d=b.getBoundingClientRect(),f=b.ownerDocument;b=f.body;f=f.documentElement;return{top:d.top+(self.pageYOffset||c.support.boxModel&&f.scrollTop||b.scrollTop)-(f.clientTop||b.clientTop||0),left:d.left+(self.pageXOffset||c.support.boxModel&&f.scrollLeft||b.scrollLeft)-(f.clientLeft||b.clientLeft||0)}}:function(a){var b=
this[0];if(a)return this.each(function(r){c.offset.setOffset(this,a,r)});if(!b||!b.ownerDocument)return null;if(b===b.ownerDocument.body)return c.offset.bodyOffset(b);c.offset.initialize();var d=b.offsetParent,f=b,e=b.ownerDocument,j,i=e.documentElement,o=e.body;f=(e=e.defaultView)?e.getComputedStyle(b,null):b.currentStyle;for(var k=b.offsetTop,n=b.offsetLeft;(b=b.parentNode)&&b!==o&&b!==i;){if(c.offset.supportsFixedPosition&&f.position===""fixed"")break;j=e?e.getComputedStyle(b,null):b.currentStyle;
k-=b.scrollTop;n-=b.scrollLeft;if(b===d){k+=b.offsetTop;n+=b.offsetLeft;if(c.offset.doesNotAddBorder&&!(c.offset.doesAddBorderForTableAndCells&&/^t(able|d|h)$/i.test(b.nodeName))){k+=parseFloat(j.borderTopWidth)||0;n+=parseFloat(j.borderLeftWidth)||0}f=d;d=b.offsetParent}if(c.offset.subtractsBorderForOverflowNotVisible&&j.overflow!==""visible""){k+=parseFloat(j.borderTopWidth)||0;n+=parseFloat(j.borderLeftWidth)||0}f=j}if(f.position===""relative""||f.position===""static""){k+=o.offsetTop;n+=o.offsetLeft}if(c.offset.supportsFixedPosition&&
f.position===""fixed""){k+=Math.max(i.scrollTop,o.scrollTop);n+=Math.max(i.scrollLeft,o.scrollLeft)}return{top:k,left:n}};c.offset={initialize:function(){var a=s.body,b=s.createElement(""div""),d,f,e,j=parseFloat(c.curCSS(a,""marginTop"",true))||0;c.extend(b.style,{position:""absolute"",top:0,left:0,margin:0,border:0,width:""1px"",height:""1px"",visibility:""hidden""});b.innerHTML=""<div style='position:absolute;top:0;left:0;margin:0;border:5px solid #000;padding:0;width:1px;height:1px;'><div></div></div><table style='position:absolute;top:0;left:0;margin:0;border:5px solid #000;padding:0;width:1px;height:1px;' cellpadding='0' cellspacing='0'><tr><td></td></tr></table>"";
a.insertBefore(b,a.firstChild);d=b.firstChild;f=d.firstChild;e=d.nextSibling.firstChild.firstChild;this.doesNotAddBorder=f.offsetTop!==5;this.doesAddBorderForTableAndCells=e.offsetTop===5;f.style.position=""fixed"";f.style.top=""20px"";this.supportsFixedPosition=f.offsetTop===20||f.offsetTop===15;f.style.position=f.style.top="""";d.style.overflow=""hidden"";d.style.position=""relative"";this.subtractsBorderForOverflowNotVisible=f.offsetTop===-5;this.doesNotIncludeMarginInBodyOffset=a.offsetTop!==j;a.removeChild(b);
c.offset.initialize=c.noop},bodyOffset:function(a){var b=a.offsetTop,d=a.offsetLeft;c.offset.initialize();if(c.offset.doesNotIncludeMarginInBodyOffset){b+=parseFloat(c.curCSS(a,""marginTop"",true))||0;d+=parseFloat(c.curCSS(a,""marginLeft"",true))||0}return{top:b,left:d}},setOffset:function(a,b,d){if(/static/.test(c.curCSS(a,""position"")))a.style.position=""relative"";var f=c(a),e=f.offset(),j=parseInt(c.curCSS(a,""top"",true),10)||0,i=parseInt(c.curCSS(a,""left"",true),10)||0;if(c.isFunction(b))b=b.call(a,
d,e);d={top:b.top-e.top+j,left:b.left-e.left+i};""using""in b?b.using.call(a,d):f.css(d)}};c.fn.extend({position:function(){if(!this[0])return null;var a=this[0],b=this.offsetParent(),d=this.offset(),f=/^body|html$/i.test(b[0].nodeName)?{top:0,left:0}:b.offset();d.top-=parseFloat(c.curCSS(a,""marginTop"",true))||0;d.left-=parseFloat(c.curCSS(a,""marginLeft"",true))||0;f.top+=parseFloat(c.curCSS(b[0],""borderTopWidth"",true))||0;f.left+=parseFloat(c.curCSS(b[0],""borderLeftWidth"",true))||0;return{top:d.top-
f.top,left:d.left-f.left}},offsetParent:function(){return this.map(function(){for(var a=this.offsetParent||s.body;a&&!/^body|html$/i.test(a.nodeName)&&c.css(a,""position"")===""static"";)a=a.offsetParent;return a})}});c.each([""Left"",""Top""],function(a,b){var d=""scroll""+b;c.fn[d]=function(f){var e=this[0],j;if(!e)return null;if(f!==w)return this.each(function(){if(j=wa(this))j.scrollTo(!a?f:c(j).scrollLeft(),a?f:c(j).scrollTop());else this[d]=f});else return(j=wa(e))?""pageXOffset""in j?j[a?""pageYOffset"":
""pageXOffset""]:c.support.boxModel&&j.document.documentElement[d]||j.document.body[d]:e[d]}});c.each([""Height"",""Width""],function(a,b){var d=b.toLowerCase();c.fn[""inner""+b]=function(){return this[0]?c.css(this[0],d,false,""padding""):null};c.fn[""outer""+b]=function(f){return this[0]?c.css(this[0],d,false,f?""margin"":""border""):null};c.fn[d]=function(f){var e=this[0];if(!e)return f==null?null:this;if(c.isFunction(f))return this.each(function(j){var i=c(this);i[d](f.call(this,j,i[d]()))});return""scrollTo""in
e&&e.document?e.document.compatMode===""CSS1Compat""&&e.document.documentElement[""client""+b]||e.document.body[""client""+b]:e.nodeType===9?Math.max(e.documentElement[""client""+b],e.body[""scroll""+b],e.documentElement[""scroll""+b],e.body[""offset""+b],e.documentElement[""offset""+b]):f===w?c.css(e,d):this.css(d,typeof f===""string""?f:f+""px"")}});A.jQuery=A.$=c})(window);
";
        #endregion
        #region jQuery template
        const string jquery_tmpl_min_js = @"(function(a){var r=a.fn.domManip,d=""_tmplitem"",q=/^[^<]*(<[\w\W]+>)[^>]*$|\{\{\! /,b={},f={},e,p={key:0,data:{}},h=0,c=0,l=[];function g(e,d,g,i){var c={data:i||(d?d.data:{}),_wrap:d?d._wrap:null,tmpl:null,parent:d||null,nodes:[],calls:u,nest:w,wrap:x,html:v,update:t};e&&a.extend(c,e,{nodes:[],parent:d});if(g){c.tmpl=g;c._ctnt=c._ctnt||c.tmpl(a,c);c.key=++h;(l.length?f:b)[h]=c}return c}a.each({appendTo:""append"",prependTo:""prepend"",insertBefore:""before"",insertAfter:""after"",replaceAll:""replaceWith""},function(f,d){a.fn[f]=function(n){var g=[],i=a(n),k,h,m,l,j=this.length===1&&this[0].parentNode;e=b||{};if(j&&j.nodeType===11&&j.childNodes.length===1&&i.length===1){i[d](this[0]);g=this}else{for(h=0,m=i.length;h<m;h++){c=h;k=(h>0?this.clone(true):this).get();a.fn[d].apply(a(i[h]),k);g=g.concat(k)}c=0;g=this.pushStack(g,f,i.selector)}l=e;e=null;a.tmpl.complete(l);return g}});a.fn.extend({tmpl:function(d,c,b){return a.tmpl(this[0],d,c,b)},tmplItem:function(){return a.tmplItem(this[0])},template:function(b){return a.template(b,this[0])},domManip:function(d,l,j){if(d[0]&&d[0].nodeType){var f=a.makeArray(arguments),g=d.length,i=0,h;while(i<g&&!(h=a.data(d[i++],""tmplItem"")));if(g>1)f[0]=[a.makeArray(d)];if(h&&c)f[2]=function(b){a.tmpl.afterManip(this,b,j)};r.apply(this,f)}else r.apply(this,arguments);c=0;!e&&a.tmpl.complete(b);return this}});a.extend({tmpl:function(d,h,e,c){var j,k=!c;if(k){c=p;d=a.template[d]||a.template(null,d);f={}}else if(!d){d=c.tmpl;b[c.key]=c;c.nodes=[];c.wrapped&&n(c,c.wrapped);return a(i(c,null,c.tmpl(a,c)))}if(!d)return[];if(typeof h===""function"")h=h.call(c||{});e&&e.wrapped&&n(e,e.wrapped);j=a.isArray(h)?a.map(h,function(a){return a?g(e,c,d,a):null}):[g(e,c,d,h)];return k?a(i(c,null,j)):j},tmplItem:function(b){var c;if(b instanceof a)b=b[0];while(b&&b.nodeType===1&&!(c=a.data(b,""tmplItem""))&&(b=b.parentNode));return c||p},template:function(c,b){if(b){if(typeof b===""string"")b=o(b);else if(b instanceof a)b=b[0]||{};if(b.nodeType)b=a.data(b,""tmpl"")||a.data(b,""tmpl"",o(b.innerHTML));return typeof c===""string""?(a.template[c]=b):b}return c?typeof c!==""string""?a.template(null,c):a.template[c]||a.template(null,q.test(c)?c:a(c)):null},encode:function(a){return(""""+a).split(""<"").join(""&lt;"").split("">"").join(""&gt;"").split('""').join(""&#34;"").split(""'"").join(""&#39;"")}});a.extend(a.tmpl,{tag:{tmpl:{_default:{$2:""null""},open:""if($notnull_1){_=_.concat($item.nest($1,$2));}""},wrap:{_default:{$2:""null""},open:""$item.calls(_,$1,$2);_=[];"",close:""call=$item.calls();_=call._.concat($item.wrap(call,_));""},each:{_default:{$2:""$index, $value""},open:""if($notnull_1){$.each($1a,function($2){with(this){"",close:""}});}""},""if"":{open:""if(($notnull_1) && $1a){"",close:""}""},""else"":{_default:{$1:""true""},open:""}else if(($notnull_1) && $1a){""},html:{open:""if($notnull_1){_.push($1a);}""},""="":{_default:{$1:""$data""},open:""if($notnull_1){_.push($.encode($1a));}""},""!"":{open:""""}},complete:function(){b={}},afterManip:function(f,b,d){var e=b.nodeType===11?a.makeArray(b.childNodes):b.nodeType===1?[b]:[];d.call(f,b);m(e);c++}});function i(e,g,f){var b,c=f?a.map(f,function(a){return typeof a===""string""?e.key?a.replace(/(<\w+)(?=[\s>])(?![^>]*_tmplitem)([^>]*)/g,""$1 ""+d+'=""'+e.key+'"" $2'):a:i(a,e,a._ctnt)}):e;if(g)return c;c=c.join("""");c.replace(/^\s*([^<\s][^<]*)?(<[\w\W]+>)([^>]*[^>\s])?\s*$/,function(f,c,e,d){b=a(e).get();m(b);if(c)b=j(c).concat(b);if(d)b=b.concat(j(d))});return b?b:j(c)}function j(c){var b=document.createElement(""div"");b.innerHTML=c;return a.makeArray(b.childNodes)}function o(b){return new Function(""jQuery"",""$item"",""var $=jQuery,call,_=[],$data=$item.data;with($data){_.push('""+a.trim(b).replace(/([\\'])/g,""\\$1"").replace(/[\r\t\n]/g,"" "").replace(/\$\{([^\}]*)\}/g,""{{= $1}}"").replace(/\{\{(\/?)(\w+|.)(?:\(((?:[^\}]|\}(?!\}))*?)?\))?(?:\s+(.*?)?)?(\(((?:[^\}]|\}(?!\}))*?)\))?\s*\}\}/g,function(m,l,j,d,b,c,e){var i=a.tmpl.tag[j],h,f,g;if(!i)throw""Template command not found: ""+j;h=i._default||[];if(c&&!/\w$/.test(b)){b+=c;c=""""}if(b){b=k(b);e=e?"",""+k(e)+"")"":c?"")"":"""";f=c?b.indexOf(""."")>-1?b+c:""(""+b+"").call($item""+e:b;g=c?f:""(typeof(""+b+"")==='function'?(""+b+"").call($item):(""+b+""))""}else g=f=h.$1||""null"";d=k(d);return""');""+i[l?""close"":""open""].split(""$notnull_1"").join(b?""typeof(""+b+"")!=='undefined' && (""+b+"")!=null"":""true"").split(""$1a"").join(g).split(""$1"").join(f).split(""$2"").join(d?d.replace(/\s*([^\(]+)\s*(\((.*?)\))?/g,function(d,c,b,a){a=a?"",""+a+"")"":b?"")"":"""";return a?""(""+c+"").call($item""+a:d}):h.$2||"""")+""_.push('""})+""');}return _;"")}function n(c,b){c._wrap=i(c,true,a.isArray(b)?b:[q.test(b)?b:a(b).html()]).join("""")}function k(a){return a?a.replace(/\\'/g,""'"").replace(/\\\\/g,""\\""):null}function s(b){var a=document.createElement(""div"");a.appendChild(b.cloneNode(true));return a.innerHTML}function m(o){var n=""_""+c,k,j,l={},e,p,i;for(e=0,p=o.length;e<p;e++){if((k=o[e]).nodeType!==1)continue;j=k.getElementsByTagName(""*"");for(i=j.length-1;i>=0;i--)m(j[i]);m(k)}function m(j){var p,i=j,k,e,m;if(m=j.getAttribute(d)){while(i.parentNode&&(i=i.parentNode).nodeType===1&&!(p=i.getAttribute(d)));if(p!==m){i=i.parentNode?i.nodeType===11?0:i.getAttribute(d)||0:0;if(!(e=b[m])){e=f[m];e=g(e,b[i]||f[i],null,true);e.key=++h;b[h]=e}c&&o(m)}j.removeAttribute(d)}else if(c&&(e=a.data(j,""tmplItem""))){o(e.key);b[e.key]=e;i=a.data(j.parentNode,""tmplItem"");i=i?i.key:0}if(e){k=e;while(k&&k.key!=i){k.nodes.push(j);k=k.parent}delete e._ctnt;delete e._wrap;a.data(j,""tmplItem"",e)}function o(a){a=a+n;e=l[a]=l[a]||g(e,b[e.parent.key+n]||e.parent,null,true)}}}function u(a,d,c,b){if(!a)return l.pop();l.push({_:a,tmpl:d,item:this,data:c,options:b})}function w(d,c,b){return a.tmpl(a.template(d),c,b,this)}function x(b,d){var c=b.options||{};c.wrapped=d;return a.tmpl(a.template(b.tmpl),b.data,c,b.item)}function v(d,c){var b=this._wrap;return a.map(a(a.isArray(b)?b.join(""""):b).filter(d||""*""),function(a){return c?a.innerText||a.textContent:a.outerHTML||s(a)})}function t(){var b=this.nodes;a.tmpl(null,null,null,this).insertBefore(b[0]);a(b).remove()}})(jQuery)";
        #endregion
        #region base.js
        const string base_js = @"/**********************************************************
*                                                         *
*   © Microsoft. All rights reserved.                     *
*                                                         *
*   This library is intended for use in WWAs only.        *  
*                                                         *
**********************************************************/
// x86chk.fbl_pac_dev(chrisan) 
(function (global, rootNamespace, _undefined) {
    var expandProperties = function (properties, isStatic) {
        var expandedProperties = {};
        if (properties) {
            var keys = Object.keys(properties);
            for (var i = 0, len = keys.length; i < len; i++) {
                var name = keys[i],
                    property = properties[name],
                    propertyValue;

                // If the property name starts with an underscore, make it non-enumerable
                var isEnumerable = (name[0] !== '_');
                switch (typeof (property)) {
                    case ""object"":
                        if (property !== null && (property.value !== _undefined || typeof (property.get) === ""function"" || typeof (property.set) === ""function"")) {
                            if (property.enumerable === _undefined) {
                                property.enumerable = isEnumerable;
                            }
                            propertyValue = property;
                        } else {
                            propertyValue = { value: property, writable: !isStatic, enumerable: isEnumerable, configurable: false };
                        }
                        break;

                    case ""function"":
                        propertyValue = { value: property, writable: false, enumerable: isEnumerable, configurable: false };
                        break;

                    default:
                        propertyValue = { value: property, writable: !isStatic, enumerable: isEnumerable, configurable: false };
                        break;
                }

                expandedProperties[name] = propertyValue;
            }
        }

        return expandedProperties;
    };

    var constant = function (value) {
        return { value: value, writable: false /* WOOB: 1126722, this shouldn't be needed */ };
    };

    // Create the rootNamespace in the global namespace
    if (!global[rootNamespace]) {
        global[rootNamespace] = Object.create(null);
    }

    // Cache the rootNamespace we just created in a local variable
    var _rootNamespace = global[rootNamespace];
    if (!_rootNamespace.Namespace) {
        _rootNamespace.Namespace = Object.create(null);
    }

    // Establish members of the ""Win.Namespace"" namespace
    Object.defineProperties(_rootNamespace.Namespace, {
        defineWithParent: constant(
            function (parentNamespace, name, members) {
                /// <summary>
                /// Defines a new namespace with the specified name, under the specified parent namespace. 
                /// </summary>
                /// <param name='parentNamespace'>
                /// The parent namespace which will contain the new namespace.
                /// </param>
                /// <param name='name'>
                /// Name of the new namespace.
                /// </param>
                /// <param name='parentNamespace'>
                /// Members in the new namespace.
                /// </param>
                /// <returns>
                /// The newly defined namespace.
                /// </returns>
                var currentNamespace = parentNamespace,
                    namespaceFragments = name.split(""."");
                for (var i = 0, len = namespaceFragments.length; i < len; i++) {
                    var namespaceName = namespaceFragments[i];
                    if (!currentNamespace[namespaceName]) {
                        Object.defineProperty(currentNamespace, namespaceName, { value: Object.create(Object.prototype), writable: false, enumerable: true, configurable: true });
                    }
                    currentNamespace = currentNamespace[namespaceName];
                }

                if (members) {
                    var newProperties = expandProperties(members, true);
                    Object.defineProperties(currentNamespace, newProperties);
                }

                return currentNamespace;
            }
        ),

        define: constant(
            function (name, members) {
                /// <summary>
                /// Defines a new namespace with the specified name.
                /// </summary>
                /// <param name='name'>
                /// Name of the namespace.  This could be a dot-separated nested name.
                /// </param>
                /// <param name='parentNamespace'>
                /// Members in the new namespace.
                /// </param>
                /// <returns>
                /// The newly defined namespace.
                /// </returns>
                return this.defineWithParent(global, name, members);
            }
        )
    });

    // Establish members of ""Win.Class"" namespace
    _rootNamespace.Namespace.defineWithParent(_rootNamespace, ""Class"", {
        _objectFromProperties: function (baseClass, properties, constructor, statics) {
            if (typeof (constructor) !== ""function"") {
                throw ""Constructors have to be functions."";
            }

            var outerObj = constructor,
                expandedProperties = expandProperties(properties, false);
            expandedProperties._super = { value: baseClass.prototype, writable: false };

            Object.defineProperty(outerObj, ""prototype"", { value : Object.create(baseClass.prototype, expandedProperties) });
            Object.defineProperties(outerObj, expandProperties(statics, true));

            return outerObj;
        },

        define: function (baseClass, properties, constructor, statics) {
            /// <summary>
            /// Defines a new class derived from the baseClass, with the specified properties and constructors.  
            /// The statics will be available as top-level members on the Class object.
            /// </summary>
            /// <param name='baseClass'>
            /// The class to inherit from.
            /// </param>
            /// <param name='properties'>
            /// The set of new properties on the new class.
            /// </param>
            /// <param name='constructor'>
            /// A constructor function that can instantiate this class.
            /// </param>
            /// <param name='statics'>
            /// A set of static members to be attached to the top-level class object.
            /// </param>
            /// <returns>
            /// The newly defined class.
            /// </returns>
            return this._objectFromProperties(
                baseClass || _rootNamespace.Class,
                properties,
                constructor || function () { },
                statics);
        },

        prototype: {}
    });
})(this, ""Win"");

(function (global, Win, _undefined) {

    // Establish members of ""Win.Utilities"" namespace
    Win.Namespace.defineWithParent(Win, ""Utilities"", {
        /// <summary>
        /// Gets the leaf-level type or namespace as specified by the name.
        /// </summary>
        /// <param name='name'>
        /// The name of the member.
        /// </param>
        /// <returns>
        /// The leaf-level type of namespace inside the specified parent namespace.
        /// </returns>
        getMember: function (name) {
            if (!name) {
                return null;
            }

            return name.split(""."").reduce(function (currentNamespace, name) {
                if (currentNamespace) {
                    return currentNamespace[name];
                }
                return null;
            }, global);
        },

        /// <summary>
        /// Returns a merged namespace object from all the namespaces passed in.
        /// </summary>
        /// <returns>
        /// A merged namespace object.
        /// </returns>
        // UNDONE: this would be implemented by a native construct in Eze to avoid
        // the performance implications of this eager model.
        merge: function () {
            var merged = {};
            var addProperty = function (namespaceObject) {
                Object.keys(namespaceObject).forEach(function (memberName) {
                    Object.defineProperty(merged, memberName, {
                        get: function () { return namespaceObject[memberName]; },
                        set: function (value) { namespaceObject[memberName] = value; }
                    });
                });
            };

            // arguments is an ""array-like"" structure, you can't use forEach on it,
            // so we simulate it here to ensure we get the right closure semantics.
            for (var i = 0, len = arguments.length; i < len; i++) {
                addProperty(arguments[i]);
            }

            return merged;
        },
        
        /// <summary>
        /// Ensures the given function only executes after the DOMContentLoaded event has fired
        /// for the current page.
        /// </summary>
        /// <param name=""callback"">
        /// A JS Function to execute after DOMContentLoaded has fired.
        /// </param>
        /// <param name=""async"">
        /// If true then the callback should be asynchronously executed.
        /// </param>
        executeAfterDomLoaded: function(callback, async) {
        
            var readyState = this.testReadyState || document.readyState;
            
            if(readyState === ""complete"" || readyState === ""interactive"") {
              if(async) {
                  window.setTimeout(function () { callback(); }, 0);
              }
              else {
                  callback();
              }
            }
            else {
              window.addEventListener(""DOMContentLoaded"", callback, false);
            }
        }
    });

    // Promote ""merge"".  This is one place where we modify the global namespace.
    if (!global.merge) {
        global.merge = Win.Utilities.merge;
    }
})(this, Win);

";

        #endregion
        #region ui.js
        const string ui_js = @"/**********************************************************
*                                                         *
*   © Microsoft. All rights reserved.                     *
*                                                         *
*   This library is intended for use in WWAs only.        *  
*                                                         *
**********************************************************/
// x86chk.fbl_pac_dev(chrisan) 
// Copyright (c) Microsoft Corporation
// All rights reserved

/// <reference path=""../base/_es3.js"" />
/// <reference path=""../base/base.js"" />

(function (Win, undefined) {
    Win.Namespace.defineWithParent(Win, ""Controls"", {
        Control: Win.Class.define(null, {
            _domElement: null,

            addEventListener: function (type, listener, useCapture) {
                /// <summary>
                /// Adds an event listener to the control.
                /// </summary>
                /// <param name='type'>
                /// The type (name) of the event.
                /// </param>
                /// <param name='listener'>
                /// The listener to invoke when the event gets raised.
                /// </param>
                /// <param name='useCapture'>
                /// Specifies whether or not to initiate capture.
                /// </param>
                if (this._domElement) {
                    this._domElement.addEventListener(type, listener, useCapture);
                }
            },

            raiseEvent: function (type, eventProperties) {
                /// <summary>
                /// Raises an event of the specified type and with additional properties.
                /// </summary>
                /// <param name='type'>
                /// The type (name) of the event.
                /// </param>
                /// <param name='eventProperties'>
                /// The set of additional properties to be attached to the event object when the event is raised.
                /// </param>
                if (this._domElement) {
                    var customEvent = document.createEvent(""Event"");
                    customEvent.initEvent(type, false, false);

                    if (eventProperties) {
                        var keys = Object.keys(eventProperties);
                        for (var i = 0; i < keys.length; i++) {
                            var name = keys[i];
                            var value = eventProperties[name];

                            customEvent[name] = value;
                        }
                    }
                    this._domElement.dispatchEvent(customEvent);
                }
            },

            removeEventListener: function (type, listener, useCapture) {
                /// <summary>
                /// Removes an event listener from the control.
                /// </summary>
                /// <param name='type'>
                /// The type (name) of the event.
                /// </param>
                /// <param name='listener'>
                /// The listener to remove from the invoke list.
                /// </param>
                /// <param name='useCapture'>
                /// Specifies whether or not to initiate capture.
                /// </param>
                if (this._domElement) {
                    this._domElement.removeEventListener(type, listener, useCapture);
                }
            },

            setOptions: function (options) {
                /// <summary>
                /// Applies the set of declaratively specified options (properties and events) on the specified control.
                /// </summary>
                /// <param name='control' domElement='false'>
                /// The control on which the properties and events are to be applied.
                /// </param>
                /// <param name='options' domElement='false'>
                /// The set of options that were specified declaratively.
                /// </param>
                if (options) {
                    var keys = Object.keys(options);
                    for (var i = 0; i < keys.length; i++) {
                        var name = keys[i];
                        var value = options[name];

                        // Look for an event
                        if (this._domElement &&
                            name.length > 2 && name.substr(0, 2).toUpperCase() == ""ON"" &&
                            typeof (value) === ""function"") {

                            this.addEventListener(name.substr(2), value);
                        }
                        else {
                            this[name] = value;
                        }
                    }
                }
            },
        })
    });
})(Win);
// Copyright (c) Microsoft Corporation
// All rights reserved

/// <reference path=""../base/_es3.js"" />
/// <reference path=""../base/base.js"" />
/// <reference path=""elementUtilities.js"" />
/// <reference path=""control.js"" />

(function (Win, undefined) {
    Win.Namespace.defineWithParent(Win, ""Controls"", {
        _templateProcessingDataKey: ""templateProcessingData"",

        DataTemplate: Win.Class.define(Win.Controls.Control, {
            _templateNode: null,
            _templateData: null,
            _regExp: /\{(\w+)\}/g,
            _regExpTest: /\{(\w+)\}/,
            _dataTransform: null,

            _applyData: function (templateData, element, data) {
                if (!templateData) {
                    return;
                }

                if (this.dataTransform) {
                    data = this.dataTransform(data);
                }

                if (templateData.processTextData) {
                    element.data = this._transformText(element.data, data);
                } else {
                    for (var i = 0; i < templateData.attributes.length; i++) {
                        var attribute = element.attributes[templateData.attributes[i]];
                        attribute.value = this._transformText(attribute.value, data);
                    }

                    var keys = Object.keys(templateData.children);
                    for (var i = 0; i < keys.length; i++) {
                        var childIndex = keys[i];
                        var childTemplateData = templateData.children[childIndex];
                        this._applyData(childTemplateData, element.childNodes[childIndex], data);
                    }
                }
            },

            _constructTemplateData: function (element) {
                var processingData = Win.Utilities.getData(element, Win.Controls._templateProcessingDataKey);
                if (processingData !== undefined) {
                    return processingData;
                }

                // We pre-process the element to detect and mark interesting tree-traversals.
                // These paths are stored as indices into attributes and child nodes.  This 
                // makes applyData fast, but also makes it fragile if people modify the visual
                // tree of the template using, say, innerHTML property.
                var requiresProcessing = false;
                processingData = { processTextData: false, children: {}, attributes: [] };

                if (element instanceof Text && this._regExpTest.test(element.data)) {
                    processingData.processTextData = true;
                    requiresProcessing = true;
                }
                else if (element instanceof HTMLElement) {
                    for (var i = 0, len = element.attributes.length; i < len; i++) {
                        var attribute = element.attributes.item(i);
                        if (this._regExpTest.test(attribute.value)) {
                            processingData.attributes.push(i);
                            requiresProcessing = true;
                        }
                    }

                    for (var i = 0, len = element.childNodes.length; i < len; i++) {
                        var childData = this._constructTemplateData(element.childNodes.item(i));
                        if (childData) {
                            requiresProcessing = true;
                            processingData.children[i] = childData;
                        }
                    }
                }

                if (requiresProcessing) {
                    Win.Utilities.setData(element, Win.Controls._templateProcessingDataKey, processingData);
                    return processingData;
                } else {
                    Win.Utilities.setData(element, Win.Controls._templateProcessingDataKey, null);
                    return null;
                }
            },

            createElement: function (data) {
                var newElement = this._templateNode.cloneNode(true);
                this._applyData(this._templateData, newElement, data);

                Win.Controls.processAll(newElement, function () { }, data);
                Win.Utilities.removeClass(newElement, ""ms-hidden"");
                return newElement;
            },

            dataTransform: {
                get: function () { return this._dataTransform; },
                set: function (value) {
                    if (value && typeof (value) !== ""function"") {
                        throw ""dataTransform should be a function."";
                    }
                    this._dataTransform = value;
                }
            },

            setElement: function (element) {
                this._templateNode = element.cloneNode(true);

                this._templateNode.setAttribute(""id"", """");
                this._templateData = this._constructTemplateData(element);
                Win.Utilities.addClass(element, ""ms-hidden"");

                // Attach a renderItem function that will be used by Items Control.
                var that = this;
                element.renderItem = function (getIndex, key, data, itemId) {
                    return that.createElement(data);
                };
            },

            _transformText: function (text, data) {
                // Reset this so the matching will begin from the start.
                this._regExp.lastIndex = 0;

                var keys;
                var newText = text;

                while ((keys = this._regExp.exec(text))) {
                    var keyName = keys[1];
                    newText = newText.replace(keys[0], data[keyName]);
                }

                return newText;
            }
        },
        function (element, options) {
            if (!(this instanceof Win.Controls.DataTemplate)) {
                return new Win.Controls.DataTemplate(element, options);
            }
            this.setElement(element);
            this.setOptions(options);
        },
        {
            applyTemplate: function (templateElement, data) {
                var template = Win.Controls.getControl(templateElement);
                if (!template) {
                    template = Win.Controls.process(templateElement);
                }
                if (!template) {
                    template = new Win.Controls.DataTemplate(templateElement);
                }

                return template.createElement(data);
            },
            applyTemplateAndAddToContainer: function (containerElement, templateElement, dataArray) {
                var template = Win.Controls.getControl(templateElement);
                if (!template) {
                    template = Win.Controls.process(templateElement);
                }
                if (!template) {
                    template = new Win.Controls.DataTemplate(templateElement);
                }

                if (typeof dataArray.forEach === ""function"") {
                    dataArray.forEach(function (data) {
                        containerElement.appendChild(template.createElement(data));
                    });
                }
            }
        })
    })
})(Win);// Copyright (c) Microsoft Corporation
// All rights reserved

/// <reference path=""../base/_es3.js"" />
/// <reference path=""../base/base.js"" />
/// <reference path=""elementUtilities.js"" />

var InvalidHandler = ""Invalid data-ms-control attribute"";

(function (Win, undefined) {
    Win.Namespace.defineWithParent(Win, ""Controls"", {
        _keyValueRegEx: /\s*([A-Za-z_\$][\w\$]*|\'[^\']*\'|\""[^\""]*\"")\s*\:\s*((-\d*\.\d*|\+\d*\.\d*|\d*\.\d*)|(-\d+|\+\d+|\d+)|\'([^\']*)\'|\""([^\""]*)\""|([A-Za-z_\$][\w\.\$]*|\'[^\']*\'|\""[^\""]*\""))\s*,?/g,
        // -----------------|Identifier                              ||Colon | |Float Number                 | |Integer       | |Sngl & dbl qte string  | |Identifier                                 |Comma | 

        _evaluateSymbol: function (symbol) {
            return Win.Utilities.getMember(symbol);
        },

        _getControlHandler: function (element) {
            var evaluator = element.getAttribute(""data-ms-control"");
            if (evaluator) {
                var handler = Win.Utilities.getMember(evaluator);
                if (!handler) {
                    throw InvalidHandler;
                }
                return handler;
            }
        },

        _optionsFromElement: function (element) {
            var result = {};
            var optionsAttribute = element.getAttribute(""data-ms-options"");
            if (optionsAttribute) {
                result = this._parseOptionsString(optionsAttribute);
            }
            return result;
        },

        _parseOptionsString: function (optionsString) {
            var obj = {};
            var result = null;
            while ((result = this._keyValueRegEx.exec(optionsString))) {
                var key = result[1];
                if (key.length) {
                    var firstChar = key[0];
                    if ((firstChar == '""' || firstChar == ""'"") && (key[key.length - 1] == firstChar)) {
                        key = key.substring(1, key.length - 1);
                    }
                }

                var value = undefined;
                if (result[3])
                    value = parseFloat(result[3]);
                else if (result[4])
                    value = parseInt(result[4]);
                else if (result[5])
                    value = result[5];
                else if (result[6])
                    value = result[6];
                else if (result[7]) {
                    var miscValue = result[7];
                    if (miscValue == ""true"")
                        value = true;
                    else if (miscValue == ""false"")
                        value = false;
                    else if (miscValue == ""null"")
                        value = null;
                    else 
                        value = this._evaluateSymbol(miscValue);
                }

                obj[key] = value;
            }

            return obj;
        },

        getControl: function (element) {
            /// <summary>
            /// Given a DOM element, retrieves the associated Control.
            /// </summary>
            /// <param name='element' domElement='true'>
            /// Element whose associated Control is requested.
            /// </param>
            /// <returns>
            /// The control associated with the dom element.
            /// </returns>
            return Win.Utilities.getData(element, ""declControl"");
        },

        processAll: function (rootElement, complete, dataContext) {
            /// <summary>
            /// Applies declarative control binding to all elements, starting optionally at rootElement.
            /// </summary>
            /// <param name='rootElement' domElement='true'>
            /// Element to start searching at, if not specified, the entire document is searched.
            /// </param>
            
            var that = this;
            var processAllImpl = function(rootElement, complete, dataContext) {
                rootElement = rootElement || document.body;
                var pending = 0;
                var controls = rootElement.querySelectorAll(""[data-ms-control]"");
                var checkAllComplete = undefined;
                if (complete) {
                    checkAllComplete = function () { 
                        pending = pending - 1;
                        if (pending < 0) { 
                            complete(); 
                        } 
                    };
                }
 
                pending++;
                that.process(rootElement, checkAllComplete, dataContext);
 
                for (var i = 0, len = controls.length; i < len; i++) {
                    var element = controls[i];
                    if (!Win.Controls.getControl(element, ""declControl"")) {
                        pending = pending + 1;
                        that.process(element, checkAllComplete, dataContext);
                    }
                }
 
                if (checkAllComplete) {
                    checkAllComplete();
                }
            }
            
            Win.Utilities.executeAfterDomLoaded(function() { processAllImpl(rootElement, complete, dataContext); }, false);
        },

        process: function (element, complete, dataContext) {
            /// <summary>
            /// Applies declarative control binding to the specified element.
            /// </summary>
            /// <param name='element' domElement='true'>
            /// Element to bind.
            /// </param>
            var handler = this._getControlHandler(element);
            if (!handler) {
                // Need to call complete even if there is nothing to handle
                if (complete) {
                    complete();
                }
                return;
            }
 
            var that = this;
            var optionsGenerator = handler.optionsGenerator || function (element) { return that._optionsFromElement(element); };
 
            var temp = window[""dataContext""];
            var options;
            window[""dataContext""] = dataContext;
            try {
                options = optionsGenerator(element);
            } 
            finally {
                window[""dataContext""] = temp;
            }
 
            // handler is required to call complete if it takes that parameter
            var ctl = handler(element, options, complete);
            if (complete && handler.length < 3) {
                complete();
            }
 
            return Win.Utilities.setData(element, ""declControl"", ctl);
        }
    });
})(Win);
// Copyright (c) Microsoft Corporation
// All rights reserved

/// <reference path=""../base/_es3.js"" />
/// <reference path=""../base/base.js"" />
/// <reference path=""elementUtilities.js"" />

(function (Win, undefined) {

    
    var QueryCollection = Win.Class.define(Array, 
        {
            addClass: function(name) {
                this.forEach(function(item) {
                  Win.Utilities.addClass(item, name);
                });
                return this;
            },
            hasClass: function(name) {
                if(this.length > 0) {
                  return Win.Utilities.hasClass(this[0], name);
                }
                return false;
            },
            removeClass: function(name) {
                this.forEach(function(item) {
                  Win.Utilities.removeClass(item, name);
                });
                return this;
            },
            toggleClass: function(name) {
                this.forEach(function(item) {
                  Win.Utilities.toggleClass(item, name);
                });
                return this;
            },
            addEventListener: function(eventType, listener, capture) {
                this.forEach(function(item) {
                  item.addEventListener(eventType, listener, capture);
                });
                return this;
            },
            removeEventListener: function(eventType, listener, capture) {
                this.forEach(function(item) {
                  item.removeEventListener(eventType, listener, capture);
                });
                return this;
            },
            setStyle: function(name, value) {
                this.forEach(function(item) {
                  item.style[name] = value;
                });
                return this;
            },
            clearStyle: function(name, value) {
                this.forEach(function(item) {
                  item.style[name] = """";
                });
                return this;
            },
            query: function(query) {
                 var newCollection = new QueryCollection();
                 this.forEach(function(item) {
                    newCollection.include(item.querySelectorAll(query)); 
                 });
                 return newCollection;
             },
             include: function(items) {
                 for(var i = 0; i < items.length; i++) {
                     this.push(items[i]);
                 }
             },
        }, 
        function(items) {
            if(items) {
                this.include(items);
            }
        });
   
    Win.Namespace.defineWithParent(Win, ""Utilities"", {
        query: function (query, element) {
            return new QueryCollection((element || document).querySelectorAll(query));
        }
    });
})(Win);// Copyright (c) Microsoft Corporation
// All rights reserved

/// <reference path=""../base/_es3.js"" />
/// <reference path=""../base/base.js"" />

(function (Win, undefined) {
    function getClassName(e) {
        var name = e.className || """";
        if (typeof(name) == ""string"") {
            return name;
        }
        else {
            return name.baseVal || """";
        }
    };
    function setClassName(e, value) {
        // SVG elements (which use e.className.baseVal) are never undefined, 
        // so this logic makes the comparison a bit more compact.
        //
        var name = e.className || """";
        if (typeof(name) == ""string"") {
            e.className = value;
        }
        else {
            e.className.baseVal = value;
        }
        return e;
    };

    Win.Namespace.defineWithParent(Win, ""Utilities"", {
        _dataKey: ""_msDataKey"",

        getData: function (element, key) {
            var data = element[Win.Utilities._dataKey] || {};
            return data[key];
        },

        setData: function (element, key, value) {
            var data = element[Win.Utilities._dataKey] || {};
            data[key] = value;
            element[Win.Utilities._dataKey] = data;
            return value;
        },

        hasClass: function (e, name) {
            var className = getClassName(e);
            var names = className.trim().split("" "");
            var l = names.length;
            for (var i = 0; i < l; i++) {
                if (names[i] == name) {
                    return true;
                }
            }
            return false;
        },

        addClass: function (e, name) {
            var className = getClassName(e);
            var names = className.trim().split("" "");
            var l = names.length;
            var found = false;
            for (var i = 0; i < l; i++) {
                if (names[i] == name) {
                    found = true;
                }
            }
            if (!found) {
                if (l > 0 && names[0].length > 0) {
                    setClassName(e, className + "" "" + name);
                }
                else {
                    setClassName(e, className + name);
                }
            }
        },

        removeClass: function (e, name) {
            var names = getClassName(e).trim().split("" "");
            setClassName(e, names.reduce(function (r, e) {
                if (e == name) {
                    return r;
                }
                else if (r && r.length > 0) {
                    return r + "" "" + e;
                }
                else {
                    return e;
                }
            }, """"));
        },

        toggleClass: function (e, name) {
            var className = getClassName(e);
            var names = className.trim().split("" "");
            var l = names.length;
            var found = false;
            for (var i = 0; i < l; i++) {
                if (names[i] == name) {
                    found = true;
                }
            }
            if (!found) {
                if (l > 0 && names[0].length > 0) {
                    setClassName(e, className + "" "" + name);
                }
                else {
                    setClassName(e, className + name);
                }
            }
            else {
                setClassName(e, names.reduce(function (r, e) {
                    if (e == name) {
                        return r;
                    }
                    else if (r && r.length > 0) {
                        return r + "" "" + e;
                    }
                    else {
                        return e;
                    }
                }, """"));
            }
        },

        getRelativeLeft: function (element, parent) {
            /// <summary>
            /// Gets the left coordinate of the element relative to the specified parent.
            /// </summary>
            /// <param name='element' domElement='true'>
            /// Element whose relative coordinate is needed.
            /// </param>
            /// <param name='parent' domElement='true'>
            /// Element to which the coordinate will be relative to.
            /// </param>
            /// <returns>
            /// Relative left co-ordinate.
            /// </returns>
            if (element === null)
                return 0;

            var left = element.offsetLeft;
            var e = element.parentNode;
            while (e !== null) {
                left -= e.offsetLeft;

                if (e === parent)
                    break;
                e = e.parentNode;
            }

            return left;
        },

        getRelativeTop: function (element, parent) {
            /// <summary>
            /// Gets the top coordinate of the element relative to the specified parent.
            /// </summary>
            /// <param name='element' domElement='true'>
            /// Element whose relative coordinate is needed.
            /// </param>
            /// <param name='parent' domElement='true'>
            /// Element to which the coordinate will be relative to.
            /// </param>
            /// <returns>
            /// Relative top co-ordinate.
            /// </returns>
            if (element === null)
                return 0;

            var top = element.offsetTop;
            var e = element.parentNode;
            while (e !== null) {
                top -= e.offsetTop;

                if (e === parent)
                    break;
                e = e.parentNode;
            }

            return top;
        },

        removeAllChildren: function (element) {
            /// <summary>
            /// Removes all the child nodes from the specified element.
            /// </summary>
            /// <param name='element' domElement='true'>
            /// The element whose child nodes will be removed.
            /// </param>
            for (var i = element.childNodes.length - 1; i >= 0; i--) {
                element.removeChild(element.childNodes.item(i));
            }
        },

        trackDragMove: function (element, mouseDown, mouseMove, mouseUp) {
            /// <summary>
            /// Signs the element for drag events and tracks the drag operation.
            /// </summary>
            /// <param name='element' domElement='true'>
            /// The element to track.
            /// </param>
            /// <param name='mouseDown'>
            /// The listener to call back when the mouseDown event arrives.
            /// </param>
            /// <param name='mouseMove'>
            /// The listener to call back when the mouseMove event arrives.
            /// </param>
            /// <param name='mouseUp'>
            /// The listener to call back when the mouseUp event arrives.
            /// </param>
            element.onmousedown = function (e) {
                if (mouseDown) {
                    mouseDown(e);
                }

                var moveHandler = function (e) {
                    if (mouseMove) {
                        mouseMove(e);
                    }

                    e.cancelBubble = true;
                    e.stopPropagation();
                };

                var upHandler = function (e) {
                    if (mouseUp) {
                        mouseUp(e);
                    }

                    if (element.releaseCapture) {
                        element.onmousemove = null;
                        element.onmouseup = null;
                        element.releaseCapture();
                    }
                    else {
                        window.removeEventListener(""mousemove"", moveHandler, true);
                        window.removeEventListener(""mouseup"", upHandler, true);
                    }
                };

                if (element.setCapture) {
                    element.setCapture();
                    element.onmousemove = moveHandler;
                    element.onmouseup = upHandler;
                }
                else {
                    window.addEventListener(""mousemove"", moveHandler, true);
                    window.addEventListener(""mouseup"", upHandler, true);
                }

                e.cancelBubble = true;
                e.stopPropagation();
            };
        },
    });
})(Win);
// Copyright (c) Microsoft Corporation
// All rights reserved

/// <reference path=""../base/_es3.js"" />
/// <reference path=""../base/base.js"" />

(function (Win, globalObj, undefined) {
    var loaderStateProp = ""-ms-fragmentLoader-state"";

    // UNDONE: should we hoist this to a shared location?
    //
    var forEach = function (arrayLikeValue, action) {
        for (var i = 0, l = arrayLikeValue.length; i < l; i++) {
            action(arrayLikeValue[i]);
        }
    };
    var head = document.head || document.getElementsByTagName(""head"")[0];

    Win.Namespace.defineWithParent(Win, ""Controls.FragmentLoader"", {
        _scripts: {},
        _styles: {},
        _links: {},
        _states: {},
        _initialized: { value: false, writable: true },

        _idFromHref: function (href) {
            if (typeof (href) == ""string"") {
                return href;
            }
            else {
                return href.id;
            }
        },

        _addScript: function (scriptTag, fragmentHref, position) {
            /// <summary>
            /// PRIVATE METHOD: Adds a script tag based on the data from the fragment to the host document.
            /// </summary>

            // We synthesize a name for inline scripts because today we put the 
            // inline scripts in the same processing pipeline as src scripts. If
            // we seperated inline scripts into their own logic, we could simplify
            // this somewhat.
            //
            var src = scriptTag.src;
            if (!src) {
                src = fragmentHref + ""script["" + position + ""]"";
            }

            if (!(src in Win.Controls.FragmentLoader._scripts)) {
                Win.Controls.FragmentLoader._scripts[src] = true;
                var n = document.createElement(""script"");
                if (scriptTag.language) {
                    n.setAttribute(""language"", ""javascript"");
                }
                if (scriptTag.type == ""ms-deferred/javascript"") {
                    n.setAttribute(""type"", ""text/javascript"");
                }
                else {
                    n.setAttribute(""type"", scriptTag.type);
                }
                if (scriptTag.id) {
                    n.setAttribute(""id"", scriptTag.id);
                }
                if (scriptTag.src) {
                    n.setAttribute(""src"", scriptTag.src);
                }
                else {
                    n.text = scriptTag.text;
                }
                head.appendChild(n);
            }
        },

        _addStyle: function (styleTag, fragmentHref, position) {
            /// <summary>
            /// PRIVATE METHOD: Adds a CSS link tag based on the data from the fragment to the host document.
            /// </summary>

            var src = fragmentHref + ""script["" + position + ""]"";
            if (!(styleTag.href in Win.Controls.FragmentLoader._styles)) {
                Win.Controls.FragmentLoader._styles[src] = true;
                var n = document.createElement(""style"");
                n.setAttribute(""type"", ""text/css"");
                n.innerText = styleTag.innerText;
                head.appendChild(n);
            }
        },

        _addLink: function (styleTag) {
            /// <summary>
            /// PRIVATE METHOD: Adds a CSS link tag based on the data from the fragment to the host document.
            /// </summary>

            if (!(styleTag.href in Win.Controls.FragmentLoader._links)) {
                Win.Controls.FragmentLoader._links[styleTag.href] = true;
                var n = document.createElement(""link"");
                n.setAttribute(""type"", ""text/css"");
                n.setAttribute(""rel"", styleTag.rel);
                n.setAttribute(""href"", styleTag.href);
                head.appendChild(n);
            }
        },

        _controlStaticState: function (href, success) {
            /// <summary>
            /// PRIVATE METHOD: retrieves the static (not per-instance) state for a fragment at the
            /// URL ""href"". ""success"" will be called either synchronously (for an already loaded
            /// fragment) or asynchronously when the fragment is loaded and ready to be used.
            /// </summary>
            var fragmentId = Win.Controls.FragmentLoader._idFromHref(href);

            var state = Win.Controls.FragmentLoader._states[fragmentId];

            var intervalId;
            var callback = function () {
                if (state.templateElement) {
                    if (state.loadScript) {
                        var load = globalObj[state.loadScript];
                        if (load) {
                            success(load, Win.Controls.FragmentLoader._states[fragmentId]);
                            if (intervalId) { clearInterval(intervalId); }
                            return true;
                        }
                    }
                    else {
                        success(undefined, Win.Controls.FragmentLoader._states[fragmentId]);
                        if (intervalId) { clearInterval(intervalId); }
                        return true;
                    }
                }
                return false;
            }


            // If the state record was found, then we either are ready to 
            // roll immediately (everything is loaded & parsed) or are in
            // process of loading. If possible, we want to directly invoke
            // to avoid any flickering, however if we are still loading
            // the content, we must wait.
            //
            if (state) {
                if (!callback()) {
                    intervalId = setInterval(callback, 20);
                }

                return;
            }
            else {
                Win.Controls.FragmentLoader._states[fragmentId] = state = {};
            }

            if (typeof (href) === ""string"") {
                var temp = document.createElement('iframe');
                document[loaderStateProp] = ""loading"";
                temp.src = href;
                temp.style.display = 'none';
                
                var domContentLoaded = null;

                var complete = function (load) {
                    // This is to work around a weird bug where removing the 
                    // IFrame from the DOM triggers DOMContentLoaded a second time.
                    temp.contentDocument.removeEventListener(""DOMContentLoaded"", domContentLoaded, false);
                    temp.parentNode.removeChild(temp);
                    delete temp;
                    delete document[loaderStateProp];
                    success(load, state);
                };
                
                domContentLoaded = function() {
                    Win.Controls.FragmentLoader._controlStaticStateLoaded(href, temp, state, complete);
                }
                

                document.body.appendChild(temp);
                temp.contentDocument.addEventListener(""DOMContentLoaded"", domContentLoaded, false);                
            }
            else {
                state.loadScript = href.getAttribute('data-ms-fragmentLoad') || state.loadScript;
                state.templateElement = href;
                if (!callback()) {
                    intervalId = setInterval(callback, 20);
                }
            }
        },

        _controlStaticStateLoaded: function (href, temp, state, complete) {
            /// <summary>
            /// PRIVATE METHOD: Once the control's static state has been loaded in the temporary iframe,
            /// this method spelunks the iframe's document to retrieve all relevant information. Also,
            /// this performs any needed fixups on the DOM (like adjusting relative URLs).
            /// </summary>

            var cd = temp.contentDocument;

            var links = cd.querySelectorAll('head > link[type=""text/css""]');
            state.styles = links;
            forEach(links, function (e) {
                Win.Controls.FragmentLoader._addLink(e);
            });

            // NOTE: no need to cache the style objects, as they are unique per fragment
            //
            forEach(cd.querySelectorAll('head > style'), function (e) {
                Win.Controls.FragmentLoader._addStyle(e);
            });

            var scripts = cd.getElementsByTagName('script');
            state.scripts = scripts;

            var scriptPosition = 0;
            forEach(scripts, function (e) {
                Win.Controls.FragmentLoader._addScript(e, href, scriptPosition);
                scriptPosition++;

                state.loadScript = e.getAttribute('data-ms-fragmentLoad') || state.loadScript;
            });

            state.loadScript = cd.body.getAttribute('data-ms-fragmentLoad') || state.loadScript;

            // UNDONE: figure out all the elements we should do URI fixups for
            //
            forEach(cd.body.getElementsByTagName('img'), function (e) {
                e.src = e.href;
            });
            forEach(cd.body.getElementsByTagName('a'), function (e) {
                // UNDONE: for # only anchor tags, we don't update the href... good design?
                //
                if (e.href !== """") {
                    var href = e.getAttribute(""href"");
                    if (href && href[0] != ""#"") {
                        e.href = e.href;
                    }
                }
            });

            // strip inline scripts from the body, they got copied to the 
            // host document with the rest of the scripts above... 
            //
            var scripts = cd.body.getElementsByTagName(""script"");
            while (scripts.length > 0) {
                scripts[0].parentNode.removeChild(scripts[0]);
            }

            // UNDONE: capture a documentfragment with the list of body.children
            //
            state.templateElement = document.importNode(temp.contentDocument.body, true);

            // huge ugly kludge
            if (state.loadScript) {
                var intervalId = setInterval(function () {
                    var load = globalObj[state.loadScript];
                    if (load) {
                        complete(load);
                        clearInterval(intervalId);
                    }
                }, 20);
            }
            else {
                complete();
            }
        },

        _initialize: function () {
            /// <summary>
            /// PRIVATE METHOD: Initializes the fragment loader with the list of scripts and 
            /// styles already present in the host document
            /// </summary>
            if (Win.Controls.FragmentLoader._initialized) { return; }

            Win.Controls.FragmentLoader._initialized = true;

            var scripts = head.querySelectorAll(""script"");
            for (var i = 0, l = scripts.length; i < l; i++) {
                Win.Controls.FragmentLoader._scripts[scripts[i].src] = true;
            }

            var csss = head.querySelectorAll('link[type=""text/css""]');
            for (var i = 0, l = csss.length; i < l; i++) {
                Win.Controls.FragmentLoader._links[csss[i].href] = true;
            }
        },

        addFragment: function (element, href, options, complete) {
            /// <summary>
            /// Adds the content of the fragment specified by ""href"" to the children of ""element"".
            /// The ""options"" record is pased (optionaly) to the load handler for the fragment.
            /// If supplied ""complete"" will be called when the  fragment has been loaded and the 
            /// load handler is complete.
            /// </summary>
            Win.Controls.FragmentLoader._initialize();

            Win.Controls.FragmentLoader._controlStaticState(href, function (load, state) {
                istate = Object.create(state, { element: { value: element} });

                var adopted = istate.templateElement.cloneNode(true);
                var c = adopted.children;
                var generatedElements = [];
                while (c.length > 0) {
                    generatedElements.push(c[0]);
                    element.appendChild(c[0]);
                }
                if (load) {
                    load(generatedElements, options);
                }
                if (complete) {
                    complete();
                }
            });
        },

        createFragment: function (href, options, complete) {
            /// <summary>
            /// Returns the content of the fragment specified by ""href"" to the children of ""element"".
            /// The ""options"" record is pased (optionaly) to the load handler for the fragment.
            /// If supplied ""complete"" will be called when the  fragment has been loaded and the 
            /// load handler is complete.
            ///
            /// The will be placed in a wrapper ""div"" element.
            /// </summary>
            var container = document.createElement(""div"");
            Win.Controls.FragmentLoader.addFragment(container, href, options, complete);
            return container;
        },

        prepareFragment: function (href, complete) {
            /// <summary>
            /// Starts loading the fragment at the specified location, success will be 
            /// called when the fragment is ready to be used
            /// </summary>
            Win.Controls.FragmentLoader._initialize();

            var callback = function () {
                if (complete) {
                    complete();
                }
            };
            Win.Controls.FragmentLoader._controlStaticState(href, callback);
        },

        unprepareFragment: function (href) {
            /// <summary>
            /// Removes any cached information about the fragment, this will not unload scripts 
            /// or styles referenced by the fragment.
            /// </summary>

            delete this._states[this._idFromHref(href)];
        },

        selfhost: function (load) {
            /// <summary>
            /// This is used in the fragment definition markup to allow a fragment to 
            /// be loaded as a stand alone page.
            /// </summary>
            if (globalObj.parent) {
                if (globalObj.parent.document[loaderStateProp] != ""loading"") {
                    forEach(globalObj.document.querySelectorAll('head > script[type=""ms-deferred/javascript""]'),
                        function (e) {
                            Win.Controls.FragmentLoader._addScript(e);
                        });

                    globalObj.addEventListener(""DOMContentLoaded"", function (event) {
                        load(globalObj.document.body.children);
                    }, false);
                }
            }
        }
    });
})(Win, this);
";
        #endregion
        #region win8ui.js
        const string win8ui_js = @"
/**********************************************************
*                                                         *
*   © Microsoft. All rights reserved.                     *
*                                                         *
*   This library is intended for use in WWAs only.        *  
*                                                         *
**********************************************************/
// Build: 7917.0.x86chk.fbl_pac_dev(chrisan).110105-0958 
Win.Namespace.define(""Win.UI.Controls"", {});
Win.Namespace.define(""Win.UI.Utilities"", {});
Win.Namespace.define(""Win.UI.Animations"", {});

var Win8 = Win;

Win8.UI.setTimeout = function (callback, delay) {
    return window.setTimeout(callback, delay);
};

// In order to keep JS syntax checking in VS working every preprocessor directive (#ifdef, #endif, etc) 
// should be preceded by // (comment). So instead of #ifdef we should use //#ifdef in JavaScript code.



var assertionFailed = function (condition, file, line) {
    if (!confirm(""Assertion failed: "" + condition + ""\n\n"" +
            ""file: "" + file.slice(file.lastIndexOf(""\\"") + 1, file.lastIndexOf("".pp"")) + "".js"" + ""\n"" +
            ""line: "" + (line + 1) + ""\n\n"" +
            ""(Press Cancel to debug the application)"")) {
        /*jslint debug: true */
        debugger;
        /*jslint debug: false */
    }
};

var logOutput = function (text) {
    if (window.console && console.log) {
        console.log(text); 
    } 
};























(function (thisWinUI) {

// Private utilities function

var objectIsNotValidJson = ""Error: dataObjects must be representable as valid JSON."";

Win.Namespace.defineWithParent(thisWinUI, ""Utilities"", {
    _dataKey: ""msWin8uiData"",
    _pixelsRE: /^-?\d+(px)?$/i,
    _numberRE: /^-?\d+/i,

    Key: {
        backspace:        8,
        tab:              9,
        enter:           13,
        shift:           16,
        ctrl:            17,
        alt:             18,
        pause:           19,
        capsLock:        20,
        escape:          27,
        space:           32,
        pageUp:          33,
        pageDown:        34,
        end:             35,
        home:            36,
        leftArrow:       37,
        upArrow:         38,
        rightArrow:      39,
        downArrow:       40,
        insert:          45,
        deleteKey:       46,
        num0:            48,
        num1:            49,
        num2:            50,
        num3:            51,
        num4:            52,
        num5:            53,
        num6:            54,
        num7:            55,
        num8:            56,
        num9:            57,
        a:               65,
        b:               66,
        c:               67,
        d:               68,
        e:               69,
        f:               70,
        g:               71,
        h:               72,
        i:               73,
        j:               74,
        k:               75,
        l:               76,
        m:               77,
        n:               78,
        o:               79,
        p:               80,
        q:               81,
        r:               82,
        s:               83,
        t:               84,
        u:               85,
        v:               86,
        w:               87,
        x:               88,
        y:               89,
        z:               90,
        leftWindows:     91,
        rightWindows:    92,
        numPad0:         96,
        numPad1:         97,
        numPad2:         98,
        numPad3:         99,
        numPad4:        100,
        numPad5:        101,
        numPad6:        102,
        numPad7:        103,
        numPad8:        104,
        numPad9:        105,
        multiply:       106,
        add:            107,
        subtract:       109,
        decimalPoint:   110,
        divide:         111,
        F1:             112,
        F2:             113,
        F3:             114,
        F4:             115,
        F5:             116,
        F6:             117,
        F7:             118,
        F8:             119,
        F9:             120,
        F10:            121,
        F11:            122,
        F12:            123,
        numLock:        144,
        scrollLock:     145,
        semicolon:      186,
        equal:          187,
        comma:          188,
        dash:           189,
        period:         190,
        forwardSlash:   191,
        graveAccent:    192,
        openBracket:    219,
        backSlash:      220,
        closeBracket:   221,
        singleQuote:    222
    },

    getData: function Utilities_getData(element, key) {
        var data = element[this._dataKey] || {};
        return data[key];
    },

    setData: function Utilities_setData(element, key, value) {
        var data = element[this._dataKey] || {};
        data[key] = value;
        element[this._dataKey] = data;
    },

    extend: function Utilities_extend(target, source) {
        target = target || {};
        for (var fieldname in source) {
            if (!target.hasOwnProperty(fieldname)) {
                target[fieldname] = source[fieldname];
            }
        }
        return target;
    },

    extendOverwrite: function Utilities_extendOverwrite(target, source) {
        target = target || {};
        for (var fieldname in source) {
            if (true) { // For analysis tools that expect a test for hasOwnProperty
                target[fieldname] = source[fieldname];
            }
        }
        return target;
    },

    contentWidth: function Utilities_contentWidth(element) {
        var border = this.getDimension(element, ""borderLeftWidth"") + this.getDimension(element, ""borderRightWidth""),
            padding = this.getDimension(element, ""paddingLeft"") + this.getDimension(element, ""paddingRight"");
        return element.offsetWidth - border - padding;
    },

    totalWidth: function Utilities_totalWidth(element) {
        var margin = this.getDimension(element, ""marginLeft"") + this.getDimension(element, ""marginRight"");
        return element.offsetWidth + margin;
    },

    contentHeight: function Utilities_contentHeight(element) {
        var border = this.getDimension(element, ""borderTopWidth"") + this.getDimension(element, ""borderBottomWidth""),
            padding = this.getDimension(element, ""paddingTop"") + this.getDimension(element, ""paddingBottom"");
        return element.offsetHeight - border - padding;
    },

    totalHeight: function Utilities_totalHeight(element) {
        var margin = this.getDimension(element, ""marginTop"") + this.getDimension(element, ""marginBottom"");
        return element.offsetHeight + margin;
    },

    position: function Utilities_position(fromElement) {
        var element = fromElement,
            offsetParent = element.offsetParent,
            top = element.offsetTop,
            left = element.offsetLeft;

        while ((element = element.parentNode) !== null &&
                element !== document.body &&
                element !== document.documentElement) {
            top -= element.scrollTop;
            left -= element.scrollLeft;

            if (element === offsetParent) {
                top += element.offsetTop;
                left += element.offsetLeft;

                offsetParent = element.offsetParent;
            }
        }

        return {
            left: left,
            top: top,
            width: fromElement.offsetWidth,
            height: fromElement.offsetHeight
        };
    },

    offsetRight: function Utilities_offsetRight(element) {
        return element.offsetLeft + element.offsetWidth;
    },

    offsetBottom: function Utilities_offsetBottom(element) {
        return element.offsetTop + element.offsetHeight;
    },

    scrollRight: function Utilities_scrollRight(element) {
        return element.scrollLeft + element.offsetWidth;
    },

    scrollBottom: function Utilities_scrollBottom(element) {
        return element.scrollTop + element.offsetHeight;
    },

    getDimension: function Utilities_getDimension(element, property) {
        return this.convertToPixels(element, this.getStyle(element, property));
    },

    convertToPixels: function Utilities_convertToPixels(element, value) {
        if (!this._pixelsRE.test(value) && this._numberRE.test(value)) {
            var previousValue = element.style.left;

            element.style.left = value;
            value = element.style.pixelLeft;

            element.style.left = previousValue;

            return value;
        } else {
            return parseInt(value, 10) || 0;
        }
    },

    getStyle: function Utilities_getStyle(element, property) {
        return window.getComputedStyle(element, null)[property];
    },

    empty: function Utilities_empty(element) {
        if (element)
            while (element.hasChildNodes()) {
                element.removeChild(element.firstChild);
            }
    },

    children: function Utilities_children(element) {
        var childElements = [];
        var curr = element.firstChild;
        while (curr) {
            if (this.isDOMElement(curr)) {
                childElements.push(curr);
            }
            curr = curr.nextSibling;
        }
        return childElements;
    },

    addClass: function Utilities_addClass(element, cssClass) {
        var currentClassName = element.className;
        if (!currentClassName) {
            element.className = cssClass;
        } else {
            if (("" "" + currentClassName + "" "").indexOf("" "" + cssClass + "" "") === -1) {
                element.className = (currentClassName.length ? currentClassName + "" "" : """") + cssClass;
            }
        }
    },

    removeClass: function Utilities_removeClass(element, cssClass) {
        if (element.className) {
            var re = new RegExp(""(^| +)"" + cssClass + ""( +|$)"", ""i"");
            element.className = element.className.replace(re, "" "").trim();
        }
    },

    hasClass: function Utilities_hasClass(element, cssClass) {
        return ("" "" + element.className + "" "").indexOf("" "" + cssClass + "" "") >= 0;
    },

    isDOMElement: function Utilities_isDOMElement(element) {
        return element &&
            typeof element === ""object"" &&
            typeof element.tagName === ""string"";
    },

    isEmptyObject: function Utilities_isEmptyObject(object) {
        for (var property in object) {
            if (true) { // For analysis tools that expect a test for hasOwnProperty 
                return false;
            }
        }
        return true;
    },

    disableTab: function Utilities_disableTab(element) {
        if (this.isDOMElement(element) && element.preservedTabIndex === undefined) {
            element.preservedTabIndex = element.getAttribute(""tabindex"");
            element.setAttribute(""tabindex"", -1);
            var curr = element.firstChild;
            while (curr) {
                this.disableTab(curr);
                curr = curr.nextSibling;
            }
        }
    },

    enableTab: function Utilities_enableTab(element) {
        if (element.preservedTabIndex !== undefined && this.isDOMElement(element)) {
            element.setAttribute(""tabindex"", element.preservedTabIndex);
            delete element.preservedTabIndex;
            var curr = element.firstChild;
            while (curr) {
                this.enableTab(curr);
                curr = curr.nextSibling;
            }
        }
    },

    isNonNegativeNumber: function Utilities_isNonNegativeNumber(n) {
        return (typeof n === ""number"") && n >= 0;
    },

    isNonNegativeInteger: function Utilities_isNonNegativeInteger(n) {
        return this.isNonNegativeNumber(n) && n === Math.floor(n);
    },

    validateDataObject: function Utilities_validateDataObject(dataObject) {
        if (dataObject === undefined) {
            return dataObject;
        } else {
            // Convert the data object to JSON and back to enforce the constraints we want.  For example, we don't want
            // functions, arrays with extra properties, DOM objects, cyclic or acyclic graphs, or undefined values.
            var dataObjectValidated = JSON.parse(JSON.stringify(dataObject));

            if (dataObjectValidated === undefined) {
                throw new Error(objectIsNotValidJson);
            }

            return dataObjectValidated;
        }
    }
});


})(Win8.UI);

(function (thisWinUI) {

// Items Manager

// Utilities are private and global pointer will be deleted so we need to cache it locally
var utilities = thisWinUI.Utilities;

var indexIsInvalid = ""Invalid argument: index must be a non-negative integer."";
var countIsInvalid = ""Invalid argument: count must be a non-negative integer."";
var keyIsInvalid = ""Invalid argument: key must be a string."";
var prefixIsInvalid = ""Invalid argument: prefix must be a string."";
var listNotEmpty = ""Error: itemFromPrefix must be called without any instantiated items."";
var callbackIsInvalid1 = ""Invalid argument: "";
var callbackIsInvalid2 = "" must be a function."";
var callbackIsInvalid2Optional = "", if present, must be a function."";
var priorityIsInvalid = ""Invalid argument: priority must be one of following values: Priority.high or Priority.medium."";
var undefinedItemReturned = ""Error: data source returned undefined item."";
var invalidKeyReturned = ""Error: data source returned item with undefined or null key."";
var invalidIndexReturned = ""Error: data source should return undefined, null or a non-negative integer for the index."";
var invalidCountReturned = ""Error: data source should return undefined, null, CountResult.unknown, CountResult.failure, or a non-negative integer for the count."";
var invalidRequestedCountReturned = ""Error: data source should return CountResult.unknown, CountResult.failure, or a non-negative integer for the count."";
var invalidRendererOutput = ""Error: a renderer should return a DOM element or an HTML string (with a single root element)."";
var dataSourceIsInvalid = ""Invalid argument: dataSource must be an object or a string."";
var itemRendererIsInvalid = ""Invalid argument: itemRenderer must be a function or a string."";
var itemIsInvalid = ""Invalid argument: item must be a DOM element that was returned by the Items Manager, and has not been replaced or released."";

thisWinUI.createItemsManager = function (dataSource, itemRenderer, elementNotificationHandler, options) {
    /// <summary>
    ///     Creates an Items Manager object bound to the given data source.
    /// </summary>
    /// <param name=""dataSource"" type=""DataSource"">
    ///     The data source object that serves as the intermediary between the Items Manager and the actual data
    ///     source.  Object must implement the DataSource interface.
    /// </param>
    /// <param name=""itemRenderer"" mayBeNull=""true"" type=""Function"">
    ///     Callback for rendering fetched items.  Function's signature should match that of itemRendererCallback.
    /// </param>
    /// <param name=""elementNotificationHandler"" type=""ElementNotificationHandler"">
    ///     A notification handler object that the Items Manager will call when the instantiated items
    ///     change in the data source.  Object must implement the ElementNotificationHandler interface.
    /// </param>
    /// <param name=""options"" mayBeNull=""true"" optional=""true"" type=""Object"">
    ///     Options for the Items Manager.  Properties on this object may include:
    ///     
    ///     placeholderRenderer (type=""Object""): 
    ///         Callback for rendering placeholder elements while items are fetched.  Function's signature should match
    ///         that of placeholderRendererCallback.
    ///
    ///     itemNotificationHandler (type=""ItemNotificationHandler""):
    ///         A notification handler object that the Items Manager will call to signal various state changes.  Object
    ///         must implement the ItemNotificationHandler interface.
    ///
    ///     ownerElement (type=""Object"", domElement=""true""):
    ///         The DOM element for the owner control; the Items Manager will fire events on this node, and will make
    ///         use of its ID, if it has one.
    ///     
    /// </param>
    /// <returns type=""ItemsManager"" />

    return new ItemsManager(dataSource, itemRenderer, elementNotificationHandler, options);
};

// Some characters must be escaped in HTML and JavaScript, but the only other requirement for element IDs is that they
// be unique strings.  Use ` as the escape character, simply because it's rarely used.
var escapeMap = {
    ""`"": ""``"",
    ""'"": ""`s"",
    '""': '`d',
    '<': '`l',
    '>': '`g',
    '&': '`a',
    '\\': '`b',
    '\/': '`f'
};

thisWinUI.itemID = function (ownerElement, key) {
    var instanceID = """";
    if (ownerElement) {
        var elementID = ownerElement.id;
        if (elementID !== undefined) {
            instance = elementID;
        }
    }

    return ""im"" + instanceID + ""_"" + key.replace(/[`'""<>&\\\/]/g, function (character) {
        return escapeMap[character];
    });
};

thisWinUI.startMarker = {};
thisWinUI.endMarker = {};

thisWinUI.Priority = {
    high: 0,
    medium: 1,
    low: 2,
    max: 3
};

thisWinUI.ItemsManagerStatus = {
    ready: 0,
    waiting: 1,
    failure: 2,
    max: 3
};

thisWinUI.CountResult = {
    unknown: -1,
    failure: -2
};

thisWinUI.FetchResult = {
    // No ""success"" code since valid array of results should be returned in that case
    doesNotExist: 0,
    noResponse: 1,
    max: 2
};

thisWinUI.EditResult = {
    success: 0,
    noResponse: 1,
    notPermitted: 2,
    noLongerMeaningful: 3,
    itemNotReady: 4,
    max: 5
};

var simultaneousResourceFetches = 6;
var outstandingResourceFetches = 0;

// Sentinel for circular linked lists of objects provided by Items Manager instances that need resources to be loaded
var resourceRequestQueues = {};
resourceRequestQueues[thisWinUI.Priority.high] = resourceRequestQueues;
resourceRequestQueues[thisWinUI.Priority.medium] = resourceRequestQueues;
resourceRequestQueues[thisWinUI.Priority.low] = resourceRequestQueues;

// Adds the resource request to the head of the given global priority queue, if it isn't in it already
function pushResourceRequest(resourceRequest, priority) {
    // Use the priority as the ""next"" property
    if (!resourceRequest[priority]) {
        do { if (resourceRequestQueues[priority]) { } else { assertionFailed(""resourceRequestQueues[priority]"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 542); } } while (false);
        resourceRequest[priority] = resourceRequestQueues[priority];
        resourceRequestQueues[priority] = resourceRequest;
    }
}

var fetchingNextResources = false;

// Find the highest-priority outstanding request for resources, and start a fetch; continue until the desired number of
// simultaneous fetches are in progress.
function fetchNextResources() {
    // Re-entrant calls are redundant, as the loops below will continue fetching when the callee returns
    if (!fetchingNextResources) {
        fetchingNextResources = true;

        var priorityMax = thisWinUI.Priority.max;
        for (var priority = 0; priority < priorityMax; priority++) {
            var resourceRequest;
            while ((resourceRequest = resourceRequestQueues[priority]) !== resourceRequestQueues) {
                if (resourceRequest.fetchResources(priority)) {
                    fetchingNextResources = false;
                    return;
                } else {
                    do { if (resourceRequest[priority]) { } else { assertionFailed(""resourceRequest[priority]"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 565); } } while (false);
                    resourceRequestQueues[priority] = resourceRequest[priority];
                    delete resourceRequest[priority];
                }
            }
        }

        fetchingNextResources = false;
    }
}

function ItemsManager(dataSource, itemRenderer, elementNotificationHandler, options) {
    /// <summary>
    ///     Constructor for Items Manager object, including public methods for enumerating through the list of items
    ///     generated by the bound data source.
    /// </summary>

    // Private members

    var placeholderRenderer,
        itemNotificationHandler,
        ownerElement,
        compareByIdentity,
        listEditor,
        dataNotificationHandler,
        status,
        notificationsSent,
        finishNotificationsPosted,
        editsInProgress,
        editsQueued,
        applyNextEdit,
        waitForRefresh,
        dataNotificationsInProgress,
        countDelta,
        indexUpdateDeferred,
        nextTempKey,
        currentRefreshID,
        nextFetchID,
        fetchesInProgress,
        knownCount,
        slotsStart,
        slotsEnd,
        keyMap,
        indexMap,
        elementMap,
        releasedSlots,
        releasedSlotsMax,
        lastSlotReleased,
        releasedSlotReductionInProgress,
        queues,
        refreshRequested,
        refreshInProgress,
        refreshFetchesInProgress,
        refreshItemsFetched,
        refreshCount,
        refreshStart,
        refreshEnd,
        keyFetchIDs,
        refreshKeyMap,
        refreshIndexMap,
        deletedKeys,
        synchronousProgress,
        reentrantContinue,
        synchronousRefresh,
        reentrantRefresh,
        dummyParent;

    // Type-checks a callback parameter, since a failure will be hard to diagnose when it occurs
    function checkCallback(callback, name, optional) {
        if ((!optional || (callback !== undefined && callback !== null)) && typeof callback !== ""function"") {
            throw new Error(callbackIsInvalid1 + name + (optional ? callbackIsInvalid2Optional : callbackIsInvalid2));
        }
    }



    function checkListIntegrity(listStart, listEnd) {
        for (var slotCheck = listStart; slotCheck !== listEnd; slotCheck = slotCheck.next) {
            do { if (slotCheck.next) { } else { assertionFailed(""slotCheck.next"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 643); } } while (false);
            do { if (slotCheck.next.prev === slotCheck) { } else { assertionFailed(""slotCheck.next.prev === slotCheck"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 644); } } while (false);
            if (slotCheck.lastInSequence) {
                do { if (slotCheck.next.firstInSequence) { } else { assertionFailed(""slotCheck.next.firstInSequence"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 646); } } while (false);
            }

            if (slotCheck !== listStart) {
                do { if (slotCheck.prev) { } else { assertionFailed(""slotCheck.prev"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 650); } } while (false);
                do { if (slotCheck.prev.next === slotCheck) { } else { assertionFailed(""slotCheck.prev.next === slotCheck"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 651); } } while (false);
                if (slotCheck.firstInSequence) {
                    do { if (slotCheck.prev.lastInSequence) { } else { assertionFailed(""slotCheck.prev.lastInSequence"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 653); } } while (false);
                }
            }
        }
    }











    function postCall(callback) {
        thisWinUI.setTimeout(callback, 0);
    }

    function setStatus(statusNew) {
        if (status !== statusNew) {
            status = statusNew;
            if (itemNotificationHandler.updateStatus) {
                itemNotificationHandler.updateStatus(status);
            }
        }
    }

    function handlerToNotify() {
        if (!notificationsSent) {
            notificationsSent = true;

            if (elementNotificationHandler.beginNotifications) {
                elementNotificationHandler.beginNotifications();
            }
        }
        return elementNotificationHandler;
    }

    function finishNotifications() {
        if (notificationsSent && !editsInProgress && !dataNotificationsInProgress) {
            notificationsSent = false;

            if (elementNotificationHandler.endNotifications) {
                elementNotificationHandler.endNotifications();
            }
        }
    }

    function changeCount(count) {
        var oldCount = knownCount;
        knownCount = count;
        if (elementNotificationHandler.countChanged) {
            handlerToNotify().countChanged(knownCount, oldCount);
        }
    }

    function defaultRenderer(getIndex, key, dataObject, itemID) {
        return document.createElement(""div"");
    }

    // Renderers can return either an HTML string (with a single root element) or a DOM element.  This function ensures
    // that the given output is in one of these forms, and if it is HTML, parses it.
    function ensureDomElement(rendererOutput) {
        switch (typeof rendererOutput) {
            case ""object"":
                do { if (rendererOutput) { } else { assertionFailed(""rendererOutput"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 720); } } while (false);
                return rendererOutput;

            case ""string"":
                dummyParent.innerHTML = rendererOutput;
                if (dummyParent.childNodes.length !== 1) {
                    throw new Error(invalidRendererOutput);
                }
                var element = dummyParent.removeChild(dummyParent.firstChild);
                do { if (element) { } else { assertionFailed(""element"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 729); } } while (false);
                dummyParent.innerHTML = null;
                return element;
        }

        throw new Error(invalidRendererOutput);
    }

    // Renderers can return either an HTML string (with a single root element) or a DOM element.  This function ensures
    // that the given output is in one of these forms, and if it is a DOM element, converts it to HTML.
    function ensureHtmlString(rendererOutput) {
        switch (typeof rendererOutput) {
            case ""object"":
                dummyParent.appendChild(rendererOutput);
                var html = dummyParent.innerHTML;
                dummyParent.removeChild(rendererOutput);
                return html;

            case ""string"":
                return rendererOutput;
        }

        throw new Error(invalidRendererOutput);
    }

    function renderPlaceholderElement(slot, index) {
        return placeholderRenderer(
            function () {
                slot.indexObserved = true;
                return index;
            }
        );
    }

    function renderItemElement(slot, dataObject, index) {
        do { if (slot.key !== undefined) { } else { assertionFailed(""slot.key !== undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 764); } } while (false);
        return itemRenderer(
            function () {
                slot.indexObserved = true;
                return index;
            },
            slot.key,
            dataObject,
            thisWinUI.itemID(ownerElement, slot.key)
        );
    }

    function slotFromItem(item, tolerateUnknownElements) {
        var msDataItem = item.msDataItem;
        var slot = msDataItem ? keyMap[msDataItem.key] : elementMap[item.uniqueID];
        if ((!slot || !slot.element) && !tolerateUnknownElements) {
            throw new Error(itemIsInvalid);
        }

        return slot;
    }

    // Returns the slot after the last insertion point between sequences
    function lastInsertionPoint(listStart, listEnd) {
        var slotNext = listEnd;
        while (!slotNext.firstInSequence) {
            slotNext = slotNext.prev;

            if (slotNext === listStart) {
                return undefined;
            }
        }

        return slotNext;
    }

    function successorFromIndex(index, indexMapForSlot, listStart, listEnd) {
        do { if (index !== undefined) { } else { assertionFailed(""index !== undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 801); } } while (false);
        do { if (!indexMapForSlot[index]) { } else { assertionFailed(""!indexMapForSlot[index]"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 802); } } while (false);

        // Try the previous index
        var slotNext = indexMapForSlot[index - 1];
        if (slotNext !== undefined) {
            // We want the successor
            slotNext = slotNext.next;
        } else {
            // Try the next index
            slotNext = indexMapForSlot[index + 1];
            if (slotNext === undefined) {
                // Resort to a linear search
                slotNext = listStart.next;
                var lastSequenceStart;
                while (slotNext.index === undefined || slotNext.index < index) {
                    do { if (slotNext) { } else { assertionFailed(""slotNext"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 817); } } while (false);
                    if (slotNext.firstInSequence) {
                        lastSequenceStart = slotNext;
                    }

                    if (slotNext === listEnd) {
                        break;
                    }

                    slotNext = slotNext.next;
                }

                if (slotNext === listEnd) {
                    // Return the last insertion point between sequences, or undefined if none
                    slotNext = (lastSequenceStart && lastSequenceStart.index === undefined ? lastSequenceStart : undefined);
                }
            }
        }

        return slotNext;
    }

    function setSlotKey(slot, key) {
        do { if (slot.key === undefined) { } else { assertionFailed(""slot.key === undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 840); } } while (false);
        slot.key = key;

        // Add the slot to the keyMap, so it is possible to quickly find the slot given its key.

        do { if (!keyMap[slot.key]) { } else { assertionFailed(""!keyMap[slot.key]"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 845); } } while (false);
        keyMap[slot.key] = slot;
    }

    function setSlotIndex(slot, index, indexMapForSlot) {
        do { if (slot.index === undefined) { } else { assertionFailed(""slot.index === undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 850); } } while (false);
        do { if (isNaN(index) || (typeof index === ""number"" && index >= 0)) { } else { assertionFailed(""isNaN(index) || (typeof index === \""number\"" && index >= 0)"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 851); } } while (false);

        // Tolerate NaN, so clients can pass (undefined - 1) or (undefined + 1)
        if (!isNaN(index)) {
            slot.index = index;

            // Add the slot to the indexMap, so it is possible to quickly find the slot given its index.
            indexMapForSlot[index] = slot;
        }
    }

    function changeSlotIndex(slot, index, indexMapForSlot) {
        do { if (index === undefined || (typeof index === ""number"" && index >= 0)) { } else { assertionFailed(""index === undefined || (typeof index === \""number\"" && index >= 0)"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 863); } } while (false);

        if (slot.index !== undefined && indexMapForSlot[slot.index] === slot) {
            // Remove the slot's old index from the indexMap
            delete indexMapForSlot[slot.index];
        }

        if (index === undefined) {
            do { if (!slot.indexRequested) { } else { assertionFailed(""!slot.indexRequested"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 871); } } while (false);
            delete slot.index;
        } else {
            slot.index = index;

            // Add the slot to the indexMap, so it is possible to quickly find the slot given its index.
            indexMapForSlot[index] = slot;
        }
    }

    function insertSlot(slot, slotNext) {
        do { if (slotNext) { } else { assertionFailed(""slotNext"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 882); } } while (false);
        do { if (slotNext.prev) { } else { assertionFailed(""slotNext.prev"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 883); } } while (false);

        slot.prev = slotNext.prev;
        slot.next = slotNext;

        slot.prev.next = slot;
        slotNext.prev = slot;
    }

    // Creates a new slot and adds it to the item list
    function createSlot(slotNext, index, indexMapForSlot) {
        var slotNew = {};

        setSlotIndex(slotNew, index, indexMapForSlot);
        insertSlot(slotNew, slotNext);

        return slotNew;
    }

    function createSlotSequence(slotNext, index, indexMapForSlot) {
        var slotNew = createSlot(slotNext, index, indexMapForSlot);

        slotNew.firstInSequence = true;
        slotNew.lastInSequence = true;

        return slotNew;
    }

    function addSlotBefore(slotNext, indexMapForSlot) {
        do { if (slotNext.firstInSequence) { } else { assertionFailed(""slotNext.firstInSequence"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 912); } } while (false);
        do { if (slotNext.prev.lastInSequence) { } else { assertionFailed(""slotNext.prev.lastInSequence"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 913); } } while (false);
        var slotNew = createSlot(slotNext, slotNext.index - 1, indexMapForSlot);
        delete slotNext.firstInSequence;

        // See if we've bumped into the previous sequence
        if (slotNew.prev.index === slotNew.index - 1) {
            delete slotNew.prev.lastInSequence;
        } else {
            slotNew.firstInSequence = true;
        }

        return slotNew;
    }

    function addSlotAfter(slotPrev, indexMapForSlot) {
        do { if (slotPrev.lastInSequence) { } else { assertionFailed(""slotPrev.lastInSequence"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 928); } } while (false);
        do { if (slotPrev.next.firstInSequence) { } else { assertionFailed(""slotPrev.next.firstInSequence"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 929); } } while (false);
        var slotNew = createSlot(slotPrev.next, slotPrev.index + 1, indexMapForSlot);
        delete slotPrev.lastInSequence;

        // See if we've bumped into the next sequence
        if (slotNew.next.index === slotNew.index + 1) {
            delete slotNew.next.firstInSequence;
        } else {
            slotNew.lastInSequence = true;
        }

        return slotNew;
    }

    // Inserts a slot in the middle of a sequence or between sequences.  If the latter, mergeWithPrev and
    // mergeWithNext parameters specify whether to merge the slow with the previous sequence, or next, or neither.
    function insertAndMergeSlot(slot, slotNext, mergeWithPrev, mergeWithNext) {
        insertSlot(slot, slotNext);

        var slotPrev = slot.prev;

        if (slotPrev.lastInSequence) {
            do { if (slotNext.firstInSequence) { } else { assertionFailed(""slotNext.firstInSequence"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 951); } } while (false);

            if (mergeWithPrev) {
                do { if (slotNext.firstInSequence) { } else { assertionFailed(""slotNext.firstInSequence"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 954); } } while (false);
                delete slotPrev.lastInSequence;
                slot.lastInSequence = true;
            } else {
                slot.firstInSequence = true;
            }

            if (mergeWithNext) {
                do { if (slotPrev.lastInSequence) { } else { assertionFailed(""slotPrev.lastInSequence"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 962); } } while (false);
                delete slotNext.firstInSequence;
                slot.firstInSequence = true;
            } else {
                slot.lastInSequence = true;
            }
        }
    }

    function reinsertSlot(slot, slotNext, mergeWithPrev, mergeWithNext) {
        insertAndMergeSlot(slot, slotNext, !firstInSequence, !lastInSequence);
        keyMap[slot.key] = slot;
        var index = slot.index;
        if (slot.index !== undefined) {
            indexMap[slot.index] = slot;
        }
    }

    function mergeSequences(slotPrev) {
        delete slotPrev.lastInSequence;
        delete slotPrev.next.firstInSequence;
    }

    function splitSequences(slotPrev) {
        slotPrev.lastInSequence = true;
        slotPrev.next.firstInSequence = true;
    }

    function removeSlot(slot) {
        if (slot.lastInSequence) {
            delete slot.lastInSequence;
            slot.prev.lastInSequence = true;
        }
        if (slot.firstInSequence) {
            delete slot.firstInSequence;
            slot.next.firstInSequence = true;
        }
        slot.prev.next = slot.next;
        slot.next.prev = slot.prev;
    }

    function removeSlotPermanently(slot) {
        removeSlotFromQueue(slot);
        removeSlot(slot);

        if (slot.key !== undefined) {
            delete keyMap[slot.key];
        }
        if (slot.index !== undefined) {
            delete indexMap[slot.index];
        }
    }

    function deleteUnrequestedSlot(slot) {
        splitSequences(slot);
        removeSlotPermanently(slot);
    }

    function sendInsertedNotification(slot) {
        var slotPrev = slot.prev,
            slotNext = slot.next;

        handlerToNotify().inserted(slot.element,
                slotPrev.lastInSequence || slotPrev === slotsStart ? undefined : slotPrev.element,
                slotNext.firstInSequence || slotNext === slotsEnd ? undefined : slotNext.element
                );
    }

    function sendDataObjectChangedNotification(slot, dataObjectOld) {
        if (elementNotificationHandler.dataObjectChanged) {
            handlerToNotify().dataObjectChanged(slot.element, slot.dataObject, dataObjectOld);
        }
    }

    function moveSlot(slot, slotMoveBefore, mergeWithPrev, mergeWithNext) {
        var slotMoveAfter = slotMoveBefore.prev;

        // If the slot is being moved before or after itself, adjust slotMoveAfter or slotMoveBefore accordingly
        if (slotMoveBefore === slot) {
            slotMoveBefore = slot.next;
        } else if (slotMoveAfter === slot) {
            slotMoveAfter = slot.prev;
        }

        // Send the notification before the move
        handlerToNotify().moved(slot.element,
                (slotMoveAfter.lastInSequence && !mergeWithPrev) || slotMoveAfter === slotsStart ? undefined : slotMoveAfter.element,
                (slotMoveBefore.firstInSequence && !mergeWithNext) || slotMoveBefore === slotsEnd ? undefined : slotMoveBefore.element
                );

        removeSlot(slot);
        insertAndMergeSlot(slot, slotMoveBefore, mergeWithPrev, mergeWithNext);
    }

    // Reverts a slot to its state before instantiation
    function uninstantiateSlot(slot) {
        removeSlotFromQueue(slot);
        delete slot.element;
        delete slot.elementTree;
        delete slot.instantiationState;
        delete slot.indexObserved;
    }

    function deleteSlot(slot, mirage) {
        var element = slot.element;
        if (element !== undefined) {
            handlerToNotify().removed(element, mirage);
        }
        uninstantiateSlot(slot);

        removeSlotPermanently(slot);
    }

    // Creates a new queue, implemented as a circular doubly-linked list
    function createQueue() {
        var queue = {};
        queue.queuePrev = queue.queueNext = queue;
        return queue;
    }

    // Prepends a slot to the given queue
    function pushSlot(slot, queue) {
        do { if (slot.queuePrev === undefined) { } else { assertionFailed(""slot.queuePrev === undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 1084); } } while (false);
        do { if (slot.queueNext === undefined) { } else { assertionFailed(""slot.queueNext === undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 1085); } } while (false);

        var queueHead = queue.queueNext;
        slot.queueNext = queueHead;
        queueHead.queuePrev = slot;

        slot.queuePrev = queue;
        queue.queueNext = slot;

    }

    // Appends a slot to the given queue
    function queueSlot(slot, queue) {
        do { if (slot.queuePrev === undefined) { } else { assertionFailed(""slot.queuePrev === undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 1098); } } while (false);
        do { if (slot.queueNext === undefined) { } else { assertionFailed(""slot.queueNext === undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 1099); } } while (false);

        var queueTail = queue.queuePrev;

        slot.queuePrev = queueTail;
        slot.queueNext = queue;

        queueTail.queueNext = slot;
        queue.queuePrev = slot;
    }

    // If the slot is in a queue, removes it and returns true.  Otherwise, returns false.
    function removeSlotFromQueue(slot) {
        var queuePrev = slot.queuePrev;
        if (queuePrev) {
            do { if (slot.instantiationState === InstantiationState.resourceFetchInProgress) { } else { assertionFailed(""slot.instantiationState === InstantiationState.resourceFetchInProgress"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 1114); } } while (false);

            var queueNext = slot.queueNext;
            do { if (queueNext) { } else { assertionFailed(""queueNext"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 1117); } } while (false);

            queuePrev.queueNext = queueNext;
            queueNext.queuePrev = queuePrev;

            delete slot.queuePrev;
            delete slot.queueNext;

            return true;
        } else {
            return false;
        }
    }

    function popSlot(queue) {
        var slot = queue.queueNext;
        if (slot === queue) {
            do { if (queue.queuePrev === queue) { } else { assertionFailed(""queue.queuePrev === queue"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 1134); } } while (false);
            return undefined;
        } else {
            removeSlotFromQueue(slot);
            return slot;
        }
    }

    // Some functions may be called synchronously or asynchronously, so it's best to post finishNotifications to avoid
    // calling it prematurely
    function postFinishNotifications() {
        if (!finishNotificationsPosted) {
            finishNotificationsPosted = true;
            postCall(function () {
                finishNotificationsPosted = false;
                finishNotifications();
            });
        }
    }

    var InstantiationState = {
        resourceFetchInProgress: 1,
        waitingForPrevious: 2
    };

    function instantiateItem(slot, changed) {
        // Do not instantiate a high-priority item if there is an uninstantiated high priority item before it
        if (!changed && slot.prev.instantiationState && slot.priority === thisWinUI.Priority.high && slot.prev.priority === thisWinUI.Priority.high) {
            slot.instantiationState = InstantiationState.waitingForPrevious;
        } else {
            do {
                delete slot.priority;
                delete slot.instantiationState;

                var elementOld = slot.element;

                do { if (slot.elementTree) { } else { assertionFailed(""slot.elementTree"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 1170); } } while (false);
                slot.element = slot.elementTree;
                delete slot.elementTree;

                if (changed) {
                    do { if (slot.kind === ""item"") { } else { assertionFailed(""slot.kind === \""item\"""", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 1175); } } while (false);
                    if (itemNotificationHandler.saveState) {
                        itemNotificationHandler.saveState(slot.key, elementOld);
                    }

                    handlerToNotify().changed(slot.element, elementOld);

                    if (slot.dataObjectDifferent) {
                        delete slot.dataObjectDifferent;

                        do { if (elementOld.msDataItem.dataObject) { } else { assertionFailed(""elementOld.msDataItem.dataObject"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 1185); } } while (false);
                        sendDataObjectChangedNotification(slot, elementOld.msDataItem.dataObject);
                    }
                } else {
                    do { if (slot.kind !== ""item"" && (elementOld === undefined || slot.kind === ""placeholder"")) { } else { assertionFailed(""slot.kind !== \""item\"" && (elementOld === undefined || slot.kind === \""placeholder\"")"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 1189); } } while (false);
                    slot.kind = ""item"";

                    // Finish modifying the slot before calling back into user code, in case there is a reentrant call
                    delete slot.indexRequested;

                    if (elementOld === undefined) {
                        // This is a reentrant call, so no placeholder element was generated, and there is therefore n
                        // need to notify the client that the item is available, as the fetch result can be returned
                        // from the original call.
                    } else {
                        // We can use the msDataItem expando to get the key, then keyMap, so we don't need two
                        // element->slot map entries.
                        delete elementMap[elementOld.uniqueID];

                        do { if (utilities.isDOMElement(slot.element)) { } else { assertionFailed(""utilities.isDOMElement(slot.element)"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 1204); } } while (false);
                        handlerToNotify().itemAvailable(slot.element, elementOld);
                    }
                }

                if (itemNotificationHandler.restoreState) {
                    itemNotificationHandler.restoreState(slot.key, slot.element);
                }

                // If the next item is a high priority and ready to instantiate, do so now
                slot = slot.next;
            } while (slot.priority === thisWinUI.Priority.high && slot.instantiationState === InstantiationState.waitingForPrevious);
        }

        postFinishNotifications();
    }

    function setIframeLoadHandler(element, onIframeLoad) {
        element.onload = function () {
            onIframeLoad(element);
        };
    }

    // Tracks the loading of resources for the following tags:
    // 
    //     <img src=""[URL]"">
    //     <iframe src=""[URL]"">
    //     <script src=""[URL]"">
    //     <input type=""image"" src=""[URL]"">
    //     <video poster=""[URL]"">
    //     <object data=""[URL]"">
    // 
    // Ensures itemAvailable will be called only when all resources in the given subtree have loaded (or failed to load).
    function loadItemResources(slot, changed) {
        var subtree = slot.elementTree;

        // Initialize the count to 1 to ensure the itemAvailable event doesn't fire prematurely
        var remainingResources = 1;

        // Similarly, increment the global fetch count to offset the extra call to onResourceLoad below
        outstandingResourceFetches++;

        function onResourceLoad() {
            do { if (outstandingResourceFetches > 0) { } else { assertionFailed(""outstandingResourceFetches > 0"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 1247); } } while (false);
            do { if (remainingResources > 0) { } else { assertionFailed(""remainingResources > 0"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 1248); } } while (false);

            if (--outstandingResourceFetches < simultaneousResourceFetches) {
                fetchNextResources();
            }

            if (--remainingResources === 0) {
                // Check that the resources we've loaded are for the current elementTree - it's possible that this item
                // has been released and re-rendered since loadItemResources was called.
                if (subtree === slot.elementTree) {
                    instantiateItem(slot, changed);
                }
            }
        }

        function onError() {
            setStatus(thisWinUI.ItemsManagerStatus.failure);

            onResourceLoad();
        }

        function onIframeLoad(element) {
            element.parentNode.removeChild(element);
            onResourceLoad();
        }

        // Walk the tree and locate elements that will load resources
        for (var element = subtree, elementPrev = null; elementPrev !== subtree || element !== elementPrev.nextSibling; element = element || elementPrev.nextSibling) {
            if (element) {
                var tagName = element.tagName,
                    resourceUrl,
                    resourceAttribute = ""src"";

                resourceUrl = undefined;

                switch (tagName) {
                    case ""IMG"":
                    case ""IFRAME"":
                    case ""SCRIPT"":
                        resourceUrl = element.src;
                        break;

                    case ""INPUT"":
                        if (element.type === ""image"") {
                            tagName = ""img"";
                            resourceUrl = element.src;
                        }
                        break;

                    case ""VIDEO"":
                        tagName = ""img"";
                        resourceUrl = element.poster;
                        break;

                    case ""OBJECT"":
                        resourceUrl = element.data;
                        resourceAttribute = ""data"";
                        break;
                }

                // If this element loads a resource, create a dummy element so we can set its onload handler
                if (resourceUrl) {
                    var dummyElement = document.createElement(tagName);

                    outstandingResourceFetches++;
                    remainingResources++;

                    // Set the onload handler before the resource attribute, to guarantee that it fires.  Note that the
                    // event might fire synchronously.
                    if (element.tagName === ""IFRAME"") {
                        // Must attach an IFRAME to get it to load
                        dummyElement.style.display = ""none"";
                        document.body.appendChild(dummyElement);
                        setIframeLoadHandler(dummyElement, onIframeLoad);
                    } else {
                        dummyElement.onload = onResourceLoad;
                        dummyElement.onerror = onError;
                    }
                    dummyElement[resourceAttribute] = resourceUrl;
                }

                // Continue walking the tree
                elementPrev = element;
                element = element.firstChild;
            } else {
                elementPrev = elementPrev.parentNode;
            }
        }

        // Call onResourceLoad directly to compensate for initializing the count to 1.  If all onload events fired
        // synchronously, this will trigger item instantiation and the itemAvailable event.
        onResourceLoad();
    }

    function itemElement(rendererOutput, slot) {
        var element = ensureDomElement(rendererOutput);

        // Attach an expando property to the element with the key and dataObject
        element.msDataItem = { key: slot.key, dataObject: slot.dataObject };

        return element;
    }

    function instantiateItemTree(slot, immediately, forceChangedNotification) {
        if (slot.kind === ""item"") {
            // It's now time to see if this item has actually changed - doing so earlier would have started a resource
            // fetch for all such items
            rerenderItem(slot, slot.indexOld, forceChangedNotification);
            delete slot.indexOld;

            if (slot.elementTree) {
                if (immediately) {
                    if (forceChangedNotification) {
                        // If we're forcing the changed notification, we also want to force the dataObjectChanged
                        // notification
                        slot.dataObjectDifferent = true;
                    }
                    instantiateItem(slot, true);
                } else {
                    loadItemResources(slot, true);
                }
            } else {
                // Nothing has changed in the element tree, so the resources have already been loaded
                delete slot.priority;
                delete slot.instantiationState;

                if (slot.dataObjectDifferent) {
                    delete slot.dataObjectDifferent;

                    // If dataObject changed, at least update the msDataItem
                    var msDataItem = slot.element.msDataItem;
                    var dataObjectOld = msDataItem.dataObject;
                    msDataItem.dataObject = slot.dataObject;
                    sendDataObjectChangedNotification(slot, dataObjectOld);
                    postFinishNotifications();
                }
            }
        } else {
            var elementPlaceholder = slot.element,
                elementTree = itemElement(renderItemElement(slot, slot.dataObject, slot.index), slot);

            do { if (!slot.elementTree) { } else { assertionFailed(""!slot.elementTree"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 1389); } } while (false);
            slot.elementTree = elementTree;

            loadItemResources(slot, false);
        }
    }

    // Object to be linked into a global list when this Items Manager needs to request resources
    var resourceRequest = {
        // Begins resource fetches for items with the given priority, until the desired number of simultaneous fetches
        // is reached.  Returns false if this Items Manager runs out of items at the given priority.
        fetchResources: function (priority) {
            do { if (outstandingResourceFetches < simultaneousResourceFetches) { } else { assertionFailed(""outstandingResourceFetches < simultaneousResourceFetches"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 1401); } } while (false);

            var queue = queues[priority];

            while (outstandingResourceFetches < simultaneousResourceFetches) {
                var slot = popSlot(queue);
                if (!slot) {
                    return false;
                }

                instantiateItemTree(slot);
            }

            return true;
        }
    };

    function createPlaceholder(slot) {
        slot.element = ensureDomElement(renderPlaceholderElement(slot, slot.index));
        slot.kind = ""placeholder"";

        elementMap[slot.element.uniqueID] = slot;

        if (slot.prev === slotsStart && !slot.firstInSequence && !indexUpdateDeferred) {
            slot.indexRequested = true;
            do { if (slot.index === undefined || slot.index === 0) { } else { assertionFailed(""slot.index === undefined || slot.index === 0"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 1426); } } while (false);
            if (slot.index === undefined) {
                setSlotIndex(slot, 0, indexMap);
            }
        }
    }

    function queueItemForInstantiation(slot) {
        // Check if item has already been queued
        if (!slot.instantiationState) {
            slot.instantiationState = InstantiationState.resourceFetchInProgress;

            // See if the resource fetch can begin immediately
            if (outstandingResourceFetches < simultaneousResourceFetches) {
                instantiateItemTree(slot);
            } else {
                // Append requested items to the low-priority queue
                var priority = slot.priority;
                if (priority === undefined) {
                    priority = thisWinUI.Priority.low;
                }
                queueSlot(slot, queues[priority]);
                pushResourceRequest(resourceRequest, priority);
            }

            do { if (slot.kind !== ""mirage"") { } else { assertionFailed(""slot.kind !== \""mirage\"""", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 1451); } } while (false);

            if (!slot.kind) {
                // The resource fetch was not synchronous, so a placeholder must be generated for now
                createPlaceholder(slot);
            }
        }
    }

    function slotDataPresent(slot) {
        return slot.dataObject !== undefined;
    }

    function readyForInstantiationQueue(slot) {
        return slotDataPresent(slot) &&
                !slot.firstInSequence && (slot.prev === slotsStart || slotDataPresent(slot.prev)) &&
                !slot.lastInSequence && (slot.next === slotsEnd || slotDataPresent(slot.next));
    }

    function prepareForInstantiation(slot) {
        var prevRequired = (slot.firstInSequence || (slot.prev !== slotsStart && !slotDataPresent(slot.prev)));
        var nextRequired = (slot.lastInSequence || (slot.next !== slotsEnd && !slotDataPresent(slot.next)));

        fetchItemsFromIdentity(slot, (prevRequired ? 1 : 0), (nextRequired ? 1 : 0));
    }

    function dataObjectChanged(slot) {
        var changed;
        if (compareByIdentity) {
            changed = (slot.dataObject !== slot.element.msDataItem.dataObject);
        } else {
            changed = (JSON.stringify(slot.dataObject) !== JSON.stringify(slot.element.msDataItem.dataObject));

            if (!changed) {
                // Ensure the identities match
                slot.dataObject = slot.element.msDataItem.dataObject;
            }
        }

        return changed;
    }

    function rerenderItem(slot, indexOld, forceRerender) {
        do { if (slot.kind === ""item"") { } else { assertionFailed(""slot.kind === \""item\"""", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 1494); } } while (false);
        var rendererOutput = renderItemElement(slot, slot.dataObject, slot.index);

        if (forceRerender || ensureHtmlString(rendererOutput) !== ensureHtmlString(renderItemElement(slot, slot.element.msDataItem.dataObject, indexOld))) {
            slot.elementTree = itemElement(rendererOutput, slot);
        }
    }

    function rerenderPlaceholder(slot, indexOld) {
        var rendererOutputOld = renderPlaceholderElement(slot, indexOld);
        var rendererOutput = renderPlaceholderElement(slot, slot.index);

        if (ensureHtmlString(rendererOutputOld) !== ensureHtmlString(rendererOutput)) {
            var elementOld = slot.element;
            var element = ensureDomElement(rendererOutput);
            handlerToNotify().changed(element, elementOld);
        }
    }

    function slotRequested(slot) {
        if (slot.element === undefined) {
            if (slot.released) {
                releasedSlots--;
                delete slot.released;
            }
            if (readyForInstantiationQueue(slot)) {
                queueItemForInstantiation(slot);
            } else {
                prepareForInstantiation(slot);

                if (!slot.kind) {
                    // The fetch to prepare for instantiation was not synchronous, so a placeholder must be generated
                    // for now.
                    createPlaceholder(slot);
                }
            }
        }

        return slot.element;
    }

    function slotCreated(slot) {
        if (slot.kind !== ""item"") {
            if (slot.kind === ""mirage"") {
                return null;
            }

            createPlaceholder(slot);
        }

        return slot.element;
    }

    function requestSlotBefore(slotNext, fetchItems) {
        // First, see if the previous slot already exists
        if (!slotNext.firstInSequence) {
            var slotPrev = slotNext.prev;

            // Next, see if the item is known to not exist
            if (slotPrev === slotsStart) {
                return null;
            } else {
                // Request the slot, i.e. ensure some kind of element exists
                return slotRequested(slotPrev);
            }
        }

        // Create a new slot and start a request for it
        var slotNew = addSlotBefore(slotNext, indexMap);
        fetchItems(slotNew);
        return slotCreated(slotNew);
    }

    function requestSlotAfter(slotPrev, fetchItems) {
        // First, see if the next slot already exists
        if (!slotPrev.lastInSequence) {
            var slotNext = slotPrev.next;

            // Next, see if the item is known to not exist
            if (slotNext === slotsEnd) {
                return null;
            } else {
                // Request the slot, i.e. ensure some kind of element exists
                return slotRequested(slotNext);
            }
        }

        // Create a new slot and start a request for it
        var slotNew = addSlotAfter(slotPrev, indexMap);
        fetchItems(slotNew);
        return slotCreated(slotNew);
    }

    function slotShouldBeFetched(slot) {
        return slot.element && !slotDataPresent(slot) && (slot.fetchID === undefined || !fetchesInProgress[slot.fetchID]);
    }

    function setFetchID(slot, fetchID) {
        if (slotShouldBeFetched(slot)) {
            slot.fetchID = fetchID;
        }
    }

    function newFetchID() {
        var fetchID = nextFetchID;
        ++nextFetchID;

        fetchesInProgress[fetchID] = true;

        return fetchID;
    }

    function setFetchIDs(slot, countBefore, countAfter) {
        var fetchID = newFetchID();
        setFetchID(slot, fetchID);

        var slotBefore = slot;
        while (!slotBefore.firstInSequence && countBefore > 0) {
            slotBefore = slotBefore.prev;
            --countBefore;
            setFetchID(slotBefore, fetchID);
        }

        var slotAfter = slot;
        while (!slotAfter.lastInSequence && countAfter > 0) {
            slotAfter = slotAfter.next;
            --countAfter;
            setFetchID(slotAfter, fetchID);
        }

        return fetchID;
    }

    function resultsCallback(slot, fetchID) {
        var refreshID = currentRefreshID;
        return function (results, offset, count, index) {
            processResults(slot, refreshID, fetchID, results, offset, count, index);
        };
    }

    function resultsForIndexCallback(indexRequested, slot) {
        var refreshID = currentRefreshID;
        return function (results, offset, count, index) {
            processResultsForIndex(indexRequested, slot, refreshID, results, offset, count, index);
        };
    }

    function fetchItemsFromStart(slot, count) {
        if (!refreshInProgress) {
            var fetchID = setFetchIDs(slot, 0, count - 1);

            if (dataSource.itemsFromStart) {
                dataSource.itemsFromStart(count, resultsCallback(slot, fetchID));
            } else {
                dataSource.itemsFromIndex(0, 0, count - 1, resultsCallback(slot, fetchID));
            }
        }
    }

    function fetchItemsFromEnd(slot, count) {
        if (!refreshInProgress) {
            var fetchID = setFetchIDs(slot, 0, count - 1);

            dataSource.itemsFromEnd(count, resultsCallback(slot, fetchID));
        }
    }

    function fetchItemsFromIdentity(slot, countBefore, countAfter) {
        if (!refreshInProgress) {
            var fetchID = setFetchIDs(slot, countBefore, countAfter);

            if (dataSource.itemsFromKey && slot.key !== undefined) {
                dataSource.itemsFromKey(slot.key, countBefore, countAfter, resultsCallback(slot, fetchID));
            } else {
                // Don't ask for items with negative indices
                var index = slot.index;
                dataSource.itemsFromIndex(index, Math.min(countBefore, index), countAfter, resultsCallback(slot, fetchID));
            }
        }
    }

    function fetchItemsFromIndex(slot, countBefore, countAfter) {
        do { if (slot !== slotsStart) { } else { assertionFailed(""slot !== slotsStart"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 1676); } } while (false);

        if (!refreshInProgress) {
            var index = slot.index;

            // Don't ask for items with negative indices
            if (countBefore > index) {
                countBefore = index;
            }

            if (dataSource.itemsFromIndex) {
                var fetchID = setFetchIDs(slot, countBefore, countAfter);

                dataSource.itemsFromIndex(index, countBefore, countAfter, resultsCallback(slot, fetchID));
            } else {
                // If the slot key is known, we just need to request the surrounding items
                if (slot.key !== undefined) {
                    fetchItemsFromIdentity(slot, countBefore, countAfter);
                } else {
                    // Search the instantiated list for the slot with the closest index that has a known key (using
                    // the start of the list as a last resort)
                    var slotClosest = slotsStart,
                        closestDelta = index + 1,
                        slotSearch,
                        delta;

                    // First search backwards
                    for (slotSearch = slot.prev; slotSearch !== slotsStart; slotSearch = slotSearch.prev) {
                        if (slotSearch.index !== undefined && slotSearch.key !== undefined) {
                            do { if (index > slotSearch.index) { } else { assertionFailed(""index > slotSearch.index"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 1705); } } while (false);
                            delta = index - slotSearch.index;
                            if (closestDelta > delta) {
                                closestDelta = delta;
                                slotClosest = slotSearch;
                            }
                            break;
                        }
                    }

                    // Then search forwards
                    for (slotSearch = slot.next; slotSearch !== slotsEnd; slotSearch = slotSearch.next) {
                        if (slotSearch.index !== undefined && slotSearch.key !== undefined) {
                            do { if (slotSearch.index > index) { } else { assertionFailed(""slotSearch.index > index"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 1718); } } while (false);
                            delta = slotSearch.index - index;
                            if (closestDelta > delta) {
                                closestDelta = delta;
                                slotClosest = slotSearch;
                            }
                            break;
                        }
                    }

                    if (slotClosest === slotsStart) {
                        dataSource.itemsFromStart(index + 1, resultsForIndexCallback(slotsStart.index, slot));
                    } else if (slotSearch.index !== undefined && slotSearch.key !== undefined) {
                        dataSource.itemsFromKey(
                            slotSearch.key,
                            Math.max(slotSearch.index - index, 0),
                            Math.max(index - slotSearch.index, 0), 
                            resultsForIndexCallback(slotSearch.index, slot)
                        );
                    }
                }
            }
        }
    }

    function fetchItemsFromPrefix(slot, prefix, countBefore, countAfter) {
        if (!refreshInProgress) {
            var fetchID = setFetchIDs(slot, countBefore, countAfter);

            dataSource.itemsFromPrefix(prefix, countBefore, countAfter, resultsCallback(slot, fetchID));
        }
    }

    function queueFetchFromStart(queue, slot, count) {
        queue.push(function () {
            fetchItemsFromStart(slot, count);
        });
    }

    function queueFetchFromEnd(queue, slot, count) {
        queue.push(function () {
            fetchItemsFromEnd(slot, count);
        });
    }

    function queueFetchFromIdentity(queue, slot, countBefore, countAfter) {
        queue.push(function () {
            fetchItemsFromIdentity(slot, countBefore, countAfter);
        });
    }

    function queueFetchFromIndex(queue, slot, countBefore, countAfter) {
        queue.push(function () {
            fetchItemsFromIndex(slot, countBefore, countAfter);
        });
    }

    function resetRefreshState() {
        // Give the start sentinel an index so we can always use predecessor + 1
        refreshStart = {
            firstInSequence: true,
            lastInSequence: true,
            index: -1
        };
        refreshEnd = {
            firstInSequence: true,
            lastInSequence: true
        };
        refreshStart.next = refreshEnd;
        refreshEnd.prev = refreshStart;


        refreshStart.debugInfo = ""*** refreshStart ***"";
        refreshEnd.debugInfo = ""*** refreshEnd ***"";


        refreshItemsFetched = false;
        refreshCount = thisWinUI.CountResult.unknown;
        keyFetchIDs = {};
        refreshKeyMap = {};
        refreshIndexMap = {};
        refreshIndexMap[-1] = refreshStart;
        deletedKeys = {};
    }

    function beginRefresh() {
        if (refreshRequested) {
            // There's already a refresh that has yet to start
            return;
        }

        refreshRequested = true;

        // TODO: Actually set this to waiting, and ready once all fetches have finished
        setStatus(thisWinUI.ItemsManagerStatus.ready);

        if (waitForRefresh) {
            waitForRefresh = false;

            // The edit queue has been paused until the next refresh - resume it now
            if (editsQueued) {
                do { if (applyNextEdit) { } else { assertionFailed(""applyNextEdit"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 1819); } } while (false);
                applyNextEdit();

                // This code is a little subtle.  If applyNextEdit emptied the queue, it will have cleared editsQueued
                // and called beginRefresh.  However, since refreshRequested is true, the latter will be a no-op, so
                // execution must fall through to the test for editsQueued below.
            }
        } 
        
        if (editsQueued) {
            // The refresh will be started once the edit queue empties out
            return;
        }

        ++currentRefreshID;
        refreshInProgress = true;
        refreshFetchesInProgress = 0;

        resetRefreshState();

        // Do the rest of the work asynchronously
        postCall(function () {
            refreshRequested = false;
            startRefreshFetches();
        });
    }

    function refreshCallback(key, fetchID) {
        var refreshID = currentRefreshID;
        return function (results, offset, count, index) {
            processRefreshResults(key, refreshID, fetchID, results, offset, count, index);
        };
    }

    function refreshRange(slot, fetchID, countBefore, countAfter) {
        var searchDelta = 20;

        ++refreshFetchesInProgress;

        if (dataSource.itemsFromKey) {
            // Keys are the preferred identifiers when the item might have moved

            // Fetch at least one item before and after, just to verify item's position in list
            dataSource.itemsFromKey(slot.key, countBefore + 1, countAfter + 1, refreshCallback(slot.key, fetchID));
        } else {
            // Request additional items to try to locate items that have moved (but don't ask for items with negative
            // indices)
            var index = slot.index;
            dataSource.itemsFromIndex(index, Math.min(countBefore + searchDelta, index), countAfter + searchDelta, refreshCallback(slot.key, fetchID));
        }
    }

    function refreshFirstItem(fetchID) {
        ++refreshFetchesInProgress;

        if (dataSource.itemsFromStart) {
            dataSource.itemsFromStart(1, refreshCallback(undefined, fetchID));
        } else {
            dataSource.itemsFromIndex(0, 0, 0, refreshCallback(undefined, fetchID));
        }
    }

    function keyFetchInProgress(key) {
        return fetchesInProgress[keyFetchIDs[key]];
    }

    function refreshRanges(slotFirst, allRanges) {
        // Fetch a few extra items each time, to catch insertions without requiring an extra fetch
        var refreshFetchExtra = 3;

        var refreshID = currentRefreshID;

        var slotFetchFirst,
            fetchCount = 0,
            fetchID;

        // Walk through the slot list looking for keys we haven't fetched or attempted to fetch yet
        // Rely on the heuristic that items that were close together before the refresh are likely to remain so after,
        // so batched fetches will locate most of the instantiated items
        for (var slot = slotFirst; slot !== slotsEnd; slot = slot.next) {
            if (slotFetchFirst === undefined && slot.kind === ""item"" && !deletedKeys[slot.key] && !keyFetchInProgress(slot.key)) {
                var slotRefresh = refreshKeyMap[slot.key];

                // Keep attempting to fetch an item until at least one item on either side of it has been observed, so
                // we can determine its position relative to others
                if (!slotRefresh || slotRefresh.firstInSequence || slotRefresh.lastInSequence) {
                    slotFetchFirst = slot;
                    fetchID = newFetchID();
                }
            }

            if (slotFetchFirst === undefined) {
                // Also attempt to fetch placeholders for requests for specific keys, just in case those items no
                // longer exist.
                if (slot.kind === ""placeholder"") {
                    if (slot.key !== undefined && slot.dataObject === undefined && !deletedKeys[slot.key]) {
                        // Fulfill each ""itemFromKey"" request
                        do { if (dataSource.itemsFromKey) { } else { assertionFailed(""dataSource.itemsFromKey"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 1916); } } while (false);
                        if (!refreshKeyMap[slot.key]) {
                            // Fetch at least one item before and after, just to verify item's position in list
                            ++refreshFetchesInProgress;
                            dataSource.itemsFromKey(slot.key, 1, 1, refreshCallback(slot.key, newFetchID()));
                        }
                    }
                }
            } else {
                var keyAlreadyFetched = keyFetchInProgress(slot.key);

                if (!deletedKeys[slot.key] && !refreshKeyMap[slot.key] && !keyAlreadyFetched) {
                    if (slot.kind === ""item"") {
                        keyFetchIDs[slot.key] = fetchID;
                    }
                    ++fetchCount;
                }

                if (slot.lastInSequence || slot.next === slotsEnd || keyAlreadyFetched) {
                    // TODO: fetch a random item from the middle of the list, rather than the first one?
                    refreshRange(slotFetchFirst, fetchID, 0, fetchCount - 1 + refreshFetchExtra);


                    fetchID = undefined;


                    if (!allRanges) {
                        break;
                    }

                    slotFetchFirst = undefined;
                    fetchCount = 0;
                }
            }
        }

        if (refreshFetchesInProgress === 0 && !refreshItemsFetched && currentRefreshID === refreshID) {
            // If nothing was successfully fetched, try fetching the first item, to detect an empty list
            refreshFirstItem(newFetchID());
        }

        do { if (fetchID === undefined) { } else { assertionFailed(""fetchID === undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 1957); } } while (false);
    }

    function startRefreshFetches() {
        var refreshID = currentRefreshID;

        do {
            synchronousProgress = false;
            reentrantContinue = true;
            refreshRanges(slotsStart.next, true);
            reentrantContinue = false;
        } while (refreshFetchesInProgress === 0 && synchronousProgress && currentRefreshID === refreshID);

        if (refreshFetchesInProgress === 0 && currentRefreshID === refreshID) {
            concludeRefresh();
        }
    }

    function continueRefresh(key) {
        var refreshID = currentRefreshID;

        // If the key is undefined, then the attempt to fetch the first item just completed, and there is nothing else
        // to fetch
        if (key !== undefined) {
            var slotContinue = keyMap[key];
            if (!slotContinue) {
                // In a rare case, the slot might have been deleted; just start scanning from the beginning again
                slotContinue = slotsStart.next;
            }

            do {
                synchronousRefresh = false;
                reentrantRefresh = true;
                refreshRanges(slotContinue, false);
                reentrantRefresh = false;
            } while (synchronousRefresh && currentRefreshID === refreshID);
        }

        if (reentrantContinue) {
            synchronousProgress = true;
        } else {
            if (refreshFetchesInProgress === 0 && currentRefreshID === refreshID) {
                // Walk through the entire list one more time, in case any edits were made during the refresh
                startRefreshFetches();
            }
        }
    }

    // Adds markers on behalf of the data source if their presence can be deduced
    function addMarkers(results, offset, count, index) {
        if (utilities.isNonNegativeNumber(index)) {
            if (utilities.isNonNegativeNumber(count)) {
                var resultsLength = results.length;
                if (results[resultsLength - 1] !== thisWinUI.endMarker && index - offset + resultsLength === count) {
                    results.push(thisWinUI.endMarker);
                }
            }

            if (results[0] !== thisWinUI.startMarker && offset === index) {
                results.unshift(thisWinUI.startMarker);
                ++offset;
            }
        }

        return offset;
    }

    function slotRefreshFromResult(result) {
        if (result === undefined) {
            throw new Error(undefinedItemReturned);
        } else if (result === thisWinUI.startMarker) {
            return refreshStart;
        } else if (result === thisWinUI.endMarker) {
            return refreshEnd;
        } else if (result.key === undefined || result.key === null) {
            throw new Error(invalidKeyReturned);
        } else {
            return refreshKeyMap[result.key];
        }
    }

    function processRefreshSlotIndex(slot, expectedIndex) {
        while (slot.index === undefined) {
            setSlotIndex(slot, expectedIndex, refreshIndexMap);

            if (slot.firstInSequence) {
                return true;
            }

            slot = slot.prev;
            --expectedIndex;
        }

        if (slot.index !== expectedIndex) {
            // Something has changed since the refresh began; start again
            beginRefresh();
            return false;
        }

        return true;
    }

    function copyRefreshSlotData(slotRefresh, slot) {
        setSlotKey(slot, slotRefresh.key);
        slot.dataObject = slotRefresh.dataObject;
    }

    function validateIndexReturned(index) {
        if (index === null) {
            index = undefined;
        } else if (index !== undefined && !utilities.isNonNegativeInteger(index)) {
            throw new Error(invalidIndexReturned);
        }

        return index;
    }

    function validateCountReturned(count) {
        if (count === null) {
            count = undefined;
        } else if (count !== undefined && !utilities.isNonNegativeInteger(count) && count !== thisWinUI.CountResult.unknown) {
            throw new Error(invalidCountReturned);
        }

        return count;
    }

    function validateDataObject(dataObject) {
        return compareByIdentity ? dataObject : utilities.validateDataObject(dataObject);
    }

    function setRefreshSlotResult(slotRefresh, result) {
        do { if (result.key !== undefined) { } else { assertionFailed(""result.key !== undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2089); } } while (false);
        slotRefresh.key = result.key;
        slotRefresh.dataObject = validateDataObject(result.dataObject);
        do { if (!refreshKeyMap[slotRefresh.key]) { } else { assertionFailed(""!refreshKeyMap[slotRefresh.key]"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2092); } } while (false);
        refreshKeyMap[slotRefresh.key] = slotRefresh;
    }

    function processRefreshResults(key, refreshID, fetchID, results, offset, count, index) {
        // This fetch has completed, whatever it has returned
        delete fetchesInProgress[fetchID];
        --refreshFetchesInProgress;

        if (refreshID !== currentRefreshID) {
            // This information is out of date.  Ignore it.
            return;
        }

        index = validateIndexReturned(index);
        count = validateCountReturned(count);

        checkListIntegrity(refreshStart, refreshEnd);

        // Check if an error result was returned
        if (results === thisWinUI.FetchResult.noResponse) {
            setStatus(thisWinUI.ItemsManagerStatus.failure);
            return;
        } else if (results === thisWinUI.FetchResult.doesNotExist) {
            if (key === undefined) {
                // The attempt to fetch the first item failed, so the list must be empty
                do { if (refreshStart.next === refreshEnd) { } else { assertionFailed(""refreshStart.next === refreshEnd"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2118); } } while (false);
                do { if (refreshStart.lastInSequence && refreshEnd.firstInSequence) { } else { assertionFailed(""refreshStart.lastInSequence && refreshEnd.firstInSequence"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2119); } } while (false);

                mergeSequences(refreshStart);

                refreshItemsFetched = true;
            } else {
                deletedKeys[key] = true;
            }
        } else {
            var keyPresent = false;

            refreshItemsFetched = true;

            offset = addMarkers(results, offset, count, index);

            var indexFirst = index - offset,
                result = results[0];

            if (result.key === key) {
                keyPresent = true;
            }

            var slot = slotRefreshFromResult(result);
            if (slot === undefined) {
                if (refreshIndexMap[indexFirst]) {
                    // Something has changed since the refresh began; start again
                    beginRefresh();
                    return;
                }

                // See if these results should be appended to an existing sequence
                var slotPrev;
                if (index !== undefined && (slotPrev = refreshIndexMap[indexFirst - 1])) {
                    if (!slotPrev.lastInSequence) {
                        // Something has changed since the refresh began; start again
                        beginRefresh();
                        return;
                    }
                    slot = addSlotAfter(slotPrev, refreshIndexMap);
                } else {
                    // Create a new sequence
                    var slotSuccessor = indexFirst === undefined ?
                            lastInsertionPoint(refreshStart, refreshEnd) :
                            successorFromIndex(indexFirst, refreshIndexMap, refreshStart, refreshEnd);

                    if (slotSuccessor === undefined) {
                        // Something has changed since the refresh began; start again
                        beginRefresh();
                        return;
                    }

                    slot = createSlotSequence(slotSuccessor, indexFirst, refreshIndexMap);
                }

                setRefreshSlotResult(slot, results[0]);
            } else {
                if (indexFirst !== undefined) {
                    if (!processRefreshSlotIndex(slot, indexFirst)) {
                        return;
                    }
                }
            }

            var resultsCount = results.length;
            for (var i = 1; i < resultsCount; ++i) {
                result = results[i];

                if (result.key === key) {
                    keyPresent = true;
                }

                var slotNext = slotRefreshFromResult(result);

                if (slotNext === undefined) {
                    if (!slot.lastInSequence) {
                        // Something has changed since the refresh began; start again
                        beginRefresh();
                        return;
                    }
                    slotNext = addSlotAfter(slot, refreshIndexMap);
                    setRefreshSlotResult(slotNext, result);
                } else {
                    if (slot.index !== undefined && !processRefreshSlotIndex(slotNext, slot.index + 1)) {
                        return;
                    }

                    // If the slots aren't adjacent, see if it's possible to reorder sequences to make them so
                    if (slotNext !== slot.next) {
                        if (!slot.lastInSequence || !slotNext.firstInSequence) {
                            // Something has changed since the refresh began; start again
                            beginRefresh();
                            return;
                        }

                        var slotLast = sequenceEnd(slotNext);
                        if (slotLast !== refreshEnd) {
                            moveSequenceAfter(slot, slotNext, slotLast);
                        } else {
                            var slotFirst = sequenceStart(slot);
                            if (slotFirst !== refreshStart) {
                                moveSequenceBefore(slotNext, slotFirst, slot);
                            } else {
                                // Something has changed since the refresh began; start again
                                beginRefresh();
                                return;
                            }
                        }

                        mergeSequences(slot);
                    } else if (slot.lastInSequence) {
                        do { if (slotNext.firstInSequence) { } else { assertionFailed(""slotNext.firstInSequence"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2229); } } while (false);

                        mergeSequences(slot);
                    }
                }

                slot = slotNext;
            }

            if (!keyPresent) {
                deletedKeys[key] = true;
            }
        }

        // If the count wasn't provided, see if it can be determined from the end of the list.
        if (!utilities.isNonNegativeNumber(count) && !refreshEnd.firstInSequence) {
            var indexLast = refreshEnd.prev.index;
            if (indexLast !== undefined) {
                count = indexLast + 1;
            }
        }

        if (utilities.isNonNegativeNumber(count) || count === thisWinUI.CountResult.unknown) {
            if (utilities.isNonNegativeNumber(refreshCount)) {
                if (count !== refreshCount) {
                    // Something has changed since the refresh began; start again
                    beginRefresh();
                    return;
                }
            } else {
                refreshCount = count;
            }
        }

        checkListIntegrity(refreshStart, refreshEnd);

        if (reentrantRefresh) {
            synchronousRefresh = true;
        } else {
            continueRefresh(key);
        }
    }

    function slotFromSlotRefresh(slotRefresh) {
        if (slotRefresh === refreshStart) {
            return slotsStart;
        } else if (slotRefresh === refreshEnd) {
            return slotsEnd;
        } else {
            return keyMap[slotRefresh.key];
        }
    }

    function slotRefreshFromSlot(slot) {
        if (slot === slotsStart) {
            return refreshStart;
        } else if (slot === slotsEnd) {
            return refreshEnd;
        } else {
            return refreshKeyMap[slot.key];
        }
    }

    function potentialRefreshMirage(slot) {
        return slot.kind === ""placeholder"" && !slot.indexRequested;
    }

    function mergeSequencesForRefresh(slotPrev) {
        mergeSequences(slotPrev);

        // Mark placeholders at the merge point as potential mirages
        var slot;
        for (slot = slotPrev; potentialRefreshMirage(slot); slot = slot.prev) {
            slot.potentialMirage = true;
        }
        for (slot = slotPrev.next; potentialRefreshMirage(slot); slot = slot.next) {
            slot.potentialMirage = true;
        }

        // Mark the merge point, so we can distinguish insertions from unrequested items
        slotPrev.next.mergedForRefresh = true;
    }

    function addNewSlot(slotRefresh, slotNext, insertAfter) {
        var slotNew = {};

        copyRefreshSlotData(slotRefresh, slotNew);
        setSlotIndex(slotNew, slotRefresh.index, indexMap);
        insertAndMergeSlot(slotNew, slotNext, insertAfter, !insertAfter);

        return slotNew;
    }

    function concludeRefresh() {
        keyFetchIDs = {};

        var i,
            j,
            slot,
            slotPrev,
            slotNext,
            slotRefresh,
            slotsAvailable = [],
            sequenceCountOld,
            sequencesOld = [],
            sequenceOld,
            sequenceOldPrev,
            sequenceOldBestMatch,
            sequenceCountNew,
            sequencesNew = [],
            sequenceNew,
            sequenceStart;

        checkListIntegrity(slotsStart, slotsEnd);
        checkListIntegrity(refreshStart, refreshEnd);

        // Assign a sequence number and slot number to each refresh slot
        var slotNumberNew = 0;
        sequenceCountNew = 0;
        for (slotRefresh = refreshStart; slotRefresh; slotRefresh = slotRefresh.next) {
            slotRefresh.sequenceNumber = sequenceCountNew;
            slotRefresh.number = slotNumberNew;
            ++slotNumberNew;

            if (slotRefresh.firstInSequence) {
                sequenceStart = slotRefresh;
            }

            if (slotRefresh.lastInSequence) {
                sequencesNew[sequenceCountNew] = {
                    first: sequenceStart,
                    last: slotRefresh,
                    matchingItems: 0
                };
                ++sequenceCountNew;
            }
        }

        // If the count is known, see if there are any placeholders with requested indices that exceed it
        if (utilities.isNonNegativeNumber(refreshCount)) {
            removeMirageIndices(refreshCount);
        }

        // Remove unnecessary information from main slot list, and update the dataObjects
        lastSlotReleased = undefined;
        releasedSlots = 0;
        for (slot = slotsStart.next; slot !== slotsEnd; ) {
            slotRefresh = refreshKeyMap[slot.key];
            slotNext = slot.next;
            if (!slot.element) {
                // Strip unrequested items from the main slot list, as they'll just get in the way from now on.
                // Since we're discarding these, but don't know if they're actually going away, split the sequence
                // as our starting assumption must be that the items on either side are in separate sequences.
                deleteUnrequestedSlot(slot);
            } else if (slot.key !== undefined && !slotRefresh) {
                // Remove items that have been deleted (or moved far away) and send removed notifications
                deleteSlot(slot, false);
            } else {
                // Clear keys and dataObjects that have never been observed by client
                if (slot.kind === ""placeholder"" && slot.key !== undefined && !slot.keyRequested) {
                    delete keyMap[slot.key];
                    delete slot.key;
                    delete slot.dataObject;
                }

                if (slotRefresh) {
                    // Overwrite the data object unconditionally; if there's a rendered item, this value will be compared
                    // with that stored in msDataItem later
                    slot.dataObject = slotRefresh.dataObject;
                }
            }

            slot = slotNext;
        }

        checkListIntegrity(slotsStart, slotsEnd);

        // Placeholders generated by itemsAtIndex, and adjacent placeholders, should not move.
        // Match these to items now if possible, or remove conflicting ones as mirages.
        for (slot = slotsStart.next; slot !== slotsEnd; ) {
            slotNext = slot.next;

            do { if (slot.element) { } else { assertionFailed(""slot.element"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2411); } } while (false);
            do { if (slot.key === undefined || refreshKeyMap[slot.key]) { } else { assertionFailed(""slot.key === undefined || refreshKeyMap[slot.key]"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2412); } } while (false);

            if (slot.indexRequested) {
                do { if (slot.kind === ""placeholder"") { } else { assertionFailed(""slot.kind === \""placeholder\"""", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2415); } } while (false);
                do { if (slot.index !== undefined) { } else { assertionFailed(""slot.index !== undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2416); } } while (false);

                slotRefresh = refreshIndexMap[slot.index];
                if (slotRefresh) {
                    if (slotFromSlotRefresh(slotRefresh)) {
                        deleteSlot(slot, true);
                    } else {
                        setSlotKey(slot, slotRefresh.key);
                        slot.dataObject = slotRefresh.dataObject;
                    }
                }
            }

            slot = slotNext;
        }

        checkListIntegrity(slotsStart, slotsEnd);

        // Match old sequences to new sequences
        var bestMatch,
            bestMatchCount,
            newSequenceCounts = [],
            sequenceIndexRequested,
            slotIndexRequested;

        sequenceCountOld = 0;
        for (slot = slotsStart; slot; slot = slot.next) {
            if (slot.firstInSequence) {
                sequenceStart = slot;
                sequenceIndexRequested = false;
                for (i = 0; i < sequenceCountNew; ++i) {
                    newSequenceCounts[i] = 0;
                }
            }

            if (slot.indexRequested) {
                sequenceIndexRequested = true;
                slotIndexRequested = slot;
            }

            slotRefresh = slotRefreshFromSlot(slot);
            if (slotRefresh) {
                do { if (slotRefresh.sequenceNumber !== undefined) { } else { assertionFailed(""slotRefresh.sequenceNumber !== undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2458); } } while (false);
                ++newSequenceCounts[slotRefresh.sequenceNumber];
            }

            if (slot.lastInSequence) {
                // Determine which new sequence is the best match for this old one
                bestMatchCount = 0;
                for (i = 0; i < sequenceCountNew; ++i) {
                    if (bestMatchCount < newSequenceCounts[i]) {
                        bestMatchCount = newSequenceCounts[i];
                        bestMatch = i;
                    }
                }

                sequenceOld = {
                    first: sequenceStart,
                    last: slot,
                    sequenceNew: (bestMatchCount > 0 ? sequencesNew[bestMatch] : undefined),
                    matchingItems: bestMatchCount
                };

                if (sequenceIndexRequested) {
                    sequenceOld.indexRequested = true;
                    sequenceOld.stationarySlot = slotIndexRequested;
                }

                sequencesOld[sequenceCountOld] = sequenceOld;

                ++sequenceCountOld;
            }
        }

        // Special case: split the old start into a separate sequence if the new start isn't its best match
        if (sequencesOld[0].sequenceNew !== sequencesNew[0]) {
            do { if (sequencesOld[0].first === slotsStart) { } else { assertionFailed(""sequencesOld[0].first === slotsStart"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2492); } } while (false);
            do { if (!slotsStart.lastInSequence) { } else { assertionFailed(""!slotsStart.lastInSequence"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2493); } } while (false);
            splitSequences(slotsStart);
            sequencesOld[0].first = slotsStart.next;
            sequencesOld.unshift({
                first: slotsStart,
                last: slotsStart,
                sequenceNew: sequencesNew[0],
                matchingItems: 1
            });
            ++sequenceCountOld;
        }

        // Special case: split the old end into a separate sequence if the new end isn't its best match
        if (sequencesOld[sequenceCountOld - 1].sequenceNew !== sequencesNew[sequenceCountNew - 1]) {
            do { if (sequencesOld[sequenceCountOld - 1].last === slotsEnd) { } else { assertionFailed(""sequencesOld[sequenceCountOld - 1].last === slotsEnd"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2507); } } while (false);
            do { if (!slotsEnd.firstInSequence) { } else { assertionFailed(""!slotsEnd.firstInSequence"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2508); } } while (false);
            splitSequences(slotsEnd.prev);
            sequencesOld[sequenceCountOld - 1].last = slotsEnd.prev;
            sequencesOld[sequenceCountOld] = {
                first: slotsEnd,
                last: slotsEnd,
                sequenceNew: sequencesNew[sequenceCountNew - 1],
                matchingItems: 1
            };
            ++sequenceCountOld;
        }

        // Map new sequences to old sequences
        for (i = 0; i < sequenceCountOld; ++i) {
            sequenceNew = sequencesOld[i].sequenceNew;
            if (sequenceNew && sequenceNew.matchingItems < sequencesOld[i].matchingItems) {
                sequenceNew.matchingItems = sequencesOld[i].matchingItems;
                sequenceNew.sequenceOld = sequencesOld[i];
            }
        }

        // The old start must always be the best match for the new start
        sequencesNew[0].sequenceOld = sequencesOld[0];
        sequencesOld[0].stationarySlot = slotsStart;

        // The old end must always be the best match for the new end (if the new end is also the new start, they will
        // be merged below).
        sequencesNew[sequenceCountNew - 1].sequenceOld = sequencesOld[sequenceCountOld - 1];
        sequencesOld[sequenceCountOld - 1].stationarySlot = slotsEnd;

        checkListIntegrity(slotsStart, slotsEnd);

        // Merge additional old sequences when possible

        // First do a forward pass
        for (i = 0; i < sequenceCountOld; ++i) {
            sequenceOld = sequencesOld[i];
            do { if (sequenceOld) { } else { assertionFailed(""sequenceOld"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2545); } } while (false);
            if (sequenceOld.sequenceNew && (sequenceOldBestMatch = sequenceOld.sequenceNew.sequenceOld) === sequenceOldPrev) {
                do { if (sequenceOldBestMatch.last.next === sequenceOld.first) { } else { assertionFailed(""sequenceOldBestMatch.last.next === sequenceOld.first"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2547); } } while (false);
                mergeSequencesForRefresh(sequenceOldBestMatch.last, sequenceOld.first);
                sequenceOldBestMatch.last = sequenceOld.last;
                delete sequencesOld[i];
            }
            else {
                sequenceOldPrev = sequenceOld;
            }
        }

        // Now do a reverse pass
        sequenceOldPrev = undefined;
        for (i = sequenceCountOld; i--; ) {
            sequenceOld = sequencesOld[i];
            // From this point onwards, some members of sequencesOld may be undefined
            if (sequenceOld) {
                if (sequenceOld.sequenceNew && (sequenceOldBestMatch = sequenceOld.sequenceNew.sequenceOld) === sequenceOldPrev) {
                    do { if (sequenceOld.last.next === sequenceOldBestMatch.first) { } else { assertionFailed(""sequenceOld.last.next === sequenceOldBestMatch.first"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2564); } } while (false);
                    mergeSequencesForRefresh(sequenceOld.last, sequenceOldBestMatch.first);
                    sequenceOldBestMatch.first = sequenceOld.first;
                    delete sequencesOld[i];
                } else {
                    sequenceOldPrev = sequenceOld;
                }
            }
        }

        // Remove placeholders in old sequences that don't map to new sequences (and don't contain requests for a
        // specific index), as they no longer have meaning.
        for (i = 0; i < sequenceCountOld; ++i) {
            sequenceOld = sequencesOld[i];
            if (sequenceOld && !sequenceOld.indexRequested && (!sequenceOld.sequenceNew || sequenceOld.sequenceNew.sequenceOld !== sequenceOld)) {
                sequenceOld.sequenceNew = undefined;

                slot = sequenceOld.first;
                while (true) {
                    slotNext = slot.next;

                    if (slot.kind === ""placeholder"") {
                        do { if (!slot.indexRequested) { } else { assertionFailed(""!slot.indexRequested"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2586); } } while (false);
                        deleteSlot(slot, true);
                        if (sequenceOld.first === slot) {
                            if (sequenceOld.last === slot) {
                                delete sequencesOld[i];
                                break;
                            } else {
                                sequenceOld.first = slot.next;
                            }
                        } else if (sequenceOld.last === slot) {
                            sequenceOld.last = slot.prev;
                        }
                    }

                    if (slot === sequenceOld.last) {
                        break;
                    }

                    slot = slotNext;
                }
            }
        }

        checkListIntegrity(slotsStart, slotsEnd);

        // Locate boundaries of new items in new sequences
        for (i = 0; i < sequenceCountNew; ++i) {
            sequenceNew = sequencesNew[i];
            for (slotRefresh = sequenceNew.first; !slotFromSlotRefresh(slotRefresh) && !slotRefresh.lastInSequence; slotRefresh = slotRefresh.next) {
            }
            if (slotRefresh.lastInSequence && !slotFromSlotRefresh(slotRefresh)) {
                sequenceNew.firstInner = sequenceNew.lastInner = undefined;
            } else {
                sequenceNew.firstInner = slotRefresh;
                for (slotRefresh = sequenceNew.last; !slotFromSlotRefresh(slotRefresh); slotRefresh = slotRefresh.prev) {
                }
                sequenceNew.lastInner = slotRefresh;
            }
        }

        // Determine which items to move
        for (i = 0; i < sequenceCountOld; ++i) {
            sequenceOld = sequencesOld[i];
            if (sequenceOld) {
                sequenceNew = sequenceOld.sequenceNew;
                if (sequenceNew !== undefined && sequenceNew.firstInner !== undefined) {
                    // Number the slots in each new sequence with their offset in the corresponding old sequence (or undefined
                    // if in a different old sequence)
                    var ordinal = 0;
                    for (slot = sequenceOld.first; true; slot = slot.next, ++ordinal) {
                        slotRefresh = slotRefreshFromSlot(slot);
                        if (slotRefresh && slotRefresh.sequenceNumber === sequenceNew.firstInner.sequenceNumber) {
                            slotRefresh.ordinal = ordinal;
                        }

                        if (slot.lastInSequence) {
                            do { if (slot === sequenceOld.last) { } else { assertionFailed(""slot === sequenceOld.last"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2642); } } while (false);
                            break;
                        }
                    }

                    // Determine longest subsequence of items that are in the same order before and after
                    var piles = [];
                    for (slotRefresh = sequenceNew.firstInner; true; slotRefresh = slotRefresh.next) {
                        ordinal = slotRefresh.ordinal;
                        if (ordinal !== undefined) {
                            var searchFirst = 0,
                                searchLast = piles.length - 1;
                            while (searchFirst <= searchLast) {
                                var searchMidpoint = Math.floor((searchFirst + searchLast) * 0.5);
                                if (piles[searchMidpoint].ordinal < ordinal) {
                                    searchFirst = searchMidpoint + 1;
                                } else {
                                    searchLast = searchMidpoint - 1;
                                }
                            }
                            piles[searchFirst] = slotRefresh;
                            if (searchFirst > 0) {
                                slotRefresh.predecessor = piles[searchFirst - 1];
                            }
                        }

                        if (slotRefresh === sequenceNew.lastInner) {
                            break;
                        }
                    }

                    // The items in the longest ordered subsequence don't move; everything else does
                    var stationaryItems = [],
                        stationaryItemCount = piles.length;
                    do { if (stationaryItemCount > 0) { } else { assertionFailed(""stationaryItemCount > 0"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2676); } } while (false);
                    slotRefresh = piles[stationaryItemCount - 1];
                    for (j = stationaryItemCount; j--; ) {
                        slotRefresh.stationary = true;
                        stationaryItems[j] = slotRefresh;
                        slotRefresh = slotRefresh.predecessor;
                    }
                    do { if (slotRefresh === undefined) { } else { assertionFailed(""slotRefresh === undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2683); } } while (false);
                    sequenceOld.stationarySlot = slotFromSlotRefresh(stationaryItems[0]);

                    // Try to match new items between stationary items to placeholders
                    for (j = 0; j < stationaryItemCount - 1; ++j) {
                        slotRefresh = stationaryItems[j];
                        slot = slotFromSlotRefresh(slotRefresh);
                        do { if (slot) { } else { assertionFailed(""slot"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2690); } } while (false);
                        var slotRefreshStop = stationaryItems[j + 1],
                            slotStop = slotFromSlotRefresh(slotRefreshStop);
                        do { if (slotStop) { } else { assertionFailed(""slotStop"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2693); } } while (false);

                        // Find all the new items
                        for (slotRefresh = slotRefresh.next; slotRefresh !== slotRefreshStop && slot !== slotStop; slotRefresh = slotRefresh.next) {
                            if (!slotFromSlotRefresh(slotRefresh)) {
                                // Find the next placeholder
                                for (slot = slot.next; slot !== slotStop; slot = slot.next) {
                                    if (slot.kind === ""placeholder"") {
                                        copyRefreshSlotData(slotRefresh, slot);
                                        slot.stationary = true;
                                        break;
                                    }
                                }
                            }
                        }

                        // Delete remaining placeholders, sending notifications
                        while (slot !== slotStop) {
                            slotNext = slot.next;

                            if (slot.kind === ""placeholder"" && slot.key === undefined) {
                                deleteSlot(slot, !!slot.potentialMirage);
                            }

                            slot = slotNext;
                        }
                    }
                }
            }
        }

        checkListIntegrity(slotsStart, slotsEnd);

        // Move items and send notifications
        for (i = 0; i < sequenceCountNew; ++i) {
            sequenceNew = sequencesNew[i];

            if (sequenceNew.firstInner) {
                slotPrev = undefined;
                for (slotRefresh = sequenceNew.firstInner; true; slotRefresh = slotRefresh.next) {
                    slot = slotFromSlotRefresh(slotRefresh);
                    if (slot) {
                        if (!slotRefresh.stationary) {
                            do { if (slot !== slotsStart) { } else { assertionFailed(""slot !== slotsStart"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2736); } } while (false);
                            do { if (slot !== slotsEnd) { } else { assertionFailed(""slot !== slotsEnd"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2737); } } while (false);

                            var slotMoveBefore,
                                mergeWithPrev = false,
                                mergeWithNext = false;
                            if (slotPrev) {
                                slotMoveBefore = slotPrev.next;
                                mergeWithPrev = true;
                            } else {
                                // The first item will be inserted before the first stationary item, so find that now
                                var slotRefreshStationary;
                                for (slotRefreshStationary = sequenceNew.firstInner; !slotRefreshStationary.stationary && slotRefreshStationary !== sequenceNew.lastInner; slotRefreshStationary = slotRefreshStationary.next) {
                                }

                                if (!slotRefreshStationary.stationary) {
                                    // There are no stationary items, as all the items are moving from another old sequence

                                    var index = slotRefresh.index;

                                    // Find the best place to insert the new sequence
                                    if (index === 0) {
                                        // Index 0 is a special case
                                        slotMoveBefore = slotsStart.next;
                                        mergeWithPrev = true;
                                    } else {
                                        slotMoveBefore = index === undefined ?
                                            lastInsertionPoint(slotsStart, slotsEnd) :
                                            successorFromIndex(index, indexMap, slotsStart, slotsEnd);
                                    }
                                } else {
                                    slotMoveBefore = slotFromSlotRefresh(slotRefreshStationary);
                                    mergeWithNext = true;
                                }
                            }

                            // Preserve merge boundaries
                            if (slot.mergedForRefresh) {
                                delete slot.mergedForRefresh;
                                if (!slot.lastInSequence) {
                                    slot.next.mergedForRefresh = true;
                                }
                            }

                            moveSlot(slot, slotMoveBefore, mergeWithPrev, mergeWithNext);
                        }

                        slotPrev = slot;
                    }

                    if (slotRefresh === sequenceNew.lastInner) {
                        break;
                    }
                }
            }
        }

        checkListIntegrity(slotsStart, slotsEnd);

        // Insert new items (with new indices) and send notifications
        for (i = 0; i < sequenceCountNew; ++i) {
            sequenceNew = sequencesNew[i];

            if (sequenceNew.firstInner) {
                slotPrev = undefined;
                for (slotRefresh = sequenceNew.firstInner; true; slotRefresh = slotRefresh.next) {
                    slot = slotFromSlotRefresh(slotRefresh);
                    if (!slot) {
                        var slotInsertBefore;
                        if (slotPrev) {
                            slotInsertBefore = slotPrev.next;
                        } else {
                            // The first item will be inserted *before* the first old item, so find that now
                            var slotRefreshOld;
                            for (slotRefreshOld = sequenceNew.firstInner; !slotFromSlotRefresh(slotRefreshOld); slotRefreshOld = slotRefreshOld.next) {
                                do { if (slotRefreshOld !== sequenceNew.lastInner) { } else { assertionFailed(""slotRefreshOld !== sequenceNew.lastInner"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2811); } } while (false);
                            }
                            slotInsertBefore = slotFromSlotRefresh(slotRefreshOld);
                        }

                        // Create a new slot for the item
                        slot = addNewSlot(slotRefresh, slotInsertBefore, !!slotPrev);

                        if (!slotInsertBefore.mergedForRefresh) {
                            // Instantiate the item now
                            do { if (readyForInstantiationQueue(slot)) { } else { assertionFailed(""readyForInstantiationQueue(slot)"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2821); } } while (false);
                            queueItemForInstantiation(slot);

                            // Send the notification after the insertion
                            sendInsertedNotification(slot);
                        }
                    }
                    slotPrev = slot;

                    if (slotRefresh === sequenceNew.lastInner) {
                        break;
                    }
                }
            }
        }

        checkListIntegrity(slotsStart, slotsEnd);

        // Set placeholder indices, merge sequences and send mirage notifications if necessary, match outer new items
        // to outer placeholders, add extra outer new items (possibly merging with Start, End)
        for (i = 0; i < sequenceCountOld; ++i) {
            sequenceOld = sequencesOld[i];
            if (sequenceOld) {
                do { if (sequenceOld.stationarySlot) { } else { assertionFailed(""sequenceOld.stationarySlot"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2844); } } while (false);
                sequenceNew = sequenceOld.sequenceNew;
                if (sequenceNew) {
                    // Re-establish the start of sequenceOld, since it might have been invalidated by the moves and insertions
                    var slotBefore = sequenceOld.stationarySlot;
                    while (!slotBefore.firstInSequence) {
                        slotBefore = slotBefore.prev;
                    }
                    sequenceOld.first = slotBefore;

                    // Walk backwards through outer placeholders and new items at the start of the sequence
                    while (potentialRefreshMirage(slotBefore)) {
                        do { if (!slotBefore.lastInSequence) { } else { assertionFailed(""!slotBefore.lastInSequence"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2856); } } while (false);
                        slotBefore = slotBefore.next;
                    }

                    var newItemBefore = sequenceNew ? sequenceNew.firstInner : undefined,
                        indexBefore = slotBefore.index;

                    while (!slotBefore.firstInSequence) {
                        --indexBefore;

                        // Check for index collision with other sequences
                        if (indexBefore !== undefined) {
                            var slotCollisionBefore = indexMap[indexBefore];
                            if (slotCollisionBefore && slotCollisionBefore !== slotBefore.prev) {
                                removeMiragesAndMerge(slotCollisionBefore, slotBefore);
                                break;
                            }

                            if (slotBefore.prev.index !== indexBefore) {
                                do { if (slotBefore.prev.kind === ""placeholder"") { } else { assertionFailed(""slotBefore.prev.kind === \""placeholder\"""", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2875); } } while (false);
                                do { if (!slotBefore.prev.indexRequested) { } else { assertionFailed(""!slotBefore.prev.indexRequested"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2876); } } while (false);
                                changeSlotIndex(slotBefore.prev, indexBefore, indexMap);
                            }
                        }

                        slotBefore = slotBefore.prev;

                        // Match items
                        if (newItemBefore) {
                            if (newItemBefore.firstInSequence) {
                                newItemBefore = undefined;
                            } else {
                                newItemBefore = newItemBefore.prev;
                                copyRefreshSlotData(newItemBefore, slotBefore);
                            }
                        }
                    }

                    if (newItemBefore) {
                        // Add extra new items to the start of the sequence
                        while (!newItemBefore.firstInSequence) {
                            do { if (slotBefore.firstInSequence) { } else { assertionFailed(""slotBefore.firstInSequence"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2897); } } while (false);
                            newItemBefore = newItemBefore.prev;

                            if (newItemBefore === refreshStart) {
                                mergeSequences(slotsStart);
                                break;
                            } else {
                                do { if (sequenceOld.first === slotBefore) { } else { assertionFailed(""sequenceOld.first === slotBefore"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2904); } } while (false);
                                slotBefore = addNewSlot(newItemBefore, slotBefore, false);
                                sequenceOld.first = slotBefore;
                            }
                        }
                    }

                    // Re-establish the end of sequenceOld, since it might have been invalidated by the moves and insertions
                    var slotAfter = sequenceOld.stationarySlot;
                    while (!slotAfter.lastInSequence) {
                        slotAfter = slotAfter.next;
                    }
                    sequenceOld.last = slotAfter;

                    // Walk forwards through outer placeholders and new items at the end of the sequence
                    while (potentialRefreshMirage(slotAfter)) {
                        do { if (!slotAfter.firstInSequence) { } else { assertionFailed(""!slotAfter.firstInSequence"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2920); } } while (false);
                        slotAfter = slotAfter.prev;
                    }

                    var newItemAfter = sequenceNew ? sequenceNew.lastInner : undefined,
                        indexAfter = slotAfter.index;

                    while (!slotAfter.lastInSequence) {
                        ++indexAfter;

                        // Check for index collision with other sequences
                        if (indexAfter !== undefined) {
                            var slotCollisionAfter = indexMap[indexAfter];
                            if (slotCollisionAfter && slotCollisionAfter !== slotAfter.next) {
                                removeMiragesAndMerge(slotAfter, slotCollisionAfter);
                                break;
                            }

                            if (slotAfter.next.index !== indexAfter) {
                                do { if (slotAfter.next.kind === ""placeholder"") { } else { assertionFailed(""slotAfter.next.kind === \""placeholder\"""", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2939); } } while (false);
                                do { if (!slotAfter.next.indexRequested) { } else { assertionFailed(""!slotAfter.next.indexRequested"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2940); } } while (false);
                                changeSlotIndex(slotAfter.next, indexAfter, indexMap);
                            }
                        }

                        slotAfter = slotAfter.next;

                        // Match items
                        if (newItemAfter) {
                            if (newItemAfter.lastInSequence) {
                                newItemAfter = undefined;
                            } else {
                                newItemAfter = newItemAfter.next;
                                copyRefreshSlotData(newItemAfter, slotAfter);
                            }
                        }
                    }

                    if (newItemAfter) {
                        // Add extra new items to the end of the sequence
                        while (!newItemAfter.lastInSequence) {
                            do { if (slotAfter.lastInSequence) { } else { assertionFailed(""slotAfter.lastInSequence"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2961); } } while (false);
                            newItemAfter = newItemAfter.next;

                            if (newItemAfter === refreshEnd) {
                                mergeSequences(slotAfter.prev);
                                break;
                            } else {
                                do { if (sequenceOld.last === slotAfter) { } else { assertionFailed(""sequenceOld.last === slotAfter"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2968); } } while (false);
                                slotAfter = addNewSlot(newItemAfter, slotAfter.next, true);
                                sequenceOld.last = slotAfter;
                            }
                        }
                    }
                }
            }
        }

        checkListIntegrity(slotsStart, slotsEnd);

        // Instantiate all items, detect changes; send itemAvailable, changed, indexChanged notifications
        for (i = 0; i < sequenceCountOld; ++i) {
            sequenceOld = sequencesOld[i];
            if (sequenceOld) {
                var offset = 0,
                    indexFirst;

                // Find a reference index for the entire sequence
                indexFirst = undefined;
                for (slot = sequenceOld.first; true; slot = slot.next, ++offset) {
                    if (slot === slotsStart) {
                        indexFirst = -1;
                    } else if (slot.indexRequested) {
                        do { if (slot.index !== undefined) { } else { assertionFailed(""slot.index !== undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 2993); } } while (false);
                        indexFirst = slot.index - offset;
                        // TODO: Handle case of slot.index being out of sync with results indices
                    } else if (indexFirst === undefined && slot.key !== undefined) {
                        var indexNew = refreshKeyMap[slot.key].index;
                        if (indexNew !== undefined) {
                            indexFirst = indexNew - offset;
                        }
                    }

                    // Clean up in this final pass
                    delete slot.potentialMirage;
                    delete slot.mergedForRefresh;

                    if (slot.lastInSequence) {
                        do { if (slot === sequenceOld.last) { } else { assertionFailed(""slot === sequenceOld.last"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 3008); } } while (false);
                        break;
                    }
                }

                updateItemRange(sequenceOld.first, sequenceOld.last, indexFirst, undefined, sequenceOld.first, sequenceOld.last);
            }
        }

        checkListIntegrity(slotsStart, slotsEnd);

        // Send countChanged notification
        if (refreshCount !== knownCount) {
            changeCount(refreshCount);
        }

        var fetches = [];

        // Kick-start fetches for remaining placeholders
        for (i = 0; i < sequenceCountOld; ++i) {
            sequenceOld = sequencesOld[i];

            if (sequenceOld) {
                var firstPlaceholder,
                    placeholderCount,
                    slotRequestedByIndex,
                    requestedIndexOffset,
                    lastItem;

                firstPlaceholder = undefined;
                slotRequestedByIndex = undefined;
                lastItem = undefined;
                for (slot = sequenceOld.first; true; slot = slot.next) {
                    if (slot.kind === ""placeholder"") {
                        // Count the number of placeholders in a row
                        if (firstPlaceholder === undefined) {
                            firstPlaceholder = slot;
                            placeholderCount = 1;
                        } else {
                            ++placeholderCount;
                        }

                        // If this group of slots was requested by index, re-request them that way (since that may be the only way to get them)
                        if (slot.indexRequested && slotRequestedByIndex === undefined) {
                            do { if (slot.index !== undefined) { } else { assertionFailed(""slot.index !== undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 3052); } } while (false);
                            slotRequestedByIndex = slot;
                            requestedIndexOffset = placeholderCount - 1;
                        }
                    } else if (slot.kind === ""item"") {
                        if (firstPlaceholder !== undefined) {
                            // Fetch the group of placeholders before this item
                            queueFetchFromIdentity(fetches, slot, placeholderCount + 1, 0);
                            firstPlaceholder = undefined;
                            slotRequestedByIndex = undefined;
                        }

                        lastItem = slot;
                    }

                    if (slot.lastInSequence) {
                        if (firstPlaceholder !== undefined) {
                            if (lastItem !== undefined) {
                                // Fetch the group of placeholders after the last item
                                queueFetchFromIdentity(fetches, lastItem, 0, placeholderCount + 1);
                            } else if (firstPlaceholder.prev === slotsStart) {
                                // Fetch the group of placeholders at the start
                                queueFetchFromStart(fetches, firstPlaceholder, placeholderCount + 1);
                            } else if (slot === slotsEnd) {
                                // Fetch the group of placeholders at the end
                                queueFetchFromEnd(fetches, slot.prev, placeholderCount + 1);
                            } else {
                                do { if (slotRequestedByIndex !== undefined) { } else { assertionFailed(""slotRequestedByIndex !== undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 3079); } } while (false);

                                // Fetch the group of placeholders by index
                                queueFetchFromIndex(fetches, slotRequestedByIndex, requestedIndexOffset + 1, placeholderCount - requestedIndexOffset);
                            }
                        }

                        do { if (slot === sequenceOld.last) { } else { assertionFailed(""slot === sequenceOld.last"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 3086); } } while (false);
                        break;
                    }
                }
            }
        }

        finishNotifications();

        resetRefreshState();
        refreshInProgress = false;

        if (applyNextEdit) {
            applyNextEdit();
        }

        var fetchCount = fetches.length;
        for (i = 0; i < fetchCount; ++i) {
            fetches[i]();
        }
    }

    function slotFromResult(result, candidateKeyMap) {
        if (result === undefined) {
            throw new Error(undefinedItemReturned);
        } else if (result === null) {
            return undefined;
        } else if (result === thisWinUI.startMarker) {
            return slotsStart;
        } else if (result === thisWinUI.endMarker) {
            return slotsEnd;
        } else if (result.key === undefined || result.key === null) {
            throw new Error(invalidKeyReturned);
        } else {
            // A requested slot gets the highest priority...
            var slot = keyMap[result.key];
            if (slot && slot.element !== undefined) {
                return slot;
            } else {
                if (candidateKeyMap) {
                    // ...then a candidate placeholder...
                    var candidate = candidateKeyMap[result.key];
                    if (candidate) {
                        return candidate;
                    }
                }

                // ...then an unrequested item, if any
                return slot;
            }
        }
    }

    // Returns true if the given slot and result refer to different items
    function slotResultMismatch(slot, result) {
        return slot.key !== undefined && result !== null && result.key !== undefined && slot.key !== result.key;
    }

    // Searches for placeholders that could map to members of the results array.  If there is more than one candidate
    // for a given result, either would suffice, so use the first one.
    function generateCandidateKeyMap(results) {
        var candidateKeyMap = {},
            resultsCount = results.length;

        for (var offset = 0; offset < resultsCount; ++offset) {
            var slot = slotFromResult(results[offset], undefined);
            if (slot !== undefined) {
                // Walk backwards from the slot looking for candidate placeholders
                var slotBefore = slot,
                    offsetBefore = offset;
                while (offsetBefore > 0 && !slotBefore.firstInSequence) {
                    slotBefore = slotBefore.prev;
                    --offsetBefore;

                    if (slotBefore.kind !== ""placeholder"") {
                        break;
                    }

                    var resultBefore = results[offsetBefore];
                    if (resultBefore && resultBefore.key !== undefined) {
                        candidateKeyMap[resultBefore.key] = slotBefore;
                    }
                }

                // Walk forwards from the slot looking for candidate placeholders
                var slotAfter = slot,
                    offsetAfter = offset;
                while (offsetAfter < resultsCount - 1 && !slotAfter.lastInSequence) {
                    slotAfter = slotAfter.next;
                    ++offsetAfter;

                    if (slotAfter.kind !== ""placeholder"") {
                        break;
                    }

                    var resultAfter = results[offsetAfter];
                    if (resultAfter && resultAfter.key !== undefined) {
                        candidateKeyMap[resultAfter.key] = slotAfter;
                    }
                }
            }
        }

        return candidateKeyMap;
    }

    // Processes a single result returned by a data source.  Returns true if the result is consistent with the current
    // state of the slot, false otherwise.
    function processResult(slot, result) {
        delete slot.fetchID;

        if (result === null) {
            setStatus(thisWinUI.ItemsManagerStatus.failure);
        } else {
            if (slot.key !== undefined) {
                // If there's a key assigned to this slot already, and it's not that of the result, something has
                // changed
                if (slot.key !== result.key) {
                    return false;
                }
            } else {
                setSlotKey(slot, result.key);
            }

            // Overwrite the data object unconditionally; if there's a rendered item, this value will be compared with
            // that stored in msDataItem later
            slot.dataObject = validateDataObject(result.dataObject);
        }

        return true;
    }

    function potentialMirage(slot) {
        return (slot.kind === ""placeholder"" && !slot.indexRequested) || slot.element === undefined;
    }

    function sequenceStart(slot) {
        while (!slot.firstInSequence) {
            slot = slot.prev;
        }

        return slot;
    }

    function sequenceEnd(slot) {
        while (!slot.lastInSequence) {
            slot = slot.next;
        }

        return slot;
    }

    // Returns true if slotBefore and slotAfter can be made adjacent by simply removing ""mirage"" placeholders and
    // merging two sequences.
    function mergePossible(slotBefore, slotAfter, notificationsPermitted, asynchronousContinuation) {
        // If anything after slotBefore other than placeholders (even slotAfter is bad!), return false
        var slotBeforeEnd = slotBefore;
        while (!slotBeforeEnd.lastInSequence) {
            slotBeforeEnd = slotBeforeEnd.next;
            if (!potentialMirage(slotBeforeEnd)) {
                return false;
            }
        }

        // If anything before slotAfter other than placeholders (even slotBefore is bad!), return false
        var slotAfterStart = slotAfter;
        while (!slotAfterStart.firstInSequence) {
            slotAfterStart = slotAfterStart.prev;
            if (!potentialMirage(slotAfterStart)) {
                return false;
            }
        }

        // If slotBefore and slotAfter aren't in adjacent sequences, ensure that at least one of them can be moved
        if (slotBeforeEnd.next !== slotAfterStart &&
                sequenceStart(slotBefore) === slotsStart && sequenceEnd(slotAfter) === slotsEnd) {
            return false;
        }

        // If slotBefore and slotAfter are in the same sequence (in reverse order), return false!
        while (!slotBefore.firstInSequence) {
            slotBefore = slotBefore.prev;
            if (slotBefore === slotAfter) {
                return false;
            }
        }

        return true;
    }

    // Returns true if there are any instantiated items that will need to be removed before slotAfter can be positioned
    // immediately after slotBefore in the list.
    function mergeRequiresNotifications(slotBefore, slotAfter) {
        while (!slotBefore.lastInSequence) {
            slotBefore = slotBefore.next;
            if (slotBefore.element) {
                return true;
            }
        }

        while (!slotAfter.firstInSequence) {
            slotAfter = slotAfter.prev;
            if (slotAfter.element) {
                return true;
            }
        }

        return false;
    }

    // Does a little careful surgery to the slot sequence from slotFirst to slotLast before slotNext
    function moveSequenceBefore(slotNext, slotFirst, slotLast) {
        do { if (slotFirst !== slotsStart) { } else { assertionFailed(""slotFirst !== slotsStart"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 3298); } } while (false);
        do { if (slotLast !== slotsEnd) { } else { assertionFailed(""slotLast !== slotsEnd"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 3299); } } while (false);
        do { if (slotFirst.firstInSequence) { } else { assertionFailed(""slotFirst.firstInSequence"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 3300); } } while (false);
        do { if (slotLast.lastInSequence) { } else { assertionFailed(""slotLast.lastInSequence"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 3301); } } while (false);
        do { if (slotNext.firstInSequence) { } else { assertionFailed(""slotNext.firstInSequence"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 3302); } } while (false);
        do { if (slotNext.prev.lastInSequence) { } else { assertionFailed(""slotNext.prev.lastInSequence"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 3303); } } while (false);

        slotFirst.prev.next = slotLast.next;
        slotLast.next.prev = slotFirst.prev;

        slotFirst.prev = slotNext.prev;
        slotLast.next = slotNext;

        slotFirst.prev.next = slotFirst;
        slotNext.prev = slotLast;

        return true;
    }

    // Does a little careful surgery to the slot sequence from slotFirst to slotLast after slotPrev
    function moveSequenceAfter(slotPrev, slotFirst, slotLast) {
        do { if (slotFirst !== slotsStart) { } else { assertionFailed(""slotFirst !== slotsStart"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 3319); } } while (false);
        do { if (slotLast !== slotsEnd) { } else { assertionFailed(""slotLast !== slotsEnd"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 3320); } } while (false);
        do { if (slotFirst.firstInSequence) { } else { assertionFailed(""slotFirst.firstInSequence"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 3321); } } while (false);
        do { if (slotLast.lastInSequence) { } else { assertionFailed(""slotLast.lastInSequence"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 3322); } } while (false);
        do { if (slotPrev.lastInSequence) { } else { assertionFailed(""slotPrev.lastInSequence"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 3323); } } while (false);
        do { if (slotPrev.next.firstInSequence) { } else { assertionFailed(""slotPrev.next.firstInSequence"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 3324); } } while (false);

        slotFirst.prev.next = slotLast.next;
        slotLast.next.prev = slotFirst.prev;

        slotFirst.prev = slotPrev;
        slotLast.next = slotPrev.next;

        slotPrev.next = slotFirst;
        slotLast.next.prev = slotLast;

        return true;
    }

    function removeMiragesAndMerge(slotBefore, slotAfter) {
        do { if (slotBefore.next !== slotAfter || (slotBefore.lastInSequence && slotAfter.firstInSequence)) { } else { assertionFailed(""slotBefore.next !== slotAfter || (slotBefore.lastInSequence && slotAfter.firstInSequence)"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 3339); } } while (false);

        // If slotBefore and slotAfter aren't in adjacent sequences, ensure that at least one of them can be moved
        if (sequenceEnd(slotBefore).next !== sequenceStart(slotAfter) &&
                sequenceStart(slotBefore) === slotsStart && sequenceEnd(slotAfter) === slotsEnd) {
            return false;
        }

        // Remove the placeholders and unrequested items after slotBefore
        while (!slotBefore.lastInSequence) {
            do { if (slotBefore.kind !== ""item"") { } else { assertionFailed(""slotBefore.kind !== \""item\"""", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 3349); } } while (false);
            deleteSlot(slotBefore.next, true);
        }

        // Remove the placeholders and unrequested items before slotAfter
        while (!slotAfter.firstInSequence) {
            do { if (slotAfter.kind !== ""item"") { } else { assertionFailed(""slotAfter.kind !== \""item\"""", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 3355); } } while (false);
            deleteSlot(slotAfter.prev, true);
        }

        // Move one sequence if necessary
        if (slotBefore.next !== slotAfter) {
            var slotLast = sequenceEnd(slotAfter);
            if (slotLast !== slotsEnd) {
                moveSequenceAfter(slotBefore, slotAfter, slotLast);
            } else {
                moveSequenceBefore(slotAfter, sequenceStart(slotBefore), slotBefore);
            }
        }

        // Proceed with the merge
        mergeSequences(slotBefore);

        return true;
    }

    // Updates the indices of a range of items, rerenders them as necessary (or queues them for rerendering), and sends
    // indexChanged notifications
    function updateItemRange(slotFirst, slotLast, indexFirst, slotNew, slotFirstChanged, slotLastChanged) {
        var slot = slotFirst,
            index = indexFirst,
            inNewRange;
        while (true) {
            var indexOld = slot.index,
                indexChanged = false,
                element = slot.element;

            if (slot === slotFirstChanged) {
                do { if (inNewRange === undefined) { } else { assertionFailed(""inNewRange === undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 3387); } } while (false);
                inNewRange = true;
            }

            if (index !== indexOld) {
                do { if (slot !== slotsStart || index === -1) { } else { assertionFailed(""slot !== slotsStart || index === -1"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 3392); } } while (false);
                changeSlotIndex(slot, index, indexMap);
                if (slot.element) {
                    indexChanged = true;
                }
            }

            if (slot.element || slot === slotNew) {
                if (slot.kind === ""item"") {
                    // If we're in the region for which new results just arrived, see if the dataObject changed
                    if (inNewRange && dataObjectChanged(slot)) {
                        slot.dataObjectDifferent = true;
                    }

                    // If it did, or if the index changed and was observed, rerender the item
                    if (slot.dataObjectDifferent || (indexChanged && slot.indexObserved)) {
                        slot.indexOld = indexOld;
                        queueItemForInstantiation(slot);
                    }
                } else {
                    do { if (!slot.kind || slot.kind === ""placeholder"") { } else { assertionFailed(""!slot.kind || slot.kind === \""placeholder\"""", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 3412); } } while (false);

                    // A requested slot is treated as a placeholder
                    if (readyForInstantiationQueue(slot)) {
                        queueItemForInstantiation(slot);
                    } else if (indexChanged && slot.indexObserved) {
                        rerenderPlaceholder(slot, indexOld);
                    }
                }
            }

            // Send out index change notifications after we have at least tried to rerender the items
            if (indexChanged && slot.element && elementNotificationHandler.indexChanged) {
                handlerToNotify().indexChanged(slot.element, slot.index, indexOld);
            }

            if (slot === slotLast) {
                break;
            }

            if (slot === slotLastChanged) {
                inNewRange = false;
            }

            slot = slot.next;
            ++index;
        }
    }

    // Removes any placeholders with requested indices that exceed the given upper bound on the count
    function removeMirageIndices(countMax) {
        do { if (utilities.isNonNegativeInteger(countMax)) { } else { assertionFailed(""utilities.isNonNegativeInteger(countMax)"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 3443); } } while (false);

        for (var slot = slotsEnd.prev; slot !== slotsStart; ) {
            var slotPrev = slot.prev;

            if (slot.index < countMax) {
                break;
            } else if (slot.indexRequested) {
                do { if (slot.kind === ""placeholder"") { } else { assertionFailed(""slot.kind === \""placeholder\"""", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 3451); } } while (false);
                do { if (slot.index !== undefined) { } else { assertionFailed(""slot.index !== undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 3452); } } while (false);

                deleteSlot(slot, true);
            }

            slot = slotPrev;
        }
    }

    // Adjust the indices of all slots to be consistent with any indexNew properties, and strip off the indexNews
    function updateIndices() {
        indexUpdateDeferred = false;

        var slotFirstInSequence,
            indexNew;

        for (var slot = slotsStart; slot; slot = slot.next) {
            if (slot.firstInSequence) {
                slotFirstInSequence = slot;
                if (slot.indexNew !== undefined) {
                    indexNew = slot.indexNew;
                    delete slot.indexNew;
                } else {
                    indexNew = slot.index;
                }
            }

            if (slot.lastInSequence) {
                updateItemRange(slotFirstInSequence, slot, indexNew, undefined, slotFirstInSequence, slot);
            }
        }

        if (countDelta && knownCount !== undefined) {
            changeCount(knownCount + countDelta);

            countDelta = 0;
        }
    }

    function processResultsAsynchronously(slot, refreshID, fetchID, results, offset, count, index) {
        postCall(function () {
            processResults(slot, refreshID, fetchID, results, offset, count, index);
        });
    }

    // Merges the results of a fetch into the slot list data structure, and determines if any notifications need to be
    // synthesized.
    function processResults(slot, refreshID, fetchID, results, offset, count, index) {
        // This fetch has completed, whatever it has returned
        delete fetchesInProgress[fetchID];

        if (refreshID !== currentRefreshID) {
            // This information is out of date.  Ignore it.
            return;
        }

        index = validateIndexReturned(index);
        count = validateCountReturned(count);

        if (indexUpdateDeferred) {
            updateIndices();
        }

        checkListIntegrity(slotsStart, slotsEnd);

        var refreshRequired = false,
            countChanged = false,
            countMax,
            slotFirst,
            fetchCountBefore = 0,
            slotLast,
            fetchCountAfter = 0;

        (function () {
            var synchronousCallback = (slot.element === undefined);

            // Check if an error result was returned
            if (results === thisWinUI.FetchResult.noResponse) {
                setStatus(thisWinUI.ItemsManagerStatus.failure);
                return;
            } else if (results === thisWinUI.FetchResult.doesNotExist) {
                if (slot.key === undefined) {
                    if (!utilities.isNonNegativeNumber(count) && slot.indexRequested) {
                        do { if (slot.index !== undefined) { } else { assertionFailed(""slot.index !== undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 3535); } } while (false);

                        // We now have an upper bound on the count
                        if (countMax === undefined || countMax > slot.index) {
                            countMax = slot.index;
                        }
                    }

                    // This item counts as a mirage, since for all we know it never existed
                    if (synchronousCallback) {
                        removeSlotPermanently(slot);
                        slot.kind = ""mirage"";
                    } else {
                        deleteSlot(slot, true);
                    }
                }

                // It's likely that the client requested this item because something has changed since the client's
                // latest observations of the data.  Begin a refresh just in case.
                refreshRequired = true;
                return;
            }

            offset = addMarkers(results, offset, count, index);

            // See if the result returned already exists in a different slot
            var slotExisting = slotFromResult(results[offset], undefined);
            if ((slotExisting !== undefined && slotExisting !== slot) || !processResult(slot, results[offset])) {
                // A contradiction has been found, so we can't proceed further
                refreshRequired = true;
                return;
            }

            // Now determine how the other results fit into the slot list

            var mergeQueue = [];

            // First generate a map of existing placeholders that could map to the results
            var candidateKeyMap = generateCandidateKeyMap(results);

            // Now walk backwards from the given slot
            var slotBefore = slot,
                offsetBefore = offset,
                fetchCountDetermined = false;
            while (true) {
                if (offsetBefore > 0) {
                    // There are still results to process
                    var slotExpectedBefore = slotFromResult(results[offsetBefore - 1], candidateKeyMap);
                    if (slotExpectedBefore !== undefined) {
                        if (slotBefore.firstInSequence || slotExpectedBefore !== slotBefore.prev) {
                            if (!mergePossible(slotExpectedBefore, slotBefore)) {
                                // A contradiction has been found, so we can't proceed further
                                refreshRequired = true;
                                return;
                            } else if (synchronousCallback && mergeRequiresNotifications(slotExpectedBefore, slotBefore)) {
                                // Process these results from an asynchronous call
                                processResultsAsynchronously(slot, refreshID, fetchID, results, offset, count, index);
                                return;
                            } else {
                                // Unrequested items will be silently deleted, but if they don't match the items that
                                // are arriving now, consider that a refresh hint
                                var slotMirageBefore = slotBefore,
                                    offsetMirageBefore = offsetBefore;
                                while (offsetMirageBefore > 0 && !slotMirageBefore.firstInSequence) {
                                    slotMirageBefore = slotMirageBefore.prev;
                                    --offsetMirageBefore;
                                    if (slotResultMismatch(slotMirageBefore, results[offsetMirageBefore])) {
                                        refreshRequired = true;
                                    }
                                }

                                mergeQueue.push({ slotBefore: slotExpectedBefore, slotAfter: slotBefore });
                            }
                        }
                        slotBefore = slotExpectedBefore;
                    } else if (slotBefore.firstInSequence) {
                        slotBefore = addSlotBefore(slotBefore, indexMap);
                    } else {
                        slotBefore = slotBefore.prev;
                    }
                    --offsetBefore;

                    if (slotBefore === slotsStart) {
                        break;
                    }

                    if (!processResult(slotBefore, results[offsetBefore])) {
                        // A contradiction has been found, so we can't proceed further
                        refreshRequired = true;
                        return;
                    }
                } else {
                    // Keep walking to determine (and verify consistency) of indices, if necessary

                    if (offsetBefore === 0) {
                        slotFirst = slotBefore;
                    }

                    if (slotBefore.firstInSequence) {
                        break;
                    }

                    slotBefore = slotBefore.prev;
                    --offsetBefore;

                    if (!fetchCountDetermined) {
                        if (slotShouldBeFetched(slotBefore)) {
                            ++fetchCountBefore;
                        } else {
                            fetchCountDetermined = true;
                        }
                    }
                }

                // See if the indices are consistent
                if (slotBefore.index !== undefined) {
                    var indexGivenSlotBefore = slotBefore.index + offset - offsetBefore;
                    if (index !== undefined) {
                        if (index !== indexGivenSlotBefore) {
                            // A contradiction has been found, so we can't proceed further
                            refreshRequired = true;
                            return;
                        }
                    } else {
                        // This is the first information we have about the indices of any of these slots
                        index = indexGivenSlotBefore;
                    }
                }

                // Once the results are processed, it's only necessary to walk until the index is known (if it isn't
                // already) and the number of additional items to fetch has been determined
                if (fetchCountDetermined && index !== undefined) {
                    break;
                }
            }

            // Then walk forwards
            var slotAfter = slot,
                offsetAfter = offset;

            fetchCountDetermined = false;

            var resultsCount = results.length;
            while (true) {
                if (offsetAfter < resultsCount - 1) {
                    // There are still results to process
                    var slotExpectedAfter = slotFromResult(results[offsetAfter + 1], candidateKeyMap);
                    if (slotExpectedAfter !== undefined) {
                        if (slotAfter.lastInSequence || slotExpectedAfter !== slotAfter.next) {
                            if (!mergePossible(slotAfter, slotExpectedAfter)) {
                                // A contradiction has been found, so we can't proceed further
                                refreshRequired = true;
                                return;
                            } else if (synchronousCallback && mergeRequiresNotifications(slotAfter, slotExpectedAfter)) {
                                // Process these results from an asynchronous call
                                processResultsAsynchronously(slot, refreshID, fetchID, results, offset, count, index);
                                return;
                            } else {
                                // Unrequested items will be silently deleted, but if they don't match the items that
                                // are arriving now, consider that a refresh hint
                                var slotMirageAfter = slotAfter,
                                    offsetMirageAfter = offsetAfter;
                                while (offsetMirageAfter < resultsCount - 1 && !slotMirageAfter.lastInSequence) {
                                    slotMirageAfter = slotMirageAfter.next;
                                    ++offsetMirageAfter;
                                    if (slotResultMismatch(slotMirageAfter, results[offsetMirageAfter])) {
                                        refreshRequired = true;
                                    }
                                }

                                mergeQueue.push({ slotBefore: slotAfter, slotAfter: slotExpectedAfter });
                            }
                        }
                        slotAfter = slotExpectedAfter;
                    } else if (slotAfter.lastInSequence) {
                        slotAfter = addSlotAfter(slotAfter, indexMap);
                    } else {
                        slotAfter = slotAfter.next;
                    }
                    ++offsetAfter;

                    if (slotAfter === slotsEnd) {
                        break;
                    }

                    if (!processResult(slotAfter, results[offsetAfter])) {
                        // A contradiction has been found, so we can't proceed further
                        refreshRequired = true;
                        return;
                    }
                } else {
                    // Keep walking to determine (and verify consistency) of indices, if necessary

                    if (offsetAfter === resultsCount - 1) {
                        slotLast = slotAfter;
                    }

                    if (slotAfter.lastInSequence) {
                        break;
                    }

                    slotAfter = slotAfter.next;
                    ++offsetAfter;

                    if (!fetchCountDetermined) {
                        if (slotShouldBeFetched(slotAfter)) {
                            ++fetchCountAfter;
                        } else {
                            fetchCountDetermined = true;
                        }
                    }
                }

                // See if the indices are consistent
                if (slotAfter.index !== undefined) {
                    var indexGivenSlotAfter = slotAfter.index + offset - offsetAfter;
                    if (index !== undefined) {
                        if (index !== indexGivenSlotAfter) {
                            // A contradiction has been found, so we can't proceed further
                            refreshRequired = true;
                            return;
                        }
                    } else {
                        // This is the first information we have about the indices of any of these slots
                        index = indexGivenSlotAfter;
                    }
                }

                // Once the results are processed, it's only necessary to walk until the index is known (if it isn't
                // already) and the number of additional items to fetch has been determined
                if (fetchCountDetermined && index !== undefined) {
                    break;
                }
            }

            // We're ready to perform the sequence merges, although in rare cases a contradiction might still be found
            while (mergeQueue.length > 0) {
                var merge = mergeQueue.pop();
                if (!removeMiragesAndMerge(merge.slotBefore, merge.slotAfter)) {
                    // A contradiction has been found, so we can't proceed further
                    refreshRequired = true;
                    return;
                }
            }

            // Now walk through the entire range of interest, and detect items that can now be rendered, items that have
            // changed, and indices that were unknown but are now known
            updateItemRange(slotBefore, slotAfter, index - offset + offsetBefore, slot.released ? undefined : slot, slotFirst, slotLast);
        })();

        // If the count wasn't provided, see if it can be determined from the end of the list.
        if (!utilities.isNonNegativeNumber(count) && !slotsEnd.firstInSequence) {
            var indexLast = slotsEnd.prev.index;
            if (indexLast !== undefined) {
                count = indexLast + 1;
            }
        }

        // If the count has changed, and the end of the list had been reached, that's a hint to refresh, but
        // since there are no known contradictions we can proceed with what we have.
        if (utilities.isNonNegativeNumber(count) || count === thisWinUI.CountResult.unknown) {
            if (utilities.isNonNegativeNumber(knownCount)) {
                if (count !== knownCount) {
                    countChanged = true;
                    if (!slotsEnd.firstInSequence) {
                        // Don't send the countChanged notification until the refresh, so don't update knownCount now
                        refreshRequired = true;
                    }
                }
            } else {
                countChanged = true;
            }
        }

        if (utilities.isNonNegativeNumber(count)) {
            removeMirageIndices(count);
        } else if (countMax !== undefined) {
            removeMirageIndices(countMax);
        }

        if (refreshRequired) {
            beginRefresh();
        } else {
            // If the count changed, but that's the only thing, just send the notification
            if (countChanged) {
                changeCount(count);
            }

            // See if there are more requests we can now fulfill
            if (fetchCountBefore > 0) {
                fetchItemsFromIdentity(slotFirst, fetchCountBefore + 1, 0);
            }
            if (fetchCountAfter > 0) {
                fetchItemsFromIdentity(slotLast, 0, fetchCountAfter + 1);
            }
        }

        finishNotifications();

        checkListIntegrity(slotsStart, slotsEnd);
    }

    function processResultsForIndex(indexRequested, slot, refreshID, fetchID, results, offset, count, index) {
        if (refreshID !== currentRefreshID) {
            // This information is out of date.  Ignore it.
            return;
        }

        index = validateIndexReturned(index);
        count = validateCountReturned(count);

        if (results === thisWinUI.FetchResult.noResponse) {
            setStatus(thisWinUI.ItemsManagerStatus.failure);
        } else if (results === thisWinUI.FetchResult.doesNotExist) {
            if (indexRequested === slotsStart.index) {
                // The request was for the start of the list, so the item must not exist
                processResults(slot, refreshID, undefined, thisWinUI.FetchResult.doesNotExist);
            } else {
                // Something has changed, so request a refresh
                beginRefresh();
            }
        } else if (index !== undefined && index !== indexRequested) {
            // Something has changed, so request a refresh
            beginRefresh();
        } else {
            var indexFirst = indexRequested - offset;

            var resultsCount = results.length;
            if (slot.index >= indexFirst && slot.index < indexFirst + resultsCount) {
                // The item is in this batch of results - process them all
                processResults(slot, refreshID, undefined, results, offset, count, index);
            } else if (offset === resultsCount - 1 && indexRequested < slot.index) {
                // The requested index does not exist
                // Let processResults handle this case
                processResults(slot, refreshID, undefined, thisWinUI.FetchResult.doesNotExist);
            } else {
                // We didn't get all the results we requested - pick up where they left off
                if (slot.index < indexFirst) {
                    dataSource.itemsFromKey(
                        results[0].key,
                        indexFirst - slot.index,
                        0,
                        resultsForIndexCallback(indexFirst, slot)
                    );
                } else {
                    var indexLast = indexFirst + resultsCount - 1;
                    do { if (slot.index > indexLast) { } else { assertionFailed(""slot.index > indexLast"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 3881); } } while (false);

                    dataSource.itemsFromKey(
                        results[resultsCount - 1].key,
                        0,
                        slot.index - indexLast,
                        resultsForIndexCallback(indexLast, slot)
                    );
                }
            }
        }        
    }

    function reduceReleasedSlotCount() {
        // If lastSlotReleased has been removed from the list, use the end of the list instead
        if (!lastSlotReleased.prev) {
            do { if (!lastSlotReleased.next) { } else { assertionFailed(""!lastSlotReleased.next"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 3897); } } while (false);
            lastSlotReleased = slotsEnd.prev;
        }

        // Retain at least half the maximum number, but remove a substantial number
        var releasedSlotsTarget = Math.max(releasedSlotsMax / 2, Math.min(releasedSlotsMax * 0.9, releasedSlotsMax - 10));

        // Now use the simple heuristic of walking outwards in both directions from lastSlotReleased until the target
        // count is reached, the removing everything else
        var slotPrev = lastSlotReleased.prev,
            slotNext = lastSlotReleased.next,
            releasedSlotsFound = 0,
            slotToDelete;

        function considerDeletingSlot() {
            if (slotToDelete.released) {
                if (releasedSlotsFound <= releasedSlotsTarget) {
                    releasedSlotsFound++;
                } else {
                    deleteUnrequestedSlot(slotToDelete);
                }
            }
        }

        while (slotPrev && slotNext) {
            if (slotPrev) {
                slotToDelete = slotPrev;
                slotPrev = slotToDelete.prev;
                considerDeletingSlot();
            }
            if (slotNext) {
                slotToDelete = slotNext;
                slotNext = slotToDelete.next;
                considerDeletingSlot();
            }
        }
    }

    // Updates the new index of the first slot in each sequence after the given slot
    function updateNewIndicesAfterSlot(slot, indexDelta) {
        // Adjust all the indexNews after this slot
        for (slot = slot.next; slot; slot = slot.next) {
            if (slot.firstInSequence) {
                indexNew = (slot.indexNew !== undefined ? slot.indexNew : slot.index);
                if (indexNew !== undefined) {
                    slot.indexNew = indexNew + indexDelta;
                }
            }
        }

        // Adjust the overall count
        countDelta += indexDelta;

        indexUpdateDeferred = true;

        // Increment currentRefreshID so any outstanding fetches don't cause trouble.  If a refresh is in progress,
        // restart it (which will also increment currentRefreshID).
        if (refreshInProgress) {
            beginRefresh();
        } else {
            currentRefreshID++;
        }
    }

    // Updates the new index of the given slot if necessary, and all subsequent new indices
    function updateNewIndices(slot, indexDelta) {
        do { if (indexDelta !== 0) { } else { assertionFailed(""indexDelta !== 0"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 3963); } } while (false);

        // If this slot is at the start of a sequence, transfer the indexNew
        if (slot.firstInSequence) {
            var indexNew;

            if (indexDelta < 0) {
                // The given slot is about to be removed
                indexNew = slot.indexNew;
                if (indexNew !== undefined) {
                    delete slot.indexNew;
                } else {
                    indexNew = slot.index;
                }

                if (!slot.lastInSequence) {
                    // Update the next slot now
                    slot = slot.next;
                    if (indexNew !== undefined) {
                        slot.indexNew = indexNew;
                    }
                }
            } else {
                // The given slot was just inserted
                if (!slot.lastInSequence) {
                    var slotNext = slot.next;

                    indexNew = slotNext.indexNew;
                    if (indexNew !== undefined) {
                        delete slotNext.indexNew;
                    } else {
                        indexNew = slotNext.index;
                    }

                    if (indexNew !== undefined) {
                        slot.indexNew = indexNew;
                    }
                }
            }
        }

        updateNewIndicesAfterSlot(slot, indexDelta);
    }

    // Updates the new index of the first slot in each sequence after the given new index
    function updateNewIndicesFromIndex(index, indexDelta) {
        do { if (indexDelta !== 0) { } else { assertionFailed(""indexDelta !== 0"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 4009); } } while (false);

        for (var slot = slotsStart; slot !== slotsEnd; slot = slot.next) {
            var indexNew = slot.indexNew;

            if (indexNew !== undefined && index <= indexNew) {
                updateNewIndicesAfterSlot(slot, indexDelta);
                break;
            }
        }        
    }

    function instantiateNewItem(slot) {
        queueItemForInstantiation(slot);

        // Send the notification after the insertion
        sendInsertedNotification(slot);
    }

    function insertNewSlot(key, dataObject, slotInsertBefore, mergeWithPrev, mergeWithNext, instantiate) {
        // Create a new slot, but don't worry about its index, as indices will be updated during endEdits
        var slot = {};
        insertAndMergeSlot(slot, slotInsertBefore, mergeWithPrev, mergeWithNext);
        setSlotKey(slot, key);
        slot.dataObject = dataObject;

        updateNewIndices(slot, 1);

        if (instantiate) {
            instantiateNewItem(slot);
        }

        return slot;
    }

    function reinstantiateItem(slot, forceChangedNotification) {
        // See if the item renders differently now.  This should be done synchronously to
        // maintain a consistent observable state.  Doing so will force the resources for
        // this item to the front of the queue, but the point of this is to give immediate
        // visual feedback.
        if (slot.kind === ""item"") {
            instantiateItemTree(slot, true, forceChangedNotification);
        }
    }

    function ListEditor() {
        /// <summary>
        ///     Constructor for list editor object, which can be returned to client of Items Manager to enable editing
        ///     of the list contents.
        /// </summary>

        this.beginEdits = function () {
            /// <summary>
            ///     Notifies the Items Manager that a sequence of edits is about to begin.  The Items Manager will call
            ///     beginNotifications and endNotifications once each for a sequence of edits.
            /// </summary>

            editsInProgress = true;
        };

        var editQueue = {};
        editQueue.next = editQueue;
        editQueue.prev = editQueue;


        editQueue.debugInfo = ""*** editQueueHead/Tail ***"";


        function dequeueEdit() {
            do { if (editQueue.next !== editQueue) { } else { assertionFailed(""editQueue.next !== editQueue"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 4078); } } while (false);
            var editNext = editQueue.next.next;

            editQueue.next = editNext;
            editNext.prev = editQueue;
        }

        var synchronousEdit;

        function attemptEdit(edit) {
            var keyUpdate = edit.keyUpdate;

            var reentrant = true;
            edit.applyEdit(function (result, keyNew) {
                var EditResult = thisWinUI.EditResult;
                switch (result) {
                    case EditResult.success:
                        if (keyUpdate && keyUpdate.key !== keyNew) {
                            if (reentrant) {
                                // We can use the correct key, so there's no need for a later update
                                keyUpdate.key = keyNew;
                            } else {
                                do { if (keyUpdate.slot) { } else { assertionFailed(""keyUpdate.slot"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 4100); } } while (false);
                                var slot = keyUpdate.slot;
                                var keyOld = slot.key;
                                if (keyOld) {
                                    do { if (slot.key === keyOld) { } else { assertionFailed(""slot.key === keyOld"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 4104); } } while (false);
                                    do { if (keyMap[keyOld] === slot) { } else { assertionFailed(""keyMap[keyOld] === slot"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 4105); } } while (false);
                                    delete keyMap[keyOld];
                                }

                                // setSlotKey asserts that the slot key is undefined
                                delete slot.key;

                                setSlotKey(slot, keyNew);

                                reinstantiateItem(slot);

                                // msDataItem.key may or may not have been updated, so ensure that it is
                                slot.element.msDataItem.key = keyNew;

                                if (elementNotificationHandler.keyChanged) {
                                    handlerToNotify().keyChanged(slot.element, slot.key, keyOld);
                                    finishNotifications();
                                }
                            }
                        }

                        dequeueEdit();
                        break;

                    case EditResult.noResponse:
                        // Report the failure to the client, but do not dequeue the edit
                        setStatus(thisWinUI.ItemsManagerStatus.failure);
                        waitForRefresh = true;
                        break;

                    case EditResult.notPermitted:
                        // Discard all remaining edits, rather than try to determine which subsequent ones depend
                        // on this one
                        edit.failed = true;
                        discardEditQueue();
                        break;

                    case EditResult.noLongerMeaningful:
                        if (edit.isDeletion) {
                            // Special case - if a deletion is no longer meaningful, assume that's because the item no
                            // longer exists, in which case there's no point in undoing it
                            dequeueEdit();
                        } else {
                            // Discard all remaining edits, rather than try to determine which subsequent ones depend
                            // on this one
                            edit.failed = true;
                            discardEditQueue();
                        }

                        // Something has changed, so request a refresh
                        beginRefresh();
                        break;
                }

                // Notify the client of the edit result
                if (edit.editResult) {
                    edit.editResult(result);
                }

                if (!waitForRefresh) {
                    if (reentrant) {
                        synchronousEdit = true;
                    } else {
                        applyNextEdit();
                    }
                }
            });
            reentrant = false;
        }

        // Define ItemsManager's applyNextEdit method now
        applyNextEdit = function () {
            // See if there are any outstanding edits, and try to process as many as possible synchronously
            while (editQueue.next !== editQueue) {
                synchronousEdit = false;
                attemptEdit(editQueue.next);
                if (!synchronousEdit) {
                    return;
                }
            }

            // The queue emptied out synchronously (or was empty to begin with)
            concludeEdits();
        };

        // Queues an edit and immediately ""optimistically"" apply it to the slots list, sending reentrant notifications
        function queueEdit(applyEdit, keyUpdate, isDeletion, editResult, updateSlots, undo) {
            var editQueueTail = editQueue.prev,
                edit = {
                    prev: editQueueTail,
                    next: editQueue,
                    applyEdit: applyEdit,
                    keyUpdate: keyUpdate,
                    isDeletion: isDeletion,
                    editResult: editResult
                };
            editQueueTail.next = edit;
            editQueue.prev = edit;
            editsQueued = true;

            if (!refreshInProgress && editQueue.next === edit) {
                // Attempt the edit immediately, in case it completes synchronously
                attemptEdit(edit);
            }

            // If the edit succeeded or is still pending, apply it to the slots (in the latter case, ""optimistically"")
            if (!edit.failed) {
                updateSlots();

                // Supply the undo function now
                edit.undo = undo;
            }

            if (!editsInProgress) {
                completeEdits();
            }
        }

        // Once the edit queue has emptied, update state appropriately and resume normal operation
        function concludeEdits() {
            editsQueued = false;

            // See if there's a refresh that needs to begin
            if (refreshRequested) {
                refreshRequested = false;
                beginRefresh();
            }
        }

        function completeEdits() {
            do { if (!editsInProgress) { } else { assertionFailed(""!editsInProgress"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 4235); } } while (false);

            updateIndices();

            finishNotifications();

            if (editQueue.next === editQueue) {
                concludeEdits();
            }
        }

        // Undo all queued edits, starting with the most recent
        function discardEditQueue() {
            while (editQueue.prev !== editQueue) {
                var editLast = editQueue.prev;

                // Edits that haven't been applied to the slots yet don't need to be undone
                if (editLast.undo) {
                    editLast.undo();
                }

                editQueue.prev = editLast.prev;
            }
            editQueue.next = editQueue;

            editsInProgress = false;

            completeEdits();
        }

        // Only implement each editing method if the data source implements the corresponding DataSource method

        function insertItem(key, dataObject, slotInsertBefore, append, instantiate, editResult, applyEdit) {
            // It is acceptable to pass null in as a temporary key, but since we need unique keys, one will be
            // generated
            if (key === null) {
                key = ""__temp`"" + nextTempKey++;
            }

            dataObject = validateDataObject(dataObject);

            var keyUpdate = { key: key };

            queueEdit(
                // applyEdit
                applyEdit,

                // keyUpdate, isDeletion, editResult
                keyUpdate, false, editResult,

                // updateSlots
                function () {
                    keyUpdate.slot = insertNewSlot(keyUpdate.key, dataObject, slotInsertBefore, append, !append, instantiate);
                },

                // undo
                function () {
                    var slot = keyUpdate.slot;

                    updateNewIndices(slot, -1);
                    deleteSlot(slot, false);
                }
            );
        }

        if (dataSource.insertAtStart) {
            this.insertAtStart = function (key, dataObject, editResult) {
                /// <summary>
                ///     Inserts an item at the start of the list.
                /// </summary>
                /// <param name=""key"" mayBeNull=""true"" type=""String"">
                ///     The unique key of the item, if known.
                /// </param>
                /// <param name=""dataObject"" type=""Object"">
                ///     The data object of the item.
                /// </param>
                /// <param name=""editResult"" optional=""true"" type=""Function"">
                ///     Function to call when the result of the edit is known.  Function's signature must match that of
                ///     editResultFunction.
                /// </param>

                // Add item to start of list, only instantiate and notify if the first item was instantiated
                insertItem(
                    key, dataObject, slotsStart.next, true, !slotsStart.lastInSequence, editResult,

                    // applyEdit
                    function (editResult) {
                        dataSource.insertAtStart(key, dataObject, editResult);
                    }
                );
            };
        }

        if (dataSource.insertBefore) {
            this.insertBefore = function (key, dataObject, nextItem, editResult) {
                /// <summary>
                ///     Inserts an item before a given item in the list.
                /// </summary>
                /// <param name=""key"" mayBeNull=""true"" type=""String"">
                ///     The unique key of the item, if known.
                /// </param>
                /// <param name=""dataObject"" type=""Object"">
                ///     The data object of the item.
                /// </param>
                /// <param name=""nextItem"" type=""Object"" domElement=""true"">
                ///     The item immediately after the insertion point.
                /// </param>
                /// <param name=""editResult"" optional=""true"" type=""Function"">
                ///     Function to call when the result of the edit is known.  Function's signature must match that of
                ///     editResultFunction.
                /// </param>

                var slotNext = slotFromItem(nextItem);

                if (slotNext.kind !== ""item"") {
                    editResult(thisWinUI.EditResult.itemNotReady);
                } else {
                    // Add item before given item, instantiate it, and send notification
                    insertItem(
                        key, dataObject, slotNext, false, true, editResult,
                        
                        // applyEdit
                        function (editResult) {
                            dataSource.insertBefore(key, dataObject, slotNext.key, editResult, slotNext.index);
                        }
                    );
                }
            };
        }

        if (dataSource.insertAfter) {
            this.insertAfter = function (key, dataObject, previousItem, editResult) {
                /// <summary>
                ///     Inserts an item after a given item in the list.
                /// </summary>
                /// <param name=""key"" mayBeNull=""true"" type=""String"">
                ///     The unique key of the item, if known.
                /// </param>
                /// <param name=""dataObject"" type=""Object"">
                ///     The data object of the item.
                /// </param>
                /// <param name=""previousItem"" type=""Object"" domElement=""true"">
                ///     The item immediately before the insertion point.
                /// </param>
                /// <param name=""editResult"" optional=""true"" type=""Function"">
                ///     Function to call when the result of the edit is known.  Function's signature must match that of
                ///     editResultFunction.
                /// </param>

                var slotPrev = slotFromItem(previousItem);

                if (slotPrev.kind !== ""item"") {
                    editResult(thisWinUI.EditResult.itemNotReady);
                } else {
                    // Add item after given item, instantiate it, and send notification
                    insertItem(
                        key, dataObject, slotPrev.next, true, true, editResult,

                        // applyEdit
                        function (editResult) {
                            dataSource.insertAfter(key, dataObject, slotPrev.key, editResult, slotPrev.index);
                        }
                    );
                }
            };
        }

        if (dataSource.insertAtEnd) {
            this.insertAtEnd = function (key, dataObject, editResult) {
                /// <summary>
                ///     Inserts an item at the end of the list.
                /// </summary>
                /// <param name=""key"" mayBeNull=""true"" type=""String"">
                ///     The unique key of the item, if known.
                /// </param>
                /// <param name=""dataObject"" type=""Object"">
                ///     The data object of the item.
                /// </param>
                /// <param name=""editResult"" optional=""true"" type=""Function"">
                ///     Function to call when the result of the edit is known.  Function's signature must match that of
                ///     editResultFunction.
                /// </param>

                // Add item to start of list, only instantiate and notify if the first item was instantiated
                insertItem(
                    key, dataObject, slotsEnd, false, !slotsEnd.firstInSequence, editResult,

                    // applyEdit
                    function (editResult) {
                        dataSource.insertAtEnd(key, dataObject, editResult);
                    }
                );
            };
        }

        if (dataSource.change) {
            this.change = function (item, newDataObject, editResult) {
                /// <summary>
                ///     Changes the data object of an item.
                /// </summary>
                /// <param name=""item"" type=""Object"" domElement=""true"">
                ///     The item to change.
                /// </param>
                /// <param name=""newDataObject"" type=""Object"">
                ///     The new data object of the item.
                /// </param>
                /// <param name=""editResult"" optional=""true"" type=""Function"">
                ///     Function to call when the result of the edit is known.  Function's signature must match that of
                ///     editResultFunction.
                /// </param>

                newDataObject = validateDataObject(newDataObject);

                var slot = slotFromItem(item);

                if (slot.kind !== ""item"") {
                    editResult(thisWinUI.EditResult.itemNotReady);
                } else {
                    var dataObjectOld;

                    queueEdit(
                        // applyEdit
                        function (editResult) {
                            dataSource.change(slot.key, newDataObject, editResult, slot.index);
                        },

                        // keyUpdate, isDeletion, editResult
                        null, false, editResult,

                        // updateSlots
                        function () {
                            dataObjectOld = slot.dataObject;
                            slot.dataObject = newDataObject;
                            reinstantiateItem(slot, true);
                        },

                        // undo
                        function () {
                            slot.dataObject = dataObjectOld;
                        }
                    );
                }
            };
        }

        function moveItem(slot, slotMoveBefore, append, editResult, applyEdit) {
            if (slot.kind !== ""item"") {
                editResult(thisWinUI.EditResult.itemNotReady);
            } else {
                var slotNext,
                    firstInSequence,
                    lastInSequence;

                queueEdit(
                    // applyEdit
                    applyEdit,

                    // keyUpdate, isDeletion, editResult
                    null, false, editResult,

                    // updateSlots
                    function () {
                        slotNext = slot.next;
                        firstInSequence = slot.firstInSequence;
                        lastInSequence = slot.lastInSequence;

                        updateNewIndices(slot, -1);
                        moveSlot(slot, slotMoveBefore, append, !append);
                        updateNewIndices(slot, 1);
                    },

                    // undo
                    function () {
                        updateNewIndices(slot, -1);
                        moveSlot(slot, slotNext, !firstInSequence, !lastInSequence);
                        updateNewIndices(slot, 1);
                    }
                );
            }
        }

        if (dataSource.moveToStart) {
            this.moveToStart = function (item, editResult) {
                /// <summary>
                ///     Moves an item to the start of the list.
                /// </summary>
                /// <param name=""item"" type=""Object"" domElement=""true"">
                ///     The item to move.
                /// </param>
                /// <param name=""editResult"" optional=""true"" type=""Function"">
                ///     Function to call when the result of the edit is known.  Function's signature must match that of
                ///     editResultFunction.
                /// </param>

                var slot = slotFromItem(item);

                moveItem(
                    slot, slotsStart.next, true, editResult,

                    // applyEdit
                    function (editResult) {
                        dataSource.moveToStart(slot.key, editResult, slot.index);
                    }
                );
            };
        }

        if (dataSource.moveBefore) {
            this.moveBefore = function (item, nextItem, editResult) {
                /// <summary>
                ///     Moves an item before a given item.
                /// </summary>
                /// <param name=""item"" type=""Object"" domElement=""true"">
                ///     The item to move.
                /// </param>
                /// <param name=""nextItem"" type=""Object"" domElement=""true"">
                ///     The item immediately after the insertion point.
                /// </param>
                /// <param name=""editResult"" optional=""true"" type=""Function"">
                ///     Function to call when the result of the edit is known.  Function's signature must match that of
                ///     editResultFunction.
                /// </param>

                var slot = slotFromItem(item),
                    slotNext = slotFromItem(nextItem);

                if (slotNext.kind !== ""item"") {
                    editResult(thisWinUI.EditResult.itemNotReady);
                } else {
                    moveItem(
                        slot, slotNext, false, editResult,

                        // applyEdit
                        function (editResult) {
                            dataSource.moveBefore(slot.key, slotNext.key, editResult, slot.index, slotNext.index);
                        }
                    );
                }
            };
        }

        if (dataSource.moveAfter) {
            this.moveAfter = function (item, previousItem, editResult) {
                /// <summary>
                ///     Moves an item after a given item.
                /// </summary>
                /// <param name=""item"" type=""Object"" domElement=""true"">
                ///     The item to move.
                /// </param>
                /// <param name=""previousItem"" type=""Object"" domElement=""true"">
                ///     The item immediately before the insertion point.
                /// </param>
                /// <param name=""editResult"" optional=""true"" type=""Function"">
                ///     Function to call when the result of the edit is known.  Function's signature must match that of
                ///     editResultFunction.
                /// </param>

                var slot = slotFromItem(item),
                    slotPrev = slotFromItem(previousItem);

                if (slotPrev.kind !== ""item"") {
                    editResult(thisWinUI.EditResult.itemNotReady);
                } else {
                    moveItem(
                        slot, slotPrev.next, true, editResult,

                        // applyEdit
                        function (editResult) {
                            dataSource.moveAfter(slot.key, slotPrev.key, editResult, slot.index, slotPrev.index);
                        }
                    );
                }
            };
        }

        if (dataSource.moveToEnd) {
            this.moveToEnd = function (item, editResult) {
                /// <summary>
                ///     Moves an item to the end of the list.
                /// </summary>
                /// <param name=""item"" type=""Object"" domElement=""true"">
                ///     The item to move.
                /// </param>
                /// <param name=""editResult"" optional=""true"" type=""Function"">
                ///     Function to call when the result of the edit is known.  Function's signature must match that of
                ///     editResultFunction.
                /// </param>

                var slot = slotFromItem(item);

                moveItem(
                    slot, slotsEnd, false, editResult,

                    // applyEdit
                    function (editResult) {
                        dataSource.moveToEnd(slot.key, editResult, slot.index);
                    }
                );
            };
        }

        if (dataSource.remove) {
            this.remove = function (item, editResult) {
                /// <summary>
                ///     Removes an item.
                /// </summary>
                /// <param name=""item"" type=""Object"" domElement=""true"">
                ///     The item to remove.
                /// </param>
                /// <param name=""editResult"" optional=""true"" type=""Function"">
                ///     Function to call when the result of the edit is known.  Function's signature must match that of
                ///     editResultFunction.
                /// </param>

                var slot = slotFromItem(item);

                if (slot.kind !== ""item"") {
                    editResult(thisWinUI.EditResult.itemNotReady);
                } else {
                    var slotNext,
                        firstInSequence,
                        lastInSequence;

                    queueEdit(
                        // applyEdit
                        function (editResult) {
                            dataSource.remove(slot.key, editResult, slot.index);
                        },

                        // keyUpdate, isDeletion, editResult
                        null, true, editResult,

                        // updateSlots
                        function () {
                            slotNext = slot.next;
                            firstInSequence = slot.firstInSequence;
                            lastInSequence = slot.lastInSequence;

                            updateNewIndices(slot, -1);
                            deleteSlot(slot, false);
                        },

                        // undo
                        function () {
                            reinsertSlot(slot, slotNext, !firstInSequence, !lastInSequence);
                            updateNewIndices(slot, 1);
                            instantiateNewItem(slot);
                        }
                    );
                }
            };
        }

        if (dataSource.removeRange) {
            this.removeRange = function (firstItem, lastItem, editResult) {
                /// <summary>
                ///     Removes a range of items.
                /// </summary>
                /// <param name=""firstItem"" type=""Object"" domElement=""true"">
                ///     The first item to remove.
                /// </param>
                /// <param name=""lastItem"" type=""Object"" domElement=""true"">
                ///     The last item to remove.  May equal firstItem.
                /// </param>
                /// <param name=""editResult"" optional=""true"" type=""Function"">
                ///     Function to call when the result of the edit is known.  Function's signature must match that of
                ///     editResultFunction.
                /// </param>

                var slotFirst = slotFromItem(firstItem),
                    slotLast = slotFromItem(lastItem);

                // TODO: Validate that slotLast is after slotFirst in the list

                if (slotFirst.kind !== ""item"" || slotLast.kind !== ""item"") {
                    editResult(thisWinUI.EditResult.itemNotReady);
                } else {
                    var slotNext,
                        firstInSequence,
                        lastInSequenceInstantiated = [],    // Array of Booleans for the instantiated slots
                        slotsInstantiated = [],
                        countInstantiated,
                        countRange;

                    queueEdit(
                        // applyEdit
                        function (editResult) {
                            dataSource.removeRange(slotFirst.key, slotLast.key, editResult, slotFirst.index, slotLast.index);
                        },

                        // keyUpdate, isDeletion, editResult
                        null, true, editResult,

                        // updateSlots
                        function () {
                            countInstantiated = 0;
                            countRange = 0;
                            firstInSequence = slotFirst.firstInSequence;

                            var indexDelta = slotLast.index - slotFirst.index;

                            // Remove all items but the last one.  Retain instantiated items in case an undo is
                            // necessary, but discard all other slots.
                            while (true) {
                                slotNext = slotFirst.next;

                                countRange++;

                                if (slotFirst.kind === ""item"") {
                                    slotsInstantiated[countInstantiated] = slotFirst;

                                    if (slotFirst.lastInSequence || (slotFirst !== slotLast && slotNext.kind !== ""item"")) {
                                        lastInSequenceInstantiated[countInstantiated] = true;
                                    }

                                    countInstantiated++;
                                }
    
                                if (slotFirst === slotLast) {
                                    // Don't remove the last slot just yet
                                    break;
                                }

                                deleteSlot(slotFirst, false);

                                slotFirst = slotNext;
                            }
                            do { if (countInstantiated > 0) { } else { assertionFailed(""countInstantiated > 0"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 4762); } } while (false);
                            do { if (slotNext === slotLast.next) { } else { assertionFailed(""slotNext === slotLast.next"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 4763); } } while (false);

                            // So far, countInstantiated is just a guess, and a poor one if there was a discontinuity
                            // in the list.  If indices are available, use them instead.
                            if (!isNaN(indexDelta)) {
                                countRange = indexDelta;
                            }

                            // Adjust the indices, then remove the last slot
                            updateNewIndices(slotLast, -countRange);
                            deleteSlot(slotLast, false);
                        },

                        // undo
                        function () {
                            for (var i = countInstantiated - 1; i >= 0; i--) {
                                var slot = slotsInstantiated[i];
                                reinsertSlot(slot, slotNext, (i === 0 ? !firstInSequence : false), !lastInSequenceInstantiated[i]);
                                instantiateNewItem(slot);
                                slotNext = slot;
                            }

                            updateNewIndices(slotLast, countRange);
                        }
                    );
                }
            };
        }

        this.endEdits = function () {
            /// <summary>
            ///     Notifies the Items Manager that a sequence of edits has ended.  The Items Manager will call
            ///     beginNotifications and endNotifications once each for a sequence of edits.
            /// </summary>

            editsInProgress = false;
            completeEdits();
        };

    } // ListEditor

    function DataNotificationHandler() {
        /// <summary>
        ///     Methods on data notification handler object passed to DataSource.setDataNotificationHandler.
        /// </summary>

        this.invalidateAll = function () {
            /// <summary>
            ///     Notifies the Items Manager that some data has changed, without specifying what.  Since it may be
            ///     impractical for some data sources to call this method for any or all changes, doing so is optional.
            ///     However, if it is not called by a given data source, the application should periodically call refresh
            ///     to update the associated Items Manager.
            /// </summary>

            beginRefresh();
        };

        this.beginNotifications = function () {
            /// <summary>
            ///     May be called before a sequence of other notification calls, to minimize the number of countChanged
            ///     and indexChanged notifications sent to the client of the Items Manager.  Must be paired with a call
            ///     to endNotifications, and pairs may not be nested.
            /// </summary>

            dataNotificationsInProgress = true;
        };

        function completeNotification() {
            if (!dataNotificationsInProgress) {
                updateIndices();
                finishNotifications();
            }
        }

        this.inserted = function (key, dataObject, previousKey, nextKey, index) {
            /// <summary>
            ///     Called when an item has been inserted.
            /// </summary>
            /// <param name=""key"" type=""String"">
            ///     The key of the inserted item.
            /// </param>
            /// <param name=""dataObject"" type=""Object"">
            ///     The data object of the inserted item.
            /// </param>
            /// <param name=""previousKey"" mayBeNull=""true"" type=""String"">
            ///     The key of the item before the insertion point, null if the item was inserted at the start of the list.
            /// </param>
            /// <param name=""nextKey"" mayBeNull=""true"" type=""String"">
            ///     The key of the item after the insertion point, null if the item was inserted at the end of the list.
            /// </param>
            /// <param name=""index"" optional=""true"" type=""Number"" integer=""true"">
            ///     The index of the inserted item.
            /// </param>

            if (editsQueued) {
                // We can't change the slots out from under any queued edits
                beginRefresh();
            } else {
                var slotPrev = keyMap[previousKey],
                    slotNext = keyMap[nextKey];

                if (keyMap[key] || (slotPrev && slotNext && (slotPrev.next !== slotNext || slotPrev.lastInSequence || slotNext.firstInSequence))) {
                    // Something has changed, start a refresh
                    beginRefresh();
                } else if (slotPrev || slotNext) {
                    insertNewSlot(key, dataObject, (slotNext ? slotNext : slotPrev.next), !!slotPrev, !!slotNext, true);

                    completeNotification();
                } else if (index !== undefined) {
                    updateNewIndicesFromIndex(index, 1);

                    completeNotification();
                }
            }
        };

        this.changed = function (key, newDataObject) {
            /// <summary>
            ///     Called when an item's data object has been changed.
            /// </summary>
            /// <param name=""key"" type=""String"">
            ///     The key of the item that has changed.
            /// </param>
            /// <param name=""newDataObject"" type=""Object"">
            ///     The item's new data object.
            /// </param>

            if (editsQueued) {
                // We can't change the slots out from under any queued edits
                beginRefresh();
            } else {
                var slot = keyMap[key];

                if (slot) {
                    slot.dataObject = newDataObject;
                    reinstantiateItem(slot, true);

                    completeNotification();
                }
            }
        };

        this.moved = function (key, dataObject, previousKey, nextKey, oldIndex, newIndex) {
            /// <summary>
            ///     Called when an item has been moved to a new position.
            /// </summary>
            /// <param name=""key"" type=""String"">
            ///     The key of the item that has moved.
            /// </param>
            /// <param name=""dataObject"" type=""Object"">
            ///     The dataObject of the item that has moved.
            /// </param>
            /// <param name=""previousKey"" mayBeNull=""true"" type=""String"">
            ///     The key of the item before the insertion point, null if the item was moved to the start of the list.
            /// </param>
            /// <param name=""nextKey"" mayBeNull=""true"" type=""String"">
            ///     The key of the item after the insertion point, null if the item was moved to the end of the list.
            /// </param>
            /// <param name=""oldIndex"" optional=""true"" type=""Number"" integer=""true"">
            ///     The index of the item before it was moved.
            /// </param>
            /// <param name=""newIndex"" optional=""true"" type=""Number"" integer=""true"">
            ///     The index of the item after it has moved.
            /// </param>

            if (editsQueued) {
                // We can't change the slots out from under any queued edits
                beginRefresh();
            } else {
                var slot = keyMap[key],
                    slotPrev = keyMap[previousKey],
                    slotNext = keyMap[nextKey];

                if (slot) {
                    if (slotPrev && slotNext && (slotPrev.next !== slotNext || slotPrev.lastInSequence || slotNext.firstInSequence)) {
                        // Something has changed, start a refresh
                        beginRefresh();
                    } else if (!slotPrev && !slotNext) {
                        // If we can't tell where the item moved to, treat this like a removal
                        updateNewIndices(slot, -1);
                        deleteSlot(slot, false);

                        if (oldIndex !== undefined) {
                            // TODO: VALIDATE(newIndex !== undefined);

                            if (oldIndex < newIndex) {
                                newIndex--;
                            }

                            updateNewIndicesFromIndex(newIndex, 1);
                        }

                        completeNotification();

                        this.removed(key);
                    } else {
                        updateNewIndices(slot, -1);
                        moveSlot(slot, (slotNext ? slotNext : slotPrev.next), !!slotPrev, !!slotNext);
                        updateNewIndices(slot, 1);

                        completeNotification();
                    }
                } else if (slotPrev || slotNext) {
                    // If previousKey or nextKey is known, but key isn't, treat this like an insertion.

                    if (oldIndex !== undefined) {
                        // TODO: VALIDATE(newIndex !== undefined);

                        updateNewIndicesFromIndex(oldIndex, -1);

                        if (oldIndex < newIndex) {
                            newIndex--;
                        }
                    }

                    this.inserted(key, dataObject, previousKey, nextKey, newIndex);
                } else if (oldIndex !== undefined) {
                    // TODO: VALIDATE(newIndex !== undefined);

                    updateNewIndicesFromIndex(oldIndex, -1);
                    updateNewIndicesFromIndex(newIndex, 1);

                    completeNotification();
                }
            }
        };

        this.removed = function (key, index) {
            /// <summary>
            ///     Called when an item has been removed.
            /// </summary>
            /// <param name=""key"" type=""String"">
            ///     The key of the item that has been removed.
            /// </param>
            /// <param name=""index"" optional=""true"" type=""Number"" integer=""true"">
            ///     The index of the item that has been removed.
            /// </param>

            if (editsQueued) {
                // We can't change the slots out from under any queued edits
                beginRefresh();
            } else {
                var slot = keyMap[key];

                if (slot) {
                    updateNewIndices(slot, -1);
                    deleteSlot(slot, false);

                    completeNotification();
                } else if (index !== undefined) {
                    updateNewIndicesFromIndex(index, -1);
                    completeNotification();
                }
            }
        };

        this.endNotifications = function () {
            /// <summary>
            ///     Concludes a sequence of notifications.
            /// </summary>

            dataNotificationsInProgress = false;
            completeNotification();
        };

    } // DataNotificationHandler

    // Construction

    // Process creation parameters
    if (!dataSource) {
        throw new Error(dataSourceIsInvalid);
    }
    if (Array.isArray(dataSource)) {
        dataSource = thisWinUI.createObjectDataSource(dataSource);
    }
    if (!itemRenderer) {
        throw new Error(itemRendererIsInvalid);
    }
    placeholderRenderer = defaultRenderer;
    itemNotificationHandler = {};   // Dummy object so it's always defined
    if (options) {
        if (options.placeholderRenderer) {
            placeholderRenderer = options.placeholderRenderer;
        }
        if (options.itemNotificationHandler) {
            itemNotificationHandler = options.itemNotificationHandler;
        }
        if (options.ownerElement) {
            ownerElement = options.ownerElement;
        }
    }

    // Request from the data source to avoid serialization to JSON
    compareByIdentity = !!dataSource.compareByIdentity;

    // Cached listEditor initially undefined

    // Cached dataNotificationHandler initially undefined
    if (dataSource.setDataNotificationHandler) {
        dataNotificationHandler = new DataNotificationHandler();

        // DEPRECATED: For backwards-compatibility, make dataNotificationHandler a function object
        if (true) {
            var invalidateAll = function () {
                beginRefresh();
            };

            for (var method in dataNotificationHandler) {
                if (dataNotificationHandler.hasOwnProperty(method)) {
                    invalidateAll[method] = dataNotificationHandler[method];
                }
            }

            dataNotificationHandler = invalidateAll;
        }

        dataSource.setDataNotificationHandler(dataNotificationHandler);
    }

    // Status of the Items Manager
    status = thisWinUI.ItemsManagerStatus.ready;

    // Track whether endNotifications needs to be sent
    notificationsSent = false;

    // Track whether finishNotifications has been posted already
    finishNotificationsPosted = false;

    // Track whether finishNotifications should be called after each edit
    editsInProgress = false;

    // Track whether there are currently edits queued
    editsQueued = false;

    // applyNextEdit function is undefined until a ListEditor is created

    // If an edit has returned noResponse, the edit queue will be reapplied when the next refresh is requested
    waitForRefresh = false;

    // Change to count while multiple edits are taking place
    countDelta = 0;

    // True while the indices are temporarily in a bad state due to multiple edits
    indexUpdateDeferred = false;

    // Next temporary key to use
    nextTempKey = 0;

    // ID of the refresh in progress, incremented each time a new refresh is started
    currentRefreshID = 0;

    // ID of a given fetch, incremented each time a new fetch is initiated
    nextFetchID = 0;

    // Set of fetches for which results have not yet arrived
    fetchesInProgress = {};

    // The ItemsManager tracks the count returned explicitly or implicitly by the data source
    knownCount = thisWinUI.CountResult.unknown;

    // Sentinel objects for list of instantiated items
    // Give the start sentinel an index so we can always use predecessor + 1
    slotsStart = {
        firstInSequence: true,
        lastInSequence: true,
        index: -1
    };
    slotsEnd = {
        firstInSequence: true,
        lastInSequence: true
    };
    slotsStart.next = slotsEnd;
    slotsEnd.prev = slotsStart;


    slotsStart.debugInfo = ""*** slotsStart ***"";
    slotsEnd.debugInfo = ""*** slotsEnd ***"";


    // Map of keys to instantiated items
    keyMap = {};

    // Map of indices to instantiated items
    indexMap = {};
    indexMap[-1] = slotsStart;

    // Map of (the uniqueIDs of) elements to instantiated items
    elementMap = {};

    // Count of slots that have been released but not deleted
    releasedSlots = 0;

    // Maximum number of released slots to retain
    releasedSlotsMax = 200;

    // lastSlotReleased is initially undefined

    // At most one call to reduce the number of refresh slots should be posted at any given time
    releasedSlotReductionInProgress = false;

    // Queues for resource loading
    queues = [
        createQueue(),
        createQueue(),
        createQueue()
    ];

    // Multiple refresh requests are coalesced
    refreshRequested = false;

    // Requests do not cause fetches while a refresh is in progress
    refreshInProgress = false;

    // Dummy parent node used to parse HTML returned by renderers
    dummyParent = document.createElement(""div"");

    if (!document.body.uniqueID) {
        (function () {
            var id = 1;
            HTMLElement.prototype.__defineGetter__(
                ""uniqueID"",
                function () {
                    return this[""uniqueID ""] || (this[""uniqueID ""] = ""zz__id"" + id++);
                }
            );
        })();
    }

    // Public methods

    this.count = function (countAvailable) {
        /// <summary>
        ///     Fetches the total number of items.
        /// </summary>
        /// <param name=""countAvailable"" type=""Function"">
        ///     Callback for returning the count.  Function's signature should match that of
        ///     countAvailableCallback.
        /// </param>

        checkCallback(countAvailable, ""countAvailable"", false);

        // If the data source adaptor doesn't support the count method, return the Items Manager's reckoning
        // of the count.
        if (!dataSource.count) {
            postCall(function () {
                countAvailable(knownCount);
            });
        } else {
            // Always do a fetch, even if there is a cached result
            var reentrant = true;
            dataSource.count(function (count) {
                if (!utilities.isNonNegativeInteger(count) && count !== thisWinUI.CountResult.unknown && count !== thisWinUI.CountResult.failure) {
                    throw new Error(invalidRequestedCountReturned);
                }

                if (count === thisWinUI.CountResult.failure) {
                    // Report the failure, but still report last known count
                    setStatus(thisWinUI.ItemsManagerStatus.failure);
                    count = knownCount;
                } else {
                    if (count !== knownCount) {
                        changeCount(count);
                        finishNotifications();
                    }

                    if (count === 0) {
                        if (slotsStart.next !== slotsEnd) {
                            // A contradiction has been found
                            beginRefresh();
                        } else if (slotsStart.lastInSequence) {
                            mergeSequences(slotsStart);
                        }
                    }
                }

                if (reentrant) {
                    postCall(function () {
                        countAvailable(count);
                    });
                } else {
                    countAvailable(count);
                }
            });
            reentrant = false;
        }
    };

    this.firstItem = function () {
        /// <summary>
        ///     Returns an element representing the first item.  This may be a placeholder, a rendering of a
        ///     successfully fetched item, or an indicator that the attempt to fetch the item failed.
        /// </summary>
        /// <returns type=""Object"" mayBeNull=""true"" domElement=""true"" />

        return requestSlotAfter(slotsStart, function (slotNew) {
            fetchItemsFromStart(slotNew, 2);
        });
    };

    this.previousItem = function (item) {
        /// <summary>
        ///     Returns an element representing the item immediately before a given item.  This may be a placeholder,
        ///     a rendering of a successfully fetched item, or an indicator that the attempt to fetch the item failed.
        /// </summary>
        /// <param name=""item"" type=""Object"" domElement=""true"">
        ///     The element representing the item immediately after the requested item.
        /// </param>
        /// <returns type=""Object"" mayBeNull=""true"" domElement=""true"" />

        return requestSlotBefore(slotFromItem(item), function (slotNew) {
            var slotNext = slotNew.next;
            do { if (slotNext.element) { } else { assertionFailed(""slotNext.element"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 5275); } } while (false);
            if (slotNext.key !== undefined && slotNext.kind !== ""placeholder"") {
                do { if (slotNext.kind === ""item"") { } else { assertionFailed(""slotNext.kind === \""item\"""", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 5277); } } while (false);
                fetchItemsFromIdentity(slotNext, 2, 0);
            }
        });
    };

    this.nextItem = function (item) {
        /// <summary>
        ///     Returns an element representing the item immediately after a given item.  This may be a placeholder,
        ///     a rendering of a successfully fetched item, or an indicator that the attempt to fetch the item failed.
        /// </summary>
        /// <param name=""item"" type=""Object"" domElement=""true"">
        ///     The element representing the item immediately before the requested item.
        /// </param>
        /// <returns type=""Object"" mayBeNull=""true"" domElement=""true"" />

        return requestSlotAfter(slotFromItem(item), function (slotNew) {
            var slotPrev = slotNew.prev;
            do { if (slotPrev.element) { } else { assertionFailed(""slotPrev.element"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 5295); } } while (false);
            if (slotPrev.key !== undefined && slotPrev.kind !== ""placeholder"") {
                do { if (slotPrev.kind === ""item"") { } else { assertionFailed(""slotPrev.kind === \""item\"""", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 5297); } } while (false);
                fetchItemsFromIdentity(slotPrev, 0, 2);
            }
        });
    };

    // Only enable the lastItem method if the data source implements the itemsFromEnd method
    if (dataSource.itemsFromEnd) {
        this.lastItem = function () {
            /// <summary>
            ///     Returns an element representing the last item.  This may be a placeholder, a rendering of a
            ///     successfully fetched item, or an indicator that the attempt to fetch the item failed.
            /// </summary>
            /// <returns type=""Object"" mayBeNull=""true"" domElement=""true"" />

            return requestSlotBefore(slotsEnd, function (slotNew) {
                fetchItemsFromEnd(slotNew, 2);
            });
        };
    }

    this.itemFromKey = function (key) {
        /// <summary>
        ///     Returns an element representing the item with the given key.  This may be a placeholder or a rendering
        ///     of a successfully fetched item.
        /// </summary>
        /// <param name=""key"" type=""String"">
        ///     The key of the requested item.
        /// </param>
        /// <returns type=""Object"" mayBeNull=""true"" domElement=""true"" />

        if (typeof key !== ""string"") {
            throw new Error(keyIsInvalid);
        }

        var slot = keyMap[key];
        var element;

        if (slot === slotsEnd) {
            element = null;
        } else if (slot && slot.key === key) {
            element = slotRequested(slot);
        } else {
            var slotNext = lastInsertionPoint(slotsStart, slotsEnd);

            if (slotNext === undefined) {
                // The complete list is instantiated, and this key isn't a part of it; a refresh may be necessary
                return null;
            }

            // Create a new slot and start a request for it
            slot = createSlotSequence(slotNext, undefined, indexMap);
            setSlotKey(slot, key);
            slot.keyRequested = true;

            fetchItemsFromIdentity(slot, 1, 1);

            element = slotCreated(slot);
        }

        do { if ((element === null && slot.kind === ""mirage"") || (utilities.isDOMElement(element) && element.msDataItem && keyMap[element.msDataItem.key]) || elementMap[element.uniqueID]) { } else { assertionFailed(""(element === null && slot.kind === \""mirage\"") || (utilities.isDOMElement(element) && element.msDataItem && keyMap[element.msDataItem.key]) || elementMap[element.uniqueID]"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 5357); } } while (false);

        return element;
    };

    this.itemFromPrefix = function (prefix) {
        /// <summary>
        ///     Returns an element representing the first item with a prefix matching or after the given one, as
        ///     interpreted by the data source.  This may be a placeholder or a rendering of a successfully fetched
        ///     item.  This method may only be called when there are no instantiated items in the list.
        /// </summary>
        /// <param name=""prefix"" type=""String"">
        ///     The requested prefix, to be interpreted by the data source.
        /// </param>
        /// <returns type=""Object"" mayBeNull=""true"" domElement=""true"" />

        if (typeof prefix !== ""string"") {
            throw new Error(prefixIsInvalid);
        }

        var slot;

        // Verify that there are no instantiated items
        for (slot = slotsStart.next; slot !== slotsEnd; slot = slot.next) {
            if (slot.element) {
                throw new Error(listNotEmpty);
            }
        }

        // Delete any cached items
        for (slot = slotsStart.next; slot !== slotsEnd; ) {
            var slotNext = slot.next;
            do { if (!slot.element) { } else { assertionFailed(""!slot.element"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 5389); } } while (false);
            removeSlotPermanently(slot);
            slot = slotNext;
        }

        // Just in case the list has been observed to be empty, ""forget"" this
        splitSequences(slotsStart);

        // Create a new slot and start a request for it
        slot = createSlotSequence(slotsEnd, undefined, indexMap);

        fetchItemsFromPrefix(slot, prefix, 1, 1);

        var element = slotCreated(slot);

        do { if ((element === null && slot.kind === ""mirage"") || (utilities.isDOMElement(element) && element.msDataItem && keyMap[element.msDataItem.key]) || elementMap[element.uniqueID]) { } else { assertionFailed(""(element === null && slot.kind === \""mirage\"") || (utilities.isDOMElement(element) && element.msDataItem && keyMap[element.msDataItem.key]) || elementMap[element.uniqueID]"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 5404); } } while (false);

        return element;
    };

    this.itemAtIndex = function (index) {
        /// <summary>
        ///     Returns an element representing the item at the given index.  This may be a placeholder or a rendering
        ///     of a successfully fetched item.
        /// </summary>
        /// <param name=""index"" type=""Number"" integer=""true"">
        ///     The index of the requested item.
        /// </param>
        /// <returns type=""Object"" mayBeNull=""true"" domElement=""true"" />

        if (typeof index !== ""number"" || index < 0) {
            throw new Error(indexIsInvalid);
        }

        var slot = indexMap[index];
        var element;

        if (slot === slotsEnd) {
            element = null;
        } else if (slot && slot.index === index) {
            element = slotRequested(slot);
        } else {
            var slotNext = successorFromIndex(index, indexMap, slotsStart, slotsEnd);

            if (slotNext === undefined) {
                // The complete list is instantiated, and this index isn't a part of it; a refresh may be necessary
                return null;
            }

            // Create a new slot and start a request for it
            if (slotNext.prev.index === index - 1) {
                slot = addSlotAfter(slotNext.prev, indexMap);
            }
            else if (slotNext.index === index + 1) {
                slot = addSlotBefore(slotNext, indexMap);
            } else {
                slot = createSlotSequence(slotNext, index, indexMap);
            }

            if ((slot.firstInSequence || slot.prev.kind !== ""placeholder"") && (slot.lastInSequence || slot.next.kind !== ""placeholder"")) {
                fetchItemsFromIndex(slot, 1, 1);
            }

            element = slotCreated(slot);
        }

        do { if (slot.index !== undefined) { } else { assertionFailed(""slot.index !== undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 5455); } } while (false);
        if (slot.kind === ""placeholder"") {
            slot.indexRequested = true;
        }

        do { if ((element === null && slot.kind === ""mirage"") || (utilities.isDOMElement(element) && element.msDataItem && keyMap[element.msDataItem.key]) || elementMap[element.uniqueID]) { } else { assertionFailed(""(element === null && slot.kind === \""mirage\"") || (utilities.isDOMElement(element) && element.msDataItem && keyMap[element.msDataItem.key]) || elementMap[element.uniqueID]"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 5460); } } while (false);

        return element;
    };

    this.prioritize = function (first, last, priority) {
        /// <summary>
        ///     Directs the Items Manager to prioritize the loading of a given range of items, including their
        ///     resources.
        /// </summary>
        /// <param name=""first"" type=""Object"" domElement=""true"">
        ///     The element representing the first item in the range.
        /// </param>
        /// <param name=""last"" type=""Object"" domElement=""true"">
        ///     The element representing the last item in the range.
        /// </param>
        /// <param name=""priority"" optional=""true"" type=""Priority"">
        ///     The priority level at which to load the given range of items.  Legal values are Priority.high and
        ///     Priority.medium.  By default, all items load at low-priority.  Calling this method with a priority of
        ///     Priority.high resets all items outside the given range to low-priority.  Calling this method with a
        ///     priority of Priority.medium does not affect items outside the given range.  If the priority parameter
        ///     is undefined, Priority.high will be assumed.
        /// </param>

        var Priority = thisWinUI.Priority;

        if (priority !== undefined && priority !== Priority.high && priority !== Priority.medium) {
            throw new Error(priorityIsInvalid);
        }

        if (priority === undefined) {
            priority = Priority.high;
        }

        var slot,
            slotFirst = slotFromItem(first),
            slotLast = slotFromItem(last);

        if (priority === Priority.high) {
            var inRange = false;
            for (slot = slotsStart; slot !== slotsEnd; slot = slot.next) {
                if (slot === slotFirst) {
                    inRange = true;
                }

                if (inRange && slot.kind !== ""item"") {
                    slot.priority = Priority.high;
                } else {
                    delete slot.priority;
                }

                if (removeSlotFromQueue(slot)) {
                    if (slot.priority === Priority.high) {
                        queueSlot(slot, queues[Priority.high]);
                    } else {
                        queueSlot(slot, queues[Priority.low]);
                    }
                }

                if (slot === slotLast) {
                    inRange = false;
                }
            }

            // Add requests for high- and low-priority resources to the global queues.  (No harm done if there aren't
            // any.)
            pushResourceRequest(resourceRequest, Priority.high);
            pushResourceRequest(resourceRequest, Priority.low);
        } else {
            do { if (priority === Priority.medium) { } else { assertionFailed(""priority === Priority.medium"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 5529); } } while (false);

            // Walk backwards through the given range, pushing each item in turn to the head of the medium-priority queue
            var slotBeforeRange = slotFirst.prev;
            for (slot = slotLast; slot !== slotBeforeRange; slot = slot.prev) {
                if (slot.kind !== ""item"") {
                    slot.priority = Priority.medium;
                }

                if (removeSlotFromQueue(slot)) {
                    pushSlot(slot, queues[Priority.medium]);
                }
            }

            // Add requests for medium-priority resources to the global queue
            pushResourceRequest(resourceRequest, Priority.medium);
        }
    };

    this.isPlaceholder = function (item) {
        /// <summary>
        ///     Returns a value indicating whether the element representing a given item is a placeholder, a
        ///     rendering of a successfully fetched item, or an indicator that the attempt to fetch the item
        ///     failed.
        /// </summary>
        /// <param name=""item"" type=""Object"" domElement=""true"">
        ///     The element representing the item.
        /// </param>
        /// <returns type=""Boolean"">
        /// True if the item is a placeholder.
        /// </returns>

        var slot = slotFromItem(item);
        do { if (slot.kind === ""placeholder"" || slot.kind === ""item"") { } else { assertionFailed(""slot.kind === \""placeholder\"" || slot.kind === \""item\"""", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 5562); } } while (false);
        return slot.kind === ""placeholder"";
    };

    this.itemIndex = function (item) {
        /// <summary>
        ///     Returns the index of the given item, if available.
        /// </summary>
        /// <param name=""item"" type=""Object"" domElement=""true"">
        ///     The element representing the item.
        /// </param>
        /// <returns type=""Number"" integer=""true"" />

        return slotFromItem(item).index;
    };

    this.releaseItem = function (item) {
        /// <summary>
        ///     Notifies the Items Manager that the element representing a given item no longer needs to be
        ///     retained.
        /// </summary>
        /// <param name=""item"" type=""Object"" domElement=""true"">
        ///     The element representing the item.
        /// </param>

        var slot = slotFromItem(item, true);

        if (slot) {
            if (itemNotificationHandler.saveState && slot.kind === ""item"") {
                itemNotificationHandler.saveState(slot.key, item);
            }

            // Revert the slot to the state of an unrequested item
            uninstantiateSlot(slot);
            delete slot.priority;
            delete slot.kind;
            delete slot.indexRequested;
            delete slot.keyRequested;
            delete slot.indexOld;
            delete slot.dataObjectDifferent;

            // Ensure that an outstanding fetch doesn't ""re-request"" the item
            slot.released = true;

            // If a refresh is in progress, retain all slots, just in case the user re-requests some of them
            // before the refresh completes
            if (!refreshInProgress) {
                // If releasedSlotsMax is 0, delete the released slot immediately
                if (releasedSlotsMax === 0) {
                    deleteUnrequestedSlot(slot);
                } else {
                    // Track which slot was released most recently
                    releasedSlots++;
                    lastSlotReleased = slot;

                    // See if the number of released slots has exceeded the maximum allowed
                    if (!releasedSlotReductionInProgress && releasedSlots > releasedSlotsMax) {
                        releasedSlotReductionInProgress = true;

                        postCall(function () {
                            reduceReleasedSlotCount();
                            releasedSlotReductionInProgress = false;
                        });
                    }
                }
            }
        }
    };

    this.refresh = function () {
        /// <summary>
        ///     Directs the Items Manager to communicate with the data source to determine if any aspects of the
        ///     instantiated items have changed.
        /// </summary>

        beginRefresh();
    };

    var listEditorMethods = [
        ""insertAtStart"",
        ""insertBefore"",
        ""insertAfter"",
        ""insertAtEnd"",
        ""change"",
        ""moveToStart"",
        ""moveBefore"",
        ""moveAfter"",
        ""moveToEnd"",
        ""remove""
    ];

    // Only define the listEditor method if at least one of the editor methods is present on the data source
    var editorMethodDefined = false;
    listEditorMethods.forEach(function (listEditorMethod) {
        if (dataSource[listEditorMethod]) {
            editorMethodDefined = true;
        }
    });

    if (editorMethodDefined) {
        this.listEditor = function () {
            /// <summary>
            ///     Returns a list editor to allow manipulation of the items in the data
            ///     source.
            /// </summary>
            /// <returns type=""ListEditor"" />

            // Don't create the listEditor object until it's first needed
            if (!listEditor) {
                listEditor = new ListEditor();
            }

            return listEditor;
        };
    }

} // ItemsManager

// Object Data Source

thisWinUI.createObjectDataSource = function (objects, options) {

    // Private members

    var array = [],
        keyToIndexMap = {},
        nextAvailableKey = 0,
        inputJSON,
        storeReferences = false;

    function validateDataObject(dataObject) {
        // Check if the identity of the objects must be preserved, or if copies can be stored
        return storeReferences ? dataObject : utilities.validateDataObject(dataObject);
    }

    function item(key, dataObject) {
        return { key: key.toString(), dataObject: dataObject };
    }

    function updateKeyToIndexMap(first) {
        // Update the key map entries for all indices that changed
        for (var i = first; i < array.length; ++i) {
            keyToIndexMap[array[i].key] = i;
        }
    }

    function insert(index, dataObject, editResult) {
        array.splice(index, 0, item(nextAvailableKey, validateDataObject(dataObject)));
        updateKeyToIndexMap(index);

        editResult(Win.UI.EditResult.success, nextAvailableKey.toString());

        ++nextAvailableKey;
    }

    function move(indexTo, key, editResult) {
        var indexFrom = keyToIndexMap[key],
            removed = array.splice(indexFrom, 1);

        if (indexFrom < indexTo) {
            --indexTo;
        }

        array.splice(indexTo, 0, removed[0]);
        updateKeyToIndexMap(Math.min(indexFrom, indexTo));

        editResult(Win.UI.EditResult.success);
    }

    // Construction

    if (options) {
        if (options.storeReferences || options.compareByIdentity) {  // DEPRECATED: options.compareByIdentity will be removed soon
            storeReferences = true;
        }
    }

    // Assume a string is JSON text
    inputJSON = (typeof objects === ""string"");
    if (inputJSON) {
        objects = JSON.parse(objects);
    }

    // Ensure the objects are in an array
    if (!Array.isArray(objects)) {
        objects = [objects];
    }

    // Build the item array and key map
    for (var i = 0, len = objects.length; i < len; ++i) {
        var dataObject = objects[i];

        // No need to validate items that were passed in as JSON
        if (!inputJSON) {
            dataObject = validateDataObject(dataObject);
        }
        array[i] = item(nextAvailableKey, dataObject);
        keyToIndexMap[nextAvailableKey] = i;

        ++nextAvailableKey;
    }

    // Public methods

    return {
        // setDataNotificationHandler: not implemented

        // The Items Manager should always compare these items by identity; in rare cases, it will do some unnecesssary
        // rerendering, but at least fetching will not stringify dataObjects we already know to be valid.
        compareByIdentity: true,

        itemsFromEnd: function (count, itemsAvailable) {
            if (array.length === 0) {
                itemsAvailable(Win.UI.FetchResult.doesNotExist);
            } else {
                this.itemsFromIndex(array.length - 1, count - 1, 0, itemsAvailable);
            }
        },

        itemsFromKey: function (key, countBefore, countAfter, itemsAvailable) {
            var index = keyToIndexMap[key];

            if (index === undefined) {
                itemsAvailable(Win.UI.FetchResult.doesNotExist);
            } else {
                this.itemsFromIndex(index, countBefore, countAfter, itemsAvailable);
            }
        },

        itemsFromIndex: function (index, countBefore, countAfter, itemsAvailable) {
            if (index >= array.length) {
                itemsAvailable(Win.UI.FetchResult.doesNotExist);
            } else {
                var first = Math.max(0, index - countBefore),
                    last = index + countAfter + 1,
                    results = array.slice(first, last),
                    offset = index - first;

                itemsAvailable(results, offset, array.length, index);
            }
        },

        // itemsFromPrefix: not implemented

        count: function (countAvailable) {
            countAvailable(array.length);
        },

        // Editing methods

        insertAtStart: function (key, dataObject, editResult) {
            // key parameter is ignored, as keys are generated
            insert(0, dataObject, editResult);
        },

        insertBefore: function (key, dataObject, nextKey, editResult) {
            // key parameter is ignored, as keys are generated
            insert(keyToIndexMap[nextKey], dataObject, editResult);
        },

        insertAfter: function (key, dataObject, previousKey, editResult) {
            // key parameter is ignored, as keys are generated
            insert(keyToIndexMap[previousKey] + 1, dataObject, editResult);
        },

        insertAtEnd: function (key, dataObject, editResult) {
            // key parameter is ignored, as keys are generated
            insert(array.length, dataObject, editResult);
        },

        change: function (key, newDataObject, editResult) {
            array[keyToIndexMap[key]].dataObject = validateDataObject(newDataObject);

            editResult(Win.UI.EditResult.success);
        },

        moveToStart: function (key, editResult) {
            move(0, key, editResult);
        },

        moveBefore: function (key, nextKey, editResult) {
            move(keyToIndexMap[nextKey], key, editResult);
        },

        moveAfter: function (key, previousKey, editResult) {
            move(keyToIndexMap[previousKey] + 1, key, editResult);
        },

        moveToEnd: function (key, editResult) {
            move(array.length, key, editResult);
        },

        remove: function (key, editResult) {
            var index = keyToIndexMap[key];

            // TODO:  Validate key here (and all other entry points)

            delete keyToIndexMap[key];
            array.splice(index, 1);
            updateKeyToIndexMap(index);

            editResult(Win.UI.EditResult.success);
        },

        removeRange: function (keyFirst, keyLast, editResult) {
            var indexFirst = keyToIndexMap[keyFirst],
                indexLast = keyToIndexMap[keyLast];

            for (var i = indexFirst; i <= indexLast; i++) {
                delete keyToIndexMap[array[i].key];
            }
            array.splice(indexFirst, indexLast - indexFirst + 1);
            updateKeyToIndexMap(indexFirst);

            editResult(Win.UI.EditResult.success);
        }
    };
};

// Iterator Data Source

thisWinUI.IteratorDataSource = function (iterator) {
    /// <summary>
    /// Creates an IIterator-based data source.
    /// </summary>
    /// <param name=""iterator"" type=""Object"">
    /// An object that implements the IIterator interface.
    /// </param>
    /// <remarks>
    /// iterator must implement the following WinRT methods:
    /// 
    ///     HasCurrent([out, retval] bool);
    ///     Current([out, retval] T*);
    ///     MoveNext([out, retval] bool);
    ///     
    /// </remarks>

    // Invariant: if the iterator field is assigned, it has a 'current' item that is pending insertion.
    this._iterator = (iterator && iterator.HasCurrent) ? iterator : null;
    this._items = [];
    this._handler = {};
};

// Consumes a single item from the iterator
thisWinUI.IteratorDataSource.prototype._read = function () {
    // Store in key/dataObject pair
    var len = this._items.length;
    this._items[len] = { key: len.toString(), dataObject: this._iterator.Current };

    // Maintain invariant
    if (!this._iterator.MoveNext()) {
        this._iterator = null;
    }
};

// setDataNotificationHandler: not implemented

// itemsFromStart: not implemented

thisWinUI.IteratorDataSource.prototype.itemsFromEnd = function (count, itemsAvailable) {
    // Read the entire list until the count is known
    while (this._iterator) {
        this._read();
    }

    this.itemsFromIndex(this._items.length - 1, count - 1, 0, itemsAvailable);
};

// itemsFromKey: not implemented

thisWinUI.IteratorDataSource.prototype.itemsFromIndex = function (index, countBefore, countAfter, itemsAvailable) {
    var last = index + countAfter + 1;

    while (this._iterator && this._items.length < last) {
        this._read();
    }

    var len = this._items.length;
    if (index >= len) {
        itemsAvailable(thisWinUI.FetchResult.doesNotExist);
    } else {
        var first = Math.max(0, index - countBefore),
            results = this._items.slice(first, last),
            offset = index - first;
        itemsAvailable(results, offset, this._iterator ? null : len, index);
    }
};

// itemsFromPrefix: not implemented

// Editing methods not implemented

// Vector / VectorView Data Source

thisWinUI.createVectorViewDataSource = function (vectorView) {
    /// <summary>
    /// Creates an IVectorView-based data source.
    /// </summary>
    /// <param name=""vectorView"" type=""Object"">
    /// An object that implements the IVectorView interface.
    /// </param>
    /// <remarks>
    /// vectorView must implement the following WinRT methods:
    ///
    ///     GetAt([in] UInt32 n, [out, retval] T*);
    ///     Size([out, retval] UInt32);
    ///
    /// When the underlying vector view is invalidated, the source invalidates all content and behaves as if it were
    /// empty.
    /// TODO: consider making this visible by exposing a field/property?
    /// </remarks>

    // Private members
    var _invalidateAll;

    // Tracks whether the view has changed (at which point it behaves as if it were empty).
    var changed = false;

    function exceptionIsChanged(exception) {
        // TODO: detect an E_CHANGED_STATE result as a script exception.
        return exception.message === ""E_CHANGED_STATE"";
    }

    function handleException(exception, itemsAvailable, countAvailable) {
        if (exceptionIsChanged(exception)) {
            changed = true;

            _invalidateAll();

            if (itemsAvailable) {
                itemsAvailable(thisWinUI.FetchResult.doesNotExist);
            } else if (countAvailable) {
                countAvailable(0);
            }
        } else {
            throw exception;
        }
    }

    function item(key, dataObject) {
        return { key: key.toString(), dataObject: dataObject };
    }

    // Public methods for read-only sources (IVectorView)

    return {
        setDataNotificationHandler: function (invalidateAll) {
            _invalidateAll = invalidateAll;
        },

        // itemsFromStart: not implemented

        itemsFromEnd: function (count, itemsAvailable) {
            if (changed) {
                itemsAvailable(thisWinUI.FetchResult.doesNotExist);
                return;
            }

            try {
                this.itemsFromIndex(vectorView.Size - 1, count - 1, 0, itemsAvailable);
            } catch (e) {
                handleException(e, itemsAvailable, null);
            }
        },

        // itemsFromKey: not implemented

        itemsFromIndex: function (index, countBefore, countAfter, itemsAvailable) {
            if (changed) {
                itemsAvailable(thisWinUI.FetchResult.doesNotExist);
                return;
            }

            var first = Math.max(0, index - countBefore),
                last = index + countAfter + 1,
                results = [],
                offset;

            try {
                var vectorLength = vectorView.Size;

                // Index can be negative if itemsFromEnd called on empty list
                if (isNaN(index) || index < 0 || index >= vectorLength) {
                    itemsAvailable(Win.UI.FetchResult.doesNotExist);
                    return;
                }

                if (last >= vectorLength) {
                    last = vectorLength;
                }

                var count = 0;
                for (var i = first; i < last; i++) {
                    var data = vectorView.GetAt(i);
                    results[count++] = item(i, data);
                }

                offset = index - first;

                itemsAvailable(results, offset, vectorLength, index);
            } catch (e) {
                handleException(e, itemsAvailable, null);
            }
        },

        // itemsFromPrefix: not implemented

        count: function (countAvailable) {
            if (changed) {
                countAvailable(0);
                return;
            }

            try {
                countAvailable(vectorView.Size);
            } catch (e) {
                handleException(e, null, countAvailable);
            }
        }
    };

};

thisWinUI.createVectorDataSource = function (vector) {
    /// <summary>
    /// Creates an IVector-based data source.
    /// </summary>
    /// <param name=""vector"">
    /// An object that implements the IVector interface.
    /// </param>
    /// <remarks>
    /// TODO:
    /// The problem for this implementation is that the vector
    /// is accessed by index and does not have stable keys, but
    /// the editing API requires stable keys to identify the
    /// objects on which to operate.
    ///
    /// This implementation generates a simple map between
    /// indices and auto-generated keys, and then updates
    /// the map as operations take place.
    ///
    /// A more sophisticated implementation would delay
    /// creation of the whole map (or fragments) until absolutely
    /// necessary, and would maintain a log of changes to post-process
    /// calculations instead of eagerly updating the (possibly implied) map.
    /// 
    /// One option might be to provide indexHint parameters to the editing methods.
    /// </remarks>

    // Private members
    var keyToIndexMap = {},
        indexToKeyMap = [];

    var i,
        len = vector.Size;

    for (i = 0; i < len; i++) {
        var keyAsString = i.toString();
        keyToIndexMap[keyAsString] = i;
        indexToKeyMap[i] = keyAsString;
    }

    var nextKey = len;

    function moveToIndex (sourceIndex, targetIndex, editResult) {
        // When we remove a source item that's prior to the target, that shifts the content
        var insertionIndex = (sourceIndex > targetIndex) ? targetIndex : targetIndex - 1;

        if (sourceIndex !== targetIndex) {
            var key = indexToKeyMap[sourceIndex];
            var item = vector.GetAt(sourceIndex);
            vector.RemoveAt(sourceIndex);

            // This can be improved by adjusting only a range rather than the whole array
            indexToKeyMap.splice(sourceIndex, 1);
            indexToKeyMap.splice(insertionIndex, 0, key);

            if (vector.Size === insertionIndex) {
                vector.Append(item);
            } else {
                vector.InsertAt(insertionIndex, item);
            }

            // The items between the moved items shift by one
            var first, count, delta;
            if (targetIndex < sourceIndex) {
                // Eg: move 4 before 2 in 1,2,3,4,5 becomes 1,4,2,3,5 - the shifted items move forward
                first = targetIndex + 1;
                count = sourceIndex - targetIndex;
                delta = 1;
            } else {
                // Eg: move 1 before 3 in 1,2,3,4,5 becomes 2,1,3,4,5 - the shifted items move backward
                first = sourceIndex;
                count = insertionIndex - sourceIndex;
                delta = -1;
            }

            shiftKeyToIndexMapCount(first, delta, count);
            keyToIndexMap[key] = insertionIndex;
        }

        if (editResult) {
            editResult(Win.UI.EditResult.success);
        }
    }

    // Inserts an item at the specified index, pushing items back to make room
    function insertAtIndex (index, dataObject, editResult) {
        // Insert first, as it's harder to verify correct than the tracking code (presumably)
        if (index === vector.Size) {
            vector.Append(dataObject);
        } else {
            vector.InsertAt(index, dataObject);
        }

        var keyValue = (nextKey++).toString();

        keyToIndexMap[keyValue] = index;
        indexToKeyMap.splice(index, 0, keyValue);

        shiftKeyToIndexMapForward(index + 1);

        if (editResult) {
            editResult(Win.UI.EditResult.success, keyValue);
        }
    }

    function itemAtIndex (index, dataObject) {
        return { key: indexToKeyMap[index], dataObject: dataObject };
    }

    // Shifts the given number of items by the given delta the keyToIndexMap, starting with the first index
    function shiftKeyToIndexMapCount(first, delta, count) {
        for (var i = first, len = first + count; i < len; i++) {
            var key = indexToKeyMap[i];
            keyToIndexMap[key] += delta;
        }
    }

    // Shifts all items by the given delta the keyToIndexMap, starting with the first index
    function shiftKeyToIndexMap(first, delta) {
        shiftKeyToIndexMapCount(first, delta, indexToKeyMap.length - first);
    }

    function shiftKeyToIndexMapForward(first) {
        shiftKeyToIndexMap(first, 1);
    }

    function shiftKeyToIndexMapBackward(first) {
        shiftKeyToIndexMap(first, -1);
    }

    var result = {
        // itemsFromStart: not implemented

        itemsFromEnd: function (count, itemsAvailable) {
            this.itemsFromIndex(vector.Size - 1, count - 1, 0, itemsAvailable);
        },

        // itemsFromKey: not implemented

        itemsFromIndex: function (index, countBefore, countAfter, itemsAvailable) {
            var first = Math.max(0, index - countBefore),
                last = index + countAfter + 1,
                results = [],
                offset;

            var vectorLength = vector.Size;

            // Index can be negative if itemsFromEnd called on empty list
            if (isNaN(index) || index < 0 || index >= vectorLength) {
                // The result is undefined
                itemsAvailable(Win.UI.FetchResult.doesNotExist);
                return;
            }

            if (last >= vectorLength) {
                last = vectorLength;
            }

            for (var i = first; i < last; i++) {
                var data = vector.GetAt(i);
                results.push(itemAtIndex(i, data));
            }

            offset = index - first;

            itemsAvailable(results, offset, vectorLength, index);
        },

        // itemsFromPrefix: not implemented

        count: function (countAvailable) {
            countAvailable(vector.Size);
        },

        // Editing methods

        insertAtStart: function (key, dataObject, editResult) {
            // key parameter is ignored, as keys are generated.
            insertAtIndex(0, dataObject, editResult);
        },

        insertBefore: function (key, dataObject, nextKey, editResult) {
            // key parameter is ignored, as keys are generated
            var index = keyToIndexMap[nextKey];
            if (index === undefined) {
                if (editResult) {
                    editResult(thisWinUI.EditResult.noLongerMeaningful);
                }
                return;
            }

            insertAtIndex(index, dataObject, editResult);
        },

        insertAfter: function (key, dataObject, previousKey, editResult) {
            // key parameter is ignored, as keys are generated
            var index = keyToIndexMap[previousKey];
            if (index === undefined) {
                if (editResult) {
                    editResult(thisWinUI.EditResult.noLongerMeaningful);
                }
                return;
            }

            insertAtIndex(index + 1, dataObject, editResult);
        },

        insertAtEnd: function (key, dataObject, editResult) {
            // key parameter is ignored, as keys are generated
            insertAtIndex(vector.Size, dataObject, editResult);
        },

        change: function (key, newDataObject, editResult) {
            var index = keyToIndexMap[key];
            if (index === undefined) {
                if (editResult) {
                    editResult(thisWinUI.EditResult.noLongerMeaningful);
                }
                return;
            }

            vector.SetAt(index, newDataObject);
            editResult(Win.UI.EditResult.success);
        },

        moveToStart: function (key, editResult) {
            var index = keyToIndexMap[key];
            if (index === undefined) {
                if (editResult) {
                    editResult(thisWinUI.EditResult.noLongerMeaningful);
                }
                return;
            }

            moveToIndex(index, 0, editResult);
        },

        moveBefore: function (key, nextKey, editResult) {
            var index = keyToIndexMap[key];
            if (index === undefined) {
                if (editResult) {
                    editResult(thisWinUI.EditResult.noLongerMeaningful);
                }
                return;
            }

            var targetIndex = keyToIndexMap[nextKey];
            if (targetIndex === undefined) {
                if (editResult) {
                    editResult(thisWinUI.EditResult.noLongerMeaningful);
                }
                return;
            }

            moveToIndex(index, targetIndex, editResult);
        },

        moveAfter: function (key, previousKey, editResult) {
            var index = keyToIndexMap[key];
            if (index === undefined) {
                if (editResult) {
                    editResult(thisWinUI.EditResult.noLongerMeaningful);
                }
                return;
            }

            var targetIndex = keyToIndexMap[previousKey];
            if (targetIndex === undefined) {
                if (editResult) {
                    editResult(thisWinUI.EditResult.noLongerMeaningful);
                }
                return;
            }

            moveToIndex(index, targetIndex + 1, editResult);
        },

        moveToEnd: function (key, editResult) {
            var index = keyToIndexMap[key];
            if (index === undefined) {
                if (editResult) {
                    editResult(thisWinUI.EditResult.noLongerMeaningful);
                }
                return;
            }

            moveToIndex(index, vector.Size, editResult);
        },

        remove: function (key, editResult) {
            var index = keyToIndexMap[key];
            if (index === undefined) {
                if (editResult) {
                    editResult(thisWinUI.EditResult.noLongerMeaningful);
                }
                return;
            }

            vector.RemoveAt(index);

            delete keyToIndexMap[key];
            indexToKeyMap.splice(index, 1);
            shiftKeyToIndexMapBackward(index);

            if (editResult) {
                editResult(Win.UI.EditResult.success);
            }
        }
    };

    return result;
};

})(Win8.UI);

(function (thisWinUI) {

// Utilities are private and global pointer will be deleted so we need to cache it locally
var utilities = thisWinUI.Utilities;

var rtlListViewClass = ""win8-listview-rtl"";
var browseModeClass = ""win8-listview-browse"";
var singleSelectionModeClass = ""win8-listview-singleSelection"";
var multiSelectionModeClass = ""win8-listview-multiSelection"";
var staticModeClass = ""win8-listview-static"";
var itemClass = ""win8-listview-item"";
var selectedClass = ""win8-listview-item-selected"";
var pressedClass = ""win8-listview-item-pressed"";
var hoverClass = ""win8-listview-item-hover"";
var headerClass = ""win8-listview-groupHeader"";
var draggedItemClass = ""win8-listview-item-inTransit"";
var draggedNumberClass = ""win8-listview-item-inTransitNumber"";
var progressClass = ""win8-listview-progressbar"";
var progressContainerClass = ""win8-listview-progressbarContainer"";

var INVALID_INDEX = -1;
var UNINITIALIZED = -1;

var INITIALIZED = 0;
var REALIZED = 1;
var READY = 3;

var LEFT_MOUSE_BUTTON = 1;


// In CHK build threshold is small to exercise a code path which unwinds callstack. 
var FIND_GROUP_LOOP_THRESHOLD = 5;

// For better performance in FRE build ListView calls reclusively up to 100 frames on stack before unwinding callstack.




var SCROLLBAR_RANGE_FIX_DELAY = 1000;

var DEFAULT_ITEM_WIDTH = 256;
var DEFAULT_ITEM_HEIGHT = 88;
var DRAG_START_THRESHOLD = 10;

var AUTOSCROLL_THRESHOLD = 11;
var AUTOSCROLL_INTERVAL = 50;
var AUTOSCROLL_DELTA = 50;

var REORDER_FORMAT = ""msListViewReorder"";
var DRAG_TARGET_EXPANDO = ""msDragTarget"";

var DEFAULT_PAGES_TO_LOAD = 5;
var DEFAULT_ITEMS_TO_LOAD = 50; // used in flow layout
var DEFAULT_PAGE_LOAD_THRESHOLD = 2;
var DEFAULT_ITEM_LOAD_THRESHOLD = 20;

var UP = 0;
var RIGHT = 1;
var DOWN = 2;
var LEFT = 3;
var elementIsInvalid = ""Invalid argument: ListView expects valid DOM element as the first argument."";
var layoutIsInvalid = ""Invalid argument: layout must be one of following values: 'verticalgrid', "" +
    ""'horizontalgrid' or 'list'."";
var modeIsInvalid = ""Invalid argument: mode must be one of following values: 'static', 'browse', "" +
    ""'singleselection' or 'multiselection'."";
var loadingBehaviorIsInvalid = ""Invalid argument: loadingBehavior must be 'incremental' or 'randomaccess'."";
var itemIndexIsInvalid = ""Invalid argument: index is invalid."";
var pagesToLoadIsInvalid = ""Invalid argument: pagesToLoad must be a positive number."";
var itemsToLoadIsInvalid = ""Invalid argument: itemsToLoad must be a positive number."";
var pageLoadThresholdIsInvalid = ""Invalid argument: pageLoadThreshold must be a positive number."";
var itemLoadThresholdIsInvalid = ""Invalid argument: itemLoadThreshold must be a positive number."";// This component is responsible for calculating items' positions in vertical grid mode. 
// It doesn't operate on DOM elements. This is pure geometry.

function VerticalGroupedGridLayout(layoutSite) {
    this.site = layoutSite;
}

VerticalGroupedGridLayout.prototype = {
    update: function VerticalGroupedGridLayout_update(count) {
        do { if (count !== undefined) { } else { assertionFailed(""count !== undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 6478); } } while (false);

        this.count = count;
        this.viewportSize = this.site._getViewportSize();
        this.rtl = this.site._rtl();
        this.headerMargin = this.site._getHeaderMargin().top;
        this.leadingMargin = this.site._getLeadingMargin().top;
        this.leadingMargin -= this.headerMargin;
        this.headerSize = this.site._getHeaderTotalSize();
        if (this.site._getOptions().groupHeaderAbove) {
            this.headerSlot = {
                cx: 0,
                cy: this.headerSize.cy
            };
        } else {
            this.headerSlot = {
                cx: this.headerSize.cx,
                cy: this.headerMargin
            };
        }
        var totalSize = this.site._getItemTotalSize();
        this.itemsPerRow = Math.floor((this.viewportSize.cx - this.headerSlot.cx) / totalSize.cx);
        this.itemsPerRow = this.itemsPerRow > 0 ? this.itemsPerRow : 1;
        this.itemHeight = totalSize.cy;
        this.itemWidth = totalSize.cx;
        this.additionalMargin = (
            this.site._getOptions().justified && (this.itemsPerRow > 1) ?
                (this.viewportSize.cx - this.headerSlot.cx - totalSize.cx * this.itemsPerRow) / (this.itemsPerRow - 1) :
                0
        );
    },

    calcMaxItemsPerViewport: function VerticalGroupedGridLayout_calcMaxItemsPerViewport() {
        return Math.ceil(this.viewportSize.cy / this.itemHeight) * this.itemsPerRow;
    },

    getCanvasHeight: function VerticalGroupedGridLayout_getCanvasHeight(groups, absolute) {
        var offset = 0,
            count = this.count;

        if (groups && groups.length() > 0) {
            var lastGroup = groups.group(groups.length() - 1);

            offset = lastGroup[absolute ? ""absoluteOffset"" : ""offset""];
            count -= lastGroup.startIndex;
        }

        return offset + Math.ceil(count / this.itemsPerRow) * this.itemHeight + this.headerSlot.cy;
    },

    calcCanvasSize: function VerticalGroupedGridLayout_calcCanvasSize(groups, absolute) {
        return {
            cx: this.viewportSize.cx,
            cy: this.getCanvasHeight(groups, absolute)
        };
    },

    getAdjacent: function VerticalGroupedGridLayout_getAdjacent(index, direction, groups) {
        do { if ((direction === UP || direction === DOWN || direction === LEFT || direction === RIGHT) && groups) { } else { assertionFailed(""(direction === UP || direction === DOWN || direction === LEFT || direction === RIGHT) && groups"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 6536); } } while (false);
        if (this.rtl) {
            if (direction === LEFT) {
                direction = RIGHT;
            } else if (direction === RIGHT) {
                direction = LEFT;
            }
        }

        if (direction === LEFT) {
            return index - 1;
        } else if (direction === RIGHT) {
            return index + 1;
        }

        var currentGroup = groups.group(groups.groupFromItem(index)),
            neighboringGroups = groups.getNeighboringGroups(currentGroup),
            currentRow = Math.floor((index - currentGroup.startIndex) / this.itemsPerRow);

        if (direction === UP) {
            if (currentRow === 0) {
                if (neighboringGroups.prev === null) {
                    return -1;
                }

                return currentGroup.startIndex - 1;
            }

            return index - this.itemsPerRow;
        } else {
            if (neighboringGroups.next === null) {
                return index + this.itemsPerRow;
            } else {
                var nextStartIndex = neighboringGroups.next.startIndex;
                var lastRowOfGroup = Math.floor((nextStartIndex - currentGroup.startIndex - 1) / this.itemsPerRow);
                if (currentRow === lastRowOfGroup) {
                    return nextStartIndex;
                }

                return Math.min(index + this.itemsPerRow, nextStartIndex - 1);
            }
        }
    },

    indexToCoordinate: function VerticalGroupedGridLayout_indexToCoordinates(index) {
        var row = Math.floor(index / this.itemsPerRow);
        return {
            row: row,
            column: index - row * this.itemsPerRow
        };
    },

    // Calculates position of an item assuming Left To Right direction
    calcItemLtrPosition: function VerticalGroupedGridLayout_calcItemLtrPosition(index, groupIndex, groups, absolute) {
        this.updateOffsets(groups);

        var group = groups.group(groupIndex),
            coordinates = this.indexToCoordinate(group ? index - group.startIndex : index),
            pos = {
                top: (group ? group[absolute ? ""absoluteOffset"" : ""offset""] : 0) + this.headerSlot.cy + (groupIndex ? 0 : this.leadingMargin) + coordinates.row * this.itemHeight,
                left: this.headerSlot.cx + coordinates.column * (this.itemWidth + this.additionalMargin)
            };

        do { if (!group || group.offset !== undefined) { } else { assertionFailed(""!group || group.offset !== undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 6599); } } while (false);

        return pos;
    },

    calcItemPosition: function VerticalGroupedGridLayout_calcItemPosition(index, groupIndex, groups, absolute) {
        var pos = this.calcItemLtrPosition(index, groupIndex, groups, absolute);

        if (!absolute && this.rtl) {
            pos.left = this.viewportSize.cx - pos.left - this.itemWidth;
        }

        return pos;
    },

    calcHeaderPosition: function VerticalGroupedGridLayout_calcHeaderPosition(groupIndex, groups) {
        this.updateOffsets(groups);

        var group = groups.group(groupIndex),
            pos = {
                top: group.offset,
                left: 0
            };

        do { if (group.offset !== undefined) { } else { assertionFailed(""group.offset !== undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 6623); } } while (false);

        if (this.rtl) {
            pos.left = this.viewportSize.cx - pos.left - this.headerSize.cx;
        }

        return pos;
    },

    groupFromOffset: function VerticalGroupedGridLayout_groupFromOffset(offset, groups) {
        this.updateOffsets(groups);
        return groups.groupFromOffset(offset);
    },

    calcFirstDisplayedItem: function VerticalGroupedGridLayout_calcFirstDisplayedItem(scrollbarPos, wholeItem, groups) {
        var groupIndex = this.groupFromOffset(scrollbarPos, groups);
        if (groupIndex !== null) {
            var nextGroup;
            if (groupIndex + 1 < groups.length()) {
                nextGroup = groups.group(groupIndex + 1);
            }
            var group = groups.group(groupIndex),
                startIndex = group.startIndex,
                groupOffset = group.offset,
                index = Math[wholeItem ? ""ceil"" : ""floor""](Math.max(0, (scrollbarPos - groupOffset - this.headerSlot.cy) / this.itemHeight)) * this.itemsPerRow;
            return nextGroup ? Math.min(startIndex + index, nextGroup.startIndex - 1) : startIndex + index;
        } else {
            return Math[wholeItem ? ""ceil"" : ""floor""](scrollbarPos / this.itemHeight) * this.itemsPerRow;
        }
    },

    calcLastDisplayedItem: function VerticalGroupedGridLayout_calcLastDisplayedItem(scrollbarPos, wholeItem, groups) {
        var offset = scrollbarPos + this.viewportSize.cy,
            groupIndex = this.groupFromOffset(offset, groups);

        if (groupIndex !== null) {
            var group = groups.group(groupIndex),
                startIndex = group.startIndex,
                groupOffset = group.offset;
            if (offset - groupOffset >= this.headerSlot.cy) {
                var index = Math[wholeItem ? ""floor"" : ""ceil""](Math.max(1, (offset - groupOffset - this.headerSlot.cy) / this.itemHeight)) * this.itemsPerRow - 1;
                do { if (index >= 0) { } else { assertionFailed(""index >= 0"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 6664); } } while (false);
                if (groupIndex + 1 < groups.length()) {
                    return Math.min(startIndex + index, groups.group(groupIndex + 1).startIndex - 1);
                } else {
                    return startIndex + index;
                }
            } else {
                return Math.max(0, startIndex - 1);
            }
        } else {
            return Math[wholeItem ? ""floor"" : ""ceil""](offset / this.itemHeight) * this.itemsPerRow - 1;
        }
    },

    hitTest: function VerticalGroupedGridLayout_hitTest(x, y) {
        x = this.rtl ? this.viewportSize.cx - x : x;
        return Math.floor((y - this.leadingMargin) / this.itemHeight) * this.itemsPerRow +
               Math.min(this.itemsPerRow - 1, Math.floor(x / (this.itemWidth + this.additionalMargin)));
    },

    scrollTo: function VerticalGroupedGridLayout_scrollTo(itemIndex, groups) {
        var groupIndex = groups.groupFromItem(itemIndex),
            pos = this.calcItemLtrPosition(itemIndex, groupIndex, groups, true);

        groups.pinItem(itemIndex, pos);
        return pos.top;
    },

    ensureVisible: function VerticalGroupedGridLayout_ensureVisible(scrollbarPos, itemIndex, groups) {
        if (itemIndex < this.calcFirstDisplayedItem(scrollbarPos, true, groups)) {
            return this.scrollTo(itemIndex, groups);
        } else if (itemIndex > this.calcLastDisplayedItem(scrollbarPos, true, groups)) {
            var groupIndex = groups.groupFromItem(itemIndex),
                pos = this.calcItemLtrPosition(itemIndex, groupIndex, groups, true);
            groups.pinItem(itemIndex, pos);
            pos.bottom = pos.top + this.itemHeight;
            return pos.bottom - this.viewportSize.cy;
        } else {
            return scrollbarPos;
        }
    },

    getGroupSize: function VerticalGroupedGridLayout_getGroupSize(itemsCount, groupIndex) {
        return itemsCount > 0 ? Math.ceil(itemsCount / this.itemsPerRow) * this.itemHeight + this.headerSlot.cy + (groupIndex ? 0 : this.leadingMargin) : 0;
    },

    updateOffsets: function VerticalGroupedGridLayout_updateOffsets(groups) {
        if (groups.dirty) {
            var count = groups.length();

            if (count) {
                var previousStartIndex = 0,
                    previousOffset = 0;

                for (var i = 0; i < count; i++) {
                    var group = groups.group(i),
                        itemsCount = group.startIndex - previousStartIndex;

                    group.offset = previousOffset + this.getGroupSize(itemsCount, i - 1);
                    group.absoluteOffset = group.offset;
                    previousOffset = group.offset;
                    previousStartIndex = group.startIndex;
                }

                if (groups.pinnedItem !== undefined) {
                    var pinnedGroupIndex = groups.groupFromItem(groups.pinnedItem),
                        pinnedGroup = groups.group(pinnedGroupIndex),
                        pinnedCoordinates = this.indexToCoordinate(groups.pinnedItem - pinnedGroup.startIndex),
                        pinnedGroupOffset = groups.pinnedOffset.top - this.headerSlot.cy - (pinnedGroupIndex ? 0 : this.leadingMargin) - pinnedCoordinates.row * this.itemHeight,
                        correction = pinnedGroupOffset - pinnedGroup.offset;
                    for (i = 0; i < count; i++) {
                        groups.group(i).offset += correction;
                    }
                }
            }

            groups.dirty = false;
        }
    }
};
// This component is responsible for calculating items' positions in horizontal grid mode. 
// It doesn't operate on DOM elements. This is pure geometry.

function HorizontalGroupedGridLayout(layoutSite) {
    this.site = layoutSite;
}

HorizontalGroupedGridLayout.prototype = {
    update: function HorizontalGroupedGridLayout_update(count) {
        do { if (count !== undefined) { } else { assertionFailed(""count !== undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 6753); } } while (false);

        this.count = count;
        this.viewportSize = this.site._getViewportSize();
        this.rtl = this.site._rtl();
        this.headerMargin = this.site._getHeaderMargin()[this.rtl ? ""right"" : ""left""];
        this.leadingMargin = this.site._getLeadingMargin()[this.rtl ? ""right"" : ""left""];
        this.leadingMargin -= this.headerMargin;
        this.headerSize = this.site._getHeaderTotalSize();
        if (this.site._getOptions().groupHeaderAbove) {
            this.headerSlot = {
                cx: this.headerMargin,
                cy: this.headerSize.cy
            };
        } else {
            this.headerSlot = {
                cx: this.headerSize.cx,
                cy: 0
            };
        }

        var totalSize = this.site._getItemTotalSize();
        this.itemsPerColumn = Math.floor((this.viewportSize.cy - this.headerSlot.cy) / totalSize.cy);
        this.itemsPerColumn = this.itemsPerColumn > 0 ? this.itemsPerColumn : 1;
        this.itemHeight = totalSize.cy;
        this.itemWidth = totalSize.cx;
        this.additionalMargin = (
            this.site._getOptions().justified && (this.itemsPerColumn > 1) ?
                (this.viewportSize.cy - this.headerSlot.cy - totalSize.cy * this.itemsPerColumn) / (this.itemsPerColumn - 1) :
                0
        );
    },

    calcMaxItemsPerViewport: function HorizontalGroupedGridLayout_calcMaxItemsPerViewport() {
        return Math.ceil(this.viewportSize.cx / this.itemWidth) * this.itemsPerColumn;
    },

    getCanvasWidth: function HorizontalGroupedGridLayout_getCanvasWidth(groups, absolute) {
        var offset = 0,
            count = this.count,
            lastGroup;

        if (groups && groups.length() > 0) {
            lastGroup = groups.group(groups.length() - 1);

            offset = lastGroup[absolute ? ""absoluteOffset"" : ""offset""];
            count -= lastGroup.startIndex;
        }

        return offset + Math.ceil(count / this.itemsPerColumn) * this.itemWidth + this.headerSlot.cx;
    },

    calcCanvasSize: function HorizontalGroupedGridLayout_calcCanvasSize(groups, absolute) {
        return {
            cx: this.getCanvasWidth(groups, absolute),
            cy: this.viewportSize.cy
        };
    },

    getAdjacent: function HorizontalGroupedGridLayout_getAdjacent(index, direction, groups) {
        do { if ((direction === UP || direction === DOWN || direction === LEFT || direction === RIGHT) && groups) { } else { assertionFailed(""(direction === UP || direction === DOWN || direction === LEFT || direction === RIGHT) && groups"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 6813); } } while (false);
        if (this.rtl) {
            if (direction === LEFT) {
                direction = RIGHT;
            } else if (direction === RIGHT) {
                direction = LEFT;
            }
        }

        if (direction === UP) {
            return index - 1;
        } else if (direction === DOWN) {
            return index + 1;
        }

        var currentGroup = groups.group(groups.groupFromItem(index)),
            neighboringGroups = groups.getNeighboringGroups(currentGroup),
            currentColumn = Math.floor((index - currentGroup.startIndex) / this.itemsPerColumn);

        if (direction === LEFT) {
            if (currentColumn === 0) {
                if (neighboringGroups.prev === null) {
                    return -1;
                }

                return currentGroup.startIndex - 1;
            }

            return index - this.itemsPerColumn;
        } else {
            if (neighboringGroups.next === null) {
                return index + this.itemsPerColumn;
            } else {
                var nextStartIndex = neighboringGroups.next.startIndex;
                var lastColumnOfGroup = Math.floor((nextStartIndex - currentGroup.startIndex - 1) / this.itemsPerColumn);
                if (currentColumn === lastColumnOfGroup) {
                    return nextStartIndex;
                }

                return Math.min(index + this.itemsPerColumn, nextStartIndex - 1);
            }
        }
    },

    indexToCoordinate: function HorizontalGroupedGridLayout_indexToCoordinates(index) {
        var column = Math.floor(index / this.itemsPerColumn);
        return {
            column: column,
            row: index - column * this.itemsPerColumn
        };
    },

    // Calculates position of an item assuming Left To Right direction
    calcItemLtrPosition: function HorizontalGroupedGridLayout_calcItemLtrPosition(index, groupIndex, groups, absolute) {
        this.updateOffsets(groups);

        var group = groups.group(groupIndex),
            coordinates = this.indexToCoordinate(group ? index - group.startIndex : index),
            pos = {
                top: this.headerSlot.cy + coordinates.row * (this.itemHeight + this.additionalMargin),
                left: (group ? group[absolute ? ""absoluteOffset"" : ""offset""] : 0) + this.headerSlot.cx + (groupIndex ? 0 : this.leadingMargin) + coordinates.column * this.itemWidth
            };

        do { if (!group || group.offset !== undefined) { } else { assertionFailed(""!group || group.offset !== undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 6876); } } while (false);

        return pos;
    },

    calcItemPosition: function HorizontalGroupedGridLayout_calcItemPosition(index, groupIndex, groups, absolute) {
        var pos = this.calcItemLtrPosition(index, groupIndex, groups, absolute);

        if (!absolute && this.rtl) {
            pos.left = this.getCanvasWidth(groups) - pos.left - this.itemWidth;
        }

        return pos;
    },

    calcHeaderPosition: function HorizontalGroupedGridLayout_calcHeaderPosition(groupIndex, groups) {
        this.updateOffsets(groups);

        var group = groups.group(groupIndex),
            pos = {
                top: 0,
                left: group.offset
            };

        do { if (group.offset !== undefined) { } else { assertionFailed(""group.offset !== undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 6900); } } while (false);

        if (this.rtl) {
            pos.left = this.getCanvasWidth(groups) - pos.left - this.headerSize.cx - (groupIndex ? 0 : this.leadingMargin);
        }

        return pos;
    },

    groupFromOffset: function HorizontalGroupedGridLayout_groupFromOffset(offset, groups) {
        this.updateOffsets(groups);
        return groups.groupFromOffset(offset);
    },

    calcFirstDisplayedItem: function HorizontalGroupedGridLayout_calcFirstDisplayedItem(scrollbarPos, wholeItem, groups) {
        var groupIndex = this.groupFromOffset(scrollbarPos, groups);
        if (groupIndex !== null) {
            var nextGroup;
            if (groupIndex + 1 < groups.length()) {
                nextGroup = groups.group(groupIndex + 1);
            }
            var group = groups.group(groupIndex),
                startIndex = group.startIndex,
                groupOffset = group.offset,
                index = Math[wholeItem ? ""ceil"" : ""floor""](Math.max(0, (scrollbarPos - groupOffset - this.headerSlot.cx) / this.itemWidth)) * this.itemsPerColumn;
            return nextGroup ? Math.min(startIndex + index, nextGroup.startIndex - 1) : startIndex + index;
        } else {
            return Math[wholeItem ? ""ceil"" : ""floor""](Math.max(0, scrollbarPos - this.leadingMargin) / this.itemWidth) * this.itemsPerColumn;
        }
    },

    calcLastDisplayedItem: function HorizontalGroupedGridLayout_calcLastDisplayedItem(scrollbarPos, wholeItem, groups) {
        var offset = scrollbarPos + this.viewportSize.cx,
            groupIndex = this.groupFromOffset(offset, groups);

        if (groupIndex !== null) {
            var group = groups.group(groupIndex),
                startIndex = group.startIndex,
                groupOffset = group.offset;
            if (offset - groupOffset >= this.headerSlot.cx) {
                var index = Math[wholeItem ? ""floor"" : ""ceil""](Math.max(1, (offset - groupOffset - this.headerSlot.cx) / this.itemWidth)) * this.itemsPerColumn - 1;
                do { if (index >= 0) { } else { assertionFailed(""index >= 0"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 6941); } } while (false);
                if (groupIndex + 1 < groups.length()) {
                    return Math.min(startIndex + index, groups.group(groupIndex + 1).startIndex - 1);
                } else {
                    return startIndex + index;
                }
            } else {
                return Math.max(0, startIndex - 1);
            }
        } else {
            return Math[wholeItem ? ""floor"" : ""ceil""](Math.max(0, offset - this.leadingMargin) / this.itemWidth) * this.itemsPerColumn - 1;
        }
    },

    hitTest: function HorizontalGroupedGridLayout_hitTest(x, y, groups) {
        if (this.rtl) {
            x = this.getCanvasWidth(groups) - x;
        }
        return Math.floor((x - this.leadingMargin) / this.itemWidth) * this.itemsPerColumn +
               Math.min(this.itemsPerColumn - 1, Math.floor(y / (this.itemHeight + this.additionalMargin)));
    },

    scrollTo: function HorizontalGroupedGridLayout_scrollTo(itemIndex, groups) {
        var groupIndex = groups.groupFromItem(itemIndex),
            pos = this.calcItemLtrPosition(itemIndex, groupIndex, groups, true);

        groups.pinItem(itemIndex, pos);
        return pos.left;
    },

    ensureVisible: function HorizontalGroupedGridLayout_ensureVisible(scrollbarPos, itemIndex, groups) {
        if (itemIndex < this.calcFirstDisplayedItem(scrollbarPos, true, groups)) {
            return this.scrollTo(itemIndex, groups);
        } else if (itemIndex > this.calcLastDisplayedItem(scrollbarPos, true, groups)) {
            var groupIndex = groups.groupFromItem(itemIndex),
                pos = this.calcItemLtrPosition(itemIndex, groupIndex, groups, true);
            groups.pinItem(itemIndex, pos);
            pos.right = pos.left + this.itemWidth;
            return pos.right - this.viewportSize.cx;
        } else {
            return scrollbarPos;
        }
    },

    getGroupSize: function HorizontalGroupedGridLayout_getGroupSize(itemsCount, groupIndex) {
        return itemsCount > 0 ? Math.ceil(itemsCount / this.itemsPerColumn) * this.itemWidth + this.headerSlot.cx + (groupIndex ? 0 : this.leadingMargin) : 0;
    },

    updateOffsets: function HorizontalGroupedGridLayout_updateOffsets(groups) {
        if (groups.dirty) {
            var count = groups.length();

            if (count) {
                var previousStartIndex = 0,
                    previousOffset = 0;

                for (var i = 0; i < count; i++) {
                    var group = groups.group(i),
                        itemsCount = group.startIndex - previousStartIndex;
                    group.offset = previousOffset + this.getGroupSize(itemsCount, i - 1);
                    group.absoluteOffset = group.offset;
                    previousOffset = group.offset;
                    previousStartIndex = group.startIndex;
                }

                if (groups.pinnedItem !== undefined) {
                    var pinnedGroupIndex = groups.groupFromItem(groups.pinnedItem),
                        pinnedGroup = groups.group(pinnedGroupIndex),
                        pinnedCoordinates = this.indexToCoordinate(groups.pinnedItem - pinnedGroup.startIndex),
                        pinnedGroupOffset = groups.pinnedOffset.left - this.headerSlot.cx - (pinnedGroupIndex ? 0 : this.leadingMargin) - pinnedCoordinates.column * this.itemWidth,
                        correction = pinnedGroupOffset - pinnedGroup.offset;
                    for (i = 0; i < count; i++) {
                        groups.group(i).offset += correction;
                    }
                }
            }

            groups.dirty = false;
        }
    }
};
// This component is responsible for calculating items' positions in list mode. 

function ListLayout(layoutSite) {
    this.site = layoutSite;
}

ListLayout.prototype = {
    update: function ListLayout_update(count) {
        var itemTotalSize = this.site._getItemTotalSize(),
            itemContentSize = this.site._getItemContentSize(),
            overhead = itemTotalSize.cx - itemContentSize.cx;
        this.count = count;
        this.viewportSize = this.site._getViewportSize();
        this.itemHeight = itemTotalSize.cy;
        this.itemWidth = this.viewportSize.cx - overhead;
    },

    calcCanvasSize: function ListLayout_calcCanvasSize() {
        return {
            cx: this.site._getViewportSize().cx,
            cy: this.count * this.itemHeight
        };
    },

    calcItemLtrPosition: function ListLayout_calcItemLtrPosition(index) {
        return {
            top: index * this.itemHeight,
            left: 0,
            width: this.itemWidth,
            height: this.itemHeight
        };
    },

    calcItemPosition: function ListLayout_calcItemPosition(index) {
        return this.calcItemLtrPosition(index);
    },

    getAdjacent: function ListLayout_getAdjacent(index, direction) {
        do { if (direction === UP || direction === DOWN || direction === LEFT || direction === RIGHT) { } else { assertionFailed(""direction === UP || direction === DOWN || direction === LEFT || direction === RIGHT"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 7060); } } while (false);
        if (this.rtl) {
            if (direction === LEFT) {
                direction = RIGHT;
            } else if (direction === RIGHT) {
                direction = LEFT;
            }
        }

        if (direction === UP) {
            return index - 1;
        } else if (direction === DOWN) {
            return index + 1;
        } else if (direction === LEFT) { // TODO: evaluate whether or not left/right should go no where in flow layout, or to next/prev item
            return index - 1;
        } else {
            return index + 1;
        }
    },

    calcFirstDisplayedItem: function ListLayout_calcFirstDisplayedItem(scrollbarPos, wholeItem) {
        return Math[wholeItem ? ""ceil"" : ""floor""](scrollbarPos / this.itemHeight);
    },

    calcLastDisplayedItem: function ListLayout_calcLastDisplayedItem(scrollbarPos, wholeItem) {
        return Math[wholeItem ? ""floor"" : ""ceil""]((scrollbarPos + this.viewportSize.cy) / this.itemHeight) - 1;
    },

    hitTest: function ListLayout_hitTest(x, y) {
        return Math.floor(y / this.itemHeight);
    },

    scrollTo: function ListLayout_scrollTo(itemIndex) {
        return this.calcItemPosition(itemIndex).top;
    },

    ensureVisible: function ListLayout_ensureVisible(scrollbarPos, itemIndex) {
        if (itemIndex < this.calcFirstDisplayedItem(scrollbarPos, true)) {
            return this.scrollTo(itemIndex);
        } else if (itemIndex > this.calcLastDisplayedItem(scrollbarPos, true)) {
            var pos = this.calcItemPosition(itemIndex);
            pos.bottom = pos.top + this.itemHeight;
            return pos.bottom - this.viewportSize.cy;
        } else {
            return scrollbarPos;
        }
    }
};
// These are mostly empty functions because this mode is used when browser's flow layout is used. 

function FlowLayout(layoutSite) {
    this.site = layoutSite;
}

FlowLayout.prototype = {
    update: function FlowLayout_update() {
        this.viewportSize = this.site._getViewportSize();
    },

    calcItemPosition: function FlowLayout_calcItemPosition(index) {
        return undefined;
    },

    calcFirstDisplayedItem: function FlowLayout_calcFirstDisplayedItem(scrollbarPos, wholeItem) {
        var childNodes = this.site._getCanvas().childNodes,
            ignoredNodesCount = 0;
        for (var i = 0, count = childNodes.length; i < count; i++) {
            var item = childNodes[i],
                offsetTop = item.offsetTop,
                bottom = offsetTop + item.offsetHeight;

            if (item.ignoreInDisplayedItems) {
                ignoredNodesCount++;
            }

            if ((scrollbarPos >= offsetTop) && (scrollbarPos < bottom)) {
                var index = wholeItem && (scrollbarPos > offsetTop) ? i + 1 : i;
                return index - ignoredNodesCount;
            }
        }

        return -1;
    },

    calcLastDisplayedItem: function FlowLayout_calcLastDisplayedItem(scrollbarPos, wholeItem) {
        var bottomEdge = scrollbarPos + this.viewportSize.cy,
            childNodes = this.site._getCanvas().childNodes,
            ignoredNodesCount = 0;

        // It's necessary to iterate forward in calcLastDisplayedItem here because IncrementalMode adds
        // elements to the layout that aren't actually a part of the counted indices. 
        for (var i = 0, count = childNodes.length; i < count; i++) {
            var item = childNodes[i],
                offsetTop = item.offsetTop,
                bottom = offsetTop + item.offsetHeight;

            if (item.ignoreInDisplayedItems) {
                ignoredNodesCount++;
            }

            if ((bottomEdge > offsetTop) && (bottomEdge <= bottom)) {
                var index = wholeItem && (bottomEdge < bottom) ? i - 1 : i;
                return index - ignoredNodesCount;
            }
        }
        return childNodes.length;
    },

    getAdjacent: function FlowLayout_getAdjacent(index, direction) {
        do { if (direction === UP || direction === DOWN || direction === LEFT || direction === RIGHT) { } else { assertionFailed(""direction === UP || direction === DOWN || direction === LEFT || direction === RIGHT"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 7169); } } while (false);
        if (this.rtl) {
            if (direction === LEFT) {
                direction = RIGHT;
            } else if (direction === RIGHT) {
                direction = LEFT;
            }
        }

        if (direction === UP) {
            return index - 1;
        } else if (direction === DOWN) {
            return index + 1;
        } else if (direction === LEFT) { // TODO: evaluate whether or not left/right should go no where in flow layout, or to next/prev item
            return index - 1;
        } else {
            return index + 1;
        }
    },

    scrollTo: function FlowLayout_scrollTo(itemIndex) {
        var element = this.site._itemAt(itemIndex);
        return element.offsetTop;
    },

    ensureVisible: function FlowLayout_ensureVisible(scrollbarPos, itemIndex) {
        if (itemIndex < this.calcFirstDisplayedItem(scrollbarPos, true)) {
            return this.scrollTo(itemIndex);
        } else if (itemIndex > this.calcLastDisplayedItem(scrollbarPos, true)) {
            var element = this.site._itemAt(itemIndex);
            return element.offsetTop + element.offsetHeight - this.viewportSize.cy;
        } else {
            return scrollbarPos;
        }
    }
};
function ItemsContainer(site) {
    this.site = site;
    this.itemData = {};
    this.dataIndexToLayoutIndex = {};
}

ItemsContainer.prototype = {
    setItems: function ItemsContainer_setItems(newItems) {
        var tmp;

        for (var i = 0, count = newItems.length; i < count; i++) {
            tmp = newItems[i];
            this.itemData[tmp.index] = tmp;
        }
    },

    removeItems: function ItemsContainer_removeItems() {
        this.itemData = {};
    },

    itemAt: function ItemsContainer_itemAt(itemIndex) {
        var itemData = this.itemData[itemIndex];
        return itemData ? itemData.element : null;
    },

    itemDataAt: function ItemsContainer_itemDataAt(itemIndex) {
        return this.itemData[itemIndex];
    },

    itemFrom: function ItemsContainer_itemFrom(element) {
        while (element && element !== this.site._viewport && !utilities.hasClass(element, itemClass)) {
            element = element.parentNode;
        }
        return element !== this.site._viewport ? element : null;
    },

    index: function ItemsContainer_index(element) {
        var item = this.itemFrom(element);
        if (item) {
            for (var index in this.itemData) {
                if (this.itemData[index].element === item) {
                    return parseInt(index, 10);
                }
            }
        }

        return INVALID_INDEX;
    },

    updateSelection: function ItemsContainer_updateSelection(unselected, selected) {
        var itemData;
        for (var i = 0, len = unselected.length; i < len; i++) {
            itemData = this.itemData[unselected[i]];
            if (itemData) {
                utilities.removeClass(itemData.element, selectedClass);
                itemData.element.setAttribute(""aria-selected"", false);
            }
        }
        for (i = 0, len = selected.length; i < len; i++) {
            itemData = this.itemData[selected[i]];
            if (itemData) {
                utilities.addClass(itemData.element, selectedClass);
                itemData.element.setAttribute(""aria-selected"", true);
            }
        }
    },

    each: function ItemsContainer_each(callback) {
        for (var index in this.itemData) {
            if (this.itemData.hasOwnProperty(index)) {
                callback(parseInt(index, 10), this.itemData[index].element);
            }
        }
    },

    deleteItems: function ItemsContainer_deleteItems(itemIndices) {
        for (var i = 0, count = itemIndices.length; i < count; i++) {
            do { if (this.itemData[itemIndices[i]] !== undefined) { } else { assertionFailed(""this.itemData[itemIndices[i]] !== undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 7282); } } while (false);
            delete this.itemData[itemIndices[i]];
        }
    },

    setLayoutIndices: function ItemsContainer_setLayoutIndices(indices) {
        this.dataIndexToLayoutIndex = indices;
    },

    getLayoutIndex: function ItemsContainer_getLayoutIndex(dataIndex) {
        var layoutIndex = this.dataIndexToLayoutIndex[dataIndex];
        return layoutIndex === undefined ? dataIndex : layoutIndex;
    }
};
// This component is responsible for dividing the items into groups and storing the information about these groups.

function GroupsContainer(groupByFunction, groupRenderer) {
    this.groupByFunction = groupByFunction;
    this.groupRenderer = groupRenderer;
    this.groups = [];
    this.dirty = true;
}

GroupsContainer.prototype = {

    addItem: function GroupsContainer_addItem(itemsManager, itemIndex, element, groupAddedCallback) {
        var that = this;

        var previousItem = this.previousItem;
        this.previousItem = itemIndex;

        var currentIndex = this.groupFromItem(itemIndex);
        if (currentIndex === null && this.groups.length > 0) {
            currentIndex = 0;
        }
        
        var currentGroup = null,
            currentData = null,
            nextGroup = null;
        if (currentIndex !== null) {
            currentGroup = this.groups[currentIndex];
            currentData = currentGroup.userData;
            if (currentIndex + 1 < this.groups.length) {
                nextGroup = this.groups[currentIndex + 1];
            }
        }

        // The application verifies if the item belongs to the current group
        var newGroupData = this.groupByFunction(currentData, element.msDataItem.dataObject, itemIndex);
        do { if (newGroupData) { } else { assertionFailed(""newGroupData"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 7331); } } while (false);
        if (newGroupData === currentData) {
            if (itemIndex < currentGroup.startIndex) {
                currentGroup.startIndex = itemIndex;
                this.dirty = true;
            }
            // The item belongs to the current group
            if (currentGroup.waitingList) {
                currentGroup.waitingList.push(groupAddedCallback.bind(window, currentIndex));
            } else {
                groupAddedCallback(currentIndex);
            }
            // Maybe the item belongs to the next group. This can happen when the beginning of the next group is still not known (nextGroup.waitingList!== undefined).
        } else if (nextGroup && nextGroup.waitingList && nextGroup.userData === this.groupByFunction(nextGroup.userData, element.msDataItem.dataObject, itemIndex)) {
            nextGroup.waitingList.push(groupAddedCallback.bind(window, currentIndex + 1));
        } else {
            // The item belongs to a new group

            // If the item's index was just incremented then this new group starts with this item, so the startIndex is known
            if (previousItem + 1 === itemIndex) {
                currentIndex = this.addGroup(currentGroup, currentIndex, {
                    userData: newGroupData,
                    startIndex: itemIndex
                });
                groupAddedCallback(currentIndex);
            } else if (newGroupData.startIndex !== undefined) {
                // The application has provided startIndex for this group
                currentIndex = this.addGroup(currentGroup, currentIndex, {
                    userData: newGroupData,
                    startIndex: newGroupData.startIndex
                });
                groupAddedCallback(currentIndex);
            } else if (itemIndex === 0) {
                currentIndex = this.addGroup(null, null, {
                    userData: newGroupData,
                    startIndex: itemIndex
                });
                groupAddedCallback(currentIndex);
            } else {
                // We need to find the beginning of the group
                var newGroup = {
                    userData: newGroupData,
                    startIndex: itemIndex,
                    waitingList: []
                };
                currentIndex = this.addGroup(currentGroup, currentIndex, newGroup);
                newGroup.waitingList.push(groupAddedCallback.bind(window, currentIndex));
                this.findStart(itemsManager, newGroup, itemIndex, 0);
            }
        }
    },

    addGroup: function GroupsContainer_addGroup(currentGroup, currentIndex, toInsert) {
        if (currentGroup) {
            this.groups.splice(++currentIndex, 0, toInsert);
        } else {
            currentIndex = this.groups.length;
            this.groups.unshift(toInsert);
        }

        this.dirty = true;
        return currentIndex;
    },

    startFound: function GroupsContainer_startFound(group, itemIndex) {

        group.startIndex = itemIndex;
        this.dirty = true;

        var tmpWaiting = [];
        for (var i = 0, len = group.waitingList.length; i < len; i++) {
            tmpWaiting.push(group.waitingList[i]);
        }
        delete group.waitingList;

        // Beginning of the group has been found. The correct position of an item can be calculated at this point so 
        // all callbacks waiting until the group is added and ready to use can be called.
        for (i = 0; i < len; i++) {
            tmpWaiting[i](i === 0);
        }
    },

    findStart: function GroupsContainer_findStart(itemsManager, group, itemIndex, counter) {
        var that = this;

        if (itemIndex > 0) {
            if (counter < FIND_GROUP_LOOP_THRESHOLD) {
                itemsManager.simplerItemAtIndex(--itemIndex, function (element) {
                    var newGroupData = that.groupByFunction(group.userData, element.msDataItem.dataObject, itemIndex);
                    if (newGroupData !== group.userData) {
                        that.startFound(group, itemIndex + 1);
                    } else {
                        that.findStart(itemsManager, group, itemIndex, ++counter);
                    }
                });
            } else {
                group.startIndex = itemIndex;
                this.dirty = true;

                thisWinUI.setTimeout(function () {
                    that.findStart(itemsManager, group, itemIndex, 0);
                }, 0);
            }
        } else {
            this.startFound(group, itemIndex);
        }
    },

    groupFromImpl: function GroupsContainer_groupFromImpl(fromGroup, toGroup, comp) {
        if (toGroup < fromGroup) {
            return null;
        }

        var center = fromGroup + Math.floor((toGroup - fromGroup) / 2),
            centerGroup = this.groups[center];
        if (comp(centerGroup, center)) {
            return this.groupFromImpl(fromGroup, center - 1, comp);
        } else if (center < toGroup && !comp(this.groups[center + 1], center + 1)) {
            return this.groupFromImpl(center + 1, toGroup, comp);
        } else {
            return center;
        }
    },

    groupFrom: function GroupsContainer_groupFrom(comp) {
        if (this.groups.length > 0) {
            var lastGroupIndex = this.groups.length - 1,
                lastGroup = this.groups[lastGroupIndex];
            if (!comp(lastGroup, lastGroupIndex)) {
                return lastGroupIndex;
            } else {
                return this.groupFromImpl(0, this.groups.length - 1, comp);
            }
        } else {
            return null;
        }
    },

    groupFromItem: function GroupsContainer_groupFromItem(itemIndex) {
        return this.groupFrom(function (group) {
            return itemIndex < group.startIndex;
        });
    },

    groupFromOffset: function GroupsContainer_groupFromOffset(offset) {
        return this.groupFrom(function (group, groupIndex) {
            do { if (group.offset !== undefined) { } else { assertionFailed(""group.offset !== undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 7477); } } while (false);
            return offset < group.offset;
        });
    },

    group: function GroupsContainer_getGroup(index) {
        return this.groups[index];
    },

    length: function GroupsContainer_length() {
        return this.groups.length;
    },

    renderGroup: function GroupsContainer_renderGroup(index) {
        var group = this.groups[index],
            element = this.groupRenderer(group.userData);
        do { if (element) { } else { assertionFailed(""element"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 7493); } } while (false);
        do { if (utilities.isDOMElement(element)) { } else { assertionFailed(""utilities.isDOMElement(element)"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 7494); } } while (false);
        return element;
    },

    setHeaders: function GroupsContainer_setHeaders(newHeaders) {
        for (var i = 0, len = newHeaders.length; i < len; i++) {
            var header = newHeaders[i],
                group = this.groups[header.index];
            do { if (!group.element) { } else { assertionFailed(""!group.element"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 7502); } } while (false);
            group.element = header.element;
            group.left = header.left;
            group.top = header.top;
        }
    },

    removeGroups: function GroupsContainer_removeGroups() {
        this.groups = [];
        delete this.previousItem;
        delete this.pinnedItem;
        delete this.pinnedOffset;
        this.dirty = true;
    },

    resetGroups: function GroupsContainer_resetGroups(canvas) {
        for (var i = 0, len = this.groups.length; i < len; i++) {
            var group = this.groups[i];
            if (group.element) {
                canvas.removeChild(group.element);
            }
        }
        this.removeGroups();
    },

    rebuildGroups: function GroupsContainer_rebuildGroups(itemsManager, itemIndex, end, allGroupAddedCallback) {
        var that = this,
            counter = end - itemIndex;

        function itemAddedCallback() {
            if (--counter === 0) {
                allGroupAddedCallback();
            }
        }

        function addItemWrapper(itemsManager, itemIndex, callback) {
            itemsManager.simplerItemAtIndex(itemIndex, function (element) {
                that.addItem(itemsManager, itemIndex, element, callback);
            });
        }

        if (counter > 0) {
            if (itemIndex > 0) {
                // The first group always needs to be added
                counter++;
                addItemWrapper(itemsManager, 0, itemAddedCallback);
            }

            for (; itemIndex < end; itemIndex++) {
                addItemWrapper(itemsManager, itemIndex, itemAddedCallback);
            }
        } else {
            allGroupAddedCallback();
        }
    },

    pinItem: function GroupsContainer_pinItem(item, offset) {
        this.pinnedItem = item;
        this.pinnedOffset = offset;
        this.dirty = true;
    },

    getNeighboringGroups: function GroupsContainer_getNeighboringGroups(group) {
        var groups = this.groups;
        for (var i = 0; i < groups.length; i++) {
            if (groups[i] === group) {
                return {
                    prev: i > 0 ? groups[i - 1] : null,
                    next: i < (groups.length - 1) ? groups[i + 1] : null
                };
            }
        }
    }
};

function NoGroups() {
    this.groups = [{ startIndex: 0 }];
    this.dirty = true;

    this.rebuildGroups = function (itemsManager, begin, end, callback) {
        callback();
    };

    this.addItem = function (itemsManager, itemIndex, element, callback) {
        callback(null);
    };

    this.removeGroups = function () {
        this.groups = [{ startIndex: 0 }];
        delete this.previousItem;
        delete this.pinnedItem;
        delete this.pinnedOffset;
        this.dirty = true;
    };

    this.renderGroup = function () {
        return null;
    };
}

NoGroups.prototype = GroupsContainer.prototype;

// Virtualized scroll view

function ScrollView(scrollViewSite) {
    this.site = scrollViewSite;
    this.layout = new VerticalGroupedGridLayout(scrollViewSite);
    this.items = new ItemsContainer(scrollViewSite);
    this.groups = new NoGroups();
    this.begin = 0;
    this.end = 0;
    this.realizePass = 1;
    this.progressBar = document.createElement(""progress"");
    this.newItems = false;
    utilities.addClass(this.progressBar, progressClass);
}

ScrollView.prototype = {

    addItem: function ScrollView_addItem(fragment, newItems, newHeaders, itemIndex, itemSize, headerSize, count, currentPass, groupAddedCallback) {
        var that = this,
            im = this.site._getItemsManager();

        im.simplerItemAtIndex(itemIndex, function (element) {
            that.groups.addItem(im, itemIndex, element, function (groupIndex) {
                groupAddedCallback(function () {
                    if (that.realizePass === currentPass) {
                        if (groupIndex !== null) {
                            var group = that.groups.group(groupIndex);
                            if (!group.element && group.startIndex === itemIndex) {
                                that.addHeader(fragment, newHeaders, groupIndex, headerSize);
                            }
                        }

                        var layoutIndex = that.items.getLayoutIndex(itemIndex);
                        if (layoutIndex !== INVALID_INDEX) {
                            var itemPos = that.layout.calcItemPosition(layoutIndex, groupIndex, that.groups);

                            utilities.addClass(element, itemClass);

                            element.setAttribute(""role"", that.site._itemRole);
                            element.setAttribute(""aria-setsize"", count);
                            element.setAttribute(""aria-posinset"", itemIndex);

                            var width = itemPos.width ? itemPos.width : itemSize.cx;
                            element.style.cssText += ""position:absolute; left:"" + itemPos.left + ""px; top:"" + itemPos.top + ""px; width:"" + width + ""px; height:"" + itemSize.cy + ""px;"";

                            if (that.site._isSelected(itemIndex)) {
                                utilities.addClass(element, selectedClass);
                                element.setAttribute(""aria-selected"", true);
                            }

                            fragment.appendChild(element);

                            newItems.push({
                                index: itemIndex,
                                element: element,
                                left: itemPos.left,
                                top: itemPos.top,
                                width: width,
                                visible: true
                            });
                        }
                    }
                });
            });
        });
    },

    finalItem: function ScrollView_finalItem(callback) {
        this.site._itemsCount(function (count) {
            callback(count - 1);
        });
    },

    showProgressBar: function ScrollView_showProgressBar() {
        this.site._viewport.appendChild(this.progressBar);
    },

    hideProgressBar: function ScrollView_hideProgressBar() {
        if (this.progressBar.parentNode) {
            this.progressBar.parentNode.removeChild(this.progressBar);
        }
    },

    pageUp: function ScrollView_pageUp(currentFocus) {
        var scrollbarPos = this.site._scrollbarPos(),
            layout = this.layout,
            firstElementOnPage = layout.calcFirstDisplayedItem(scrollbarPos, true, this.groups);

        if (currentFocus !== firstElementOnPage) {
            return firstElementOnPage;
        }

        var offsetProp = this.site._horizontal() ? ""offsetWidth"" : ""offsetHeight"",
            currentItem = this.items.itemAt(currentFocus),
            newFocus = layout.calcFirstDisplayedItem(Math.max(0, scrollbarPos - this.site._getViewportLength() + (currentItem ? currentItem[offsetProp] : 0)), false, this.groups);

        // This check is necessary for items that are larger than the viewport
        newFocus = newFocus < currentFocus ? newFocus : currentFocus - 1;

        return newFocus;
    },

    pageDown: function ScrollView_pageDown(currentFocus) {
        var scrollbarPos = this.site._scrollbarPos(),
            layout = this.layout,
            lastElementOnPage = layout.calcLastDisplayedItem(scrollbarPos, true, this.groups);

        if (currentFocus !== lastElementOnPage) {
            return lastElementOnPage;
        }

        var offsetProp = this.site._horizontal() ? ""offsetWidth"" : ""offsetHeight"",
            currentItem = this.items.itemAt(currentFocus),
            newFocus = layout.calcLastDisplayedItem(scrollbarPos + this.site._getViewportLength() - (currentItem ? currentItem[offsetProp] : 0), false, this.groups);

        // This check is necessary for items that are larger than the viewport
        newFocus = newFocus > currentFocus ? newFocus : currentFocus + 1;

        return newFocus;
    },

    updateItem: function ScrollView_updateItem(itemData, itemIndex, itemIsReadyCallback) {
        var layoutIndex = this.items.getLayoutIndex(itemIndex);
        if (layoutIndex !== INVALID_INDEX) {
            var groupIndex = this.groups.groupFromItem(itemIndex),
                itemPos = this.layout.calcItemPosition(layoutIndex, groupIndex, this.groups);
            if (itemData.left !== itemPos.left ||
                itemData.top !== itemPos.top ||
                (itemPos.width && itemData.width !== itemPos.width) ||
                !itemData.visible) {

                itemData.visible = true;
                itemData.left = itemPos.left;
                itemData.top = itemPos.top;
                itemData.width = itemPos.width;

                var newStyle = ""display:block; left:"" + itemPos.left + ""px; top:"" + itemPos.top + ""px;"";
                if (itemPos.width) {
                    newStyle += ""width: "" + itemPos.width + ""px"";
                }
                itemData.element.style.cssText += newStyle;
            }
        } else {
            itemData.visible = false;
            itemData.element.style.display = ""none"";
        }

        itemIsReadyCallback();
    },

    realizeItems: function ScrollView_realizeItems(fragment, newItems, newHeaders, itemIndex, end, count, currentPass, itemsAreReadyCallback) {
        var counter = end - itemIndex,
            waitingList = [];

        function itemIsReady(addItemCallback) {
            if (addItemCallback) {
                waitingList.push(addItemCallback);
            }

            if (--counter === 0) {
                for (var i = 0, len = waitingList.length; i < len; i++) {
                    waitingList[i]();
                }
                itemsAreReadyCallback();
            }
        }

        if (counter > 0) {
            var itemSize = this.site._getItemContentSize(),
                headerSize = this.site._getHeaderContentSize();
            for (; itemIndex < end; itemIndex++) {
                var itemData = this.items.itemDataAt(itemIndex);
                if (!itemData) {
                    this.addItem(fragment, newItems, newHeaders, itemIndex, itemSize, headerSize, count, currentPass, itemIsReady);
                } else {
                    this.updateItem(itemData, itemIndex, itemIsReady);
                }
            }
        } else {
            itemsAreReadyCallback();
        }
    },

    addHeader: function ScrollView_addHeader(fragment, newHeaders, groupIndex, headerSize) {
        var element = this.groups.renderGroup(groupIndex);
        if (element) {
            var style = element.style,
                headerPos = this.layout.calcHeaderPosition(groupIndex, this.groups);

            utilities.addClass(element, headerClass);

            style.position = ""absolute"";
            style.left = headerPos.left + ""px"";
            style.top = headerPos.top + ""px"";
            style.width = headerSize.cx + ""px"";
            style.height = headerSize.cy + ""px"";

            if (groupIndex) {
                fragment.appendChild(element);
            } else {
                fragment.insertBefore(element, fragment.firstChild);
            }

            newHeaders.push({
                index: groupIndex,
                element: element,
                left: headerPos.left,
                top: headerPos.top
            });
        }
    },

    updateHeader: function ScrollView_updateHeader(group, groupIndex, headerSize) {
        var headerPos = this.layout.calcHeaderPosition(groupIndex, this.groups);
        if (group.left !== headerPos.left ||
            group.top !== headerPos.top) {
            var style = group.element.style;
            style.top = headerPos.top + ""px"";
            style.left = headerPos.left + ""px"";
            style.width = headerSize.cx + ""px"";
            style.height = headerSize.cy + ""px"";
            group.left = headerPos.left;
            group.top = headerPos.top;
        }
    },

    updateHeaders: function ScrollView_updateHeaders(fragment, newHeaders, begin, end) {
        var headerSize = this.site._getHeaderContentSize(),
            that = this;

        function updateGroup(index) {
            var group = that.groups.group(index);
            if (group) {
                if (group.element) {
                    that.updateHeader(group, index, headerSize);
                } else {
                    that.addHeader(fragment, newHeaders, index, headerSize);
                }
            }
        }

        var groupIndex = this.groups.groupFromItem(begin),
            groupEnd = this.groups.groupFromItem(end);
        if (groupIndex !== null) {
            do { if (groupEnd !== null) { } else { assertionFailed(""groupEnd !== null"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 7848); } } while (false);
            groupEnd++;

            if (groupIndex > 0) {
                updateGroup(0);
            }

            for (; groupIndex < groupEnd; groupIndex++) {
                updateGroup(groupIndex);
            }
        }
    },

    // This function removes items which are outside of current viewport and prefetched area
    purgeItems: function ScrollView_purgeItems() {
        var canvas = this.site._getCanvas(),
            im = this.site._getItemsManager(),
            that = this;

        var toDelete = [];
        this.items.each(function (index, item) {
            if (index !== 0 && (index < that.begin || index >= that.end)) {
                im.releaseItem(item);
                canvas.removeChild(item);
                toDelete.push(index);
            }
        });
        this.items.deleteItems(toDelete);

        var beginGroup = this.groups.groupFromItem(this.begin);
        if (beginGroup !== null) {
            var endGroup = this.groups.groupFromItem(this.end) + 1;
            for (var i = 1, len = this.groups.groups.length; i < len; i++) {
                var group = this.groups.groups[i];
                if (group.element && (i < beginGroup || i >= endGroup)) {
                    canvas.removeChild(group.element);
                    delete group.element;
                    delete group.left;
                    delete group.top;
                }
            }
        }
    },

    realizePage: function ScrollView_realizePage(scrollbarPos, realizePageEndedCallback) {
        var that = this,
            currentPass = ++this.realizePass,
            itemsManager = this.site._getItemsManager();

        function pageRealized() {
            that.site._setViewState(READY);

            if (realizePageEndedCallback) {
                realizePageEndedCallback();
            }
        }

        this.site._setViewState(INITIALIZED);
        this.showProgressBar();
        this.site._itemsCount(function (count) {
            if (!that.destroyed) {
                if (count !== 0) {
                    // If the application developer didn't specify item size we need
                    // to get it from template before calling layout manager
                    that.site._updateItemSize(count, function (success) {
                        if (success) {
                            that.layout.update(count);
                            that.groups.dirty = true;

                            if (!that.site._usingChildNodes) {
                                // Items to realize are determined on the basis of scrollbar position and viewport size
                                var viewportLength = that.site._getViewportLength();
                                that.begin = that.layout.calcFirstDisplayedItem(Math.max(0, scrollbarPos - viewportLength), false, that.groups);
                                that.end = that.layout.calcLastDisplayedItem(scrollbarPos + viewportLength, false, that.groups) + 1;

                                that.begin = Math.max(0, that.begin);
                                that.end = Math.min(count, that.end);
                            } else {
                                that.begin = 0;
                                that.end = count;
                            }

                            if (that.begin < that.end) {
                                var newHeaders = [], newItems = [];

                                that.realizeItems(that.site._getCanvas(), newItems, newHeaders, that.begin, that.end, count, currentPass, function () {
                                    if (that.realizePass === currentPass) {

                                        that.groups.setHeaders(newHeaders);
                                        newHeaders = [];

                                        that.updateHeaders(that.site._getCanvas(), newHeaders, that.begin, that.end);
                                        that.groups.setHeaders(newHeaders);

                                        that.updateScrollbar();

                                        that.items.setItems(newItems);
                                        that.newItems = that.newItems || (newItems.length > 0);

                                        do { if (that.validate()) { } else { assertionFailed(""that.validate()"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 7947); } } while (false);

                                        if (that.groups.pinItem) {
                                            that.groups.pinItem(that.begin, that.layout.calcItemLtrPosition(that.begin, that.groups.groupFromItem(that.begin), that.groups));
                                        }

                                        itemsManager.prioritize(that.items.itemAt(that.begin), that.items.itemAt(that.end - 1), Win.UI.Priority.high);

                                        that.hideProgressBar();
                                        that.site._setViewState(REALIZED);

                                        if (!that.site._usingChildNodes) {
                                            // Items outside of current viewport and prefetched area can be removed  
                                            that.purgeItems();
                                        }

                                        pageRealized();
                                    }
                                });
                            } else {
                                that.updateScrollbar();
                                pageRealized();
                            }
                        } else {
                            pageRealized();
                        }
                    });
                } else {
                    that.hideProgressBar();
                    that.site._updateItemSize(count, function () {
                        that.layout.update(count);
                        that.groups.dirty = true;
                        that.updateScrollbar();
                        pageRealized();
                    });
                }
            }
        });
    },

    onScroll: function ScrollView_onScroll(scrollbarPos, scrollLength, viewportSize) {
        var that = this;

        this.realizePage(scrollbarPos, function () {
            // TODO: This is temporary workaround for the lack of virtual scrollbar range in M2. 
            if (scrollbarPos === 0) {
                // If the user hit the beginning of the list apply the fix immediately
                that.fixScrollbarRange(scrollbarPos);
            } else {
                // In the other case it can wait until the user stops scrolling
                that.deferScrollbarFix(function () {
                    that.fixScrollbarRange(scrollbarPos);
                });
            }
        });
    },

    onResize: function ScrollView_onResize(scrollbarPos, viewportSize) {
        this.realizePage(scrollbarPos);
    },

    reset: function ScrollView_reset(viewportSize) {
        this.items.removeItems();
        this.groups.removeGroups();
        utilities.empty(this.site._getCanvas());
        this.realizePage(0);
    },

    refresh: function ScrollView_refresh(scrollbarPos, scrollLength, viewportSize, newCount) {
        this.realizePage(scrollbarPos);
    },

    updateScrollbar: function ScrollView_updateScrollbar(absolute) {
        var canvasSize = this.layout.calcCanvasSize(this.groups, absolute),
            canvasStyle = this.site._getCanvas().style;
        canvasStyle.width = canvasSize.cx + ""px"";
        canvasStyle.height = canvasSize.cy + ""px"";
    },

    update: function ScrollView_update(count) {
        this.layout.update(count);
        that.groups.dirty = true;
    },

    updateLayout: function ScrollView_updateLayout(layout, groups) {

        this.layout = null;


        switch (layout) {
            case ""list"":
                this.layout = new ListLayout(this.site);
                break;

            case ""verticalgrid"":
                this.layout = new VerticalGroupedGridLayout(this.site);
                break;

            case ""horizontalgrid"":
                this.layout = new HorizontalGroupedGridLayout(this.site);
                break;
        }
        do { if (this.layout) { } else { assertionFailed(""this.layout"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 8049); } } while (false);
    },

    updateGroups: function ScrollView_updateGroups(groupByFunction, groupRenderer) {
        if (groupByFunction) {
            this.groups = new GroupsContainer(groupByFunction, groupRenderer);
        } else {
            this.groups = new NoGroups();
        }
    },


    validate: function ScrollView_validate() {
        var groupContainer = this.groups.groups;
        if (groupContainer && groupContainer.length > 0) {
            var calculatedOffset = groupContainer[0].offset;
            for (var i = 0; i < groupContainer.length; i++) {
                var group = groupContainer[i];
                do { if (calculatedOffset === group.offset) { } else { assertionFailed(""calculatedOffset === group.offset"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 8067); } } while (false);
                if (i + 1 < groupContainer.length) {
                    var nextGroup = groupContainer[i + 1];
                    do { if (group.startIndex < nextGroup.startIndex) { } else { assertionFailed(""group.startIndex < nextGroup.startIndex"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 8070); } } while (false);
                    calculatedOffset = group.offset + this.layout.getGroupSize(nextGroup.startIndex - group.startIndex, i);
                }
            }
        }
        var canvas = this.site._getCanvas();
        this.items.each(function (index, item) {
            do { if (item.parentNode === canvas || item.parentNode === null) { } else { assertionFailed(""item.parentNode === canvas || item.parentNode === null"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 8077); } } while (false);
        });
        return true;
    },


    fixScrollbarRange: function ScrollView_fixScrollbarRange(scrollbarPos) {
        var fixedPos = scrollbarPos;
        if (this.groups.length() && this.groups.group(0).offset) {

            this.updateScrollbar(true);

            var firstDisplayed = this.layout.calcFirstDisplayedItem(scrollbarPos, false, this.groups);
            this.groups.pinItem(firstDisplayed, this.layout.calcItemLtrPosition(firstDisplayed, this.groups.groupFromItem(firstDisplayed), this.groups, true));
            fixedPos = scrollbarPos - this.groups.group(0).offset;
        }

        if (fixedPos !== scrollbarPos) {
            this.realizePage(fixedPos);
            this.site._scrollbarPos(fixedPos);
        }
    },

    resetScrollbarFix: function ScrollView_resetScrollbarFix() {
        if (this.scrollbarFixTimer) {
            clearTimeout(this.scrollbarFixTimer);
            delete this.scrollbarFixTimer;
        }
    },

    deferScrollbarFix: function ScrollView_deferScrollbarFix(action) {
        this.resetScrollbarFix();

        var that = this;
        this.scrollbarFixTimer = thisWinUI.setTimeout(function () {
            delete that.scrollbarFixTimer;
            action();
        }, SCROLLBAR_RANGE_FIX_DELAY);
    },

    cleanUp: function ScrollView_cleanUp() {
        this.resetScrollbarFix();
        this.destroyed = true;
    }
};
// IncrementalView doesn't use virtualization. It creates all the items immediately but it creates 
// only a small set of items - a chunk. By default there are 50 items in a chunk. When the user 
// scrolls to the last item the next chunk of items is created.
function IncrementalView(scrollViewSite) {
    this.site = scrollViewSite;
    this.layout = new HorizontalGroupedGridLayout(scrollViewSite);
    this.layoutInitialized = false;
    this.usingGridLayout = true;
    this.items = new ItemsContainer(scrollViewSite);
    this.lastPageBegin = 0;
    this.lastItem = -1;
    this.loadingInProgress = false;
    this.newItems = false;
    this.initializeUIElements();
    
    var options = this.site._getOptions();
    this.pagesToLoad = options.pagesToLoad ? options.pagesToLoad : DEFAULT_PAGES_TO_LOAD;
    this.itemsToLoad = options.itemsToLoad ? options.itemsToLoad : DEFAULT_ITEMS_TO_LOAD;
    this.pageLoadThreshold = options.pageLoadThreshold ? options.pageLoadThreshold : DEFAULT_PAGE_LOAD_THRESHOLD;
    this.itemLoadThreshold = options.itemLoadThreshold ? options.itemLoadThreshold : DEFAULT_ITEM_LOAD_THRESHOLD;
    this.groups = new NoGroups();
    this.resetView();
}

IncrementalView.prototype = {
    addItem: function IncrementalView_addItem(fragment, newItems, itemIndex, finishCallback) {
        var that = this,
            im = this.site._getItemsManager();
        im.simplerItemAtIndex(itemIndex, function (item) {
            utilities.addClass(item, itemClass);
            item.setAttribute(""role"", that.site._itemRole);

            that.updateItem(item, itemIndex, item.msDataItem.dataObject);

            fragment.appendChild(item);
            newItems.push({
                index: itemIndex,
                element: item
            });

            finishCallback();
        });
    },

    initializeUIElements: function () {
        this.progressBar = document.createElement(""div"");
        utilities.addClass(this.progressBar, progressClass);
        this.progressBarContainer = document.createElement(""div"");
        utilities.addClass(this.progressBarContainer, progressContainerClass);
        this.progressBar.ignoreInDisplayedItems = true;
        this.progressBarContainer.ignoreInDisplayedItems = true;
        this.progressBarContainer.appendChild(this.progressBar);
    },

    pageUp: function IncrementalView_pageUp(currentFocus) {
        var scrollbarPos = this.site._scrollbarPos(),
            layout = this.layout,
            firstElementOnPage = layout.calcFirstDisplayedItem(scrollbarPos, true, this.groups);

        if (currentFocus !== firstElementOnPage) {
            return firstElementOnPage;
        }

        var offsetProp = this.site._horizontal() ? ""offsetWidth"" : ""offsetHeight"",
            currentItem = this.items.itemAt(currentFocus),
            newFocus = layout.calcFirstDisplayedItem(Math.max(0, scrollbarPos - this.site._getViewportLength() + (currentItem ? currentItem[offsetProp] : 0)), false, this.groups);

        // This check is necessary for items that are larger than the viewport
        newFocus = newFocus < currentFocus ? newFocus : currentFocus - 1;

        return newFocus;
    },

    pageDown: function IncrementalView_pageDown(currentFocus) {
        var scrollbarPos = this.site._scrollbarPos(),
            layout = this.layout,
            lastElementOnPage = layout.calcLastDisplayedItem(scrollbarPos, true, this.groups);

        if (currentFocus !== lastElementOnPage) {
            return lastElementOnPage;
        }

        var offsetProp = this.site._horizontal() ? ""offsetWidth"" : ""offsetHeight"",
            currentItem = this.items.itemAt(currentFocus),
            newFocus = layout.calcLastDisplayedItem(scrollbarPos + this.site._getViewportLength() - (currentItem ? currentItem[offsetProp] : 0), false, this.groups);

        // This check is necessary for items that are larger than the viewport
        if (newFocus > currentFocus) {
            newFocus = currentFocus + 1;
        }

        return newFocus;
    },

    updateItem: function IncrementalView_updateItem(item, itemIndex, dataObject) {
        var itemPos = this.layout.calcItemPosition(itemIndex, 0, this.groups);
        if (itemPos) {
            var style = item.style;
            style.position = ""absolute"";
            style.top = itemPos.top + ""px"";
            style.left = itemPos.left + ""px"";
            if (itemPos.width) {
                style.width = itemPos.width + ""px"";
            }
        }

        if (this.site._isSelected(itemIndex)) {
            utilities.addClass(item, selectedClass);
            item.setAttribute(""aria-selected"", true);
        }
    },

    realizeItems: function IncrementalView_realizeItem(fragment, newItems, itemIndex, end, finishCallback) {
        var counter = end - itemIndex;

        this.hideProgressBar();

        function callCallback() {
            if (--counter === 0) {
                finishCallback();
            }
        }

        if (counter !== 0) {
            for (; itemIndex < end; itemIndex++) {
                var item = this.items.itemAt(itemIndex);
                if (!item) {
                    this.addItem(fragment, newItems, itemIndex, callCallback);
                } else {
                    // Item already exists. Only position needs to be updated 
                    this.updateItem(item, itemIndex, item.msDataItem.dataObject);
                    callCallback();
                }
            }
        } else {
            finishCallback();
        }
    },

    end: function IncrementalView_end(begin, count) {
        if (!this.layoutInitialized) {
            // Incremental mode needs to calculate the number of items to load based on a set number of pages.
            // The problem is, in order to calculate that, we need the layout to have been given item dimensions.
            // Since those dimensions are only set when update is called, and update is only called AFTER we have
            // our next end, we get around this with a layoutInitialized flag. Here the layout is updated with 0 items
            // just to get the item dimensions, at which point we can calculate the real endpoint and update appropriately.
            this.layoutInitialized = true;
            this.layout.update(0);
        }
        return this.site._usingChildNodes ?
            count :
            Math.min(count, this.layout.calcMaxItemsPerViewport ? begin + this.pagesToLoad * this.layout.calcMaxItemsPerViewport() : begin + this.itemsToLoad);
    },

    loadNextChunk: function IncrementalView_loadNextChunk(callback) {
        var that = this;
        this.site._itemsCount(function (count) {
            if (!that.destroyed) {
                if (count > that.lastItem + 1) {
                    // If the application developer didn't specify item size we need
                    // to get it from template before calling layout manager
                    that.site._updateItemSize(count, function (success) {
                        if (success) {
                            var fragment = document.createDocumentFragment(),
                            begin = that.lastItem + 1,
                            end = that.end(begin, count),
                            newItems = [];

                            that.layout.update(end);
                            // Realized items are inserted into document fragment and then 
                            // transferred into main document when all are ready
                            that.realizeItems(fragment, newItems, begin, end, function () {
                                that.site._getCanvas().appendChild(fragment);
                                that.items.setItems(newItems);
                                that.newItems = that.newItems || (newItems.length > 0);
                                that.lastPageBegin = begin;
                                that.lastItem = end - 1;
                                that.updateScrollbar();
                                if (that.site._rtl()) {
                                    for (var i = 0; i < end; i++) {
                                        var item = that.items.itemAt(i);
                                        that.updateItem(item, i, item.msDataItem.dataObject);
                                    }
                                }

                                callback();
                            });
                        } else {
                            callback();
                        }
                    });
                } else {
                    callback();
                }
            }
        });
    },

    updateItems: function IncrementalView_updateItems(callback) {
        var that = this;
        this.site._itemsCount(function (count) {
            if (!that.destroyed) {
                if (count !== 0) {
                    that.site._updateItemSize(count, function (success) {
                        if (success) {
                            that.layout.update(that.lastItem + 1);
                            that.updateScrollbar();

                            var fragment = document.createDocumentFragment(),
                                newItems = [];

                            that.realizeItems(fragment, newItems, 0, that.lastItem + 1, function () {
                                that.site._getCanvas().appendChild(fragment);
                                that.items.setItems(newItems);
                                that.newItems = that.newItems || (newItems.length > 0);
                                callback();
                            });
                        } else {
                            callback();
                        }
                    });
                } else {
                    callback();
                }
            }
        });
    },

    download: function IncrementalView_download(action, callback) {
        var that = this;

        // Setting ready state in incremental mode needs to be asynchronous to match up with ScrollView behavior
        function setReady() {
            window.setTimeout(function () {
                that.site._setViewState(READY);
            }, 0);
        }

        if (this.site._cachedCount === UNINITIALIZED || this.lastItem === UNINITIALIZED) {
            this.showProgressBar();
        }

        if (!this.loadingInProgress) {
            this.loadingInProgress = true;

            this.site._setViewState(INITIALIZED);

            action(function () {
                that.loadingInProgress = false;
                var scrollbarPos = that.site._scrollbarPos();

                if (that.site._cachedCount !== (that.lastItem + 1)) {
                    that.showProgressBar();
                }

                setReady();
            });
        }
    },

    showProgressBar: function IncrementalView_showProgressBar() {
        var canvas = this.site._getCanvas(),
            progressBarStyle = this.progressBar.style;
        if (this.lastItem === UNINITIALIZED) {
            progressBarStyle.position = ""absolute"";
            progressBarStyle.left = ""50%"";
            progressBarStyle.top = ""50%"";
            this.site._viewport.appendChild(this.progressBar);
        } else {
            if (this.usingGridLayout) {
                if (this.site._horizontal()) {
                    if (this.site._rtl()) {
                        progressBarStyle.right = ""100%"";
                        progressBarStyle.marginRight = -progressBarStyle.offsetWidth + ""px"";
                    } else {
                        progressBarStyle.left = ""100%"";
                        progressBarStyle.marginLeft = -progressBarStyle.offsetWidth + ""px"";
                    }
                } else {
                    progressBarStyle.top = ""100%"";
                    progressBarStyle.marginTop = -progressBarStyle.offsetHeight + ""px"";
                }
                canvas.appendChild(this.progressBar);
            } else {
                if (this.progressBar.parentNode !== this.progressBarContainer) {
                    if (this.progressBar.parentNode) {
                        this.progressBar.parentNode.removeChild(this.progressBar);
                    }
                    this.progressBarContainer.appendChild(this.progressBar);
                }
                canvas.appendChild(this.progressBarContainer);
            }
        }
    },

    hideProgressBar: function IncrementalView_hideProgressBar() {
        if (this.progressBarContainer.parentNode) {
            this.progressBarContainer.parentNode.removeChild(this.progressBarContainer);
        }

        if (this.progressBar.parentNode && this.progressBar.parentNode !== this.progressBarContainer) {
            this.progressBar.parentNode.removeChild(this.progressBar);
        }
    },

    scrollbarAtEnd: function IncrementalView_scrollbarAtEnd(scrollbarPos, scrollLength, viewportSize) {
        if (!this.layoutInitialized) {
            this.layoutInitialized = true;
            this.layout.update(0);
        }
        var lastDisplayedItem = this.layout.calcLastDisplayedItem(scrollbarPos, false, this.groups),
            countToEnd = Math.max(0, this.lastItem - lastDisplayedItem),
            itemsThreshold = this.usingGridLayout ? this.layout.calcMaxItemsPerViewport() * this.pageLoadThreshold : this.itemLoadThreshold;
        return countToEnd <= itemsThreshold;
    },

    updateLoadThreshold: function IncrementalView_updateLoadThreshold(pageThreshold, itemThreshold) {
        this.pageLoadThreshold = pageThreshold;
        this.itemLoadThreshold = itemThreshold;
        var site = this.site;
        this.onScroll(
            site._scrollbarPos(),
            site._viewport[site._scrollLength],
            site._getViewportSize()
        );
    },

    finalItem: function IncrementalView_finalItem(callback) {
        callback(this.lastItem);
    },

    onScroll: function IncrementalView_onScroll(scrollbarPos, scrollLength, viewportSize) {
        if (this.scrollbarAtEnd(scrollbarPos, scrollLength, viewportSize)) {
            this.download(this.loadNextChunk.bind(this));
        }
    },

    onResize: function IncrementalView_onResize(scrollbarPos, viewportSize) {
        this.download(this.updateItems.bind(this));
    },

    reset: function IncrementalView_reset(viewportSize) {
        this.lastItem = -1;
        this.items.removeItems();
        utilities.empty(this.site._getCanvas());

        this.download(this.loadNextChunk.bind(this));
    },

    resetView: function IncrementalView_resetView() {
        this.site._scrollbarPos(0);
    },

    refresh: function IncrementalView_refresh(scrollbarPos, scrollLength, viewportSize, newCount) {
        var that = this,
            end = this.end(this.lastPageBegin, newCount);

        this.lastItem = end - 1;
        that.updateScrollbar();

        var toDelete = [],
            canvas = this.site._getCanvas();

        this.items.each(function (index, item) {
            if ((index < 0) || (index > that.lastItem)) {
                canvas.removeChild(item);
                toDelete.push(index);
            }
        });
        this.items.deleteItems(toDelete);

        this.download(function (callback) {
            that.updateItems(function () {
                if (that.scrollbarAtEnd(scrollbarPos, scrollLength, viewportSize) && (newCount > end)) {
                    that.loadNextChunk(callback);
                } else {
                    callback(newCount);
                }
            });
        });
    },

    updateScrollbar: function IncrementalView_updateScrollbar() {
        var style = this.site._getCanvas().style;
        if (this.usingGridLayout) {
            var canvasSize = this.layout.calcCanvasSize(this.groups);
            style.width = canvasSize.cx + ""px"";
            style.height = canvasSize.cy + ""px"";
        } else {
            style.width = this.site._getViewportSize().cx + ""px"";
            style.height = 0;
        }
    },

    updateLayout: function IncrementalView_updateLayout(layout) {

        this.layout = null;


        switch (layout) {
            case ""list"":
                this.layout = new FlowLayout(this.site);
                this.usingGridLayout = false;
                break;

            case ""verticalgrid"":
                this.layout = new VerticalGroupedGridLayout(this.site);
                this.usingGridLayout = true;
                break;

            case ""horizontalgrid"":
                this.layout = new HorizontalGroupedGridLayout(this.site);
                this.usingGridLayout = true;
                break;
        }

        this.layoutInitialized = false;
        this.showProgressBar();
        do { if (this.layout) { } else { assertionFailed(""this.layout"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 8540); } } while (false);
    },

    cleanUp: function IncrementalView_cleanUp() {
        this.destroyed = true;
    }
};

// This component is responsible for handling input in Browse Mode. 
// When the user clicks on an item in this mode itemInvoked event is fired.
function BrowseMode(modeSite) {
    this.initialize(modeSite);
}

BrowseMode.prototype = {
    initialize: function (modeSite) {

        this.site = modeSite;
        this.pressedItem = null;
        this.pressedIndex = INVALID_INDEX;
        this.pressedPosition = null;
        this.name = ""browse"";

        this.keyboardNavigationHandlers = {};

        function createArrowHandler(direction) {
            return function (view, oldFocus) {
                return view.layout.getAdjacent(oldFocus, direction, view.groups);
            };
        }

        var Key = utilities.Key;
        this.keyboardNavigationHandlers[Key.upArrow] = createArrowHandler(UP);
        this.keyboardNavigationHandlers[Key.downArrow] = createArrowHandler(DOWN);
        this.keyboardNavigationHandlers[Key.leftArrow] = createArrowHandler(LEFT);
        this.keyboardNavigationHandlers[Key.rightArrow] = createArrowHandler(RIGHT);
        this.keyboardNavigationHandlers[Key.home] = function () {
            return 0;
        };
        this.keyboardNavigationHandlers[Key.pageUp] = function (view, oldFocus) {
            return view.pageUp(oldFocus);
        };
        this.keyboardNavigationHandlers[Key.pageDown] = function (view, oldFocus) {
            return view.pageDown(oldFocus);
        };
    },

    activate: function () {
        utilities.addClass(this.site._element, browseModeClass);
        this.site._selection.set([]);
    },

    deactivate: function () {
        utilities.removeClass(this.site._element, browseModeClass);
    },

    onMouseDown: function (eventObject) {
        if (eventObject.button === LEFT_MOUSE_BUTTON) {
            var site = this.site;

            this.pressedItem = site._itemFrom(eventObject.srcElement);
            if (this.pressedItem) {
                utilities.removeClass(this.pressedItem, hoverClass);
                utilities.addClass(this.pressedItem, pressedClass);
                this.pressedIndex = site.index(this.pressedItem);

                this.pressedPosition = {
                    x: eventObject.x,
                    y: eventObject.y
                };

                this.changeFocus(site._selection.getFocused(), this.pressedIndex);
            }
        }
    },

    onMouseOut: function (eventObject) {
        var fromItem = this.site._itemFrom(eventObject.fromElement);
        if ((this.pressedIndex === INVALID_INDEX) && fromItem) {
            utilities.removeClass(fromItem, hoverClass);
        }

        var newItem = this.site._itemFrom(eventObject.srcElement);
        if (this.pressedItem && (newItem !== this.pressedItem)) {
            utilities.removeClass(this.pressedItem, pressedClass);
            this.pressedItem = null;
        }
    },

    onMouseOver: function (eventObject) {
        var toItem = this.site._itemFrom(eventObject.toElement);
        if ((this.pressedIndex === INVALID_INDEX) && toItem) {
            utilities.addClass(toItem, hoverClass);
        }

        var newIndex = this.site.index(eventObject.srcElement);
        if ((newIndex !== INVALID_INDEX) && (newIndex === this.pressedIndex)) {
            this.pressedItem = this.site._itemAt(newIndex);
            utilities.addClass(this.pressedItem, pressedClass);
        }
    },

    onMouseUp: function (eventObject) {
        if (this.pressedItem) {
            this.fireInvokeEvent(this.pressedIndex);
            utilities.removeClass(this.pressedItem, pressedClass);

        }
        this.pressedItem = null;
        this.pressedIndex = INVALID_INDEX;
    },

    fireInvokeEvent: function (itemIndex) {
        var eventObject = document.createEvent(""Event"");
        eventObject.initEvent(""iteminvoked"", true, false);
        eventObject.itemIndex = itemIndex;
        this.pressedItem.dispatchEvent(eventObject);
    },

    onMouseMove: function (eventObject) {
        if (eventObject.button !== LEFT_MOUSE_BUTTON && this.pressedIndex !== INVALID_INDEX) {
            if (this.pressedItem) {
                utilities.removeClass(this.pressedItem, pressedClass);
            }
            this.pressedItem = null;
            this.pressedIndex = INVALID_INDEX;
        }

        if (this.pressedItem &&
            (Math.abs(this.pressedPosition.x - eventObject.x) > DRAG_START_THRESHOLD || Math.abs(this.pressedPosition.y - eventObject.y) > DRAG_START_THRESHOLD)) {

            var site = this.site,
                items = site._selection.get();

            if (items.length > 0) {
                items.sort(function (left, right) {
                    return left - right;
                });
                do { if (items.indexOf(this.pressedIndex) !== -1) { } else { assertionFailed(""items.indexOf(this.pressedIndex) !== -1"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 8678); } } while (false);
            } else {
                items = [this.pressedIndex];
            }

            var dragData = new DataTransfer();
            if (this.reorderSupported()) {
                dragData.setData(REORDER_FORMAT, {
                    uniqueID: site._element.uniqueID,
                    draggedItems: items
                });
            }

            var draggedItem = this.pressedItem,
                thumbnail = this.createThumbnail(draggedItem, items.length),
                offset = {
                    x: this.pressedPosition.x - this.pressedItem.offsetLeft,
                    y: this.pressedPosition.y - this.pressedItem.offsetTop
                };

            // This event gives the application a chance to insert data in a custom format to the dataTransfer object
            if (this.site._dragSupported()) {
                this.fireDragStartEvent(items, dragData, thumbnail, offset);
            }

            if (dragData.count() > 0) {

                utilities.removeClass(this.pressedItem, pressedClass);
                this.pressedItem = null;
                this.pressedIndex = INVALID_INDEX;

                site._viewport.setCapture();

                site._pushMode(new DragSourceMode(site, dragData, items, draggedItem, thumbnail, offset));
            }
        }
    },

    onDragStart: function (eventObject) {
        event.returnValue = false;
    },

    changeFocus: function (oldFocus, newFocus) {
        var site = this.site;

        function onItemsRealized(event) {
            if (site.viewState() === Win.UI.Controls.ListViewState.realized) {
                site._setupTabOrder();
                site._setFocusOnItem(newFocus);
                site.removeEventListener(""viewstatechanged"", onItemsRealized, false);
            }
        }

        site._setupTabOrder();
        site._unsetFocusOnItem(oldFocus);
        site._hasKeyboardFocus = true;
        site._selection.setFocused(newFocus);
        site.addEventListener(""viewstatechanged"", onItemsRealized, false);
        site.ensureVisible(newFocus);
        site._setupTabOrder();
        site._setFocusOnItem(newFocus);
    },

    onKeyDown: function (eventObject) {
        var view = this.site._view,
           oldFocus = this.site._selection.getFocused(),
           newFocus = oldFocus,
           that = this,
           handled = true;

        function setNewFocus() {
            // We need to get the final item in the view so that we don't try setting focus out of bounds.
            view.finalItem(function (maxIndex) {
                // Since getAdjacent is purely geometry oriented, it can return us out of bounds numbers, so this check is necessary
                if (newFocus < 0) {
                    return;
                }
                newFocus = Math.min(maxIndex, newFocus);
                if (oldFocus !== newFocus) {
                    that.changeFocus(oldFocus, newFocus);
                }
            });
        }

        var Key = utilities.Key,
            keyCode = eventObject.keyCode;

        if (this.keyboardNavigationHandlers[keyCode]) {
            newFocus = this.keyboardNavigationHandlers[keyCode](view, oldFocus);
            setNewFocus();
            // The end key is a special case that depends on an async operation before it knows where to go, so it is extracted out of the keyboardNavigationHandlers array
        } else if (keyCode === Key.end) {
            // The two views need to treat their ends a bit differently. scrollView is virtualized and will allow one to jump
            // to the end of the list, but incrementalview requires that the item be loaded before it can be jumped to.
            // Due to that limitation, we need to ask the view what its final item is and jump to that. The incremental view
            // will give the final loaded item, while the scrollview will give count - 1.
            view.finalItem(function (maxIndex) {
                newFocus = maxIndex;
                setNewFocus();
            });
        } else if (keyCode === Key.enter) {
            // Todo: Evaluate whether or not this needs some sort of visual for invoking via enter
            this.pressedItem = this.site._itemAt(oldFocus);
            this.fireInvokeEvent(oldFocus);
            this.pressedItem = null;
        } else if (keyCode === Key.F2 && this.reorderSupported()) {
            this.site._pushMode(new KeyboardReorderMode(this.site, [newFocus], newFocus, this.createThumbnail(this.site._itemAt(newFocus), 1)));
        } else {
            handled = false;
        }

        if (handled) {
            eventObject.stopPropagation();
            eventObject.preventDefault();
        }
    },

    fireDragStartEvent: function (items, dragData, thumbnail, offset) {
        var eventObject = document.createEvent(""Event"");
        eventObject.initEvent(""dragitemsstart"", true, false);
        eventObject.thumbnail = thumbnail;
        eventObject.thumbnailOffset = offset;
        eventObject.items = items;
        eventObject.dragData = dragData;
        this.site._element.dispatchEvent(eventObject);
    },

    reorderSupported: function () {
        return this.site._options.reorder && !this.site._options.groupByFunction && this.site._dragSupported();
    },

    createThumbnail: function (dragged, count) {
        var element = document.createElement(""div""),
            style = element.style;

        style.position = ""absolute"";
        style.left = dragged.offsetLeft + ""px"";
        style.top = dragged.offsetTop + ""px"";
        style.width = utilities.totalWidth(dragged) + ""px"";
        style.height = utilities.totalHeight(dragged) + ""px"";
        utilities.addClass(element, draggedItemClass);

        var clone = dragged.cloneNode(true);
        style = clone.style;
        style.left = style.top = 0;
        utilities.removeClass(clone, pressedClass);
        utilities.removeClass(clone, selectedClass);
        utilities.removeClass(clone, hoverClass);
        element.appendChild(clone);

        if (count > 1) {
            var number = document.createElement(""div"");
            number.innerText = count;
            utilities.addClass(number, draggedNumberClass);
            element.appendChild(number);
        }

        return element;
    }
};

function DataTransfer() {
    this.formatsMap = {};
    this.dropEffect = ""move"";
}

DataTransfer.prototype = {

    setData: function DataTransfer_setData(format, data) {
        this.formatsMap[format] = data;
    },

    getData: function DataTransfer_getData(format) {
        return this.formatsMap[format];
    },

    count: function DataTransfer_count() {
        return Object.keys(this.formatsMap).length;
    }
};
// ListView switches to this interaction mode when the user starts drag something in this ListView.
// This mode calls methods of the drag target interface in a response to the mouse input.

function DragSourceMode(modeSite, dragData, items, draggedItem, thumbnail, offset) {
    do { if (items.length > 0) { } else { assertionFailed(""items.length > 0"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 8862); } } while (false);
    do { if (dragData.count() >= 1) { } else { assertionFailed(""dragData.count() >= 1"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 8863); } } while (false);

    this.site = modeSite;
    this.previousPosition = { x: 0, y: 0 };
    this.items = items;
    this.dragData = dragData;
    this.thumbnail = thumbnail;
    this.thumbnailOffset = offset;
    this.viewportOffset = utilities.position(this.site._viewport);
    this.target = INVALID_INDEX;

    if (dragData.count() > 1 || !dragData.getData(REORDER_FORMAT)) {
        this.targets = this.getTargets();
    } else {
        do { if (dragData.count() === 1 && !!dragData.getData(REORDER_FORMAT)) { } else { assertionFailed(""dragData.count() === 1 && !!dragData.getData(REORDER_FORMAT)"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 8877); } } while (false);
        this.targets = [{
            element: this.site._element,
            position: utilities.position(this.site._element)
        }];
    }

    document.body.appendChild(this.thumbnail);
}

DragSourceMode.prototype = {
    activate: function () {
    },

    deactivate: function () {
    },

    onLoseCapture: function (eventObject) {
        document.body.removeChild(this.thumbnail);

        this.callDragHandler(""onDragLeave"", eventObject);
        this.target = INVALID_INDEX;

        this.site._popMode();

        var view = this.site._view;
        view.items.setLayoutIndices({});
        view.refresh(this.site._scrollbarPos());

        this.fireDragEndEvent();
    },

    onMouseUp: function (eventObject) {
        this.callDragHandler(""onDrop"", eventObject);
        this.target = INVALID_INDEX;

        document.releaseCapture();
    },

    onMouseMove: function (eventObject) {
        if (this.previousPosition.x !== eventObject.x || this.previousPosition.y !== eventObject.y) {

            this.previousPosition = {
                x: eventObject.x,
                y: eventObject.y
            };

            var cursorPosition = {
                x: this.viewportOffset.left + eventObject.offsetX,
                y: this.viewportOffset.top + eventObject.offsetY
            };

            var style = this.thumbnail.style;
            style.left = cursorPosition.x - this.thumbnailOffset.x + ""px"";
            style.top = cursorPosition.y - this.thumbnailOffset.y + ""px"";

            var newTarget = this.targetFromPosition(cursorPosition);
            if (this.target !== newTarget) {
                this.callDragHandler(""onDragLeave"", eventObject);

                this.target = newTarget;
                this.dragData.dropEffect = this.target !== INVALID_INDEX ? ""copy"" : ""none"";

                this.callDragHandler(""onDragEnter"", eventObject);
            }

            this.callDragHandler(""onDragOver"", eventObject);

            if (this.dragData.dropEffect === ""move"" && !this.site._options.groupByFunction) {
                var view = this.site._view,
                    layoutToData = createLayoutToDataMap(view, this.items),
                    dataIndexToLayoutIndex = {};
                
                for (var i = 0, len = this.items.length; i < len; i++) {
                    dataIndexToLayoutIndex[this.items[i]] = INVALID_INDEX;
                }
                
                view.items.setLayoutIndices(getDataToLayout(layoutToData, dataIndexToLayoutIndex));
                view.refresh(this.site._scrollbarPos());
            }

            if (this.cursorOwner) {
                this.cursorOwner.style.cursor = this.previousCursor;
            }
            this.cursorOwner = eventObject.srcElement;
            style = this.cursorOwner.style;
            this.previousCursor = style.cursor;

            style.cursor = this.dragData.dropEffect === ""none"" ? ""no-drop"" : ""default"";

            this.fireDragEvent();
        }
    },

    callDragHandler: function (method, eventObject) {
        if (this.target !== INVALID_INDEX) {
            var targetObject = this.targets[this.target];

            eventObject.cursorPosition = {
                x: this.viewportOffset.left + eventObject.offsetX,
                y: this.viewportOffset.top + eventObject.offsetY
            };
            eventObject.cursorPosition.x -= targetObject.position.left;
            eventObject.cursorPosition.y -= targetObject.position.top;
            eventObject.dragData = this.dragData;

            if (targetObject.element[DRAG_TARGET_EXPANDO][method]) {
                targetObject.element[DRAG_TARGET_EXPANDO][method](eventObject);
            }
        }
    },

    targetFromPosition: function (cursor) {
        for (var i = 0, len = this.targets.length; i < len; i++) {
            var target = this.targets[i].position;
            if (cursor.x >= target.left && cursor.x < target.left + target.width &&
                cursor.y >= target.top && cursor.y < target.top + target.height) {
                return i;
            }
        }
        return -1;
    },

    getTargets: function () {
        var targets = [];

        for (var element = document.body, elementPrev = null;
             elementPrev !== document.body || element !== elementPrev.nextSibling;
             element = element || elementPrev.nextSibling) {

            if (element) {

                if (element[DRAG_TARGET_EXPANDO]) {
                    targets.unshift({
                        element: element,
                        position: utilities.position(element)
                    });
                }

                elementPrev = element;
                element = element.firstChild;

            } else {
                elementPrev = elementPrev.parentNode;
            }
        }

        return targets;
    },

    onDataChanged: function () {
        document.releaseCapture();
    },

    fireDragEvent: function () {
        var eventObject = document.createEvent(""Event"");
        eventObject.initEvent(""dragitems"", true, false);
        eventObject.items = this.items;
        eventObject.dragData = this.dragData;
        this.site._element.dispatchEvent(eventObject);
    },

    fireDragEndEvent: function () {
        var eventObject = document.createEvent(""Event"");
        eventObject.initEvent(""dragitemsend"", true, false);
        eventObject.items = this.items;
        eventObject.dragData = this.dragData;
        this.site._element.dispatchEvent(eventObject);
    },

    onKeyDown: function (eventObject) {
        if (event.keyCode === utilities.Key.escape) {
            document.releaseCapture();
        }
    }
};

// This component implements the target side of drag and drop operations. Its methods are called by DragSourceMode from source ListView.

function DragTargetHandler(site) {
    this.site = site;
}

DragTargetHandler.prototype = {
    onDragEnter: function (eventObject) {
        var dragEnterEvent = this.fireDragEnterEvent(eventObject.srcElement, eventObject.dragData),
            reorderData = eventObject.dragData.getData(REORDER_FORMAT);

        if (dragEnterEvent.count && this.site._dragSupported()) {
            this.dragMode = new CustomDragMode(this.site, dragEnterEvent.count);
        } else if (reorderData && reorderData.uniqueID === this.site._element.uniqueID) {
            this.dragMode = new ReorderMode(this.site, reorderData.draggedItems);
        } else {
            delete this.dragMode;
        }

        if (this.dragMode) {
            this.translateCursorPosition(eventObject);
            this.dragMode.onDragEnter(eventObject);
        }
    },

    onDragOver: function (eventObject) {
        if (this.dragMode) {
            this.cursorPosition = eventObject.cursorPosition;

            if (this.inScrollZone(this.cursorPosition)) {
                this.startAutoScroll();
            } else {
                this.stopAutoScroll();
            }

            this.translateCursorPosition(eventObject);
            this.dragMode.onDragOver(eventObject);
        }
    },

    onDragLeave: function (eventObject) {
        if (this.dragMode) {
            this.stopAutoScroll();
            this.translateCursorPosition(eventObject);
            this.dragMode.onDragLeave(eventObject);
        }
        delete this.dragMode;
    },

    onDrop: function (eventObject) {
        if (this.dragMode) {
            this.stopAutoScroll();
            this.translateCursorPosition(eventObject);
            this.dragMode.onDrop(eventObject);
        }
        delete this.dragMode;
    },

    onDataChanged: function () {
        if (this.dragMode) {
            this.stopAutoScroll();
        }
        delete this.dragMode;
    },

    translateCursorPosition: function (eventObject) {
        var site = this.site,
            scrollPos = site._scrollbarPos();
        
        if (site._rtl() && site._horizontal()) {
            scrollPos = site._viewport[site._scrollLength] - scrollPos - site._viewport.offsetWidth;
        }

        var newCursorPositon = site._horizontal() ? {
            x: eventObject.cursorPosition.x + scrollPos,
            y: eventObject.cursorPosition.y
        } : {
            x: eventObject.cursorPosition.x,
            y: eventObject.cursorPosition.y + scrollPos
        };

        eventObject.cursorPosition = newCursorPositon;
    },

    inScrollZone: function (position) {
        var viewportSize = this.site._getViewportSize();

        if (this.site._horizontal()) {
            return position.x < AUTOSCROLL_THRESHOLD || position.x > (viewportSize.cx - AUTOSCROLL_THRESHOLD);
        } else {
            return position.y < AUTOSCROLL_THRESHOLD || position.y > (viewportSize.cy - AUTOSCROLL_THRESHOLD);
        }
    },

    autoScroll: function () {
        if (this.inScrollZone(this.cursorPosition)) {
            var site = this.site,
                scrollDelta = AUTOSCROLL_DELTA * (this.cursorPosition[site._horizontal() ? ""x"" : ""y""] < AUTOSCROLL_THRESHOLD ? -1 : 1);
            if (site._horizontal() && site._rtl()) {
                scrollDelta = -scrollDelta;
            }
            site._scrollbarPos(site._scrollbarPos() + scrollDelta);
        }
    },

    startAutoScroll: function () {
        if (this.autoScrollTimer === undefined) {
            var that = this;
            this.autoScrollTimer = setInterval(function () {
                that.autoScroll();
            }, AUTOSCROLL_INTERVAL);
        }
    },

    stopAutoScroll: function () {
        if (this.autoScrollTimer !== undefined) {
            clearTimeout(this.autoScrollTimer);
            delete this.autoScrollTimer;
        }
    },

    fireDragEnterEvent: function (srcElement, dragData) {
        var eventObject = document.createEvent(""Event"");
        eventObject.initEvent(""dragitemsenter"", true, false);
        eventObject.dragSource = srcElement;
        eventObject.dragData = dragData;

        this.site._element.dispatchEvent(eventObject);

        return eventObject;
    }
};function ReorderMode(modeSite, draggedItems) {
    this.site = modeSite;
    this.draggedItems = draggedItems;
}

ReorderMode.prototype = {

    onDragEnter: function (eventObject) {
    },

    onDragOver: function (eventObject) {
        var view = this.site._view,
            insertIndex = view.layout.hitTest(eventObject.cursorPosition.x, eventObject.cursorPosition.y, view.groups);

        if (insertIndex !== INVALID_INDEX && insertIndex !== this.currentIndex) {
            this.currentIndex = insertIndex;

            var layoutToData = createLayoutToDataMap(view, this.draggedItems),
                dataIndexToLayoutIndex = insertDraggedItems(layoutToData, this.currentIndex, this.draggedItems);

            view.items.setLayoutIndices(dataIndexToLayoutIndex);
            view.refresh(this.site._scrollbarPos());
        }
    },

    onDragLeave: function (eventObject) {
        var view = this.site._view;
        view.items.setLayoutIndices({});
        view.refresh(this.site._scrollbarPos());
    },

    onDrop: function (eventObject) {
        var view = this.site._view,
            insertIndex = view.layout.hitTest(eventObject.cursorPosition.x, eventObject.cursorPosition.y, view.groups);

        view.items.setLayoutIndices({});
        view.refresh(this.site._scrollbarPos());

        reorderItems(this.site, insertIndex, this.draggedItems);
    }
};

function createLayoutToDataMap(view, draggedItems) {
    var layoutToData = [],
        begin = view.begin,
        end = view.end,
        len,
        i;
    
    for (i = begin; i <= end; i++) {
        layoutToData[i] = i;
    }

    for (i = 0, len = draggedItems.length; i < len; i++) {
        var n = layoutToData.indexOf(draggedItems[i]);
        if (n !== -1) {
            layoutToData.splice(n, 1);
        }
    }

    return layoutToData;
}

function getDataToLayout(layoutToData, dataIndexToLayoutIndex) {
    dataIndexToLayoutIndex = dataIndexToLayoutIndex || {};

    for (var i = 0, len = layoutToData.length; i < len; i++) {
        var n = layoutToData[i];
        if (n !== INVALID_INDEX) {
            dataIndexToLayoutIndex[n] = i;
        }
    }

    return dataIndexToLayoutIndex;
}

function insertDraggedItems(layoutToData, insertIndex, draggedItems) {
    var dataIndexToLayoutIndex = {};
    for (var i = 0, len = draggedItems.length; i < len; i++) {
        var n = draggedItems[i];
        layoutToData.splice(insertIndex++, 0, INVALID_INDEX);
        dataIndexToLayoutIndex[n] = INVALID_INDEX;
    }

    return getDataToLayout(layoutToData, dataIndexToLayoutIndex);
}

function reorderItems(site, insertIndex, draggedItems) {
    var layoutToData = createLayoutToDataMap(site._view, draggedItems);
    
    insertIndex = layoutToData[insertIndex];
    insertIndex = insertIndex < site._cachedCount ? insertIndex : INVALID_INDEX;

    var eventObject = document.createEvent(""Event"");
    eventObject.initEvent(""itemsmoved"", true, true);
    eventObject.index = insertIndex;
    eventObject.items = draggedItems;
    if (site._element.dispatchEvent(eventObject)) {
        site.beginEdits();
        for (var i = 0, len = draggedItems.length; i < len; i++) {
            site.moveItem(draggedItems[i], insertIndex);
        }
        site.endEdits();
    }
}
function KeyboardReorderMode(modeSite, draggedItems, cursor, thumbnail) {
    this.site = modeSite;
    this.draggedItems = draggedItems;
    this.cursor = cursor;
    this.thumbnail = thumbnail;

    this.site._canvas.appendChild(this.thumbnail);

    this.moveCursor(this.cursor);
}

KeyboardReorderMode.prototype = {

    moveCursor: function (newPosition) {
        var insertIndex = Math.max(0, Math.min(this.site._cachedCount - 1, newPosition));
        this.cursor = insertIndex;

        var view = this.site._view,
            layoutToData = createLayoutToDataMap(view, this.draggedItems),
            dataIndexToLayoutIndex = insertDraggedItems(layoutToData, this.cursor, this.draggedItems);

        view.items.setLayoutIndices(dataIndexToLayoutIndex);

        var layout = view.layout,
            newScrollbarPos = layout.ensureVisible(
            this.site._scrollbarPos(),
            this.cursor,
            view.groups);
        if (newScrollbarPos !== this.site._scrollbarPos()) {
            this.site._scrollbarPos(newScrollbarPos);
        } else {
            view.refresh(this.site._scrollbarPos());
        }

        var cursorPos = layout.calcItemPosition(this.cursor, view.groups.groupFromItem(this.cursor), view.groups),
            style = this.thumbnail.style;
        style.left = cursorPos.left + ""px"";
        style.top = cursorPos.top + ""px"";
    },

    reset: function () {
        var view = this.site._view;

        view.items.setLayoutIndices({});
        view.refresh(this.site._scrollbarPos());

        this.site._canvas.removeChild(this.thumbnail);

        this.site._popMode();
    },

    apply: function () {
        this.reset();

        reorderItems(this.site, this.cursor, this.draggedItems);
    },

    onDataChanged: function () {
        this.reset();
    },

    onMouseDown: function (eventObject) {
        this.apply();
    },

    onKeyDown: function (eventObject) {
        var Key = utilities.Key,
            layout = this.site._view.layout,
            groups = this.site._view.groups,
            handled = true;

        switch (eventObject.keyCode) {
            case Key.escape:
                this.reset();
                break;
            case Key.enter:
                this.apply();
                break;
            case Key.space:
                this.apply();
                break;
            case Key.leftArrow:
                this.moveCursor(layout.getAdjacent(this.cursor, LEFT, groups));
                break;
            case Key.rightArrow:
                this.moveCursor(layout.getAdjacent(this.cursor, RIGHT, groups));
                break;
            case Key.upArrow:
                this.moveCursor(layout.getAdjacent(this.cursor, UP, groups));
                break;
            case Key.downArrow:
                this.moveCursor(layout.getAdjacent(this.cursor, DOWN, groups));
                break;
            default:
                handled = false;
        }

        if (handled) {
            eventObject.stopPropagation();
            eventObject.preventDefault();
        }
    }
};
// This component implements the drag target and it is used when data in a custom format is dragged 

function CustomDragMode(modeSite, count) {
    this.site = modeSite;
    this.count = count;
}

CustomDragMode.prototype = {
    onDragEnter: function (eventObject) {
    },

    onDragOver: function (eventObject) {
        var view = this.site._view,
            insertIndex = view.layout.hitTest(eventObject.cursorPosition.x, eventObject.cursorPosition.y, view.groups);

        if (insertIndex !== INVALID_INDEX && insertIndex !== this.currentIndex) {
            this.currentIndex = insertIndex;

            var layoutToData = createLayoutToDataMap(view, []);

            for (var i = 0, len = this.count; i < len; i++) {
                layoutToData.splice(insertIndex++, 0, INVALID_INDEX);
            }

            view.items.setLayoutIndices(getDataToLayout(layoutToData));
            view.refresh(this.site._scrollbarPos());
        }
    },

    onDragLeave: function (eventObject) {
        var view = this.site._view;
        view.items.setLayoutIndices({});
        view.refresh(this.site._scrollbarPos());
    },

    onDrop: function (eventObject) {
        var view = this.site._view,
            insertIndex = view.layout.hitTest(eventObject.cursorPosition.x, eventObject.cursorPosition.y, view.groups);

        view.items.setLayoutIndices({});
        view.refresh(this.site._scrollbarPos());

        this.fireDropEvent(insertIndex < this.site._cachedCount ? insertIndex : INVALID_INDEX, eventObject.dragData);
    },

    fireDropEvent: function (insertIndex, dragData) {
        var eventObject = document.createEvent(""Event"");
        eventObject.initEvent(""dropitems"", true, false);
        eventObject.index = insertIndex;
        eventObject.dragData = dragData;
        this.site._element.dispatchEvent(eventObject);
    }
};
// This component is responsible for holding selection state

function SelectionManager(site) {
    this.site = site;
    this.indices = {};
    this.focused = 0;
}

SelectionManager.prototype = {
    set: function (newSelection) {
        var oldIndices = {};
        for (var index in this.indices) {
            if (this.indices.hasOwnProperty(index)) {
                oldIndices[index] = true;
            }
        }

        var newIndices = {},
            selected = [];
        for (var i = 0, len = newSelection.length; i < len; i++) {
            index = newSelection[i];
            newIndices[index] = true;
            if (oldIndices[index]) {
                delete oldIndices[index];
            } else {
                selected[selected.length] = index;
            }
        }
        var unselected = [];
        for (index in oldIndices) {
            if (oldIndices.hasOwnProperty(index)) {
                unselected[unselected.length] = parseInt(index, 10);
            }
        }

        if ((unselected.length > 0) || (selected.length > 0)) {
            if (this.fireSelectionChanging(newSelection)) {
                this.indices = newIndices;
                this.site._updateSelection(unselected, selected);
                this.fireSelectionChanged();
            }
        }
    },

    isSelected: function (index) {
        return (this.indices[index] === true);
    },

    get: function () {
        var selection = [];
        for (var index in this.indices) {
            if (this.indices.hasOwnProperty(index)) {
                selection[selection.length] = parseInt(index, 10);
            }
        }
        return selection;
    },

    fireSelectionChanging: function (newSelection) {
        var eventObject = document.createEvent(""Event"");
        eventObject.initEvent(""selectionchanging"", true, false);
        eventObject.newSelection = newSelection;
        eventObject.allowed = true;
        this.site._element.dispatchEvent(eventObject);
        // TODO: Switch to preventDefault
        return eventObject.allowed;
    },

    fireSelectionChanged: function () {
        var eventObject = document.createEvent(""Event"");
        eventObject.initEvent(""selectionchanged"", true, false);
        this.site._element.dispatchEvent(eventObject);
    },

    getFocused: function () {
        return this.focused;
    },

    setFocused: function (index) {
        this.focused = index;
    },

    add: function (index) {
        var selection = this.get();
        if (selection.indexOf(index) === -1) {
            selection.push(index);
            this.set(selection);
        }
    },

    remove: function (index) {
        var selection = this.get(),
            filtered = selection.filter(function (value) {
                return value !== index;
            });
        this.set(filtered);
    }
};

// This component is responsible for handling input in SingleSelectionMode. 
// When the user clicks on an item in this mode selection state if item is toggled.

function SingleSelectionMode(modeSite) {
    this.initialize(modeSite);
}

SingleSelectionMode.prototype = utilities.extend({
    initialize: function (modeSite) {
        BrowseMode.prototype.initialize.call(this, modeSite);

        this.site = modeSite;
        this.name = ""singleselection"";
        this.previousState = false;
    },

    activate: function () {
        utilities.addClass(this.site._element, singleSelectionModeClass);
        this.site._selection.set([]);
    },

    deactivate: function () {
        utilities.removeClass(this.site._element, singleSelectionModeClass);
    },

    onMouseDown: function (eventObject) {
        if (eventObject.button === LEFT_MOUSE_BUTTON) {
            var clicked = this.site.index(eventObject.srcElement);
            if (clicked !== INVALID_INDEX) {
                this.previousState = this.site._selection.isSelected(clicked);
                if (!this.previousState) {
                    this.site._selection.set([clicked]);
                }
            }

            // Call onMouseDown in base class
            BrowseMode.prototype.onMouseDown.call(this, eventObject);
        }
    },

    onMouseUp: function (eventObject) {
        if (eventObject.button === LEFT_MOUSE_BUTTON) {
            var clicked = this.site.index(eventObject.srcElement);
            if (clicked !== INVALID_INDEX) {
                if (this.site._selection.isSelected(clicked) && this.previousState) {
                    this.site._selection.set([]);
                }
            }

            // Call onMouseUp in base class
            BrowseMode.prototype.onMouseUp.call(this, eventObject);
        }
    },

    canSelect: function (newSelection) {
        return (newSelection.length <= 1);
    },

    onKeyDown: function (eventObject) {
        var selection = this.site._selection,
            oldSelectedItem = selection.getFocused();
        BrowseMode.prototype.onKeyDown.call(this, eventObject);
        var newSelectedItem = selection.getFocused();
        if (oldSelectedItem !== newSelectedItem) {
            var selected = [newSelectedItem];
            selection.set(selected);
        }
    }
}, BrowseMode.prototype);
// This component is responsible for handling input in multi-selection mode

function MultiSelectionMode(modeSite) {
    this.initialize(modeSite);
}

MultiSelectionMode.prototype = utilities.extend({
    initialize: function (modeSite) {
        BrowseMode.prototype.initialize.call(this, modeSite);

        this.site = modeSite;
        this.name = ""multiselection"";
        this.previousState = false;
    },

    activate: function () {
        utilities.addClass(this.site._element, multiSelectionModeClass);
    },

    deactivate: function () {
        utilities.removeClass(this.site._element, multiSelectionModeClass);
    },

    onMouseDown: function (eventObject) {
        if (eventObject.button === LEFT_MOUSE_BUTTON) {
            var clicked = this.site.index(eventObject.srcElement);
            if (clicked !== INVALID_INDEX) {
                if (eventObject.shiftKey) {
                    var selection = [],
                        focused = this.site._selection.getFocused();
                    for (var i = Math.min(clicked, focused), to = Math.max(clicked, focused); i <= to; i++) {
                        selection[selection.length] = i;
                    }
                    this.site._selection.set(selection);
                    this.previousState = false;
                } else {
                    // Selection state of the clicked item is toggled 
                    this.site._selection.setFocused(clicked);
                    this.previousState = this.site._selection.isSelected(clicked);
                    if (!this.previousState) {
                        this.site._selection.add(clicked);
                    }
                }
            }

            // Call onMouseDown in base class
            BrowseMode.prototype.onMouseDown.call(this, eventObject);
        }
    },

    onMouseUp: function (eventObject) {
        if (eventObject.button === LEFT_MOUSE_BUTTON) {
            var clicked = this.site.index(eventObject.srcElement);
            if (clicked !== INVALID_INDEX && !eventObject.shiftKey) {
                if (this.site._selection.isSelected(clicked) && this.previousState) {
                    this.site._selection.remove(clicked);
                }
            }

            // Call onMouseUp in base class
            BrowseMode.prototype.onMouseUp.call(this, eventObject);
        }
    },

    canSelect: function (newSelection) {
        return true;
    },

    onKeyDown: function (eventObject) {
        BrowseMode.prototype.onKeyDown.call(this, eventObject);

        if (eventObject.keyCode === utilities.Key.space) {
            var selection = this.site._selection,
                focusedItem = selection.getFocused();
            if (selection.isSelected(focusedItem)) {
                selection.remove(focusedItem);
            } else {
                selection.add(focusedItem);
            }

            eventObject.stopPropagation();
            eventObject.preventDefault();
        }
    }

}, BrowseMode.prototype);
// ListView implementation

var numberRE = /^-?\d+/i;

// default renderer for Listview
function trivialHtmlRenderer(getIndex, key, dataObject, itemID) {
    return dataObject;
}

function StaticMode(modeSite) {
    this.site = modeSite;
    this.name = ""static"";
}

StaticMode.prototype = {
    activate: function () {
        var site = this.site;
        utilities.addClass(site._element, staticModeClass);
        site._selection.set([]);
        site._unsetFocusOnItem(site._selection.getFocused());
        site._selection.setFocused(0);
        site._hasKeyboardFocus = false;
    },

    deactivate: function () {
        utilities.removeClass(this.site._element, staticModeClass);
    }
};

function validateOptions(options) {
    var validators = {
        layout: function (value) {
            if (typeof value === ""string"") {
                if (value.match(/^(verticalgrid|horizontalgrid|list)$/)) {
                    return value;
                }
            }
            throw new Error(layoutIsInvalid);
        },

        mode: function (value) {
            if (typeof value === ""string"") {
                if (value.match(/^(static|browse|singleselection|multiselection)$/)) {
                    return value;
                }
            }
            throw new Error(modeIsInvalid);
        },

        loadingBehavior: function (value) {
            if (typeof value === ""string"") {
                if (value.match(/^(incremental|randomaccess)$/)) {
                    return value;
                }
            }
            throw new Error(loadingBehaviorIsInvalid);
        },
        pagesToLoad: function (value) {
            if ((typeof value === ""number"") && (value > 0)) {
                return value;
            }
            throw new Error(pagesToLoadIsInvalid);
        },

        itemsToLoad: function (value) {
            if ((typeof value === ""number"") && (value > 0)) {
                return value;
            }
            throw new Error(itemsToLoadIsInvalid);
        },
        pageLoadThreshold: function (value) {
            if ((typeof value === ""number"") && (value > 0)) {
                return value;
            }
            throw new Error(pageLoadThresholdIsInvalid);
        },
        itemLoadThreshold: function (value) {
            if ((typeof value === ""number"") && (value > 0)) {
                return value;
            }
            throw new Error(itemLoadThresholdIsInvalid);
        }
    };

    for (var fieldname in options) {
        if (validators[fieldname]) {
            options[fieldname] = validators[fieldname](options[fieldname]);
        }
    }

    return options;
}

Win.Namespace.defineWithParent(thisWinUI, ""Controls"", {
    ListView: Win.Class.define(null, {

        // Public methods

        options: function ListView_options(newOptions) {
            if (newOptions) {
                this._setOptions(validateOptions(newOptions));
            } else {
                return this._getOptions();
            }
        },

        item: function ListView_item(itemIndex) {
            return this._view.items.itemAt(itemIndex);
        },

        index: function ListView_index(item) {
            return this._view.items.index(item);
        },

        scrollTo: function ListView_scrollTo(itemIndex) {
            this._scrollbarPos(this._view.layout.scrollTo(itemIndex, this._view.groups));
        },

        ensureVisible: function ListView_ensureVisible(itemIndex) {
            this._scrollbarPos(this._view.layout.ensureVisible(
                this._scrollbarPos(),
                itemIndex,
                this._view.groups));
        },

        firstVisible: function ListView_firstVisible() {
            return this._view.layout.calcFirstDisplayedItem(
                this._scrollbarPos(),
                false,
                this._view.groups);
        },

        lastVisible: function ListView_lastVisible() {
            return Math.min(this._cachedCount, this._view.layout.calcLastDisplayedItem(
                this._scrollbarPos(),
                false,
                this._view.groups));
        },

        // We need to use a function instead of the HTML disabled attribute because onpropertychanged and DOMAttrModified
        // events aren't fired on element when this attribute is set to true, so the ListView doesn't have a chance
        // to execute code when this attribute is changed.
        disabled: function ListView_disabled(value) {
            if (value !== undefined) {
                var currentMode = this._currentMode();
                if (value) {
                    this._pushMode(new StaticMode(this));
                } else if (currentMode instanceof StaticMode) {
                    this._popMode();
                }
                this._element.disabled = value;
            } else {
                return this._element.disabled;
            }
        },

        addEventListener: function ListView_addEventListener(eventName, eventCallback, capture) {
            return this._element.addEventListener(eventName, eventCallback, capture);
        },

        removeEventListener: function ListView_removeEventListener(eventName, eventCallback, capture) {
            return this._element.removeEventListener(eventName, eventCallback, capture);
        },

        selection: function ListView_selection(newSelection) {
            var retVal;
            if (newSelection !== undefined) {
                if (!Array.isArray(newSelection)) {
                    newSelection = [newSelection];
                }
                this._setSelection(newSelection);
            } else {
                return this._selection.get();
            }
        },

        count: function ListView_count(callback) {
            return this._itemsManager.count(function (count) {
                callback.success(count);
            });
        },

        dataObject: function ListView_dataObject(itemIndex, callback) {
            if ((itemIndex >= 0) && (itemIndex < this._cachedCount)) {
                var something = this._itemsManager.simplerItemAtIndex(itemIndex, function (element) {
                    callback.success(element.msDataItem.dataObject);
                });
                if (!something && callback.error) {
                    callback.error(itemIndexIsInvalid);
                }
            } else if (callback.error) {
                callback.error(itemIndexIsInvalid);
            }
        },

        key: function ListView_key(itemIndex, callback) {
            if ((itemIndex >= 0) && (itemIndex < this._cachedCount)) {
                var something = this._itemsManager.simplerItemAtIndex(itemIndex, function (element) {
                    callback.success(element.msDataItem.key);
                });
                if (!something && callback.error) {
                    callback.error(itemIndexIsInvalid);
                }
            } else if (callback.error) {
                callback.error(itemIndexIsInvalid);
            }
        },

        indexFromKey: function ListView_indexFromKey(key, callback) {
            this._indexFromElement(this._itemsManager.itemFromKey(key), callback);
        },

        itemFromPrefix: function ListView_indexFromPrefix(prefix, callback) {
            this._indexFromElement(this._itemsManager.itemFromPrefix(prefix), callback);
        },

        beginEdits: function ListView_beginEdits() {
            this._itemsManager.listEditor().beginEdits();
        },

        insertItem: function ListView_insertItem(insertBefore, dataObject, callback) {
            var that = this,
                listEditor = this._itemsManager.listEditor();
            if (insertBefore < 0 || insertBefore >= this._cachedCount) {
                listEditor.insertAtEnd(null, dataObject, this._createListEditCallback(callback));
            } else {
                this._itemsManager.simplerItemAtIndex(insertBefore, function (nextElement) {
                    listEditor.insertBefore(null, dataObject, nextElement, that._createListEditCallback(callback));
                });
            }
        },

        updateItem: function ListView_updateItem(itemIndex, dataNew, callback) {
            var that = this,
                listEditor = this._itemsManager.listEditor();
            this._itemsManager.simplerItemAtIndex(itemIndex, function (element) {
                listEditor.change(element, dataNew, that._createListEditCallback(callback));
            });
        },

        moveItem: function ListView_moveItem(itemIndex, insertBefore, callback) {
            var that = this,
                listEditor = that._itemsManager.listEditor();
            this._itemsManager.simplerItemAtIndex(itemIndex, function (element) {
                if (insertBefore < 0 || insertBefore >= that._cachedCount) {
                    listEditor.moveToEnd(element, that._createListEditCallback(callback));
                } else {
                    that._itemsManager.simplerItemAtIndex(insertBefore, function (nextElement) {
                        listEditor.moveBefore(element, nextElement, that._createListEditCallback(callback));
                    });
                }
            });
        },

        deleteItem: function ListView_deleteItem(itemIndex, callback) {
            var that = this,
                listEditor = this._itemsManager.listEditor();
            this._itemsManager.simplerItemAtIndex(itemIndex, function (element) {
                listEditor.remove(element, that._createListEditCallback(callback));
            });
        },

        endEdits: function ListView_endEdits() {
            this._itemsManager.listEditor().endEdits();
        },

        viewState: function ListView_viewState() {
            return this._viewState;
        },

        // TODO: This is temporary interface added to enable testing. It will be re-designed in M3.
        group: function ListView_group(groupIndex) {
            return this._view.groups.group(groupIndex);
        },

        groupCount: function ListView_groupCount() {
            return this._view.groups.length();
        },

        // Private methods

        _setupInternalTree: function ListView_setupInternalTree() {
            if (this._rtl()) {
                utilities.addClass(this._element, rtlListViewClass);
            } else {
                utilities.removeClass(this._element, rtlListViewClass);
            }

            var childElements = utilities.children(this._element);
            utilities.empty(this._element);

            var viewportSize = this._getViewportSize();

            this._viewport = document.createElement(""div"");
            var viewportStyle = this._viewport.style;
            viewportStyle.overflowY = ""auto"";
            viewportStyle.overflowX = ""hidden"";
            viewportStyle.position = ""relative"";
            viewportStyle.left = 0;
            viewportStyle.top = 0;
            viewportStyle.width = viewportSize.cx + ""px"";
            viewportStyle.height = viewportSize.cy + ""px"";
            this._element.appendChild(this._viewport);

            this._canvas = document.createElement(""div"");
            this._canvas.onselectstart = function () { return false; };
            var canvasStyle = this._canvas.style;
            canvasStyle.position = ""relative"";
            canvasStyle.width = viewportSize.cx + ""px"";
            canvasStyle.height = viewportSize.cy + ""px"";

            this._viewport.appendChild(this._canvas);

            return childElements;
        },

        _setupTabOrder: function ListView_setupTabOrder() {
            if (this._view.newItems) {
                this._view.newItems = false;

                this._view.items.each(function (index, element) {
                    // We want to set a tabindex on the element we're given if it doesn't have one already so we can focus that element.
                    // Without explicitly making the div tabbable, calling focus() on it will do nothing and not give us a selection rect.
                    if (element.getAttribute(""tabindex"") === null) {
                        element.setAttribute(""tabindex"", 0);
                    }

                    utilities.disableTab(element);
                });

                var focusedIndex = this._selection.getFocused();
                if (focusedIndex >= this.firstVisible() && focusedIndex <= this.lastVisible() && this._view.items.getLayoutIndex(focusedIndex) !== INVALID_INDEX) {
                    this._setFocusOnItem(focusedIndex);
                }
            }
        },

        _unsetFocusOnItem: function ListView_unsetFocusOnItem(index) {
            var item = this._view.items.itemAt(index);
            if (item) {
                utilities.disableTab(item);
            }
        },

        _setFocusOnItem: function ListView_setFocusOnItem(index) {
            if (this._currentMode() instanceof StaticMode) {
                return;
            }

            var item = this._view.items.itemAt(index);

            // If the item is loaded already we can set focus on it immediately, but if not we'll let the element have its focus set when addItem is called on it
            if (item) {
                utilities.enableTab(item);
                if (this._hasKeyboardFocus) {
                    // Some consumers of ListView listen for item invoked events and hide the listview when an item is clicked.
                    // Since keyboard interactions rely on async operations, sometimes an invoke event can be received before we get
                    // to item.focus(), and the listview will be made invisible. If that happens and we call item.focus(), an exception
                    // is raised for trying to focus on an invisible item. Checking visibility is non-trivial, so it's best
                    // just to catch the exception and ignore it.
                    try {
                        item.focus();
                    } catch (error) { }
                }
            }
        },

        _events: function ListView_events() {
            var that = this;
            function listViewHandler(eventName) {
                return {
                    name: eventName.toLowerCase(),
                    handler: function (eventObject) {
                        var fn = that[""_on"" + eventName];
                        if (fn) {
                            fn.apply(that, [eventObject]);
                        }
                    }
                };
            }
            function modeHandler(eventName) {
                return {
                    name: eventName.toLowerCase(),
                    handler: function (eventObject) {
                        var currentMode = that._modes[that._modes.length - 1],
                            fn = currentMode[""on"" + eventName];
                        if (fn) {
                            fn.apply(currentMode, [eventObject]);
                        }
                    }
                };
            }

            var elementEvents = [
                listViewHandler(""Resize""),
                listViewHandler(""PropertyChange""),
                modeHandler(""MouseDown""),
                modeHandler(""MouseMove""),
                modeHandler(""MouseOut""),
                modeHandler(""MouseOver""),
                modeHandler(""MouseUp""),
                modeHandler(""DragStart"")
            ];

            var keyDownHandler = modeHandler(""KeyDown""),
                focusHandler = listViewHandler(""Focus""),
                blurHandler = listViewHandler(""Blur"");

            // onmouseout and onmouseover handlers don't receive valid srcElement in IE9 when setCapture is called. 
            // Also the resize event is not fired for element when addEventListener is used. Using attachEvent temporarily
            for (var i = 0; i < elementEvents.length; i++) {
                if (this._element.attachEvent) {
                    this._element.attachEvent(""on"" + elementEvents[i].name, elementEvents[i].handler);
                } else {
                    this._element.addEventListener(elementEvents[i].name, elementEvents[i].handler, false);
                }
            }

            // KeyDown handler needs to be added explicitly via addEventListener instead of using the above attachEvent.
            // If it's not added via addEventListener, the eventObject given to us on event does not have the functions stopPropagation() and preventDefault();
            this._element.addEventListener(keyDownHandler.name, keyDownHandler.handler, false);
            // Focus and blur events need to be handled during routing, not bubbling.
            this._element.addEventListener(focusHandler.name, focusHandler.handler, true);
            this._element.addEventListener(blurHandler.name, blurHandler.handler, true);

            var viewportEvents = [
                listViewHandler(""Scroll""),
                modeHandler(""MouseLeave""),
                modeHandler(""LoseCapture"")
            ];

            for (i = 0; i < viewportEvents.length; i++) {
                if (this._viewport.attachEvent) {
                    this._viewport.attachEvent(""on"" + viewportEvents[i].name, viewportEvents[i].handler);
                } else {
                    this._viewport.addEventListener(viewportEvents[i].name, viewportEvents[i].handler, false);
                }
            }

            this._element.addEventListener(""DOMNodeInserted"", function (event) {
                if (event.target === that._element) {
                    that._onResize();
                }
            }, false);

            document.body.addEventListener(""keydown"", function (eventObject) {
                if (eventObject.keyCode === utilities.Key.tab) {
                    that._setupTabOrder();
                }
            }, true);
        },

        _setOptions: function ListView_setOptions(options) {
            var params = utilities.extend(options, this._options),
                oldOptions = this._options;

            this._options = params;

            var realizePage = false,
                resetItems = false,
                resetItemSize = false,
                updateLayout = false;

            if ((this._modes.length === 0) ||
                    (oldOptions.mode !== this._options.mode)) {
                this._updateMode();
                realizePage = true;
            }

            if (this._scrollProperty === null ||
                    oldOptions.layout !== this._options.layout ||
                    oldOptions.loadingBehavior !== this._options.loadingBehavior ||
                    oldOptions.pagesToLoad !== this._options.pagesToLoad ||
                    oldOptions.itemsToLoad !== this._options.itemsToLoad ||
                    oldOptions.groupHeaderAbove !== this._options.groupHeaderAbove) {
                updateLayout = true;
                realizePage = true;
            }
            if (oldOptions.loadingBehavior !== this._options.loadingBehavior) {
                resetItems = true;
            }
            if (oldOptions.dataSource && oldOptions.dataSource !== this._options.dataSource) {
                this._usingChildNodes = false;
            }
            if ((this._itemsManager === null) ||
                (oldOptions.dataSource !== this._options.dataSource) ||
                (oldOptions.itemRenderer !== this._options.itemRenderer)) {
                resetItems = true;
                resetItemSize = true;
            }
            if (oldOptions.justified !== this._options.justified) {
                realizePage = true;
            }
            if (oldOptions.groupByFunction !== this._options.groupByFunction ||
                oldOptions.groupRenderer !== this._options.groupRenderer) {
                updateLayout = true;
                resetItems = true;
                resetItemSize = true;
            }
            if (oldOptions.groupRenderer !== this._options.groupRenderer &&
                    this._options.groupRenderer &&
                    typeof this._options.groupRenderer === ""object"") {
                this._options.groupRenderer = this._options.groupRenderer.renderItem;
            }
            if ((oldOptions.pageLoadThreshold !== this._options.pageLoadThreshold ||
                    oldOptions.itemLoadThreshold !== this._options.itemLoadThreshold) &&
                    this._options.loadingBehavior === ""incremental"" && !updateLayout) {
                this._view.updateLoadThreshold(oldOptions.pageLoadThreshold, oldOptions.itemLoadThreshold);
            }

            if (resetItemSize) {
                this._invalidateLayout();
            }

            if (resetItems) {
                this._setViewState(INITIALIZED);
                this._updateItemsManager();
            }

            if (updateLayout) {
                this._updateLayout();
            }

            if (resetItems || realizePage) {
                this._view.reset(this._getViewportSize());
                this._scrollbarPos(0);
            }
        },

        _getOptions: function ListView_getOptions() {
            return utilities.extend({}, this._options);
        },

        _invalidateLayout: function ListView_invalidateLayout() {
            this._totalItemWidth = this._totalItemHeight = this._totalHeaderWidth = this._totalHeaderHeight = UNINITIALIZED;
        },

        _updateItemsManager: function ListView_updateItemsManager() {
            var that = this,
                notificationHandler = {
                    createUpdater: function ListView_createUpdater() {
                        if (!that._updater) {
                            var updater = {
                                oldCount: that._cachedCount,
                                changed: false,
                                elements: {},
                                selection: {},
                                oldFocus: INVALID_INDEX,
                                newFocus: INVALID_INDEX
                            };

                            that._view.items.each(function (index, tile) {
                                updater.elements[tile.uniqueID] = {
                                    tile: tile,
                                    index: index,
                                    newIndex: index
                                };
                            });

                            var selection = that._selection.get();
                            for (var i = 0, len = selection.length; i < len; i++) {
                                updater.selection[selection[i]] = selection[i];
                            }
                            updater.oldFocus = that._selection.getFocused();
                            updater.newFocus = updater.oldFocus;

                            that._updater = updater;
                        }
                    },

                    // Following methods are used by ItemsManager
                    beginNotifications: function ListView_beginNotifications() {
                    },

                    changed: function ListView_changed(newItem, oldItem) {
                        this.createUpdater();

                        do { if (utilities.isDOMElement(newItem)) { } else { assertionFailed(""utilities.isDOMElement(newItem)"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 10277); } } while (false);
                        do { if (newItem.msDataItem !== undefined) { } else { assertionFailed(""newItem.msDataItem !== undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 10278); } } while (false);

                        var elementInfo = that._updater.elements[oldItem.uniqueID];
                        if (elementInfo) {
                            that._canvas.removeChild(elementInfo.tile);
                            delete that._updater.elements[oldItem.uniqueID];
                            that._updater.changed = true;
                        }
                    },

                    removed: function ListView_removed(item, mirage) {
                        this.createUpdater();

                        var index,
                            elementInfo = that._updater.elements[item.uniqueID];
                        if (elementInfo) {
                            index = elementInfo.index;
                            that._canvas.removeChild(elementInfo.tile);
                            delete that._updater.elements[item.uniqueID];
                        } else {
                            index = that._itemsManager.itemIndex(item);
                        }

                        if (that._updater.oldFocus === index) {
                            that._updater.newFocus = INVALID_INDEX;
                        }

                        if (that._updater.selection[index] !== undefined) {
                            delete that._updater.selection[index];
                        }

                        that._updater.changed = true;
                    },

                    indexChanged: function ListView_indexChanged(item, newIndex, oldIndex) {
                        this.createUpdater();

                        var elementInfo = that._updater.elements[item.uniqueID];
                        if (elementInfo) {
                            do { if (elementInfo.index === oldIndex) { } else { assertionFailed(""elementInfo.index === oldIndex"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 10317); } } while (false);
                            elementInfo.newIndex = newIndex;
                            that._updater.changed = true;
                        }
                        if (that._updater.oldFocus === oldIndex) {
                            that._updater.newFocus = newIndex;
                            that._updater.changed = true;
                        }
                        if (that._updater.selection[oldIndex] !== undefined) {
                            that._updater.selection[oldIndex] = newIndex;
                            that._updater.changed = true;
                        }
                    },

                    endNotifications: function ListView_endNotifications() {
                        var updater = that._updater;
                        that._updater = null;

                        if (updater && updater.changed) {
                            that._view.items.setLayoutIndices({});

                            that._element[DRAG_TARGET_EXPANDO].onDataChanged();

                            if (that._currentMode().onDataChanged) {
                                that._currentMode().onDataChanged();
                            }

                            var newSelection = {};
                            for (var i in updater.selection) {
                                if (updater.selection.hasOwnProperty(i)) {
                                    newSelection[updater.selection[i]] = true;
                                }
                            }
                            that._selection.indices = newSelection;
                            that._selection.focused = updater.newFocus;

                            var newItems = {};
                            for (i in updater.elements) {
                                if (updater.elements.hasOwnProperty(i)) {
                                    var elementInfo = updater.elements[i];
                                    newItems[elementInfo.newIndex] = { element: elementInfo.tile };
                                }
                            }
                            that._view.items.itemData = newItems;

                            that._view.groups.resetGroups(that._canvas);
                            that._view.groups.rebuildGroups(that._itemsManager,
                                Math.min(that._cachedCount, that._view.begin),
                                Math.min(that._cachedCount, that._view.end),
                                function () {
                                    that._view.refresh(
                                        that._scrollbarPos(),
                                        that._viewport[that._scrollLength],
                                        that._getViewportSize(),
                                        that._cachedCount);
                                }
                            );
                        }

                        var eventObject = document.createEvent(""Event"");
                        eventObject.initEvent(""datasourcechanged"", true, false);
                        that._element.dispatchEvent(eventObject);
                    },

                    itemAvailable: function ListView_itemAvailable(item, placeholder) {
                        var callbacksMap = that._itemsManager.callbacksMap,
                            placeholderID = placeholder.uniqueID,
                            callbacks = callbacksMap[placeholderID];

                        if (callbacks) {
                            delete callbacksMap[placeholderID];
                            for (var i = 0, len = callbacks.length; i < len; i++) {
                                callbacks[i](item, placeholder);
                            }
                        }
                    },

                    inserted: function ListView_inserted(item, previous, next) {
                        this.createUpdater();
                        that._updater.changed = true;
                    },

                    moved: function ListView_moved(item, previous, next) {
                    },

                    countChanged: function ListView_countChanged(newCount, oldCount) {
                        do { if (newCount !== undefined) { } else { assertionFailed(""newCount !== undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 10403); } } while (false);
                        that._cachedCount = newCount;
                    }
                };

            var itemRenderer = trivialHtmlRenderer;
            if (typeof this._options.itemRenderer === ""function"") {
                itemRenderer = this._options.itemRenderer;
            }
            else if (typeof this._options.itemRenderer === ""object"") {
                itemRenderer = this._options.itemRenderer.renderItem;
            }
            
            this._cachedCount = UNINITIALIZED;
            this._itemsManager = thisWinUI.createItemsManager(this._options.dataSource, itemRenderer, notificationHandler, {
                ownerElement: this._element
            });
            this._itemsManager.callbacksMap = {};
            this._itemsManager.simplerItemAtIndex = function (index, callback) {
                var something = this.itemAtIndex(index);
                if (something) {
                    if (!this.isPlaceholder(something)) {
                        callback(something);
                    } else {
                        var placeholderID = something.uniqueID,
                            callbacks = this.callbacksMap[placeholderID];
                        if (!callbacks) {
                            this.callbacksMap[placeholderID] = callbacks = [callback];
                        } else {
                            callbacks.push(callback);
                        }
                    }
                }
                return something;
            };
        },

        _updateLayout: function ListView_updateLayout() {
            var options = this._options;

            if (this._view) {
                this._view.cleanUp();
            }

            if (options.loadingBehavior === ""incremental"") {
                this._view = new IncrementalView(this);
            } else {
                do { if (options.loadingBehavior === ""randomaccess"") { } else { assertionFailed(""options.loadingBehavior === \""randomaccess\"""", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 10450); } } while (false);
                this._view = new ScrollView(this);
            }

            if (this._view.updateGroups) {
                this._view.updateGroups(options.groupByFunction, options.groupRenderer);
            }

            this._view.updateLayout(options.layout, options.groupByFunction);

            var style = this._viewport.style;
            if ((options.layout === ""list"") || (options.layout === ""verticalgrid"")) {
                this._scrollProperty = ""scrollTop"";
                this._scrollLength = ""scrollHeight"";
                style.overflowY = ""auto"";
                style.overflowX = ""hidden"";
                this._viewport.scrollLeft = 0;
            } else {
                do { if (options.layout === ""horizontalgrid"") { } else { assertionFailed(""options.layout === \""horizontalgrid\"""", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 10468); } } while (false);
                this._scrollProperty = ""scrollLeft"";
                this._scrollLength = ""scrollWidth"";
                style.overflowY = ""hidden"";
                style.overflowX = ""auto"";
                this._viewport.scrollTop = 0;
            }
        },

        _pushMode: function ListView_pushMode(newMode) {
            var currentMode = this._currentMode();
            if (currentMode.deactivate) {
                currentMode.deactivate();
            }
            this._modes.push(newMode);
            currentMode = this._currentMode();
            if (currentMode.activate) {
                currentMode.activate();
            }
            this._options.mode = currentMode.name;
        },

        _currentMode: function ListView_currentMode() {
            do { if (this._modes.length > 0) { } else { assertionFailed(""this._modes.length > 0"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 10491); } } while (false);
            return this._modes[this._modes.length - 1];
        },

        _popMode: function ListView_popMode() {
            var currentMode = this._currentMode();
            if (currentMode.deactivate) {
                currentMode.deactivate();
            }
            this._modes.pop();
            currentMode = this._currentMode();
            if (currentMode.activate) {
                currentMode.activate();
            }
            this._options.mode = currentMode.name;
        },

        _updateMode: function ListView_updateMode() {
            var currentMode = this._modes.length > 0 ? this._modes[this._modes.length - 1] : null;
            if (currentMode && currentMode.deactivate) {
                currentMode.deactivate();
            }

            var selection = false;
            switch (this._options.mode) {
                case ""browse"":
                    this._modes = [new BrowseMode(this)];
                    break;

                case ""singleselection"":
                    this._modes = [new SingleSelectionMode(this)];
                    selection = true;
                    break;

                case ""multiselection"":
                    this._modes = [new MultiSelectionMode(this)];
                    selection = true;
                    break;

                case ""static"":
                    this._modes = [new StaticMode(this)];
                    break;

                default:
                    // TODO: temporary
                    this._modes = [new BrowseMode(this)];
                    break;
            }

            if (selection) {
                this._itemRole = ""option"";
                this._element.setAttribute(""role"", ""listbox"");
                this._element.setAttribute(""aria-multiselectable"", this._options.mode === ""multiselection"");
            } else {
                this._itemRole = ""listitem"";
                this._element.setAttribute(""role"", ""list"");
            }

            currentMode = this._currentMode();
            if (currentMode.activate) {
                currentMode.activate();
            }

            if (this._element.disabled) {
                this.disabled(true);
            }
        },

        _onResize: function ListView_onResize() {
            if ((this._previousWidth !== this._element.offsetWidth) ||
                (this._previousHeight !== this._element.offsetHeight)) {
                this._previousWidth = this._element.offsetWidth;
                this._previousHeight = this._element.offsetHeight;

                this._viewportWidth = UNINITIALIZED;
                this._viewportHeight = UNINITIALIZED;

                var viewportSize = this._getViewportSize(),
                    viewportStyle = this._viewport.style;
                viewportStyle.width = viewportSize.cx + ""px"";
                viewportStyle.height = viewportSize.cy + ""px"";

                this._setViewState(INITIALIZED);
                this._view.onResize(this._scrollbarPos(), this._getViewportSize());
            }
        },

        _onFocus: function ListView_onFocus() {
            this._hasKeyboardFocus = true;
        },

        _onBlur: function ListView_onBlur() {
            this._hasKeyboardFocus = false;
        },

        _onScroll: function ListView_onScroll() {
            this._view.onScroll(
                this._scrollbarPos(),
                this._viewport[this._scrollLength],
                this._getViewportSize());
        },

        _onPropertyChange: function ListView_onPropertyChange() {
            if ((event.propertyName === ""dir"") || (event.propertyName === ""style.direction"")) {
                if (this._rtl()) {
                    utilities.addClass(this._element, rtlListViewClass);
                } else {
                    utilities.removeClass(this._element, rtlListViewClass);
                }
                this._invalidateLayout();
                this._view.reset(this._getViewportSize());
                this._scrollbarPos(0);
            }
        },

        // Methods in the site interface used by ScrollView

        _getCanvas: function ListView_getCanvas() {
            return this._canvas;
        },

        _getItemsManager: function ListView_getItemsManager() {
            return this._itemsManager;
        },

        _getViewportSize: function ListView_getViewportSize() {
            if (this._viewportWidth === UNINITIALIZED || this._viewportHeight === UNINITIALIZED) {
                this._viewportWidth = utilities.contentWidth(this._element);
                this._viewportHeight = utilities.contentHeight(this._element);
            }
            return {
                cx: this._viewportWidth,
                cy: this._viewportHeight
            };
        },

        _getHeaderTotalSize: function ListView_getHeaderTotalSize() {
            do { if (this._totalHeaderWidth !== UNINITIALIZED) { } else { assertionFailed(""this._totalHeaderWidth !== UNINITIALIZED"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 10628); } } while (false);
            return {
                cx: this._totalHeaderWidth,
                cy: this._totalHeaderHeight
            };
        },

        _getHeaderContentSize: function ListView_getHeaderContentSize() {
            do { if (this._headerWidth !== UNINITIALIZED) { } else { assertionFailed(""this._headerWidth !== UNINITIALIZED"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 10636); } } while (false);
            return {
                cx: this._headerWidth,
                cy: this._headerHeight
            };
        },

        _getHeaderMargin: function ListView_getHeaderMargin() {
            return this._headerMargin;
        },

        _getLeadingMargin: function ListView_getLeadingMargin() {
            return this._leadingMargin;
        },

        _getItemTotalSize: function ListView_getItemTotalSize() {
            do { if (this._totalItemWidth !== UNINITIALIZED) { } else { assertionFailed(""this._totalItemWidth !== UNINITIALIZED"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 10652); } } while (false);
            return {
                cx: this._totalItemWidth,
                cy: this._totalItemHeight
            };
        },

        _getItemContentSize: function ListView_getItemContentSize() {
            do { if (this._itemWidth !== UNINITIALIZED) { } else { assertionFailed(""this._itemWidth !== UNINITIALIZED"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 10660); } } while (false);
            return {
                cx: this._itemWidth,
                cy: this._itemHeight
            };
        },

        _updateItemSize: function ListView_updateItemSize(count, callback) {
            var that = this;

            function defaultItemSize() {
                that._itemWidth = DEFAULT_ITEM_WIDTH;
                that._itemHeight = DEFAULT_ITEM_HEIGHT;
                that._totalItemWidth = DEFAULT_ITEM_WIDTH;
                that._totalItemHeight = DEFAULT_ITEM_HEIGHT;
                that._leadingMargin = {
                    left: 0,
                    right: 0,
                    top: 0
                };
            }

            function defaultHeaderSize() {
                that._headerWidth = that._headerHeight = that._totalHeaderWidth = that._totalHeaderHeight = 0;
                that._headerMargin = {
                    left: 0,
                    right: 0,
                    top: 0
                };
            }

            function getMargins(element) {
                return {
                    left: utilities.getDimension(element, ""marginLeft""),
                    right: utilities.getDimension(element, ""marginRight""),
                    top: utilities.getDimension(element, ""marginTop"")
                };
            }

            if (this._totalItemWidth !== UNINITIALIZED && this._totalItemHeight !== UNINITIALIZED &&
                this._headerWidth !== UNINITIALIZED && this._headerHeight !== UNINITIALIZED) {
                // If the sizes are already calculated we don't have to do anything
                callback(true);
            } else if (count > 0) {
                var something = this._itemsManager.simplerItemAtIndex(0, function (element) {

                    utilities.addClass(element, itemClass);
                    var secondElement = element.cloneNode(true);

                    that._element.insertBefore(element, that._element.firstChild);
                    that._element.appendChild(secondElement);

                    var leadingMargin;
                    if (that._options.groupByFunction) {
                        var newGroup = that._options.groupByFunction(null, element.msDataItem.dataObject, 0),
                            headers = [];

                        for (var i = 1; i >= 0; i--) {
                            headers[i] = that._options.groupRenderer(newGroup);
                            that._element.insertBefore(headers[i], that._element.firstChild);
                            utilities.addClass(headers[i], headerClass);
                        }

                        // This calls offsetWidth so it can trigger layout pass and cause reentry
                        that._headerWidth = utilities.contentWidth(headers[1]);
                        that._headerHeight = utilities.contentHeight(headers[1]);
                        that._totalHeaderWidth = utilities.totalWidth(headers[1]);
                        that._totalHeaderHeight = utilities.totalHeight(headers[1]);
                        that._headerMargin = getMargins(headers[1]);

                        leadingMargin = getMargins(headers[0]);

                        for (i = 1; i >= 0; i--) {
                            that._element.removeChild(headers[i]);
                        }

                    } else {
                        leadingMargin = getMargins(element);

                        defaultHeaderSize();
                    }

                    // This calls offsetWidth so it can trigger layout pass and cause reentry
                    var itemWidth = utilities.contentWidth(secondElement),
                        itemHeight = utilities.contentHeight(secondElement),
                        totalItemWidth = utilities.totalWidth(secondElement),
                        totalItemHeight = utilities.totalHeight(secondElement);

                    that._element.removeChild(secondElement);
                    // This element could be reparented by a reentrant call so the parent must be checked before removal
                    if (element.parentNode === that._element) {
                        that._element.removeChild(element);
                    }

                    if (that._totalItemWidth === UNINITIALIZED || that._totalItemHeight === UNINITIALIZED) {
                        if (totalItemWidth !== 0 && totalItemHeight !== 0) {
                            that._itemWidth = itemWidth;
                            that._itemHeight = itemHeight;
                            that._totalItemWidth = totalItemWidth;
                            that._totalItemHeight = totalItemHeight;
                            that._leadingMargin = leadingMargin;
                        } else {
                            defaultItemSize();
                        }
                    }

                    callback(true);
                });

                if (!something) {
                    callback(false);
                }
            } else {
                defaultItemSize();
                defaultHeaderSize();

                callback(true);
            }
        },

        _itemsCount: function ListView_itemsCount(callback) {
            if (this._cachedCount !== UNINITIALIZED) {
                callback(this._cachedCount);
            } else {
                var that = this;
                this._itemsManager.count(function (count) {
                    that._cachedCount = count;
                    callback(count);
                });
            }
        },

        _isSelected: function ListView_isSelected(index) {
            return this._selection.isSelected(index);
        },

        _setViewState: function ListView_setViewState(state) {
            if (state !== this._viewState) {
                this._viewState = state;
                var eventObject = document.createEvent(""Event"");
                eventObject.initEvent(""viewstatechanged"", true, false);
                this._element.dispatchEvent(eventObject);
            }
        },

        // Methods used by SelectionManager

        _updateSelection: function ListView_updateSelection(unselected, selected) {
            return this._view.items.updateSelection(unselected, selected);
        },

        _itemAt: function ListView_itemAt(itemIndex) {
            return this._view.items.itemAt(itemIndex);
        },

        _itemFrom: function ListView_itemFrom(element) {
            return this._view.items.itemFrom(element);
        },

        _getViewportLength: function ListView_getViewportLength() {
            return this._getViewportSize()[this._horizontal() ? ""cx"" : ""cy""];
        },

        _horizontal: function ListView_horizontal() {
            return this._scrollProperty === ""scrollLeft"";
        },

        _scrollbarPos: function ListView_scrollbarPos(newPos) {
            if (newPos !== undefined) {
                this._viewport[this._scrollProperty] = newPos;
            } else {
                return this._viewport[this._scrollProperty];
            }
        },

        _setSelection: function ListView_setSelection(newSelection) {
            for (var i = 0, count = newSelection.length; i < count; i++) {
                var index = newSelection[i];
                if ((index < 0) || (index >= this._cachedCount)) {
                    throw new Error(itemIndexIsInvalid);
                }
            }
            var currentMode = this._currentMode();
            if (currentMode.canSelect) {
                if (currentMode.canSelect(newSelection)) {
                    return this._selection.set(newSelection);
                }
            }
            throw new Error(itemIndexIsInvalid);
        },

        _setDragHandler: function ListView_setDragHandler() {
            this._element[DRAG_TARGET_EXPANDO] = new DragTargetHandler(this);
        },

        _createListEditCallback: function ListView__createListEditCallack(userCallback) {
            return function (editResult) {
                if (editResult === thisWinUI.EditResult.success && userCallback && userCallback.success) {
                    userCallback.success();
                } else if (editResult !== thisWinUI.EditResult.success && userCallback && userCallback.error) {
                    userCallback.error(editResult);
                }
            };
        },

        _indexFromElement: function ListView_indexFromElement(something, callback) {
            var that = this;
            if (something && !this._itemsManager.isPlaceholder(something)) {
                callback.success(this._itemsManager.itemIndex(something));
            } else {
                var placeholderID = something.uniqueID,
                    callbacks = this._itemsManager.callbacksMap[placeholderID];
                if (!callbacks) {
                    this._itemsManager.callbacksMap[placeholderID] = callbacks = [];
                }
                callbacks.push(function (element) {
                    callback.success(that._itemsManager.itemIndex(element));
                });
            }
        },

        _dragSupported: function ListView_dragSupported() {
            return this._options.loadingBehavior === ""randomaccess"";
        },

        _rtl: function ListView_rtl() {
            return window.getComputedStyle(this._element, null).direction === ""rtl"";
        }
    },

    function (element, options) {
        if (!element) {
            throw new Error(elementIsInvalid);
        }

        if (this === window || this === Win.UI.Controls) {
            var listView = utilities.getData(element, ""listView"");
            if (listView) {
                return listView;
            } else {
                return new Win.UI.Controls.ListView(element, options);
            }
        }

        options = validateOptions(options || {});


        this._debug = true;


        // Attaching JS control to DOM element
        utilities.setData(element, ""listView"", this);

        this._element = element;
        this._options = {
            layout: ""verticalgrid"",
            loadingBehavior: ""randomaccess"",
            justified: false,
            reorder: false,
            mode: ""browse"",
            groupHeaderAbove: false
        };
        this._view = new ScrollView(this);
        this._selection = new SelectionManager(this);
        this._modes = [];
        this._scrollProperty = null;
        this._scrollLength = null;
        this._itemsManager = null;
        this._canvas = null;
        this._cachedCount = UNINITIALIZED;
        this._viewState = 0;
        this._totalItemWidth = UNINITIALIZED;
        this._totalItemHeight = UNINITIALIZED;
        this._itemWidth = UNINITIALIZED;
        this._itemHeight = UNINITIALIZED;
        this._viewportWidth = UNINITIALIZED;
        this._viewportHeight = UNINITIALIZED;
        this._totalHeaderWidth = UNINITIALIZED;
        this._totalHeaderHeight = UNINITIALIZED;
        this._headerWidth = UNINITIALIZED;
        this._headerHeight = UNINITIALIZED;
        var childElements = this._setupInternalTree();
        if (!options.dataSource) {
            this._options.dataSource = thisWinUI.createObjectDataSource(childElements, { storeReferences: true });
            this._usingChildNodes = true;
        }
        this._events();
        this._setOptions(options);
        this._setDragHandler();
    })
});

Win.Namespace.defineWithParent(thisWinUI, ""Controls"", {
    ListViewState: Win.Class.define(null, {},
        function () {
        }, {
            initialized: INITIALIZED,
            realized: REALIZED,
            ready: READY
        }
    )
});

})(Win8.UI);

(function (Win) {
    var thisWinUI = Win.UI;

    // Utilities are private and global pointer will be deleted so we need to cache it locally
    var utilities = thisWinUI.Utilities;

    var largeMidpoint = 50000;

    function isFlipper(element) {
        return utilities.getData(element, ""flipper"") !== undefined;
    }

    Win.Namespace.defineWithParent(thisWinUI, ""Controls"", {

        // Definition of our private utility
        _FlipPageManager: Win.Class.define(null,
            {
                // Public methods

                initializeHorizontal: function (initialIndex, countToLoad) {
                    this._initializeHorizontalFunctions();
                    this._initialize(initialIndex, countToLoad);
                },

                initializeVertical: function (initialIndex, countToLoad) {
                    this._initializeVerticalFunctions();
                    this._initialize(initialIndex, countToLoad);
                },

                setNewItemsManager: function (manager, initialIndex) {
                    do { if (initialIndex !== undefined) { } else { assertionFailed(""initialIndex !== undefined"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 10995); } } while (false);
                    this._resetBuffer(null);
                    this._itemsManager = manager;
                    if (this._itemsManager) {
                        this._currentPage.setElement(this._itemsManager.firstItem());
                        this._fetchPreviousItems(true);
                        this._fetchNextItems();
                        if (this._currentPage.element) {
                            if (initialIndex !== 0 && !this.jumpToIndex(initialIndex)) {
                                throw new Error(thisWinUI.Controls.Flipper.badCurrentPage);
                            }
                        }

                        this._setButtonStates();
                    }

                    this._ensureCentered();
                },

                currentIndex: function () {
                    if (!this._itemsManager) {
                        return 0;
                    }

                    var element = this._currentPage.element;
                    return (element ? this._itemsManager.itemIndex(element) : 0);
                },

                resetScrollPos: function () {
                    this._ensureCentered();
                },

                scrollPosChanged: function () {
                    if (!this._itemsManager) {
                        return;
                    }

                    //TODO for M3 when DirectManipulation is integrated
                    this._setButtonStates();
                    this._checkElementVisibility(false);
                    // TODO(jumyhres): M3: When storyboard finishes for snap point, call ensureCentered()
                },

                itemRetrieved: function (real, placeholder) {
                    var curr = this._prevMarker;
                    do {
                        if (curr.element === placeholder) {
                            curr.setElement(real);
                            break;
                        }
                        curr = curr.next;
                    } while (curr !== this._prevMarker);
                },

                centerContent: function () {
                    var curr = this._prevMarker;
                    do {
                        curr.centerElement();
                        curr = curr.next;
                    } while (curr !== this._prevMarker);
                },

                next: function () {
                    if (!this._currentPage.next.element || !this._itemsManager) {
                        return false;
                    }

                    this._currentPage = this._currentPage.next;
                    var newBufferItem = this._currentPage,
                        flipPageBufferCount = thisWinUI.Controls._FlipPageManager.flipPageBufferCount;
                    for (var i = 0; i < flipPageBufferCount; i++) {
                        newBufferItem = newBufferItem.next;
                    }
                    this._fetchOneNext(newBufferItem);

                    // TODO(jumyhres): M3: This needs to be animated. setButtonStates should both be called here, but ensureCentered() should be called post-animation
                    this._setButtonStates();
                    this._ensureCentered();

                    return true;
                },

                previous: function () {
                    if (!this._currentPage.prev.element || !this._itemsManager) {
                        return false;
                    }

                    this._currentPage = this._currentPage.prev;
                    var newBufferItem = this._currentPage,
                        flipPageBufferCount = thisWinUI.Controls._FlipPageManager.flipPageBufferCount;
                    for (var i = 0; i < flipPageBufferCount; i++) {
                        newBufferItem = newBufferItem.prev;
                    }
                    this._fetchOnePrevious(newBufferItem);

                    // TODO(jumyhres): M3: This needs to be animated. setButtonStates should both be called here, but ensureCentered() should be called post-animation
                    this._setButtonStates();
                    this._ensureCentered();

                    return true;
                },

                jumpToIndex: function (index) {
                    if (!this._itemsManager || !this._currentPage.element || index < 0) {
                        return false;
                    }

                    // If we're close we can just call next/prev
                    var currIndex = this._itemsManager.itemIndex(this._currentPage.element),
                        distance = Math.abs(index - currIndex);
                    if (distance < 2) {
                        if (index < currIndex) {
                            return this.previous();
                        } else if (index > currIndex) {
                            return this.next();
                        } else {
                            return true;
                        }
                    }

                    // If we've got to keep our pages in memory, we need to iterate through every single item from our current position to the desired target
                    var i;
                    if (this._keepItemsInMemory) {
                        var newCurrent = this._currentPage;
                        if (index > currIndex) {
                            for (i = 0; i < distance && newCurrent.element; i++) {
                                if (newCurrent.next === this._prevMarker) {
                                    this._fetchOneNext(newCurrent.next);
                                }
                                newCurrent = newCurrent.next;
                                if (!newCurrent.element) {
                                    newCurrent.setElement(this._itemsManager.nextItem(newCurrent.prev.element));
                                }
                            }
                        } else {
                            for (i = 0; i < distance && newCurrent.element; i++) {
                                if (newCurrent.prev === this._prevMarker.prev) {
                                    this._fetchOnePrevious(newCurrent.prev);
                                }
                                newCurrent = newCurrent.prev;
                                if (!newCurrent.element) {
                                    newCurrent.setElement(this._itemsManager.previousItem(newCurrent.next.element));
                                }
                            }
                        }

                        if (!newCurrent.element) {
                            return false;
                        } else {
                            // Need to announce the old currentPage as invisible before changing to the new one, then announce the new one as visible
                            this._currentPage = newCurrent;
                            this._fetchNextItems();
                            this._fetchPreviousItems(false);
                        }

                    } else {
                        var elementAtIndex = this._itemsManager.itemAtIndex(index);
                        if (!elementAtIndex) {
                            return false;
                        }

                        this._resetBuffer(elementAtIndex);
                        this._currentPage.setElement(elementAtIndex);
                        this._fetchNextItems();
                        this._fetchPreviousItems(true);
                    }
                    this._setButtonStates();
                    this._ensureCentered();

                    return true;
                },

                inserted: function (element, prev, next) {
                    var curr = this._prevMarker,
                        passedCurrent = false;

                    if (next && next === this._prevMarker.element) {
                        if (this._keepItemsInMemory) {
                            this._prevMarker = this._insertNewFlipPage(this._prevMarker.prev);
                            this._prevMarker.setElement(element);
                        }
                    } else if (!prev) {
                        if (!next) {
                            this._currentPage.setElement(element);
                        } else {
                            while (curr.next !== this._prevMarker && curr.element !== next) {
                                if (curr === this._currentPage) {
                                    passedCurrent = true;
                                }
                                curr = curr.next;
                            }

                            // We never should go past current if prev is null/undefined.
                            do { if (!passedCurrent) { } else { assertionFailed(""!passedCurrent"", ""d:\\nt.obj.x86chk\\windows\\webcontrols\\library\\objchk\\i386\\win8ui.pp"", 11188); } } while (false);

                            if (curr.element === next && curr !== this._prevMarker) {
                                curr.prev.setElement(element);
                            } else {
                                this._itemsManager.releaseItem(element);
                            }
                        }
                    } else {
                        do {
                            if (curr === this._currentPage) {
                                passedCurrent = true;
                            }
                            if (curr.element === prev) {
                                if (this._keepItemsInMemory) {
                                    var newPage = this._insertNewFlipPage(curr);
                                    newPage.setElement(element);
                                } else {
                                    var pageShifted = curr,
                                        lastElementMoved = element,
                                        temp;
                                    if (passedCurrent) {
                                        while (pageShifted.next !== this._prevMarker) {
                                            temp = pageShifted.next.element;
                                            pageShifted.next.setElement(lastElementMoved, true);
                                            lastElementMoved = temp;
                                            pageShifted = pageShifted.next;
                                        }
                                    } else {
                                        while (pageShifted.next !== this._prevMarker) {
                                            temp = pageShifted.element;
                                            pageShifted.setElement(lastElementMoved, true);
                                            lastElementMoved = temp;
                                            pageShifted = pageShifted.prev;
                                        }
                                    }
                                    if (lastElementMoved) {
                                        this._itemsManager.releaseItem(lastElementMoved);
                                    }
                                }
                                break;
                            }
                            curr = curr.next;
                        } while (curr !== this._prevMarker);
                    }

                    this._setButtonStates();
                    this._ensureCentered();
                },

                changed: function (newVal, element) {
                    var curr = this._prevMarker;
                    do {
                        if (curr.element === element) {
                            curr.setElement(newVal, true); // TODO (jumyhres): M3: Replacement animation
                            break;
                        }
                        curr = curr.next;
                    } while (curr !== this._prevMarker);

                    this._checkElementVisibility(false);
                },

                moved: function (element, prev, next) {
                    this.removed(element, false, true);
                    this.inserted(element, prev, next);
                },

                removed: function (element, mirage, preserveElement) {
                    var elementRemoved = false,
                        prevMarker = this._prevMarker;
                    if (this._currentPage.element === element) {
                        if (this._currentPage.next.element) {
                            this._shiftLeft(this._currentPage);
                        } else if (this._currentPage.prev.element) {
                            this._shiftRight(this._currentPage);
                        } else {
                            this._currentPage.setElement(null, true);
                        }
                        elementRemoved = true;
                    } else if (prevMarker.element === element) {
                        prevMarker.setElement(this._itemsManager.previousItem(element));
                        elementRemoved = true;
                    } else if (prevMarker.prev.element === element) {
                        prevMarker.prev.setElement(this._itemsManager.nextItem(element));
                        elementRemoved = true;
                    } else {
                        var curr = this._currentPage.prev,
                            handled = false;
                        while (curr !== prevMarker && !handled) {
                            if (curr.element === element) {
                                this._shiftRight(curr);
                                elementRemoved = true;
                                handled = true;
                            }

                            curr = curr.prev;
                        }

                        curr = this._currentPage.next;
                        while (curr !== prevMarker && !handled) {
                            if (curr.element === element) {
                                this._shiftLeft(curr);
                                elementRemoved = true;
                                handled = true;
                            }

                            curr = curr.next;
                        }
                    }

                    if (elementRemoved && !preserveElement) {
                        this._itemsManager.releaseItem(element);
                    }

                    //todo: if currentPage is null, try not to get into a mirage loop
                    this._setButtonStates();
                    this._ensureCentered();
                },

                indexChanged: function (element, newIndex) {
                    //TODO(jumyhres) M3: This'll probably be useful.
                },

                getItemSpacing: function () {
                    return this._itemSpacing;
                },

                setItemSpacing: function (space) {
                    this._itemSpacing = space;
                    this._ensureCentered();
                },

                // Private methods

                _initialize: function (initialIndex, countToLoad) {
                    var currPage = null;
                    if (!this._currentPage) {
                        this._currentPage = this._createFlipPage(null, this, this._itemsManager);
                        currPage = this._currentPage;
                        this._panningDiv.appendChild(currPage.div);

                        // flipPageBufferCount is added here twice. Once for the buffer prior to the current item, and once for the buffer ahead of the current item.
                        var pagesToInit = (countToLoad > 0 ? countToLoad - 1 : 0) + 2 * thisWinUI.Controls._FlipPageManager.flipPageBufferCount;
                        for (var i = 0; i < pagesToInit; ++i) {
                            currPage = this._createFlipPage(currPage, this, this._itemsManager);
                            this._panningDiv.appendChild(currPage.div);
                        }
                    }

                    this._prevMarker = this._currentPage.prev;
                    this._ensureCentered();

                    if (this._itemsManager) {
                        this.setNewItemsManager(this._itemsManager, 0); // We'll use 0 here just to load a currentPage up. We'll be prefetching a bunch of items down below if countToLoad > 0
                        // so we'll load, prefetch, then jumpToPage
                    }

                    if (countToLoad > 0) {
                        var curr = this._currentPage,
                            alreadyLoaded = 0;
                        while (curr.element && curr !== this._prevMarker) {
                            alreadyLoaded++;
                            curr = curr.next;
                        }

                        countToLoad -= alreadyLoaded;
                        for (var j = 0; j < countToLoad && curr !== this._prevMarker; j++) {
                            curr.setElement(this._itemsManager.nextItem(curr.prev.element));
                            curr = curr.next;
                        }
                    }

                    if (initialIndex > 0) {
                        if (!this.jumpToIndex(initialIndex)) {
                            throw new Error(thisWinUI.Controls.Flipper.badCurrentPage);
                        }
                    }
                },

                _resetBuffer: function (elementToSave) {
                    var head = this._currentPage,
                        curr = head;
                    do {
                        if (curr.element && curr.element === elementToSave) {
                            curr.setElement(null, true);
                        } else {
                            curr.setElement(null);
                        }
                        curr = curr.next;
                    } while (curr !== head);
                },

                _insertNewFlipPage: function (prevElement) {
                    var newPage = this._createFlipPage(prevElement, this, this._itemsManager);
                    this._panningDiv.appendChild(newPage.div);
                    return newPage;
                },

                _fetchNextItems: function () {
                    var curr = this._currentPage,
                        flipPageBufferCount = thisWinUI.Controls._FlipPageManager.flipPageBufferCount;
                    for (var i = 0; i < flipPageBufferCount; i++) {
                        if (curr.next === this._prevMarker) {
                            this._insertNewFlipPage(curr);
                        }
                        if (curr.element) {
                            curr.next.setElement(this._itemsManager.nextItem(curr.element));
                        } else {
                            curr.next.setElement(null);
                        }
                        curr = curr.next;
                    }
                },

                _fetchOneNext: function (target) {
                    // If the target we want to fill with the next item is the end of the circular buffer but we want to keep everything in memory, we've got to increase the buffer size
                    // so that we don't reuse prevMarker.
                    if (this._prevMarker === target) {
                        if (this._keepItemsInMemory) {
                            target = this._insertNewFlipPage(target.prev);
                        } else {
                            this._prevMarker = this._prevMarker.next;
                        }
                    }
                    var prevElement = target.prev.element;
                    if (!prevElement) {
                        target.setElement(null);
                        return;
                    }
                    target.setElement(this._itemsManager.nextItem(prevElement));
                },

                _fetchPreviousItems: function (setPrevMarker) {
                    var curr = this._currentPage,
                        flipPageBufferCount = thisWinUI.Controls._FlipPageManager.flipPageBufferCount;
                    for (var i = 0; i < flipPageBufferCount; i++) {
                        if (curr.element) {
                            curr.prev.setElement(this._itemsManager.previousItem(curr.element));
                        } else {
                            curr.prev.setElement(null);
                        }
                        curr = curr.prev;
                    }

                    if (setPrevMarker) {
                        this._prevMarker = curr;
                    }
                },

                _fetchOnePrevious: function (target) {
                    // If the target we want to fill with the previous item is the end of the circular buffer but we want to keep everything in memory, we've got to increase the buffer size
                    // so that we don't reuse prevMarker. We'll add a new element to be prevMarker's prev, then set prevMarker to point to that new element
                    if (this._prevMarker === target.next) {
                        if (this._keepItemsInMemory) {
                            target = this._insertNewFlipPage(target.prev);
                            this._prevMarker = target;
                        } else {
                            this._prevMarker = this._prevMarker.prev;
                        }
                    }
                    var nextElement = target.next.element;
                    if (!nextElement) {
                        target.setElement(null);
                        return;
                    }
                    target.setElement(this._itemsManager.previousItem(nextElement));
                },

                _setButtonStates: function () {
                    if (this._currentPage.prev.element) {
                        this._buttonVisibilityHandler.showPreviousButton();
                    } else {
                        this._buttonVisibilityHandler.hidePreviousButton();
                    }

                    if (this._currentPage.next.element) {
                        this._buttonVisibilityHandler.showNextButton();
                    } else {
                        this._buttonVisibilityHandler.hideNextButton();
                    }
                },

                _ensureCentered: function () {
                    var center = largeMidpoint;

                    this._itemStart(this._currentPage, center, 0);
                    var curr = this._currentPage;
                    while (curr !== this._prevMarker) {
                        this._movePageBehind(curr, curr.prev);
                        curr = curr.prev;
                    }

                    curr = this._currentPage;
                    while (curr.next !== this._prevMarker) {
                        this._movePageAhead(curr, curr.next);
                        curr = curr.next;
                    }
                    this._viewportStart(this._itemStart(this._currentPage));
                    this._checkElementVisibility(true);
                },

                _shiftLeft: function (startingPoint) {
                    var curr = startingPoint,
                        nextEl = null;
                    while (curr !== this._prevMarker && curr.next !== this._prevMarker) {
                        nextEl = curr.next.element;
                        curr.next.setElement(null, true);
                        curr.setElement(nextEl, true);
                        curr = curr.next;
                    }
                    if (curr !== this._prevMarker && curr.prev.element) {
                        curr.setElement(this._itemsManager.nextItem(curr.prev.element));
                    }
                },

                _shiftRight: function (startingPoint) {
                    var curr = startingPoint,
                        prevEl = null;
                    while (curr !== this._prevMarker) {
                        prevEl = curr.prev.element;
                        curr.prev.setElement(null, true);
                        curr.setElement(prevEl, true);
                        curr = curr.prev;
                    }
                    if (curr.next.element) {
                        curr.setElement(this._itemsManager.previousItem(curr.next.element));
                    }
                },

                _checkElementVisibility: function (viewWasReset) {
                    var i,
                        len;
                    if (viewWasReset) {
                        var currentElement = this._currentPage.element;
                        for (i = 0, len = this._visibleElements.length; i < len; i++) {
                            if (this._visibleElements[i] !== currentElement) {
                                this._announceElementInvisible(this._visibleElements[i]);
                            }
                        }

                        this._visibleElements = [this._currentPage.element];
                        this._announceElementVisible(currentElement);
                    } else {
                        // Elements that have been removed completely from the flipper still need to raise pageVisChangedEvents if they were visible prior to being removed,
                        // so before going through all the elements we go through the ones that we knew were visible and see if they're missing a parentNode. If they are,
                        // the elements were removed and we announce them as invisible.
                        for (i = 0, len = this._visibleElements.length; i < len; i++) {
                            if (!this._visibleElements[i].parentNode) {
                                this._announceElementInvisible(this._visibleElements[i]);
                            }
                        }
                        this._visibleElements = [];
                        var curr = this._prevMarker;
                        do {
                            var element = curr.element;
                            if (element) {
                                if (this._itemInView(curr)) {
                                    this._visibleElements.push(element);
                                    this._announceElementVisible(element);
                                } else {
                                    this._announceElementInvisible(element);
                                }
                            }

                            curr = curr.next;
                        } while (curr !== this._prevMarker);
                    }
                },

                _announceElementVisible: function (element) {
                    if (element && !element.visible) {
                        utilities.enableTab(element);
                        element.visible = true;

                        var event = document.createEvent(""Event"");
                        event.initEvent(thisWinUI.Controls.Flipper.pageVisibilityChangedEvent, true, true);
                        event.source = this._flipperDiv;
                        event.visible = true;

                        element.dispatchEvent(event);
                    }
                },

                _announceElementInvisible: function (element) {
                    if (element && element.visible) {
                        utilities.disableTab(element);
                        element.visible = false;

                        // Elements that have been removed from the flipper still need to fire invisible events, but they can't do that without being in the DOM.
                        // To fix that, we add the element back into the flipper, fire the event, then remove it.
                        var addedToDomForEvent = false;
                        if (!element.parentNode) {
                            addedToDomForEvent = true;
                            this._panningDivContainer.appendChild(element);
                        }

                        var event = document.createEvent(""Event"");
                        event.initEvent(thisWinUI.Controls.Flipper.pageVisibilityChangedEvent, true, true);
                        event.source = this._flipperDiv;
                        event.visible = false;

                        element.dispatchEvent(event);
                        if (addedToDomForEvent) {
                            this._panningDivContainer.removeChild(element);
                        }
                    }
                },

                _createFlipPage: function (prev, manager, itemsManager) {
                    var page = {};
                    page.element = null;
                    page.pageAxisOffset = -100 * this._totalFlipPages;
                    this._totalFlipPages++;

                    // The flip pages are managed as a circular doubly-linked list. this.currentItem should always refer to the current item in view, and this._prevMarker marks the point 
                    // in the list where the last previous item is stored. Why a circular linked list?
                    // The virtualized flipper reuses its flip pages. When a new item is requested, the flipper needs to reuse an old item from the buffer. In the case of previous items,
                    // the flipper has to go all the way back to the farthest next item in the buffer and recycle it (which is why having a .prev pointer on the prev-most item is really useful),
                    // and in the case of the next-most item, it needs to recycle next's next (ie, the this._prevMarker). The linked structure comes in really handy when iterating through the list
                    // and separating out prev items from next items (like removed and ensureCentered do). If we were to use a structure like an array it would be pretty messy to do that and still
                    // maintain a buffer of recyclable items.
                    if (!prev) {
                        page.next = page;
                        page.prev = page;
                    } else {
                        page.prev = prev;
                        page.next = prev.next;
                        page.next.prev = page;
                        prev.next = page;
                    }

                    page.div = document.createElement(""div"");
                    var pageStyle = page.div.style;
                    pageStyle.position = ""relative"";
                    pageStyle.overflow = ""hidden"";
                    pageStyle.top = page.pageAxisOffset + ""%"";
                    pageStyle.width = ""100%"";
                    pageStyle.height = ""100%"";

                    // Simple function to center the element contained in the page div. Also serves as the callback for page.div.onresize + hosted element.onresize.
                    page.centerElement = function () {
                        if (page.element && page.element.style !== undefined) {
                            var x = 0,
                                y = 0;
                            if (page.element.offsetWidth < page.div.offsetWidth) {
                                x = Math.floor((page.div.offsetWidth - page.element.offsetWidth) / 2);
                            }

                            if (page.element.offsetHeight < page.div.offsetHeight) {
                                y = Math.floor((page.div.offsetHeight - page.element.offsetHeight) / 2);
                            }
                            page.element.style.left = x + ""px"";
                            page.element.style.top = y + ""px"";
                        }
                    };

                    // Sets the element to display in this flip page
                    page.setElement = function (element, isReplacement) {
                        if (element === undefined) {
                            element = null;
                        }
                        if (element === page.element) {
                            return;
                        }
                        if (page.element) {
                            if (!isReplacement) {
                                itemsManager.releaseItem(page.element);
                            }
                            page.element.detachEvent(""onresize"", page.centerElement);
                            page.element.flipperResizeHandlerSet = false;
                        }
                        page.element = element;
                        utilities.empty(page.div);

                        if (page.element) {
                            page.div.appendChild(page.element);
                            if (!manager._itemInView(page)) {
                                utilities.disableTab(page.element);
                            }
                            if (page.element.style !== undefined) {
                                page.element.style.position = ""absolute"";
                            }
                            if (page.element.flipperResizeHandlerSet === undefined || page.element.flipperResizeHandlerSet === false) {
                                page.element.attachEvent(""onresize"", page.centerElement);
                                page.element.flipperResizeHandlerSet = true;
                            }
                            page.centerElement();
                            if (isFlipper(page.element)) {
                                // Our current element is a nested flipper. Normally, a flipper element should be keyboard focusable. When the item returned to us
                                // by the IM is a flipper, however, things change. When that happens, the developer's trying to make a 2D flipper. If we don't do anything,
                                // the keyboard interactions will be weird. If you tab to the outer flipper, two nav keys will work. Tabbing again will make the inner flipper
                                // focused (but apparently not have any visible change to the user) and suddenly the two perpendicular nav keys work. 
                                // That's not the interaction model we want, so we need to do some sorcery here. 
                                // We'll check if the item is a flipper. If it is, we'll remove it from the tab order. The flipper will then be responsible for intercepting nav events
                                // and routing it to the page manager (see Flipper.js's keyup event handler)
                                page.element.setAttribute(""tabindex"", -1);
                            }
                        }
                    };

                    page.div.attachEvent(""onresize"", page.centerElement);
                    return page;
                },

                _itemInView: function (flipPage) {
                    return this._itemEnd(flipPage) > this._viewportStart() && this._itemStart(flipPage) < this._viewportEnd();
                },

                _initializeHorizontalFunctions: function () {
                    // In IE's RTL mode, scrollLeft starts at 0 on the very right-hand side of the scrolled element. 
                    // That also means that scrollLeft is actually on the RIGHT border of the viewing region (rather than the left like in LTR).
                    // Positions inside the scrolled element still react as expected. Setting item.style.left = 0 will position the item at the left side of the 
                    // region, but in order to make that item visible the scrollLeft of the scrolling region needs to be at scrollRegionWidth-viewportWidth
                    // (eg, if we had a 10000px panning region, 200px wide items, and a 200px viewport, an item positioned with item.style.left = 0px would
                    // require the viewport scrollLeft = 9800 in order for that item to be displayed.
                    // Since items are still positioned using an LTR scheme, we'll make a wrapper around scrollLeft/Right here so that viewportStart/End
                    // can be used by the rest of the flipper code as if there were no change to RTL in the first place.
                    this._viewportStart = function (newValue) {
                        if (newValue === undefined) {
                            return this._rtl ? (this._panningDiv.offsetWidth - (this._panningDivContainer.scrollLeft + this._panningDivContainer.offsetWidth)) : this._panningDivContainer.scrollLeft;
                        }

                        if (this._rtl) {
                            newValue = (this._panningDiv.offsetWidth - newValue) - this._panningDivContainer.offsetWidth;
                        }
                        this._panningDivContainer.scrollLeft = newValue;
                    };
                    this._viewportEnd = function () {
                        if (this._rtl) {
                            return this._viewportStart() + this._panningDivContainer.offsetWidth;
                        } else {
                            return utilities.scrollRight(this._panningDivContainer);
                        }
                    };
                    this._itemStart = function (flipPage, newValue, margin) {
                        if (newValue === undefined) {
                            return flipPage.div.offsetLeft;
                        }

                        flipPage.div.style.left = this._rtl ? (-newValue) + ""%"" : newValue + ""%"";
                        flipPage.margin = margin;
                        flipPage.div.style[this._rtl ? ""marginRight"" : ""marginLeft""] = margin + ""px"";
                        flipPage.offsetValue = newValue;
                    };
                    this._itemEnd = function (flipPage) {
                        return utilities.offsetRight(flipPage.div);
                    };
                    this._itemSize = function (flipPage) {
                        return flipPage.div.offsetWidth;
                    };
                    this._panningDivEnd = function () {
                        return this._panningDiv.offsetWidth;
                    };
                    this._movePageAhead = function (referencePage, pageToPlace) {
                        this._itemStart(pageToPlace, referencePage.offsetValue + 100, referencePage.margin + this._itemSpacing);
                    };
                    this._movePageBehind = function (referencePage, pageToPlace) {
                        this._itemStart(pageToPlace, referencePage.offsetValue - 100, referencePage.margin - this._itemSpacing);
                    };
                    this.stopKeyEventBubble = function (keyCode) {
                        var Key = utilities.Key;
                        if (keyCode === Key.leftArrow || keyCode === Key.rightArrow) {
                            return true;
                        }

                        if (this._currentPage.element && isFlipper(this._currentPage.element)) {
                            var nestedFlipper = Win.UI.Controls.Flipper(this._currentPage.element),
                                flipperOptions = nestedFlipper.options();
                            if (flipperOptions.orientation === ""vertical"") {
                                if (keyCode === Key.upArrow || keyCode === Key.downArrow) {
                                    return true;
                                }
                            }
                        }

                        return false;
                    };
                    var leftArrowNavigationFunction = this._rtl ? ""next"" : ""previous"",
                        rightArrowNavigationFunction = this._rtl ? ""previous"" : ""next"";
                    this.handleKeyboardNavigation = function (keyCode) {
                        var navigated = false,
                            Key = utilities.Key;
                        if (keyCode === Key.leftArrow) {
                            this[leftArrowNavigationFunction]();
                            navigated = true;
                        } else if (keyCode === Key.rightArrow) {
                            this[rightArrowNavigationFunction]();
                            navigated = true;
                        } else if (this._currentPage.element && isFlipper(this._currentPage.element)) {
                            var nestedFlipper = Win.UI.Controls.Flipper(this._currentPage.element),
                                flipperOptions = nestedFlipper.options();
                            if (flipperOptions.orientation === ""vertical"") {
                                if (keyCode === Key.upArrow) {
                                    nestedFlipper.previous();
                                } else {
                                    nestedFlipper.next();
                                }
                                navigated = true;
                            }
                        }

                        if (navigated) {
                            this._flipperDiv.focus();
                        }

                        return navigated;
                    };
                },

                _initializeVerticalFunctions: function () {
                    this._viewportStart = function (newValue) {
                        if (newValue === undefined) {
                            return this._panningDivContainer.scrollTop;
                        }

                        this._panningDivContainer.scrollTop = newValue;
                    };
                    this._viewportEnd = function () {
                        return utilities.scrollBottom(this._panningDivContainer);
                    };
                    this._itemStart = function (flipPage, newValue, margin) {
                        if (newValue === undefined) {
                            return flipPage.div.offsetTop;
                        }

                        flipPage.div.style.top = newValue + flipPage.pageAxisOffset + ""%"";
                        flipPage.margin = margin;
                        flipPage.div.style.marginTop = margin + ""px"";
                        flipPage.offsetValue = newValue;
                    };
                    this._itemEnd = function (flipPage) {
                        return utilities.offsetBottom(flipPage.div);
                    };
                    this._itemSize = function (flipPage) {
                        return flipPage.div.offsetHeight;
                    };
                    this._panningDivEnd = function () {
                        return this._panningDiv.offsetHeight;
                    };
                    this._movePageAhead = function (referencePage, pageToPlace) {
                        this._itemStart(pageToPlace, referencePage.offsetValue + 100, referencePage.margin + this._itemSpacing);
                    };
                    this._movePageBehind = function (referencePage, pageToPlace) {
                        this._itemStart(pageToPlace, referencePage.offsetValue - 100, referencePage.margin - this._itemSpacing);
                    };
                    this.stopKeyEventBubble = function (keyCode) {
                        var Key = utilities.Key;
                        if (keyCode === Key.upArrow || keyCode === Key.downArrow) {
                            return true;
                        }

                        if (this._currentPage.element && isFlipper(this._currentPage.element)) {
                            var nestedFlipper = Win.UI.Controls.Flipper(this._currentPage.element),
                                flipperOptions = nestedFlipper.options();
                            if (flipperOptions.orientation === ""horizontal"") {
                                if (keyCode === Key.leftArrow || keyCode === Key.rightArrow) {
                                    return true;
                                }
                            }
                        }

                        return false;
                    };
                    var leftArrowNavigationFunction = this._rtl ? ""next"" : ""previous"",
                        rightArrowNavigationFunction = this._rtl ? ""previous"" : ""next"";
                    this.handleKeyboardNavigation = function (keyCode) {
                        var navigated = false,
                            Key = utilities.Key;
                        if (keyCode === Key.upArrow) {
                            this.previous();
                            navigated = true;
                        } else if (keyCode === Key.downArrow) {
                            this.next();
                            navigated = true;
                        } else if (this._currentPage.element && isFlipper(this._currentPage.element)) {
                            var nestedFlipper = Win.UI.Controls.Flipper(this._currentPage.element),
                                flipperOptions = nestedFlipper.options();
                            if (flipperOptions.orientation === ""horizontal"") {
                                if (keyCode === Key.leftArrow) {
                                    nestedFlipper[leftArrowNavigationFunction]();
                                } else {
                                    nestedFlipper[rightArrowNavigationFunction]();
                                }
                                navigated = true;
                            }
                        }

                        if (navigated) {
                            this._flipperDiv.focus();
                        }

                        return navigated;
                    };
                }
            },

        // Construction

            function (flipperDiv, panningDiv, panningDivContainer, itemsManager, keepInMemory, itemSpacing, buttonVisibilityHandler) {
                this._totalFlipPages = 0;
                this._visibleElements = [];
                this._flipperDiv = flipperDiv;
                this._panningDiv = panningDiv;
                this._panningDivContainer = panningDivContainer;
                this._buttonVisibilityHandler = buttonVisibilityHandler;
                this._keepItemsInMemory = keepInMemory;
                this._currentPage = null;
                this._rtl = window.getComputedStyle(this._flipperDiv, null).direction === ""rtl"";
                this._itemsManager = itemsManager;
                this._itemSpacing = itemSpacing;
            },

        // Statics

            {
                flipPageBufferCount: 2 // The number of items that should surround the current item as a buffer at any time
            }
        )
    });
})(Win);(function (Win) {
    var thisWinUI = Win.UI;
    var utilities = thisWinUI.Utilities;

    // Class names
    var navButtonLeftClass = ""win8-flipper-navleft"",
        navButtonRightClass = ""win8-flipper-navright"",
        navButtonTopClass = ""win8-flipper-navtop"",
        navButtonBottomClass = ""win8-flipper-navbottom"",
        placeholderContainerClass = ""win8-flipper-progresscontainer"",
        placeholderProgressbarClass = ""win8-flipper-progressbar"";

    // Aria labels
    var flipperLabel = ""Flipper"",
        previousButtonLabel = ""Previous"",
        nextButtonLabel = ""Next"";

    // Default renderers for Flipper
    function trivialHtmlRenderer(getIndex, key, dataObject, itemID) {
        return dataObject;
    }

    function trivialPlaceholderRenderer(renderInfo) {
        var placeholderDiv = document.createElement(""div"");
        placeholderDiv.className = placeholderContainerClass;
        placeholderDiv.innerHTML = ""<progress class='"" + placeholderProgressbarClass + ""'>"";
        return placeholderDiv;
    }

    Win.Namespace.defineWithParent(thisWinUI, ""Controls"", {

        Flipper: Win.Class.define(null, {

            // Public methods

            next: function () {
                return this._pageManager.next();
            },

            previous: function () {
                return this._pageManager.previous();
            },

            currentPage: {
                get: function () {
                    return this._getCurrentIndex();
                },
                set: function (index) {
                    return this._setCurrentIndex(index);
                }
            },

            count: function (callbacks) {
                if (typeof callbacks.success !== ""function"") {
                    throw new Error(thisWinUI.Controls.Flipper.badCallbacksObject);
                }

                return this._count(callbacks);
            },

            options: function (options) {
                if (options === undefined) {
                    return this._getOptions();
                } else {
                    if (options.itemSpacing && !utilities.isNonNegativeInteger(options.itemSpacing)) {
                        throw new Error(thisWinUI.Controls.Flipper.badItemSpacingAmount);
                    }
                    if (options.currentPage && !utilities.isNonNegativeInteger(options.currentPage)) {
                        throw new Error(thisWinUI.Controls.Flipper.badCurrentPage);
                    }

                    if (!options.dataSource && (options.itemRenderer || options.placeholderRenderer)) {
                        options.dataSource = this._dataSource;
                    }

                    if (options.dataSource && !options.itemRenderer) {
                        options.itemRenderer = this._itemRenderer;
                    }

                    if (options.dataSource && !options.placeholderRenderer) {
                        options.placeholderRenderer = this._placeholderRenderer;
                    }

                    if (options.itemRenderer && typeof options.itemRenderer === ""object"") {
                        options.itemRenderer = options.itemRenderer.renderItem;
                    }

                    this._setOptions(options);
                }
            },

            // Private members

            // Flipper internal ""enums""
            _flipAxisHorizontal: 0,
            _flipAxisVertical: 1,

            _initializeFlipper: function (element, flipAx, dataSource, itemRenderer, placeholderRenderer, initialIndex, keepInMemory, countToLoad, itemSpacing) {
                this._flipperDiv = element;
                this._contentDiv = document.createElement(""div"");
                this._panningDivContainer = document.createElement(""div"");
                this._panningDiv = document.createElement(""div"");
                this._prevButton = document.createElement(""button"");
                this._nextButton = document.createElement(""button"");
                this._flipAxis = flipAx;
                this._dataSource = dataSource;
                this._itemRenderer = itemRenderer;
                this._itemsManager = null;
                this._pageManager = null;
                this._cachedSize = -1;
                this._placeholderRenderer = placeholderRenderer;

                if (!this._flipperDiv.getAttribute(""tabindex"")) {
                    this._flipperDiv.setAttribute(""tabindex"", 0);
                }
                this._flipperDiv.setAttribute(""role"", ""list"");
                this._flipperDiv.setAttribute(""aria-label"", flipperLabel);
                if (!this._flipperDiv.style.overflow) {
                    this._flipperDiv.style.overflow = ""hidden"";
                }
                this._contentDiv.style.position = ""relative"";
                this._contentDiv.style.width = ""100%"";
                this._contentDiv.style.height = ""100%"";
                this._panningDiv.style.position = ""relative"";
                this._panningDivContainer.style.position = ""relative"";
                this._panningDivContainer.style.width = ""100%"";
                this._panningDivContainer.style.height = ""100%"";
                this._panningDivContainer.style.overflow = ""hidden"";

                this._contentDiv.appendChild(this._panningDivContainer);
                this._flipperDiv.appendChild(this._contentDiv);

                this._panningDiv.style.width = ""100%"";
                this._panningDiv.style.height = ""100%"";
                if (this._flipAxis === this._flipAxisHorizontal) {
                    var rtl = window.getComputedStyle(this._flipperDiv, null).direction === ""rtl"";
                    if (rtl) {
                        this._prevButton.className = navButtonRightClass;
                        this._nextButton.className = navButtonLeftClass;
                    } else {
                        this._prevButton.className = navButtonLeftClass;
                        this._nextButton.className = navButtonRightClass;
                    }
                } else {
                    this._prevButton.className = navButtonTopClass;
                    this._nextButton.className = navButtonBottomClass;
                }
                this._prevButton.setAttribute(""aria-label"", previousButtonLabel);
                this._nextButton.setAttribute(""aria-label"", nextButtonLabel);
                this._prevButton.setAttribute(""aria-hidden"", true);
                this._nextButton.setAttribute(""aria-hidden"", true);
                this._prevButton.style.visibility = ""hidden"";
                this._nextButton.style.visibility = ""hidden"";
                this._prevButton.style.opacity = 0.0;
                this._nextButton.style.opacity = 0.0;

                this._panningDivContainer.appendChild(this._panningDiv);
                this._contentDiv.appendChild(this._prevButton);
                this._contentDiv.appendChild(this._nextButton);

                var that = this;

                this._itemsManagerCallback = {
                    // Callbacks for itemsManager
                    inserted: function (element, prev, next) {
                        that._pageManager.inserted(element, prev, next);
                    },

                    countChanged: function (newCount, oldCount) {
                        that._cachedSize = newCount;
                        that._fireDatasourceCountChangedEvent();
                    },

                    changed: function (newElement, oldElement) {
                        that._pageManager.changed(newElement, oldElement);
                    },

                    moved: function (element, prev, next) {
                        that._pageManager.moved(element, prev, next);
                    },

                    removed: function (element) {
                        that._pageManager.removed(element);
                    },

                    indexChanged: function (element, newIndex) {
                        that._pageManager.indexChanged(element, newIndex);
                    },

                    knownUpdatesComplete: function () {
                    },

                    beginNotifications: function () {
                    },

                    endNotifications: function () {

                    },

                    itemAvailable: function (real, placeholder) {
                        that._pageManager.itemRetrieved(real, placeholder);
                    }
                };

                if (this._dataSource) {
                    this._itemsManager = thisWinUI.createItemsManager(this._dataSource, this._itemRenderer, this._itemsManagerCallback, {
                        placeholderRenderer: this._placeholderRenderer,
                        ownerElement: this._flipperDiv
                    });
                    this._itemsManager.count(function (count) {
                        that._cachedSize = count;
                    });
                }

                this._pageManager = new thisWinUI.Controls._FlipPageManager(this._flipperDiv, this._panningDiv, this._panningDivContainer, this._itemsManager, keepInMemory, itemSpacing,
                {
                    // TODO(jumyhres) M3: Animate these buttons fading in/out
                    hidePreviousButton: function () {
                        that._prevButton.style.visibility = ""hidden"";
                        that._prevButton.setAttribute(""aria-hidden"", true);
                    },

                    showPreviousButton: function () {
                        that._prevButton.style.visibility = ""visible"";
                        that._prevButton.setAttribute(""aria-hidden"", false);
                    },

                    hideNextButton: function () {
                        that._nextButton.style.visibility = ""hidden"";
                        that._nextButton.setAttribute(""aria-hidden"", true);
                    },

                    showNextButton: function () {
                        that._nextButton.style.visibility = ""visible"";
                        that._nextButton.setAttribute(""aria-hidden"", false);
                    }
                });

                if (this._flipAxis === this._flipAxisHorizontal) {
                    this._pageManager.initializeHorizontal(initialIndex, countToLoad);
                } else {
                    this._pageManager.initializeVertical(initialIndex, countToLoad);
                }

                this._prevButton.addEventListener(""click"", function () {
                    that.previous();
                }, false);

                this._nextButton.addEventListener(""click"", function () {
                    that.next();
                }, false);

                // resize / onresize doesn't get hit with addEventListener, but it does get hit via attachEvent, so we'll use that here.
                this._flipperDiv.attachEvent(""onresize"", function () {
                    that._resize();
                });

                this._contentDiv.addEventListener(""mouseenter"", function () {
                    //TODO (jumyhres) M3: Use CSS stuff for making buttons visible instead of messing around with the style directly
                    that._prevButton.style.opacity = 1.0;
                    that._nextButton.style.opacity = 1.0;
                }, false);

                this._contentDiv.addEventListener(""mouseleave"", function () {
                    that._prevButton.style.opacity = 0.0;
                    that._nextButton.style.opacity = 0.0;
                }, false);

                this._panningDivContainer.addEventListener(""scroll"", function () {
                    that._scrollPosChanged();
                }, false);

                // When an element is removed and inserted, its scroll position gets reset to 0 (and no onscroll event is generated). This is a major problem
                // for the flipper thanks to the fact that we 1) Do a lot of inserts/removes of child elements, and 2) Depend on our scroll location being right to
                // display the right stuff. The page manager preserves scroll location. When a flipper element is reinserted, it'll fire DOMNodeInserted and we can reset
                // its scroll location there.
                // This event handler won't be hit in IE8. 
                this._flipperDiv.addEventListener(""DOMNodeInserted"", function (event) {
                    if (event.target === that._flipperDiv) {
                        that._pageManager.resetScrollPos();
                        that._pageManager.centerContent();
                    }
                }, false);

                this._flipperDiv.addEventListener(""keydown"", function (event) {
                    if (that._pageManager.stopKeyEventBubble(event.keyCode)) {
                        event.stopPropagation();
                        event.preventDefault();
                        return false;
                    }
                }, false);

                this._flipperDiv.addEventListener(""keyup"", function (event) {
                    var Key = utilities.Key,
                        keyCode = event.keyCode;
                    if ((keyCode === Key.leftArrow || keyCode === Key.rightArrow || keyCode === Key.upArrow || keyCode === Key.downArrow) && that._pageManager.handleKeyboardNavigation(keyCode)) {
                        event.stopPropagation();
                        event.preventDefault();
                        return false;
                    }
                }, false);
            },

            _resize: function () {
                this._pageManager.centerContent();
            },

            _setCurrentIndex: function (index) {
                return this._pageManager.jumpToIndex(index);
            },

            _getCurrentIndex: function () {
                return this._pageManager.currentIndex();
            },

            _setOptions: function (options) {
                if (options.itemSpacing !== undefined) {
                    this._pageManager.setItemSpacing(options.itemSpacing);
                }

                if (options.dataSource !== undefined) {
                    var newIndex = 0;
                    if (options.currentPage !== undefined) {
                        newIndex = options.currentPage;
                    }
                    this._setDatasource(options.dataSource, options.itemRenderer, options.placeholderRenderer, newIndex);
                } else if (options.currentPage !== undefined) {
                    this._pageManager.jumpToIndex(options.currentPage);
                }
            },

            _getOptions: function () {
                var options = {};
                options.currentPage = this._getCurrentIndex();
                options.orientation = this._axisAsString();
                options.dataSource = this._dataSource;
                options.itemRenderer = this._itemRenderer;
                options.placeholderRenderer = this._placeholderRenderer;
                options.itemSpacing = this._pageManager.getItemSpacing();
                options.keepItemsInMemory = this._pageManager._keepItemsInMemory;
                return options;
            },

            _count: function (callbacks) {
                var error = """";
                if (this._itemsManager) {
                    if (this._cachedSize >= 0) {
                        callbacks.success(this._cachedSize);
                        return;
                    } else {
                        this._itemsManager.count(function (count) {
                            this._cachedSize = count;
                            if (count > 0) {
                                callbacks.success(count);
                            } else {
                                if (typeof callbacks.error === ""function"") {
                                    callbacks.error(thisWinUI.Controls.Flipper.noCountAvailable);
                                }
                            }
                        });
                        return;
                    }
                } else {
                    error = thisWinUI.Controls.Flipper.noitemsManagerForCount;
                }

                if (typeof callbacks.error === ""function"") {
                    callbacks.error(error);
                }
            },

            _setDatasource: function (source, template, placeholderRenderer, index) {
                var initialIndex = 0;
                if (index !== undefined) {
                    initialIndex = index;
                }
                this._dataSource = source;
                this._itemRenderer = template;
                this._placeholderRenderer = placeholderRenderer;
                this._itemsManager = thisWinUI.createItemsManager(this._dataSource, this._itemRenderer, this._itemsManagerCallback, {
                    placeholderRenderer: this._placeholderRenderer,
                    ownerElement: this._flipperDiv
                });

                var that = this;
                this._itemsManager.count(function (count) {
                    that._cachedSize = count;
                });
                this._pageManager.setNewItemsManager(this._itemsManager, initialIndex);
            },

            _fireDatasourceCountChangedEvent: function () {
                if (document.createEvent) {
                    var event = document.createEvent(""Event"");
                    event.initEvent(thisWinUI.Controls.Flipper.datasourceCountChangedEvent, true, true);
                    this._flipperDiv.dispatchEvent(event);
                }
            },

            _scrollPosChanged: function () {
                this._pageManager.scrollPosChanged();
            },

            _axisAsString: function () {
                if (this._flipAxis === this._flipAxisHorizontal) {
                    return ""horizontal"";
                } else {
                    return ""vertical"";
                }
            }
        },

        // Construction

        function (element, options) {
            if (!element) {
                throw new Error(thisWinUI.Controls.Flipper.noElement);
            }

            if (this === window || this === thisWinUI.Controls) {
                var flipper = utilities.getData(element, ""flipper"");
                if (flipper) {
                    return flipper;
                } else {
                    return new thisWinUI.Controls.Flipper(element, options);
                }
            }

            var flipAxis = this._flipAxisHorizontal,
                dataSource = null,
                itemRenderer = trivialHtmlRenderer,
                placeholderRenderer = null,
                initialIndex = 0,
                keepInMemory = false,
                itemSpacing = 0;

            if (options) {
                // flipAxis parameter checking. Must be a string, either ""horizontal"" or ""vertical""
                if (options.orientation) {
                    if (typeof options.orientation === ""string"") {
                        switch (options.orientation.toLowerCase()) {
                            case ""horizontal"":
                                flipAxis = this._flipAxisHorizontal;
                                break;

                            case ""vertical"":
                                flipAxis = this._flipAxisVertical;
                                break;

                            default:
                                throw new Error(thisWinUI.Controls.Flipper.badAxis);
                        }
                    } else {
                        throw new Error(thisWinUI.Controls.Flipper.badAxis);
                    }
                }

                // currentPage. Should be a number >= 0. If it's negative, we can throw an error now. If it's positive, we might throw an error later when it turns out that number's out of bounds
                if (options.currentPage) {
                    if (utilities.isNonNegativeInteger(options.currentPage)) {
                        initialIndex = Math.floor(options.currentPage); // A number isn't necessarily an int, so we'll force it to be so here.
                    } else {
                        throw new Error(thisWinUI.Controls.Flipper.badCurrentPage);
                    }
                }

                if (options.dataSource) {
                    dataSource = options.dataSource;
                }

                if (options.itemRenderer) {
                    if (typeof options.itemRenderer === ""function"") {
                        itemRenderer = options.itemRenderer;
                    }
                    else if (typeof options.itemRenderer === ""object"") {
                        itemRenderer = options.itemRenderer.renderItem;
                    }
                } 

                placeholderRenderer = options.placeholderRenderer ? options.placeholderRenderer : trivialPlaceholderRenderer;

                if (options.itemSpacing) {
                    if (utilities.isNonNegativeInteger(options.itemSpacing)) {
                        itemSpacing = Math.floor(options.itemSpacing);
                    } else {
                        throw new Error(thisWinUI.Controls.Flipper.badItemSpacingAmount);
                    }
                }

                if (options.keepItemsInMemory) {
                    if (typeof options.keepItemsInMemory === ""boolean"") {
                        keepInMemory = options.keepItemsInMemory;
                    } else {
                        throw new Error(thisWinUI.Controls.Flipper.badKeepItemsInMemory);
                    }
                }
            }

            var countToLoad = 0;
            if (!dataSource) {
                var childElements = utilities.children(element);
                if (childElements.length > 0) {
                    dataSource = thisWinUI.createObjectDataSource(childElements, { storeReferences: true });
                    keepInMemory = true;
                    countToLoad = childElements.length;
                }
            }
            utilities.empty(element);

            this._initializeFlipper(element, flipAxis, dataSource, itemRenderer, placeholderRenderer, initialIndex, keepInMemory, countToLoad, itemSpacing);

            this.addEventListener = function (eventName, eventHandler, useCapture) {
                return this._flipperDiv.addEventListener(eventName, eventHandler, useCapture);
            };

            this.removeEventListener = function (eventName, eventHandler, useCapture) {
                return this._flipperDiv.removeEventListener(eventName, eventHandler, useCapture);
            };

            utilities.setData(element, ""flipper"", this);
        },

        // Statics

        {
        // Events
        datasourceCountChangedEvent: ""datasourcecountchanged"",
        pageVisibilityChangedEvent: ""pagevisibilitychanged"",

        // Errors
        noElement: ""Invalid argument: A flipper requires a DOM element passed in as its first parameter"", // TODO: it seems like noElement should be a Controls namespace error instead of a dedicated flipper one
        badAxis: ""Invalid argument: orientation must be a string, either 'horizontal' or 'vertical'"",
        badCurrentPage: ""Invalid argument: currentPage must be a number greater than or equal to zero and be within the bounds of the datasource"",
        noitemsManagerForCount: ""Invalid operation: can't get count if no dataSource has been set"",
        noCountAvailable: ""A count is not yet available from the datasource"",
        badItemSpacingAmount: ""Invalid argument: itemSpacing must be a number greater than or equal to zero"",
        badCallbacksObject: ""Invalid argument: callbacks requires a .success function to call when count is retrieved"",
        badKeepItemsInMemory: ""Invalid argument: keepItemsInMemory must be a boolean""
    })
});
})(Win);(function (thisWinUI) {

// Utilities are private and global pointer will be deleted so we need to cache it locally
var utilities = thisWinUI.Utilities;

var elementIsInvalid = ""Invalid argument: Rating control expects a valid DOM element as the first argument."";
var maxRatingIsInvalid = ""Invalid argument: maxRating must be an integer number greater than zero."";
var maxRatingCannotBeUpdated = ""Invalid argument: maxRating cannot be set after instantiation."";
var userRatingIsInvalid = ""Invalid argument: userRating must be null or an integer number greater than zero and less than or equal to maxRating."";
var averageRatingIsInvalid = ""Invalid argument: averageRating must be null or a number greater than or equal to one and less than or equal to maxRating."";
var readOnlyIsInvalid = ""Invalid argument: readOnly must be a boolean value."";
var defaultMaxRating = 5;
var defaultReadOnly = false;
var defaultHeart = {
    paddingLeft: ""2px"",
    paddingRight: ""2px"",
    paddingTop: ""3px"",
    paddingBottom: ""3px"",
    offsetWidth: 19,
    offsetHeight: 19,
    width: ""15px"",
    height: ""13px"",
    widthUnit: ""px"",
    widthValue: 15
};

var msRating = ""win8-rating"";
var msRatingAverageEmpty = ""win8-rating-average-empty"";
var msRatingAverageFull = ""win8-rating-average-full"";
var msRatingUserEmpty = ""win8-rating-user-empty"";
var msRatingUserFull = ""win8-rating-user-full"";
var msRatingTentativeEmpty = ""win8-rating-tentative-empty"";
var msRatingTentativeFull = ""win8-rating-tentative-full"";
var msRatingDisabledEmpty = ""win8-rating-disabled-empty"";
var msRatingDisabledFull = ""win8-rating-disabled-full"";// Rating control implementation
Win.Namespace.defineWithParent(thisWinUI, ""Controls"", {
    Rating: Win.Class.define(null, {
        _cancel: ""cancel"",
        _change: ""change"",
        _previewChange: ""previewchange"",

        _customEvents: null,
        _element: null,
        _elements: null,
        _heart: null,
        _floatingStar: null,
        _lastEventWasChange: false,
        _options: null,
        _tempRating: 1,
        _captured: false,
        _mouseDownFocus: false,

        addEventListener: function RatingControl_addEventListener(eventName, eventCallBack, capture) {
            if ((eventName === this._previewChange) || (eventName === this._change) || (eventName === this._cancel)) {
                this._element.addEventListener(eventName, eventCallBack, capture);
            }
        },

        _createControl: function RatingControl_createControl() {
            // rating control could have more than one class name
            utilities.addClass(this._element, msRating);

            // current state of class names
            this._elementsClassName = {};
            // array of elements - hearts
            this._elements = {};
            // create control
            for (var i = 0; i < this._options.maxRating; i++) {
                var oneStar = document.createElement(""div"");
                this._element.appendChild(oneStar);
                oneStar.id = this._element.id + ""_"" + i;
                oneStar.className = msRatingUserEmpty;
                this._elementsClassName[i] = msRatingUserEmpty;
                this._elements[i] = oneStar;
            }
            // add one more star
            var helpStar = document.createElement(""div"");
            this._element.appendChild(helpStar);
            helpStar.id = this._element.id + ""_"" + this._options.maxRating;
            helpStar.className = msRatingAverageFull;
            this._elements[this._options.maxRating] = helpStar;

            // default heart properties
            this._heart = {
                paddingLeft: defaultHeart.paddingLeft,
                paddingRight: defaultHeart.paddingRight,
                paddingTop: defaultHeart.paddingTop,
                paddingBottom: defaultHeart.paddingBottom,
                offsetWidth: defaultHeart.offsetWidth,
                offsetHeight: defaultHeart.offsetHeight,
                width: defaultHeart.width,
                height: defaultHeart.height,
                widthUnit: defaultHeart.widthUnit,
                widthValue: defaultHeart.widthValue
            };

            // we will use this variable for keyboard interaction
            this._tempRating = -1;

            // add focus capability relative to element's position in the document
            this._element.tabIndex = ""0"";

            // we are storing current size of floting star
            this._floatingStar = 0;

            // is the change the last fired event
            this._lastEventWasChange = false;

            // calculate control size based on css styles and default sizes
            this._reCalculateControlSize();

            // check the mouse capture
            this._captured = false;

            this._mouseDownFocus = false;
        },

        // decrease temproray rating by one
        _decreaseRating: function RatingControl_decreaseRating() {
            if (this._tempRating > 1) {
                this._tempRating--;
            } else {
                if (this._tempRating == -1) {
                    if (this._options.userRating !== null) {
                        if (this._options.userRating > 0) {
                            this._tempRating = this._options.userRating - 1;
                        } else {
                            this._tempRating = 0;
                        }
                    } else {
                        this._tempRating = 0;
                    }
                }
            }
            this._showTemRating();
        },

        _events: function RatingControl_events() {
            var that = this;
            function ratingHandler(eventName) {
                return {
                    name: eventName.toLowerCase(),
                    handler: function (event) {
                        var fn = that[""_on"" + eventName];
                        if (fn) {
                            fn.apply(that, [event]);
                        }
                    }
                };
            }

            var events = [
                    ratingHandler(""MouseOver""),
                    ratingHandler(""MouseOut""),
                    ratingHandler(""MouseDown""),
                    ratingHandler(""MouseUp""),
                    ratingHandler(""Click""),
                    ratingHandler(""KeyDown""),
                    ratingHandler(""Blur""),
                    ratingHandler(""Focus"")
                ];

            var propertyEvent = ratingHandler(""PropertyChange"");

            for (var i = 0; i < events.length; ++i) {
                this._element.addEventListener(events[i].name, events[i].handler, false);
            }
            // PropertyChange event is not fired for element when addEventListener is used.
            this._element.attachEvent(""on"" + propertyEvent.name, propertyEvent.handler);
        },

        _getOptions: function RatingControl_getOptions() {
            return this._options;
        },

        _getStarNumber: function RatingControl_getStarNumber(star) {
            for (var i = 0; i < this._options.maxRating; i++) {
                if (this._elements[i] === star) {
                    return i;
                }
            }
            // check if it is the average star
            if (this._elements[this._options.maxRating] === star) {
                return Math.floor(this._options.averageRating);
            }

            return -1;
        },

        _hideHelpStar: function RatingControl_hideHelpStar() {
            // check if this average rating control
            if (this._options.averageRating !== null) {
                // hide the empty star
                this._resetHelpStar();
            }
        },

        // increase temporary rating by one
        _increaseRating: function RatingControl_increaseRating() {
            if (this._tempRating != -1) {
                if (this._tempRating < this._options.maxRating) {
                    this._tempRating++;
                }
            } else {
                if (this._options.userRating !== null) {
                    if (this._options.userRating < this._options.maxRating) {
                        this._tempRating = this._options.userRating + 1;
                    } else {
                        this._tempRating = this._options.maxRating;
                    }
                } else {
                    this._tempRating = 1;
                }
            }
            this._showTemRating();
        },

        _onPropertyChange: function RatingControl_onPropertyChange() {
            if ((event.propertyName === ""dir"") || (event.propertyName === ""style.direction"")) {
                this._updateControl();
            }
        },

        _onMouseDown: function RatingControl_onMouseDown() {
            this._captured = true;
            this._mouseDownFocus = true;
            this._element.focus();
            this._element.setCapture();
        },

        _onMouseUp: function RatingControl_onMouseUp() {
            this._captured = false;
            document.releaseCapture();
        },

        _onBlur: function RatingControl_onBlur() {
            this._showCurrentRating();
            if (!this._options.readOnly && !this._lastEventWasChange) {
                this._raiseEvent(this._cancel, null);
            }
        },

        _onClick: function RatingControl_onMouseClick() {
            // check onchange event
            var fireOnChange = false,
                starNum;

            // if the control read only then we dont change anything
            if (!this._options.readOnly) {
                // check for drag and drop
                if (this._element === window.event.srcElement) {
                    starNum = this._tempRating - 1;
                } else {
                    // check on what star is the mouse
                    starNum = this._getStarNumber(window.event.srcElement);
                }

                if (starNum >= 0) {
                    if (this._options.userRating !== (starNum + 1)) {
                        fireOnChange = true;
                    }
                    this._tempRating = starNum + 1;
                    // update userRating
                    this._options.userRating = starNum + 1;
                    this._options.userRating = this._options.userRating;
                    // change states for all previous stars
                    this._setStarClasses(msRatingUserFull, starNum, msRatingUserEmpty);
                    this._setCurrentStarClasses(msRatingUserFull, starNum, msRatingUserEmpty);
                    // hide help star
                    this._hideHelpStar();
                }
            }

            // should we fire onchange event
            if (fireOnChange) {
                this._raiseEvent(this._change, this._options.userRating);
            }
        },

        _onFocus: function RatingControl_onFocus() {
            // if the control is read only don't hover stars
            if (!this._options.readOnly) {
                // change states for all previous stars
                // but only if user didnt vote
                if (this._options.userRating === null) {
                    for (var i = 0; i < this._options.maxRating; i++) {
                        this._elements[i].className = msRatingTentativeEmpty;
                    }
                }
                // hide the help star
                this._hideHelpStar();
            }

            if (!this._mouseDownFocus) {
                if (this._options.userRating !== null) {
                    this._raiseEvent(this._previewChange, this._options.userRating);
                } else {
                    this._raiseEvent(this._previewChange, 0);
                }
            }
            this._mouseDownFocus = false;
        },

        _onKeyDown: function RatingControl_onKeyDown(eventObject) {
            var Key = utilities.Key;
            var keyCode = eventObject.keyCode;
            var rtlString = this._element.currentStyle.direction;
            var handled = true;
            switch (keyCode) {
                case Key.enter: // Enter
                    // check temporary rating
                    if (this._tempRating > 0) { // if it is 0 do not do anything
                        // check onchange event
                        var fireOnChange = false;
                        if (this._options.userRating !== this._tempRating) {
                            fireOnChange = true;
                        }
                        this._setOptions({ userRating: this._tempRating });
                        if (fireOnChange) {
                            this._raiseEvent(this._change, this._options.userRating);
                        }
                    }
                    break;
                case Key.escape: // escape
                    this._showCurrentRating();

                    if (!this._options.readOnly && !this._lastEventWasChange) {
                        this._raiseEvent(this._cancel, null);
                    }

                    break;
                case Key.leftArrow: // Arrow Left
                    if (rtlString === ""rtl"") {
                        this._increaseRating();
                    } else {
                        this._decreaseRating();
                    }
                    break;
                case Key.upArrow: // Arrow Up
                    this._increaseRating();
                    break;
                case Key.rightArrow: // Arrow Right
                    if (rtlString === ""rtl"") {
                        this._decreaseRating();
                    } else {
                        this._increaseRating();
                    }
                    break;
                case Key.downArrow: // Arrow Down
                    this._decreaseRating();
                    break;
                case Key.num1: case Key.numPad1: // number 1
                    this._tempRating = Math.min(1, this._options.maxRating);
                    this._showTemRating();
                    break;
                case Key.num2: case Key.numPad2: // number 2
                    this._tempRating = Math.min(2, this._options.maxRating);
                    this._showTemRating();
                    break;
                case Key.num3: case Key.numPad3: // number 3
                    this._tempRating = Math.min(3, this._options.maxRating);
                    this._showTemRating();
                    break;
                case Key.num4: case Key.numPad4: // number 4
                    this._tempRating = Math.min(4, this._options.maxRating);
                    this._showTemRating();
                    break;
                case Key.num5: case Key.numPad5: // number 5
                    this._tempRating = Math.min(5, this._options.maxRating);
                    this._showTemRating();
                    break;
                case Key.num6: case Key.numPad6: // number 6
                    this._tempRating = Math.min(6, this._options.maxRating);
                    this._showTemRating();
                    break;
                case Key.num7: case Key.numPad7: // number 7
                    this._tempRating = Math.min(7, this._options.maxRating);
                    this._showTemRating();
                    break;
                case Key.num8: case Key.numPad8: // number 8
                    this._tempRating = Math.min(8, this._options.maxRating);
                    this._showTemRating();
                    break;
                case Key.num9: case Key.numPad9: // number 9
                    this._tempRating = Math.min(9, this._options.maxRating);
                    this._showTemRating();
                    break;
                default:
                    handled = false;
            }

            if (handled) {
                eventObject.stopPropagation();
                eventObject.preventDefault();
            }
        },

        _onMouseOut: function RatingControl_onMouseOut() {
            this._showCurrentRating();
            // do not fire cancel event if we are changing rating on the same control
            if (((this._getStarNumber(window.event.fromElement) < 0) || (this._getStarNumber(window.event.toElement) < 0)) && !this._lastEventWasChange && !this._captured) {
                this._raiseEvent(this._cancel, this._tempRating);
            }
        },

        _onMouseOver: function RatingControl_onMouseOver() {
            // check on what star is the mouse
            var starNum = this._getStarNumber(window.event.srcElement);
            if (starNum >= 0) {
                // increase number by one (stars beginning from 0)
                this._tempRating = starNum + 1;

                // if the control is read only don't hover stars
                if (!this._options.readOnly) {
                    // change states for all stars
                    this._setStarClasses(msRatingTentativeFull, starNum, msRatingTentativeEmpty);
                    // hide help star
                    this._hideHelpStar();
                }

                this._raiseEvent(this._previewChange, this._tempRating);
            }
        },

        options: function RatingControl_options(options) {
            if (options) {
                if (""maxRating"" in options) {
                    throw new Error(maxRatingCannotBeUpdated);
                }
                this._setOptions(options);
            } else {
                return Object.create(this._getOptions());
            }
        },

        _raiseEvent: function RatingControl_raiseEvent(eventName, tempRating) {
            this._lastEventWasChange = (eventName === this._change);
            if (document.createEvent) {
                var event = document.createEvent(""Event"");
                event.initEvent(eventName, false, false);
                event.tempRating = tempRating;
                this._element.dispatchEvent(event);
            }
        },

        _reCalculateControlSize: function RatingControl_reCalculateControlSize() {
            // find the heart properties in the CSS styles
            var helpHeart = this._elements[this._options.maxRating];
            var ratingStyles = [msRatingAverageEmpty, msRatingAverageFull, msRatingDisabledEmpty, msRatingDisabledFull,
                                msRatingTentativeEmpty, msRatingTentativeFull, msRatingUserEmpty, msRatingUserFull];
            for (var ratingStyle in ratingStyles) {
                if (ratingStyle !== null) {
                    // check heart properties for each state
                    helpHeart.className = ratingStyles[ratingStyle];
                    if ((parseInt(helpHeart.currentStyle.width, 10) > 0) && (parseInt(helpHeart.currentStyle.height, 10) > 0)) {
                        if ((helpHeart.currentStyle.height !== defaultHeart.height) || (helpHeart.currentStyle.width !== defaultHeart.width) ||
                                (helpHeart.currentStyle.paddingLeft !== defaultHeart.paddingLeft) || (helpHeart.currentStyle.paddingRight !== defaultHeart.paddingRight) ||
                                (helpHeart.currentStyle.paddingTop !== defaultHeart.paddingTop) || (helpHeart.currentStyle.paddingBottom !== defaultHeart.paddingBottom)) {
                            this._heart.height = helpHeart.currentStyle.height;
                            this._heart.width = helpHeart.currentStyle.width;
                            this._heart.offsetWidth = helpHeart.offsetWidth;
                            this._heart.offsetHeight = helpHeart.offsetHeight;
                            this._heart.paddingLeft = helpHeart.currentStyle.paddingLeft;
                            this._heart.paddingRight = helpHeart.currentStyle.paddingRight;
                            this._heart.paddingTop = helpHeart.currentStyle.paddingTop;
                            this._heart.paddingBottom = helpHeart.currentStyle.paddingBottom;
                            this._heart.widthValue = parseInt(helpHeart.currentStyle.width, 10);
                            this._heart.widthUnit = helpHeart.currentStyle.width.substring(this._heart.widthValue.toString().length);
                        }
                    }
                }
            }

            // return defult state to floating star
            this._elements[this._options.maxRating].className = msRatingAverageFull;

            // calculate control size
            var controlWidth = parseInt(this._element.currentStyle.width, 10);
            var controlHeight = parseInt(this._element.currentStyle.height, 10);
            if (isNaN(controlWidth) && isNaN(controlHeight)) {
                controlWidth = this._options.maxRating * this._heart.offsetWidth;
                controlHeight = this._heart.offsetHeight;
            } else if (isNaN(controlWidth)) {
                controlHeight = this._element.offsetHeight;
                this._resizeHearts(controlHeight / this._heart.offsetHeight);
                controlWidth = this._heart.offsetWidth * this._options.maxRating;
            } else if (isNaN(controlHeight)) {
                controlWidth = this._element.offsetWidth;
                this._resizeHearts(controlWidth / (this._heart.offsetWidth * this._options.maxRating));
                controlHeight = this._heart.offsetHeight;
            } else {
                controlHeight = this._element.offsetHeight;
                controlWidth = this._element.offsetWidth;
                this._resizeHeartsHeight(controlHeight / this._heart.offsetHeight);
                this._resizeHeartsWidth(controlWidth / (this._heart.offsetWidth * this._options.maxRating));
            }

            this._element.style.width = controlWidth + ""px"";
            this._element.style.height = controlHeight + ""px"";

            // set elements size
            for (var i = 0; i <= this._options.maxRating; i++) {
                this._elements[i].style.width = this._heart.width;
                this._elements[i].style.height = this._heart.height;
                this._elements[i].style.paddingLeft = this._heart.paddingLeft;
                this._elements[i].style.paddingRight = this._heart.paddingRight;
                this._elements[i].style.paddingTop = this._heart.paddingTop;
                this._elements[i].style.paddingBottom = this._heart.paddingBottom;
                this._elements[i].style.backgroundPosition = this._heart.paddingLeft + "" "" + this._heart.paddingTop;
                this._elements[i].style.backgroundSize = this._heart.width + "" "" + this._heart.height;
                this._elements[i].style.display = ""inline-block"";
            }
            this._elements[this._options.maxRating].style.width = ""0px"";
            this._elements[this._options.maxRating].style.paddingLeft = ""0px"";
            this._elements[this._options.maxRating].style.paddingRight = ""0px"";
            this._elements[this._options.maxRating].style.display = ""none"";
        },

        _resetHelpStar: function RatingControl_resetHelpStar() {
            if (this._elements[this._options.maxRating].nextSibling !== null) {
                this._elements[this._options.maxRating].nextSibling.style.width = this._heart.width;
                this._elements[this._options.maxRating].nextSibling.style.paddingLeft = this._heart.paddingLeft;
                this._elements[this._options.maxRating].nextSibling.style.paddingRight = this._heart.paddingRight;
                this._elements[this._options.maxRating].nextSibling.style.backgroundPosition = this._heart.paddingLeft + "" "" + this._heart.paddingTop;
            }
            this._elements[this._options.maxRating].style.width = ""0px"";
            this._elements[this._options.maxRating].style.paddingLeft = ""0px"";
            this._elements[this._options.maxRating].style.paddingRight = ""0px"";
            this._elements[this._options.maxRating].style.display = ""none"";
        },

        _resizeHearts: function RatingControl_resizeHearts(factor) {
            this._resizeHeartsHeight(factor);
            this._resizeHeartsWidth(factor);
        },

        _resizeHeartsHeight: function RatingControl_resizeHeartsHeight(factor) {
            this._heart.height = this._resizeStringValue(this._heart.height, factor);
            this._heart.offsetHeight = this._heart.offsetHeight * factor;
            this._heart.paddingTop = this._resizeStringValue(this._heart.paddingTop, factor);
            this._heart.paddingBottom = this._resizeStringValue(this._heart.paddingBottom, factor);
        },

        _resizeHeartsWidth: function RatingControl_resizeHeartsWidth(factor) {
            this._heart.width = this._resizeStringValue(this._heart.width, factor);
            this._heart.offsetWidth = this._heart.offsetWidth * factor;
            this._heart.widthValue = parseFloat(this._heart.width, 10);
            this._heart.paddingLeft = this._resizeStringValue(this._heart.paddingLeft, factor);
            this._heart.paddingRight = this._resizeStringValue(this._heart.paddingRight, factor);
        },

        _resizeStringValue: function RatingControl_ResizeStringValue(string, factor) {
            var number = parseInt(string, 10);
            var unit = string.substring(number.toString(10).length);
            number = number * factor;
            return (number + unit);
        },

        _setControlSize: function RatingControl_setControlSize(options) {
            if (options !== undefined) {
                if (""maxRating"" in options) {
                    if ((typeof options.maxRating === ""number"") && (options.maxRating > 0) && (Math.floor(options.maxRating) === options.maxRating)) {
                        this._options.maxRating = options.maxRating;
                    } else {
                        throw new Error(maxRatingIsInvalid);
                    }
                }
            }
        },

        _setCurrentStarClasses: function RatingControl_setCurrentStarClasses(classNameBeforeThreshold, threshold, classNameAfterThreshold) {
            for (var i = 0; i < this._options.maxRating; i++) {
                if (i <= threshold) {
                    this._elementsClassName[i] = classNameBeforeThreshold;
                } else {
                    this._elementsClassName[i] = classNameAfterThreshold;
                }
            }
        },

        _setOptions: function RatingControl_setOptions(options) {
            this._validateOptions(options);
            if (options !== undefined) {
                if (""userRating"" in options) {
                    this._options.userRating = options.userRating;
                }
                if (""averageRating"" in options) {
                    this._options.averageRating = options.averageRating;
                }
                if (""readOnly"" in options) {
                    this._options.readOnly = options.readOnly;
                }
            }

            this._updateControl();
        },

        _setStarClasses: function RatingControl_setStarClasses(classNameBeforeThreshold, threshold, classNameAfterThreshold) {
            for (var i = 0; i < this._options.maxRating; i++) {
                if (i <= threshold) {
                    this._elements[i].className = classNameBeforeThreshold;
                } else {
                    this._elements[i].className = classNameAfterThreshold;
                }
            }
        },

        // show current rating
        _showCurrentRating: function RatingControl_showCurrentRating() {
            // reset temporary rating
            this._tempRating = -1;
            // if the control is read only then we didn't change anything on hover
            if (!this._options.readOnly) {
                if (this._elementsClassName !== null) {
                    for (var i = 0; i < this._options.maxRating; i++) {
                        this._elements[i].className = this._elementsClassName[i];
                    }
                    // check for average value
                    if ((this._options.averageRating !== null) && (this._options.userRating === null)) {
                        if (this._element.currentStyle.direction == ""rtl"") {
                            this._elements[this._options.maxRating].style.backgroundPosition = ""-"" + (this._heart.widthValue - this._floatingStar) + this._heart.widthUnit + "" "" + this._heart.paddingTop;
                            this._elements[this._options.maxRating].style.paddingLeft = this._heart.paddingRight;
                            this._elements[this._options.maxRating].nextSibling.style.paddingRight = ""0px"";
                        } else {
                            this._elements[this._options.maxRating].nextSibling.style.backgroundPosition = ""-"" + this._floatingStar + this._heart.widthUnit + "" "" + this._heart.paddingTop;
                            this._elements[this._options.maxRating].style.paddingLeft = this._heart.paddingLeft;
                            this._elements[this._options.maxRating].nextSibling.style.paddingLeft = ""0px"";
                        }
                        this._elements[this._options.maxRating].style.width = this._floatingStar + this._heart.widthUnit;
                        this._elements[this._options.maxRating].style.display = ""inline-block"";
                        this._elements[this._options.maxRating].nextSibling.style.width = (this._heart.widthValue - this._floatingStar) + this._heart.widthUnit;
                    }
                }
            }
        },

        _showTemRating: function RatingControl_showTemRating() {
            // we are inside this function if the number or the arrow key is pressed (number from 1-9)
            // if the control is read only don't hover stars
            if ((!this._options.readOnly) && (this._tempRating >= 0)) {
                this._setStarClasses(msRatingTentativeFull, this._tempRating - 1, msRatingTentativeEmpty);

                // hide the empty star
                this._hideHelpStar();
            }

            if (!this._options.readOnly) {
                this._raiseEvent(this._previewChange, this._tempRating);
            }
        },

        _updateControl: function RatingControl_updateControl() {
            var i;
            // check for average rating (if user rating is specified then we are not showing average rating)
            if ((this._options.averageRating !== null) && (this._options.userRating === null)) {
                if ((this._options.averageRating >= 1) && (this._options.averageRating <= this._options.maxRating)) {
                    for (i = 0; i < this._options.maxRating; i++) {
                        if ((i + 1) < this._options.averageRating) {
                            this._elements[i].className = msRatingAverageFull;
                            this._elementsClassName[i] = msRatingAverageFull;
                        } else {
                            this._elements[i].className = msRatingAverageEmpty;
                            this._elementsClassName[i] = msRatingAverageEmpty;
                        }
                        // check if it is floating star
                        if ((i < this._options.averageRating) && ((i + 1) >= this._options.averageRating)) {
                            if (this._elements[this._options.maxRating].nextSibling !== null) {
                                this._elements[this._options.maxRating].nextSibling.style.width = this._heart.width;
                                this._elements[this._options.maxRating].nextSibling.style.paddingLeft = this._heart.paddingLeft;
                                this._elements[this._options.maxRating].nextSibling.style.paddingRight = this._heart.paddingRight;
                                this._elements[this._options.maxRating].nextSibling.style.backgroundPosition = this._heart.paddingLeft + "" "" + this._heart.paddingTop;
                            }

                            this._element.insertBefore(this._elements[this._options.maxRating], this._elements[i]);

                            var floatingValue = this._options.averageRating - i;
                            this._floatingStar = floatingValue * this._heart.widthValue;
                            if (this._element.currentStyle.direction == ""rtl"") {
                                this._elements[this._options.maxRating].style.backgroundPosition = ""-"" + (this._heart.widthValue - this._floatingStar) + this._heart.widthUnit + "" "" + this._heart.paddingTop;
                                this._elements[this._options.maxRating].style.paddingLeft = this._heart.paddingRight;
                                this._elements[i].style.paddingRight = ""0px"";
                            } else {
                                this._elements[this._options.maxRating].style.backgroundPosition = this._heart.paddingLeft + "" "" + this._heart.paddingTop;
                                this._elements[i].style.backgroundPosition = ""-"" + this._floatingStar + this._heart.widthUnit + "" "" + this._heart.paddingTop;
                                this._elements[this._options.maxRating].style.paddingLeft = this._heart.paddingLeft;
                                this._elements[i].style.paddingLeft = ""0px"";
                            }
                            this._elements[this._options.maxRating].style.width = this._floatingStar + this._heart.widthUnit;
                            this._elements[this._options.maxRating].style.display = ""inline-block"";
                            this._elements[i].style.width = (this._heart.widthValue - this._floatingStar) + this._heart.widthUnit;
                        }
                    }
                }
            }

            // check if it is user rating control
            if (this._options.userRating !== null) {
                if ((this._options.userRating >= 1) && (this._options.userRating <= this._options.maxRating)) {
                    for (i = 0; i < this._options.maxRating; i++) {
                        if (i < this._options.userRating) {
                            if (this._options.readOnly) {
                                this._elements[i].className = msRatingDisabledFull;
                                this._elementsClassName[i] = msRatingDisabledFull;
                            } else {
                                this._elements[i].className = msRatingUserFull;
                                this._elementsClassName[i] = msRatingUserFull;
                            }
                        } else {
                            if (this._options.readOnly) {
                                this._elements[i].className = msRatingDisabledEmpty;
                                this._elementsClassName[i] = msRatingDisabledEmpty;
                            } else {
                                this._elements[i].className = msRatingUserEmpty;
                                this._elementsClassName[i] = msRatingUserEmpty;
                            }
                        }
                    }

                    // hide helping floating star
                    this._resetHelpStar();
                }
            }

            // update hearts if the rating is not set
            if ((this._options.userRating === null) && (this._options.averageRating === null)) {
                for (i = 0; i < this._options.maxRating; i++) {
                    if (this._options.readOnly) {
                        this._elements[i].className = msRatingDisabledEmpty;
                        this._elementsClassName[i] = msRatingDisabledEmpty;
                    } else {
                        this._elements[i].className = msRatingUserEmpty;
                        this._elementsClassName[i] = msRatingUserEmpty;
                    }
                }

                // hide helping floating star
                this._resetHelpStar();
            }

            var cursor = (this._options.readOnly ? ""default"" : ""pointer"");

            for (i = 0; i <= this._options.maxRating; i++) {
                this._elements[i].style.cursor = cursor;
            }
        },

        _validateOptions: function RatingControl_validateOptions(options) {
            if (options !== undefined) {
                if ((""userRating"" in options) && (options.userRating !== null)) {
                    if ((typeof options.userRating !== ""number"") || (options.userRating <= 0) || (options.userRating > this._options.maxRating) || (Math.floor(options.userRating) !== options.userRating)) {
                        throw new Error(userRatingIsInvalid);
                    }
                }
                if ((""averageRating"" in options) && (options.averageRating !== null)) {
                    if ((typeof options.averageRating !== ""number"") || (options.averageRating < 1) || (options.averageRating > this._options.maxRating)) {
                        throw new Error(averageRatingIsInvalid);
                    }
                }
                if (""readOnly"" in options) {
                    if (typeof options.readOnly !== ""boolean"") {
                        throw new Error(readOnlyIsInvalid);
                    }
                }
            }
        }
    },
    function (element, options) {
        if (!element) {
            throw new Error(elementIsInvalid);
        }

        if (this === window || this === Win.UI.Controls) {
            var rating = utilities.getData(element, ""rating"");
            if (rating) {
                return rating;
            } else {
                return new Win.UI.Controls.Rating(element, options);
            }
        }

        this._element = element;
        this._options = { maxRating: defaultMaxRating, userRating: null, averageRating: null, readOnly: defaultReadOnly };
        this._setControlSize(options);
        this._validateOptions(options);
        this._createControl();
        this._setOptions(options);
        this._events();
        this._customEvents = {};
        utilities.setData(element, ""rating"", this);
    })
});
})(Win8.UI);

// Starting the AppBar Control

(function (thisWinUI) {

// Utilities are private and global pointer will be deleted so we need to cache it locally
var utilities = thisWinUI.Utilities;

var noElement = ""Invalid argument: An AppBar requires a DOM element passed in as its first parameter"";
var badAutoHide = ""Invalid argument: AppBar option autoHide must be a non-negative number"";
var badDynamic = ""Invalid argument: AppBar option dynamic must be a bool"";
var badLightDismiss = ""Invalid argument: AppBar option lightDismiss must be a bool"";
var badPosition = 'Invalid argument: AppBar option position must be ""top"" or ""bottom""';
// AppBar attributes
var appbarPosition = ""data-ms-appbar-position"";
var appbarDynamic = ""data-ms-appbar-dynamic"";
var appbarAutoHide = ""data-ms-appbar-autohide"";
var appbarLightDismiss = ""data-ms-appbar-lightdismiss"";

// AppBar constants
var appbarPositionTop = ""top"";
var appbarPositionBottom = ""bottom"";

// Events
var beforeShowAppBarEvent = ""beforeshowappbar"";
var beforeHideAppBarEvent = ""beforehideappbar"";
var afterShowAppBarEvent = ""aftershowappbar"";
var afterHideAppBarEvent = ""afterhideappbar"";

// TODO: Temporary animation handler for multiple appbars

// TODO: This needs to go away
function Anim() {
}

Anim.prototype = {
    frame: null,
    bars: null,
    ms: null,
    frames: null,
    timer: null,
    show: function () {
        this.frame++;
        for (var i = 0; i < this.bars.length; i++) {
            var bar = this.bars[i];
            if (bar !== null) {
                // If someone interrupted our showing, turn off the animation
                if (bar.msAnimating != ""showing"") {
                    bar = null;
                    this.bars[i] = null;
                }
                else {
                    var position = bar.getAttribute(appbarPosition, 1);
                    if (position === ""top"") {
                        bar.style.top = (this.frame === this.frames) ? ""0px"" :
                        (0 - ((this.frames - this.frame) * bar.clientHeight) / this.frames) + ""px"";
                    } else if (position === ""bottom"") {
                        bar.style.bottom = (this.frame === this.frames) ? ""0px"" :
                        (0 - ((this.frames - this.frame) * bar.clientHeight) / this.frames) + ""px"";
                    } else {
                        bar.style.opacity = (this.frame === this.frames) ? 1 : this.frame / this.frames;
                    }
                    // Make sure we fire the end event
                    if (this.frame === this.frames) {
                        bar.msAnimating = undefined;
                        // Need the aftershowappbar event
                        var event = document.createEvent(""Event"");
                        event.initEvent(afterShowAppBarEvent, true, true);
                        event.appBarElement = bar;
                        bar.dispatchEvent(event);
                    }
                }
            }
        }
        // if done, stop
        if (this.frame >= this.frames) {
            clearInterval(this.timer);
        }
    },
    hide: function () {
        this.frame++;
        for (var i = 0; i < this.bars.length; i++) {
            var bar = this.bars[i];
            if (bar !== null) {
                // If someone interrupted our showing, turn off the animation
                if (bar.msAnimating != ""hiding"") {
                    bar = null;
                    this.bars[i] = null;
                } else {
                    var position = bar.getAttribute(appbarPosition, 1);
                    if (position === ""top"") {
                        bar.style.top = (this.frame === this.frames) ? (0 - bar.clientHeight) + ""px"" :
                                (0 - (this.frame * bar.clientHeight) / this.frames) + ""px"";
                    } else if (position === ""bottom"") {
                        bar.style.bottom = (this.frame === this.frames) ? (0 - bar.clientHeight) + ""px"" :
                                (0 - (this.frame * bar.clientHeight) / this.frames) + ""px"";
                    } else {
                        bar.style.opacity = (this.frames - this.frame) / this.frames;
                    }
                    // Make sure its hidden at end
                    if (this.frame === this.frames) {
                        bar.style.visibility = ""hidden"";
                        bar.msAnimating = undefined;
                        // Need the afterhideappbar event
                        var event = document.createEvent(""Event"");
                        event.initEvent(afterHideAppBarEvent, true, true);
                        event.appBarElement = bar;
                        bar.dispatchEvent(event);
                    }
                }
            }
        }
        // if done, stop
        if (this.frame >= this.frames) {
            clearInterval(this.timer);
        }
    },
    trigger: function anim_trigger(callback) {
        this.timer = setInterval(callback, this.ms);
    }
};
// Internal appbar object used by public interface.
function AppBarImpl(element, options) {
    this.appBarDiv = element;
    if (options) {
        this.setOptions(options);
    }
    var that = this;
    this.clickEvent = element.addEventListener(""mousedown"", that.onMouseDown, false);
}

AppBarImpl.prototype = {
    autoHideTimeout: null,

    getElement: function () {
        return this.appBarDiv;
    },

    setOptions: function (options) {
        if (typeof (options.position) == ""string"") {
            this.setPosition(options.position);
        }
        if (typeof (options.dynamic) == ""boolean"") {
            this.setDynamic(options.dynamic);
        }
        if (typeof (options.autoHide) == ""number"") {
            this.setAutoHide(options.autoHide);
        }
        if (typeof (options.lightDismiss) == ""boolean"") {
            this.setLightDismiss(options.lightDismiss);
        }
    },

    getOptions: function () {
        var options = {};
        options.position = this.getPosition();
        options.dynamic = this.getDynamic();
        options.autoHide = this.getAutoHide();
        options.lightDismiss = this.getLightDismiss();
        return options;
    },

    show: function () {
        // TODO: Need to do real animation
        if (this.appBarDiv.style.visibility != ""visible"" ||
            this.appBarDiv.msAnimating == ""hiding"") {
            this.appBarDiv.msAnimating = ""showing"";
            // Need the beforeshowappbar event
            var event = document.createEvent(""Event"");
            event.initEvent(beforeShowAppBarEvent, true, true);
            event.appBarElement = this.appBarDiv;
            this.appBarDiv.dispatchEvent(event);

            // preposition our div
            var position = this.appBarDiv.getAttribute(appbarPosition, 1);
            if (position == ""top"") {
                this.appBarDiv.style.top = (0 - this.appBarDiv.clientHeight) + ""px"";
                this.appBarDiv.style.bottom = ""auto"";
            } else if (position == ""bottom"") {
                this.appBarDiv.style.bottom = (0 - this.appBarDiv.clientHeight) + ""px"";
                this.appBarDiv.style.top = ""auto"";
            } else {
                this.appBarDiv.style.opacity = 0;
            }
            this.appBarDiv.style.visibility = ""visible"";
            var anim = new Anim();
            anim.frame = 0;
            anim.bars = [this.appBarDiv];
            anim.ms = 15;
            anim.frames = 10;
            anim.trigger(function () { anim.show(); });

            // turn on our autohidetimer
            this.resetAutoHideTimer();
        }
    },

    hide: function () {
        // Clear the timer
        this.resetAutoHideTimer(true);

        // TODO: Need to do real animation
        if (this.appBarDiv.style.visibility != ""hidden"" ||
            this.appBarDiv.msAnimating == ""showing"") {
            this.appBarDiv.msAnimating = ""hiding"";
            // Need the beforehideappbar event
            var event = document.createEvent(""Event"");
            event.initEvent(beforeHideAppBarEvent, true, true);
            event.appBarElement = this.appBarDiv;
            this.appBarDiv.dispatchEvent(event);

            // preposition our div
            var position = this.appBarDiv.getAttribute(appbarPosition, 1);
            if (position == ""top"") {
                this.appBarDiv.style.top = ""0px"";
                this.appBarDiv.style.bottom = ""auto"";
            } else if (position == ""bottom"") {
                this.appBarDiv.style.bottom = ""0px"";
                this.appBarDiv.style.top = ""auto"";
            } else {
                this.appBarDiv.style.opacity = 1;
            }
            this.appBarDiv.style.visibility = ""visible"";
            var anim = new Anim();
            anim.frame = 0;
            anim.bars = [this.appBarDiv];
            anim.ms = 15;
            anim.frames = 10;
            anim.trigger(function () { anim.hide(); });
        }
    },

    setPosition: function (position) {
        if (position != this.getPosition()) {
            // If visible, fade out/in before changing position
            var visible = this.appBarDiv.style.visibility;
            if (visible == ""visible"") {
                this.hide();
                // TODO: Should actually use afterhideappbar event
                var temp = this;
                temp.eventHandler = temp.appBarDiv.addEventListener(""afterhideappbar"", function () {
                    temp.appBarDiv.setAttribute(appbarPosition, position, 1);
                    // show will ""fix"" whichever top or bottom needs corrected
                    temp.appBarDiv.style.top = ""auto"";
                    temp.appBarDiv.style.bottom = ""auto"";
                    temp.show();
                    temp.appBarDiv.removeEventListener(""afterhideappbar"", arguments.callee, false);
                }, false);
            } else {
                this.appBarDiv.setAttribute(appbarPosition, position, 1);
                // Need to let the top or bottom behave
                // Adjust top or bottom if top or bottom so that we're off-screen
                if (position == ""top"") {
                    this.appBarDiv.style.top = (0 - this.appBarDiv.clientHeight) + ""px"";
                    this.appBarDiv.style.bottom = ""auto"";
                } else if (position == ""bottom"") {
                    this.appBarDiv.style.bottom = (0 - this.appBarDiv.clientHeight) + ""px"";
                    this.appBarDiv.style.top = ""auto"";
                } else {
                    // Disassociate from previous top/bottom behavior
                    this.appBarDiv.style.top = ""auto"";
                    this.appBarDiv.style.bottom = ""auto"";
                }
            }
        }
    },

    getPosition: function () {
        return this.appBarDiv.getAttribute(appbarPosition, 1);
    },

    setDynamic: function (dynamic) {
        this.appBarDiv.setAttribute(appbarDynamic, dynamic, 1);
        // TODO: May need to enable/disable autohide & lightdimiss
    },

    getDynamic: function () {
        var dynamic = this.appBarDiv.getAttribute(appbarDynamic, 1);
        return (dynamic === true || dynamic == ""true"") ? true : false;
    },

    setAutoHide: function (autoHide) {
        this.appBarDiv.setAttribute(appbarAutoHide, autoHide, 1);
        // May need to enable or disable it
        this.resetAutoHideTimer();
    },

    // set the timer, clear forces clearing (like about to hide bar)
    resetAutoHideTimer: function (clear) {
        var timeout = this.getAutoHide();
        // Clear any old timer
        if (this.autoHideTimeout) {
            clearTimeout(this.autoHideTimeout);
            this.autoHideTimeout = null;
        }
        // Set the new timer (assuming we're not hidden)
        if (!clear && timeout > 0 && this.getDynamic() && this.appBarDiv.style.visibility != ""hidden"") {
            var temp = this;
            this.autoHideTimeout = setTimeout(function () { temp.hide(); }, timeout);
        }
    },

    getAutoHide: function () {
        var autoHide = this.appBarDiv.getAttribute(appbarAutoHide, 1);
        // If it's not a number, turn it into a number
        if (typeof autoHide != ""number"") {
            if (autoHide !== undefined) {
                autoHide = parseInt(autoHide, 10);
            } else {
                autoHide = 0;
            }
        }
        return autoHide;
    },

    setLightDismiss: function (lightDismiss) {
        this.appBarDiv.setAttribute(appbarLightDismiss, lightDismiss, 1);
    },

    getLightDismiss: function () {
        var lightDismiss = this.appBarDiv.getAttribute(appbarLightDismiss, 1);
        return (lightDismiss === true || lightDismiss == ""true"") ? true : false;
    },

    onMouseDown: function (mouseEvent) {
        // If we're autohide, need to reset the timer
        var appbar = mouseEvent.currentTarget.msControlObject;
        if (appbar) {
            appbar.resetAutoHide();
        }
        // Remember the bar so lightDismiss knows what to touch
        mouseEvent.clickedBar = mouseEvent.currentTarget;
        // Reset autohide timer
    }
};
// Public APIs for AppBar control.

thisWinUI.Controls.AppBar = function (element, options) {
    if (!element) {
        throw new Error(noElement);
    }

    // Checking if JS control was already attached to this DOM element
    var appbar = element.msControlObject;
    if (appbar === undefined) {
        // This is real AppBar implementation hidden here inside closure.
        // The app dev can see only public API exposed by PublicAppBar class
        var appBarImpl = new AppBarImpl(element, options);

        appbar = {
            getElement: function () {
                return appBarImpl.getElement();
            },
            show: function () {
                return appBarImpl.show();
            },
            hide: function () {
                return appBarImpl.hide();
            },
            options: function (options) {
                if (options) {
                    if (options.autoHide !== undefined && typeof (options.autoHide) != ""number"") {
                        throw new Error(badAutoHide);
                    }
                    if (options.dynamic !== undefined && typeof (options.dynamic) != ""boolean"") {
                        throw new Error(badDynamic);
                    }
                    if (options.badLightDismiss !== undefined && typeof (options.lightDismiss) != ""boolean"") {
                        throw new Error(badLightDismiss);
                    }
                    if (options.position !== undefined &&
                        options.position != appbarPositionTop && options.position != appbarPositionBottom) {
                        throw new Error(badPosition);
                    }

                    appBarImpl.setOptions(options);
                } else {
                    var copy = {};
                    return utilities.extend(copy, appBarImpl.getOptions());
                }
            },
            resetAutoHide: function (clear) {
                return appBarImpl.resetAutoHideTimer(clear);
            }
        };

        // Attaching JS control to DOM element
        element.msControlObject = appbar;
    }

    return appbar;
};
// Internal AppBarManager used by public interface.
  
// Everything is static for the manager
AppBarManagerImpl = {
    appBars: null,
    appBarCommandEvent: null,
    appBarClickEvent: null,
    initialize: function (appBars) {
        var oldBars = AppBarManagerImpl.appBars;
        var hideInstantly = false;

        if (!oldBars) {
            hideInstantly = true;
            oldBars = [];
        }

        if (!appBars) {
            appBars = [];
        }

        AppBarManagerImpl.appBars = appBars;
        var allBars = oldBars.concat(appBars);

        AppBarManagerImpl.hideBars(allBars, hideInstantly);

        // TODO: Need to hook up our real event
        if (!AppBarManagerImpl.appBarCommandEvent) {
            AppBarManagerImpl.appBarCommandEvent = document.addEventListener(""keydown"", AppBarManagerImpl.onAppCommand, false);
        }

        // Get appbars to event listeners (construct them)
        for (var i = 0; i < appBars.length; i++) {
            Win8.UI.Controls.AppBar(appBars[i]);
        }

        // Get event listener for main document (for light dismissal)
        if (!AppBarManagerImpl.appBarClickEvent) {
            AppBarManagerImpl.appBarClickEvent = document.addEventListener(""mousedown"", AppBarManagerImpl.onMouseDown, false);
        }
    },
    showAppBars: function () {
        // These are the dynamic DOM objects
        var dynamic = AppBarManagerImpl.getDynamicBars(AppBarManagerImpl.appBars);

        // TODO: Use real animation
        if (dynamic) {
            // Make them visible, ignore ones that are already visible
            var i;
            var useBars = [];
            for (i = 0; i < dynamic.length; i++) {
                var bar = dynamic[i];
                if (bar.style.visibility != ""visible"" ||
                    bar.msAnimating == ""hiding"") {
                    bar.msAnimating = ""showing"";
                    // Need the beforeshowappbar event
                    var event = document.createEvent(""Event"");
                    event.initEvent(beforeShowAppBarEvent, true, true);
                    event.appBarElement = bar;
                    bar.dispatchEvent(event);

                    useBars[useBars.length] = bar;
                    var position = bar.getAttribute(appbarPosition, 1);
                    if (position == ""top"") {
                        bar.style.top = (0 - bar.clientHeight) + ""px"";
                        bar.style.bottom = ""auto"";
                    } else if (position == ""bottom"") {
                        bar.style.bottom = (0 - bar.clientHeight) + ""px"";
                        bar.style.top = ""auto"";
                    } else {
                        bar.style.opacity = 0;
                    }
                    bar.style.visibility = ""visible"";
                }
                // Ensure the timer is OK
                AppBarManagerImpl.checkAppBarAutoHide(bar, false);
            }

            var anim = new Anim();
            anim.frame = 0;
            anim.bars = useBars;
            anim.ms = 15;
            anim.frames = 10;
            anim.trigger(function () { anim.show(); });
        }
    },
    hideAppBars: function (instantly) {
        AppBarManagerImpl.hideBars(AppBarManagerImpl.appBars, instantly);
    },
    onAppCommand: function (event) {
        // TODO: This isn't a proper event yet, need to fix it
        // TODO: For now we're pretending ctrl-alt-a is appbar commend
        if ((event.key == ""a"" || event.key == ""A"") && event.altKey === true && event.ctrlKey === true) {
            var bars = AppBarManagerImpl.getDynamicBars(AppBarManagerImpl.appBars);
            var show = false;
            // If any are hidden, then show them all
            for (var i = 0; i < bars.length; i++) {
                if (bars[i].style.visibility == ""hidden"") {
                    show = true;
                }
            }
            if (show) {
                AppBarManagerImpl.showAppBars();
            } else {
                AppBarManagerImpl.hideAppBars();
            }
        }
    },
    hideBars: function (bars, instantly) {
        // These are the dynamic DOM objects
        var dynamic = AppBarManagerImpl.getDynamicBars(bars);
        if (dynamic) {
            var i;
            var bar;
            var position;
            // If instant, don't animate     
            if (instantly) {
                // Instant only happens on initial startup, so we don't have to check autohide
                for (i = 0; i < dynamic.length; i++) {
                    bar = dynamic[i];
                    bar.style.visibility = ""hidden"";
                    position = bar.getAttribute(appbarPosition, 1);
                    if (position === ""top"") {
                        bar.style.top = (0 - bar.clientHeight) + ""px"";
                    } else if (position === ""bottom"") {
                        bar.style.bottom = (0 - bar.clientHeight) + ""px"";
                    } else {
                        bar.style.opacity = 0;
                    }
                }
            } else {
                // Make them invisible, ignore ones that are already visible
                var useBars = [];
                for (i = 0; i < dynamic.length; i++) {
                    bar = dynamic[i];
                    // Ensure the timer is OK
                    AppBarManagerImpl.checkAppBarAutoHide(bar, true);
                    // TODO: Use real animation
                    if (bar.style.visibility != ""hidden"" ||
                        bar.msAnimating == ""showing"") {
                        bar.msAnimating = ""hiding"";
                        // Need the beforehideappbar event
                        var event = document.createEvent(""Event"");
                        event.initEvent(beforeHideAppBarEvent, true, true);
                        event.appBarElement = bar;
                        bar.dispatchEvent(event);

                        useBars[useBars.length] = bar;
                        position = bar.getAttribute(appbarPosition, 1);
                        if (position == ""top"") {
                            bar.style.top = ""0px"";
                            bar.style.bottom = ""auto"";
                        }
                        else if (position == ""bottom"") {
                            bar.style.bottom = ""0px"";
                            bar.style.top = ""auto"";
                        }
                        else {
                            bar.style.opacity = 1;
                        }
                        bar.style.visibility = ""visible"";
                    }
                }

                var anim = new Anim();
                anim.frame = 0;
                anim.bars = useBars;
                anim.ms = 15;
                anim.frames = 10;
                anim.trigger(function () { anim.hide(); });
            }
        }
    },
    // get the DOM objects in our input DOM bars array tagged as dynamic
    getDynamicBars: function (bars) {
        var dynamicBars = [];
        if (bars) {
            for (var i = 0; i < bars.length; i++) {
                if (bars[i]) {
                    var dynamic = bars[i].getAttribute(appbarDynamic, 1);
                    if (dynamic === true || dynamic == ""true"") {
                        dynamicBars[dynamicBars.length] = bars[i];
                    }
                }
            }
        }
        return dynamicBars;
    },
    onMouseDown: function (mouseEvent) {
        // Stored appbar div that got clicked on (if any) is in mouseEvent.clickedBar
        var dynamic = AppBarManagerImpl.getDynamicBars(AppBarManagerImpl.appBars);
        // We want to hide any all in sync
        var hideBars = [];
        if (dynamic) {
            for (var i = 0; i < dynamic.length; i++) {
                // Don't hide hidden bars or ones that were touched
                var bar = dynamic[i];
                if (mouseEvent.clickedBar != bar &&
                    bar.style.visibility != ""hidden"") {
                    var lightDismiss = bar.getAttribute(appbarLightDismiss, 1);
                    if (lightDismiss === true || lightDismiss == ""true"") {
                        hideBars[hideBars.length] = bar;
                    }
                }
            }
        }
        // Did we find any to hide?
        if (hideBars.length > 0) {
            AppBarManagerImpl.hideBars(hideBars, false);
        }
    },
    checkAppBarAutoHide: function (appBarElement, clear) {
        // We should have an appbar because initialize made one
        var appbar = appBarElement.msControlObject;
        if (appbar) {
            appbar.resetAutoHide(clear);
        }
    }
};
// Public APIs for AppBar control.

// All functions are static for the manager
thisWinUI.Controls.AppBarManager = {};

thisWinUI.Controls.AppBarManager.initialize = function (appBars) {
    if (appBars === undefined) {
        appBars = [];
    } else if (appBars instanceof Array === false) {
        appBars = [appBars];
    }
    AppBarManagerImpl.initialize(appBars);
};

thisWinUI.Controls.AppBarManager.showAppBars = function() {
    AppBarManagerImpl.showAppBars();
};

thisWinUI.Controls.AppBarManager.hideAppBars = function () {
    AppBarManagerImpl.hideAppBars(false);
};

thisWinUI.Controls.AppBarManager.onAppCommand = function(event) {
    AppBarManagerImpl.onAppCommand(event);
};

})(Win8.UI);

(function (thisWinUI) {

Win.Namespace.defineWithParent(thisWinUI, ""Animations"", {
        FadeIn: function Animations_FadeIn(Shown, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;

            var animationEnd_Shown = function (event) {
                styleSheet.deleteRule(cssRules.length - 1);
                Shown.removeEventListener('msAnimationEnd', animationEnd_Shown, false);
                Shown.style.opacity = 1.0;
                callback();
            };
            try {
                styleSheet.insertRule('@-ms-keyframes Animations_Opacity_Shown_1 { from { opacity: 0; } to { opacity: 1.0; } }', cssRules.length);
                Shown.addEventListener('msAnimationEnd', animationEnd_Shown, false);
                Shown.style.msAnimationName = 'Animations_Opacity_Shown_1';
                Shown.style.msAnimationDelay = '0ms';
                Shown.style.msAnimationDuration = '167ms';
                Shown.style.msAnimationTimingFunction = 'linear';
                Shown.style.zIndex = 0;
            } catch (error_1) {}
        },


        FadeOut: function Animations_FadeOut(Hidden, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;

            var animationEnd_Hidden = function (event) {
                styleSheet.deleteRule(cssRules.length - 1);
                Hidden.removeEventListener('msAnimationEnd', animationEnd_Hidden, false);
                Hidden.style.opacity = 0;
                callback();
            };
            try {
                styleSheet.insertRule('@-ms-keyframes Animations_Opacity_Hidden_1 { from { opacity: ; } to { opacity: 0; } }', cssRules.length);
                Hidden.addEventListener('msAnimationEnd', animationEnd_Hidden, false);
                Hidden.style.msAnimationName = 'Animations_Opacity_Hidden_1';
                Hidden.style.msAnimationDelay = '0ms';
                Hidden.style.msAnimationDuration = '167ms';
                Hidden.style.msAnimationTimingFunction = 'linear';
                Hidden.style.zIndex = 0;
            } catch (error_1) {}
        },


        Expand: function Animations_Expand(arrClicked, arrAffected, arrAffected_PosX, arrAffected_PosY, arrRevealed, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;
            var i;

            var transitionEnd_arrClicked_0 = function () {
                arrClicked.removeEventListener('msTransitionEnd', transitionEnd_arrClicked_0, false);
                arrClicked.addEventListener('msTransitionEnd', transitionEnd_arrClicked_100, false);
                arrClicked.style.msTransitionDelay = '0ms';
                arrClicked.style.msTransitionDuration = '125ms';
                arrClicked.style.msTransitionTimingFunction = 'linear';
                arrClicked.style.msTransform = ' scale(1, 1)';
            };
            var transitionEnd_arrClicked_100 = function () {
                arrClicked.removeEventListener('msTransitionEnd', transitionEnd_arrClicked_100, false);
            };
            arrClicked.addEventListener('msTransitionEnd', transitionEnd_arrClicked_0);
            arrClicked.style.msTransitionProperty = '-ms-transform';
            arrClicked.style.msTransitionDelay = '0ms';
            arrClicked.style.msTransitionDuration = '100ms';
            arrClicked.style.msTransitionTimingFunction = 'linear';
            arrClicked.style.msTransform = 'scale(0.8, 0.8)';
            arrClicked.style.zIndex = 2;

            var transitionEnd_arrAffected_200 = function () {
                var j = arrAffected.length - 1;
                for (; j >= 0; j -= 1) {
                    arrAffected[j].removeEventListener('msTransitionEnd', transitionEnd_arrAffected_200, false);
                }
            };
            i = arrAffected.length - 1;
            arrAffected[i].addEventListener('msTransitionEnd', transitionEnd_arrAffected_200, false);
            for (; i >= 0; i -= 1) {
                arrAffected[i].style.msTransitionProperty = '-ms-transform';
                arrAffected[i].style.msTransitionDelay = '200ms';
                arrAffected[i].style.msTransitionDuration = '125ms';
                arrAffected[i].style.msTransitionTimingFunction = 'ease-out';
                arrAffected[i].style.msTransform = 'translate(' + arrAffected_PosX[i] + 'px, ' + arrAffected_PosY[i] + 'px)';
                arrAffected[i].style.zIndex = 1;
            }

            var animationEnd_arrRevealed = function (event) {
                styleSheet.deleteRule(cssRules.length - 1);
                arrRevealed.removeEventListener('msAnimationEnd', animationEnd_arrRevealed, false);
                arrRevealed.style.opacity = 1;
                callback();
            };
            try {
                styleSheet.insertRule('@-ms-keyframes Animations_Opacity_arrRevealed_1 { from { opacity: 0; } to { opacity: 1; } }', cssRules.length);
                arrRevealed.addEventListener('msAnimationEnd', animationEnd_arrRevealed, false);
                arrRevealed.style.msAnimationName = 'Animations_Opacity_arrRevealed_1';
                arrRevealed.style.msAnimationDelay = '300ms';
                arrRevealed.style.msAnimationDuration = '250ms';
                arrRevealed.style.msAnimationTimingFunction = 'ease-in';
                arrRevealed.style.zIndex = 0;
            } catch (error_3) {}
        },


        Collapse: function Animations_Collapse(arrClicked, arrAffected, arrAffected_PosX, arrAffected_PosY, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;
            var i;

            var animationEnd_arrClicked = function (event) {
                styleSheet.deleteRule(cssRules.length - 1);
                arrClicked.removeEventListener('msAnimationEnd', animationEnd_arrClicked, false);
                arrClicked.style.opacity = 0.0;
            };
            try {
                styleSheet.insertRule('@-ms-keyframes Animations_Opacity_arrClicked_1 { from { opacity: ; } to { opacity: 0.0; } }', cssRules.length);
                arrClicked.addEventListener('msAnimationEnd', animationEnd_arrClicked, false);
                arrClicked.style.msAnimationName = 'Animations_Opacity_arrClicked_1';
                arrClicked.style.msAnimationDelay = '0ms';
                arrClicked.style.msAnimationDuration = '350ms';
                arrClicked.style.msAnimationTimingFunction = 'cubic-bezier(0.1, 0.25, 0.75, 0.9)';
                arrClicked.style.zIndex = 0;
            } catch (error_1) {}

            var transitionEnd_arrAffected_300 = function () {
                var j = arrAffected.length - 1;
                for (; j >= 0; j -= 1) {
                    arrAffected[j].removeEventListener('msTransitionEnd', transitionEnd_arrAffected_300, false);
                }
                callback();
            };
            i = arrAffected.length - 1;
            arrAffected[i].addEventListener('msTransitionEnd', transitionEnd_arrAffected_300, false);
            for (; i >= 0; i -= 1) {
                arrAffected[i].style.msTransitionProperty = '-ms-transform';
                arrAffected[i].style.msTransitionDelay = '300ms';
                arrAffected[i].style.msTransitionDuration = '400ms';
                arrAffected[i].style.msTransitionTimingFunction = 'cubic-bezier(0.1, 0.25, 0.75, 0.9)';
                arrAffected[i].style.msTransform = 'translate(' + arrAffected_PosX[i] + 'px, ' + arrAffected_PosY[i] + 'px)';
                arrAffected[i].style.zIndex = 1;
            }
        },


        Reposition: function Animations_Reposition(arrTarget, arrTarget_PosX, arrTarget_PosY, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;
            var i;

            var transitionEnd_arrTarget_0 = function () {
                var j = arrTarget.length - 1;
                for (; j >= 0; j -= 1) {
                    arrTarget[j].removeEventListener('msTransitionEnd', transitionEnd_arrTarget_0, false);
                }
            };
            i = arrTarget.length - 1;
            arrTarget[i].addEventListener('msTransitionEnd', transitionEnd_arrTarget_0, false);
            for (; i >= 0; i -= 1) {
                arrTarget[i].style.msTransitionProperty = '-ms-transform';
                arrTarget[i].style.msTransitionDelay = '0ms';
                arrTarget[i].style.msTransitionDuration = '200ms';
                arrTarget[i].style.msTransitionTimingFunction = 'ease-in';
                arrTarget[i].style.msTransform = 'translate(' + arrTarget_PosX[i] + 'px, ' + arrTarget_PosY[i] + 'px)';
                arrTarget[i].style.zIndex = 0;
            }
        },


        Add: function Animations_Add(arrTarget, arrAffected, arrAffected_PosX, arrAffected_PosY, arrRowOut, arrRowOut_PosX, arrRowOut_PosY, arrRowIn, arrRowIn_PosX, arrRowIn_PosY, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;
            var i;

            var animationEnd_arrTarget = function (event) {
                var i = arrTarget.length - 1;
                styleSheet.deleteRule(cssRules.length - 1);
                arrTarget[i].removeEventListener('msAnimationEnd', animationEnd_arrTarget, false);
                for (; i >= 0; i -= 1) {
                    arrTarget[i].style.opacity = 1;
                }
            };
            try {
                styleSheet.insertRule('@-ms-keyframes Animations_Opacity_arrTarget_1 { from { opacity: 0; } to { opacity: 1; } }', cssRules.length);
                i = arrTarget.length - 1;
                arrTarget[i].addEventListener('msAnimationEnd', animationEnd_arrTarget, false);
                for (; i >= 0; i -= 1) {
                    arrTarget[i].style.msAnimationName = 'Animations_Opacity_arrTarget_1';
                    arrTarget[i].style.msAnimationDelay = '0ms';
                    arrTarget[i].style.msAnimationDuration = '100ms';
                    arrTarget[i].style.msAnimationTimingFunction = 'linear';
                    arrTarget[i].style.zIndex = 3;
                }
            } catch (error_1) {}

            var transitionEnd_arrTarget_0 = function () {
                var j = arrTarget.length - 1;
                for (; j >= 0; j -= 1) {
                    arrTarget[j].removeEventListener('msTransitionEnd', transitionEnd_arrTarget_0, false);
                }
            };
            i = arrTarget.length - 1;
            arrTarget[i].addEventListener('msTransitionEnd', transitionEnd_arrTarget_0, false);
            for (; i >= 0; i -= 1) {
                arrTarget[i].style.msTransitionProperty = '-ms-transform';
                arrTarget[i].style.msTransitionDelay = '0ms';
                arrTarget[i].style.msTransitionDuration = '220ms';
                arrTarget[i].style.msTransitionTimingFunction = 'cubic-bezier(0.5, 1, 0.5, 1)';
                arrTarget[i].style.msTransform = 'scale(1, 1)';
            }

            var transitionEnd_arrAffected_0 = function () {
                var j = arrAffected.length - 1;
                for (; j >= 0; j -= 1) {
                    arrAffected[j].removeEventListener('msTransitionEnd', transitionEnd_arrAffected_0, false);
                }
                callback();
            };
            i = arrAffected.length - 1;
            arrAffected[i].addEventListener('msTransitionEnd', transitionEnd_arrAffected_0, false);
            for (; i >= 0; i -= 1) {
                arrAffected[i].style.msTransitionProperty = '-ms-transform';
                arrAffected[i].style.msTransitionDelay = '0ms';
                arrAffected[i].style.msTransitionDuration = '300ms';
                arrAffected[i].style.msTransitionTimingFunction = 'cubic-bezier(0.5, 1, 0.5, 1)';
                arrAffected[i].style.msTransform = 'translate(' + arrAffected_PosX[i] + 'px, ' + arrAffected_PosY[i] + 'px)';
                arrAffected[i].style.zIndex = 2;
            }

            var transitionEnd_arrRowOut_0 = function () {
                var j = arrRowOut.length - 1;
                for (; j >= 0; j -= 1) {
                    arrRowOut[j].removeEventListener('msTransitionEnd', transitionEnd_arrRowOut_0, false);
                }
                callback();
            };
            i = arrRowOut.length - 1;
            arrRowOut[i].addEventListener('msTransitionEnd', transitionEnd_arrRowOut_0, false);
            for (; i >= 0; i -= 1) {
                arrRowOut[i].style.msTransitionProperty = '-ms-transform';
                arrRowOut[i].style.msTransitionDelay = '0ms';
                arrRowOut[i].style.msTransitionDuration = '300ms';
                arrRowOut[i].style.msTransitionTimingFunction = 'cubic-bezier(0.5, 1, 0.5, 1)';
                arrRowOut[i].style.msTransform = 'translate(' + arrRowOut_PosX[i] + 'px, ' + arrRowOut_PosY[i] + 'px)';
                arrRowOut[i].style.zIndex = 1;
            }

            var transitionEnd_arrRowIn_0 = function () {
                var j = arrRowIn.length - 1;
                for (; j >= 0; j -= 1) {
                    arrRowIn[j].removeEventListener('msTransitionEnd', transitionEnd_arrRowIn_0, false);
                }
                callback();
            };
            i = arrRowIn.length - 1;
            arrRowIn[i].addEventListener('msTransitionEnd', transitionEnd_arrRowIn_0, false);
            for (; i >= 0; i -= 1) {
                arrRowIn[i].style.msTransitionProperty = '-ms-transform';
                arrRowIn[i].style.msTransitionDelay = '0ms';
                arrRowIn[i].style.msTransitionDuration = '300ms';
                arrRowIn[i].style.msTransitionTimingFunction = 'cubic-bezier(0.5, 1, 0.5, 1)';
                arrRowIn[i].style.msTransform = 'translate(' + arrRowIn_PosX[i] + 'px, ' + arrRowIn_PosY[i] + 'px)';
                arrRowIn[i].style.zIndex = 0;
            }
        },


        Delete: function Animations_Delete(arrTarget, arrRemaining, arrRemaining_PosX, arrRemaining_PosY, arrRowOut, arrRowOut_PosX, arrRowOut_PosY, arrRowIn, arrRowIn_PosX, arrRowIn_PosY, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;
            var i;

            var animationEnd_arrTarget = function (event) {
                var i = arrTarget.length - 1;
                styleSheet.deleteRule(cssRules.length - 1);
                arrTarget[i].removeEventListener('msAnimationEnd', animationEnd_arrTarget, false);
                for (; i >= 0; i -= 1) {
                    arrTarget[i].style.opacity = 0;
                }
            };
            try {
                styleSheet.insertRule('@-ms-keyframes Animations_Opacity_arrTarget_1 { from { opacity: ; } to { opacity: 0; } }', cssRules.length);
                i = arrTarget.length - 1;
                arrTarget[i].addEventListener('msAnimationEnd', animationEnd_arrTarget, false);
                for (; i >= 0; i -= 1) {
                    arrTarget[i].style.msAnimationName = 'Animations_Opacity_arrTarget_1';
                    arrTarget[i].style.msAnimationDelay = '0ms';
                    arrTarget[i].style.msAnimationDuration = '140ms';
                    arrTarget[i].style.msAnimationTimingFunction = 'linear';
                    arrTarget[i].style.zIndex = 3;
                }
            } catch (error_1) {}

            var transitionEnd_arrTarget_0 = function () {
                var j = arrTarget.length - 1;
                for (; j >= 0; j -= 1) {
                    arrTarget[j].removeEventListener('msTransitionEnd', transitionEnd_arrTarget_0, false);
                }
            };
            i = arrTarget.length - 1;
            arrTarget[i].addEventListener('msTransitionEnd', transitionEnd_arrTarget_0, false);
            for (; i >= 0; i -= 1) {
                arrTarget[i].style.msTransitionProperty = '-ms-transform';
                arrTarget[i].style.msTransitionDelay = '0ms';
                arrTarget[i].style.msTransitionDuration = '220ms';
                arrTarget[i].style.msTransitionTimingFunction = 'cubic-bezier(0.5, 1, 0.5, 1)';
                arrTarget[i].style.msTransform = 'scale(0.3, 0.3)';
            }

            var transitionEnd_arrRemaining_0 = function () {
                var j = arrRemaining.length - 1;
                for (; j >= 0; j -= 1) {
                    arrRemaining[j].removeEventListener('msTransitionEnd', transitionEnd_arrRemaining_0, false);
                }
                callback();
            };
            i = arrRemaining.length - 1;
            arrRemaining[i].addEventListener('msTransitionEnd', transitionEnd_arrRemaining_0, false);
            for (; i >= 0; i -= 1) {
                arrRemaining[i].style.msTransitionProperty = '-ms-transform';
                arrRemaining[i].style.msTransitionDelay = '0ms';
                arrRemaining[i].style.msTransitionDuration = '300ms';
                arrRemaining[i].style.msTransitionTimingFunction = 'cubic-bezier(0.5, 1, 0.5, 1)';
                arrRemaining[i].style.msTransform = 'translate(' + arrRemaining_PosX[i] + 'px, ' + arrRemaining_PosY[i] + 'px)';
                arrRemaining[i].style.zIndex = 2;
            }

            var transitionEnd_arrRowOut_0 = function () {
                var j = arrRowOut.length - 1;
                for (; j >= 0; j -= 1) {
                    arrRowOut[j].removeEventListener('msTransitionEnd', transitionEnd_arrRowOut_0, false);
                }
                callback();
            };
            i = arrRowOut.length - 1;
            arrRowOut[i].addEventListener('msTransitionEnd', transitionEnd_arrRowOut_0, false);
            for (; i >= 0; i -= 1) {
                arrRowOut[i].style.msTransitionProperty = '-ms-transform';
                arrRowOut[i].style.msTransitionDelay = '0ms';
                arrRowOut[i].style.msTransitionDuration = '300ms';
                arrRowOut[i].style.msTransitionTimingFunction = 'cubic-bezier(0.5, 1, 0.5, 1)';
                arrRowOut[i].style.msTransform = 'translate(' + arrRowOut_PosX[i] + 'px, ' + arrRowOut_PosY[i] + 'px)';
                arrRowOut[i].style.zIndex = 1;
            }

            var transitionEnd_arrRowIn_0 = function () {
                var j = arrRowIn.length - 1;
                for (; j >= 0; j -= 1) {
                    arrRowIn[j].removeEventListener('msTransitionEnd', transitionEnd_arrRowIn_0, false);
                }
                callback();
            };
            i = arrRowIn.length - 1;
            arrRowIn[i].addEventListener('msTransitionEnd', transitionEnd_arrRowIn_0, false);
            for (; i >= 0; i -= 1) {
                arrRowIn[i].style.msTransitionProperty = '-ms-transform';
                arrRowIn[i].style.msTransitionDelay = '0ms';
                arrRowIn[i].style.msTransitionDuration = '300ms';
                arrRowIn[i].style.msTransitionTimingFunction = 'cubic-bezier(0.5, 1, 0.5, 1)';
                arrRowIn[i].style.msTransform = 'translate(' + arrRowIn_PosX[i] + 'px, ' + arrRowIn_PosY[i] + 'px)';
                arrRowIn[i].style.zIndex = 0;
            }
        },


        WindowOpen: function Animations_WindowOpen(Target, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;

            var animationEnd_Target = function (event) {
                styleSheet.deleteRule(cssRules.length - 1);
                Target.removeEventListener('msAnimationEnd', animationEnd_Target, false);
                Target.style.opacity = 1.0;
                callback();
            };
            try {
                styleSheet.insertRule('@-ms-keyframes Animations_Opacity_Target_1 { from { opacity: ; } to { opacity: 1.0; } }', cssRules.length);
                Target.addEventListener('msAnimationEnd', animationEnd_Target, false);
                Target.style.msAnimationName = 'Animations_Opacity_Target_1';
                Target.style.msAnimationDelay = '0ms';
                Target.style.msAnimationDuration = '250ms';
                Target.style.msAnimationTimingFunction = 'cubic-bezier(0.1, 0.25, 0.75, 0.9)';
                Target.style.zIndex = 0;
            } catch (error_1) {}

            var transitionEnd_Target_0 = function () {
                Target.removeEventListener('msTransitionEnd', transitionEnd_Target_0, false);
                callback();
            };
            Target.addEventListener('msTransitionEnd', transitionEnd_Target_0);
            Target.style.msTransitionProperty = '-ms-transform';
            Target.style.msTransitionDelay = '0ms';
            Target.style.msTransitionDuration = '250ms';
            Target.style.msTransitionTimingFunction = 'cubic-bezier(0.1, 0.25, 0.75, 0.9)';
            Target.style.msTransform = ' ';
        },


        WindowClose: function Animations_WindowClose(Target, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;

            var animationEnd_Target = function (event) {
                styleSheet.deleteRule(cssRules.length - 1);
                Target.removeEventListener('msAnimationEnd', animationEnd_Target, false);
                Target.style.opacity = 0;
                callback();
            };
            try {
                styleSheet.insertRule('@-ms-keyframes Animations_Opacity_Target_1 { from { opacity: 1.0; } to { opacity: 0; } }', cssRules.length);
                Target.addEventListener('msAnimationEnd', animationEnd_Target, false);
                Target.style.msAnimationName = 'Animations_Opacity_Target_1';
                Target.style.msAnimationDelay = '0ms';
                Target.style.msAnimationDuration = '166ms';
                Target.style.msAnimationTimingFunction = 'cubic-bezier(0.25, 0.1, 0.9, 0.75)';
                Target.style.zIndex = 0;
            } catch (error_1) {}

            var transitionEnd_Target_0 = function () {
                Target.removeEventListener('msTransitionEnd', transitionEnd_Target_0, false);
                callback();
            };
            Target.addEventListener('msTransitionEnd', transitionEnd_Target_0);
            Target.style.msTransitionProperty = '-ms-transform';
            Target.style.msTransitionDelay = '0ms';
            Target.style.msTransitionDuration = '166ms';
            Target.style.msTransitionTimingFunction = 'cubic-bezier(0.25, 0.1, 0.9, 0.75)';
            Target.style.msTransform = '';
        },


        WindowMinimize: function Animations_WindowMinimize(Target, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;

            var animationEnd_Target = function (event) {
                styleSheet.deleteRule(cssRules.length - 1);
                Target.removeEventListener('msAnimationEnd', animationEnd_Target, false);
                Target.style.opacity = $user;
                callback();
            };
            try {
                styleSheet.insertRule('@-ms-keyframes Animations_Opacity_Target_1 { from { opacity: ; } to { opacity: $user; } }', cssRules.length);
                Target.addEventListener('msAnimationEnd', animationEnd_Target, false);
                Target.style.msAnimationName = 'Animations_Opacity_Target_1';
                Target.style.msAnimationDelay = '0ms';
                Target.style.msAnimationDuration = '250ms';
                Target.style.msAnimationTimingFunction = 'linear';
                Target.style.zIndex = 0;
            } catch (error_1) {}

            var transitionEnd_Target_0 = function () {
                Target.removeEventListener('msTransitionEnd', transitionEnd_Target_0, false);
                callback();
            };
            Target.addEventListener('msTransitionEnd', transitionEnd_Target_0);
            Target.style.msTransitionProperty = '-ms-transform';
            Target.style.msTransitionDelay = '0ms';
            Target.style.msTransitionDuration = '250ms';
            Target.style.msTransitionTimingFunction = 'linear';
            Target.style.msTransform = '  ';
        },


        WindowRestoreFromMinimized: function Animations_WindowRestoreFromMinimized(Target, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;

            var animationEnd_Target = function (event) {
                styleSheet.deleteRule(cssRules.length - 1);
                Target.removeEventListener('msAnimationEnd', animationEnd_Target, false);
                Target.style.opacity = $user;
                callback();
            };
            try {
                styleSheet.insertRule('@-ms-keyframes Animations_Opacity_Target_1 { from { opacity: ; } to { opacity: $user; } }', cssRules.length);
                Target.addEventListener('msAnimationEnd', animationEnd_Target, false);
                Target.style.msAnimationName = 'Animations_Opacity_Target_1';
                Target.style.msAnimationDelay = '0ms';
                Target.style.msAnimationDuration = '250ms';
                Target.style.msAnimationTimingFunction = 'ease-in';
                Target.style.zIndex = 0;
            } catch (error_1) {}

            var transitionEnd_Target_0 = function () {
                Target.removeEventListener('msTransitionEnd', transitionEnd_Target_0, false);
                callback();
            };
            Target.addEventListener('msTransitionEnd', transitionEnd_Target_0);
            Target.style.msTransitionProperty = '-ms-transform';
            Target.style.msTransitionDelay = '0ms';
            Target.style.msTransitionDuration = '250ms';
            Target.style.msTransitionTimingFunction = 'ease-in';
            Target.style.msTransform = '  ';
        },


        SlideIn: function Animations_SlideIn(arrTarget, arrTarget_PosX, arrTarget_PosY, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;
            var i;

            var transitionEnd_arrTarget_0 = function () {
                var j = arrTarget.length - 1;
                for (; j >= 0; j -= 1) {
                    arrTarget[j].removeEventListener('msTransitionEnd', transitionEnd_arrTarget_0, false);
                }
            };
            i = arrTarget.length - 1;
            arrTarget[i].addEventListener('msTransitionEnd', transitionEnd_arrTarget_0, false);
            for (; i >= 0; i -= 1) {
                arrTarget[i].style.msTransitionProperty = '-ms-transform';
                arrTarget[i].style.msTransitionDelay = '0ms';
                arrTarget[i].style.msTransitionDuration = '550ms';
                arrTarget[i].style.msTransitionTimingFunction = 'cubic-bezier(0.5, 1, 0.5, 1)';
                arrTarget[i].style.msTransform = 'translate(' + arrTarget_PosX[i] + 'px, ' + arrTarget_PosY[i] + 'px)';
                arrTarget[i].style.zIndex = 0;
            }
        },


        SlideOut: function Animations_SlideOut(arrTarget, arrTarget_PosX, arrTarget_PosY, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;
            var i;

            var transitionEnd_arrTarget_0 = function () {
                var j = arrTarget.length - 1;
                for (; j >= 0; j -= 1) {
                    arrTarget[j].removeEventListener('msTransitionEnd', transitionEnd_arrTarget_0, false);
                }
            };
            i = arrTarget.length - 1;
            arrTarget[i].addEventListener('msTransitionEnd', transitionEnd_arrTarget_0, false);
            for (; i >= 0; i -= 1) {
                arrTarget[i].style.msTransitionProperty = '-ms-transform';
                arrTarget[i].style.msTransitionDelay = '0ms';
                arrTarget[i].style.msTransitionDuration = '550ms';
                arrTarget[i].style.msTransitionTimingFunction = 'cubic-bezier(0.5, 1, 0.5, 1)';
                arrTarget[i].style.msTransform = 'translate(' + arrTarget_PosX[i] + 'px, ' + arrTarget_PosY[i] + 'px)';
                arrTarget[i].style.zIndex = 0;
            }
        },


        Pagination: function Animations_Pagination(Revealed, Revealed_PosX, Revealed_PosY, Hidden, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;

            var transitionEnd_Revealed_0 = function () {
                Revealed.removeEventListener('msTransitionEnd', transitionEnd_Revealed_0, false);
            };
            Revealed.addEventListener('msTransitionEnd', transitionEnd_Revealed_0);
            Revealed.style.msTransitionProperty = '-ms-transform';
            Revealed.style.msTransitionDelay = '0ms';
            Revealed.style.msTransitionDuration = '330ms';
            Revealed.style.msTransitionTimingFunction = 'cubic-bezier(0.5, 1, 0.5, 1)';
            Revealed.style.msTransform = 'translate(' + Revealed_PosX + 'px, ' + Revealed_PosY + 'px)';
            Revealed.style.zIndex = 1;

            var transitionEnd_Hidden_0 = function () {
                Hidden.removeEventListener('msTransitionEnd', transitionEnd_Hidden_0, false);
            };
            Hidden.addEventListener('msTransitionEnd', transitionEnd_Hidden_0);
            Hidden.style.msTransitionProperty = '-ms-transform';
            Hidden.style.msTransitionDelay = '0ms';
            Hidden.style.msTransitionDuration = '330ms';
            Hidden.style.msTransitionTimingFunction = 'linear';
            Hidden.style.msTransform = 'scale(1, 1)';
            Hidden.style.zIndex = 0;
        },


        PopIn: function Animations_PopIn(Target, Target_PosX, Target_PosY, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;

            var animationEnd_Target = function (event) {
                styleSheet.deleteRule(cssRules.length - 1);
                Target.removeEventListener('msAnimationEnd', animationEnd_Target, false);
                Target.style.opacity = 1;
                callback();
            };
            try {
                styleSheet.insertRule('@-ms-keyframes Animations_Opacity_Target_1 { from { opacity: ; } to { opacity: 1; } }', cssRules.length);
                Target.addEventListener('msAnimationEnd', animationEnd_Target, false);
                Target.style.msAnimationName = 'Animations_Opacity_Target_1';
                Target.style.msAnimationDelay = '0ms';
                Target.style.msAnimationDuration = '330ms';
                Target.style.msAnimationTimingFunction = 'linear';
                Target.style.zIndex = 0;
            } catch (error_1) {}

            var transitionEnd_Target_0 = function () {
                Target.removeEventListener('msTransitionEnd', transitionEnd_Target_0, false);
                callback();
            };
            Target.addEventListener('msTransitionEnd', transitionEnd_Target_0);
            Target.style.msTransitionProperty = '-ms-transform';
            Target.style.msTransitionDelay = '0ms';
            Target.style.msTransitionDuration = '330ms';
            Target.style.msTransitionTimingFunction = 'cubic-bezier(0.5, 1, 0.5, 1)';
            Target.style.msTransform = 'translate(' + Target_PosX + 'px, ' + Target_PosY + 'px)';
        },


        PopOut: function Animations_PopOut(Target, Target_PosX, Target_PosY, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;

            var animationEnd_Target = function (event) {
                styleSheet.deleteRule(cssRules.length - 1);
                Target.removeEventListener('msAnimationEnd', animationEnd_Target, false);
                Target.style.opacity = 0;
                callback();
            };
            try {
                styleSheet.insertRule('@-ms-keyframes Animations_Opacity_Target_1 { from { opacity: ; } to { opacity: 0; } }', cssRules.length);
                Target.addEventListener('msAnimationEnd', animationEnd_Target, false);
                Target.style.msAnimationName = 'Animations_Opacity_Target_1';
                Target.style.msAnimationDelay = '0ms';
                Target.style.msAnimationDuration = '330ms';
                Target.style.msAnimationTimingFunction = 'linear';
                Target.style.zIndex = 0;
            } catch (error_1) {}

            var transitionEnd_Target_0 = function () {
                Target.removeEventListener('msTransitionEnd', transitionEnd_Target_0, false);
                callback();
            };
            Target.addEventListener('msTransitionEnd', transitionEnd_Target_0);
            Target.style.msTransitionProperty = '-ms-transform';
            Target.style.msTransitionDelay = '0ms';
            Target.style.msTransitionDuration = '330ms';
            Target.style.msTransitionTimingFunction = 'cubic-bezier(0.5, 1, 0.5, 1)';
            Target.style.msTransform = 'translate(' + Target_PosX + 'px, ' + Target_PosY + 'px)';
        },


        AppLaunch: function Animations_AppLaunch(arrActivated, arrRemaining, arrLauncher, arrLauncher_PosX, arrLauncher_PosY, arrAppScreen, arrBackground, arrBackground_PosX, arrBackground_PosY, arrTimer, arrTimer_PosX, arrTimer_PosY, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;
            var i;

            var transitionEnd_arrActivated_30 = function () {
                arrActivated.removeEventListener('msTransitionEnd', transitionEnd_arrActivated_30, false);
            };
            arrActivated.addEventListener('msTransitionEnd', transitionEnd_arrActivated_30);
            arrActivated.style.msTransitionProperty = '-ms-transform';
            arrActivated.style.msTransitionDelay = '30ms';
            arrActivated.style.msTransitionDuration = '320ms';
            arrActivated.style.msTransitionTimingFunction = 'ease-in';
            arrActivated.style.msTransform = 'scale(, )';
            arrActivated.style.zIndex = 2;

            var animationEnd_arrRemaining = function (event) {
                var i = arrRemaining.length - 1;
                styleSheet.deleteRule(cssRules.length - 1);
                arrRemaining[i].removeEventListener('msAnimationEnd', animationEnd_arrRemaining, false);
                for (; i >= 0; i -= 1) {
                    arrRemaining[i].style.opacity = 0;
                }
            };
            try {
                styleSheet.insertRule('@-ms-keyframes Animations_Opacity_arrRemaining_1 { from { opacity: ; } to { opacity: 0; } }', cssRules.length);
                i = arrRemaining.length - 1;
                arrRemaining[i].addEventListener('msAnimationEnd', animationEnd_arrRemaining, false);
                for (; i >= 0; i -= 1) {
                    arrRemaining[i].style.msAnimationName = 'Animations_Opacity_arrRemaining_1';
                    arrRemaining[i].style.msAnimationDelay = '0ms';
                    arrRemaining[i].style.msAnimationDuration = '100ms';
                    arrRemaining[i].style.msAnimationTimingFunction = 'linear';
                    arrRemaining[i].style.zIndex = 1;
                }
            } catch (error_2) {}

            var transitionEnd_arrRemaining_0 = function () {
                var j = arrRemaining.length - 1;
                for (; j >= 0; j -= 1) {
                    arrRemaining[j].removeEventListener('msTransitionEnd', transitionEnd_arrRemaining_0, false);
                }
            };
            i = arrRemaining.length - 1;
            arrRemaining[i].addEventListener('msTransitionEnd', transitionEnd_arrRemaining_0, false);
            for (; i >= 0; i -= 1) {
                arrRemaining[i].style.msTransitionProperty = '-ms-transform';
                arrRemaining[i].style.msTransitionDelay = '0ms';
                arrRemaining[i].style.msTransitionDuration = '70ms';
                arrRemaining[i].style.msTransitionTimingFunction = 'ease-in';
                arrRemaining[i].style.msTransform = 'scale(0.8, 0.8)';
            }

            var animationEnd_arrLauncher = function (event) {
                styleSheet.deleteRule(cssRules.length - 1);
                arrLauncher.removeEventListener('msAnimationEnd', animationEnd_arrLauncher, false);
                arrLauncher.style.opacity = 0;
            };
            try {
                styleSheet.insertRule('@-ms-keyframes Animations_Opacity_arrLauncher_1 { from { opacity: 1; } to { opacity: 0; } }', cssRules.length);
                arrLauncher.addEventListener('msAnimationEnd', animationEnd_arrLauncher, false);
                arrLauncher.style.msAnimationName = 'Animations_Opacity_arrLauncher_1';
                arrLauncher.style.msAnimationDelay = '200ms';
                arrLauncher.style.msAnimationDuration = '50ms';
                arrLauncher.style.msAnimationTimingFunction = 'linear';
                arrLauncher.style.zIndex = 4;
            } catch (error_3) {}

            var transitionEnd_arrLauncher_160 = function () {
                arrLauncher.removeEventListener('msTransitionEnd', transitionEnd_arrLauncher_160, false);
            };
            arrLauncher.addEventListener('msTransitionEnd', transitionEnd_arrLauncher_160);
            arrLauncher.style.msTransitionProperty = '-ms-transform';
            arrLauncher.style.msTransitionDelay = '160ms';
            arrLauncher.style.msTransitionDuration = '90ms';
            arrLauncher.style.msTransitionTimingFunction = 'cubic-bezier(0.5, 1, 0.5, 1)';
            arrLauncher.style.msTransform = 'translate(' + arrLauncher_PosX + 'px, ' + arrLauncher_PosY + 'px) scale(0.75, 0.75)';

            var animationEnd_arrAppScreen = function (event) {
                styleSheet.deleteRule(cssRules.length - 1);
                arrAppScreen.removeEventListener('msAnimationEnd', animationEnd_arrAppScreen, false);
                arrAppScreen.style.opacity = 1;
            };
            try {
                styleSheet.insertRule('@-ms-keyframes Animations_Opacity_arrAppScreen_1 { from { opacity: 0; } to { opacity: 1; } }', cssRules.length);
                arrAppScreen.addEventListener('msAnimationEnd', animationEnd_arrAppScreen, false);
                arrAppScreen.style.msAnimationName = 'Animations_Opacity_arrAppScreen_1';
                arrAppScreen.style.msAnimationDelay = '290ms';
                arrAppScreen.style.msAnimationDuration = '50ms';
                arrAppScreen.style.msAnimationTimingFunction = 'linear';
                arrAppScreen.style.zIndex = 3;
            } catch (error_4) {}

            var transitionEnd_arrAppScreen_290 = function () {
                arrAppScreen.removeEventListener('msTransitionEnd', transitionEnd_arrAppScreen_290, false);
            };
            arrAppScreen.addEventListener('msTransitionEnd', transitionEnd_arrAppScreen_290);
            arrAppScreen.style.msTransitionProperty = '-ms-transform';
            arrAppScreen.style.msTransitionDelay = '290ms';
            arrAppScreen.style.msTransitionDuration = '160ms';
            arrAppScreen.style.msTransitionTimingFunction = 'cubic-bezier(0.25, 0.5, 0.5, 1)';
            arrAppScreen.style.msTransform = '   ';

            var transitionEnd_arrBackground_160 = function () {
                arrBackground.removeEventListener('msTransitionEnd', transitionEnd_arrBackground_160, false);
                callback();
            };
            arrBackground.addEventListener('msTransitionEnd', transitionEnd_arrBackground_160);
            arrBackground.style.msTransitionProperty = '-ms-transform';
            arrBackground.style.msTransitionDelay = '160ms';
            arrBackground.style.msTransitionDuration = '390ms';
            arrBackground.style.msTransitionTimingFunction = 'cubic-bezier(0.5, 1, 0.5, 1)';
            arrBackground.style.msTransform = 'translate(' + arrBackground_PosX + 'px, ' + arrBackground_PosY + 'px)';
            arrBackground.style.zIndex = 0;

            var transitionEnd_arrTimer_0 = function () {
                arrTimer.removeEventListener('msTransitionEnd', transitionEnd_arrTimer_0, false);
            };
            arrTimer.addEventListener('msTransitionEnd', transitionEnd_arrTimer_0);
            arrTimer.style.msTransitionProperty = '-ms-transform';
            arrTimer.style.msTransitionDelay = '0ms';
            arrTimer.style.msTransitionDuration = '250ms';
            arrTimer.style.msTransitionTimingFunction = 'linear';
            arrTimer.style.msTransform = 'translate(' + arrTimer_PosX + 'px, ' + arrTimer_PosY + 'px)';
        },


        TapDown: function Animations_TapDown(Target, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;

            var transitionEnd_Target_0 = function () {
                Target.removeEventListener('msTransitionEnd', transitionEnd_Target_0, false);
            };
            Target.addEventListener('msTransitionEnd', transitionEnd_Target_0);
            Target.style.msTransitionProperty = '-ms-transform';
            Target.style.msTransitionDelay = '0ms';
            Target.style.msTransitionDuration = '70ms';
            Target.style.msTransitionTimingFunction = 'linear';
            Target.style.msTransform = 'scale(0.97, 0.97) ';
        },


        TapUp: function Animations_TapUp(Target, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;

            var transitionEnd_Target_0 = function () {
                Target.removeEventListener('msTransitionEnd', transitionEnd_Target_0, false);
            };
            Target.addEventListener('msTransitionEnd', transitionEnd_Target_0);
            Target.style.msTransitionProperty = '-ms-transform';
            Target.style.msTransitionDelay = '0ms';
            Target.style.msTransitionDuration = '70ms';
            Target.style.msTransitionTimingFunction = 'linear';
            Target.style.msTransform = 'scale(1.0, 1.0) ';
        },


        SelectDown: function Animations_SelectDown(Target, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;

            var transitionEnd_Target_0 = function () {
                Target.removeEventListener('msTransitionEnd', transitionEnd_Target_0, false);
            };
            Target.addEventListener('msTransitionEnd', transitionEnd_Target_0);
            Target.style.msTransitionProperty = '-ms-transform';
            Target.style.msTransitionDelay = '0ms';
            Target.style.msTransitionDuration = '70ms';
            Target.style.msTransitionTimingFunction = 'linear';
            Target.style.msTransform = 'scale(1.05, 1.05)';
        },


        SelectUp: function Animations_SelectUp(Target, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;

            var transitionEnd_Target_0 = function () {
                Target.removeEventListener('msTransitionEnd', transitionEnd_Target_0, false);
            };
            Target.addEventListener('msTransitionEnd', transitionEnd_Target_0);
            Target.style.msTransitionProperty = '-ms-transform';
            Target.style.msTransitionDelay = '0ms';
            Target.style.msTransitionDuration = '120ms';
            Target.style.msTransitionTimingFunction = 'linear';
            Target.style.msTransform = 'scale(1.0, 1.0)';
        },


        Drag: function Animations_Drag(Target, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;

            var transitionEnd_Target_0 = function () {
                Target.removeEventListener('msTransitionEnd', transitionEnd_Target_0, false);
            };
            Target.addEventListener('msTransitionEnd', transitionEnd_Target_0);
            Target.style.msTransitionProperty = '-ms-transform';
            Target.style.msTransitionDelay = '0ms';
            Target.style.msTransitionDuration = '50ms';
            Target.style.msTransitionTimingFunction = 'linear';
            Target.style.msTransform = 'scale(1.10, 1.05)';
        },


        Drop: function Animations_Drop(Target, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;

            var transitionEnd_Target_0 = function () {
                Target.removeEventListener('msTransitionEnd', transitionEnd_Target_0, false);
            };
            Target.addEventListener('msTransitionEnd', transitionEnd_Target_0);
            Target.style.msTransitionProperty = '-ms-transform';
            Target.style.msTransitionDelay = '0ms';
            Target.style.msTransitionDuration = '50ms';
            Target.style.msTransitionTimingFunction = 'linear';
            Target.style.msTransform = 'scale(1.0, 1.0)';
        },


        DragOver: function Animations_DragOver(Target, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;

            var transitionEnd_Target_0 = function () {
                Target.removeEventListener('msTransitionEnd', transitionEnd_Target_0, false);
            };
            Target.addEventListener('msTransitionEnd', transitionEnd_Target_0);
            Target.style.msTransitionProperty = '-ms-transform';
            Target.style.msTransitionDelay = '0ms';
            Target.style.msTransitionDuration = '50ms';
            Target.style.msTransitionTimingFunction = 'cubic-bezier(0.1, 0.25, 0.75, 0.9)';
            Target.style.msTransform = 'scale(1.10, 1.10)';
        },


        DragOut: function Animations_DragOut(Target, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;

            var transitionEnd_Target_0 = function () {
                Target.removeEventListener('msTransitionEnd', transitionEnd_Target_0, false);
            };
            Target.addEventListener('msTransitionEnd', transitionEnd_Target_0);
            Target.style.msTransitionProperty = '-ms-transform';
            Target.style.msTransitionDelay = '0ms';
            Target.style.msTransitionDuration = '50ms';
            Target.style.msTransitionTimingFunction = 'cubic-bezier(0.1, 0.25, 0.75, 0.9)';
            Target.style.msTransform = 'scale(1.0, 1.0)';
        },


        ActiveNotify: function Animations_ActiveNotify(Target, Target_PosX, Target_PosY, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;

            var transitionEnd_Target_0 = function () {
                Target.removeEventListener('msTransitionEnd', transitionEnd_Target_0, false);
                Target.addEventListener('msTransitionEnd', transitionEnd_Target_367, false);
                Target.style.msTransitionDelay = '0ms';
                Target.style.msTransitionDuration = '167ms';
                Target.style.msTransitionTimingFunction = 'cubic-bezier(0.1, 0.25, 0.75, 0.9)';
                Target.style.msTransform = ' translate(' + Target_PosX + 'px, ' + Target_PosY + 'px)';
            };
            var transitionEnd_Target_367 = function () {
                Target.removeEventListener('msTransitionEnd', transitionEnd_Target_367, false);
                Target.addEventListener('msTransitionEnd', transitionEnd_Target_7534, false);
                Target.style.msTransitionDelay = '7000ms';
                Target.style.msTransitionDuration = '367ms';
                Target.style.msTransitionTimingFunction = 'cubic-bezier(0.1, 0.25, 0.75, 0.9)';
                Target.style.msTransform = ' translate(' + Target_PosX + 'px, ' + Target_PosY + 'px)';
            };
            var transitionEnd_Target_7534 = function () {
                Target.removeEventListener('msTransitionEnd', transitionEnd_Target_7534, false);
            };
            Target.addEventListener('msTransitionEnd', transitionEnd_Target_0);
            Target.style.msTransitionProperty = '-ms-transform';
            Target.style.msTransitionDelay = '0ms';
            Target.style.msTransitionDuration = '367ms';
            Target.style.msTransitionTimingFunction = 'cubic-bezier(0.1, 0.25, 0.75, 0.9)';
            Target.style.msTransform = 'translate(' + Target_PosX + 'px, ' + Target_PosY + 'px)';
            Target.style.zIndex = 0;
        },


        SubtleNotify: function Animations_SubtleNotify(arrContentIn, arrContentIn_PosX, arrContentIn_PosY, arrContentOut, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;
            var i;

            var animationEnd_arrContentIn = function (event) {
                var i = arrContentIn.length - 1;
                styleSheet.deleteRule(cssRules.length - 1);
                arrContentIn[i].removeEventListener('msAnimationEnd', animationEnd_arrContentIn, false);
                for (; i >= 0; i -= 1) {
                    arrContentIn[i].style.opacity = 1.0;
                }
            };
            try {
                styleSheet.insertRule('@-ms-keyframes Animations_Opacity_arrContentIn_1 { from { opacity: 0; } to { opacity: 1.0; } }', cssRules.length);
                i = arrContentIn.length - 1;
                arrContentIn[i].addEventListener('msAnimationEnd', animationEnd_arrContentIn, false);
                for (; i >= 0; i -= 1) {
                    arrContentIn[i].style.msAnimationName = 'Animations_Opacity_arrContentIn_1';
                    arrContentIn[i].style.msAnimationDelay = '250ms';
                    arrContentIn[i].style.msAnimationDuration = '417ms';
                    arrContentIn[i].style.msAnimationTimingFunction = 'linear';
                    arrContentIn[i].style.zIndex = 1;
                }
            } catch (error_1) {}

            var transitionEnd_arrContentIn_0 = function () {
                var j = arrContentIn.length - 1;
                for (; j >= 0; j -= 1) {
                    arrContentIn[j].removeEventListener('msTransitionEnd', transitionEnd_arrContentIn_0, false);
                }
                callback();
            };
            i = arrContentIn.length - 1;
            arrContentIn[i].addEventListener('msTransitionEnd', transitionEnd_arrContentIn_0, false);
            for (; i >= 0; i -= 1) {
                arrContentIn[i].style.msTransitionProperty = '-ms-transform';
                arrContentIn[i].style.msTransitionDelay = '0ms';
                arrContentIn[i].style.msTransitionDuration = '1000ms';
                arrContentIn[i].style.msTransitionTimingFunction = 'cubic-bezier(0.1, 0.25, 0.75, 0.9)';
                arrContentIn[i].style.msTransform = 'translate(' + arrContentIn_PosX[i] + 'px, ' + arrContentIn_PosY[i] + 'px)';
            }

            var animationEnd_arrContentOut = function (event) {
                var i = arrContentOut.length - 1;
                styleSheet.deleteRule(cssRules.length - 1);
                arrContentOut[i].removeEventListener('msAnimationEnd', animationEnd_arrContentOut, false);
                for (; i >= 0; i -= 1) {
                    arrContentOut[i].style.opacity = 0;
                }
            };
            try {
                styleSheet.insertRule('@-ms-keyframes Animations_Opacity_arrContentOut_1 { from { opacity: ; } to { opacity: 0; } }', cssRules.length);
                i = arrContentOut.length - 1;
                arrContentOut[i].addEventListener('msAnimationEnd', animationEnd_arrContentOut, false);
                for (; i >= 0; i -= 1) {
                    arrContentOut[i].style.msAnimationName = 'Animations_Opacity_arrContentOut_1';
                    arrContentOut[i].style.msAnimationDelay = '0ms';
                    arrContentOut[i].style.msAnimationDuration = '250ms';
                    arrContentOut[i].style.msAnimationTimingFunction = 'linear';
                    arrContentOut[i].style.zIndex = 0;
                }
            } catch (error_2) {}
        },


        LauncherLaunch: function Animations_LauncherLaunch(Desktop, Launcher, Launcher_PosX, Launcher_PosY, LauncherTiles, LauncherText, LauncherText_PosX, LauncherText_PosY, LauncherIcon, LauncherIcon_PosX, LauncherIcon_PosY, Background, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;

            var animationEnd_Desktop = function (event) {
                styleSheet.deleteRule(cssRules.length - 1);
                Desktop.removeEventListener('msAnimationEnd', animationEnd_Desktop, false);
                Desktop.style.opacity = 0;
            };
            try {
                styleSheet.insertRule('@-ms-keyframes Animations_Opacity_Desktop_1 { from { opacity: 1; } to { opacity: 0; } }', cssRules.length);
                Desktop.addEventListener('msAnimationEnd', animationEnd_Desktop, false);
                Desktop.style.msAnimationName = 'Animations_Opacity_Desktop_1';
                Desktop.style.msAnimationDelay = '40ms';
                Desktop.style.msAnimationDuration = '20ms';
                Desktop.style.msAnimationTimingFunction = 'cubic-bezier(0.5, 1, 0.5, 1)';
                Desktop.style.zIndex = 0;
            } catch (error_1) {}

            var transitionEnd_Desktop_0 = function () {
                Desktop.removeEventListener('msTransitionEnd', transitionEnd_Desktop_0, false);
            };
            Desktop.addEventListener('msTransitionEnd', transitionEnd_Desktop_0);
            Desktop.style.msTransitionProperty = '-ms-transform';
            Desktop.style.msTransitionDelay = '0ms';
            Desktop.style.msTransitionDuration = '50ms';
            Desktop.style.msTransitionTimingFunction = 'cubic-bezier(0.5, 1, 0.5, 1)';
            Desktop.style.msTransform = 'scale(1.2, 1.2)';

            var animationEnd_Launcher = function (event) {
                styleSheet.deleteRule(cssRules.length - 1);
                Launcher.removeEventListener('msAnimationEnd', animationEnd_Launcher, false);
                Launcher.style.opacity = 1;
            };
            try {
                styleSheet.insertRule('@-ms-keyframes Animations_Opacity_Launcher_1 { from { opacity: 0; } to { opacity: 1; } }', cssRules.length);
                Launcher.addEventListener('msAnimationEnd', animationEnd_Launcher, false);
                Launcher.style.msAnimationName = 'Animations_Opacity_Launcher_1';
                Launcher.style.msAnimationDelay = '160ms';
                Launcher.style.msAnimationDuration = '50ms';
                Launcher.style.msAnimationTimingFunction = 'cubic-bezier(0.5, 1, 0.5, 1)';
                Launcher.style.zIndex = 2;
            } catch (error_2) {}

            var transitionEnd_Launcher_160 = function () {
                Launcher.removeEventListener('msTransitionEnd', transitionEnd_Launcher_160, false);
            };
            Launcher.addEventListener('msTransitionEnd', transitionEnd_Launcher_160);
            Launcher.style.msTransitionProperty = '-ms-transform';
            Launcher.style.msTransitionDelay = '160ms';
            Launcher.style.msTransitionDuration = '90ms';
            Launcher.style.msTransitionTimingFunction = 'cubic-bezier(0.5, 1, 0.5, 1)';
            Launcher.style.msTransform = 'translate(' + Launcher_PosX + 'px, ' + Launcher_PosY + 'px)';

            var transitionEnd_LauncherTiles_60 = function () {
                LauncherTiles.removeEventListener('msTransitionEnd', transitionEnd_LauncherTiles_60, false);
            };
            LauncherTiles.addEventListener('msTransitionEnd', transitionEnd_LauncherTiles_60);
            LauncherTiles.style.msTransitionProperty = '-ms-transform';
            LauncherTiles.style.msTransitionDelay = '60ms';
            LauncherTiles.style.msTransitionDuration = '100ms';
            LauncherTiles.style.msTransitionTimingFunction = 'cubic-bezier(0.5, 1, 0.5, 1)';
            LauncherTiles.style.msTransform = 'scale(1, 1)';
            LauncherTiles.style.zIndex = 3;

            var animationEnd_LauncherText = function (event) {
                styleSheet.deleteRule(cssRules.length - 1);
                LauncherText.removeEventListener('msAnimationEnd', animationEnd_LauncherText, false);
                LauncherText.style.opacity = 1;
            };
            try {
                styleSheet.insertRule('@-ms-keyframes Animations_Opacity_LauncherText_1 { from { opacity: 0; } to { opacity: 1; } }', cssRules.length);
                LauncherText.addEventListener('msAnimationEnd', animationEnd_LauncherText, false);
                LauncherText.style.msAnimationName = 'Animations_Opacity_LauncherText_1';
                LauncherText.style.msAnimationDelay = '130ms';
                LauncherText.style.msAnimationDuration = '40ms';
                LauncherText.style.msAnimationTimingFunction = 'linear';
                LauncherText.style.zIndex = 4;
            } catch (error_4) {}

            var transitionEnd_LauncherText_130 = function () {
                LauncherText.removeEventListener('msTransitionEnd', transitionEnd_LauncherText_130, false);
            };
            LauncherText.addEventListener('msTransitionEnd', transitionEnd_LauncherText_130);
            LauncherText.style.msTransitionProperty = '-ms-transform';
            LauncherText.style.msTransitionDelay = '130ms';
            LauncherText.style.msTransitionDuration = '150ms';
            LauncherText.style.msTransitionTimingFunction = 'cubic-bezier(0.5, 1, 0.5, 1)';
            LauncherText.style.msTransform = 'translate(' + LauncherText_PosX + 'px, ' + LauncherText_PosY + 'px)';

            var animationEnd_LauncherIcon = function (event) {
                styleSheet.deleteRule(cssRules.length - 1);
                LauncherIcon.removeEventListener('msAnimationEnd', animationEnd_LauncherIcon, false);
                LauncherIcon.style.opacity = 1;
            };
            try {
                styleSheet.insertRule('@-ms-keyframes Animations_Opacity_LauncherIcon_1 { from { opacity: 0; } to { opacity: 1; } }', cssRules.length);
                LauncherIcon.addEventListener('msAnimationEnd', animationEnd_LauncherIcon, false);
                LauncherIcon.style.msAnimationName = 'Animations_Opacity_LauncherIcon_1';
                LauncherIcon.style.msAnimationDelay = '130ms';
                LauncherIcon.style.msAnimationDuration = '40ms';
                LauncherIcon.style.msAnimationTimingFunction = 'linear';
                LauncherIcon.style.zIndex = 5;
            } catch (error_5) {}

            var transitionEnd_LauncherIcon_130 = function () {
                LauncherIcon.removeEventListener('msTransitionEnd', transitionEnd_LauncherIcon_130, false);
            };
            LauncherIcon.addEventListener('msTransitionEnd', transitionEnd_LauncherIcon_130);
            LauncherIcon.style.msTransitionProperty = '-ms-transform';
            LauncherIcon.style.msTransitionDelay = '130ms';
            LauncherIcon.style.msTransitionDuration = '150ms';
            LauncherIcon.style.msTransitionTimingFunction = 'cubic-bezier(0.5, 1, 0.5, 1)';
            LauncherIcon.style.msTransform = 'translate(' + LauncherIcon_PosX + 'px, ' + LauncherIcon_PosY + 'px)';

            var transitionEnd_Background_60 = function () {
                Background.removeEventListener('msTransitionEnd', transitionEnd_Background_60, false);
                callback();
            };
            Background.addEventListener('msTransitionEnd', transitionEnd_Background_60);
            Background.style.msTransitionProperty = '-ms-transform';
            Background.style.msTransitionDelay = '60ms';
            Background.style.msTransitionDuration = '280ms';
            Background.style.msTransitionTimingFunction = 'linear';
            Background.style.msTransform = 'scale(1.1, 1.1)';
            Background.style.zIndex = 1;
        },


        LauncherDismiss: function Animations_LauncherDismiss(Desktop, Launcher, Background, Timer, Timer_PosX, Timer_PosY, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;

            var animationEnd_Desktop = function (event) {
                styleSheet.deleteRule(cssRules.length - 1);
                Desktop.removeEventListener('msAnimationEnd', animationEnd_Desktop, false);
                Desktop.style.opacity = 1;
            };
            try {
                styleSheet.insertRule('@-ms-keyframes Animations_Opacity_Desktop_1 { from { opacity: 0; } to { opacity: 1; } }', cssRules.length);
                Desktop.addEventListener('msAnimationEnd', animationEnd_Desktop, false);
                Desktop.style.msAnimationName = 'Animations_Opacity_Desktop_1';
                Desktop.style.msAnimationDelay = '120ms';
                Desktop.style.msAnimationDuration = '30ms';
                Desktop.style.msAnimationTimingFunction = 'cubic-bezier(0.5, 1, 0.5, 1)';
                Desktop.style.zIndex = 1;
            } catch (error_1) {}

            var transitionEnd_Desktop_120 = function () {
                Desktop.removeEventListener('msTransitionEnd', transitionEnd_Desktop_120, false);
            };
            Desktop.addEventListener('msTransitionEnd', transitionEnd_Desktop_120);
            Desktop.style.msTransitionProperty = '-ms-transform';
            Desktop.style.msTransitionDelay = '120ms';
            Desktop.style.msTransitionDuration = '30ms';
            Desktop.style.msTransitionTimingFunction = 'cubic-bezier(0.5, 1, 0.5, 1)';
            Desktop.style.msTransform = 'scale(1, 1)';

            var animationEnd_Launcher = function (event) {
                styleSheet.deleteRule(cssRules.length - 1);
                Launcher.removeEventListener('msAnimationEnd', animationEnd_Launcher, false);
                Launcher.style.opacity = 0;
            };
            try {
                styleSheet.insertRule('@-ms-keyframes Animations_Opacity_Launcher_1 { from { opacity: 1; } to { opacity: 0; } }', cssRules.length);
                Launcher.addEventListener('msAnimationEnd', animationEnd_Launcher, false);
                Launcher.style.msAnimationName = 'Animations_Opacity_Launcher_1';
                Launcher.style.msAnimationDelay = '0ms';
                Launcher.style.msAnimationDuration = '120ms';
                Launcher.style.msAnimationTimingFunction = 'cubic-bezier(0.5, 1, 0.5, 1)';
                Launcher.style.zIndex = 2;
            } catch (error_2) {}

            var transitionEnd_Launcher_0 = function () {
                Launcher.removeEventListener('msTransitionEnd', transitionEnd_Launcher_0, false);
            };
            Launcher.addEventListener('msTransitionEnd', transitionEnd_Launcher_0);
            Launcher.style.msTransitionProperty = '-ms-transform';
            Launcher.style.msTransitionDelay = '0ms';
            Launcher.style.msTransitionDuration = '120ms';
            Launcher.style.msTransitionTimingFunction = 'cubic-bezier(0.5, 1, 0.5, 1)';
            Launcher.style.msTransform = 'scale(0.9, 0.9)';

            var transitionEnd_Background_0 = function () {
                Background.removeEventListener('msTransitionEnd', transitionEnd_Background_0, false);
                callback();
            };
            Background.addEventListener('msTransitionEnd', transitionEnd_Background_0);
            Background.style.msTransitionProperty = '-ms-transform';
            Background.style.msTransitionDelay = '0ms';
            Background.style.msTransitionDuration = '200ms';
            Background.style.msTransitionTimingFunction = 'linear';
            Background.style.msTransform = 'scale(1, 1)';
            Background.style.zIndex = 0;

            var transitionEnd_Timer_0 = function () {
                Timer.removeEventListener('msTransitionEnd', transitionEnd_Timer_0, false);
            };
            Timer.addEventListener('msTransitionEnd', transitionEnd_Timer_0);
            Timer.style.msTransitionProperty = '-ms-transform';
            Timer.style.msTransitionDelay = '0ms';
            Timer.style.msTransitionDuration = '120ms';
            Timer.style.msTransitionTimingFunction = 'linear';
            Timer.style.msTransform = 'translate(' + Timer_PosX + 'px, ' + Timer_PosY + 'px)';
        },


        ContentTransition: function Animations_ContentTransition(Incoming, Incoming_PosX, Incoming_PosY, Outgoing, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;

            var animationEnd_Incoming = function (event) {
                styleSheet.deleteRule(cssRules.length - 1);
                Incoming.removeEventListener('msAnimationEnd', animationEnd_Incoming, false);
                Incoming.style.opacity = 1.0;
            };
            try {
                styleSheet.insertRule('@-ms-keyframes Animations_Opacity_Incoming_1 { from { opacity: 0; } to { opacity: 1.0; } }', cssRules.length);
                Incoming.addEventListener('msAnimationEnd', animationEnd_Incoming, false);
                Incoming.style.msAnimationName = 'Animations_Opacity_Incoming_1';
                Incoming.style.msAnimationDelay = '0ms';
                Incoming.style.msAnimationDuration = '400ms';
                Incoming.style.msAnimationTimingFunction = 'linear';
                Incoming.style.zIndex = 1;
            } catch (error_1) {}

            var transitionEnd_Incoming_0 = function () {
                Incoming.removeEventListener('msTransitionEnd', transitionEnd_Incoming_0, false);
                callback();
            };
            Incoming.addEventListener('msTransitionEnd', transitionEnd_Incoming_0);
            Incoming.style.msTransitionProperty = '-ms-transform';
            Incoming.style.msTransitionDelay = '0ms';
            Incoming.style.msTransitionDuration = '1000ms';
            Incoming.style.msTransitionTimingFunction = 'cubic-bezier(0.1, 0.25, 0.75, 0.9)';
            Incoming.style.msTransform = 'translate(' + Incoming_PosX + 'px, ' + Incoming_PosY + 'px)';

            var animationEnd_Outgoing = function (event) {
                styleSheet.deleteRule(cssRules.length - 1);
                Outgoing.removeEventListener('msAnimationEnd', animationEnd_Outgoing, false);
                Outgoing.style.opacity = 0;
            };
            try {
                styleSheet.insertRule('@-ms-keyframes Animations_Opacity_Outgoing_1 { from { opacity: ; } to { opacity: 0; } }', cssRules.length);
                Outgoing.addEventListener('msAnimationEnd', animationEnd_Outgoing, false);
                Outgoing.style.msAnimationName = 'Animations_Opacity_Outgoing_1';
                Outgoing.style.msAnimationDelay = '0ms';
                Outgoing.style.msAnimationDuration = '400ms';
                Outgoing.style.msAnimationTimingFunction = 'linear';
                Outgoing.style.zIndex = 0;
            } catch (error_2) {}
        },


        Reveal: function Animations_Reveal(Background, Content, Content_PosX, Content_PosY, Outline, Tapped, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;


            var transitionEnd_Content_0 = function () {
                Content.removeEventListener('msTransitionEnd', transitionEnd_Content_0, false);
                callback();
            };
            Content.addEventListener('msTransitionEnd', transitionEnd_Content_0);
            Content.style.msTransitionProperty = '-ms-transform';
            Content.style.msTransitionDelay = '0ms';
            Content.style.msTransitionDuration = '450ms';
            Content.style.msTransitionTimingFunction = 'cubic-bezier(0.1, 0.25, 0.75, 0.9)';
            Content.style.msTransform = 'translate(' + Content_PosX + 'px, ' + Content_PosY + 'px)';
            Content.style.zIndex = 1;

            var animationEnd_Outline = function (event) {
                styleSheet.deleteRule(cssRules.length - 1);
                Outline.removeEventListener('msAnimationEnd', animationEnd_Outline, false);
                Outline.style.opacity = 1.0;
            };
            try {
                styleSheet.insertRule('@-ms-keyframes Animations_Opacity_Outline_1 { from { opacity: 0; } to { opacity: 1.0; } }', cssRules.length);
                Outline.addEventListener('msAnimationEnd', animationEnd_Outline, false);
                Outline.style.msAnimationName = 'Animations_Opacity_Outline_1';
                Outline.style.msAnimationDelay = '0ms';
                Outline.style.msAnimationDuration = '50ms';
                Outline.style.msAnimationTimingFunction = 'linear';
                Outline.style.zIndex = 2;
            } catch (error_3) {}

            var transitionEnd_Tapped_0 = function () {
                Tapped.removeEventListener('msTransitionEnd', transitionEnd_Tapped_0, false);
            };
            Tapped.addEventListener('msTransitionEnd', transitionEnd_Tapped_0);
            Tapped.style.msTransitionProperty = '-ms-transform';
            Tapped.style.msTransitionDelay = '0ms';
            Tapped.style.msTransitionDuration = '200ms';
            Tapped.style.msTransitionTimingFunction = 'cubic-bezier(0.1, 0.25, 0.75, 0.9)';
            Tapped.style.msTransform = 'scale(1.05, 1.05)';
            Tapped.style.zIndex = 3;
        },


        Hide: function Animations_Hide(Background, Content, Content_PosX, Content_PosY, Outline, Tapped, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;


            var transitionEnd_Content_0 = function () {
                Content.removeEventListener('msTransitionEnd', transitionEnd_Content_0, false);
                callback();
            };
            Content.addEventListener('msTransitionEnd', transitionEnd_Content_0);
            Content.style.msTransitionProperty = '-ms-transform';
            Content.style.msTransitionDelay = '0ms';
            Content.style.msTransitionDuration = '200ms';
            Content.style.msTransitionTimingFunction = 'cubic-bezier(0.1, 0.25, 0.75, 0.9)';
            Content.style.msTransform = 'translate(' + Content_PosX + 'px, ' + Content_PosY + 'px)';
            Content.style.zIndex = 1;

            var animationEnd_Outline = function (event) {
                styleSheet.deleteRule(cssRules.length - 1);
                Outline.removeEventListener('msAnimationEnd', animationEnd_Outline, false);
                Outline.style.opacity = 0;
            };
            try {
                styleSheet.insertRule('@-ms-keyframes Animations_Opacity_Outline_1 { from { opacity: 1.0; } to { opacity: 0; } }', cssRules.length);
                Outline.addEventListener('msAnimationEnd', animationEnd_Outline, false);
                Outline.style.msAnimationName = 'Animations_Opacity_Outline_1';
                Outline.style.msAnimationDelay = '0ms';
                Outline.style.msAnimationDuration = '50ms';
                Outline.style.msAnimationTimingFunction = 'linear';
                Outline.style.zIndex = 2;
            } catch (error_3) {}

            var transitionEnd_Tapped_0 = function () {
                Tapped.removeEventListener('msTransitionEnd', transitionEnd_Tapped_0, false);
            };
            Tapped.addEventListener('msTransitionEnd', transitionEnd_Tapped_0);
            Tapped.style.msTransitionProperty = '-ms-transform';
            Tapped.style.msTransitionDelay = '0ms';
            Tapped.style.msTransitionDuration = '100ms';
            Tapped.style.msTransitionTimingFunction = 'cubic-bezier(0.1, 0.25, 0.75, 0.9)';
            Tapped.style.msTransform = 'scale(1.0, 1.0)';
            Tapped.style.zIndex = 3;
        },


        Login: function Animations_Login(In, Out, Background, Background_PosX, Background_PosY, callback) {
            var styleSheet = document.styleSheets[0];
            var cssRules = styleSheet.cssRules;

            var animationEnd_In = function (event) {
                styleSheet.deleteRule(cssRules.length - 1);
                In.removeEventListener('msAnimationEnd', animationEnd_In, false);
                In.style.opacity = 1.0;
            };
            try {
                styleSheet.insertRule('@-ms-keyframes Animations_Opacity_In_1 { from { opacity: .15; } to { opacity: 1.0; } }', cssRules.length);
                In.addEventListener('msAnimationEnd', animationEnd_In, false);
                In.style.msAnimationName = 'Animations_Opacity_In_1';
                In.style.msAnimationDelay = '50ms';
                In.style.msAnimationDuration = '100ms';
                In.style.msAnimationTimingFunction = 'linear';
                In.style.zIndex = 1;
            } catch (error_1) {}

            var transitionEnd_In_50 = function () {
                In.removeEventListener('msTransitionEnd', transitionEnd_In_50, false);
            };
            In.addEventListener('msTransitionEnd', transitionEnd_In_50);
            In.style.msTransitionProperty = '-ms-transform';
            In.style.msTransitionDelay = '50ms';
            In.style.msTransitionDuration = '350ms';
            In.style.msTransitionTimingFunction = 'cubic-bezier(0.35, 2.2, 0.5, 1)';
            In.style.msTransform = '  ';

            var animationEnd_Out = function (event) {
                styleSheet.deleteRule(cssRules.length - 1);
                Out.removeEventListener('msAnimationEnd', animationEnd_Out, false);
                Out.style.opacity = 0;
            };
            try {
                styleSheet.insertRule('@-ms-keyframes Animations_Opacity_Out_1 { from { opacity: 1; } to { opacity: 0; } }', cssRules.length);
                Out.addEventListener('msAnimationEnd', animationEnd_Out, false);
                Out.style.msAnimationName = 'Animations_Opacity_Out_1';
                Out.style.msAnimationDelay = '0ms';
                Out.style.msAnimationDuration = '80ms';
                Out.style.msAnimationTimingFunction = 'linear';
                Out.style.zIndex = 2;
            } catch (error_2) {}

            var transitionEnd_Background_70 = function () {
                Background.removeEventListener('msTransitionEnd', transitionEnd_Background_70, false);
                callback();
            };
            Background.addEventListener('msTransitionEnd', transitionEnd_Background_70);
            Background.style.msTransitionProperty = '-ms-transform';
            Background.style.msTransitionDelay = '70ms';
            Background.style.msTransitionDuration = '500ms';
            Background.style.msTransitionTimingFunction = 'linear';
            Background.style.msTransform = 'translate(' + Background_PosX + 'px, ' + Background_PosY + 'px)';
            Background.style.zIndex = 0;
        }
});

})(Win8.UI);

";
        #endregion
        #region wwaapp.js
        const string wwaapp_js = @"/**********************************************************
*                                                         *
*   © Microsoft. All rights reserved.                     *
*                                                         *
*   This library is intended for use in WWAs only.        *  
*                                                         *
**********************************************************/
// x86chk.fbl_pac_dev(chrisan) 
﻿// Copyright (c) Microsoft Corporation
// All rights reserved

/// <reference path=""../base/_es3.js"" />
/// <reference path=""../base/base.js"" />

(function (global, Win, undefined) {
    var windowSetTimeout = global.setTimeout;
    var windowClearInterval = global.clearInterval;
    var windowClearTimeout = global.clearTimeout;
    var windowSetInterval = global.setInterval;
    var dispatcherInitialized = false;
    var dispatcherInterval;

    var ApplicationClass = Win.Class.define(null, {
        localStorage: { get: function () { return window.localStorage; } },
        totalTime: { get: function () { return Win.Application._totalTime; } },
        state: { get: function () { return Win.Application._state.current; } },
        run: function () {
            Win.Application.run();
        },
        dispatch: function (callback) {
            Win.Application.dispatch(callback);
        },
        addEventListener: function (eventType, listener, capture) {
            Win.Application._listeners[eventType] = listener;
        }
    });

    var applicationStateMachineDefinition = Object.freeze({
        stateOnLoad: ""initializing"",
        initializing: { next: ""loaded"" },
        loaded: { next: ""running"" },
        running: {}
    });

    var currentState = function () {
        return Win.Application._state[Win.Application._state.current];
    };

    var addTimer = function (callback, delay, repeat) {
        var id = Win.Application._nextIntervalId;
        Win.Application._nextIntervalId++;
        Win.Application._intervals[id] = { start: Win.Application.totalTime, delay: delay, callback: callback, repeat: repeat };
        return id;
    };


    var wrapStringCallback = function (callback) {
        if (typeof (callback) === ""string"") {
            return function () { return eval(callback) };
        }
        else {
            return callback;
        }
    };

    var dispatchFrame = function (fps, timeAvailableForUserCode, elapsedFrames, elapsedTime, totalTime) {
        Win.Application._totalTime = totalTime;
        var start = new Date();

        var f = Win.Application._listeners.frame;
        if (f) {
            f(fps, elapsedFrames, elapsedTime, totalTime);
        }

        var end = new Date();

        var queues = Win.Application._workqueues;
        for (var qi = 0, ql = queues.length; qi < ql; qi++) {
            var q = queues[qi];

            // process timers during Q1 (normal)
            //
            if (qi == 1) {
                Win.Application._intervals.forEach(function (timer, index) {
                    if (timer.start + timer.delay < totalTime) {
                        timer.callback();
                        if (timer.repeat) {
                            // UNDONE: should this be timer.start += timer.delay?
                            timer.start = totalTime;
                        }
                        else {
                            delete Win.Application._intervals[index];
                        }
                    }
                });
            }

            // don't process any work from the background queue if we are out of time
            //
            if (qi == ql - 1) { if ((end - start) >= timeAvailableForUserCode) { break; } }

            while (q.length > 0) {
                if (q[0].timeStamp < totalTime) {
                    q.shift().callback();
                    // only process at most 1 item from the background queue
                    //
                    if (qi == ql - 1) { break; }
                    end = new Date();
                    if ((end - start) >= timeAvailableForUserCode * .9) {
                        break;
                    }
                }
                else {
                    break;
                }
            }
        }
    };

    var initializeDispatcher = function () {
        if (dispatcherInitialized) { return; }
        dispatcherInitialized = true;

        window.setInterval = Win.Application.setInterval;
        window.clearInterval = Win.Application.clearInterval;
        window.clearTimeout = Win.Application.clearTimeout;
        window.setTimeout = Win.Application.setTimeout;

        // IE will (it's a bug if this behavior changes) sync 16.7ms interval to VBlank of the 
        // monitor, so we avoid tearing by locking at 60fps 16.7ms wait.
        var fps = 60;
        var delay = 16.7;

        // UNDONE: reserve 30% of the frame for rendering... this should be dynamically tuned
        var userCodeTime = delay * .7;

        var start = new Date();
        var lastFrame = 0;
        var lastTime = 0;

        dispatcherInterval = windowSetInterval(function () {
            var now = new Date();
            var totalTime = now - start;
            var elapsedTime = totalTime - lastTime;
            var frame = ((totalTime / 1000) * fps) >> 0;
            if (frame != lastFrame) {
                dispatchFrame(fps, userCodeTime, frame - lastFrame, elapsedTime, totalTime);
                lastFrame = frame;
                lastTime = totalTime;
            }
        }, delay);
    };
    var unloadDispatcher = function () {
        if (dispatcherInitialized) {
            dispatcherInitialized = false;
            windowClearInterval(dispatcherInterval);
            window.setInterval = windowSetInterval;
            window.clearInterval = windowClearInterval;
            window.clearTimeout = windowClearTimeout;
            window.setTimeout = windowSetTimeout;
        }
    };

    var initializeMessage = function () {
        window.onmessage = function (e) {
            Win.Application.dispatchUrgent(function () {
                var listener = Win.Application._listeners[""message""];
                if (listener) {
                    listener(e);
                }

                if (e.data && e.data.length > 9 && e.data.substring(0, 8) == ""wwahost."") {
                    var name = e.data.substring(8);
                    var listener = Win.Application._listeners[name];
                    if (listener) {
                        listener(e);
                    }
                }
            });
        };
    };
    var unloadMessage = function () {
        window.onmessage = null;
    };

    var processCurrentState = function (listener, curState) {
        var complete = function (skipProcessing, nextState) {
            var next = nextState || curState.next;
            if (next) {
                Win.Application._state.current = nextState || curState.next;
                if (!skipProcessing) {
                    Win.Application._processState();
                }
            }
        };
        if (listener) {
            listener(Win.Application._applicationClass, complete);
        }
        else {
            complete();
        }
    };

    Win.Namespace.defineWithParent(Win, ""Application"", {
        _applicationClass: new ApplicationClass(),
        _state: { value: null, writable: true, enumerable: false },
        _listeners: { value: {
            initializing: function (app, complete) {
                window.addEventListener(""DOMContentLoaded"", function () { complete(); }, true);
            },
            loaded: function (app, complete) {
                Win.Application.dispatch(complete);
            }
        }, writable: true, enumerable: false
        },
        _nextIntervalId: { value: 1, writable: true, enumerable: false },
        _intervals: { value: [], writable: true, enumerable: false },
        _workqueues: { value: [[], [], []], writable: true, enumerable: false },
        _totalTime: { value: 0, writable: true, enumerable: false },

        totalTime: { get: function () { return Win.Application._totalTime; } },

        _processState: function () {
            var a = Win.Application;
            var curState = currentState();
            if (curState) {
                var handleState = function () {
                    var listener = a._listeners[a._state.current];
                    processCurrentState(listener, curState);
                };
                if (!curState.async) {
                    handleState();
                }
                else {
                    a.dispatch(handleState);
                }
            }
        },

        connect: function () {
            var a = Win.Application;
            var loaded = false;
            initializeDispatcher();
            initializeMessage();

            a._state = Object.create(applicationStateMachineDefinition);
            a._state.current = a._state.stateOnLoad || ""initializing"";

            return a._applicationClass;
        },

        disconnect: function () {
            var a = Win.Application;
            a._listeners = {};
            a._intervals = [];
            a._workqueues = [[], [], []];
            unloadDispatcher();
            unloadMessage();
        },

        dispatchBackground: function (callback) {
            initializeDispatcher();
            Win.Application._workqueues[2].push({ timeStamp: Win.Application._totalTime, callback: wrapStringCallback(callback) });
        },

        dispatch: function (callback) {
            initializeDispatcher();
            Win.Application._workqueues[1].push({ timeStamp: Win.Application._totalTime, callback: wrapStringCallback(callback) });
        },

        dispatchUrgent: function (callback) {
            initializeDispatcher();
            Win.Application._workqueues[0].push({ timeStamp: Win.Application._totalTime, callback: wrapStringCallback(callback) });
        },

        run: function () {
            Win.Application._processState();
        },

        setTimeout: function (callback, delay) {
            return addTimer(callback, delay, false);
        },
        setInterval: function (callback, delay) {
            return addTimer(callback, delay, true);
        },
        clearInterval: function (id) {
            delete Win.Application._intervals[id];
        },
        clearTimeout: function (id) {
            delete Win.Application._intervals[id];
        }
    })
})(this, Win);﻿// Copyright (c) Microsoft Corporation
// All rights reserved

/// <reference path=""../base/_es3.js"" />
/// <reference path=""../base/base.js"" />

(function (Win, undefined) {
    var navigateEventName = ""navigate"";

    Win.Namespace.defineWithParent(Win, ""Navigation"", {
        _listeners: [],
        _pending: { value: null, writable: true },
        _default: { value: """", writable: true },
        _initialized: { value: false, writable: true },
        _initialize: function () {
            if (!Win.Navigation._initialized) {
                Win.Navigation._initialized = true;
                window.addEventListener(""hashchange"", Win.Navigation._onhashchange, false);
            }
        },
        _forceUpdateHash: function (e) {
            /// <summary>
            /// Forces the window.location.hash to have the current hash. For some reason I am
            /// seeing IE9 display a blank hash in the address bar, even when window.location.hash is
            /// set to something, when you navigate back.
            /// </summary>
            var hash = window.location.hash;
            var isDefaultHash = (!hash || hash === """" || hash === ""#"" || hash === ""#"" + Win.Navigation._default);
            var requestingDefaultHash = e.urlFragment === Win.Navigation._default;

            if (!requestingDefaultHash || (requestingDefaultHash && !isDefaultHash)) {
                if (e.urlFragment) {
                    window.location.hash = ""#"" + e.urlFragment;
                }
                else {
                    window.location.hash = """";
                }
            }

        },
        _onhashchange: function (event) {
            var e = Win.Navigation._pending;
            Win.Navigation._pending = null;
            if (!e || e.urlFragment !== Win.Navigation.urlFragment) {
                e = { urlFragment: Win.Navigation.urlFragment };
            }

            // UNDONE: why is this needed... it appears that IE (at least for files?) goes 
            // back to a naked URL in the address bar when you press ""back"" in the browser...?
            //
            Win.Navigation._forceUpdateHash(e);

            Win.Navigation._listeners.forEach(function (l) {
                l(e);
            });

            if (e.succcess) {
                e.success();
            }
        },
        urlFragment: {
            get: function () {
                var hash = window.location.hash;
                if (!hash || hash == """" || hash == ""#"") {
                    return Win.Navigation._default;
                }
                else {
                    return hash.substring(1);
                }
            }
        },
        _domContentListener: function (event) {
            window.removeEventListener(""DOMContentLoaded"", Win.Navigation._domContentListener, false);
            Win.Navigation._onhashchange();
        },
        registerDefault: function (urlFragment, navigateOnReady) {
            Win.Navigation._initialize();
            Win.Navigation._default = urlFragment;
            if (navigateOnReady) {
                Win.Utilities.executeAfterDomLoaded(Win.Navigation._domContentListener, true);
            }
        },
        navigate: function (urlFragment, options) {
            var hash = window.location.hash;

            // If hash isn't right and hash is not ""default"" when you are navigating to
            var notRightHash = hash !== ""#"" + urlFragment;
            var isDefaultHash = (!hash || hash === """" || hash === ""#"" || hash === ""#"" + Win.Navigation._default);
            var requestingDefaultHash = urlFragment === Win.Navigation._default;

            if (notRightHash && (!requestingDefaultHash || (requestingDefaultHash && !isDefaultHash))) {
                Win.Navigation._initialize();
                if (Win.Navigation._pending) {
                    // UNDONE: only a single navigation at a time is allowed, is that viable?
                    // 
                    throw ""Only a single navigation at a time is allowed"";
                }
                Win.Navigation._pending = { urlFragment: urlFragment, options: options };

                if (!notRightHash) {
                    setTimeout(Win.Navigation._onhashchange, 1);
                }
                else {
                    window.location.hash = ""#"" + urlFragment;
                }
            }
        },
        canNavigate: { get: function () { return !Win.Navigation._pending; } },
        addEventListener: function (eventType, listener, capture) {
            Win.Navigation._initialize();
            if (eventType === navigateEventName) {
                Win.Navigation._listeners.push(listener);
            }
        },
        removeEventListener: function (eventType, listener, capture) {
            Win.Navigation._initialize();
            if (eventType === navigateEventName) {
                var listeners = Win.Navigation._listeners;
                for (var i = 0, l = listeners.length; i < l; i++) {
                    if (listeners[i] === listener) {
                        delete listeners[i];
                    }
                }
            }
        }
    });
})(Win);
";
        #endregion
        #region xhr.js
        const string xhr_js = @"/**********************************************************
*                                                         *
*   © Microsoft. All rights reserved.                     *
*                                                         *
*   This library is intended for use in WWAs only.        *  
*                                                         *
**********************************************************/
// x86chk.fbl_pac_dev(chrisan) 
﻿// Copyright (c) Microsoft Corporation
// All rights reserved

/// <reference path=""../base/_es3.js"" />
/// <reference path=""../base/base.js"" />

(function (undefined) {

    Win.Namespace.define(""Win"", {
        xhr: function (options) {
            var req = new XMLHttpRequest();
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
            
            if(options.async === undefined) {
                options.async = true;
            }
            req.open(options.type || ""GET"", options.url, options.async, options.user, options.password);
            if (options.headers) {
                Object.keys(options.headers).forEach(function (k) {
                    req.setRequestHeader(k, options.headers[k]);
                });
            }
            req.send(options.data);
            return { abort: function () { req.abort(); } };
        }
    });
})();";
        #endregion
    }
}