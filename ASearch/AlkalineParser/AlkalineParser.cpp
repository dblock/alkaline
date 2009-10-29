/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  _____________________________________________________

  written by Daniel Doubrovkine - dblock@vestris.com

  Revision history:

  27.08.1999: added m_SkipParseLinks, if set links inside SKIP sections are parsed anyway
  29.08.1999: added document header support, m_HeaderText/Length
  29.08.1999: clean text, title, &gt; &lt; property saved, etc.
  06.09.1999: corrected bug: all links after a relative # link skipped
  24.09.1999: corrected bug: <META HTTP-EQUIV="REFRESH" CONTENT="0; URL=..."> skipped (space after ;)
  13.11.1999: META DESCRIPTION, KEYWORDS don't appear in clear text header
  19.12.1999: -exv will verbose in GetLinks and GetUrl
  19.12.1999: words containing _ will not be broken into two parts
  18.02.2001: not splitting name=[numeric value]
  
*/

#include <alkaline.hpp>

#include "AlkalineParser.hpp"
#include <Internet/Url.hpp>
#include <String/GStrings.hpp>

CAlkalineParser::CAlkalineParser(void) {
    m_EnableObjects = false;
    m_EmptyLinks = true;
    m_EmptyLinksFlag = true;
    m_MetaRobotsNoindex = false;
    m_MetaRobotsNofollow = false;
    m_FreeAlpha = false;
    m_InTitle = false;
    m_CGI = false;
    m_NSF = false;
    m_SkipParseLinks = false;
    m_HeaderLength = 255;
    m_VerboseParser = false;	
}

CAlkalineParser::~CAlkalineParser(void) {
    
}

void CAlkalineParser::RemoveAll(void) {
    m_EmptyLinksFlag = true;
    m_InTitle = false;	
    m_ActiveXObjects.RemoveAll();
    m_Email.RemoveAll();
    m_SavedLink.Empty();
    m_MetaData.RemoveAll();
    m_MetaRawData.RemoveAll();
    m_RawText.Empty();
    m_Title.Empty();
    m_MetaDescription.Empty();
    m_MetaKeywords.Empty();
    m_BaseHref.Empty();
    m_AltText.Empty();
    m_HeaderText.Empty();
    m_Actions.RemoveAll();
}

void CAlkalineParser::Parse(const CString& String) {

    static const CString __Lt("&lt;");
    static const CString __Gt("&gt;");
    static const CString __L("<");
    static const CString __G(">");
    
    // _L_DEBUG(2, m_VerboseParser = true);
    
    RemoveAll();
    m_RawText.SetSize(String.GetLength() / 2);
    CHtmlParser::Parse(String);
    /* remaining link (unparsed) */
	AddSavedLink(true);
    /* <title> */    
    CleanTextLight(m_Title);
    m_Title.RemoveDuplicate(0, ' ');
    m_Title.Trim32();
    
    if (m_Title.GetLength()) {
        /* this is done to avoid adding lt and gt to meta data */
        m_Title.Replace(__Lt, __L);
        m_Title.Replace(__Gt, __G);
        AddMetaData("TITLE",m_Title);
        m_Title.Replace(__L, __Lt);
        m_Title.Replace(__G, __Gt);
    }
    /* raw text, cleaned for words only */
    CleanTextLight(m_RawText);
    /* header of the document */
    m_RawText.Mid(0, m_HeaderLength, &m_HeaderText);
    /* add the meta description to free text */
    if (m_MetaDescription.GetLength()) AddToRawText(m_MetaDescription);
    if (m_MetaKeywords.GetLength()) AddToRawText(m_MetaKeywords);
    /* uncleaned from the parser, for indexing only */
    m_RawText.TerminateWith(' ');
    m_RawText += m_Title;
    m_RawText.Replace(__Lt, CString::EmptyCString);
    m_RawText.Replace(__Gt, CString::EmptyCString);
    CleanText(m_RawText, false);
    CleanTextLight(m_MetaDescription);
}

