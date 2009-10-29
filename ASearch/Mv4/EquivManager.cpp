/*
    © Vestris Inc., Geneva, Switzerland
    http://www.vestris.com, 1994-1999 All Rights Reserved
    _____________________________________________________

    written by Daniel Doubrovkine - dblock@vestris.com

*/

#include <alkaline.hpp>

#include "EquivManager.hpp"
#include <Encryption/Encryption.hpp>
#include <File/LocalFile.hpp>

CString CEquivManager::GetBuffer(void) const {
    CString Result;
    for (register int i=0;i < (int) CTSStringTable::GetSize(); i++) {
        Result += (CTSStringTable::GetNameAt(i) + " = " + CTSStringTable::GetValueAt(i) + "\n");
    }
    return Result;
}

CEquivManager::CEquivManager(const CString& Class, const CString& Filename) : CTSStringTable() {	
    m_Class = Class;
    if (Filename.GetLength()) m_Filename = Filename;
    else m_Filename = CLocalPath::GetCurrentDirectory() + "equiv/equiv.struct";
    //// _L_DEBUG(6, cout << "CEquivManager::CEquivManager - loading " << m_Filename << endl);
    Read();
}

CEquivManager::~CEquivManager(void) {
    
}

void CEquivManager::Read(void) {
    struct_stat NewStat; base_stat((const char *) m_Filename.GetBuffer(), &NewStat);
    if (GetSize() && (m_Stat.st_mtime == NewStat.st_mtime)) 
        return;
    
    m_LoadMutex.Lock();
    m_Stat = NewStat;
    RemoveAll();
    (* (CTSStringTable *) this) = m_PersistentTable;
    CLocalFile File(m_Filename);
    if (File.OpenReadBinary()) {
        CVector<CString> Lines;
        File.ReadLines(&Lines);		
        CString Line;
        for (register int i=0;i<(int) Lines.GetSize();i++) {
            Line = Lines[i];
            //// _L_DEBUG(6, cout << "CEquivManager::Read() - parsing " << Line << endl);
            if (!Line.GetLength()) continue;
            if (Line[0] == '#') {
                Line.Delete(0, 1);
                if ((Line.Pos('<')>=0)&&(Line.Pos('>')> Line.Pos('<'))) m_EmailVector += Line;
            } else {
                CVector<CString> Tokens; CString::StrToVector(Line, ',', &Tokens);
                if ((Tokens.GetSize() == 2)&&(Tokens[0].GetLength())&&(Tokens[1].GetLength())) {
                    // complex equivalences
                    CVector<CString> TokensLeft; CString::StrToVector(Tokens[0], '+', &TokensLeft);
                    CVector<CString> TokensRight; CString::StrToVector(Tokens[1], '+', &TokensRight);
                    if ((int) TokensLeft.GetSize() > 1) {
                        if (TokensLeft.GetSize() > TokensRight.GetSize()) {
                            cout << "Dropping malformed equivalence at " << Tokens[0] << "=" << Tokens[1] << endl;
                        } else {
                            for (int ii=1;ii<(int)TokensLeft.GetSize();ii++) {
                                CString TL(TokensLeft[0]); TL+=TokensLeft[ii];
                                CString TR(TokensRight[0]); TR+=TokensRight[ii];
                                CTSStringTable::Set(TL, TR);
                            }
                        }
                    } else {
                        CTSStringTable::Set(Tokens[0], Tokens[1]);
                    }
                }
            }
        }
    }
    m_LoadMutex.UnLock();
}

bool CEquivManager::HasChanged(void) const {
    struct_stat NewStat; base_stat((const char *) m_Filename.GetBuffer(), &NewStat);
    return (m_Stat.st_mtime != NewStat.st_mtime);
}

void CEquivManager::Set(const CString& Name, const CString& Value) {
    m_RWMutex.StartWriting();
    m_PersistentTable.Set(Name, Value);
    m_RWMutex.StopWriting();
    CTSStringTable::Set(Name, Value);
}
