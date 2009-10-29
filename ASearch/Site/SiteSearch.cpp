/*
    © Vestris Inc., Geneva, Switzerland
    http://www.vestris.com, 1994-1999 All Rights Reserved
    ______________________________________________

    written by Daniel Doubrovkine - dblock@vestris.com
  
    Revision history:
    
*/

#include <alkaline.hpp>

#include "Site.hpp"
#include <AlkalineSession/AlkalineSession.hpp>    
#include <Main/TraceTags.hpp>

bool CSite::SimpleSearch(CSearchObject& SearchObject) {
    
    Trace(tagSearch, levVerbose, ("CSite::Search - entering."));

    CString SearchLogString;
	CString ResultsLogString;
	CString ParsedLogString;
	CString CurrentTime; 

	time_t l_Time; 
	time(&l_Time);
	CDate::ctime_r(&l_Time, CurrentTime);

	SearchLogString += "rawsearch\tfacility=site\talias=";
	SearchLogString += m_Alias;
	SearchLogString += "\trawquery=";
	SearchLogString += SearchObject.m_SearchString;
	SearchLogString += "\tpage=";
	SearchLogString += CString::IntToStr((int)(SearchObject.m_Start / SearchObject.m_Quant)+1);
	SearchLogString += "\ttime=";
	SearchLogString += CurrentTime;
    
    for (int i=0;i<(int) m_SiteConfigs.GetSize();i++) {
        WriteLog(m_SiteConfigs[i].GetLogFile(), SearchLogString, * SearchObject.m_Parent);
    }

    if (m_ShowRequests) {
        cout << "time=" << CurrentTime << "\tmessage=" << SearchLogString << endl;
        cout.flush();
    }
    
    Trace(tagSearch, levVerbose, ("CSite::Search - ParseSearchString."));
    
    SearchObject.ParseSearchString(m_ShowRequests);

    Trace(tagSearch, levVerbose, ("CSite::Search - SiteIndex::Search."));
    
    bool Result = m_SiteIndex.Search(SearchObject);

	time(&l_Time);
	CDate::ctime_r(&l_Time, CurrentTime);

	ParsedLogString = "parsedsearch\tfacility=site\talias=";
	ParsedLogString += m_Alias;
	ParsedLogString += "\tquery=";
	ParsedLogString += SearchObject.m_ParsedSearchString;
	ParsedLogString += "\ttime=";
	ParsedLogString += CurrentTime;
	
	ResultsLogString += "rawsearch\tfacility=site\talias=";
	ResultsLogString += m_Alias;
	ResultsLogString += "\tresultset=";
	ResultsLogString += CString::IntToStr(SearchObject.m_HasNext);
	ResultsLogString += "\tshown=";
	ResultsLogString += CString::IntToStr(SearchObject.m_SearchResults.GetSize());
	ResultsLogString += "\ttime=";
	ResultsLogString += CurrentTime;

    for (int i=0;i<(int) m_SiteConfigs.GetSize();i++) {
        WriteLog(m_SiteConfigs[i].GetLogFile(), ParsedLogString, * SearchObject.m_Parent);
        WriteLog(m_SiteConfigs[i].GetLogFile(), ResultsLogString, * SearchObject.m_Parent);
    }

	if (m_ShowRequests) {
        time_t l_Time; 
		time(&l_Time);
        CString CurrentTime; 
        CDate::ctime_r(&l_Time, CurrentTime);
        cout << "time=" << CurrentTime << "\tmessage=" << ResultsLogString << endl;
        cout.flush();
	}
    
    Trace(tagSearch, levVerbose, ("CSite::Search - returning."));
    
    return Result;
}

bool CSite::SearchSite(CSearchObject& SearchObject) {
    LoadSiteIndex();
    return SimpleSearch(SearchObject);
}