void CAlkalineParser::AddActiveXObject(const CHtmlTag& /* Tag */) {
    CIterator Iterator;
    CHtmlTag * SlashObject = (CHtmlTag *) m_HtmlList.GetLast(Iterator);
    CHtmlTag * StartObject = NULL;
    if (!SlashObject) {
        // cout << "Bogus ActiveXObject being processed at " << Tag << endl;
        return;
    }
    StartObject = (CHtmlTag *) m_HtmlList.GetPrev(Iterator);
    /* gather the list for mapping object command line, retrieve the CLSID */
    static const CString __OBJECT("OBJECT");
    while (StartObject) {
        if (StartObject->GetName().Same(__OBJECT)) {
            /* we have the first object reference */
            break;
        }
        StartObject = (CHtmlTag *) m_HtmlList.GetPrev(Iterator);
    }
    if (!SlashObject || !StartObject) {
        // cout << "Bogus ActiveX Object being processed at " << Tag << endl;
        return;
    }
    /* get the ClassID from the StartObject */
    static const CString __CLASSID("CLASSID");
    CString ClassID = StartObject->GetParameters().GetValue(__CLASSID);
    if (!ClassID.GetLength()) {
        // cout << "Bogus ActiveX Object, has no ClassID" << (* StartObject) << endl;
        return;
    }
    /* trace the list and store all elements */
    CStringTable ObjectTable;	
    CHtmlTag * Current = StartObject;
    static const CString __PARAM("PARAM");
    static const CString __NAME("NAME");
    static const CString __VALUE("VALUE");
    CString Name, Value;
    while (Current) {
        if (Current->GetName().Same(__PARAM)) {
            Name = Current->GetParameters().GetValue(__NAME);
            Value = Current->GetParameters().GetValue(__VALUE);
            if (Name.GetLength() && Value.GetLength())
                ObjectTable.Set(Name, Value);
        } else if (Current->GetName().Same(__OBJECT)) {
            ObjectTable.Add(Current->GetParameters());
        }
        Current = (CHtmlTag *) m_HtmlList.GetNext(Iterator);
    }
    m_ActiveXObjects.Add(ObjectTable);
    /*
    cout << "ActiveX Object Table for " << (* StartObject) << endl;
    for (register int i=0;i<(int) ObjectTable.GetSize();i++) {
    cout << "  " << ObjectTable.GetNameAt(i) << "=" << ObjectTable.GetValueAt(i) << endl;
    }	
    */
}

