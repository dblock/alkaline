/*
	© Vestris Inc., Geneva, Switzerland
	http://www.vestris.com, 1994-1999 All Rights Reserved
	______________________________________________

    written by Daniel Doubrovkine - dblock@vestris.com

	*/

#ifndef ALKALINE_CERTIFIED_SERVER_HPP
#define ALKALINE_CERTIFIED_SERVER_HPP

#include <platform/include.hpp>
#include <Server/WebServer.hpp>
#include <Mv4/EquivManager.hpp>
#include <Mv4/AdminManager.hpp>
#include <Mv4/AccessManager.hpp>
#include <Session/Certif.hpp>

class CCertifiedServer : public CWebServer {
	mutable_property(CMutex, LogMutex);
	property(CString, PortString);
	property(CString, EquivString);
	property(CString, AdminString);
	property(CEquivManager, EquivManager);
	property(CAdminManager, AdminManager);
	property(CAccessManager, AccessManager);	
    property(CCertif, Certificate);
    property(CString, StartupDirectory);
    property(CString, Realm);
    property(CString, ErrorFooter);
    property(CLocalFile, LogStream);
public:
    CCertifiedServer(unsigned int Port, const CString& Address, const CString& EquivString, const CString& AdminString);
    virtual ~CCertifiedServer(void);
    virtual void WriteLog(const CString& String) const;
    virtual bool EnsurePassword(const CString& ValidList, CHttpIo& HttpIo) const;
    void SetLogPath(const CString& LogPath);
private:
//    void ProcessPostCertif(CHttpIo&) const;
protected:
    static CString m_ServerResponseXml;
    virtual void ErrorPage(const CString& Error, CHttpIo&) const;
    virtual bool ProcessGet(CHttpIo&) const;
    virtual bool ProcessPost(CHttpIo&) const;
};

#endif
