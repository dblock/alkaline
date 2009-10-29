// Simple XML Processing Library - Ver .91 Feb 21 1998
//	Copyright 1998 Jeremie (jer@jeremie.com)

function _element() {
  this.type = "element";
  this.name = new String();
  this.attributes = new Array();
  this.contents = new Array();
  this.uid = _Xparse_count++;
  _Xparse_index[this.uid]=this;
}

function _chardata() {
  this.type = "chardata";
  this.value = new String();
}

function _pi() {
  this.type = "pi";
  this.value = new String();
}

function _comment() {
  this.type = "comment";
  this.value = new String();
}

// an internal fragment that is passed between functions
function _frag() {
  this.str = new String();
  this.ary = new Array();
  this.end = new String();
}

// global vars to track element UID's for the index
var _Xparse_count = 0;
var _Xparse_index = new Array();

// Main public function that is called to 
// parse the XML string and return a root element object
function Xparse(src) {
  var frag = new _frag();
  // remove bad \r characters and the prolog
  frag.str = _prolog(src);
  // create a root element to contain the document
  var root = new _element();
  root.name="ROOT";
  // main recursive function to process the xml
  frag = _compile(frag);
  // all done, lets return the root element + index + document
  root.contents = frag.ary;
  root.index = _Xparse_index;
  _Xparse_index = new Array();
  return root;
}

// transforms raw text input into a multilevel array
function _compile(frag)
{
  // keep circling and eating the str
  while(1) {
    // when the str is empty, return the fragment
    if(frag.str.length == 0) {
      return frag;
    }    
    var TagStart = frag.str.indexOf("<");    
    if(TagStart != 0) {
      // theres a chunk of characters here, store it and go on
      var thisary = frag.ary.length;
      frag.ary[thisary] = new _chardata();
      if(TagStart == -1) {
	frag.ary[thisary].value = _entity(frag.str);
	frag.str = "";
      } else {
	frag.ary[thisary].value = _entity(frag.str.substring(0,TagStart));
	frag.str = frag.str.substring(TagStart,frag.str.length);
      }
    } else {
      // determine what the next section is, and process it
      if(frag.str.substring(1,2) == "?") {
	frag = _tag_pi(frag);
      } else {
	if(frag.str.substring(1,4) == "!--") {
	  frag = _tag_comment(frag);
	} else {
	  if(frag.str.substring(1,9) == "![CDATA[") {
	    frag = _tag_cdata(frag);
	  } else {
	    if(frag.str.substring(1,frag.end.length + 3) == "/" + frag.end + ">" || _strip(frag.str.substring(1,frag.end.length + 3)) == "/" + frag.end) {
	      // found the end of the current tag, end the recursive process and return
	      frag.str = frag.str.substring(frag.end.length + 3,frag.str.length);
	      frag.end = "";
	      return frag;
	    } else {
	      frag = _tag_element(frag);
	    }
	  }
	}
      }      
    }
  }
  return "";
}

// functions to process different tags
function _tag_element(frag) {
  // initialize some temporary variables for manipulating the tag
  var close = frag.str.indexOf(">");
  var empty = (frag.str.substring(close - 1,close) == "/");
  if(empty) {
    close -= 1;
  }
  // split up the name and attributes
  var starttag = _normalize(frag.str.substring(1,close));
  var nextspace = starttag.indexOf(" ");
  var attribs = new String();
  var name = new String();
  if(nextspace != -1) {
    name = starttag.substring(0,nextspace);
    attribs = starttag.substring(nextspace + 1,starttag.length);
  } else {
    name = starttag;
  }
  var thisary = frag.ary.length;
  frag.ary[thisary] = new _element();
  frag.ary[thisary].name = _strip(name);
  if(attribs.length > 0) {
    frag.ary[thisary].attributes = _attribution(attribs);
  }
  if(!empty) {
    // take the contents of the tag and parse them
    var contents = new _frag();
    contents.str = frag.str.substring(close + 1,frag.str.length);
    contents.end = name;
    contents = _compile(contents);
    frag.ary[thisary].contents = contents.ary;
    frag.str = contents.str;
  } else {
    frag.str = frag.str.substring(close + 2,frag.str.length);
  }
  return frag;
}

function _tag_pi(frag) {
  var close = frag.str.indexOf("?>");
  var val = frag.str.substring(2,close);
  var thisary = frag.ary.length;
  frag.ary[thisary] = new _pi();
  frag.ary[thisary].value = val;
  frag.str = frag.str.substring(close + 2,frag.str.length);
  return frag;
}