void CAlkalineParser::OnNewTag(const CHtmlTag& Tag) {	

    static const CString __ALKALINE("ALKALINE");
    static const CString __S_ALKALINE("/ALKALINE");
    static const CString __URL("URL");
    static const CString __A("A");
    static const CString __S_A("/A");
    static const CString __HREF("HREF");
    static const CString __AREA("AREA");
    static const CString __SKIP("SKIP");
    static const CString __FRAME("FRAME");
    static const CString __SRC("SRC");
    static const CString __BASE("BASE");
    static const CString __APPLET("APPLET");
    static const CString __IMG("IMG");	
    static const CString __ALT("ALT");
    static const CString __META("META");
    static const CString __NAME("NAME");
    static const CString __HTTPEQUIV("HTTP-EQUIV");
    static const CString __CONTENT("CONTENT");
    static const CString __REFRESH("REFRESH");
    static const CString __DESCRIPTION("DESCRIPTION");
    static const CString __KEYWORDS("KEYWORDS");
    static const CString __NOINDEX("NOINDEX");
    static const CString __NOFOLLOW("NOFOLLOW");
    static const CString __TITLE("TITLE");
    static const CString __S_TITLE("/TITLE");
    static const CString __ROBOTS("ROBOTS");	
    static const CString __S_OBJECT("/OBJECT");
    static const CString __TR("TR");

    CString TagContents = Tag.GetBuffer();
    TagContents.Trim32();
    
    if (! TagContents.GetLength()) {
        if (m_InTitle) {
            m_Title.TerminateWith(' ');
        } else {
            AddToRawText(" ");
        }
        m_EmptyLinksFlag = true;
        return;
    }
    
    if (m_VerboseParser) {
        cout << "alkaline parser :: tag: {" << endl
             << Tag 
             << "}" << endl;
    }
    
    if (Tag.GetName().Same(__ALKALINE)) {
        CVector<CString> TagVector;
        if (Tag.GetParameters().GetSize()) {
            for (int a=0;a<(int) Tag.GetParameters().GetSize();a++) {
    
                if (m_VerboseParser) {
                    cout << "alkaline parser :: action: {" << Tag.GetParameters().GetNameAt(a) << "}" << endl;
                }
                
                if (Tag.GetParameters().GetNameAt(a).Same(__URL)) {
                    CString URLCandidate(Tag.GetParameters().GetValueAt(a));
                    Dequote(URLCandidate);
                    // m_Links+=URLCandidate;

                    if (m_VerboseParser) {
                        cout << "alkaline parser :: url: {" << URLCandidate << "}" << endl;
                    }
                    
                    m_Links.AddSortedUnique(URLCandidate);
                } else {
                    TagVector += Tag.GetParameters().GetNameAt(a);
                }
            }
            
            if (TagVector.GetSize()) {
                PushAction(TagVector);
            }
        }
    } else if (Tag.GetName().Same(__S_ALKALINE)) {
        PopAction();
    } else if ((!m_SkipParseLinks)&&(ActionContains(__SKIP))) {
        // hack: with m_SkipParseLinks links will be taken even inside a SKIP section
        // skip any non-alkaline tag	
    } else if (Tag.GetName().Same(__A)) {	
		AddSavedLink(true);
        m_SavedLink = Tag.GetParameters().GetValue(__HREF);		
        m_EmptyLinksFlag = false;
    } else if (Tag.GetName().Same(__S_A)) {		
		AddSavedLink(false);
    } else if (Tag.GetName().Same(__AREA) || Tag.GetName().Same(__TR)) {
        CString Lnk = Tag.GetParameters().GetValue(__HREF);
        Dequote(Lnk);
        for (int l=0;l<(int) Lnk.GetLength();l++) 
            if (((unsigned char)(Lnk[l])) <= ' ') 
                return;
        if (Lnk.GetLength()) {
            if (m_VerboseParser) {
                cout << "alkaline parser :: area url: {" << Lnk << "}" << endl;
            }
            m_Links.AddSortedUnique(Lnk); // m_Links+=Lnk;
        }
    } else if (Tag.GetName().Same(__FRAME)) {
        CString Lnk = Tag.GetParameters().GetValue(__SRC);
        Dequote(Lnk); 
        if (Lnk.GetLength()) {
            if (m_VerboseParser) {
                cout << "alkaline parser :: frame url: {" << Lnk << "}" << endl;
            }                        
            m_Links.AddSortedUnique(Lnk); // m_Links+=Lnk;
        }
    } else if (Tag.GetName().Same(__BASE)) {
        m_BaseHref = Tag.GetParameters().GetValue(__HREF);
        Dequote(m_BaseHref);
        if (m_VerboseParser) {
            cout << "alkaline parser :: base href: {" << m_BaseHref << "}" << endl;
        }              
    } else if (ActionContains(__SKIP)) {
        // skip everything else
    } else if ((Tag.GetName().Same(__IMG))||(Tag.GetName().Same(__APPLET))) {
        if (!ActionContains(__SKIP)) {
            CString Alt = Tag.GetParameters().GetValue(__ALT);
            Dequote(Alt);
            if (Alt.GetLength()) {
                m_AltText+=Alt;
                m_AltText.TerminateWith(' ');
            }
        }
        m_EmptyLinksFlag = true;
    } else if (Tag.GetName().Same(__META)) {
        
        CString Name = Tag.GetParameters().GetValue(__NAME); Dequote(Name);
        CString Equiv = Tag.GetParameters().GetValue(__HTTPEQUIV); Dequote(Equiv);
        
        if (!Name.GetLength()) 
            Name = Equiv;
        
        if (! CanParseMeta(Name)) {
            return;
        }
        
        CString Value = Tag.GetParameters().GetValue(__CONTENT); Dequote(Value);
        
        if (m_VerboseParser) {
			cout << "alkaline parser :: meta {" << Name << "}={" << Value << "}" << endl;				
        }            
        
        CString CurrentValueTokenName;
        CString CurrentValueTokenValue;
        if ((Tag.GetName().Same(__REFRESH))||(Equiv.Same(__REFRESH))) {
            CVector<CString> MetaTokens; 
            CString::StrToVector(Value, ';', &MetaTokens);
            for (int i=0;i<(int) MetaTokens.GetSize();i++) {
                int ePos = MetaTokens[i].Pos('=');
                if (ePos != -1){					
                    MetaTokens[i].Mid(0, ePos, &CurrentValueTokenName);
                    CurrentValueTokenName.Trim32();
                    MetaTokens[i].Mid(ePos+1, MetaTokens[i].GetLength(), &CurrentValueTokenValue);
                    CurrentValueTokenValue.Trim32();
                    // cout << CurrentValueTokenName << ".." << CurrentValueTokenValue << endl;
                    if ((CurrentValueTokenName.Same(__URL))&&(CurrentValueTokenValue.GetLength())) 
                        m_Links.AddSortedUnique(CurrentValueTokenValue);
                    //m_Links+=CurrentValueTokenValue;
                }
            }
        } else if ((Name.Same(__KEYWORDS))||(Equiv.Same(__KEYWORDS))) {
            m_MetaKeywords = Value;
            if (m_VerboseParser) {
                cout << "alkaline parser :: meta keywords: {" << m_MetaKeywords << "}" << endl;
            }
        } else if ((Equiv.Same(__DESCRIPTION))||(Name.Same(__DESCRIPTION))) {
            m_MetaDescription = Value;
            if (m_VerboseParser) {
                cout << "alkaline parser :: meta description: {" << m_MetaDescription << "}" << endl;
            }    
        } else if ((Equiv.Same(__ROBOTS))||(Name.Same(__ROBOTS))) {
            if (Value.Same(__NOINDEX)) {
                m_MetaRobotsNoindex = true;
                if (m_VerboseParser) {
                    cout << "alkaline parser :: robots: noindex" << endl;
                }    
            } else if (Value.Same(__NOFOLLOW)) {
                m_MetaRobotsNofollow = true;
                if (m_VerboseParser) {
                    cout << "alkaline parser :: robots: nofollow" << endl;
                }
            }
        } else if ((Name.Same(__ALKALINE))||Equiv.Same(__ALKALINE)) {
            Name.UpperCase();
            Equiv.UpperCase();
            Value.UpperCase();
        }
        
        if (Name.GetLength()) {
            AddMetaData(Name, Value, false);             
        }

		// all the other pairs are added as meta tags, for example FOO="BAR"

		for (int m = 0; m < (int) Tag.GetParameters().GetSize(); m++) {
			
			CString MetaName = Tag.GetParameters().GetNameAt(m);
			Dequote(MetaName);
			
			if (MetaName.GetLength() == 0 ||
				MetaName.Same(__NAME) ||
				MetaName.Same(__HTTPEQUIV) ||
				MetaName.Same(__CONTENT)) {
				continue;
			}

			CString MetaValue = Tag.GetParameters().GetValueAt(m);
			Dequote(MetaValue);

			if (m_VerboseParser) {
				cout << "alkaline parser :: meta {" << MetaName << "}={" << MetaValue << "}" << endl;				
			}

			AddMetaData(MetaName, MetaValue, false);
		}
        
    } else if (Tag.GetName().Same(__TITLE)) {
        if (!m_Title.GetLength())
            m_InTitle = true;
    } else if (Tag.GetName().Same(__S_TITLE)) {
        m_InTitle = false;
    } else if (Tag.GetName().Same(__S_OBJECT)) {
        /* OBJECT termination */
        if (m_EnableObjects) {
            if (m_VerboseParser) {
                cout << "alkaline parser :: adding object" << endl;
            } 
            AddActiveXObject(Tag);
        }
    } else if (Tag.GetFree().GetLength()) {
        if (m_InTitle) {
            m_Title += Tag.GetFree();
            // _L_DEBUG(2, cout << "CAlkalineParser::Parse() - title:[" << m_Title << "]" << endl);
        } else {
            AddToRawText(Tag.GetFree());
        }
        m_EmptyLinksFlag = true;
    }
    // _L_DEBUG(2, cout << "CAlkalineParser::OnNewTag - " << Tag << " (ok)" << endl);
}

