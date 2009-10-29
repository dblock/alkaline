/*
    © Vestris Inc., Geneva, Switzerland
    http://www.vestris.com, 1994-1999 All Rights Reserved
    ______________________________________________

    written by Daniel Doubrovkine - dblock@vestris.com

*/

#ifndef ALKALINE_EQUIV_MANAGER_HPP
#define ALKALINE_EQUIV_MANAGER_HPP

#include <platform/include.hpp>
#include <String/TSStringTable.hpp>
#include <Io/Io.hpp>

class CEquivManager : public CTSStringTable {
    property(CString, Filename);
    property(CString, Class);
    property(CVector<CString>, EmailVector);
    protected_property(struct_stat, Stat);
    property(CStringTable, PersistentTable);
    property(CMutex, LoadMutex);
public:
    CEquivManager(const CString& Class = CString::EmptyCString, const CString& Filename = CString::EmptyCString);
    virtual ~CEquivManager(void);
    CString GetBuffer(void) const;
    void Read(void);
    bool HasChanged(void) const;
    void Set(const CString& Name, const CString& Value);
};

#endif





