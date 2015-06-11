/// <reference path="js/base.js" />
/// <reference path="js/wwaapp.js" />

// UNDONE: Win.Application.connect() 
// var app = Win.Application.connect();

animations = [];
app.addEventListener("frame", function(fps, elapsedFrames, elapsedTime, totalTime) {
    var any = false;
    animations.forEach(function (a) {
        if (!a.complete) {
            if (totalTime >= a.startTime + a.duration) {
                a.invoke(1.0);
                a.complete = true;
                any = true;
            }
            else {
                var percent = (totalTime - a.startTime) / a.duration;
                if (percent >= 0) {
                    a.invoke(percent);
                }
            }
        }

    });
    if (any) {
        animations = animations.filter(function (a) { return !a.complete; });
    }
});
