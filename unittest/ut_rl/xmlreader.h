/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
//-----------------------------------------------------------------------------
//
// Description:
//
//   Simple C++ Xml Reader classes.
//
// Remarks:
//
//-----------------------------------------------------------------------------

namespace Xml
{


// May want Unicode someday.

typedef char Char;


class Attribute
{

public:

   Attribute(Char * name, Char * value);

   void Append(Attribute *);

   Char * GetValue(const Char * name);

   void Dump();

public:

   Attribute * Next;
   Char * Name;
   Char * Value;
};

class Node
{

public:

   Node() {}
   Node(Char * name, Attribute * attributeList);

   void CheckTag(Char * name);

   void Append(Node *);

   Node * GetChild(const Char * name);
   Char * GetAttributeValue(const Char * name);

   void Dump(int indent);
   void Dump();

public:

   Node * Next;
   Node * ChildList;
   Attribute * AttributeList;
   Char * Name;
   Char * Data;
   int LineNumber;

   static Node * TopNode;
};


bool Init();
Node * ReadFile(const char * fileName);


} // namespace Xml
