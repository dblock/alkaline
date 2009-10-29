/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  _____________________________________________________

  written by Daniel Doubrovkine - dblock@vestris.com

  18.02.2000: added SEARCH-BASE-HREF (dB.)

*/

#include <alkaline.hpp>

#include "AlkalineSession.hpp"
#include <Date/Interval.hpp>
#include <Server/Connection.hpp>
#include <Main/TraceTags.hpp>

CAtomic CAlkalineSession::m_AlkalineSessions;

CAlkalineSession::CAlkalineSession(CAlkalineData * AlkalineData) : CSession() {
	m_AlkalineData = AlkalineData;
	m_Searching = false;
	m_AlkalineSessions.Inc();
	Set("MAP","<dl><dt><a href=$url><b>$title</b></a><dd><font size=-1>$header<br><font color=gray>last modif: $modif</font><br>url: <a href=$url>$url</a></font></dl>");
	Set("NEXT","next");
	Set("PREV","prev");
	Set("SEARCH-NORESULTS","<hr>no (more) results<hr>");
	Set("DATE","$DAYENGLISH, $MONTHENGLISH $DAY, $YEAR ($HOUR:$MIN)");
	Set("EXPIRED-COUNT","365");
	Set("EXPIRED","(older than a year)");
	Set("RECENT-COUNT","1");
	Set("RECENT","(modified today)");
	Set("FREECHARSET","0");
	Set("NEXT-DIVISION","10");
	Set("NEXT-INHERIT", CString::EmptyCString);
	Set("SEARCH-BASE-HREF", CString::EmptyCString); 
	Set("HIGHLIGHT-OPEN", "<b>");
	Set("HIGHLIGHT-CLOSE", "</b>");
	Set("SEARCH-BASE-ABS", "$requesturl");
        Set("PAGE-SEPARATOR", "&nbsp;");
}

CAlkalineSession::~CAlkalineSession(void) {

}

bool CAlkalineSession::ProcessResultsURL(CString& Url) const {
	bool Result = false;

	Trace(tagRender, levInfo, ("CAlkalineSession::ProcessResultsUrl - url=%s", Url.GetBuffer()));

	for (register int i=0;i<(int)m_UrlReplacements.GetSize();i++) {
	  Trace(tagRender, levInfo, ("CAlkalineSession::ProcessResultsUrl - source=%s, target=%s", m_UrlReplacements[i].GetName().GetBuffer(), m_UrlReplacements[i].GetValue().GetBuffer()));
	  Result |= Url.Replace(m_UrlReplacements[i].GetName(), m_UrlReplacements[i].GetValue());
	}

	for (register int i=0;i<(int)m_Regexp_UrlReplacements.GetSize();i++) {
	  CString RegexpError;
	  Trace(tagRender, levInfo, ("CAlkalineSession::ProcessResultsUrl - (regexp) source=%s, target=%s [%s]", m_Regexp_UrlReplacements[i].GetName().GetBuffer(), m_Regexp_UrlReplacements[i].GetValue().GetBuffer(), Url.GetBuffer()));
	  Url = CRegExp::SearchReplace(Url, m_Regexp_UrlReplacements[i].GetName(), m_Regexp_UrlReplacements[i].GetValue(), & RegexpError);
	  Result = RegexpError.GetLength() > 0 ? true : false;
	  Trace(tagRender, levInfo, ("CAlkalineSession::ProcessResultsUrl - (regexp) source=%s, target=%s [%s][%s]", m_Regexp_UrlReplacements[i].GetName().GetBuffer(), m_Regexp_UrlReplacements[i].GetValue().GetBuffer(), Url.GetBuffer(), RegexpError.GetBuffer()));	  
	}
	
	Trace(tagRender, levInfo, ("CAlkalineSession::ProcessResultsUrl - url=%s (done)", Url.GetBuffer()));
	return Result;
}

CString CAlkalineSession::GetNextInherit(CHttpIo& /* HttpIo */) const {
	CString Result = Map(GetValue("NEXT-INHERIT"));
	if (Result.GetLength()) return CUrl::Escape(Result);
	else return CString::EmptyCString;
}

