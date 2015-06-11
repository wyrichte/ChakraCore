/*
* Async Treeview 0.1 - Lazy-loading extension for Treeview
* 
* http://bassistance.de/jquery-plugins/jquery-plugin-treeview/
*
* Copyright (c) 2007 Jörn Zaefferer
*
* Dual licensed under the MIT and GPL licenses:
*   http://www.opensource.org/licenses/mit-license.php
*   http://www.gnu.org/licenses/gpl.html
*
* Revision: $Id$
*
* 12/26/2009 Version 0.1a  Override for different trees in app
*
*
*/

; (function($) {

   function load(settings, parentId, cityId, child, container) {
      function createNode(parent) {

         var insidehtmlVal = "";
         if (this.id > 0) {
            if (settings.useCheckbox) {
               insidehtmlVal = "<input class='" + settings.checkClass + "' type='checkbox' onclick='" + settings.checkFunction + "(" + this.id + ");'>";
            }
            else if (settings.useRadio) {
               insidehtmlVal = "<input class='" + settings.checkClass + "' name='" + settings.radioGroup + "' type='radio' onclick='" + settings.checkFunction + "(" + this.id + ");'>";
            }

            if (settings.useLink) {
               var url = this.url;
               var urlType = this.urlType;
               if (url == null) {
                  url = "";
               }
               if (urlType == null) {
                  urlType = "";
               }
               insidehtmlVal = "<span>" + insidehtmlVal + "<a href='#' onclick=" + settings.linkFunction + "(" + this.id + ",'" + url + "','" + urlType + "');>" + this.text + "</a></span>";
            }
            else {
               insidehtmlVal = "<span>" + insidehtmlVal + this.text + "</span>";
            }
         }
         else {
            //called from the hasChildren internal createNode call
            insidehtmlVal = this.text;
         }

         var current = $("<li/>").attr("id", this.id || "").html(insidehtmlVal).appendTo(parent);
         if (this.classes) {
            current.children("span").addClass(this.classes);
         }
         if (this.expanded) {
            current.addClass("open");
         }
         if (this.hasChildren || this.children && this.children.length) {
            var branch = $("<ul/>").appendTo(current);
            if (this.hasChildren) {
               current.addClass("hasChildren");
               createNode.call({
                  id: 0,
                  classes: "placeholder",
                  text: "<img src='/images/ajax-loader.gif' />&nbsp;",
                  children: []
               }, branch);
            }
            if (this.children && this.children.length) {
               $.each(this.children, createNode, [branch])
            }
         }
      }
      $.ajax($.extend(true, {
         url: settings.url,
         type: "POST",
         contentType: "application/json; charset=utf-8",
         dataType: "json",
         data: '{"id": "' + parentId + '","cityId":"' + cityId + '"}',
         success: function(response) {
            child.empty();
            $.each(response.d, createNode, [child]);
            $(container).treeview({ add: child });
         }
      }, settings.ajax));
      /*
      $.getJSON(settings.url, {root: root}, function(response) {
      function createNode(parent) {
      var current = $("<li/>").attr("id", this.id || "").html("<span>" + this.text + "</span>").appendTo(parent);
      if (this.classes) {
      current.children("span").addClass(this.classes);
      }
      if (this.expanded) {
      current.addClass("open");
      }
      if (this.hasChildren || this.children && this.children.length) {
      var branch = $("<ul/>").appendTo(current);
      if (this.hasChildren) {
      current.addClass("hasChildren");
      createNode.call({
      classes: "placeholder",
      text: "&nbsp;",
      children:[]
      }, branch);
      }
      if (this.children && this.children.length) {
      $.each(this.children, createNode, [branch])
      }
      }
      }
      child.empty();
      $.each(response, createNode, [child]);
      $(container).treeview({add: child});
      });
      */
   }


   function loadCity(settings, root, type, child, container) {
      function createNode(parent) {

         var insidehtmlVal = "";
         if (this.id > 0) {
            if (settings.useLink && this.type == "city") {
               insidehtmlVal = "<span>" + "<a href='#' onclick=\"" + settings.linkFunction + "(" +
                  this.id + ",'" + this.searchname + "'," + this.latitude + ", " + this.longitude + ");\">" + this.name + "</a></span>";
            }
            else {
               insidehtmlVal = "<span>" + this.name + "</span>";
            }
         }
         else {
            //called from the hasChildren internal createNode call
            insidehtmlVal = this.text;
         }
         var current = $("<li/>").attr("id", this.id || "").html(insidehtmlVal).appendTo(parent);
         
         current.addClass(this.type);

         //current.attr("type", this.type || "");
         if (this.classes) {
            current.children("span").addClass(this.classes);
         }
         if (this.expanded) {
            current.addClass("open");
         }
                  
         if (this.hasChildren || this.children && this.children.length) {
            var branch = $("<ul/>").appendTo(current);
            if (this.hasChildren) {
               current.addClass("hasChildren");
               createNode.call({
                  id: 0,
                  classes: "placeholder",
                  text: "<img src='/images/ajax-loader.gif' />&nbsp;",
                  children: []
               }, branch);
            }
            if (this.children && this.children.length) {
               $.each(this.children, createNode, [branch])
            }
         }
      }
      $.ajax($.extend(true, {
         url: settings.url,
         type: "POST",
         contentType: "application/json; charset=utf-8",
         dataType: "json",
         data: '{"id": "' + root + '","type":"' + type + '"}',
         success: function(response) {
            child.empty();
            $.each(response.d, createNode, [child]);
            $(container).treeview({ add: child });
         }
      }, settings.ajax));

   }


   var proxied = $.fn.treeview;
   $.fn.treeview = function(settings) {
      if (!settings.url) {
         return proxied.apply(this, arguments);
      }
      var container = this;
      var root = 0;
      if (settings.rootId) {
         root = rootId;
      }
      if (!container.children().size()) {

         if (!settings.type) {
            var cityId = "";
            if (settings.cityId) {
               cityId = $(settings.cityId).attr('value');
            }
            load(settings, root, cityId, this, container);
         }
         else if (settings.type == "city") {
            loadCity(settings, root, "", this, container);
         }

      }

      var userToggle = settings.toggle;
      return proxied.call(this, $.extend({}, settings, {
         collapsed: true,
         toggle: function() {
            var $this = $(this);
            if ($this.hasClass("hasChildren")) {
               var childList = $this.removeClass("hasChildren").find("ul");

               if (!settings.type) {
                  var cityId = "";
                  if (settings.cityId) {
                     cityId = $(settings.cityId).attr('value');
                  }
                  load(settings, this.id, cityId, childList, container);
               }
               else if (settings.type == "city") {
                  if ($this.hasClass("country")) {
                     loadCity(settings, this.id, "country", childList, container);
                  }
                  else if ($this.hasClass("state")) {
                     loadCity(settings, this.id, "state", childList, container);
                  }
                  else if ($this.hasClass("top")) {
                     loadCity(settings, this.id, "", childList, container);
                  }
               }
            }
            if (userToggle) {
               userToggle.apply(this, arguments);
            }
         }
      }));
   };

})(jQuery);