void CAlkalineParser::AddToRawText(const CString& Text) {
    if ((Text.GetLength())&&(!ActionContains("SKIP"))) {
        if (Text.GetCount(0, ' ') != (int) Text.GetLength()) {
            CString TextCopy(Text);
            TranslateQuotes(TextCopy);          
            TextCopy.Replace(0, 13, ' ');
            TextCopy.RemoveDuplicate(0, ' ');
            TextCopy.Trim(); 
            if (m_RawText.GetLength())
                m_RawText.TerminateWith(' ');            
            m_RawText += TextCopy;
        }
    }
}

void CAlkalineParser::AddMetaData(const CString& Name, const CString& Value, bool VerifyParseMeta) {
    
    if (VerifyParseMeta) {
        if (! CanParseMeta(Name)) {
            return;
        }
    }
    
    m_MetaRawData.Set(Name, Value);
    
    CVector<CString> CurWords;
    CVector<CString> CurName;
    CString::StrToVector(Name, ' ', &CurName);
    register int i;
    for (i=((int) CurName.GetSize())-1;i>=0;i--) {
        if (!CurName[i].GetLength()) CurName-=i;
        else CurName[i].UpperCase();
    }
    CurName.QuickSortUnique();
    CString FreeTextClean(Value);
    CleanText(FreeTextClean, true);
    CString::StrToVector(FreeTextClean, ' ', &CurWords);
    CVector<CString> MetaWords;
    for (i=((int) CurWords.GetSize())-1;i>=0;i--) {
        if (CurWords[i].GetLength()) {
            TranslateQuotes((CString&) CurWords[i]);
            RemoveAccents(CurWords[i], m_FreeAlpha);
            MetaWords += CString(CurWords[i]);
        }
    }
    MetaWords.QuickSortUnique();
    for (i=((int) MetaWords.GetSize())-1;i>=0;i--) {
        if (MetaWords[i].GetLength()) {
            for (int j=((int)CurName.GetSize())-1;j>=0;j--) {
                CString MetaDataString = CurName[j] + ':' + MetaWords[i];
                TranslateQuotes((CString&) MetaDataString);
                m_MetaData.Add(MetaDataString);
            }
        }
    }
}

