/// <reference path='../default.html' />
var test = 2;

(function(){
    var app = Win.Application;

    try {
    var foo = Windows.ApplicationModel.Activation();
    foo.onActivated = function onActivated(contractId, contractActivationContext) {
    };
    } catch(e) {
        var bar = test.|;
    app.addEventListener('loaded', function() {
        // Ensure that all data-win-controls are processed. If you also have
        // any databinding on the page you should call: Win.Binding.processAll().
        Win.UI.processAll();

        // Hookup the default keybindings for the application.
        document.body.addEventListener('keyup', function(e) {test.|;
            if (e.altKey && (e.keyCode === 37)) { Win.Navigation.back(); }
            if (e.altKey && (e.keyCode === 39)) { Win.Navigation.forward(); }
        });

        // TODO: startup code here
    });

    Win.Navigation.addEventListener('navigated', function(e) {
        var navFrame = document.getElementById('navFrame');

        Win.UI.Fragments.clone(e.detail.location, e.detail.state).then(function(frag) {
            navFrame.innerHTML = '';
            navFrame.appendChild(frag);
            Win.UI.processAll(navFrame).then(function() {
                //Win.Binding.processAll(navFrame, e.detail.state).then(function() {
                    navFrame.focus(); // makes keystrokes work

                    var faElement = navFrame.querySelector('[data-fragmentadded]');
                    if (faElement) {
                        var fragmentadded = Win.Utilities.getMember(faElement.getAttribute('data-fragmentadded'));
                        if (fragmentadded) { fragmentadded(navFrame.childNodes[0], Win.Navigation.history.current.state); }
                    }
                //})
            })
        });

        // enable back button based on history stack
        if (Win.Navigation.canGoBack) { Win.Utilities.removeClass(backButton, 'disabled'); }
        else { Win.Utilities.addClass(backButton, 'disabled'); }
    });
  
    app.getLayoutMode = function () {
        // based on target resolution of 1366x768:
        if (window.innerWidth <= 767) { return 'snapped'; }
        else if (window.innerWidth <= 1023) { return 'portrait'; }
        else if (window.innerWidth <= 1365) { return 'filled'; }
        else { return 'landscape'; }
    }

    var oldLayoutMode = ''; // there's a bug in PDC-7, for relayout first time.
    window.addEventListener('resize', function(e) {
        var newLayoutMode = app.getLayoutMode();
        if (newLayoutMode !== oldLayoutMode) {
            app.queueEvent({ type: 'layoutModeChanged', detail: { layoutMode: newLayoutMode, oldLayoutMode: oldLayoutMode }});
            oldLayoutMode = newLayoutMode;
        }
    });

    Win.Navigation.home = function() {
        // Already home
        if (!Win.Navigation.canGoBack) { return ; }
        if (Win.Navigation.history.backStack[0].location === Win.Navigation.history.current.location) { return; }

        // Push the home location onto the stack again
        var home = Win.Navigation.history.backStack[0];
        Win.Navigation.navigate(home.location, home.state);
    };

    app.start();
})();
