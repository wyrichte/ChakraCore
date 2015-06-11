/*********************************************************************************/
/*                                 xml.js                                        */
/*********************************************************************************/

/* Some XML related utilities

    Yes, I know that there are 'real' XML parsers out there (eg MSXML),
    however these parsers were designed to do all the proper syntax checking 
    and create their own tree representation of the XML which you either use 
    their APIs to get at, or you have to transform again.

    What I need is something that makes it really easy to use XML from JScript.
    Since both XML and JScript can represent nested structures there is a 'natural' 
    mapping where JScript fields coorespond to XML entities (tags).   
    
    I want this mapping to have the following characteristics

    1) I want this it be a viable serializaion format.  Thus arbitrary JScript
       objects should be able to be written as XML.  The XML I write needs to
       be well formed XML in the sense that it follows all the XML rules.

        1) I need to encode key (field) names that don't coorespond to valid 
           XML identifiers.
 
        2) I need to be able to handle arbitrary graphs of JScript objects 
           (not just trees).  

    2) I want to be able to handle most if not all 3rd party generate XML.  
       This required support for the special handling of a field called 
       'body' in a JScript object.  This field becomes the body of the 
       XML entity (that is text in the entity that is not a sub-element)

       TODO: handle <[CDATA[]]>

    3) I want the mapping to be as 'natural' as possible.  This means 

        1) The encoding of JScript names to XML must be trival (identity), in
           all common cases (normal identifiers). 

        2) Typically XML encodes lists by repeating a element, JScript encodes
           these arrays (objects with numbers as field names).  The reader
           and writer need to bridge this gap.  

    4) NON-GOAL - to allow generation of arbitrary XML.  Currently xmlWrite
       dumps all primitive types as XML Attributes and all other JScript objects
       as elements.  Allowing control over this seems of low value.  

   ---------------------------------------------------------------------------
    Therer really are only two main functions provided by this module

        var obj = xmlRead(fileName)    // Read xml in 'fileName' create a object 
                                    // representing it.  The returned object

        xmlWrite(obj, fileName, tag)    // write an object as XML

    XML tends to blur the distinction between the type of the element and 
    the name of the element in the enclosing element.  Thus in the example
    below CONTACT is the type of the element (an element of three fields,
    NAME, ADDRESS, PHONE), but it is also the name of the field within the
    CONTACTS element.   In the mapping we use, field names are mapped to
    element names.  Thus XML objects 'know' the name of the field they are
    embedded in.   
    
    This  mismatch shows up in the APIs.  xmlWrite() takes a 'tag' parameter 
    which is the name of XML enitity.  At the top most level this tag is not
    usually interesting, so by default xmlWrite() has a default of 'object'.
   
    EXAMPLE

    var myContacts = { 
        CONTACTS: {
            ID : "4",
            CONTACT : [
                {   NAME: "MyFriend",
                    ADDRESS: "234 120th Ave",
                    PHONE: "343-3343"
                },
                {   NAME: "MyBestFriend",
                    ADDRESS: "4 300th Ave",
                    PHONE: "988-3453"
                }
            ]
        }
    };
    xmlWrite(contacts, "test.xml", "object");
        
    ----------------------------------------------------------------------
    gets encoded as the following XML 

    <?xml version='1.0' ?>
    <object>
        <CONTACTS
            ID='4'>
            <CONTACT
                NAME='MyFriend'
                ADDRESS='234 120th Ave'
                PHONE='343-3343'/>
            <CONTACT
                NAME='MyBestFriend'
                ADDRESS='4 300th Ave'
                PHONE='988-3453'/>
        </CONTACTS>
    </object>

   ---------------------------------------------------------------------------

    To read this XML object back into JScript you can do 

        var myContactAgain = xmlRead("test.xml).object;
        WScript.Echo(myContacts.CONTACTS.CONTACT[0].NAME);

    The need to fetch '.object' from the return of xmlRead is a bit of a pain,
    but it makes sense after some thought.  While it is true that XML files
    should only contain one top level element, if we returned this as a JScript
    object we would loose information (the name of the top most tag).  


    If you find this a pain, there are two wrapper functions 'xmlSerialize' and
    'xmlDeserialize' that hide this 

    xmlSerialize(myContacts, "test.xml");
    var myContactAgain = xmlDeserialize("test.xml");
    WScript.Echo(myContacts.CONTACTS.CONTACT[0].NAME);
    
   ---------------------------------------------------------------------------
    
    The object return by 'xmlRead' has the following properites

        1) Every XML element represents a field name.  Elements fields always
           point to objects or arrays.   Its attributes and sub-elements are
           fields of this object.  Attributes are fields that point at strings
           and sub-elements point at objects or arrays.  

        2) If there are sub-multiple elements with the same element tag, the
            field for that element is represented by an array.  There is an
           abiguity if we trying to represent an array of exactly one element
           in XML.  There is a special XML attribute '_singleton' which is 
           placed on the element of a singleton arrays to disabiguate this case.  
           
        3) Any part of the body of a element that is not a sub-element, is
           concatinated together and made into the 'body' field.  

        4) To support fields of any string value, we encode any characters
           that are not legal XML tag characters using the encoding _\d+_
           where \d+ is the decimal unicode value for the character.  Thus
           we can serialize anything, but 'normal' field names are left 
           unchanged. 

    ----------------------------------------------------------------------------
    One of the goals of this code is to be able to persist arbitray JStript object
    to disk.  For this we need something more powerful than straight XML because
    you might have cirular references in your JScript objects.   

    What I have done is introduce two special XML attributes objID and objRef.  
    You can add objID='tag' to any XML element and it tags that element with
    the given tag.  Then elsewhere you can use refer to it as <fieldName objRef='tag'/>
    to refer to it.  The reader keeps track of these and creats the appropriate
    aliases.   Thus you persist any JScript objectand the resulting XML
    is pretty readable.  

        var x = { field1 : 3,
                  field2 : x,
                }

    Would encode 'x' as 

    <object objID='tag1' field1='3'>
        <field2 objRef='tag1'>
    </object>
        
    When writing fields if the field ends in '_' this indicates that the field 
     is a 'subordinate' field (it will refer to an object that already exists
    and dumping it now makes for poorly readable XML.  Thus it will not dump
    the object at that time but simply put a forward reference (objRef attribute)
    Note that if there is no later reference, the definition will not be dumped
    anywhere so use this feature with care. 
*/

