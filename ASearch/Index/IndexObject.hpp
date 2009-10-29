/*
    © Vestris Inc., Geneva, Switzerland
    http://www.vestris.com, 1994-1999 All Rights Reserved
    ______________________________________________

    written by Daniel Doubrovkine - dblock@vestris.com

    Revision history:

*/

#ifndef ALKALINE_INDEX_OBJECT_HPP
#define ALKALINE_INDEX_OBJECT_HPP
    
#include <platform/include.hpp>
#include <String/String.hpp>
#include <Vector/Vector.hpp>
#include <Internet/Url.hpp>

class CSite;

typedef enum { iotUrl, iotRobot } CIndexObjectType;
    
class CIndexObject : public CObject {
  property(CIndexObjectType, IndexObjectType);
  property(CString, Url);
  property(int, Depth);
  property(bool, Lazy);
  property(CVector<CUrl>, RemoteHistory);  
  property(CString, SearchTop);
  copy_property(CSite *, pSite);
public:
  CIndexObject(void);
  ~CIndexObject(void);
  CIndexObject(const CIndexObject&);
  CIndexObject& operator=(const CIndexObject&);
  void Execute(void);
};

#endif
