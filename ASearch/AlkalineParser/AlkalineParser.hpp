/*
  
  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  _____________________________________________________
  
  written by Daniel Doubrovkine - dblock@vestris.com
  
  Alkaline's HTML parser, part of the Alkaline Search Engine
  produces meta content, raw text, links, retrieves email
  addresses, etc.
  
*/

#ifndef ALKALINE_HTML_PARSER_HPP
#define ALKALINE_HTML_PARSER_HPP

#include <Internet/HtmlParser.hpp>

#ifndef CStringVector
typedef CVector<CString> CStringVector;
#endif

class CAlkalineParser : public CHtmlParser {
	property(CVector<CStringTable>, ActiveXObjects);
	property(int, HeaderLength);
	property(bool, EnableObjects);
	property(bool, SkipParseLinks);
	property(bool, InTitle);
	property(CString, Title);
	property(CString, MetaDescription);
	private_property(CString, MetaKeywords);
	property(CString, BaseHref);
	property(bool, EmptyLinks);
	private_property(bool, EmptyLinksFlag);
	property(CString, RawText);
	property(CString, AltText);
	property(CString, HeaderText);
	CVector<CString> m_Links;
	property(CVector<CString>, Email);
	private_property(CString, SavedLink);
	property(CVector<CString>, MetaData);
    property(CStringTable, MetaRawData);
	property(bool, MetaRobotsNoindex);
	property(bool, MetaRobotsNofollow);
	property(bool, FreeAlpha);
	property(bool, CGI);
	property(bool, NSF);
    property(CVector<CString>, ParseMetas);
    property(bool, VerboseParser);
private:
	CVector<CStringVector> m_Actions;
	void PushAction(CVector<CString>&);
	void PopAction(void);
	bool ActionContains(const CString& Action) const;
	void CleanText(CString& iStr, bool Everything) const;
	void CleanTextLight(CString& iStr) const;
    bool CanParseMeta(const CString& MetaName) const;
	bool AddSavedLink(bool bEmptyLink);
public:
	CAlkalineParser(void);
	void Parse(const CString&);
	virtual ~CAlkalineParser(void);
	virtual void OnNewTag(const CHtmlTag&);
	void GetLinks(const CVector<CString>& ValidExtensions, CVector<CString>& Result, const CString& BaseHref = CString::EmptyCString, bool EnableExcludeVerbose = false) const;
	void GetWords(CVector<CString>& Result) const;
	static void RemoveAccents(CString& Str, bool m_FreeAlpha);
    bool GetMetaData(const CString& Name, CString * pValue) const;
	void AddMetaData(const CString& Name, const CString& Value, bool bVerifyParseMeta = true);
private:
	void RemoveAll(void);
	void AddActiveXObject(const CHtmlTag& Tag);
	void AddToRawText(const CString& Text);
	static bool TransformAccent(CString&, int& j, bool FreeAlpha);
	static bool TransformAccentComplex(CString&, int& j, bool FreeAlpha);
	void AddURL(CVector<CString>& Vector, const CString& Url, const CVector<CString>& ValidExtensions, bool EnableExcludeVerbose = false) const;
};

#endif
