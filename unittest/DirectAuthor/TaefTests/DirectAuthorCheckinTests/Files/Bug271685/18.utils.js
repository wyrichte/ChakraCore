
function BrowserType() {
   var srchText = navigator.userAgent;
   var brwTypes = ("Opera,MSIE,Netscape,Firefox,Chrome,Safari").split(',');
   for (var ix = 0; ix < brwTypes.length; ix++) {
      if (srchText.toString().match(brwTypes[ix])) {
         return brwTypes[ix];
      }
   }
   return null;
}

var pK = 0;
//var brwType = BrowserType();

function EscapeSingleQuote(source) {
   return source.replace("'", "\'");
}
function EscapeDoubleQuoteAndUrl(source) {
   source = source.replace('"', '\"');
   return escape(source);
}

var UTILS_CurrentSearchKeyword = "";

function OnSearchCity(keyword, skip) {
   var queryUrl = "/ajaxway.svc/GetCityByKeyword";

   if (keyword.length <= 3) {
      $('#searchCityContent').hide();
      return;
   }

   UTILS_CurrentSearchKeyword = keyword;

   $('#searchCityContent').html("<img src='/images/ajax-loader.gif' />");
   $('#searchCityContent').show();

   $.ajax({
      url: queryUrl,
      type: "POST",
      contentType: "application/json; charset=utf-8",
      dataType: "json",
      data: '{"keyword": "' + keyword + '","take":"' + 10 + '","skip":"' + skip + '"}',
      success: function(result) {
         if (keyword != UTILS_CurrentSearchKeyword) {
            return;
         }

         var cities = result.d;
         var insidehtmlVal = "";
         for (var i = 0; i < cities.length; i++) {
            var city = cities[i];
            insidehtmlVal += "<span><a href='#' onclick=\"ChooseCity(" +
                     city.id + ",'" + city.searchname + "'," + city.latitude + ", " + city.longitude + ");\">" + city.searchname + "</a></span><BR/>";
         }
         if (skip >= 10) {
            var lessSkip = skip - 10;
            insidehtmlVal += "<a href='#' onclick='OnSearchCity(\"" + EscapeDoubleQuoteAndUrl(keyword) + "\"," + lessSkip + ");'>&lt;&lt;&nbsp;&nbsp;&nbsp;&nbsp;</a>";
         }
         if (cities.length == 10) {
            skip = skip + 10;
            insidehtmlVal += "<a href='#' onclick='OnSearchCity(\"" + EscapeDoubleQuoteAndUrl(keyword) + "\"," + skip + ");'>&gt;&gt;</a>";
         }
         $('#searchCityContent').html(insidehtmlVal);
      }
   });
}

function ShowChar(myId, max, infoId) {
   var val = $(myId).attr('value');
   if (val.length > max) {
      $(myId).attr('value', val(0, max - 1));
   }
   $(infoId).html(val.length + "/" + max);
}

function NumOfCategories() {
   var test = $("#inputCategory").attr("value");
   var count = 0;
   while (test.indexOf(",") >= 0) {
      test = test.substr(test.indexOf(",") + 1);
      count++;
   }
   if (count > 1) {  //the format is ,1,2,3,  we always have an extra comma
      count = count - 1;
   }
   return count;
}

function LCat(catID) {
   var value = ',' + catID.toString() + ',';
   var inputCheck = $('#inputCategory');
   var currentVal = inputCheck.attr("value");
   if (currentVal.indexOf(value) >= 0) {
      inputCheck.attr("value", currentVal.replace(value, ','));
   }
   else {
      if (currentVal.length > 0) {
         inputCheck.attr("value", currentVal + catID.toString() + ',');
      }
      else {
         inputCheck.attr("value", value);
      }
   }
}
