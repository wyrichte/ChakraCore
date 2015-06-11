/// <reference path="js/base.js" />
/// <reference path="js/ui.js" /> 
/// <reference path="js/wwaapp.js" />
/// <reference path="js/xhr.js" /> 
/// <reference path="js/win8ui.js" />

(function (global) {
    var wa = Win.Application;

    function shuffle(a) {
        // Durstenfeld shuffle [http://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle]
        //
        var n = a.length;
        for(var i = n - 1; i > 0; i--) {
            var j = Math.floor(Math.random() * (i + 1));
            var tmp = a[i];
            a[i] = a[j];
            a[j] = tmp;
        }
        return a;
    };

    function top(a) {
        return a[a.length - 1];
    };

    function suitColor(suit) {
        if (!suit) { return; }
        if (suit == "clubs" || suit == "spades") { return "black"; }
        return "red";
    };

    function klondikeOrder(number) {
        switch (number) {
            case "A": return 1;
            case "J": return 11;
            case "Q": return 12;
            case "K": return 13;
            default:
                return new Number(number);
        }
    };

    Win.Namespace.define("Cards.Model", {
            suits: ["clubs", "spades", "hearts", "diamonds"],
            numbers: [2,3,4,5,6,7,8,9,10,"J","Q","K", "A"],
            suitColor: suitColor,
            cardNumber: function(card) {
                if (!card.suit) { return 54; }
                var n;
                switch (card.number) {
                    case "J": n = 10; break;
                    case "Q": n = 11; break;
                    case "K": n = 12; break;
                    case "A": n = 13; break;
                    default:
                        n = new Number(card.number) - 1;
                        break;
                }
                switch (card.suit) {
                    case "clubs": break;
                    case "spades": n += 13; break;
                    case "hearts": n += 26; break;
                    case "diamonds": n += 39; break;
                }
                return n;
            },
            imageName: function(card) {
                var n = Cards.Model.cardNumber(card);
                var prefix = "cards-";
                if (n < 10) {
                    prefix += "0";
                }
                return prefix + n;
            }
        }
    );

    Win.Namespace.define("Cards.Model.Klondike", {
            stringify: function(game) {
                function stringifyCard(card) {
                    var number = Cards.Model.cardNumber(card);
                    if (card.flipped) { number += 128; }
                    var str = number.toString(16);
                    if (str.length == 1) { str = "0" + str; }
                    if (str.length > 2) { throw "bad developer"; }
                    return str;
                };

                var result = "/";
                function s(e) {
                    result += stringifyCard(e);
                }

                game.deck.forEach(s);
                result += "/";
                game.waste.forEach(s);
                result += "/";
                game.tableaus.forEach(function(t, i) {
                    t.forEach(s);
                    result += "/";
                });
                game.foundations.forEach(function(f, i) {
                    f.forEach(s);
                    result += "/";
                });
                return result;
            },
            parse: function(text) {
                function parseCard(text, start) {
                    var card = text.substring(start, start + 2);
                    var n = parseInt(card, 16);

                    var result = {};
                    if (n >= 128) {
                        result.flipped = true;
                        n -= 128;
                    }
                    if (n != 54) { 
                        n = n - 1;
                        var suit = (n / 13) >> 0;
                        var c = n % 13;
                        result.suit = Cards.Model.suits[suit];
                        result.number = Cards.Model.numbers[c];
                    }
                    return result;
                };
                var segments = text.split("/").splice(1,14);
                var game = {};
                game.deck = [];
                var segidx = 0;
                var seg = segments[segidx++];
                for(var i=0;i<seg.length; i+=2) {
                    game.deck.push(parseCard(seg, i));
                }
                game.waste = [];
                var seg = segments[segidx++];
                for(var i=0;i<seg.length; i+=2) {
                    game.waste.push(parseCard(seg, i));
                }
                game.tableaus = [];
                for(var t=0;t<7; t++) {
                    game.tableaus.push([]);
                    var seg = segments[segidx++];
                    for(var i=0;i<seg.length; i+=2) {
                        game.tableaus[t].push(parseCard(seg, i));
                    }
                }
                game.foundations = [];
                for(var t=0;t<4; t++) {
                    game.foundations.push([]);
                    var seg = segments[segidx++];
                    for(var i=0;i<seg.length; i+=2) {
                        game.foundations[t].push(parseCard(seg, i));
                    }
                }
                game.state = "playing";
                Cards.Model.Klondike.updateLocations(game);
                return game;
            },
            createGame: function() {
                var game = {};
                game.deck = [];
                Cards.Model.suits.forEach(function(suit) {
                    Cards.Model.numbers.forEach(function(number) {
                        game.deck.push({ suit: suit, number: number, flipped: false });
                    });
                });
                shuffle(game.deck);
                function slot() { return [{flipped:true}]; }
                game.deck = slot().concat(game.deck);

                game.foundations = [ slot(), slot(), slot(), slot() ];
                game.waste = slot();
                game.tableaus = [];
                for (var i=0; i<7; i++) { game.tableaus.push(slot()); }
                game.state = "initialized";
                return game;
            },

            updateLocations: function(game) {
                function s(e, field, pos, index) {
                    e._location = { field: field, position: pos, index: index };
                };
                game.deck.forEach(function(e, pos) { 
                    s(e, "deck", pos);
                });
                game.waste.forEach(function(e, pos) { 
                    s(e, "waste", pos);
                });
                game.tableaus.forEach(function(t, i) {
                    t.forEach(function(e, pos) { 
                        s(e, "tableaus", pos, i);
                    });
                });
                game.foundations.forEach(function(f, i) {
                    f.forEach(function(e, pos) { 
                        s(e, "foundations", pos, i);
                    });
                });
            },

            deal: function(game, singleComplete, allComplete) {
                if (game.state != "initialized") { throw "can only deal once"; }
                
                var c = 0;
                var f = 100;

                for(var pass=0; pass<7; pass++) {
                    for (var start=pass; start<7; start++) {
                        (function(i) {
                            wa.setTimeout(function() {
                                var moving = game.deck.pop();
                                game.tableaus[i].push(moving);
                                if (singleComplete) { singleComplete(moving); }
                            }, (c++) * f);
                        })(start);
                    }
                }
                c+=3;
                for(var pass=0; pass<7; pass++) {
                    (function(i) {
                        wa.setTimeout(function() {
                            var moving = top(game.tableaus[i]);
                            moving.flipped = true;
                            if (singleComplete) { singleComplete(moving); }
                        }, (c++) * f);
                    })(pass);
                }
                wa.setTimeout(function() {
                    Cards.Model.Klondike.updateLocations(game);
                    game.state = "playing";
                    if (allComplete) { allComplete(); }
                }, (c++) * f);
                return game;
            },

            stack: function(gameModel, location) {
                var f = gameModel[location.field];
                if (location.index !== undefined) { return f[location.index]; }
                return f;
            },

            top: top,

            isLegalMove: function(fromStack, fromStackOffset, toStack, toStackType) {
                bottomOfStack = fromStack[fromStackOffset];
                if (!bottomOfStack.flipped) { return false; }
                var bottomColor = suitColor(bottomOfStack.suit);
                if (!bottomColor) { return false; }

                var topOfStack = top(toStack);
                if (!topOfStack.flipped) { return false; }
                var topColor = suitColor(topOfStack.suit);
                switch (toStackType) {
                    case "foundations":
                        var tests = {
                            moveTo: "foundations",
                            onlyOne: fromStack.length - fromStackOffset == 1,
                            emptyAcceptsAnyAce: !topColor && bottomOfStack.number == "A",
                            matchedSuit: bottomOfStack.suit == topOfStack.suit,
                            oneHigher: klondikeOrder(bottomOfStack.number) == klondikeOrder(topOfStack.number) + 1
                        };

                        return tests.onlyOne && (tests.emptyAcceptsAnyAce || (tests.matchedSuit && tests.oneHigher));
                    case "tableaus":
                        var tests = {
                            moveTo: "tableaus",
                            emptyAcceptsAnyKing: !topColor && bottomOfStack.number == "K",
                            alternateColor: bottomColor != topColor,
                            oneLower: klondikeOrder(bottomOfStack.number) == klondikeOrder(topOfStack.number) - 1
                        };
                        return tests.emptyAcceptsAnyKing || (tests.alternateColor && tests.oneLower);
                    default:
                        return false;
                }
                
                return false;
            }
        }
    );
})(this);