// Date    3/1/2004
// Dependancies
//        log.js            for logging utilities
//        fso.js            for file system access 

/*********************************************************************************/
var xmlModuleDefined = 1;         // Indicate that this module exist

if (!logModuleDefined) throw new Error(1, "Need to include log.js");

/****************************************************************************/
/* serialize the JScript object 'obj' out to the file 'fileName'.  This is
   a trivial wrapper over xmlWrite */

function xmlSerialize(obj, fileName, excludeFields, attributeQuoteCharacter) {
    xmlWrite(obj, fileName, "object", excludeFields, attributeQuoteCharacter);            // I told you it was trivial 
}

/****************************************************************************/
/* Take the serialized object in 'fileName' and bring it back as an object */
function xmlDeserialize(fileName) {

    return xmlRead(fileName).object;
}

/****************************************************************************/
/* read the XML in 'fileName'.  Note that this returns an object which has
   the field whose name is the name of the topmost XML tag in the XML file.
   If 'unicode' is true then the file is assumed to be unicode 
 */ 
function xmlRead(fileName, unicode) {
    var file = FSOOpenTextFile(fileName, 1, false, (unicode ? -1: -2)); // -1 = TriStateTrue, -2 = TriStateDefault 
    var ret = {};
    _xmlReadHelper(file, ret, {}, "");
    return ret;
}

