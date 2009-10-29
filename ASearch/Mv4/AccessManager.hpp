/*
    © Vestris Inc., Geneva, Switzerland
    http://www.vestris.com, 1994-1999 All Rights Reserved
    ______________________________________________

    written by Daniel Doubrovkine - dblock@vestris.com

*/

#ifndef ALKALINE_ACCESS_MANAGER_HPP
#define ALKALINE_ACCESS_MANAGER_HPP

#include <platform/include.hpp>
#include <String/TSStringTable.hpp>
#include <Io/Io.hpp>
#include <Mutex/RWMutex.hpp>

class CAccessManager : public CTSStringTable {
    property(CString, Filename);
    protected_property(struct_stat, Stat);
    property(CStringTable, PersistentTable);
    property(CMutex, LoadMutex);
public:	
    CAccessManager(void);
    CAccessManager(const CString&);
    virtual ~CAccessManager(void);
    bool CheckRootPassword(const CString& Password) const;
    bool CheckPassword(const CString& Name, const CString& Password) const;    
    bool CheckAccess(const CString& Name, const CString& Password) const;
    CString GetBuffer(void) const;
    bool HasChanged(void) const;	
    void Set(const CString& Name, const CString& Value);
private:
    void Read(void);
    void Write(void);
};

#endif





