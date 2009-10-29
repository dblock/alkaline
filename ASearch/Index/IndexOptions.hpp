/*
  
  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  ______________________________________________
  
  written by Daniel Doubrovkine - dblock@vestris.com
  
*/

#ifndef ALKALINE_INDEX_OPTIONS_HPP
#define ALKALINE_INDEX_OPTIONS_HPP
    
#include <platform/include.hpp>
#include <String/String.hpp>
#include <Mutex/Atomic.hpp>

class CIndexOptions {
    friend class CIndex;
    friend class CSite;
private:
    bool                   m_GatherEmail;
    bool                   m_GatherEmailAll;
    CAtomic                m_ModifiedCount;          // number of modified files
    bool                   m_Modified;
    int                    m_RetryCount;
    bool                   m_FreeAlpha;
    bool                   m_CGI;
    bool                   m_SkipParseLinks;
    bool                   m_NSF;
    bool                   m_EmptyLinks;
    bool                   m_Insens;
    int                    m_Limit;
    int                    m_Timeout;
    bool                   m_Verbose;
    CString                m_Proxy;                  // proxy settings
    CVector<CString>       m_IndexingExts;
    CString                m_IndexFile;
    CString                m_IndexHTML;    
    CString                m_CurrentFilenameNdx;
    CString                m_CurrentFilenameInf;
    CString                m_CurrentFilenameUrt;
    CString                m_CurrentFilenameLnx;
public:
    CIndexOptions(void);
    ~CIndexOptions(void);
};


#endif
