
/* BEGIN EXTERNAL SOURCE */

            
       $.ui.dialog.defaults.bgiframe = true;
       $(function () {
           $("#routeDivId").dialog({
               autoOpen: false,
               position: "right"
           });

           $("#PublishDivId").dialog({
               autoOpen: false,
               width: 650
           });


           $("#ChooseCityDivId").dialog({
               autoOpen: false
           });

           $("#SearchResultId").dialog({
               autoOpen: false,
               position: "left"
           });

           $("#ShowCommentId").dialog({
               autoOpen: false,
               width: 400,
               position: "right"
           });
       });

       $(document).ready(function () {

           $("#navigation").treeview({
               url: "ajaxway.svc/GetChildCategoryNoCity",
               persist: "location",
               collapsed: true,
               unique: true,
               useLink: true,
               linkFunction: "SC",
               animated: false,
               cityId: ".inputCurrentCityId",
               unique: false
           })

           $("#chooseCategory").treeview({
               url: "ajaxway.svc/GetChildCategoryNoCity",
               persist: "location",
               collapsed: true,
               unique: true,
               useCheckbox: true,
               checkFunction: "LCat",
               checkClass: "CC",
               animated: false,
               unique: false
           });

           $("#chooseCity").treeview({
               url: "ajaxway.svc/GetCity",
               type: "city",
               persist: "location",
               collapsed: true,
               unique: true,
               useLink: true,
               linkFunction: "ChooseCity",
               animated: false,  //will ensure we don't toggle all the leaves at the same time
               unique: false     //will ensure we don't toggle all the leaves at the same time
           });
       });
	   
	   
    
/* END EXTERNAL SOURCE */

