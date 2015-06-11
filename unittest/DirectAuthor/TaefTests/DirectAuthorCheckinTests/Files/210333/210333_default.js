/// <reference path="../default.html" />

var publicFeedUrl = "http://officetalk/api/feeds/public.json";
var personalFeedUrl = "http://officetalk/api/feeds/personal.json";
var menuPublicFeed = null;
var menuPersonalFeed = null;
var lastRefresh = null;


//Debug.enableFirstChanceException(true);

var app = Win.Application;



app.addEventListener("loaded", function() {

    // hook up click handlers:
    var reload = document.getElementById("reload");
    reload.addEventListener("click", refreshFeeds);

    menuPublicFeed = document.getElementById("navPublic");
    menuPublicFeed.addEventListener("click", publicFeed_click);

    menuPersonalFeed = document.getElementById("navPersonal");
    menuPersonalFeed.addEventListener("click", personalFeed_click);

    // fix up div widths and position:
    var width = window.innerWidth;

    var divContent = document.getElementById("content");
    divContent.style.left = "0px";

    var divPub = document.getElementById("publicFeed");
    divPub.style.left = "0px";
    divPub.style.width = width.toString() + 'px';

    var divPers = document.getElementById("personalFeed");
    divPers.style.left = width.toString() + 'px';
    divPers.style.width = width.toString() + 'px';

    refreshFeeds();
});

app.start();


function refreshFeeds() {

    //var currentTime = new Date().getTime();
    //if (lastRefresh != null && currentTime - lastRefresh < 60000) {
    //    console.log("Not refreshing feeds - too soon.");
    //    return;
    //}
    //lastRefresh = currentTime;
    // console.log("Last refresh at " + lastRefresh);
    var currentTime = new Date().getTime();
    if (lastRefresh != null && currentTime - lastRefresh < 60000) {
        console.log("Not refreshing feeds - too soon.");
        return;
    }
    lastRefresh = currentTime;

    console.log("Refreshing feeds...");
    Win.xhr({ url: publicFeedUrl }).then(processPublicPosts, feedError);  
    Win.xhr({ url: personalFeedUrl }).then(processPersonalPosts, feedError);

}

function publicFeed_click() {

    var divContent = document.getElementById("content");
    
 
    //divContent.style.left = "0px";
    
    $("#content").animate({
    'left': "+=" + window.innerWidth.toString() + "px"
    });

    menuPersonalFeed = document.getElementById("navPersonal");
    menuPublicFeed = document.getElementById("navPublic");

    menuPublicFeed.className = "navItem-selected";
    menuPersonalFeed.className = "navItem";

    refreshFeeds();
}

function personalFeed_click() {
    
    var divContent = document.getElementById("content");
    //divContent.style.left = '-' + window.innerWidth.toString() + 'px';

    $("#content").animate({
    'left': "-=" + window.innerWidth.toString() + "px"
    });

    menuPersonalFeed = document.getElementById("navPersonal");
    menuPublicFeed = document.getElementById("navPublic");

    menuPublicFeed.className = "navItem";
    menuPersonalFeed.className = "navItem-selected";

    refreshFeeds();
}


function feedError(request) {
    if (request.status == 0) {
        reportStatus(request.status);
    }
    else {
        reportFeedError(request.status);
    }
}

function reportFeedError(msg) {
    statusMsg.innerHTML = "Error downloading posts: " + msg;
}

function reportStatus(msg) {
    statusMsg.innerHTML = "Status: " + msg;
}

function processPublicPosts(request) {
    var divPub = document.getElementById("publicFeed");
    divPub.innerHTML = "";
    var items = processPosts(request);
    divPub.innerHTML = items;
}

function processPersonalPosts(request) {
    var divPers = document.getElementById("personalFeed");
    divPers.innerHTML = "";
    var items = processPosts(request);
    divPers.innerHTML = items;

    
}

function processPosts(request) {

    var items = eval('(' + request.responseText + ')');

|
    
    var feedItems = "";
    for (i in items.d) {
        var post = {
            message: items.d[i].text,
            author: items.d[i].user.alias,
            image: items.d[i].user.image_url,
            pubDate: items.d[i].created_at
        };

        console.log('Loaded message from: ' + post.author);
        feedItems += postToHtml(post);
    }

    return feedItems;
}

function postToHtml(post) {

    var date = new Date(post.pubDate);
    
    return '<div class="post"><div class="userImage"><img src="http://officetalk' + post.image + '" /></div><div class="postText"><div><span class="author">' + post.author + ':</span><span class="pubDate"> at ' + date + '</span></div><div class="message">' + post.message + '</div></div></div>';

}
