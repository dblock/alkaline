/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  ______________________________________________
  
  written by Daniel Doubrovkine - dblock@vestris.com
  
*/

#ifndef ALKALINE_C_STRING_A_HPP
#define ALKALINE_C_STRING_A_HPP

#include <platform/include.hpp>
#include <String/String.hpp>

class CStringA :  public CString {
	property(short int, TagPos);    
public:
	inline bool operator>(const CStringA& String) const { return ((Compare(String) > 0)?true:false); }
	inline bool operator<(const CStringA& String) const { return ((Compare(String) < 0)?true:false); }
	inline bool operator>=(const CStringA& String) const { return ((Compare(String) >= 0)?true:false); }
	inline bool operator<=(const CStringA& String) const { return ((Compare(String) <= 0)?true:false); }
	inline bool operator==(const CStringA& String) const { return ((Compare(String) == 0)?true:false); }
    inline unsigned int GetFakeSize(void) { return m_TagPos ? (GetLength() - m_TagPos - 1) : GetLength(); }
private:
	inline int Compare(const CStringA&) const;
public:
	CStringA(const char * Buffer);
	CStringA(void);
	CStringA(const CStringA&);
	CStringA(const CString&);
    inline virtual void operator=(const CStringA& String);
	bool IsEqualMeta(const CString&, bool bRegExp = false) const;
    inline void SetTagPos(void);
};
    
inline void CStringA::SetTagPos(void) { 
    if ((m_TagPos = Pos(':')) == -1) 
        m_TagPos = 0; 
}

inline void CStringA::operator=(const CStringA& String) {
    m_TagPos = String.m_TagPos;
    CString::operator=(String);
}

inline int CStringA::Compare(const CStringA& String) const {
	if (this == &String) 
        return 0;
	else if (GetLength() < String.GetLength()) return -1;
	else if (GetLength() > String.GetLength()) return 1;
	else for (register int i = 0; i < (int) GetLength(); i++) {
		if (GetAt(i) > String.GetAt(i)) return 1;
		else if (GetAt(i) < String.GetAt(i)) return -1;
	}
	return 0;
}

#endif
