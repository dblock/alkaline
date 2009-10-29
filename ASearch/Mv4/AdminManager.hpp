/*
    © Vestris Inc., Geneva, Switzerland
    http://www.vestris.com, 1994-1999 All Rights Reserved
    ______________________________________________

    written by Daniel Doubrovkine - dblock@vestris.com

*/

#ifndef ALKALINE_ADMIN_MANAGER_HPP
#define ALKALINE_ADMIN_MANAGER_HPP

#include <platform/include.hpp>
#include <HashTable/VectorTable.hpp>
#include <String/StringTable.hpp>
#include <Io/Io.hpp>

class CAdminManager : public CObject {    
    property(CString, Filename);
    property(CVectorTable<CStringTable>, AdminTable);
    property(CVectorTable<CStringTable>, PersistentTable);
    protected_property(struct_stat, Stat);
    mutable CRWMutex m_RWMutex;
public:    
    CAdminManager(void);
    CAdminManager(const CString&);
    virtual ~CAdminManager(void);        
    bool CheckAdmin(const CString& Name, const CString& Class) const;
    CString GetBuffer(void) const;
    CString Get(const CString& Name, const CString& Class) const;
    void Set(const CString& NValue, const CString& Class);
    bool HasChanged(void) const;
    void Read(void);
};

#endif
