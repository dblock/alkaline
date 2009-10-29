/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  _____________________________________________________
  
  written by Daniel Doubrovkine - dblock@vestris.com
  
*/

#include <alkaline.hpp>

#include "CertifiedServer.hpp"
#include <File/LocalFile.hpp>
#include <Date/Date.hpp>

CString CCertifiedServer::m_ServerResponseXml("ServerResponseXml");

CCertifiedServer::CCertifiedServer(unsigned int Port, const CString& Address, const CString& EquivString, const CString& AdminString) : CWebServer(Port, Address) {
  if (Address.GetLength())
    m_PortString = Address + ":";
  m_PortString += CString::IntToStr(Port);
  m_EquivString = EquivString;
  m_AdminString = AdminString;
  m_StartupDirectory = CLocalPath::GetCurrentDirectory();
  m_Realm = "Alkaline Certified Server";
  m_ErrorFooter = "<hr size=1><font size=\"1px\"><a href=http://www.vestris.com/alkaline target=_new>Alkaline Search Engine</a> &copy; 1994-2001 Vestris Inc., All Rights Reserved</font>";  
}

CCertifiedServer::~CCertifiedServer(void) {
	WriteLog("server stopped\tdetail=server object reference released\tfacility=server\tcategory=server\ttype=information");    
}

void CCertifiedServer::SetLogPath(const CString& LogPath) {
    if (LogPath.GetLength()) {
        CString LogFilename = LogPath;    
        CLocalPath::Terminate(LogFilename);
        LogFilename += ("asearch-" + m_PortString + ".log");
        m_LogStream.SetFilename(LogFilename);
        m_LogStream.Open(O_WRONLY | O_APPEND | O_CREAT);
    }
}

void CCertifiedServer::WriteLog(const CString& String) const {
    if (m_LogStream.IsOpened()) {        

		CString CurrentTime;
        time_t l_Time; time(&l_Time);        
        CDate::ctime_r(&l_Time, CurrentTime);

		CString LogString; 
        LogString += "time=";
		LogString += CurrentTime;
		LogString += "\tmessage=";
        LogString += String;

        m_LogMutex.Lock();
        m_LogStream.WriteLine(LogString);
        m_LogMutex.UnLock();

    }
}

bool CCertifiedServer::ProcessGet(CHttpIo& /* HttpIo */) const {
    return true;
}

bool CCertifiedServer::ProcessPost(CHttpIo& HttpIo) const {
    static const CString PostAction("ACTION");
//    static const CString CertifGen("CERTIFGEN");
    if (!HttpIo.IsPost()) 
        return false;
    CString Action = HttpIo.PostGet(PostAction);
//    if (Action.Same(CertifGen)) 
//        ProcessPostCertif(HttpIo);
    return true;
}

/*
void CCertifiedServer::ProcessPostCertif(CHttpIo& HttpIo) const {
    if (((CCertifiedServer&) (* this)).m_Certificate.WriteCertificate(HttpIo.PostGet("CERTIFICATE"))) {
		WriteLog("certificate validated\tdetail=the certification key submitted has been accepted\tfacility=server\tcategory=certification\ttype=information");        
        HttpIo.GetRFields().Set(m_ServerResponseXml, "<response><error>0</error><message>Valid certificate submitted, binary unlocked.</message></response>");
    } else {
        WriteLog("certificate not validated\tdetail=the certification key submitted has not been accepted\tfacility=server\tcategory=certification\ttype=error");        
        HttpIo.GetRFields().Set(m_ServerResponseXml, "<response><error>1</error><message>Invalid certificate submitted.</message></response>");
    }
}
*/

void CCertifiedServer::ErrorPage(const CString& Error, CHttpIo& HttpIo) const {        
    HttpIo.Write(
        "<html>\n<head><title>Server Error</title></head>\n"                                                                                                            \
        "<body bgcolor=\"#FFFFFF\"><table cellpadding=2 width=400><tr><td>"                                                                                                      \
        "<font face=\"Arial, Helvetica, Sans-Serif\" size=\"2px\">\n"                                                                                                            \
        + Error +                                                                                                                                                                \
        "</font><hr size=1></td></tr><tr><td>\n"                                                                                                                                 \
        "<font face=\"Arial, Helvetica, Sans-Serif\" size=\"2px\">"                                                                                                              \
        "The page you are looking for is currently unavailable. "                                                                                                                \
        "The server might be experiencing technical difficulties, "                                                                                                              \
        "you have made an incorrect query, or you may need to adjust your browser settings. "                                                                                    \
        "This can also be caused by a mistake in the server configuration." +
	m_ErrorFooter +
        "</font></td></tr></table></body>\n"                                                                                                                                     \
        "</html>\n");
}

bool CCertifiedServer::EnsurePassword(const CString& PasswordsList, CHttpIo& HttpIo) const {
//    static const CString VestrisUsername("VESTRIS");
    CString Username, Password; 
    HttpIo.GetUsernamePassword(Username, Password);
    CVector<CString> Passwords;
    CString::StrToVector(PasswordsList, ',', &Passwords);
//    if (Username.Same(VestrisUsername) && (m_Certificate.IsValidCertificate(Password)))
  //    return true;
    for (register int i=0;i<(int)Passwords.GetSize();i++) {
        if (Passwords[i].Same(Username) && m_AccessManager.CheckPassword(Passwords[i], Password)) {
            return true;
        }
    }
//	WriteLog("certificate validated\tdetail=the certification key submitted has been accepted\tfacility=server\tcategory=certification\ttype=information");        
    WriteLog("Invalid password (" + Password + ") submitted for " + Username);
    HttpIo.DenyAccess(m_Realm);
    return false;
}
