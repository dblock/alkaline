/*

    © Vestris Inc., Geneva, Switzerland
    http://www.vestris.com, 1994-1999 All Rights Reserved
    _____________________________________________________
  
    written by Daniel Doubrovkine - dblock@vestris.com
    
*/

#include <alkaline.hpp>

#include "AdminManager.hpp"
#include <Encryption/Encryption.hpp>
#include <File/LocalFile.hpp>

CString CAdminManager::GetBuffer(void) const {
    CString Result;
    m_RWMutex.StartReading();
    
    for (register int i=0;i < (int) m_AdminTable.GetSize(); i++) {
        Result += (m_AdminTable.GetKeyAt(i) + ':');
        for (register int j=0; j < (int) m_AdminTable[i].GetSize(); j++)
            Result += (' ' + m_AdminTable.GetElementAt(i).GetNameAt(j));
        Result += "\n";
    }
    m_RWMutex.StopReading();
    return Result;
}

CAdminManager::CAdminManager(void) {    
    m_Filename = CLocalPath::GetCurrentDirectory() + "equiv/admin.struct";
    //_L_DEBUG(6, cout << "CAdminManager::CAdminManager - loading " << m_Filename << endl);
    Read();
}

CAdminManager::CAdminManager(const CString& Filename) {    
    m_Filename = Filename;
    //_L_DEBUG(6, cout << "CAdminManager::CAdminManager - loading " << m_Filename << endl);
    Read();
}


CAdminManager::~CAdminManager(void) {
    
}

void CAdminManager::Read(void) {
    struct_stat NewStat; base_stat((const char *) m_Filename.GetBuffer(), &NewStat);
    if (m_AdminTable.GetSize() && (m_Stat.st_mtime == NewStat.st_mtime)) 
        return;
    
    m_RWMutex.StartWriting();
    m_Stat = NewStat;
    m_AdminTable.RemoveAll();
    m_AdminTable = m_PersistentTable;
    CLocalFile File(m_Filename);
    if (File.OpenReadBinary()) {
        CVector<CString> Lines;
        File.ReadLines(&Lines);		
        CString Line;
        for (register int i=0;i<(int) Lines.GetSize();i++) {
            Line = Lines[i];
            //_L_DEBUG(6, cout << "CAdminManager::Read() - parsing " << Line << endl);
            if (Line.GetLength() && (Line[0] != '#')) {
                CVector<CString> Tokens;
                CString::StrToVector(Line, ',', &Tokens);
                if ((Tokens.GetSize() == 2)&&(Tokens[0].GetLength())&&(Tokens[1].GetLength())) {
                    CVector<CString> InsideTokens;
                    CString::StrToVector(Tokens[1], '+', &InsideTokens);
                    CStringTable Table = m_AdminTable.FindElement(Tokens[0]);
                    for (register int l=0;l<(int)InsideTokens.GetSize();l++)
                        Table.Set(InsideTokens[l], Tokens[0]);
                    m_AdminTable.Set(Tokens[0], Table);
                }
            }
        }
    }
    m_RWMutex.StopWriting();
}

bool CAdminManager::CheckAdmin(const CString& Name, const CString& Class) const {
    bool bResult = false;
    m_RWMutex.StartReading();
    if ((!Class.GetLength())||(!Name.GetLength())) 
        bResult = false;
    else if (m_AdminTable.FindElement(Name).GetValue(Class).GetLength()) 
        bResult = true;
    else bResult = false;
    
    m_RWMutex.StopReading();
    return bResult;
}

bool CAdminManager::HasChanged(void) const {
    struct_stat NewStat; 
    base_stat((const char *) m_Filename.GetBuffer(), &NewStat);
    return (m_Stat.st_mtime != NewStat.st_mtime);
}

CString CAdminManager::Get(const CString& Name, const CString& Class) const { 
    CString Result;
    m_RWMutex.StartReading();    
    Result = m_AdminTable.FindElement(Name).GetValue(Class); 
    m_RWMutex.StopReading();
    return Result;
}

void CAdminManager::Set(const CString& NValue, const CString& Class) {
    CStringTable Table;
    Table.Set(NValue, Class);
    m_RWMutex.StartWriting();
    m_AdminTable.Set(NValue, Table); 
    m_PersistentTable.Set(NValue, Table); 
    m_Stat.st_mtime = -1; // force a changed
    m_RWMutex.StopWriting();
}