bool CAlkalineParser::ActionContains(const CString& Action) const {
    CString UAction = Action;
    UAction.UpperCase();
    if (m_Actions.GetSize()) return m_Actions[m_Actions.GetSize()-1].Contains(UAction);
    return false;
}

void CAlkalineParser::PushAction(CVector<CString>& TagVector) {
    for (register int i=0;i<(int)TagVector.GetSize();i++) TagVector[i].UpperCase();
    m_Actions += TagVector;
}

void CAlkalineParser::PopAction(void) {
    if (m_Actions.GetSize()) {
        m_Actions.RemoveAt(m_Actions.GetSize()-1);
    }
}

void CAlkalineParser::CleanTextLight(CString& iStr) const {
    TranslateQuotes(iStr);
    iStr.Replace(0, 31, ' ');
}

void CAlkalineParser::CleanText(CString& iStr, bool Everything) const {
    
    if (! iStr.GetLength())
        return;

    TranslateQuotes(iStr);
    
    unsigned char ch = 0;
    unsigned char prch = 0;
    unsigned char nch = iStr[0];

    bool fPunctSet = false;
    
    for (int j = 0; j < (int) iStr.GetLength(); j++) {

        prch = ch;
        ch = nch;
        nch = (j < (int) iStr.GetLength() - 1) ? (unsigned char) iStr[j + 1] : 0;
            
        // do not split numeric/alpha.numeric/alpha
        if (ch == '.') {

            if (isalnum(prch) && isalnum(nch)) {
                fPunctSet = true; // there's a punctuation set within the word
                continue;
            }

            if (fPunctSet)
                continue;

        }

        // do not split name=[-]numeric
        if (ch == '=') {
            if (isalpha(prch)) {
                if (isdigit(nch))
                    continue;
                if ((nch == '-') || (nch == '+')) {
                    if ((j < (int) iStr.GetLength() - 2) && isdigit(iStr[j+2]))
                        continue;
                }
            }
        }        
        
        if (((ch <  '0') &&
            (ch != '-') &&
            (ch != '_') &&
            (ch != '@') &&
            (ch != '\'')
            ) || (
            // the three characters between numbers and letters
            (ch == ':') ||
            (ch == ';') ||
            (ch == '?')
            )) {
            
            iStr[j] = ' ';
            fPunctSet = false;

        } else if ((!m_FreeAlpha) && Everything) {
            
            if ((ch == '-') ||
                (ch == '@')||
                (ch == '_')||
                (ch == '\'') ||
                isalnum(ch)) continue;

            // accents
            if (TransformAccent(iStr, j, m_FreeAlpha))                
                continue;
            
            // free alpha
            if (TransformAccentComplex(iStr, j, m_FreeAlpha))
                continue;

            iStr[j] = ' ';
            fPunctSet = false;
        }
    }
}

bool CAlkalineParser::TransformAccentComplex(CString& iStr, int& j, bool FreeAlpha) {
    if (FreeAlpha) 
        return false;

    switch(iStr[j]) {
    case 'æ':
        iStr[j] = 'a';
        iStr.Insert(j+1, 'e');
        j++;
        return true;
    case 'ß':
        iStr[j] = 's';
        iStr.Insert(j+1, 's');
        j++;
        return true;
    }
    return false;
}