/****************************************************************************/
/* considers the stream which is logically 'line' followed anything read
   from 'file'.  This stream is logically the body (and end tag) of an element.  
   This element should be represented by 'retObj', thus fill in a field of 
   'retObj' with all the child elements found in the body.  'objRefs' is a 
   table tha maps object IDs to previously created objects (so arbitrary 
   graphs of JScript objects can be created.   'closeTag' is the tag that 
   ends the element (only used for validation).  Finally the funtion returns
   any parts of the line that have not been consumed yet.  Any text that
   is not a sub-element' is concatintated and placed in the 'body' field
*/

function _xmlReadHelper(file, retObj, objRefs, line, closeTag) {

    // logMsg(LogScript, LogInfo, "_xmlReadHelper for body of tag: ", closeTag, " line='", line, "' {\n");
    if (line.length == 0 && !file.AtEndOfStream) 
        line = file.ReadLine();

    for(;;) {
redo:
        if (line.match(/^\s*<\s*([\w_.:-]+)\s*(?:([^>]*?)(>|(\/>)))?\s*/)) {
            var tag = RegExp.$1;
            var attrs = RegExp.$2;
            var term = RegExp.$3;

            if (term) {                // If we have read the > for the tag we can process it (otherwise read some more)
                var quickTerm = (term == '\/>');    // we used the quick /> termination
                line = RegExp.rightContext;
                // logMsg(LogScript, LogInfo, "Matched tag: ", tag, " term='", term, "'\n");

                var newObj = _xmlReadAttrs(attrs, objRefs, quickTerm);

                    // special case, <tag dateString='XXX XXX XX XX:XX:XX XXX XXXX'/>
                if (quickTerm && newObj.dateString && keys(newObj).length == 1)
                    newObj = new Date(newObj.dateString);

                var objName = tag;            // decode the tag 
                if (objName.match(/_\d*_/))
                    objName  = _xmlDecodeTag(objName);
        
                var curObj = retObj[objName];        // Create a list if this entity occurs multiple times
                if (curObj) {
                    if (curObj.length)
                        curObj.push(newObj)
                    else
                        retObj[objName] = [curObj, newObj];
                }
                else {
                    if (newObj._singleton)            // special case, if it is marked as '_singleton' it is mean to be an array of that object
                        retObj[objName] = [newObj];
                    else
                        retObj[objName] = newObj;
                }

                if (!quickTerm)
                    line = _xmlReadHelper(file, newObj, objRefs, line, tag);
            }
            else {
                while(!file.AtEndOfStream) {            // This loop is here just for efficiency
                    nextLine = file.ReadLine();
                    line += nextLine;
                    if (nextLine.match(/>/))
                        break redo;
                }
                throw Error(1, "Unterminated tag");

                // logMsg(LogScript, LogInfo, "Matched tag ", tag, " but did not close, reading more...\n");
            }
        }
        else if (line.match(/^\s*<\s*\/\s*([\w_.:-]+)>\s*/)) {                                    // tag ends 
            var tag = RegExp.$1;
            line = RegExp.rightContext;

            //logMsg(LogScript, LogInfo, "Matched close of tag: ", tag, "\n");
            if (tag != closeTag)  
                logMsg(LogScript, LogError, "Found tag ", tag, " expecting ", closeTag, " near line ", file.Line, "\n");
            break;
        }
        else if (line.match(/^(\s*[^\s<][^<]*)/)) {                        // something that is not a tag (normal body)
            var body = RegExp.$1;
            line = RegExp.rightContext;

            if (body.match(/&/)) {
                body = body.replace(/&apos;/g, "'");
                body = body.replace(/&quot;/g, "\"");
                body = body.replace(/&lt;/g, "<");
                body = body.replace(/&gt;/g, ">");
                body = body.replace(/&amp;/g, "&");
            }
            if (!retObj.body)
                retObj.body = body
            else 
                retObj.body += "\r\n" + body
            // logMsg(LogScript, LogInfo, "Matched body: '", retObj.body, "'\n");

            if (line.match(/^\s*<\s*[\/\w]/))
                continue;        // don't read if we already have something that looks good
        }
        else if (line.match(/^(\s*<!--(.|\s)*?-->\s*)|^\s+/)) {                    // comments (or whitespace)
            line = RegExp.rightContext;
        }
        else if (line.match(/^(\s*<!--)/)) {    // begining but not end of comment
            do {
                if (file.AtEndOfStream) 
                    throw Error(1, "Unterminated comment");
                line = file.ReadLine();
            } while(!line.match(/-->/))
            line = RegExp.rightContext;
        }
        else if (line.match(/^\s*<\?xml[^>]*?>/i)) {
            line = RegExp.rightContext;
        }
        else if (line.length != 0 || file.AtEndOfStream) {
            if (line.length != 0)
                logMsg(LogScript, LogError, "Could not parse at '", line.substr(0, 60), "' near line ", file.Line, "\n");
            if (closeTag != undefined)
                logMsg(LogScript, LogWarn, "Tag ", closeTag, " was not closed near line ", file.Line, "\n");
            break;
        }
        if (!file.AtEndOfStream) 
            line += file.ReadLine();
    }
    
    // logMsg(LogScript, LogInfo, "} _xmlReadHelper = '", line, "'\n");
    return line;
}

