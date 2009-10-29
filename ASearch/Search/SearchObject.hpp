/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  ______________________________________________

  written by Daniel Doubrovkine - dblock@vestris.com

  search object that contains all relevant information
  on the current search operation, results, etc.
    
*/

#ifndef ALKALINE_C_SEARCH_OBJECT_HPP
#define ALKALINE_C_SEARCH_OBJECT_HPP

#include <platform/include.hpp>
#include <Search/SearchTypes.hpp>

class CSearchObject {
    
    friend class CAlkalineSession;
    friend class CSite;
    friend class CIndex;

private:

    CSite * m_Site;                         // parent site
    CAlkalineSession * m_Parent;            // parent session      
    CString m_SearchString;                 // string being searched
    CString m_ParsedSearchString;           // parsed search string
    CVector<CString> m_DigestedSearchTerms; // digested search terms (lowercase, no quotes or markup)
    CVector<CString> m_Adjustments;         // search adjustments vector
    CSearchOptions m_SearchOptions;         // search options strucutre
    CString m_SortType;                     // results sort type    
    int m_Quant;                            // quantity of results
    int m_Start;                            // beginning of output
    int m_HasNext;                          // number of next results (after current)
    int m_HasTime;                          // time for total processing
    long m_IndexSize;                       // total size of the index searched
    CVector<CString> m_SearchResults;       // mapped results for output        
    CSearchData m_SearchData;                // search data structure    

public:
    
    CSearchObject(void);
    ~CSearchObject(void);
    
    void ParseSearchString(bool bVerbose);
    
    bool AddToVector(
        const CString& Candidate, 
        CVector<CString>& Target, 
        CVector<CString>& AdjustVector, 
        CString&);
};

#endif