bool CAlkalineSession::ProcessTag(CHttpIo& HttpIo, const CString& Tag) {
	if (!m_Searching) return false;
        Trace(tagRender, levInfo, ("CAlkalineSession::ProcessTag [%s]", Tag.GetBuffer()));	
        CString MidString;
	if (Tag.StartsWithSame("SEARCH-GENERAL")) {
                Tag.Mid(base_strlen("SEARCH-GENERAL"), Tag.GetLength(), &MidString);
                Trace(tagRender, levInfo, ("CAlkalineSession::ProcessTag mapping SEARCH-GENERAL [%s]", MidString.GetBuffer()));	
		HttpIo.Write(Map(MidString));
		return true;
	} else  if (Tag.Same("SEARCH-RESULTS")) {
                Trace(tagRender, levInfo, ("CAlkalineSession::ProcessTag mapping SEARCH-RESULTS [%d results]", m_SearchObject.m_SearchResults.GetSize()));	
		if (m_SearchObject.m_SearchResults.GetSize()) {
			for (int i=0;i<(int) m_SearchObject.m_SearchResults.GetSize();i++) {				
				HttpIo.Write(m_SearchObject.m_SearchResults[i]);
			}
		} else {
			HttpIo.Write(Map(GetValue("SEARCH-NORESULTS")));
		}
		return true;
	} else if ((m_SearchObject.m_Quant != -1)&&(Tag.Same("SEARCH-NEXT")&&(m_SearchObject.m_HasNext > m_SearchObject.m_Quant))) {
                Trace(tagRender, levInfo, ("CAlkalineSession::ProcessTag mapping SEARCH-NEXT [%d results]", m_SearchObject.m_HasNext));	
		CString RequestUrl = Map(GetValue("SEARCH-BASE-ABS")) + "?start=";
		if (m_SearchObject.m_Start > 0) {
			HttpIo.Write("<a href=\"" +
				     RequestUrl +
				     CString::IntToStr(m_SearchObject.m_Start - m_SearchObject.m_Quant) +
				     "&quant=" + CString::IntToStr(m_SearchObject.m_Quant) +
				     "&search=" +
				     CUrl::Escape(HttpIo.PostGet("SEARCH"), true) +
				     GetNextInherit(HttpIo) +
				     "\">" +
				     GetValue("PREV") +
				     "</a>" + GetValue("PAGE-SEPARATOR"));
		}

		int DivisionsSize = 10; if (GetValue("NEXT-DIVISION").IsInt(&DivisionsSize) && (DivisionsSize < 1)) DivisionsSize = 10;
		int CurrentPage = m_SearchObject.m_Start / m_SearchObject.m_Quant;
		int CurrentPageDivision = CurrentPage / DivisionsSize;
		int FirstPage = CurrentPageDivision * DivisionsSize + 1;
		
		for (int j = FirstPage, i=CurrentPageDivision*m_SearchObject.m_Quant*DivisionsSize;
		     (j < FirstPage + DivisionsSize)&&(i < m_SearchObject.m_HasNext);
		     j++, i += m_SearchObject.m_Quant) 
		{
			if (i == m_SearchObject.m_Start) {
				HttpIo.Write(
					     GetValue("C-BEFORE") +
					     CString::IntToStr(j) +
					     GetValue("C-AFTER") + GetValue("PAGE-SEPARATOR"));
			} else {
				HttpIo.Write(
					     GetValue("N-BEFORE") +
					     "<a href=\"" +
					     RequestUrl +
					     CString::IntToStr(i) +
					     "&quant=" + CString::IntToStr(m_SearchObject.m_Quant) +
					     "&search=" +
					     CUrl::Escape(HttpIo.PostGet("SEARCH"), true) +
					     GetNextInherit(HttpIo) +
					     "\">" + CString::IntToStr(j) +
					     "</a>" + GetValue("N-AFTER") + GetValue("PAGE-SEPARATOR"));
			}
		}

		if (m_SearchObject.m_HasNext > (m_SearchObject.m_Start + m_SearchObject.m_Quant)) {
			HttpIo.Write(
				     "<a href=\"" +
				     RequestUrl + CString::IntToStr(m_SearchObject.m_Start + m_SearchObject.m_Quant) +
				     "&quant=" + CString::IntToStr(m_SearchObject.m_Quant) +
				     "&search=" +
				     CUrl::Escape(HttpIo.PostGet("SEARCH"), true) +
				     GetNextInherit(HttpIo) +
				     "\">" +
				     GetValue("NEXT") +
				     "</a>");
		}
		return true;
	}
	return CSession::ProcessTag(HttpIo, Tag);
}

void CAlkalineSession::WriteLog(const CString& m_LogFilename, const CString& String) const {
	CString LogString;
	CString CurrentTime;
	time_t l_Time; 
	
	time(&l_Time);	
	CDate::ctime_r(&l_Time, CurrentTime);
	
	LogString = "time=";
	LogString += CurrentTime;
	LogString += "\tmessage=";
	LogString += String;
	LogString += "\tclient=";
	LogString += m_HttpIoPointer->GetConnection()->GetSocket().GetRemoteHost();
	
	CSession::WriteLog(m_LogFilename, LogString);
}