/****************************************************************************/
/* given a string 'attrs' that represents all the attributes of a XML element
   (text after <tag and before >), create a new object, and add fields that
   coorespond to these XML attributes.   'objRefs' is a table of JScript 
   objects that have been given objRef attributes.  If there is an 'objRef'
   in 'attrs' don't make a new object but reuse the old one.  If an old object
   does not exist, insure that the new object (that is returned by this function
   is also placed in the 'objRefs table).  quickTerm is true if the element
   use the quick termination capability.  
*/
function _xmlReadAttrs(attrs, objRefs, quickTerm) {

    var newObj = {};
    for(;;) {        // This is not really a loop, it is a label, I am using
                    // 'continue' to mean 'startOver here. 

        var curAttrs = attrs;
        var allNums = true;
        while(curAttrs.match(/^\s*([\w:\.-]+)\s*=\s*(('(.*?)')|("(.*?)")|(\S+))\s*/)) {
            anyAttr = true;
            curAttrs = RegExp.rightContext;
            var tag = RegExp.$1;
            var value = RegExp.$4 + RegExp.$6 + RegExp.$7;
            if (value.match(/&/)) {
                value = value.replace(/&apos;/g, "'");
                value = value.replace(/&quot;/g, "\"");
                value = value.replace(/&lt;/g, "<");
                value = value.replace(/&gt;/g, ">");
                value = value.replace(/&amp;/g, "&");
            }

            if (tag.match(/_\d*_/)) {
                tag  = _xmlDecodeTag(tag);
                if (!tag.match(/^\d+$/))
                    allNums = false;
            }
            else 
                allNums = false;

            newObj[tag] = value;
            // logMsg(LogScript, LogInfo, "got ", RegExp.$1, " curAttrs ", curAttrs, "\n");
        }
        if (!curAttrs.match(/^\s*$/)) 
            logMsg(LogScript, LogWarn, "Extra characters '", curAttrs, "' on tag ", tag, "\n");

        if (quickTerm && allNums) {
            var arrObj = [];             // make it an array, not just an object with number fields
            for (var i in newObj) 
                arrObj[i] = newObj[i];
            newObj = arrObj;
        }

        if (newObj.objRef) {
            if (!objRefs[newObj.objRef])
                objRefs[newObj.objRef] = {};
                
            var ret = objRefs[newObj.objRef];
            if (ret._singleton)                    // singletons only apply to the def, not to refs
                ret._singleton = undefined;
            return (ret);
        }
        if (newObj.objID) {
                // insure that our object ID counter is above this
            if (newObj.objID.match(/(\d+)$/)) {
                var id = (RegExp.$1 - 0);
                if (id >= objIds) 
                    objIds = id + 10;
            }
            if (objRefs[newObj.objID]) {
                if (objRefs[newObj.objID] !== newObj) {
                    newObj = objRefs[newObj.objID];
                    continue;
                }
            }
            else 
                objRefs[newObj.objID] = newObj;    
        }
        return newObj;
    }
}

