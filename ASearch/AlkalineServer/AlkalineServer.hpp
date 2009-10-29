/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  _____________________________________________________
  
  written by Daniel Doubrovkine - dblock@vestris.com
  
  Alkaline Server, part of the Alkaline Search Engine
  binds to port, processes command line options, creates
  index/search classes, processes online management

*/

#ifndef ALKALINE_SERVER_HPP
#define ALKALINE_SERVER_HPP

#include <platform/include.hpp>
#include <Server/WebServer.hpp>
#include <Mv4/EquivManager.hpp>
#include <Mv4/AdminManager.hpp>
#include <Mv4/AccessManager.hpp>
#include <Session/Certif.hpp>
#include <HashTable/HashTable.hpp>
#include <Mutex/Mutex.hpp>
#include <Mutex/RWMutex.hpp>
#include <AlkalineSession/AlkalineSession.hpp>
#include <Session/CertifiedServer.hpp>
#include <AlkalineData/AlkalineData.hpp>
#include <Date/Date.hpp>
#include <AlkalineServer/PingThread.hpp>
#include <Config/GlobalCnf.hpp>

class CAlkalineServer : public CCertifiedServer {
    copy_property(CAlkalineData *, AlkalineData);
	property(bool, Reindex);
    property(bool, Daemon);
    property(CHashTable<CString>, ContainersTable);
    property(CVector<CString>, Options);
    property(CString, StartupDateString);
    property(CString, ServerStartupDateString);
    property(CString, ServerAdminPath);
    property(CString, ServerStartupDirectory);
    property(CString, ServerHttpHeader);
    property(bool, SSIEnabled);
	readonly_property(CString, PingAddress);
    copy_property(CAlkalinePingThread *, PingThread);
    property(CGlobalCnf, GlobalCnf);        
#ifdef _UNIX
    mutable_property(bool, ForceTerminate);    
#endif
    mutable_property(bool, ForceShutdown);
    mutable CRWMutex m_ExecuteMutex;
public:
    virtual void Bind(void);
    virtual void UnBind(void);
    static CDate m_StartupDate;
    static CAtomic m_ServerRestartCount;
    CDate m_ServerStartupDate;
    CAlkalineServer(unsigned int Port, const CString& Address, const CString& EquivString, const CString& AdminString);
    virtual ~CAlkalineServer(void);
    virtual void Launch(void);
    void Stop(void);
private:
    void SendASFile(const CString& Filename, CHttpIo& HttpIo);
    void Initialize(void);
    bool IndexAddRemoveProcess(const CString& Action, CHttpIo& HttpIo) const;
    bool RetrieveTemplatePage(CHttpIo& HttpIo, const CString& TemplatePage, CString&, bool bForce = false);
    bool RetrieveTemplate(CHttpIo& HttpIo, const CString& TemplatePage, CString& TemplateContents);
    bool IndexRefreshProcess(const CString& Action, CHttpIo& HttpIo) const;
    bool IndexReloadProcess(const CString& Action, CHttpIo& HttpIo) const;
    
    void ExecuteWebClient(CHttpIo& HttpIo);
    void ExecuteNormal(CHttpIo&);
    bool RestartServerProcess(const CString& Action, CHttpIo& HttpIo) const;
    bool ShutdownServerProcess(const CString& Action, CHttpIo& HttpIo) const;
    virtual bool ProcessGet(CHttpIo&) const;
    virtual bool ProcessPost(CHttpIo&) const;            
private:
    CString& MapTerm(const CString& Source, CString& Target, int) const;
    CString Map(const CString&, CHttpIo& HttpIo) const;
    virtual CString& MapTermEach(CString&, int) const;
	bool AnalyzePath(const CString& LocalFile) const;
};

#endif