void CAlkalineSession::TraverseOptions(CHttpIo& HttpIo, const CString& Buffer) {
    
        static const CString Search("SEARCH");
	static const CString Quant("QUANT");
	static const CString Start("START");    
    
	m_HttpIoPointer = &HttpIo;
	m_Searching = HttpIo.IsPost() || HttpIo.GetGet(Search).GetLength();

	CSession::TraverseOptions(HttpIo, Buffer);

	CString QuantString = HttpIo.PostGet(Quant);
        if (! QuantString.GetLength()) QuantString = HttpIo.GetGet(Quant);
	if (!QuantString.IsInt(&m_SearchObject.m_Quant))
		if (!GetValue(Quant).IsInt(&m_SearchObject.m_Quant))
			m_SearchObject.m_Quant = 10;

        CString StartString = HttpIo.PostGet(Start);
        if (! StartString.GetLength()) StartString = HttpIo.GetGet(Start);
        if (!StartString.IsInt(&m_SearchObject.m_Start))
                if (!GetValue(Start).IsInt(&m_SearchObject.m_Start))
                        m_SearchObject.m_Start = 0;

	if (m_SearchObject.m_Start < 0) m_SearchObject.m_Start = 0;
	if ((m_SearchObject.m_Quant <= 0)&&(m_SearchObject.m_Quant != -1)) m_SearchObject.m_Quant = 10;

	HttpIo.PostSetNameValue(Quant, CString::IntToStr(m_SearchObject.m_Quant));
	HttpIo.PostSetNameValue(Start, CString::IntToStr(m_SearchObject.m_Start));

	if ((m_Searching)&&(!HttpIo.IsPost())) {
		HttpIo.PostSetNameValue(Search, HttpIo.GetGet(Search));
		TraversePost(HttpIo, Buffer);
	}
}

void CAlkalineSession::TraversePost(CHttpIo& HttpIo, const CString& /* Buffer */) {
	/* get all POST parameters and set them as variables */
    CString CurName, CurValue, CandidateValue;
    for (int k=0; k < (int) HttpIo.GetGetSize(); k++) {
        CurName = "post." + HttpIo.GetGetNameAt(k);
        CurName.LowerCase();
        CurValue = GetValue(CurName);
        if (CurValue.GetLength())
            CurValue += " ";
        CurValue += HttpIo.GetGetValueAt(k);
        Set(CurName, CurValue);
	}
	for (int j=0; j < (int) HttpIo.PostGetSize(); j++) {
        CurName = "post." + HttpIo.PostGetNameAt(j);
        CurName.LowerCase();                
        CurValue = GetValue(CurName);
        CandidateValue = HttpIo.PostGetValueAt(j);
        if (CurValue == CandidateValue)
            continue;
        if (CurValue.GetLength())
            CurValue += " ";
        CurValue += CandidateValue;
        Set(CurName, CurValue);
	}

    CString SearchConfig = HttpIo.GetGet("searchconfig");
    if (! SearchConfig.GetLength())
        SearchConfig = HttpIo.GetGetNameAt(0);
    
	SearchConfig.Replace('\\','/');
    SearchConfig.Trim('/');
   
    /* find the search template */
    for (int i=0; i < (int) m_AlkalineData->GetSitesList().GetSize(); i++) {        
      Trace(tagClient, levInfo, ("CAlkalineSession::Comparing - {%s}{%s}.", m_AlkalineData->GetSaneEnginesList()[i].GetBuffer(), SearchConfig.GetBuffer()));
      if (m_AlkalineData->GetSaneEnginesList()[i].Same(SearchConfig)) {
	CInterval Interval;
	/* URL replacements - Replace field in the asearch.cnf - must be valid BEFORE the search op */
	m_AlkalineData->GetSitesList()[i]->CopyReplaceTable(m_UrlReplacements);
	m_AlkalineData->GetSitesList()[i]->CopyRegexpReplaceTable(m_Regexp_UrlReplacements);
	m_SearchObject.m_SearchString = MakeSearchString(HttpIo);
	m_SearchObject.m_Parent = this;
	m_AlkalineData->GetSitesList()[i]->SearchSite(m_SearchObject);
	HttpIo.GetRFields().Set("Pragma", "nocache");
	m_SearchObject.m_HasTime = Interval.Get(itMilliseconds);
	return;
      }
    }
    ErrorPage("<h3>Alkaline Error</h3>Invalid parameter: " + HttpIo.GetGetNameAt(0), HttpIo);
}