/****************************************************************************/
/* we need the ability to encode arbitrary strings as JScript fields.  
   Because XML only allows alphaNumeric with -, : and . we need to
   encode and decode them into valid XML tags. 
*/
function _xmlDecodeTag(val) {

    if (val.match(/^__/))
        val = RegExp.rightContext;    // this is the special encoding for a number

    var ret = "";
    while (val.match(/_(\d+)_/)) {
        ret += RegExp.leftContext + String.fromCharCode(RegExp.$1);
        val = RegExp.rightContext;
    }
    ret += val;
    return ret;
}

/****************************************************************************/
/* we need the ability to encode arbitrary strings as JScript fields.  
   Because XML only allows alphaNumeric with -, : and . we need to
   encode and decode them into valid XML tags.  The encoding tries to make
   the encoded tag the same in common cases.
*/
function _xmlEncodeTag(val) {

    var ret = "";
    if (val.match(/^([^A-Za-z])/)) {
        if (val.match(/^(\d)/))                // numbers we treat specially to make them pretty 
            ret += "__" + RegExp.$1;
        else if (val.match(/^(_[A-Za-z])/))    // names like _foo does not need special encoding
            ret += RegExp.$1;
        else 
            ret += RegExp.leftContext + "_" + RegExp.$1.charCodeAt(0) + "_";
        val = RegExp.rightContext;
    }
    while (val.match(/([^A-Za-z\d\.-])/)) {
        var nonAlphaChar = RegExp.$1;
        ret += RegExp.leftContext;
        val = RegExp.rightContext;
        if (nonAlphaChar == "_" && !val.match(/^\d/))
            ret += nonAlphaChar;
        else 
            ret += "_" +  nonAlphaChar.charCodeAt(0) + "_";
    }
    ret += val;
    return ret;
}

/****************************************************************************/
/* Write out the JScript object 'data' to the file 'fileName' using the 
   convention described at the top of this file.  (fields become attributes
   (for simple types), or sub-elements (for field of object type).   'tag' 
   is the name of the top most XML element (that cooresponds to 'data').
   if 'excludeFields' is present, this is a SET of fields that should not
   be dumped to the XML (TODO: make it prefix-path based) */

function xmlWrite(data, fileName, tag, excludeFields, attributeQuoteCharacter) {

    if (tag == undefined) 
        tag = "object";
    if (excludeFields == undefined)
        excludeFields = {};
    if (attributeQuoteCharacter == undefined)
        attributeQuoteCharacter = "'";

    var file = FSOOpenTextFile(fileName, 2, true);
    file.WriteLine("<?xml version=" + attributeQuoteCharacter + "1.0" + attributeQuoteCharacter + "?>");
    _xmlWriteHelper(data, file, [], tag, "", excludeFields, attributeQuoteCharacter);
}

