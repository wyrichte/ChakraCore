//
// Copyright (C) Microsoft. All rights reserved.
//

/**
  Constructor for Operand type.
  @param opnd The operand taken from the child ScriptContext.
*/
function Operand(opnd, metadata) {
  this._opnd = opnd;
  this._htmlMode = false;

  this._metadata = metadata;
  this.RegValues = metadata.regNames;

  /**
    See Js::OpndKind from Opnd.h.
    Ensure that this Enum is consistent with the definitions found there.
  */
  this.OpndKind = new Enum([
    'OpndKindInvalid',
    'OpndKindIntConst',
    'OpndKindFloatConst',
    'OpndKindHelperCall',
    'OpndKindSym',
    'OpndKindReg',
    'OpndKindAddr',
    'OpndKindIndir',
    'OpndKindLabel',
    'OpndKindMemRef',
    'OpndKindRegBV'
  ]);
  // for improved portability, see Opnd.h:24 (OpndKind)
  // add that info to metadata component of object from backend,
  // and pass an enum value back and forth for round trip

  this.DumpInt = function() {
    if (this._htmlMode) {
      Output.WriteZeroLength('<span class="integer" title="integer">');
      Output.Write("{0}".format(""+this._opnd.value));
      Output.WriteZeroLength('</span>');
    } else {
      Output.Write("${0} (int)".format(""+this._opnd.value));
    }
  };

  this.DumpFloat = function() {
    if (this._htmlMode) {
      Output.WriteZeroLength('<span class="float" title="float">');
      Output.Write("{0}".format(""+this._opnd.value));
      Output.WriteZeroLength('</span>');
    } else {
      Output.Write("{0}f (float)".format(""+this._opnd.value));
    }
  };

  this.DumpHelperCall = function() {
    // this can be modified to create an interactive element for _htmlmode
    Output.Write("{0}".format(this._opnd.methodName));
  };

  this.DumpSym = function() {
    if (this._htmlMode) {
      Output.WriteZeroLength('<span class="ir-sym" name="s{0}" title="Argument">'.format(this._opnd.symid));
    }

    Output.Write("{s{0}}".format(this._opnd.symid, this._opnd.kind));

    if (this._htmlMode) {
      Output.WriteZeroLength('</span>');
    }
  };

  this.DumpReg = function() {
    if (this._htmlMode) {
      Output.WriteZeroLength('<span class="ir-reg" name="s{0}" title="Reg Symbol">'.format(this._opnd.symid));
    }
    Output.Write("s{0}".format(this._opnd.symid));
    if (this._htmlMode) {
      Output.WriteZeroLength('</span>');
    }

    if (this._opnd.regid) {
      if (this._htmlMode) {
        Output.WriteZeroLength('<span class="ir-register" name="register-{0}" title="Register">'
          .format(this.RegValues[this._opnd.regid]));
      }

      Output.Write("({0})".format(this.RegValues[this._opnd.regid]));

      if (this._htmlMode) {
        Output.WriteZeroLength('</span>');
      }
    }
  };

  this.DumpAddr = function() {
    var address = this._opnd.addr.toString(16);
    address = address.toUpperCase();
    while(address.length < 8)
    {
      address = "0" + address;
    }

    var detail = this._opnd.detail;
    detail = detail.replace(/(0x[\d\w]*)/, '');
    var text = "0x{0}{1}".format(address, detail);
    if (Output._htmlSpaces) {
      text = _.escape(text);
      text = text.replace(/(0x[\d\w]*)/, '<span class=\'address\' title=\'address\'>$1</span>');
      text = text.replace(/(("|&quot;).*\2)/, '<span class=\'string\' title=\'string\'>$1</span>');
      Output.Write(text);
    } else {
      Output.Write(text);
    }
  };

  this.DumpIndir = function() {
    var base = this._opnd.base;
    var index = this._opnd.index;
    var offset = this._opnd.offset;
    Output.Write("[");

    if (this._htmlMode) {
      Output.WriteZeroLength('<span class="ir-reg" name="s{0}" title="Reg Symbol">'.format(base));
      Output.Write("s{0}".format(base));
      Output.WriteZeroLength('</span>');

      if (index) {
        Output.Write(" + ");
        Output.WriteZeroLength('<span class="ir-reg" name="s{0}" title="Reg Symbol">'.format(index));
        Output.Write("s{0}".format(index));
        Output.WriteZeroLength('</span>');
      }
      if (offset) {
        Output.Write(" + ");
        Output.WriteZeroLength('<span class="integer" title="integer">'.format(index));
        Output.Write("{0}".format(offset));
        Output.WriteZeroLength('</span>');
      }
    } else {
      Output.Write("s{0}".format(base));
      if (index) {
        Output.Write(" + s{0}".format(index));
      }
      if (offset) {
        Output.Write(" + {0}".format(offset));
      }
    }

    Output.Write("]");
  };

  this.DumpDefault = function() {
    Output.Write("[{0}]".format(this._opnd.kind));
  };

  this.Dump = function(displayTags) {
    var kind = this._opnd.kind;
    this._htmlMode = displayTags;

    if (kind == this.OpndKind.OpndKindIntConst)
    {
      this.DumpInt();
    }
    else if (kind == this.OpndKind.OpndKindFloatConst)
    {
      this.DumpFloat();
    }
    else if (kind == this.OpndKind.OpndKindHelperCall)
    {
      this.DumpHelperCall();
    }
    else if (kind == this.OpndKind.OpndKindSym)
    {
      this.DumpSym();
    }
    else if (kind == this.OpndKind.OpndKindReg)
    {
      this.DumpReg();
    }
    else if (kind == this.OpndKind.OpndKindAddr)
    {
      this.DumpAddr();
    }
    else if (kind == this.OpndKind.OpndKindIndir)
    {
      this.DumpIndir();
    }
    else
    {
      this.DumpDefault();
    }
  };
}
