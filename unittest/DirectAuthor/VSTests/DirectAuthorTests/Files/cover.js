/// <reference path="../../Corsica/js/base.js" />
/// <reference path="../../Corsica/js/ui.js" />
/// <reference path="../../jquery-1.4.3.js" />

//document.addEventListener("DOMContentLoaded", delayStart, false);

function delayStart() {
	setTimeout(animate, 50);
}

/*
Set up prefix for property names and events
*/
var nspc = null;
var ua = navigator.userAgent.toLowerCase();
if (ua.indexOf("chrome") != -1 || ua.indexOf("applewebkit") != -1) {
    nspc = "-webkit-"
} else if (ua.indexOf("msie") != -1) {
    nspc = "-ms-";
} else if (ua.indexOf("gecko") != -1) {
    nspc = "-moz-";
}

var enspc = nspc.replace('-', '').replace('-', '')
enspc = enspc.replace('ms', 'MS');
enspc = enspc.replace('moz', 'Moz');

// 
// Property names
//
var transitionProperty = nspc + "transition-property";
var transitionDuration = nspc + "transition-duration";
var transitionDelay = nspc + "transition-delay";
var transform = nspc + "transform";

//
// Event names
//
var transitionEndEvent = enspc + "TransitionEnd";

//
// Misc global vars
//
var bringInThumbDuration = 1.2;
var thumbCount = 4;

// Used to track which content should be displayed in the preview pane on a given "flip", 
// as well as which thumbnail to transition
var previewCounter = 0;

var snapInTransforms = new Array(
                            "translateX(-275px) translateY(175px) rotateZ(-5deg) rotateY(180deg)",
                            "translateX(-150px) translateY(175px) rotateZ(5deg) rotateX(180deg)",
                            "translateX(0px) translateY(175px) rotateY(180deg)",
                            "translateX(125px) translateY(175px) rotateZ(-5deg) rotateY(180deg)"
                            );
                            
var articleTitles = new Array(
                            "Conquering A Cave",
                            "Telltale Scribes",
                            "Snub-Nosed Monkeys",
                            "Flashback: The Big Dip"
                            );
                            
var articleCredits = new Array(
                            "By Mark Jenkins - Photographs by Carsten Peter",
                            "By Peter Gwin - Photographs by Brent Stirton", 
                            "By Jennifer S. Holland - Photographs by Cyril Ruoso",
                            "By Margaret G. Zackowitz"
                            );
                            
var articleContent = new Array(
                            "Explorers scramble to the end of Vienam's infinite cavern",
                            "Timbuktu's books and letters are historic, magical, romantic.",
                            "The heavy fur of China's snub-nosed monkey is a boon in subzero winters. Its quirky face could help too.",
                            "In an image from 1913, hundreds of thousands flock to a town in India to wash away their sins."
                            );

////////////////////////////////////////////////
///
////////////////////////////////////////////////
function event_defocusThumb(event)
{
    var thumb = event.target;
    thumb.removeEventListener(transitionEndEvent, event_wrapper_defocusThumb);

    /*if( ( thumbCount - 1 ) != parseInt(thumb.id.replace("thumb_", "") ) )
    {*/
        thumb.style[transitionDelay] = "1s";
        thumb.style[transform] = snapInTransforms[parseInt(thumb.id.replace("thumb_", ""))];
    //}
}

////////////////////////////////////////////////
///
////////////////////////////////////////////////
function event_wrapper_defocusThumb(event)
{
    setTimeout(
        function() {
            event_defocusThumb(event)
            },
        10
        );
        
}

////////////////////////////////////////////////
///
////////////////////////////////////////////////
function getThumbIndex( thumbId )
{
    var thumbIndex = thumbId.replace( "thumb_", "") ;
    return parseInt( thumbIndex );
}

////////////////////////////////////////////////
///
////////////////////////////////////////////////
function getChildByClass(parent, className)
{
    var children = parent.children;
    for( var i = 0; i < children.length; i++ )
    {
        if( className == children[i].className )
        {
            return children[i];
        }
    }
    
    return null;
}

////////////////////////////////////////////////
///
////////////////////////////////////////////////
function event_swapPreviews(event)
{
    var articlePreview = event.target;
    
    if( previewCounter == articleTitles.length )
    {
        /*
		articlePreview.removeEventListener(transitionEndEvent, event_wrapper_swapPreviews);*
        return;*/
		previewCounter = 0;
    }
    
    //
    // Load content on back element and then flip
    //
    articlePreview.style[transitionProperty] = transform;
    if( "rotateX(180deg)" == articlePreview.style[transform])
    {
        //
        // Load the article preview data
        //
		var previewTitle = getChildByClass(articlePreview, "preview_title");
        previewTitle.innerText = articleTitles[previewCounter];

        var previewContent = getChildByClass(articlePreview, "preview_content");
        previewContent.innerText = articleContent[previewCounter];
        
		/*
        var previewCredits = getChildByClass(articlePreview, "preview_credits");
        previewCredits.innerText = articleCredits[previewCounter];
        */

        var thumb = document.getElementById("thumb_" + previewCounter);   
        focusThumb(thumb);

        articlePreview.style[transitionDelay] = "0s";
        articlePreview.style[transform] = "rotateX(0deg)";
	    
        previewCounter++;      
    }
    else
    {
		articlePreview.style[transitionDelay] = "1s";
		articlePreview.style[transform] = "rotateX(180deg)";
	}
}

