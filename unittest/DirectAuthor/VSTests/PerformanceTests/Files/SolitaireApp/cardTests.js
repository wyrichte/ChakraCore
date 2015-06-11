/// <reference path="js/base.js" />
/// <reference path="js/ui.js" /> 
/// <reference path="js/wwaapp.js" />
/// <reference path="js/xhr.js" /> 
/// <reference path="js/win8ui.js" />
/// <reference path="cards.js" />

function autoTurn(game) {
    if (!game) { throw "need a game"; }
    var k = Cards.Model.Klondike;

    function moveAnyToFoundation() {
        function moveToFoundation(sourceLoc, destLoc) {
            var check = game.history[game.history.length - 1];
            if (check != Cards.Model.Klondike.stringify(game.model)) { throw "invalid history"; }
            if (check != Cards.Model.Klondike.stringify(Cards.Model.Klondike.parse(check))) { throw "invalid history"; }

            if (game.move(sourceLoc, destLoc)) {
                lastRealMoveCycle = cycle;

                var check2 = game.history[game.history.length - 2];
                if (check != check) { throw "invalid history"; }

                return true;
            }
            return false;
        }
        var done = false;
        game.model.tableaus.forEach(function(tab, tabIndex) {
            game.model.foundations.forEach(function(f, foundIndex) {
                if (!done) { 
                    done = moveToFoundation(
                        {field:"tableaus", index:tabIndex, position: tab.length - 1}, 
                        {field:"foundations", index:foundIndex}
                    ); 
                }
            });
        });

        game.model.foundations.forEach(function(f, foundIndex) {
            if (!done) { 
                done = moveToFoundation(
                    {field:"waste", position: game.model.waste.length - 1}, 
                    {field:"foundations", index:foundIndex}
                ); 
            }
        });
        return done;
    };

    function moveAnyKingToOpenTableau() {
        var done = false

        game.model.tableaus.forEach(function(dest) {
            if (dest.length == 1) {
                game.model.tableaus.forEach(function(source) {
                    if (!done) {
                        if (source.length > 2 && !source[1].flipped) {
                            for (var p=1, l=source.length; p<l; p++) {
                                var card = source[p];
                                if (card.flipped && card.number == "K") {
                                    game.move(card._location, k.top(dest)._location);
                                    lastRealMoveCycle = cycle;
                                    done = true;
                                    break;
                                }
                            };
                        }
                    }
                });
            }
        });

        return done;
    };
    
    function moveWithinTableaus() {
        var done = false

        game.model.tableaus.forEach(function(dest) {
            if (dest.length > 1) {
                game.model.tableaus.forEach(function(source) {
                    if (!done) {
                        if (source.length > 1) {
                            for (var i=1; i<source.length; i++) {
                                if (source[i].flipped) {
                                    if (game.move(source[i]._location, k.top(dest)._location)) {
                                        lastRealMoveCycle = cycle;
                                        done = true;
                                    }
                                    break;
                                }
                            };
                        }
                    }
                });
            }
        });

        return done;
    };
        
    function moveFromWaste() {
        var done = false

        game.model.tableaus.forEach(function(dest) {
            if (!done) {
                var source = game.model.waste;
                if (source.length > 1) {
                    if (game.move(k.top(source)._location, k.top(dest)._location)) {
                        lastRealMoveCycle = cycle;
                        done = true;
                    }
                }
            }
        });

        return done;
    };

    function fromDeck() {
        var done = false;
        if (game.model.deck.length > 1) {
            game.dealCardsToWaste();
            done = true;
        }
        return done;
    }

    function restack() {
        var done = false;
        if (game.model.waste.length > 1) {
            game.stackDeckFromWaste();
            done = true;
        }
        return done;
    }

    function flipAnyTableaus() {
        game.model.tableaus.forEach(function(tab) {
            var u = tab[tab.length - 1]._ui;
            if (!u.flipped) {
                u.flipped = true;
                game.pushHistory();
            }
        });
    };

    var done = moveAnyToFoundation();
    if (!done) { done = moveAnyKingToOpenTableau(); }
    if (!done) { done = moveWithinTableaus(); }
    if (!done) { done = moveFromWaste(); }
    if (!done) { done = fromDeck(); }
    if (!done) { done = restack(); }
    if (done) {
        Win.Application.setTimeout(flipAnyTableaus, 64);
    }
    return done;
}
var foreverStart;
var foreverGameCount;
var cycle;
var lastRealMoveCycle;

function autoGame(game, completed) {
    if (!game) { throw "need a game"; }
    cycle = 0;
    lastRealMoveCycle = 0;
    autoGameWorker(game, completed);
}

function autoForever() {
    foreverStart = new Date();
    foreverGameCount = 0;
    autoForeverWorker();
}
function autoForeverWorker() {
    Win.Application.setTimeout(function() {
        foreverGameCount++;
        document.title = "#" + foreverGameCount + " @ " + (((new Date() - foreverStart)/(1000*60))>>0) + "min";
        newGame(undefined, function(game) {
            Win.Application.setTimeout(function() {
                autoGame(game, 
                    function() {
                        Win.Application.setTimeout(function() { autoForeverWorker(game); }, 3000);
                    }
                );
            }, 7000);
        });
    }, 500);
}

function autoGameWorker(game, completed) {
    if (!game) { throw "need a game"; }
    cycle++;
    if (cycle - lastRealMoveCycle < 15 && autoTurn(game)) {
        Win.Application.setTimeout(function() { autoGameWorker(game, completed); }, 512);
    }
    else {
        if (completed) { completed(); }
    }
}

function runTests() {
    function a(cond) { if (!cond) { throw "failed assert"; } }

    var k = Cards.Model.Klondike;

    var game1 = k.createGame();
    k.deal(game1);
    a(game1.tableaus.length == 7);
    a(game1.foundations.length == 4);
    a(!game1.foundations[0].suit);
    a(!game1.foundations[1].suit);
    a(!game1.foundations[2].suit);
    a(!game1.foundations[3].suit);

    a(k.isLegalMove(
        [
            {flipped:true},
            {suit:"diamonds", number:"2", flipped:true},
            {suit:"clubs", number:"A", flipped:true}
        ], 
        2, 
        [{flipped:true}], 
        "foundations"
    ));

    a(k.isLegalMove(
        [
            {flipped:true},
            {suit:"diamonds", number:"2", flipped:true},
            {suit:"clubs", number:"A", flipped:true}
        ], 
        2, 
        [
            {flipped:true},
            {suit:"diamonds", number:"K", flipped:false},
            {suit:"hearts", number:"2", flipped:true},
        ], 
        "tableaus"
    ));

    // document.title = "passed: " + new Date();
}