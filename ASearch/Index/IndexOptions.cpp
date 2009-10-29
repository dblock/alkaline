/*
  
  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  ______________________________________________
  
  written by Daniel Doubrovkine - dblock@vestris.com
  
*/

#include <alkaline.hpp>
#include "IndexOptions.hpp"
    
CIndexOptions :: CIndexOptions(void) :
    m_GatherEmail(false),
    m_GatherEmailAll(false),
    m_ModifiedCount(0),
    m_Modified(false),
    m_RetryCount(3),
    m_FreeAlpha(false),
    m_CGI(false),
    m_SkipParseLinks(false),
    m_NSF(false),
    m_EmptyLinks(true),
    m_Insens(false),
    m_Limit(1000000),
    m_Timeout(10),
    m_Verbose(false)
{
    
}

CIndexOptions :: ~CIndexOptions(void) {
    
}