/****************************************************************************/
/* given a JScript object 'data', a file to write to 'file', and the table
   that maps from object ID so the objects objIDs, and the XML tag that
   cooresponds to this object (the field that this is a object of), write
   out the XML for this object to the file and update 'objIDs' to include
   the object if 'data' has an 'objId' field.  If the object has already been
   written, then the XML is just a 'element with a 'objRef' field set,
   If 'singleton' is true, the _singleton attribute is written out to insure
   that when it is read in, it is read in as a sington array and not just
   element itself.
*/
function _xmlWriteHelper(data, file, objIDs, tag, indent, excludeFields, attributeQuoteCharacter, singleton) {

    if (attributeQuoteCharacter == undefined)
        attributeQuoteCharacter = "'";

    // logMsg(LogScript, LogInfo, "_xmlWriteHelper ", tag, "\n");

    if (data.length != undefined && typeof(data) == "object" && typeof(data[0]) == "object") {
            // it is an array we simply output all the fields in order
        for(var j=0; j < data.length; j++)
            if (data[j] != undefined) 
                _xmlWriteHelper(data[j], file, objIDs, tag, indent, excludeFields, attributeQuoteCharacter, data.length == 1);
    }
    else {
        file.Write(indent + "<"); file.Write(tag); 
        var objID = data.objID;
        if (singleton) 
            file.Write(" _singleton=" + attributeQuoteCharacter + "true" + attributeQuoteCharacter + " ");

        if (objID != undefined && (objIDs[objID] || tag.match(/_$/))) {    
                // it is an object reference
            file.WriteLine(" objRef=" + attributeQuoteCharacter + objID + attributeQuoteCharacter + " />");
        }
        else {
            if (objID != undefined)
                objIDs[objID] = data;


                // Write out all fields encoded as attributes of the entity
                // These are all fields of primtive (non-object) type. and 
                // the special field called 'body'
            var objFields = [];
            var bodyData = undefined;
            var anyFields = false;
            for (var field in data) {
                var anyFields = true;
                var fieldData = data[field];

                if (excludeFields[field])        // skip field
                    continue;
                
                if (typeof(fieldData) == "object") 
                    objFields.push(field);
                else if (typeof(fieldData) != "undefined") {
                    if (!field.match(/^[A-Za-z][A-Za-z0-9_.]*$/))
                        field = _xmlEncodeTag(field);

                        // write out all the XML attributes (fields of primitive type)
                    if (typeof(fieldData) == "string" && !fieldData.match(/^[^"'<>&]*$/)) {
                        // logMsg(LogScript, LogInfo, "_xmlWriteHelper encoding '", fieldData, "'\n");
                        fieldData = fieldData.replace(/&/g, "&amp;");
                        fieldData = fieldData.replace(/</g, "&gt;");
                        fieldData = fieldData.replace(/>/g, "&lt;");
                        fieldData = fieldData.replace(/"/g, "&quot;");
                        fieldData = fieldData.replace(/'/g, "&apos;");
                    }
                    if (field == "body")
                        bodyData = fieldData;
                    else 
                        file.Write("\r\n" + indent + "    " + field + "=" + attributeQuoteCharacter + fieldData + attributeQuoteCharacter + "");
                }
            }

                // Write out all other fields (the body field and any fields of non-primitive type)
            if (objFields.length > 0 || bodyData) {
                file.Write(">");
                if (bodyData) {
                    file.Write(bodyData);
                }
                if (objFields.length) {

                    file.WriteLine("");
                    for(var i=0; i < objFields.length; i++) {
                        var field = objFields[i];
                        var fieldData = data[field];

                        if (!field.match(/^[A-Za-z][A-Za-z0-9_.]*$/))
                            field = _xmlEncodeTag(field);
                        if (fieldData != undefined)
                            _xmlWriteHelper(fieldData, file, objIDs, field, indent + "    ", excludeFields, attributeQuoteCharacter);
                    }
                    if (!bodyData)
                        file.Write(indent);
                } 
                file.WriteLine("</" + tag + ">");

            }
            else {
                    // Check to see if it is a date
                try {
                    var dataStr = data.toString();
                    if (dataStr.match(/^\w+ \w+ \d+ \d+:\d+:\d+ \w+ \d+$/))        // looks like a date
                        file.Write(" dateString='" + dataStr + "'");
                } catch(e) {
                    logMsg(LogScript, LogWarn, "Could not do 'toString' on data for tag ", tag, "\n");
                }
                file.WriteLine("/>");
            }
        }
    }
}