function _tag_comment(frag) {
  var close = frag.str.indexOf("-->");
  var val = frag.str.substring(4,close);
  var thisary = frag.ary.length;
  frag.ary[thisary] = new _comment();
  frag.ary[thisary].value = val;
  frag.str = frag.str.substring(close + 3,frag.str.length);
  return frag;
}

function _tag_cdata(frag) {
  var close = frag.str.indexOf("]]>");
  var val = frag.str.substring(9,close);
  var thisary = frag.ary.length;
  frag.ary[thisary] = new _chardata();
  frag.ary[thisary].value = val;
  frag.str = frag.str.substring(close + 3,frag.str.length);
  return frag;
}

// util for element attribute parsing
// returns an array of all of the keys = values
function _attribution(str) {
  var all = new Array();
  while(1) {
    var eq = str.indexOf("=");
    if(str.length == 0 || eq == -1) {
      return all;
    }    
    var id1 = str.indexOf("\'");
    var id2 = str.indexOf("\"");
    var ids = new Number();
    var id = new String();
    if((id1 < id2 && id1 != -1) || id2 == -1) {
      ids = id1;
      id = "\'";
    }
    if((id2 < id1 || id1 == -1) && id2 != -1) {
      ids = id2;
      id = "\"";
    }
    var nextid = str.indexOf(id,ids + 1);
    var val = str.substring(ids + 1,nextid);
    
    var name = _strip(str.substring(0,eq));
    all[name] = _entity(val);
    str = str.substring(nextid + 1,str.length);
  }
  return "";
}

// util to remove \r characters from input string
// and return xml string without a prolog
function _prolog(str) {
  var A = new Array();  
  A = str.split("\r\n");
  str = A.join("\n");
  A = str.split("\r");
  str = A.join("\n");
  var start = str.indexOf("<");
  if(str.substring(start,start + 3) == "<?x" || str.substring(start,start + 3) == "<?X" ) {
    var close = str.indexOf("?>");
    str = str.substring(close + 2,str.length);
  }
  var start = str.indexOf("<!DOCTYPE");
  if(start != -1) {
    var close = str.indexOf(">",start) + 1;
    var dp = str.indexOf("[",start);
    if(dp < close && dp != -1) {
      close = str.indexOf("]>",start) + 2;
    }
    str = str.substring(close,str.length);
  }
  return str;
}

// util to remove white characters from input string
function _strip(str) {
  var A = new Array();  
  A = str.split("\n");
  str = A.join("");
  A = str.split(" ");
  str = A.join("");
  A = str.split("\t");
  str = A.join("");  
  return str;
}

// util to replace white characters in input string
function _normalize(str) {
  var A = new Array();  
  A = str.split("\n");
  str = A.join(" ");
  A = str.split("\t");
  str = A.join(" ");
  return str;
}

// util to replace internal entities in input string
function _entity(str) {
  var A = new Array();
  A = str.split("&lt;");
  str = A.join("<");
  A = str.split("&gt;");
  str = A.join(">");
  A = str.split("&quot;");
  str = A.join("\"");
  A = str.split("&apos;");
  str = A.join("\'");
  A = str.split("&amp;");
  str = A.join("&");
  return str;
}

//////////////////////
//// added by dB. (dblock@vestris.com)
///
function XParseGetTagValue(ArrayPath, ArrayIndex, Tag) {
  var ResultTag = XParseGetTagValueTag(ArrayPath, ArrayIndex, Tag);
  if (ResultTag) {
    var Result = new String;
    for (var j = 0; j < ResultTag.contents.length; j++) {
      Result += ResultTag.contents[j].value;
    }
    return Result;
  }
  return "";
}

function XParseGetTagValueTag(ArrayPath, ArrayIndex, Tag) {
  if (ArrayIndex >= ArrayPath.length) {
    return Tag;
  }
  while (ArrayPath[ArrayIndex].length == 0) {
    ArrayIndex++;
  }
  for (var i = 0; i < Tag.contents.length; i++) {
    if (Tag.contents[i].name == ArrayPath[ArrayIndex]) {
      return XParseGetTagValueTag(ArrayPath, ArrayIndex + 1, Tag.contents[i]);
    }
  }       
}

function XParseGetValue(Path, XParsedTag) {      
  var ArrayPath = new Array();
  var ArrayString = new String(Path);
  ArrayPath = ArrayString.split("/");
  return XParseGetTagValue(ArrayPath, 0, XParsedTag);
}     

function XParseShowXML(XmlString) {
  var XParsedObject = new Array();
  XParsedObject = Xparse(XmlString);
  document.writeln(retag(XParsedObject));
}
 