bool CAlkalineParser::TransformAccent(CString& iStr, int& j, bool FreeAlpha){
    if (FreeAlpha) 
        return false;
    
    switch (iStr[j]){
    case 'à':
    case 'á':
    case 'â':
    case 'ã':
    case 'ä':
    case 'å': iStr[j] = 'a'; break;
    case 'ç': iStr[j] = 'c'; break;
    case 'è':
    case 'é':
    case 'ê':
    case 'ë': iStr[j] = 'e'; break;
    case 'ì':
    case 'í':
    case 'î':
    case 'ï': iStr[j] = 'i'; break;
    case 'ñ': iStr[j] = 'n'; break;
    case 'ò':
    case 'ó':
    case 'ô':
    case 'õ':
    case 'ø':
    case 'ö': iStr[j] = 'o'; break;
    case 'ù':
    case 'ú':
    case 'û':
    case 'ü': iStr[j] = 'u'; break;
    case 'ý':
    case 'ÿ': iStr[j] = 'y'; break;
    case 'À':
    case 'Á':
    case 'Â':
    case 'Ã':
    case 'Ä':
    case 'Æ':
    case 'Å': iStr[j] = 'A'; break;
    case 'Ç': iStr[j] = 'C'; break;
    case 'È':
    case 'É':
    case 'Ê':
    case 'Ë': iStr[j] = 'E'; break;
    case 'Ì':
    case 'Í':
    case 'Î':
    case 'Ï': iStr[j] = 'I'; break;
    case 'Ñ': iStr[j] = 'N'; break;
    case 'Ò':
    case 'Ó':
    case 'Ô':
    case 'Õ':
    case 'Ø':
    case 'Ö': iStr[j] = 'O'; break;
    case 'Ù':
    case 'Ú':
    case 'Û':
    case 'Ü': iStr[j] = 'U'; break;
    case 'Ý': iStr[j] = 'Y'; break;
    default:
        return false;
    }

    return true;    
}

void CAlkalineParser::RemoveAccents(CString& Str, bool m_FreeAlpha){
    if (!m_FreeAlpha) {
        for (int i=0;i<(int) Str.GetLength();i++){
            TransformAccent(Str, i, m_FreeAlpha);
            TransformAccentComplex(Str, i, m_FreeAlpha);
        }
    }
}

