/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  _____________________________________________________
  
  written by Daniel Doubrovkine - dblock@vestris.com
  
  Alkaline sites wrapper, part of the Alkaline Search Engine
  manages a list of CSite-s, drives reindexing and (re)loads
  equiv containers

*/

#ifndef ALKALINE_C_ALKALINE_DATA
#define ALKALINE_C_ALKALINE_DATA

#include <platform/include.hpp>
#include <Thread/Thread.hpp>
#include <String/String.hpp>
#include <List/List.hpp>
#include <Io/Io.hpp>
#include <Site/Site.hpp>

class CAlkalineServer;

class CAlkalineData : public CThread {
    property(CVector<CString>, EnginesList);
    property(CVector<CString>, SaneEnginesList);
    property(CList<CSite *>, SitesList);
    property(int, GlobalIndex);
    property(CVector<CString>, Options);
    property(CString, Root);
    property(bool, Terminated);
    copy_property(CAlkalineServer *, ParentServer);
    mutable CRWMutex m_RWMutex;
public:
    CAlkalineData(const CString&, CAlkalineServer *);
    virtual ~CAlkalineData(void);
    bool ReloadEnginesList(void);
    void Reload(void);
    virtual void Execute(void * Arguments);
    void SetOptions(const CString& Options);
    void Load(void);
    bool CanTerminate(void) const;
    bool ConfigHasChanged(void) const;
    inline void StartWriting(void) const { m_RWMutex.StartWriting(); }
    inline void StopWriting(void) const { m_RWMutex.StopWriting(); }
    inline void StartReading(void) const { m_RWMutex.StartReading(); }
    inline void StopReading(void) const { m_RWMutex.StopReading(); }
    void GetConfigXml(CString& Target) const;	
	void GetConfigWml(CString& Target) const;
	void GetConfigWmlSearch(CString& Target) const;
    void Stop(void);
};

#endif
