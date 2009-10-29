/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  ______________________________________________
  
  written by Daniel Doubrovkine - dblock@vestris.com

*/

#include <alkaline.hpp>
#include <RegExp/RegExp.hpp>
#include "StringA.hpp"

CStringA::CStringA(const char * Buffer) : CString() {
    CString::operator=(Buffer); 
    SetTagPos();
}

CStringA::CStringA(const CString& String) : CString() {
    CString::operator=(String); 
    SetTagPos();
}

CStringA::CStringA(void) : CString() {
    SetTagPos();
}

CStringA::CStringA(const CStringA& String) : CString() {
    CString::operator=(String);
    m_TagPos = String.m_TagPos;    
}

bool CStringA::IsEqualMeta(const CString& String, bool bRegExp) const {
    if (bRegExp) {
        int m_TagDisp = (m_TagPos)?(m_TagPos+1):0;
        return (CRegExp::Match(GetBuffer() + m_TagDisp, GetLength() - m_TagDisp, String.GetBuffer()));
    } else {    
        if (m_TagPos == 0) return (CString::operator==(String));
        else {
            if ((((int)GetLength()) - m_TagPos - 1) != (int) String.GetLength()) return false;
            else for (register int i=m_TagPos+1;(i<(((int)GetLength()))) && ((i-m_TagPos) <= (int) String.GetLength()); i++)
                if (operator[](i) != String[i-m_TagPos-1]) return false;
                return true;
        }
    }
}