void CAlkalineParser::AddURL(CVector<CString>& Vector,
                             const CString& Url,
                             const CVector<CString>& ValidExtensions,
                             bool EnableExcludeVerbose) const {
    
    CString MidString;
	static const CString g_strMailTo("mailto:");
    
	if (Url.StartsWithSame(g_strMailTo)) {
        Url.Mid(g_strMailTo.GetLength(), Url.GetLength(), &MidString);
        (((CAlkalineParser&)(* this)).m_Email) += MidString;

        if (m_VerboseParser) {
            cout << "  mailto: {" << Url << "}" << endl;
        }
        
        return;
    }
    
    if (Url.StartsWith("\\")) {
        if (EnableExcludeVerbose) {
            cout << "\n   {URL} " << Url << " excluded because it starts with a backslash, which is invalid." << endl;
        }
        return;
    }
    if (Url.GetLength() && (Url[0] == '#')) {
        if (m_VerboseParser) {
            cout << "  local: {" << Url << "}" << endl;
        }
        if (EnableExcludeVerbose) {
            cout << "\n   {URL} " << Url << " excluded because it is local to current page." << endl;
        }
        return;
    }
    
    CUrl BaseURL(m_BaseHref);	
    CUrl Result(BaseURL.Resolve(Url));
    
    if (m_VerboseParser) {
        cout << "  scheme: {" << Result.GetScheme() << "}" << endl;
    }  
    
    if (Result.GetScheme().Same(g_strProto_HTTPS)) 
        Result.SetScheme(g_strProto_HTTP);
    
    if (! Result.GetScheme().Same(g_strProto_HTTP) &&
        ! Result.GetScheme().Same(g_strProto_FILE)) {
        if (EnableExcludeVerbose) {
            cout << "\n   {URL} " << Url << " excluded because Alkaline does not know how to index this protocol type." << endl;
        }
        return;
    }
    
    CString Candidate = Result.GetHttpAll();
    
    if (m_VerboseParser) {
        cout << "  url: {" << Candidate << "}" << endl;
    }    
    
    /* check for filename extension */
    CString FName = Result.GetHttpFilename();
    
    if (m_VerboseParser) {
        cout << "  filename: {" << FName << "}" << endl;
    }  
    
    if (FName.GetLength()) {
        int pPos = FName.InvPos('.');
        if (pPos >= 0){
            CString pExt;
            FName.Mid(pPos+1, FName.GetLength(), &pExt);
            for (int j=0;j<(int)pExt.GetLength();j++) {
                if (!isalnum(pExt[j])) {
                    pExt.Delete(j, pExt.GetLength());
                    break;
                }
            }
                
            if (m_VerboseParser) {
                cout << "  extension: {" << pExt << "}" << endl;
            }  
            
            if (!ValidExtensions.Contains(pExt) && pExt.GetLength()) {
                if (EnableExcludeVerbose) {
                    cout << "\n   {URL} " << Candidate << " excluded because ." << pExt << " is not a known extension." << endl;
                }
                return;
            }
        }
    }
    
    for (int i=0;i<(int) Candidate.GetLength();i++) {
        if (((unsigned char) Candidate[i]) < ' ') {
            if (EnableExcludeVerbose) cout << "\n   {URL} " << Candidate << " excluded because it contains illegal control characters." << endl;
            return;
        }
    }
    
    /* check for CGI redirection '?' */
    // _L_DEBUG(2, cout << "AddURL() - Checking for CGI" << endl);
    CString HttpSearchPath = Result.GetHttpSearchPath();
    
    if (m_VerboseParser) {
        cout << "  query string: {" << HttpSearchPath << "}" << endl;
    }  
    
    if (HttpSearchPath.GetLength() && !m_CGI) {
        if (EnableExcludeVerbose) {
            cout << "\n   {URL} " << Candidate << " contains a QueryString and is excluded because CGI is not set." << endl;
        }
        return;
    }
    
    /* check for NSF ExpandView (Lotus Notes / Domino) */
    
    static const CString Nsf(".nsf");
    static const CString OpenDocument("OpenDocument");
    static const CString OpenView("OpenView");
    static const CString ExpandView("&ExpandView");
    
    /*
      cout << "URL: " << Result.GetHttpAll() << endl;
      cout << "NSF: " << CString::BoolToStr(m_NSF) << endl;
      cout << "SPS: " << CString::BoolToStr(Result.GetHttpPath().SamePos(Nsf) != -1) << endl;
      cout << "HSP: " << CString::BoolToStr(HttpSearchPath.StartsWith(OpenDocument)) << endl;
      cout << "HOP: " << CString::BoolToStr(HttpSearchPath.StartsWith(OpenView)) << endl;
    */
    
    if (m_NSF && 
        ((Result.GetHttpPath().SamePos(Nsf) != -1) &&
         ((HttpSearchPath.StartsWith(OpenDocument))||(HttpSearchPath.StartsWith(OpenView)))
            )) {
        
        if (HttpSearchPath.StartsWith(OpenDocument)) 
            Result.SetHttpSearchPath(OpenDocument + ExpandView);
        else if (HttpSearchPath.StartsWith(OpenView)) 
            Result.SetHttpSearchPath(OpenView + ExpandView);
        Candidate = Result.GetHttpAll();
        if (m_VerboseParser) {
            cout << "  nsf-expand: {" << Candidate << "}" << endl;
        }          
    }
    
    if (m_VerboseParser) {
        cout << "  url: {" << Candidate << "} added." << endl;
    }
    
    Vector.AddSortedUnique(Candidate);
    // _L_DEBUG(2, cout << "  AddURL(OK) - " << Candidate << endl);
}

void CAlkalineParser::GetLinks(const CVector<CString>& ValidExtensions, CVector<CString>& Result, const CString& BaseHref, bool EnableExcludeVerbose) const {
    Result.RemoveAll();
    if (!m_BaseHref.GetLength()) ((CAlkalineParser *) this)->m_BaseHref = BaseHref;

    if (m_VerboseParser) {
        cout << "  alkaline parser :: links : base href {" << m_BaseHref << "}" << endl;
    }  
    
    CString Link;
    for (int i=0;i<(int) m_Links.GetSize();i++) {
        Link = m_Links[i];
        Link.Trim32();

        if (Link.GetLength()) {            
            AddURL(
                Result, 
                Link, 
                ValidExtensions, 
                EnableExcludeVerbose);
        }
    }
    
}