/* BEGIN EXTERNAL SOURCE */

       var map = null;

       var pins = new Array();
       var layer = new Array();

       var layerNum = 5;

       var layerCustomer = 0;
       var layerPark = 1;
       var layerOlympicVenue = 2;
       var layerSearch = 3;
       var layerKml = 4;
       
       var locationDefaultLat = 49.26;
       var locationDefaultLong = -123.13;

       if (navigator.geolocation) {
           navigator.geolocation.getCurrentPosition(exportPosition, errorPosition);
       }
       
       var locationFrom = new VELatLong(locationDefaultLat, locationDefaultLong);
       var locationTo = new VELatLong(locationDefaultLat, locationDefaultLong);
       var locationFromName = null;
       var locationToName = null;
              
		function exportPosition(position) {
			locationDefaultLat = position.coords.latitude;
			locationDefaultLong = position.coords.longitude;
    	}
    				
    	function errorPosition() {
			locationDefaultLat = 49.26;
			locationDefaultLong = -123.13;
    	}
        

		function GetMap(lat, lon) {
  
           m|map|;
           map = new VEMap('myMap');
  
           if (lat == null || lon == null) {
  
               map.LoadMap(new VELatLong(locationDefaultLat, locationDefaultLong), 12, 'h', false);
  
           }
           else {
               map.LoadMap(new VELatLong(lat, lon), 12, 'h', false);
           }
           CreateLayer();
           
           map.AttachEvent("onmousedown", ShapeHandler);
           map.AttachEvent("onmousemove", ShapeHandler);
           map.AttachEvent("onmouseup", ShapeHandler);
       }

       function GetCurrentCity() {
           var city = $("#SearchTitle").text();
           if (city == null || city.length == 0) {
               $("#SearchTitle").html(GetCityLink("Vancouver British Columbia"));
               city = "Vancouver";
           }
           return city;
       }
       function GetCurrentCityName() {
           var city = $(".inputCurrentCityName").attr("value");
       }

       function GetSearchCity() {
           return GetCurrentCity();
       }

       function GetDescription(shape, layerId, descSrc, url, canBePublished, name, addr, mark) {
           var desc = "<p>";
           desc += descSrc;

           if (canBePublished == null) {
               canBePublished = false;
           }

           if (name == null) {
               name = "";
           }
           if (addr != null && addr != "") {
               if (descSrc != "") {
                   desc += "<br/>";
               }
               desc += addr;
           }

           desc += '</p>';


           if (mark != "") {
               desc += "<div class='" + mark + "'></div>";
           }

           var idArg = "'" + shape.GetID() + "','" + layerId + "'";
           var locPara = "GetLatLonFromShape(" + idArg + ").Latitude, GetLatLonFromShape(" + idArg + ").Longitude,'" + EscapeSingleQuote(name) + "'";

           desc += '<p class="externalLink">';
           desc += ' <a href="#" class="SearchResultButton" onclick="SetFrom(' + locPara + ');">From</a> ';
           desc += ' <a href="#" class="SearchResultButton" onclick="SetTo(' + locPara + ');">To</a> ';

           if (name == "") {
               desc += ' <a href="#" class="SearchResultButton" onclick="RemoveShape(' + idArg + ');">Remove</a> ';
           }

           if (canBePublished) {
               desc += ' <a class="SearchResultButton publish_' + mark + '" href="#" onclick="';
               if (addr != null && addr != "") {
                   desc += "PublishShapeViaSearchResult(" + locPara + ",'" + descSrc + "','" + addr + '\');">';
               }
               else {
                   desc += 'PublishShape(' + idArg + ",'" + EscapeSingleQuote(name) + '\');">';
               }
               desc += 'Publish</a>';
           }
           desc += '</p>';

           desc += '<p class="externalLink">';
           if (url != null && url != "") {
               desc += '<a href="' + url + '" target="_blank">Homepage</a> ';
           }
           else if (descSrc.indexOf('http://') == 0) {
               desc += '<a href="' + descSrc + '" target="_blank">Homepage</a> ';
           }

           if (name != "") {
               //desc += "<br/>More in new window:";
               desc += '&nbsp;<a href="http://www.bing.com/search?q=' + EscapeDoubleQuoteAndUrl(name) + ' in ' + EscapeDoubleQuoteAndUrl(GetSearchCity()) + '" target="_blank">Bing</a>';
               desc += '&nbsp;<a href="http://www.yelp.com/search?find_desc=' + EscapeDoubleQuoteAndUrl(name).toLowerCase() + '&find_loc=' + EscapeDoubleQuoteAndUrl(GetCurrentCity()).toLowerCase() + '&ns=0" target="_blank">Yelp</a>';
               desc += '&nbsp;<a href="http://en.wikipedia.org/w/index.php?title=Special:Search&search=' + EscapeDoubleQuoteAndUrl(name) + '+' + EscapeDoubleQuoteAndUrl(GetCurrentCity()) + '&ns0=1&redirs=0" target="_blank">Wikipedia</a>';
           }
           desc += '</p>';

           return desc;
       }

       function AddPushpin(lat, lon, name, descSrc, addr, layerId, canBePublished, mapIconUrl, location) {
           var shape = new VEShape(VEShapeType.Pushpin, new VELatLong(lat, lon));
           shape.SetTitle(name);

           if (mapIconUrl != null && mapIconUrl != "") {
               shape.SetCustomIcon("<img src='" + mapIconUrl + "' />");
           }
           else if (layerId == layerSearch) {
               shape.SetCustomIcon("<img src='Images/SearchPin.png' />");
           }

           pins.push(shape);
           layer[layerId].AddShape(shape);

           var mark = shape.GetID();

           var desc = GetDescription(shape, layerId, descSrc, null, canBePublished, name, addr, mark);
           if (location != null) {
               desc += GetExternalDesc(location.LocationID, location.ImageUrl, location.Url, location.UrlLink1Mark, location.UrlLink1,
                                 location.UrlLink2Mark, location.UrlLink2, location.Rating, null, name);
           }

           shape.SetDescription(desc);

           if (location == null) {
               GetLocationDetail(name, lat, lon, layerId, mark);
           }

           return desc;
       }

       function CreateLayer() {
           var i;
           for (i = 0; i < layerNum; i++) {
               layer.push(new VEShapeLayer());
               map.AddShapeLayer(layer[i]);
           }
           layer[layerCustomer].Show();
           layer[layerSearch].Show();
       }

       function ResetLayer(layerId) {
           layer[layerId].DeleteAllShapes();
       }

       function TraceX(info) {
           document.getElementById("TestInfo").innerHTML += "<p>" + info + "</p>";
       }

       function PinInfo() {
           var geo = map.GetCenter();
           var shape = new VEShape(VEShapeType.Pushpin, geo);
           //shape.SetTitle(geo.Latitude.toString() + "," + geo.Longitude.toString());
           shape.SetTitle("Unpublished Location");

           layer[layerCustomer].AddShape(shape);
           var desc = GetDescription(shape, layerCustomer, "", "", true, "", "", "");
           shape.SetDescription(desc);
       }

       function GetLatLonFromShape(shapeID, layerID) {
           var shape = layer[layerID].GetShapeByID(shapeID);
           var points = shape.GetPoints();
           var mylatLong = points[0];

           return mylatLong;
       }

       function RemoveShape(shapeID, layerID) {
           var shape = layer[layerID].GetShapeByID(shapeID);
           layer[layerID].DeleteShape(shape);
       }

       function PublishShape(shapeID, layerID, name) {
           var publishDiv = document.getElementById("PublishDivId");
           var mylatLong = GetLatLonFromShape(shapeID, layerID);

           $("#inputLocationId").attr("value", "");
           $("#InputLatitude").attr("value", mylatLong.Latitude);
           $("#InputLongtitude").attr("value", mylatLong.Longitude);
           $("#InputLocationName").attr("value", name);

           InitCategory();

           $('#PublishDivId').dialog('option', 'title', "Publish this location");
           $("#InputButtonPublish").attr("value", "Publish this location");
           $("#publishNote").html("Note, only signed in user can publish a location");
           $('#PublishDivId').dialog('open');
       }

       function PublishShapeViaSearchResult(lat, lon, name, addr, phone) {
           $("#inputLocationId").attr("value", "");
           $("#InputLatitude").attr("value", lat);
           $("#InputLongtitude").attr("value", lon);

           $("#InputLocationName").attr("value", name);

           $("#inputAddress1").attr("value", addr);
           $("#inputPhone").attr("value", phone);

           InitCategory();

           $('#PublishDivId').dialog('option', 'title', "Publish this location");
           $("#InputButtonPublish").attr("value", "Publish this location");
           $("#publishNote").html("Note, only signed in user can publish a location");
           $('#PublishDivId').dialog('open');
       }

       function CheckAndDecorate() {
           var validation_result = Vanadium.validate();

           var success = true;
           validation_result.each(function (_element, validation_results) {
               for (var r in validation_results) {
                   if (validation_results[r].success == false) {
                       success = false;
                       break;
                   }
               }
               if (success == false) {
                   return false; // break from hashmap iteration
               }
           });
           if (!success) {
               Vanadium.decorate(validation_result);
               return false;
           }
           return success;
       };

       function PublishSubmit() {

           if (!CheckAndDecorate()) {
               return;
           }

           var count = NumOfCategories();
           if (count == 0) {
               alert("Please choose at least one category.");
               return;
           }

           if (count > 3) {
               alert("Please choose at most three categories.");
               return;
           }

           if ($('#CurrentLiveUser').html() == "") {
               alert("You need to login to windows live account to submit.");
               return;
           }

           $('.inputLocationId').attr("value", $("#inputLocationId").attr("value"));
           $('.inputLatitude').attr("value", $("#InputLatitude").attr("value"));
           $('.inputLongtitude').attr("value", $("#InputLongtitude").attr("value"));
           $('.inputLocationName').attr("value", $("#InputLocationName").attr("value"));
           $('.inputLocationdesc').attr("value", $("#inputLocationdesc").attr("value"));
           $('.inputLocationUrl').attr("value", $("#inputLocationUrl").attr("value"));
           $('.inputAddress1').attr("value", $("#inputAddress1").attr("value"));
           $('.inputAddress2').attr("value", $("#inputAddress2").attr("value"));

           $('.inputPhone').attr("value", $("#inputPhone").attr("value"));
           $('.inputUrlLink1Mark').attr("value", $("#inputUrlLink1Mark").attr("value"));
           $('.inputUrlLink1').attr("value", $("#inputUrlLink1").attr("value"));
           $('.inputUrlLink2Mark').attr("value", $("#inputUrlLink2Mark").attr("value"));
           $('.inputUrlLink2').attr("value", $("#inputUrlLink2").attr("value"));
           $('.inputImageUrl').attr("value", $("#inputImageUrl").attr("value"));

           //$('.inputListRating').attr("value", $("#inputListRating").attr("value"));
           $('.inputCategory').attr("value", $("#inputCategory").attr("value"));

           //asp.net checkbox with css class use a span to wrap it...
           if ($('.inputEmailNotify').html().indexOf("<INPUT") >= 0) {
               $('.inputEmailNotify input').attr("checked", $("#inputEmailNotify").attr("checked"));
           }
           else {
               $('.inputEmailNotify').attr("checked", $("#inputEmailNotify").attr("checked"));
           }

           $('.inputButtonPublish').click();

           $('#PublishDivId').dialog('close');
       }

       var moveShape = null;
       function ShapeHandler(e) {
           if (moveShape != null && e.eventName == "onmousemove") {
               var vpixel1 = new VEPixel(e.mapX, e.mapY);
               var ll1 = map.PixelToLatLong(vpixel1);
               moveShape.SetPoints(ll1);
               return true;
           }
           else if (moveShape != null && e.eventName == "onmouseup") {
               var vpixel2 = new VEPixel(e.mapX, e.mapY);
               var ll2 = map.PixelToLatLong(vpixel2);
               moveShape.SetPoints(ll2);
               moveShape = null;
               return true;
           }
           else if (e.elementID != null && e.eventName == "onmousedown") {
               var mapShape = map.GetShapeByID(e.elementID);
               if (mapShape == null) {
                   return false;
               }

               moveShape = layer[layerCustomer].GetShapeByID(mapShape.GetID());
               if (moveShape != null) {
                   moveShape.SetFillColor(new VEColor(0, 150, 100, 0.5));
                   return true;
               }
           }
           return false;
       }

       var SearchStartIndex = 0;
       var SearchNumOfResult = 10;

       function Find(isFirst, isForward) {
           if (isFirst) {
               SearchStartIndex = 0;
           }
           else {
               if (isForward) {
                   SearchStartIndex = SearchStartIndex + 10;
               }
               else {
                   SearchStartIndex = SearchStartIndex - 10;
                   if (SearchStartIndex <= 0) {
                       SearchStartIndex = 0;
                   }
               }
           }
           ResetLayer(layerSearch);

           var what = $('#TextGeoInfo').attr("value");
           var where = map.GetMapView();
           //var where = GetCurrentCity();
           var findType = VEFindType.Businesses;
           var shapeLayer = layer[layerSearch];
           var showResults = false;
           var createResults = true;
           var useDefaultDisambiguation = true;
           var setBestMapView = true;
           var callback = GetSearchResult;

           map.Find(what, where, findType, shapeLayer, SearchStartIndex, SearchNumOfResult, showResults, createResults, useDefaultDisambiguation, setBestMapView, callback);

       }

       function GetSearchResult(layer, resultsArray, places, hasMore, veErrorMessage) {
           var backward = "<a href='#' onclick='javascript:Find(false,false);'>" + "&lt;&lt;</a>&nbsp;&nbsp;";
           var forward = "<a href='#' onclick='javascript:Find(false,true);'>" + "&gt;&gt;</a>";
           var r;
           if (hasMore) {
               if (SearchStartIndex == 0) {
                   r = "&nbsp;&nbsp;&nbsp;&nbsp;" + forward;
               }
               else {
                   r = backward + forward;
               }
           }
           else {
               if (SearchStartIndex == 0) {
                   r = "";
               }
               else {
                   r = backward;
               }
           }
           document.getElementById("searchMore").innerHTML = r;

           if (resultsArray == null) {
               document.getElementById("searchMore").innerHTML = "No result found";
               $('#SearchResultInDiv').html("No result found");
           }
           else {
               var rect = map.GetMapView();
               var resultHtml = "";

               for (var i = 0; i < resultsArray.length; i++) {
                   var result = resultsArray[i];
                   var lat = result.LatLong.Latitude;
                   var lon = result.LatLong.Longitude;

                   if (i == 0) {
                       rect.TopLeftLatLong.Latitude = lat;
                       rect.BottomRightLatLong.Latitude = lat;
                       rect.TopLeftLatLong.Longitude = lon;
                       rect.BottomRightLatLong.Longitude = lon;
                   }

                   var mapUrl = "Images/SearchPin" + (i + 1) + ".png";
                   var desc = AddPushpin(lat, lon, result.Name, result.Description, result.Phone, layerSearch, true, mapUrl);

                   resultHtml += "<div><img src='" + mapUrl + "' class='imageSearchPin'><span class='searchName'>" + result.Name + '</span><br/>';
                   resultHtml += desc;
                   resultHtml += "</div>";

                   //lat is Y, larger the up
                   if (lat > rect.TopLeftLatLong.Latitude) {
                       rect.TopLeftLatLong.Latitude = lat;
                   }
                   else if (lat < rect.BottomRightLatLong.Latitude) {
                       rect.BottomRightLatLong.Latitude = lat;
                   }

                   //Longitude Property is X, larger the right
                   if (lon < rect.TopLeftLatLong.Longitude) {
                       rect.TopLeftLatLong.Longitude = lon;
                   }
                   else if (lon > rect.BottomRightLatLong.Longitude) {
                       rect.BottomRightLatLong.Longitude = lon;
                   }
               }
               var yInc = (rect.TopLeftLatLong.Latitude - rect.BottomRightLatLong.Latitude) / 10;
               rect.TopLeftLatLong.Latitude = rect.TopLeftLatLong.Latitude + yInc;
               rect.BottomRightLatLong.Latitude = rect.BottomRightLatLong.Latitude - yInc;

               var xInc = (rect.BottomRightLatLong.Longitude - rect.TopLeftLatLong.Longitude) / 10;
               rect.TopLeftLatLong.Longitude = rect.TopLeftLatLong.Longitude - xInc;
               rect.BottomRightLatLong.Longitude = rect.BottomRightLatLong.Longitude + xInc;

               map.SetMapView(rect);

               $('#SearchResultInDiv').html(resultHtml);
               $('#OgdiFilter').hide();
               $('#SearchResultId').dialog('open');
           }
       }

       function SetFrom(lat, lon, name) {
           locationFrom.Latitude = lat;
           locationFrom.Longitude = lon;
           locationFromName = name;
           FindRoute();
       }

       function SetTo(lat, lon, name) {
           locationTo.Latitude = lat;
           locationTo.Longitude = lon;
           locationToName = name;
           FindRoute();
       }

       function FindRoute() {
           var options = new VERouteOptions();
           options.DistanceUnit = VERouteDistanceUnit.Kilometer;

           // Otherwise what's the point?
           options.DrawRoute = true;

           // So the map doesn't change:
           options.SetBestMapView = false;

           var locations = new Array();
           locations.push(locationFrom);
           locations.push(locationTo);

           options.RouteCallback = onGotRoute;
           map.GetDirections(locations, options);
       }

       var turns = "";
       function ShowDetailDirections() {
           $('#routeDivId').html(turns + "<p></p><a href='#' onClick='PrintDetail();'>Print</a>");
           $('#routeDivId').dialog('open');
       }

       function onGotRoute(route) {
           // Unroll route
           var legs = route.RouteLegs;
           var numTurns = 0;
           var leg = null;

           turns = "From " + locationFromName + " to " + locationToName + "<br/>";
           turns += "Total distance: " + route.Distance.toFixed(1) + " km<br/>";

           // Get intermediate legs
           for (var i = 0; i < legs.length; i++) {
               // Get this leg so we don't have to derefernce multiple times
               leg = legs[i];  // Leg is a VERouteLeg object

               // Unroll each intermediate leg
               var turn = null;  // The itinerary leg

               for (var j = 0; j < leg.Itinerary.Items.length; j++) {
                   turn = leg.Itinerary.Items[j];  // turn is a VERouteItineraryItem object
                   numTurns++;
                   turns += numTurns + ". " + turn.Text + " (" + turn.Distance.toFixed(1) + " km)<br/>";
               }
           }

           if (locationFromName != null && locationToName != null) {
               ShowDetailDirections();
           }
       }

       function PrintDetail() {
           var html = '<HTML>\n<HEAD>\n';
           html += '\n</HEAD>\n<BODY>\n';
           var printWin = window.open("", "printSpecial");
           printWin.document.open();
           printWin.document.write(turns);
           printWin.document.close();
           printWin.print();

           $('#routeDivId').dialog('close');
       }

       //Clean up all objects
       function MapDispose() {
           layer = null;
           if (map != null) {
               map.Dispose();
               map = null;
           }
       }

       function SearchKeyPress(e) {
           var code;
           if (!e) e = window.event
           if (e.keyCode) code = e.keyCode;
           else if (e.which) code = e.which;

           if ((code == 13) || (code == 9)) {
               Find(true, true);
               return false;
           }
           return true;
       }


       //ado.net service query
       var dataService = null;

       function setupDataService() {
           if (dataService == null) {
               dataService = new Sys.Data.AdoNetServiceProxy("LocationDB.svc");
               dataService.set_timeout(60000);
           }
       }

       function appendString(source, appendStr) {
           if (appendStr == null) return source;
           if (source == null) source = "";
           return source + appendStr;
       }

       function onSuccessLocation(result, userContext, operation) {
           var locations = result;
           var myLayerNum = layerSearch;

           ResetLayer(myLayerNum);

           for (var i = 0; i < locations.length; i++) {
               var location = locations[i];

               var addr = "";
               addr = appendString(addr, location.Address1) + "<BR>";
               addr = appendString(addr, location.Address2);

               AddPushpin(location.Lat, location.Lon, location.Name, location.Descr, addr, myLayerNum, false, location.MapIconUrl, location);
           }

       }

       function onFailure(result, userContext, operation) {
           alert(result.get_message() + "\r\tStatus Code:  " + result.get_statusCode() + "\r\tTimed Out:  " + result.get_timedOut());
       }

       var currentCallBackLayer = layerKml;
       function SC(categoryId, url, urlType) {
           if (url == "" || urlType == "") {
               setupDataService();
               dataService.set_defaultFailedCallback(onFailure);
               dataService.set_defaultSucceededCallback(onSuccessLocation);

               var currentCityId = $(".inputCurrentCityId").attr('value');

               dataService.query("vLocCat?$filter=(CategoryID eq " + categoryId + ") and (CityID eq " + currentCityId + ")");
           }
           else if (urlType.toLowerCase() == "kml") {
               ShowKmlFile(url, layerKml, "");
           }
           else if (urlType.toLowerCase() == "ogdi") {
               ShowOGDIService(url, layerKml, "", 0, "");
           }
           else if (urlType.toLowerCase() == "csv") {
               ShowCSVFile(url, layerKml, "");
           }
           else if (urlType.toLowerCase() == "georss") {
               ShowGeoRss(url, layerKml, "");
           }
       }

       function CheckGeoRss(url, layerId, checked, mapIconUrl) {
           if (checked) {
               CreateLayerIfNotExists(layerId);
               layer[layerId].Show();
               ShowGeoRss(url, layerId, mapIconUrl);
           }
           else {
               layer[layerId].Hide();
           }
       }

       function ShowGeoRss(url, layerId, mapIconUrl) {
           currentCallBackLayer = layerId;
           currentLayerIconUrl = mapIconUrl;
           ResetLayer(layerId);

           if (url.indexOf("http") < 0) {
               //treat it as relative path, need full path
               currentUrl = document.URL;
               lastSlash = currentUrl.lastIndexOf("/");
               if (lastSlash >= 0) {
                   url = currentUrl.substring(0, lastSlash + 1) + url;
               }
           }
           var queryUrl = "GeoRSSHandler.ashx?myurl=" + url;

           ResetLayer(layerId);
           var veLayerSpec = new VEShapeSourceSpecification(
                            VEDataType.GeoRSS,
                            queryUrl,
                            layer[layerId]);
           map.ImportShapeLayerData(veLayerSpec, KmlFile_Loaded, true);
       }


       function CheckKmlFile(url, layerId, checked, mapIconUrl) {
           if (checked) {
               CreateLayerIfNotExists(layerId);
               layer[layerId].Show();
               ShowKmlFile(url, layerId, mapIconUrl);
           }
           else {
               layer[layerId].Hide();
           }
       }

       function ShowKmlFile(url, layerId, mapIconUrl) {
           currentCallBackLayer = layerId;
           currentLayerIconUrl = mapIconUrl;
           ResetLayer(layerId);
           var veLayerSpec = new VEShapeSourceSpecification(
                            VEDataType.ImportXML,
                            url,
                            layer[layerId]);
           map.ImportShapeLayerData(veLayerSpec, KmlFile_Loaded, true);
       }


       function KmlFile_Loaded(veShapeLayer) {
           var shapeCount = veShapeLayer.GetShapeCount();
           for (var j = 0; j < shapeCount; j++) {
               var shape1 = veShapeLayer.GetShapeByIndex(j);

               if (shape1.GetType() == VEShapeType.Polygon) {
                   shape1.HideIcon();
               }
           }

           for (var i = 0, length = layer[currentCallBackLayer].GetShapeCount(); i < length; i++) {
               var shape = layer[currentCallBackLayer].GetShapeByIndex(i);

               if (currentLayerIconUrl != null && currentLayerIconUrl != "") {
                   shape.SetCustomIcon("<img src='" + currentLayerIconUrl + "' />");
               }
               else if (shape.PhotoUrl == null || shape.PhotoUrl == "") {
                   shape.SetCustomIcon("<img src='Images/SearchPin.png' />");
               }
               var oriDesc = shape.GetDescription();

               var name = shape.GetTitle();
               var mark = shape.GetID();
               var desc = GetDescription(shape, currentCallBackLayer, oriDesc, "", true, name, "", mark);
               shape.SetDescription(desc);

               var lat = shape.GetPoints()[0].Latitude;
               var lon = shape.GetPoints()[0].Longitude;
               GetLocationDetail(name, lat, lon, currentCallBackLayer, mark);
           }
       }

       function InitCategory() {
           //copy the category from navigation 2nd children
           $('.CC').attr("checked", "");
           $('#inputCategory').attr("value", "");
       }

       function CreateLayerIfNotExists(layerId) {
           if (layerId + 1 > layer.length) {
               var currentNum = layer.length;
               for (i = currentNum; i < layerId + 1; i++) {
                   layer.push(new VEShapeLayer());
                   map.AddShapeLayer(layer[i]);
               }
           }
       }

       var currentOgdiUrl = "";
       var currentOgdiQueryUrl = "";
       var currentLayerIconUrl = "";
       var currentOgdiFilter = "";
       var currentOgidSkip = 0;
       var currentOgdiNameToFilter = "";
       function CheckOGDIService(url, layerId, checked, mapIconUrl) {
           if (checked) {
               CreateLayerIfNotExists(layerId);
               layer[layerId].Show();
               ShowOGDIService(url, layerId, mapIconUrl, 0, '');
           }
           else {
               layer[layerId].Hide();
           }
       }

       function ApplyOGDIFilter() {
           layer[currentCallBackLayer].Show();
           ShowOGDIService(currentOgdiUrl, currentCallBackLayer, currentLayerIconUrl, 0, $('#OGDIFilterText').attr('value'));
       }

       function ShowOGDIService(url, layerId, mapIconUrl, skip, myfilter) {
           currentOgdiUrl = url;
           currentCallBackLayer = layerId;
           currentLayerIconUrl = mapIconUrl;
           currentOgidSkip = skip;
           currentOgdiFilter = myfilter;
           if (myfilter == '') {
               $('#OGDIFilterText').attr('value', '');
           }

           ResetLayer(layerId);

           var filterSyntax = "";

           if (currentOgdiNameToFilter != "" && myfilter != "") {
               filterSyntax = currentOgdiNameToFilter + " eq '" + myfilter + "'"; //When OGDI implements contains function, can use "contains(str0, str2)" functionality
           }
           //somehow ogdi query with '&$skip=' + skip doesn't work, writing support email and working around it
           currentOgdiQueryUrl = url + "?$filter=" + filterSyntax + "&$top=" + (10 + skip);

           // using &format=kml returns a KML document instead of AtomPub
           var queryUrl = currentOgdiQueryUrl + "&format=kml";

           var veLayerSpec = new VEShapeSourceSpecification(
                            VEDataType.ImportXML,
                            queryUrl,
                            layer[layerId]);

           map.ImportShapeLayerData(veLayerSpec, KmlOgdi_Loaded, true);
       }

       function KmlOgdi_Loaded(veShapeLayer) {
           var shapeCount = veShapeLayer.GetShapeCount();
           for (var i = 0; i < shapeCount; i++) {
               var shape = veShapeLayer.GetShapeByIndex(i);

               if (shape.GetType() == VEShapeType.Polygon) {
                   shape.HideIcon();
               }
           }

           LoadAdditionalData();
       }

       function LoadAdditionalData() {
           // We don't want KML here.  We want additional "raw data" that we can match up 
           // with the data in the KML using entity.entityid.  To do this we ask for JSON 
           // using &format=json plus we provide a callback function name 
           // using &callback=yourCallbackFunctionName (this is known as JSONP).
           var queryUrl = currentOgdiQueryUrl + "&format=json&callback=?";

           $.getJSON(queryUrl, null, AdditionalData_Loaded);
       }

       function AdditionalData_Loaded(result) {
           result = result.d;
           var resultHtml = "";

           currentOgdiNameToFilter = ""; //reset

           for (var i = 0, length = layer[currentCallBackLayer].GetShapeCount(); i < length; i++) {
               //workaround, remove unwanted shape
               if (i < currentOgidSkip) {
                   continue;
               }

               var shape = layer[currentCallBackLayer].GetShapeByIndex(i);

               if (currentLayerIconUrl != null && currentLayerIconUrl != "") {
                   shape.SetCustomIcon("<img src='" + currentLayerIconUrl + "' />");
               }

               if (shape.GetType() == VEShapeType.Pushpin) {
                   // All KML queries contain entity.entityid as the value
                   // for the description element of the Placemark.
                   // You can use this to match up the results from the 
                   // JSONP query by entity
                   var entity = GetEntityById(shape.GetDescription(), result);
                   if (entity != null) {
                       // Here we can get at any of the returned object
                       // properties and formulate whatever UI for the
                       // Virtual Earth InfoBox we want.  Since this sample
                       // is designer to work with all possible queries, we
                       // just use entity.entityid since every entity has 
                       // an entityid.
                       var addr = "";
                       var phone = "";
                       var zipCode = "";
                       var city = "";
                       var url = "";
                       var name = shape.GetTitle();  //use the same title, this title is the partition key of the kml map
                       var restDesc = "";

                       jQuery.each(entity, function (key, val) {

                           key = key.toLowerCase();
                           if (key.indexOf("addr") >= 0) {
                               if (addr != "") {
                                   addr += "<BR>";
                               }
                               addr += val;
                           }
                           else if (key.indexOf("phone") >= 0) {
                               if (phone != "") {
                                   phone += "<BR>";
                               }
                               phone += val;
                           }
                           else if (key.indexOf("postal_code") >= 0 || key.indexOf("postalcode") >= 0 || key.indexOf("zip") >= 0 || key.indexOf("postcode") >= 0) {
                               if (zipCode != "") {
                                   zipCode += "<BR>";
                               }
                               zipCode += val;
                           }
                           else if (key.indexOf("city") >= 0) {
                               if (city != "") {
                                   city += "<BR>";
                               }
                               city += val;
                           }
                           else if (key.indexOf("url") >= 0) {
                               if (url != "") {
                                   url += "<BR>";
                               }
                               url += val;
                           }
                           else if (key.indexOf("name") >= 0) {
                               //if (name != "") {
                               //   name += "<BR>";
                               //}
                               //name += val;
                               if (currentOgdiNameToFilter == "") {
                                   if (name.indexOf(val) >= 0) {
                                       currentOgdiNameToFilter = key;
                                   }
                               }
                           }
                           else if (key != "partitionkey" && key != "rowkey" && key != "timestamp" && key != "entityid") {
                               restDesc += "<BR>" + key + " : " + val;
                           }

                       });

                       if (currentOgdiNameToFilter == "") {
                           //still didn't find it, find anything that is not a partitionkey, and name matches
                           jQuery.each(entity, function (key, val) {
                               key = key.toLowerCase();
                               if (val == name && key != "partitionkey") {
                                   currentOgdiNameToFilter = key;
                               }
                           });
                       }

                       if (city != "") {
                           addr += "<br/>" + city;
                       }
                       if (zipCode != "") {
                           addr += "<br/>" + zipCode;
                       }
                       if (phone != "") {
                           addr += "<br/>" + phone;
                       }

                       var mark = shape.GetID();
                       var desc = GetDescription(shape, currentCallBackLayer, restDesc, url, true, name, addr, mark);
                       shape.SetDescription(desc);

                       var mapUrl = currentLayerIconUrl;
                       if (currentLayerIconUrl == null || currentLayerIconUrl == "") {

                           //workaround, check index
                           //var index = i + 1;
                           var index = i + 1 - currentOgidSkip;

                           mapUrl = "Images/OgdiPin" + index + ".png";
                           shape.SetCustomIcon("<img src='" + mapUrl + "' />");
                       }

                       resultHtml += "<div><img src='" + mapUrl + "' class='imageSearchPin'><span class='searchName'>" + name + '</span>';
                       resultHtml += desc;
                       resultHtml += "</div>";

                       var lat = shape.GetPoints()[0].Latitude;
                       var lon = shape.GetPoints()[0].Longitude;
                       GetLocationDetail(name, lat, lon, currentCallBackLayer, mark);
                   }
               }

           } //end for loop

           //workaround, remove unwanted shape
           for (var j = 0; j < currentOgidSkip; j++) {
               var shapeDelete = layer[currentCallBackLayer].GetShapeByIndex(0);
               layer[currentCallBackLayer].DeleteShape(shapeDelete);
           }

           var para1 = EscapeSingleQuote(currentOgdiUrl) + "'," + currentCallBackLayer + ",'" + EscapeSingleQuote(currentLayerIconUrl);
           var para2 = EscapeSingleQuote(currentOgdiFilter);

           if (currentOgidSkip >= 10) {
               resultHtml += "<a href='#' onclick=\"javascript:ShowOGDIService('" + para1 + "'," + (currentOgidSkip - 10) + ",'" + para2 + "');\">" + "&lt;&lt;</a>&nbsp;&nbsp;";
           }
           if (layer[currentCallBackLayer].GetShapeCount() >= 10) {
               resultHtml += "<a href='#' onclick=\"javascript:ShowOGDIService('" + para1 + "'," + (currentOgidSkip + 10) + ",'" + para2 + "');\">" + "&gt;&gt;</a>";
           }

           $('#SearchResultInDiv').html(resultHtml);
           $('#OgdiFilter').show();
           $('#SearchResultId').dialog('open');
       }

       function GetEntityById(id, array) {
           for (var i = 0, length = array.length; i < length; i++) {
               if (array[i].entityid == id) {
                   return array[i];
               }
           }

           return null;
       }

       function PickCity() {
           $('#ChooseCityDivId').dialog('open');
       }

       var COOKIE_NAME_ID = 'currentCityCookieId';
       var COOKIE_NAME_NAME = 'currentCityCookieName';
       var COOKIE_NAME_LAT = 'currentCityCookieLAT';
       var COOKIE_NAME_LON = 'currentCityCookieLON';
       var COOKIE_NAME_HISTORY = 'cityHistory';
       var COOKIE_options = { path: '/', expires: 100 };

       function ChooseCity(id, name, latitude, longtitude) {
           /// <param name="name" type="String">city name</param>
           var cityLink = GetCityLink(name);
           $('#SearchTitle').html(cityLink);
           $('.inputCurrentCityName').attr('value', name);
           $('.inputCurrentCityId').attr('value', id);

           $.cookie(COOKIE_NAME_ID, id, COOKIE_options);
           $.cookie(COOKIE_NAME_NAME, name, COOKIE_options);
           $.cookie(COOKIE_NAME_LAT, latitude, COOKIE_options);
           $.cookie(COOKIE_NAME_LON, longtitude, COOKIE_options);

           var cityHistory = $.cookie(COOKIE_NAME_HISTORY);
           if (cityHistory == null) cityHistory = "";

           if (cityHistory.indexOf(cityLink) < 0) {
               // current city is not in the history, add it
               cityHistory = "<li>" + cityLink + "</li>" + cityHistory;
               $.cookie(COOKIE_NAME_HISTORY, cityHistory, COOKIE_options);
               InitPastCities();
           }

           $('#ChooseCityDivId').dialog('close');

           map.SetCenterAndZoom(new VELatLong(latitude, longtitude), 12);

           $('.inputButtonChangeCityHotCat').click();
       }

       function InitPastCities() {
           var cityHistory = $.cookie(COOKIE_NAME_HISTORY);
           if (cityHistory != null) {
               $("#pastCities").html(cityHistory);
           }
       }

       function ClearPastCities() {
           var cityHistory = $.cookie(COOKIE_NAME_HISTORY);
           if (cityHistory != null) {
               $.cookie(COOKIE_NAME_HISTORY, null, COOKIE_options);
               $("#pastCities").html("");
           }
       }

       function GetCityLink(city) {
           /// <param name="city" type="String">city string</param>
           var link = "<a href='Default.aspx?city=" + city.replace(/(\s)/g, "%20") +
                        "'>" + city + "</a>";
           return link;
       }

       function InitializeCity(city, latitude, longtitude, cityID) {
           InitPastCities();

           if (city != null) {
               $("#SearchTitle").html(GetCityLink(city));
               $.cookie(COOKIE_NAME_ID, cityID, COOKIE_options);
               $.cookie(COOKIE_NAME_NAME, city, COOKIE_options);
               $.cookie(COOKIE_NAME_LAT, latitude, COOKIE_options);
               $.cookie(COOKIE_NAME_LON, longtitude, COOKIE_options);
               return;
           }

           //get called from aspx onLoad 
           var savedID = $.cookie(COOKIE_NAME_ID);
           var savedName = $.cookie(COOKIE_NAME_NAME);
           var savedLat = $.cookie(COOKIE_NAME_LAT);
           var savedLong = $.cookie(COOKIE_NAME_LON);

           if (savedID == null || savedName == null || savedLat == null || savedLong == null) {
               $("#SearchTitle").html(GetCityLink("Vancouver British Columbia"));
               return;
           }

           $('#SearchTitle').html(GetCityLink(savedName));
           $('.inputCurrentCityName').attr('value', savedName);
           $('.inputCurrentCityId').attr('value', savedID);
           map.LoadMap(new VELatLong(savedLat, savedLong), 12, 'h', false);
           $('.inputButtonChangeCityHotCat').click();
       }


       function CheckCsvFile(url, layerId, checked, mapIconUrl) {
           if (checked) {
               CreateLayerIfNotExists(layerId);
               layer[layerId].Show();
               ShowCsvFile(url, layerId, mapIconUrl);
           }
           else {
               layer[layerId].Hide();
           }
       }

       function ShowCsvFile(url, layerId, mapIconUrl) {
           var queryUrl = "ajaxway.svc/GetLocationsFromCSV";
           currentCallBackLayer = layerId;
           currentLayerIconUrl = mapIconUrl;

           if (url.indexOf("http") < 0) {
               //treat it as relative path, need full path
               currentUrl = document.URL;
               lastSlash = currentUrl.lastIndexOf("/");
               if (lastSlash >= 0) {
                   url = currentUrl.substring(0, lastSlash + 1) + url;
               }
           }

           $.ajax({
               url: queryUrl,
               type: "POST",
               contentType: "application/json; charset=utf-8",
               dataType: "json",
               data: '{"url": "' + url + '"}',
               success: function (result) {
                   var locations = result.d;
                   if (locations.length > 0) {
                       ResetLayer(currentCallBackLayer);
                   }

                   for (var i = 0; i < locations.length; i++) {
                       var location = locations[i];
                       AddPushpin(location.latitude, location.longitude, location.name, location.url, location.descr, currentCallBackLayer, true, currentLayerIconUrl);
                   }

               }
           });
       }

       function GetExternalDesc(id, imageUrl, url, urlLink1Mark, urlLink1, urlLink2Mark, urlLink2, rating, commentCount, name) {
           var desc = "";
           if (imageUrl != null && imageUrl != "") {
               desc += "<img src='" + imageUrl + "' class='locImageLink'>";
           }

           desc += '<p class="externalLink">';
           if (url != null && url != "") {
               desc += '<a href="' + url + '" target="_blank">Homepage</a> ';
           }

           if (urlLink1Mark != null && urlLink1Mark != "" && urlLink1 != null && urlLink1 != "") {
               desc += '<a href="' + urlLink1 + '" target="_blank">' + urlLink1Mark + '</a> ';
           }

           if (urlLink2Mark != null && urlLink2Mark != "" && urlLink2 != null && urlLink2 != "") {
               desc += '<a href="' + urlLink2 + '" target="_blank">' + urlLink2Mark + '</a> ';
           }

           desc += '<a href="#" onclick="ShowComment(\'' + EscapeSingleQuote(name) + "', " + id + ', 0);">'
           if (rating != null) {
               desc += "<img src='images/rating" + rating + ".gif' class='rating'>";
           }
           else {
               desc += "<img src='images/rating0.gif' class='rating'>";
           }

           if (commentCount != null) {
               desc += commentCount + ' comments</a> ';
           }
           desc += '</a> ';

           //if(user is who created the link)
           desc += ' <a href="#" class="SearchResultButton" onclick="ModifyLoc(' + id + ');">Modify</a> ';

           return desc;
       }

       function GetLocationDetail(name, lat, lon, layerId, mark) {
           var queryUrl = "/ajaxway.svc/GetLocationDetail";

           $.ajax({
               url: queryUrl,
               type: "POST",
               contentType: "application/json; charset=utf-8",
               dataType: "json",
               data: '{"name": "' + name + '","mlon":"' + lon + '","mlat":"' + lat + '"}',
               success: function (result) {
                   var detail = result.d;
                   var desc = "";

                   if (detail.length == 1) {
                       var loc = detail[0];

                       $('.publish_' + mark).hide();

                       desc += GetExternalDesc(loc.id, loc.imageUrl, loc.url, loc.urlLink1Mark, loc.urlLink1, loc.urlLink2Mark, loc.urlLink2, loc.rating, loc.commentCount, name);

                       $('.' + mark).html(desc);

                       var shape = layer[layerId].GetShapeByID(mark);
                       if (shape == null) return; //just in case something went wrong

                       var pattern = "\<a class=\"SearchResultButton publish_.*\>Publish\</a\>";
                       var rx = new RegExp(pattern);

                       var oriDesc = shape.GetDescription(desc).replace(rx, "");

                       shape.SetDescription(oriDesc + desc);
                   }
               }
           });
       }

       function ModifyLoc(id) {
           setupDataService();
           dataService.set_defaultFailedCallback(onFailure);
           dataService.set_defaultSucceededCallback(onSetupModifyLocation);
           dataService.query("Location?$filter=(LocationID eq " + id + ")");
       }

       function onSetupModifyLocation(result, userContext, operation) {
           var locations = result;
           if (locations.length != 1) return;
           var loc = locations[0];

           $("#inputLocationId").attr("value", loc.LocationID);
           $("#InputLatitude").attr("value", loc.Lat);
           $("#InputLongtitude").attr("value", loc.Lon);
           $("#InputLocationName").attr("value", loc.Name);
           $("#inputLocationdesc").attr("value", loc.Descr);

           $("#inputLocationUrl").attr("value", loc.Url);
           $("#inputAddress1").attr("value", loc.Address1);
           $("#inputAddress2").attr("value", loc.Address2);
           $("#inputPhone").attr("value", loc.Phone);
           $("#inputUrlLink1Mark").attr("value", loc.UrlLink1Mark);
           $("#inputUrlLink1").attr("value", loc.UrlLink1);
           $("#inputUrlLink2Mark").attr("value", loc.UrlLink2Mark);
           $("#inputUrlLink2").attr("value", loc.UrlLink2);
           $("#inputImageUrl").attr("value", loc.ImageUrl);
           //$("#inputListRating").attr("value", loc.Rating);

           if (loc.NeedEmailUpdate) {
               $("#inputEmailNotify").attr("checked", "checked");
           }
           else {
               $("#inputEmailNotify").attr("checked", "");
           }
           InitCategory();

           $('#PublishDivId').dialog('option', 'title', "Modify this location");
           $("#InputButtonPublish").attr("value", "Modify this location");
           $("#publishNote").html("Note: Only user published this location can modify it.");
           $('#PublishDivId').dialog('open');
       }

       var currentCommentSkip = 0;
       var currentCommentId = 0;
       var currentCommentName = 0;
       function ShowComment(name, id, skip) {
           setupDataService();
           currentCommentSkip = skip;
           currentCommentId = id;
           currentCommentName = name;
           dataService.set_defaultFailedCallback(onFailure);
           dataService.set_defaultSucceededCallback(onShowComments);
           dataService.query("vCommentUser?$filter=LocationID eq " + id + "&$orderby=CommentDate desc&$top=5&$skip=" + skip);

           $("#inputCommentLocID").attr("value", id);

           if (!$('#ShowCommentId').dialog('isOpen')) {
               $('#ShowCommentId').dialog('open');
           }
           $('#ShowCommentId').dialog('option', 'title', name);
       }

       function onShowComments(result, userContext, operation) {
           var comments = result;
           var desc = "";  //"<h3>Existing Comments for " + currentCommentName + "</h3>";
           for (var i = 0; i < comments.length; i++) {
               var comment = comments[i];

               desc += '<p><span>' + comment.CommentDate + "&nbsp;&nbsp;" + "</span>";
               desc += "<img src='images/rating" + comment.Rating + ".gif' class='rating'><BR/>";
               desc += comment.CommentText + "</p>";
           }

           if (currentCommentSkip >= 5) {
               desc += "<a href='#' onclick=\"javascript:ShowComment('" + EscapeSingleQuote(currentCommentName) + "'," + currentCommentId + "," + (currentCommentSkip - 5) + ");\">" + "&lt;&lt;</a>&nbsp;&nbsp;";
           }
           if (comments.length >= 5) {
               desc += "<a href='#' onclick=\"javascript:ShowComment('" + EscapeSingleQuote(currentCommentName) + "'," + currentCommentId + "," + (currentCommentSkip + 5) + ");\">" + "&gt;&gt;</a>";
           }

           $("#ShowCommentIdInDiv").html(desc);
       }

       function CommentSubmit() {

           if ($('#CurrentLiveUser').html() == "") {
               alert("You need to login to windows live account to comment.");
               return;
           }

           if ($("#inputCommentLocID").attr('value') == "") {
               alert("Please choose a location to comment");
               return;
           }
           if ($("#inputComment").attr('value') == "") {
               alert("Please write a comment");
               return;
           }

           if ($("#inputComment").attr('value').length > 140) {
               alert("Please make sure comment length is less than 140");
               return;
           }

           $('.inputComment').attr("value", $("#inputComment").attr("value"));
           $('.inputListRating').attr("value", $("#inputListRating").attr("value"));
           $('.inputCommentLocID').attr("value", $("#inputCommentLocID").attr("value"));

           $('.inputButtonComment').click();

           $('#ShowCommentId').dialog('close');
       }
      
    
/* END EXTERNAL SOURCE */