////////////////////////////////////////////////
///
////////////////////////////////////////////////
function event_wrapper_swapPreviews(event)
{
    setTimeout(
        function() {
            event_swapPreviews(event);
        },
        10
        );
}

////////////////////////////////////////////////
///
////////////////////////////////////////////////
function focusThumb(thumb)
{
    thumb.addEventListener(transitionEndEvent, event_wrapper_defocusThumb);
    thumb.style[transitionProperty] = transform;
    thumb.style[transitionDuration] = "1s";
	thumb.style[transitionDelay] = "0s";
    thumb.style[transform] = "rotateZ(0deg) translateX(-400px) translateY(460px) scale(2.00)";
}

////////////////////////////////////////////////
///
////////////////////////////////////////////////
function event_bringInPreview(event)
{
    var thumb = event.target;
    thumb.removeEventListener(transitionEndEvent, event_wrapper_bringInPreview);

    if((thumbCount - 1) == getThumbIndex(thumb.id))
    {    
        //
        // Transition the article thumbnail
        //
        var thumb = document.getElementById("thumb_0");
        focusThumb(thumb);
    
        //
        // Transition the article preview pane
        //
        var articlePreview = document.getElementById("article_preview");
        articlePreview.addEventListener(transitionEndEvent, event_wrapper_swapPreviews);
        
        articlePreview.style[transitionProperty] = "opacity";
        articlePreview.style[transitionDuration] = "1s";
        articlePreview.style.opacity = 1;		
      
        //
        // Load and transition the article preview data
        //
        var previewTitle = getChildByClass(articlePreview, "preview_title");
        previewTitle.innerText = articleTitles[previewCounter];

        var previewContent = getChildByClass(articlePreview, "preview_content");
        previewContent.innerText = articleContent[previewCounter];

        /*
        var previewCredits = getChildByClass(articlePreview, "preview_credits");
        previewCredits.innerText = articleCredits[previewCounter];
        */
        previewCounter++;   
    }
}

////////////////////////////////////////////////
///
////////////////////////////////////////////////
function event_wrapper_bringInPreview(event)
{
    setTimeout(
        function() {
            event_bringInPreview(event)
        },
        10
        );
}

////////////////////////////////////////////////
///
////////////////////////////////////////////////
function event_snapInThumb(event)
{
    var thumb = event.target;
    
    thumb.removeEventListener(transitionEndEvent, event_wrapper_snapInThumb);
    thumb.addEventListener(transitionEndEvent, event_wrapper_bringInPreview);
 
    thumb.style[transitionDelay] = "0s";
    thumb.style[transform] = snapInTransforms[parseInt(thumb.id.replace("thumb_", ""))];
    
}

////////////////////////////////////////////////
///
////////////////////////////////////////////////
function event_wrapper_snapInThumb(event)
{
    setTimeout(
        function() {
            event_snapInThumb(event)
            },
        10
        );
}

////////////////////////////////////////////////
///
////////////////////////////////////////////////
function bringInThumb(thumb)
{
    thumb.addEventListener(transitionEndEvent, event_wrapper_snapInThumb);

    thumb.style[transitionProperty] = transform;
    thumb.style[transitionDuration] = bringInThumbDuration + "s";
    thumb.style[transitionDelay] = getThumbIndex(thumb.id)*1.2 + "s";
    thumb.style[transform] = "translateX(-175px) translateY(300px)";
}


////////////////////////////////////////////////
///
////////////////////////////////////////////////
function bringInThumbs()
{
    var thumb01 = document.getElementById("thumb_0");
    bringInThumb(thumb01);

    var thumb02 = document.getElementById("thumb_1");
    bringInThumb(thumb02);
    
    var thumb03 = document.getElementById("thumb_2");
    bringInThumb(thumb03);
    
    var thumb04 = document.getElementById("thumb_3");
    bringInThumb(thumb04);
}

////////////////////////////////////////////////
///
////////////////////////////////////////////////
function event_wrapper_bringInThumbs(event)
{
    setTimeout(
        function() {
            event_bringInThumbs(event);
        },
        10
        );
}

////////////////////////////////////////////////
///
////////////////////////////////////////////////
function event_bringInThumbs(event)
{
    var background = event.target;
    background.removeEventListener(transitionEndEvent, event_wrapper_bringInThumbs);
    
    bringInThumbs();
}

////////////////////////////////////////////////
///
////////////////////////////////////////////////
function animate()
{
    //
    // Animate the background
    //
	bringInThumbs();
    /*
	var background = document.getElementById("background");
    background.addEventListener(transitionEndEvent, event_bringInThumbs);
    
    background.style[transitionProperty] = transform;
    background.style[transitionDuration] = "2.5s";
    background.style[transform] = "translateX(-600px) translateY(-400px)";
	*/
    
}