function XParseShowTable(Header, Variables, ParsedObject) {
  document.writeln("<table width=100% cellpadding=2>");
  document.writeln("<tr><td width=100% colspan=2 bgcolor=#EEEEEE><b>" + Header + "</b></td></tr>");

  var VariablesString = new String(Variables);
  var VariablesArray = new Array();
  VariablesArray = VariablesString.split(";");
  
  var EachArray = new Array();
  var EachString = new String();
  var EachXmlValue = new String();
  
  for (var i = 0; i < VariablesArray.length; i++) {
    EachString = VariablesArray[i];
    EachArray = EachString.split(":");
    if (EachArray.length >= 2) {
      EachXmlValue = XParseGetValue(EachArray[1], ParsedObject);
      if (EachXmlValue.length == 0)
        EachXmlValue = EachArray[1];
      document.writeln("<tr><td width=35% align=right><b>" + 
         EachArray[0] + 
         ":</b></td><td>" + 
         EachXmlValue +
         "</td></tr>");
    }
  }
  document.writeln("</table>");
}

function XParseShowXMLTable(Header, Path, ParsedObject) {
  document.writeln("<table width=100% cellpadding=2>");
  document.writeln("<tr><td width=100% colspan=2 bgcolor=#EEEEEE><b>" + Header + "</b></td></tr>");

  var ArrayPath = new Array();
  var ArrayString = new String(Path);
  ArrayPath = ArrayString.split("/");

  var Tag = XParseGetTagValueTag(ArrayPath, 0, ParsedObject); 
  if (Tag) {
    for (var i=0; i < Tag.contents.length; i++) {
      var XmlValue = new String;
      var XmlTag = Tag.contents[i];
      if (XmlTag.contents != undefined) {
        for (var j = 0; j < XmlTag.contents.length; j++) {
          XmlValue += XmlTag.contents[j].value;
        }

        document.writeln("<tr><td width=35% align=right><b>" + 
                         Tag.contents[i].name + 
                         ":</b></td><td>" + 
                         XmlValue +
                         "</td></tr>");
      }
    }
  }
  document.writeln("</table>");
}
 
function retag(tag) {
  var result = new String();
  result += "<FONT COLOR='blue'>&lt;</FONT><FONT COLOR='red'>" + tag.name;
  for(attrib in tag.attributes) {
    result += " " + attrib + '="' + tag.attributes[attrib] + '"';
  }
  result += "</FONT><FONT COLOR='blue'>&gt;</FONT>";
  for(var i=0;i < tag.contents.length;i++) {
    if(tag.contents[i].type=="element") {
      result += retag(tag.contents[i]);
    } else {
      if(tag.contents[i].type == "comment") {
	result += "<FONT COLOR='blue'>&lt;</FONT><FONT COLOR='green'>!--" + entity(tag.contents[i].value) + "--</FONT><FONT COLOR='blue'>&gt;</FONT>";
      } else {
	if(tag.contents[i].type == "pi") {
	  result += "<FONT COLOR='blue'>&lt;</FONT><FONT COLOR='yellow'>?" + entity(tag.contents[i].value) + "?</FONT><FONT COLOR='blue'>&gt;</FONT>";
	} else {
	  result += entity(tag.contents[i].value);
	}
      }
    }
  }
  result += "<FONT COLOR='blue'>&lt;</FONT><FONT COLOR='red'>/" + tag.name + "</FONT><FONT COLOR='blue'>&gt;</FONT>";
  return result;
}

function entity(str) {
  var A = new Array();
  A = str.split("&");
  str = A.join("&amp;");
  A = str.split("<");
  str = A.join("&lt;");
  A = str.split(">");
  str = A.join("&gt;");
  return str;
}

function ASGetParamValue(Name) {
  var NameString = new String(Name);
  var NameLocation = document.URL.indexOf(NameString + "=");
  if (NameLocation < 0)
    return "";
  var NameLocationEnds = document.URL.indexOf("&", NameLocation);
  if (NameLocationEnds < 0)
    NameLocationEnds = document.URL.length;
  NameLocation += (NameString.length + 1);
  return document.URL.substr(NameLocation, NameLocationEnds - NameLocation);
}

function ASShowReponseXml(ResponseXml) {
  var XString = new String(ResponseXml);
  if (XString.length) {
    var XParsedObject = new Array();
    XParsedObject = Xparse(XString);    
    var bError = parseInt(XParseGetValue("/response/error", XParsedObject)); 
    if (bError) {
      document.writeln("<b><font color=red>");
    } else {
      document.writeln("<b><font color=blue>");
    } 
    document.writeln(XParseGetValue("/response/message", XParsedObject));
    document.writeln("</b></font><hr size=1>");
  }
}