#define ALKALINE_SESSION_APPEND_MACRO(_Vector, _Prefix) \
	for (i=0;i<(int)_Vector.GetSize();i++) \
		if (_Vector[i].GetLength() && (SearchString.SamePos(CString(_Prefix) + _Vector[i]) == -1)) { \
			SearchString += (CString(" ") + CString(_Prefix) + _Vector[i]); \
		}

void CAlkalineSession::MakeSearchField(const CString& FieldName, const CString& FieldValue) {

	CVector<CString> * CurTarget;

	static const CString _Host("HOST");
	static const CString _Path("PATH");
	static const CString _Url("URL");

	static const CString _Other("OTHER");
	static const CString _Before("BEFORE");
	static const CString _After("AFTER");

	static const CString _Meta("META");

	if (FieldName.Same(_Host)) CurTarget = &m_OptionHosts;
	else if (FieldName.Same(_Path)) CurTarget = &m_OptionPaths;
	else if (FieldName.Same(_Url)) CurTarget = &m_OptionUrls;
	else if (FieldName.Same(_Other)) CurTarget = &m_OptionOther;
	else if (FieldName.Same(_Before)) CurTarget = &m_OptionBefore;
	else if (FieldName.Same(_After)) CurTarget = &m_OptionAfter;
	else if (FieldName.Same(_Meta)) CurTarget = &m_OptionMeta;
	else return;

	CVector<CString> CurVector;
	CString::StrToVector(FieldValue, ',', &CurVector);

	for (int m=0;m<(int) CurVector.GetSize();m++) {
		Trace(tagClient, levInfo, ("CAlkalineSession::MakeSearchString - adding keyword [%s].", CurVector[m].GetBuffer()));
		CurTarget->AddSortedUnique(CurVector[m]);		
	}
}

CString CAlkalineSession::MakeSearchString(CHttpIo& HttpIo) {

	Trace(tagClient, levInfo, ("CAlkalineSession::MakeSearchString - post fields [%d].", HttpIo.PostGetSize()));

	for (int j=0;j<HttpIo.GetGetSize();j++) {
		MakeSearchField(HttpIo.GetGetNameAt(j), HttpIo.GetGetValueAt(j));
	}    

	for (int j=0;j<HttpIo.PostGetSize();j++) {
		MakeSearchField(HttpIo.PostGetNameAt(j), HttpIo.PostGetValueAt(j));
	}    

	CString SearchString(HttpIo.PostGet("SEARCH"));
	if (SearchString.GetLength()) {
		int i;
		ALKALINE_SESSION_APPEND_MACRO(m_OptionHosts, "host:");
		ALKALINE_SESSION_APPEND_MACRO(m_OptionPaths, "path:");
		ALKALINE_SESSION_APPEND_MACRO(m_OptionUrls, "url:");
		ALKALINE_SESSION_APPEND_MACRO(m_OptionOther, "");
		ALKALINE_SESSION_APPEND_MACRO(m_OptionBefore, "before:");
		ALKALINE_SESSION_APPEND_MACRO(m_OptionAfter, "after:");
        ALKALINE_SESSION_APPEND_MACRO(m_OptionMeta, "meta:");		

		HttpIo.PostSetNameValue("SEARCH", SearchString);
	}
    
	return SearchString;
}


