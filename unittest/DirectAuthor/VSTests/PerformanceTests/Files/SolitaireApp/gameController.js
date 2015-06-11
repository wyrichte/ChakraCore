/// <reference path="js/base.js" />
/// <reference path="js/ui.js" /> 
/// <reference path="js/wwaapp.js" />
/// <reference path="js/xhr.js" /> 
/// <reference path="js/win8ui.js" />
/// <reference path="cards.js" />
/// <reference path="cardsui.js" />

(function() {       
    var k = Cards.Model.Klondike;
    var ui = Cards.UI.Klondike;

    function spacing(col, row) {
        var maxX = 6 * ui.cardWidth * 1.5 + ui.cardWidth; 
        var offsetX = (1266 - maxX) / 2
        return { y: 10 + row * ui.cardHeight * 1.15, x: offsetX + col * ui.cardWidth * 1.5 };
    };

    Game = Win.Class.define(null, {
            move: function(fromLoc, toLoc) {
                var toStack = k.stack(this.model, toLoc);
                var fromStack = k.stack(this.model, fromLoc);

                if ((fromLoc.field == "tableaus" || fromLoc.field == "waste")
                    && toLoc.field != "waste"
                    && k.isLegalMove(
                        fromStack,
                        fromLoc.position,
                        toStack,
                        toLoc.field
                    )) {
                    
                    var count = fromStack.length - fromLoc.position;
                    if (fromLoc.field == "waste") {
                        count = 1;
                    }
                    k.stack(this.ui, toLoc).add(k.stack(this.ui, fromLoc).pull(count));
                    this.pushHistory();
                    return true;
                }
                return false;
            },

            layoutGame: function (full) {
                this.ui.tableaus.forEach(function(col) { col.updateRenderOrder(); col.spreadDown(); });
                this.ui.foundations.forEach(function(col) { col.updateRenderOrder(); col.square(); });
                this.ui.waste.updateRenderOrder();
                this.ui.waste.spreadRight();
                if (full) {
                    this.ui.deck.updateRenderOrder();
                    this.ui.deck.square();
                }
                Cards.UI.Klondike.updateRenderOrder(this.uihost);
            },
            stackDeckFromWaste: function () {
                var that = this;
                var items = this.ui.waste.pull(this.ui.waste.field.length - 1);
                items.forEach(function(e) { e.flipped = false; });
                for (var i=0,l=items.length;i<l;i++){
                    Win.Application.dispatch(function() { 
                        that.ui.deck.add([items.pop()]);
                    });
                }
                Win.Application.dispatch(function() { 
                    that.pushHistory();
                });
            },
            dealCardsToWaste: function () {
                var that = this;
                var count = Math.min(this.ui.deck.field.length - 1, 3);
                for (var i=0;i<count; i++) {
                    Win.Application.setTimeout(function() {
                        var items = that.ui.deck.pull(1);
                        items.forEach(function(e) { e.flipped = true; });
                        that.ui.waste.add(items);
                    }, i * 100);
                }
                Win.Application.setTimeout(function() { 
                    that.pushHistory();
                }, 16 + (count - 1) * 100 );
            },
            forAll: function (f) {
                this.model.tableaus.forEach(function(col) { col.forEach(f); });
                this.model.foundations.forEach(function(col) { col.forEach(f); });
                this.model.waste.forEach(f);
                this.model.deck.forEach(f);
            },
            animateWin: function (complete) {
                if (this.model) {
                    var c = 0;
                    var f = 100;
                    var wait = Win.Application.setTimeout;

                    function rand(min, max) {
                        var range = max - min;
                        return ((Math.random() * range) >> 0) + min;
                    };
                    function a1(card) {
                        wait(function() {
                            if (card.suit) {
                                var c = card._ui;
                                c.bringToFront();
                                c.flipped = true;
                                c.animationDelay = rand(100,500);
                                c.setPositionAngle({ x: rand(0,1366-ui.cardWidth), y: rand(0, 768-ui.cardHeight) }, rand(-180,180));
                            }
                        }, (c++)*f);   
                    };
                    function a2(card) {
                        wait(function() {
                            if (card.suit) {
                                var c = card._ui;
                                c.bringToFront();
                                c.flipped = false;
                                c.animationDelay = rand(100,500);
                                c.setPositionAngle(spacing(0,0), rand(-3,3));
                            }
                        }, (c++)*f);   
                    };

                    this.forAll(a1);
                    this.forAll(a2);
                    if (complete) {
                        c++;
                        wait(complete, (c++)*f);
                    }
                }
            },
            pushHistory: function() {
                this.history.push(Cards.Model.Klondike.stringify(this.model));
            }
        },
        function (initialModel, cardUIFactory, uihost, complete) {
            var that = this;
            this.model = initialModel || k.createGame();
            this.uihost = uihost;
            this.startTime = new Date();
            this.history = [];

            while (uihost.firstChild) { uihost.removeChild(uihost.firstChild); }

            k.updateLocations(this.model);

            this.model.onchange = function () {
                var all = true;
                that.model.foundations.forEach(function (col) { all = all && col.length == 14; });
                if (all) {
                    that.animateWin(newGame);
                }
            };

            this.ui = {};
            this.cardUIFactory = cardUIFactory;

            this.ui.deck = new ui.Stack(this.model, "deck");
            this.ui.deck.layout = this.ui.deck.square;
            this.ui.deck.position = spacing(0, 0);

            this.ui.waste = new ui.Stack(this.model, "waste");
            this.ui.waste.layout = this.ui.waste.spreadRight;
            this.ui.waste.position = spacing(1, 0);
    
            this.ui.foundations = [];
            for (var i=0;i<this.model.foundations.length; i++) {
                var f = this.ui.foundations[i] = new ui.Stack(this.model, "foundations", i);
                f.layout = f.square;
                f.position = spacing(i+3, 0);
            }
    
            this.ui.tableaus = [];
            for (var i=0;i<this.model.tableaus.length; i++) {
                var t = this.ui.tableaus[i] = new ui.Stack(this.model, "tableaus", i);
                t.layout = t.spreadDown;
                t.position = spacing(i, 1);
            }

            this.forAll(function(e) { cardUIFactory(that, e); });
            this.layoutGame(true);

            this.forAll(function (e) { e._ui.animationDelay = 320; });

            var c = function() { 
                that.pushHistory();
                that.forAll(function (e) { e._ui.animationDelay = 160; });
                if (complete) { complete(); }
            };
            Win.Application.dispatch(function() {
                if (!initialModel) {
                    k.deal(that.model, 
                        function(moved) { 
                            if (moved) {
                                var c = moved._ui;
                                that.uihost.removeChild(c.element); 
                                that.uihost.appendChild(c.element); 
                                c._updateElement();
                            }

                            that.layoutGame(); 
                        },
                        c
                    );
                }
                else {
                    that.layoutGame();
                    c();
                }
            });
        }
    );
})();
