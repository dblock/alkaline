/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  ______________________________________________

  written by Daniel Doubrovkine - dblock@vestris.com

*/

#ifndef ALKALINE_C_SEARCH_TYPES_HPP
#define ALKALINE_C_SEARCH_TYPES_HPP

#include <platform/include.hpp>
#include <String/String.hpp>
#include <Vector/Vector.hpp>
#include <Date/Date.hpp>

/* 
   search type 
*/
typedef enum CSearchType {
	st_Exact,                            // exact, case-insensitive 
	st_InsensExact,                      // exact, case-sensitive
	st_Partial,                          // substring, case-insensitive
	st_InsensPartial                     // substring, case-sensitive
} CSearchType;

/*
   types of case-sensitive/insensitive search
*/
typedef enum {
    sct_CaseAutomatic,
    sct_CaseSensitive,
    sct_CaseInsensitive
} CSearchCaseType;
    
/* 
   data necessary to perform a search operation
   and return results 
*/
typedef struct _CSearchData {
	CSearchCaseType m_SearchCaseType;
	int m_SearchedWords;
	CVector<CString> m_Words;	     // keywords searched
	CVector<short int> m_ResultsQuality;       // quality vector
	CVector<int> m_Results;              // results vector
	CVector<int> m_ResultsPositions;     // original positions (sorted by relevance)
	short int m_MaxPossibleQuality;
	short int m_QualityThreshold;
} CSearchData;

/*
  results sorting types
*/
typedef enum {
	CSTNone,                             // no sorting
	CSTDate,                             // sort by date
	CSTTitle,                            // sort by title
	CSTUrl,                              // sort by URL
	CSTSize,                             // sort by size
	CSTSizeInverse,                      // sort by size (inverted)
	CSTDateInverse                       // sort by date (inverted)
} CSortType;

class CAlkalineSession;

/*
  structure passed to the results mapping function for
  each result shown
*/
typedef struct _CIndexMapV {
	CVector<CString> m_HashInfo;          // document indexed information [last-modified][content-length][date][title][header][server]
	CString m_Recent;                     // recent
	CString m_Expired;                    // expired
	CString m_RecentCount;                // recent count
	CString m_Url;                        // document url	
	int m_Index;                          // output relative index
	CDate m_ModifDateObject;              // modified
	CDate m_DateObject;                   // created
    struct tm m_tmDate;                   // encoded date
	CString m_DateMap;                    // map string to use for date
	bool m_ForceQuote;                    // force results quoting
	int m_ResultPosition;                 // real position (per relevance)
	CSearchData * m_SearchData;           // search data object
	CAlkalineSession * m_Parent;          // parent session   
} CIndexMapV;

/*
  structure used to specify paths and scope restrictions
  while constructing search keywords and sorting results
*/
typedef struct _CSearchOptions {
	CVector<CString> m_OptionHosts;       // host:
	CVector<CString> m_OptionPaths;       // path:
	CVector<CString> m_OptionUrls;        // url:
	CVector<CString> m_OptionExts;        // ext:
	CVector<CString> m_OptionExtra;       // appended by routine
    CVector<CString> m_OptionMeta;        // forced meta tags
	bool m_DateAfterValid;
	bool m_DateBeforeValid;
	struct tm m_DateAfter;                // after:
	struct tm m_DateBefore;               // before:
    bool m_USFormats;                     // MMDD vs. DDMM dates
	
	bool m_OptAnd;                        // opt:and
	bool m_OptWhole;                      // opt:whole
	bool m_OptCase;                       // opt:case
	bool m_OptInsens;                     // opt:insens

} CSearchOptions;

#endif