CString& CAlkalineSession::MapTermEach(CString& Term, int Dummy) const {

    Trace(tagRender, levInfo, ("CAlkalineSession::MapTermEach - mapping [%s].", Term.GetBuffer()));

    static const CString _Query("QUERY");
	static const CString _Search("SEARCH");
	static const CString _Sort("SORT");
	static const CString _SortC("SORT:");
	static const CString _SortP("SORT.");

	static const CString _Host("HOST");
	static const CString _Path("PATH");
	static const CString _Url("URL");
	static const CString _Other("OTHER");
	static const CString _Before("BEFORE");
	static const CString _After("AFTER");

	static const CString _Total("TOTAL");
	static const CString _Size("SIZE");
	static const CString _Start("START");
	static const CString _FreeCharset("FREECHARSET");
	static const CString _Quant("QUANT");
	static const CString _Time("TIME");
	static const CString _End("END");

    static const CString _Meta("META");

	static const CString _RequestUrl("REQUESTURL");

    CString MidString;
 
	if (Term.Same(_Host)) CString::VectorToStr(m_OptionHosts,',', &Term);
	else if (Term.Same(_Path)) CString::VectorToStr(m_OptionPaths,',', &Term);
	else if (Term.Same(_Url)) CString::VectorToStr(m_OptionUrls,',', &Term);
	else if (Term.Same(_Other)) CString::VectorToStr(m_OptionOther,',', &Term);
	else if (Term.Same(_Before)) CString::VectorToStr(m_OptionBefore,',', &Term);
	else if (Term.Same(_After)) CString::VectorToStr(m_OptionAfter,',', &Term);
    else if (Term.Same(_Query)) {
        CString TermCopy = m_SearchObject.m_ParsedSearchString;
        if (! CString::StrToInt(GetValue(_FreeCharset))) {
            CHtmlParser::Quote(TermCopy, &Term);
        } else {
            CHtmlParser::QuoteQuotes(TermCopy, &Term);
        }
    }
    else if (Term.Same(_Search)) {
        CString TermCopy = m_HttpIoPointer->PostGet(_Search);
        if (! CString::StrToInt(GetValue(_FreeCharset))) {
            CHtmlParser::Quote(TermCopy, &Term);
        } else {
            CHtmlParser::QuoteQuotes(TermCopy, &Term);
        }        
    } else if (Term.StartsWithSame(_SortP) || (Term.Same(_Sort))) {
        CString SearchString = m_HttpIoPointer->PostGet(_Search);
		int sPos = SearchString.SamePos(_SortC);
		while (sPos != -1) {
			int j = sPos + _SortC.GetLength();
			if ((j < (int) SearchString.GetLength())&&(SearchString[j] == '.')) j++;
			else for (;j<(int) SearchString.GetLength();j++)
				if (!isalpha(SearchString[j])) break;
			SearchString.Delete(sPos, j - sPos);
			sPos = SearchString.SamePos(_SortC);
		}
		if (SearchString.GetLength() && SearchString[((int) SearchString.GetLength()) - 1] != ' ') SearchString += " ";
		SearchString += "sort:";
		if (!Term.Same(_SortP)) {
                  Term.Mid(_SortC.GetLength(), Term.GetLength(), &MidString);
                  SearchString+=MidString;
                }
		SearchString.Trim32();
		SearchString = CUrl::Escape(SearchString, true);
		Term = "/" + m_HttpIoPointer->GetGetNameAt(0) + '/' + m_HttpIoPointer->GetGetNameAt(1) + "?search=" + SearchString;
	} else if (Term.Same(_Total)) {
		Term = CString::IntToStr(m_SearchObject.m_HasNext);
	} else if (Term.Same(_Start)) {
		Term = CString::IntToStr(m_SearchObject.m_Start + 1);
	} else if (Term.Same(_Size)) {
		Term = CString::IntToStr(m_SearchObject.m_IndexSize);
	} else if (Term.Same(_End)) {
                if (m_SearchObject.m_Quant == -1)
                  Term = CString::IntToStr(m_SearchObject.m_HasNext);
                else {
                  int End = m_SearchObject.m_Start + m_SearchObject.m_Quant;
                  if (End > m_SearchObject.m_HasNext)
                    End = m_SearchObject.m_HasNext;
                  Term = CString::IntToStr(End);
                }
	} else if (Term.Same(_Time)) {
		Term.Empty();
		int Sec = m_SearchObject.m_HasTime / 1000;
		Term += CString::IntToStr(Sec);
		int mSec = m_SearchObject.m_HasTime - (Sec * 1000);
		if ((Sec == 0) && (mSec == 0))
			mSec = 1;
		CString mSecStr = CString::IntToStr(mSec);
		Term += ".";
		for (int m=(int) mSecStr.GetLength();m<3;m++) Term+="0";
		Term+=mSecStr;
	} else if (Term.Same(_Quant)) {
		if (m_SearchObject.m_Quant == -1) Term = CString::IntToStr(m_SearchObject.m_HasNext);
		else Term = CString::IntToStr(m_SearchObject.m_Quant);
    } else if (Term.Same(_Meta)) {
        CString::VectorToStr(m_OptionMeta,',', &Term);
	} else if (Term.Same(_RequestUrl)) {
		Term = GetValue("SEARCH-BASE-HREF") + "/" + m_HttpIoPointer->GetGetNameAt(0) + '/' + m_HttpIoPointer->GetGetNameAt(1);
    } else {
		Term = GetValue(Term);
	}
    
	return CSession::MapTermEach(Term, Dummy);
}

void CAlkalineSession::TraverseSSI(const CString SSIString, CHttpIo& HttpIo) {
  CSession::TraverseSSI(Map(SSIString), HttpIo);
}

