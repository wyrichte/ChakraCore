/// <reference path="js/base.js" />
/// <reference path="js/ui.js" /> 
/// <reference path="js/wwaapp.js" />
/// <reference path="js/xhr.js" /> 
/// <reference path="js/win8ui.js" />
/// <reference path="cards.js" />
/// <reference path="cardsui.js" />
/// <reference winrt="true" />

var k = Cards.Model.Klondike;
var ui = Cards.UI.Klondike;
var u = Win.Utilities;

var app = Win.Application.connect();

app.addEventListener("running", function() {
    var commands = id("commands");
    var lastCardOnDown;
    var dragState;
    var ignoreNextUp;

    function id(i) { return document.getElementById(i); }

    window.transformHack = { x: 0, factor: 1.0 };

    function halt(e) {
        e.cancelBubble = true;
        e.stopPropagation();
        return false;
    }

    function handleResize(e) {
        var dpiFactor = (screen.deviceXDPI||96) / 96;

        var gameHost = id("gameHost");
        var svg = id("svg");
        var width = gameHost.offsetWidth;
        var wfactor = width / 1266;
        var hfactor = gameHost.offsetHeight / 748;
        var factor = Math.max(.5, Math.min(wfactor, hfactor)); // UNDONE: min should be ~.9 or 1.0 for RTM
        var translateX = Math.max(0, ((width - (1366 * factor)) / 2) * dpiFactor);

        transformHack = { x: translateX, factor: factor };
        svg.style["-ms-transform-origin"] = "top left";
        svg.style["-ms-transform"] = "translate(" + translateX + "px,0px) scale(" + factor + ")";
    };
    window.newGame = function newGame(initialModel, complete) {
        var gameSvg = document.getElementById("gameSvg");
        while (gameSvg.firstChild) { gameSvg.removeChild(gameSvg.firstChild); }
        var f = svg("g");
        f.setAttribute("width", "1266");
        f.setAttribute("height", "748");

        /*
        var dbg = svg("rect");
        dbg.setAttribute("fill", "orange");
        dbg.setAttribute("width", "1266");
        dbg.setAttribute("height", "748");
        gameSvg.appendChild(dbg);
        */

        gameSvg.appendChild(f);
        window._inst = new Game(initialModel, justModel, f, function() {
            if (complete) { complete(window._inst); }
        });
    };
    Win.Application.setInterval(function() {
        var text = "0:0";
        if (window._inst) {
            var secs = (new Date() - window._inst.startTime) / 1000;
            var minutes = (secs / 60) >> 0;
            secs -= minutes * 60;
            secs = (secs >> 0);
            if (secs < 10) { secs = "0" + secs; }
            text = minutes + ":" + secs;
        }
        id("gameTimer").innerText = text + "  dbg:" + _inst.history.length;
    }, 320);

    window.addEventListener("resize", handleResize, false);
    handleResize();

    id("newButton").addEventListener("click", function(e) {
        newGame(); 
    }, false);
    id("fakeWin").addEventListener("click", function(e) {
        _inst.animateWin(newGame); 
    }, false);
    id("autoGameButton").addEventListener("click", function(e) {
        autoGame(window._inst);
    }, false);
    id("autoTurnButton").addEventListener("click", function(e) {
        autoTurn(window._inst);
    }, false);
    id("autoForeverButton").addEventListener("click", function(e) {
        autoForever();
    }, false);

    id("saveButton").addEventListener("click", function(e) { 
        id("currentState").value = Cards.Model.Klondike.stringify(_inst.model);
        if (window.Windows && Windows.Storage && Windows.Storage.SavePicker) {
            var picker = new Windows.Storage.SavePicker();
            var item = picker.PickOrCreateItem();
            writeFile(item, id("currentState").value, function() { });
        }
    }, false);

    function writeFile(item, body, complete) {
        try {
            var streamOperation = item.GetStreamAsync(Windows.Storage.FileAccessMode.ReadWrite);
            streamOperation.Completed = function () {
                try {
                    var byteSeeker = streamOperation.GetResults();
                    streamOperation.Close();
                    var byteWriter = byteSeeker.GetWriterAt(0);
                    var binaryWriter = new Windows.Storage.BasicBinaryReaderWriter();
                    var writeOperation = binaryWriter.WriteBinaryStringAsync(byteWriter, body);
                    writeOperation.Completed = function () {
                        try {
                            writeOperation.Close();
                            var flushOperation = byteWriter.FlushAsync();
                            flushOperation.Completed = function () {
                                flushOperation.Close();
                                complete();
                            }
                            flushOperation.Start();
                        } catch (e) {
                            Log.Exception(e);
                        }
                    };
                    writeOperation.Start();
                } catch (e) {
                    Log.Exception(e);
                }
            };
            streamOperation.Start();
        } 
        catch (e) {
            Log.Exception(e);
        }
    }


    function readFile(item, complete) {
        var streamOperation = item.GetStreamAsync(Windows.Storage.FileAccessMode.Read);
        streamOperation.Completed = function() {
            var byteSeeker = streamOperation.GetResults();
            var byteReader = byteSeeker.GetReaderAt(0);
            var binaryReader = new Windows.Storage.BasicBinaryReaderWriter();
            var readOperation = binaryReader.ReadBinaryStringAsync(byteReader, byteSeeker.Size);
            readOperation.Completed = function() {
                var binaryText = readOperation.GetResults();
                complete(binaryText);
            };
            readOperation.Start();
        };
        streamOperation.Start();
    }

    id("loadButton").addEventListener("click", function(e) { 
        if (window.Windows && Windows.Storage && Windows.Storage.OpenPicker) {
            var picker = new Windows.Storage.OpenPicker();
            var item = picker.PickSingleItem();

            readFile(item, function(str) {
                newGame(Cards.Model.Klondike.parse(str));
            });
        }
        else {
            newGame(Cards.Model.Klondike.parse(id("currentState").value));
        }
    }, false);

    id("undoButton").addEventListener("click", function(e) { 
        var hist = _inst.history;
        if (hist.length > 1) {
            hist.pop();
            var last = hist[hist.length - 1];
            newGame(Cards.Model.Klondike.parse(last), function() {
                _inst.history = hist;
            });
        }
    }, false);

    var commandDragStart;
    var commandLastAdjust;
    var commandAdjust;
    u.trackDragMove(id("commands"), 
        function (e) {
            if (e.target === id("commands") || e.target == id("primary") || e.target == id("secondary")) {
                commandDragStart = e.pageX;
            }
        },
        function (e) {
            if (commandDragStart) {
                commandAdjust = Math.max(0, Math.min(90, (commandLastAdjust||0) + (e.pageX - commandDragStart)));
                id("commands").style["-ms-transform"] = "translate(" + commandAdjust + "px, 0px)";
            }
        },
        function (e) {
            commandLastAdjust = commandAdjust;
            commandDragStart = null;
        }
    );

    function dragMouseMove(e) {
        continueDrag(e);
        return halt(e);
    }
    function dragMouseUp(e) {
        completeDrag(e);
        window.removeEventListener("mousemove", dragMouseMove, true);
        window.removeEventListener("mouseup", dragMouseUp, true);
        return halt(e);
    }
    function startDrag(card, e) {
        window.addEventListener("mousemove", dragMouseMove, true);
        window.addEventListener("mouseup", dragMouseUp, true);
        dragState = { card: card, initialEvent: e };
        dragState.card.bringToFront();
        var dragLoc = dragState.card.model._location;
        if (dragLoc.field == "tableaus") {
            var dragStack = k.stack(_inst.ui, dragLoc);
            for (var i = dragLoc.position + 1; i < dragStack.field.length; i++) {
                dragStack.field[i]._ui.bringToFront();
            }
        }
    }

    function targetAt(points) {
        var result = null;
        points.forEach(function (pt) {
            if (result) { return; }
            var hit = document.elementFromPoint(pt[0], pt[1]);
            if (hit) {
                var hitCard;
                do {
                    hitCard = u.getData(hit, "declControl");
                    if (hitCard) {
                        result = hitCard;
                        return;
                    }
                    hit = hit.parentNode;
                }
                while (!hitCard && hit);
            }
        });
        return result;
    };

    function tryAutoPlay(card, cardStack) {
        var found = false;
        _inst.model.foundations.forEach(function(stack, index) {
            var cardLoc = card.model._location;
            var fromLoc = k.top(k.stack(_inst.model, cardLoc))._location;
            if (!found && _inst.move(fromLoc, { field: "foundations", index: index })) {
                found = true;
            }
        });
        return found;
    }
    function flipCard(card) {
        card.flipped = true;
        _inst.pushHistory();
    }
    function forEachDraggingUI(f) {
        f(dragState.card);
        var dragLoc = dragState.card.model._location;

        if (dragLoc.field == "tableaus") {
            var dragStack = k.stack(_inst.ui, dragLoc);
            for (var i = dragLoc.position + 1, l=dragStack.field.length; i<l; i++) {
                f(dragStack.field[i]._ui);
            }
        }
    };

    function continueDrag(e) {
        var gameHost = id("gameHost");
        
        // what is the right distance for double click for touch?
        //
        var dist = Math.sqrt(Math.pow(e.pageX - dragState.initialEvent.pageX, 2) + Math.pow(e.pageY - dragState.initialEvent.pageY, 2));
        if (dist > 10) {
            lastCardOnDown = null;
        }

        var x = e.pageX - gameHost.offsetLeft - dragState.initialEvent.offsetX - transformHack.x;
        var y = e.pageY - gameHost.offsetTop - dragState.initialEvent.offsetY;
        x /= transformHack.factor;
        y /= transformHack.factor;

        forEachDraggingUI(function(card) {
            card.setPositionAngle({ x: x, y: y }, card.angle, false);
            y += 10;
        });

        dragState.lastEvent = e;
    }
    function completeDrag(e) {
        var target;
        forEachDraggingUI(function(card) {
            u.addClass(card.element, "hidden");
        });
        try {
            var left = e.pageX - dragState.initialEvent.offsetX + ui.cardWidth / 4;
            var top = e.pageY - dragState.initialEvent.offsetY + ui.cardHeight / 4;
            var right = left + ui.cardWidth * 3 / 4;
            var bottom = top + ui.cardHeight * 3 / 4;
            target = targetAt([[e.pageX, e.pageY], [left, top], [right, top], [right, bottom], [left, bottom]]);
        }
        finally {
            forEachDraggingUI(function(card) {
                u.removeClass(card.element, "hidden");
            });
        }

        var fromCard = dragState.card;
        var fromLoc = fromCard.model._location;

        var toCard = target;

        if (toCard && fromCard) { 
            var toLoc = toCard.model._location;

            if (!_inst.move(fromLoc, toLoc)) {
                k.stack(_inst.ui, fromLoc).layout();
            }
        }
        else {
            k.stack(_inst.ui, fromLoc).layout();
        }

        dragState = null;
    }
    function handleMouseMove(e) {
        var card = u.getData(e.currentTarget, "declControl");
        if (dragState) { 
            continueDrag(e);
        }
        return halt(e);
    };
    function handleMouseDown(e) {
        var card = u.getData(e.currentTarget, "declControl");
        var model = card.model;
        var cardLoc = model._location;

        if (model === lastCardOnDown) {
            var cardStack = k.stack(_inst.model, cardLoc);
            if ((cardLoc.field == "waste")
                || (cardLoc.field == "tableaus" 
                    && cardLoc.position == cardStack.length -1 
                    && card.flipped)) {

                ignoreNextUp = true;
                if (tryAutoPlay(cardStack[cardStack.length - 1]._ui, cardStack)) {
                    return halt(e);
                }
            }
        }
        lastCardOnDown = model;
                    
        if (cardLoc.field == "deck") {
            if (_inst.ui.deck.field.length == 1) {
                _inst.stackDeckFromWaste();
            }
            else {
                _inst.dealCardsToWaste();
            }
        }

        var cardStack = k.stack(_inst.model, cardLoc);

        if (e.button == 2 
            && (
                (cardLoc.field == "waste")
                || (cardLoc.field == "tableaus" 
                    && cardLoc.position == cardStack.length -1 
                    && card.flipped)
            )) {
            tryAutoPlay(cardStack[cardStack.length - 1]._ui, cardStack);
        }
        else if (cardLoc.field == "tableaus" 
            && cardLoc.position == cardStack.length -1
            && !card.flipped
            && e.button == 0) {
            flipCard(card);
        }
        else if (cardLoc.field == "tableaus"
            && model.suit
            && model.flipped
            && e.button == 0) {

            startDrag(card, e);
        }
        else if (cardLoc.field == "waste" && model.suit && e.button == 0) {
            var w = _inst.ui.waste.field;
            startDrag(w[w.length - 1]._ui, e);
        }

        return halt(e);
    };
    function handleMouseUp(e) {
        if (ignoreNextUp) {
            ignoreNextUp = false;
            return halt(e);
        }
        var card = u.getData(e.currentTarget, "declControl");
        var cardLoc = card.model._location;
        if (dragState) { 
            completeDrag(e);
        }
        return halt(e);
    };
    function handleTouchMove(e) {
        return handleMouseMove(e);
    };
    function handleTouchDown(e) {
        return handleMouseDown(e);
    };
    function handleTouchUp(e) {
        return handleMouseUp(e);
    };
    function svg(tagname) {
        return document.createElementNS('http://www.w3.org/2000/svg', tagname);
    };

    function createCard(game, model) {
        var cardContainer = svg("g");
        game.uihost.appendChild(cardContainer);
        var card = new ui.Card(cardContainer, model);
        u.setData(cardContainer, "declControl", card);
        Object.defineProperty(model, "_ui", { value: card, enumerable: false });
        return cardContainer;
    };
        
    function justModel(game, model) { 
        var c = createCard(game, model);
        c.addEventListener("mousemove", handleMouseMove, true);
        c.addEventListener("mousedown", handleMouseDown, true);
        c.addEventListener("mouseup", handleMouseUp, true);
        c.addEventListener("MSTouchMove", handleTouchMove, true);
        c.addEventListener("MSTouchDown", handleTouchDown, true);
        c.addEventListener("MSTouchUp", handleTouchUp, true);
    };

    Win.Controls.processAll();

    if (window.runTests) { runTests(); }

    newGame();
});

app.run();
