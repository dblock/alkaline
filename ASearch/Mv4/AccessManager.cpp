/*
    © Vestris Inc., Geneva, Switzerland
    http://www.vestris.com, 1994-1999 All Rights Reserved
    _____________________________________________________

    written by Daniel Doubrovkine - dblock@vestris.com

*/

#include <alkaline.hpp>

#include "AccessManager.hpp"
#include <Encryption/Encryption.hpp>
#include <File/LocalFile.hpp>

bool CAccessManager::CheckPassword(const CString& Name, const CString& Password) const {
    return ((Password.GetLength())&&(GetValue(Name) == Password));
}

bool CAccessManager::CheckRootPassword(const CString& Password) const {	
    return ((Password.GetLength())&&(GetValue("ROOT") == Password));
}

CString CAccessManager::GetBuffer(void) const {
    CString Result;
    for (register int i=0;i < (int) CTSStringTable::GetSize(); i++) {
        Result += (CTSStringTable::GetNameAt(i) + " = " + CTSStringTable::GetValueAt(i) + "\n");
    }
    return Result;
}

CAccessManager::CAccessManager(void) : CTSStringTable() {    
    m_Filename = CLocalPath::GetCurrentDirectory() + "equiv/access.struct";
    //_L_DEBUG(6, cout << "CAccessManager::CAccessManager - loading " << m_Filename << endl);
    Read();
}

CAccessManager::CAccessManager(const CString& Filename) : CTSStringTable() {
    
    m_Filename = Filename;
    //_L_DEBUG(6, cout << "CAccessManager::CAccessManager - loading " << m_Filename << endl);
    Read();
}


CAccessManager::~CAccessManager(void) {
    
}

void CAccessManager::Read(void) {
    struct_stat NewStat;
    base_stat((const char *) m_Filename.GetBuffer(), &NewStat);        
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
        bool Recompile = false; 
        CString Line;
        for (register int i=0;i<(int) Lines.GetSize();i++) {
            Line = Lines[i];                        
            //_L_DEBUG(6, cout << "CAccessManager::Read() - parsing " << Line << endl);
            if (Line.GetLength() && (Line[0] != '#')) {
                CVector<CString> Tokens;
                CString::StrToVector(Line, ',', &Tokens);
                if ((Tokens.GetSize() == 2)&&(Tokens[0].GetLength())&&(Tokens[1].GetLength())) {
                    if (Tokens[1][0] == ' ') {
                        Recompile = true;
                        Tokens[1].Trim32();
                    } else Tokens[1] = CEncryption::Decrypt(Tokens[1]);
                    CTSStringTable::Set(Tokens[0], Tokens[1]);
                }
            }
        }
        if (Recompile) 
            Write();
    }
    m_LoadMutex.UnLock();
}

void CAccessManager::Write(void) {
    CString Target;
    for (int m = 0;m<(int) GetSize();m++) {
        Target += (GetNameAt(m) + ',' + CEncryption::Encrypt(GetValueAt(m)) + "\n");
    }
    CLocalFile File(m_Filename);
    if (File.Open(O_WRONLY | O_TRUNC | O_CREAT)) {
        File.Write(Target);
    }
}

bool CAccessManager::CheckAccess(const CString& Name, const CString& Password) const {
    if (!Password.GetLength()) return false;
    else if ((GetValue(Name) != Password) && (GetValue("ROOT") != Password)) return false;
    else return true;
}

bool CAccessManager::HasChanged(void) const {
    struct_stat NewStat; base_stat((const char *) m_Filename.GetBuffer(), &NewStat);
    return (m_Stat.st_mtime != NewStat.st_mtime);	
}

void CAccessManager::Set(const CString& Name, const CString& Value) {
    m_RWMutex.StartWriting();
    m_PersistentTable.Set(Name, Value);
    m_RWMutex.StopWriting();
    CTSStringTable::Set(Name, Value);
}