void CAlkalineParser::GetWords(CVector<CString>& Result) const {
    int i;
    
    Result.RemoveAll();
    
    // _L_DEBUG(2, cout << "CAlkalineParser::GetWords() - creating and growing words vector" << endl);
    // _L_DEBUG(2, cout << "CAlkalineParser::GetWords() - appending ALTs" << endl);
    
    CString FreeTextClean(m_RawText);

    for (i=0;i<(int)m_MetaRawData.GetSize();i++) {
        FreeTextClean.TerminateWith(' ');
        FreeTextClean += m_MetaRawData[i].GetValue();
    }
    
    if (m_AltText.GetLength()) {
        FreeTextClean.TerminateWith(' ');
        FreeTextClean += m_AltText;
    }
    CleanText(FreeTextClean, true);
    
    // _L_DEBUG(2, cout << "CAlkalineParser::GetWords() - tokenizer" << endl);
    CString::StrToVector(FreeTextClean, ' ', &Result);
    // _L_DEBUG(2, cout << "CAlkalineParser::GetWords() - total of " << Result.GetSize() << endl);
    for (i=0;i<(int) m_Email.GetSize();i++) Result+=m_Email[i];
    for (i=0;i<(int) Result.GetSize();i++) RemoveAccents(Result[i], m_FreeAlpha);
    Result.QuickSortUnique();
    // _L_DEBUG(2, cout << "CAlkalineParser::GetWords() - removing duplicates" << endl);
    for (i=(int) Result.GetSize()-1;i>0;i--) {
        if ((Result[i-1] == Result[i])||(!Result[i].GetLength()))
            Result.RemoveAt(i);
    }
    if (Result.GetSize()&&(!Result[0].GetLength())) 
        Result.RemoveAt(0);
    // _L_DEBUG(2, cout << "CAlkalineParser::GetWords() - done, " << Result.GetSize() << " words." << endl);
}

bool CAlkalineParser::GetMetaData(const CString& Name, CString * pValue) const {
  bool bResult = m_MetaRawData.FindAndCopy(Name, * pValue);
  if (bResult) {
    pValue->Replace(0, 31, ' ');
    pValue->RemoveDuplicate(0, ' ');
  }
  return bResult;
}

bool CAlkalineParser::CanParseMeta(const CString& MetaName) const {

    if (!MetaName.GetLength())
        return false;

    if (m_ParseMetas.GetSize() == 0)
        return true;   
    
    bool bParseMeta = false;
    for (register int i = 0; i < (int) m_ParseMetas.GetSize(); i++) {
        CString CompMeta = m_ParseMetas[i];

        if (!CompMeta.GetLength())
            continue;
        
        if (CompMeta == g_strStar) {
            bParseMeta = true;
        } else if (CompMeta[0] == '-') {
            CompMeta.Delete(0, 1);
            if (CompMeta.Same(MetaName))
                return false;
        } else {
            if (CompMeta[0] == '+')
                CompMeta.Delete(0, 1);
            if (CompMeta.Same(MetaName))
                return true;
        }
    }
    
    return bParseMeta;
}

bool CAlkalineParser::AddSavedLink(bool bEmptyLink) {
	bool bResult = false;

	if (! m_SavedLink.GetLength())
		return false;

	if (m_VerboseParser) {
		cout << "alkaline parser :: url (tested): {" << m_SavedLink << "}" << endl;
	}

	Dequote(m_SavedLink);
	m_SavedLink = CUrl::UnEscape(m_SavedLink);
	TranslateQuotes(m_SavedLink);

	for (int l=0;l<(int) m_SavedLink.GetLength();l++) {
		if ((unsigned char) m_SavedLink[l] < ' ') {
			if (m_VerboseParser) {
				cout << "alkaline parser :: invalid character at position " << l << " {" << m_SavedLink << "}" << endl;
			}
			m_SavedLink.Empty();
			break;
		}
	}

	if (m_SavedLink.GetLength()) {
		if (! bEmptyLink || (m_EmptyLinksFlag || m_EmptyLinks)) {                
			m_Links.AddSortedUnique(m_SavedLink);
			if (m_VerboseParser) {
				cout << "alkaline parser :: url (added): {" << m_SavedLink << "}" << endl;
			}
			bResult = true;
		} else {
			if (m_VerboseParser) {
				cout << "alkaline parser :: not closed url (ignored): {" << m_SavedLink << "}" << endl;
			}
		}
		m_SavedLink.Empty();
	}

	return bResult;
}
