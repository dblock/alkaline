/*
    © Vestris Inc., Geneva, Switzerland
    http://www.vestris.com, 1994-1999 All Rights Reserved
    ______________________________________________

    written by Daniel Doubrovkine - dblock@vestris.com

    Revision history:

*/

#include <alkaline.hpp>
#include <Site/Site.hpp>
#include "IndexObject.hpp"
    
    
CIndexObject::CIndexObject(void) {
  m_IndexObjectType = iotUrl;
  m_pSite = NULL;
  m_Depth = 0;
  m_Lazy = false;  
}

CIndexObject::~CIndexObject(void) {
  
}

CIndexObject::CIndexObject(const CIndexObject& IndexObject) {
  operator=(IndexObject);
}

CIndexObject& CIndexObject::operator=(const CIndexObject& IndexObject) {
  m_Url = IndexObject.m_Url;
  m_Depth = IndexObject.m_Depth;
  m_Lazy = IndexObject.m_Lazy;
  m_SearchTop = IndexObject.m_SearchTop;
  m_RemoteHistory = IndexObject.m_RemoteHistory;
  m_pSite = IndexObject.m_pSite;
  m_IndexObjectType = IndexObject.m_IndexObjectType;
  return * this;
}

void CIndexObject::Execute(void) {
  // _L_DEBUG(3, cout << "CIndexObject :: Indexing [" << m_Url << "] (" << m_Depth << "/" << m_Lazy << "/" << m_SearchTop << ")" << endl);
  
  if (m_IndexObjectType == iotUrl) {
      m_pSite->ProcessURL(* this);
  } else if (m_IndexObjectType == iotRobot) {
      CVector<CString> RobotsVector;
      m_pSite->RetrieveRobots(m_Url, RobotsVector, m_Lazy);
  }  
  
  // _L_DEBUG(3, cout << "CIndexObject :: Done [" << m_Url << "]" << endl);
}
