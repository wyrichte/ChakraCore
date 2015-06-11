/// <reference path="js/base.js" />
/// <reference path="js/ui.js" /> 
/// <reference path="js/wwaapp.js" />
/// <reference path="js/xhr.js" /> 
/// <reference path="js/win8ui.js" />
/// <reference path="cards.js" />

(function (global) {
    var u = Win.Utilities;
    var k = Cards.Model.Klondike;

    // B8 cards: 63mm x  88mm
    var cardWidth = 63 * 1.8;
    var cardHeight = 88 * 1.8;

    function animPos(element, startAngle, endAngle, startPosition, endPosition, duration, complete) {
        var animation = {
            startTime: Win.Application.totalTime,
            duration: duration,
            invoke: function (percent) {
                var x = startPosition.x + percent * (endPosition.x - startPosition.x);
                var y = startPosition.y + percent * (endPosition.y - startPosition.y);
                var angle = startAngle + percent * (endAngle - startAngle);
                element.setAttribute("transform", "translate(" + x + "," + y + ") rotate(" + angle + ")");
                if (percent == 1.0 && complete) {
                    complete();
                }
            }
        };
        animations.push(animation);
        return animation;
    };
    
    function rand(min, max) {
        var range = max - min;
        return ((Math.random() * range) >> 0) + min;
    };

    function svg(tagname) {
        return document.createElementNS('http://www.w3.org/2000/svg', tagname);
    };

    function makeArray(arrayLike) {
        var arr = [];
        for (var i=0,l=arrayLike.length; i<l; i++) {
            arr.push(arrayLike[i]);
        }
        return arr;
    };
    function renderOrderSort(a,b) {
        var ao = new Number(a.getAttribute("render-order"));
        var bo = new Number(b.getAttribute("render-order"));
        if (ao < bo) { 
            return -1; 
        }
        else if (ao > bo) { 
            return 1; 
        }
        else { 
            return 0; 
        }
    };

    Win.Namespace.define("Cards.UI.Klondike", {
        cardWidth: cardWidth,
        cardHeight: cardHeight,

        updateRenderOrder: function (element) {
            var arr = makeArray(element.childNodes).
                filter(function (e) { return e.nodeType == 1; });

            var needed = false;
            var last = -1;
            arr.forEach(function (e) { 
                if (needed) { return; }
                var r = new Number(e.getAttribute("render-order"));
                if (r < last) { needed = true; }
                last = r;
            });

            if (needed) {
                while (element.firstChild) { element.removeChild(element.firstChild); }

                var index = 0;
                arr.sort(renderOrderSort).forEach(function (e) {
                    e.setAttribute("render-order", index++);
                    element.appendChild(e);
                });
            }
        },

        Stack: Win.Class.define(null, {
                position: { x: 0, y: 0 },
                pull: function(top) {
                    var items = this.field.slice(-top);
                    this.field = this.field.slice(0, this.field.length - top);
                    return items;
                },
                add: function(items) {
                    var originalLength = this.field.length;
                    this.field = this.field.concat(items);
                    var host = this.field[0]._ui.host;
                    items.forEach(function(item) { 
                        host.removeChild(item._ui.element); 
                        host.appendChild(item._ui.element);
                    });
                    this.layout(originalLength, true);
                    return this;
                },
                spreadDown: function(skip, updateAngles) {
                    var that = this;
                    var y = this.position.y;
                    this.field.forEach(function (card, pos) {
                        if (pos >= (skip || 0)) {
                            card._ui.setPositionAngle({x:that.position.x, y:y}, updateAngles ? rand(-3, 3): card._ui.angle);
                        }
                        if (card.suit) {
                            y += 5;
                            if (card.flipped) {
                                y += Cards.UI.Klondike.cardHeight/7;
                            }
                        }
                    });
                    return this;
                },
                spreadRight: function(skip, updateAngles) {
                    var that = this;
                    var x = that.position.x;
                    var l = this.field.length;
                    this.field.forEach(function (card, pos) {
                        if (pos >= (skip || 0)) {
                            card._ui.setPositionAngle({x:x, y:that.position.y}, updateAngles ? rand(-3, 3): card._ui.angle);
                        }
                        if (pos % 3 == 0) {
                            x = that.position.x;
                        }
                        else {
                            x += 15;
                        }
                    });
                    return this;
                },
                square: function(skip, updateAngles) {
                    var final = this.position;
                    this.field.forEach(function (card, pos) {
                        card._ui.setPositionAngle({x:final.x, y:final.y}, (pos > (skip||0) && updateAngles) ? rand(-3, 3): card._ui.angle);
                    });
                    return this;
                },
                updateRenderOrder: function() {
                    var initial = this.field[0]._ui.renderOrder || 0;
                    var last = 0;
                    this.field.forEach(function (card) {
                        if (!card.suit) { card._ui.renderOrder = 0; }
                        if (card._ui.renderOrder < last) {
                            card._ui.renderOrder = last + 1;
                            last++;
                        }
                    });
                    return this;
                }
            },
            function(game, field, index) {
                function done() { 
                    k.updateLocations(game);
                    if (game.onchange) { game.onchange(); }
                };

                if (index !== undefined) {
                    Object.defineProperty(this, "field", {
                        get: function() { return game[field][index]; },
                        set: function(value) { 
                            game[field][index] = value; 
                            done();
                        }
                    });
                }
                else {
                    Object.defineProperty(this, "field", {
                        get: function() { return game[field]; },
                        set: function(value) { 
                            game[field] = value; 
                            done();
                        }
                    });
                }
                this.position = { x: 0, y: 0 };
                this.layout = this.spreadDown;
            }
        ),

        Card: Win.Class.define(null, {
                bringToFront: function() {
                    this.host.removeChild(this.element);
                    this.host.appendChild(this.element);
                },
                renderOrder: { 
                    get: function() { return new Number(this.element.getAttribute("render-order")); },
                    set: function(value) { 
                        this.element.setAttribute("render-order", value);
                    }
                },
                position: {
                    get: function() { return { x: this._position.x, y: this._position.y }; },
                    set: function(value) { 
                        this.setPositionAngle(value, this.angle);
                    }
                },
                angle: {
                    get: function() { return this._angle; },
                    set: function(value) { 
                        this.setPositionAngle(this.position, value);
                    }
                },
                setPositionAngle: function (position, angle, skipAnimation) {
                    var needUpdate = false;
                    if (this._angle != angle) {
                        this._lastAngle = this.angle;
                        this._angle = angle;
                        needUpdate = true;
                    }
                    var p = this._position;
                    if (p.x != position.x || p.y != position.y) {
                        this._lastPosition = this.position;
                        p.x = position.x;
                        p.y = position.y; 
                        needUpdate = true;
                    }
                    if (needUpdate) {
                        this._updateElement(skipAnimation); 
                    }
                },
                flipped: { 
                    get: function() { return u.hasClass(this.element, "flipped"); },
                    set: function(value) { 
                        if (value) { 
                            u.addClass(this.element, "flipped");
                        }
                        else {
                            u.removeClass(this.element, "flipped");
                        }
                        this.model.flipped = value;
                    }
                },
                animationDelay: 0,
                _position: null,
                _angle: 0,
                _updateElement: function(skipAnimation) {
                    if (this.model.flipped !== this.flipped) {
                        if (this.model.flipped) { 
                            u.addClass(this.element, "flipped");
                        }
                        else {
                            u.removeClass(this.element, "flipped");
                        }
                    }

                    if (this._pendingAnimation) { this._pendingAnimation.complete = true; }
                    if (skipAnimation) {
                        this.element.setAttribute("transform", "translate(" + this._position.x + "," + this._position.y + ") rotate(" + this._angle + ")");
                        this._lastPosition = this._position; 
                        this._lastAngle = this.angle; 
                        delete this._pendingAnimation; 
                    }
                    else {
                        var that = this;
                        this._pendingAnimation = animPos(
                            this.element,
                            this._lastAngle || 0,
                            this._angle,
                            this._lastPosition,
                            this._position,
                            this.animationDelay,
                            function() {
                                that._lastPosition = that._position; 
                                that._lastAngle = that.angle; 
                                delete that._pendingAnimation; 
                            }
                        );
                    }
                },
                model: null,
                host: null,
                element: null
            }, 
            function(element, options) {
                this._position = { x: 0, y: 0 };
                this.element = element;
                this.host = element.parentNode;
                this.model = options;
                this._angle = rand(-3,3);
                if (!this.model.suit) { this._angle = 0; }
                u.addClass(element, "card-container");

                var face = svg("g");
                face.className.baseVal = "card-face";
                element.appendChild(face);

                var back = svg("g");
                back.className.baseVal = "card-back";
                element.appendChild(back);
            
                var xl = "http://www.w3.org/1999/xlink";
                var backUx = svg("image");
                backUx.className.baseVal = "card-back-ux";
                backUx.width.baseVal.value = cardWidth;
                backUx.height.baseVal.value = cardHeight;
                backUx.setAttributeNS(xl, "href", "images/cards-53.svg");
                back.appendChild(backUx);
            
                var faceUx = svg("image");
                faceUx.className.baseVal = "card-ux";
                faceUx.width.baseVal.value = cardWidth;
                faceUx.height.baseVal.value = cardHeight;
                faceUx.setAttributeNS(xl, "href", "images/" + Cards.Model.imageName(options) + ".svg");
                face.appendChild(faceUx);
                
                this._updateElement();
            }
        )
    });
})(this);