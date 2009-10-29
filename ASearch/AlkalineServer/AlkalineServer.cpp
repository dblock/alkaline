/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  _____________________________________________________
  
  written by Daniel Doubrovkine - dblock@vestris.com

*/

#include <alkaline.hpp>

#include "AlkalineServer.hpp"
#include <File/RemoteFile.hpp>
#include <AlkalineParser/AlkalineParser.hpp>
#include <AlkalineServer/AdderThread.hpp>
#include <String/GStrings.hpp>
#include <Tree/XmlTree.hpp>
#include <Main/bldver.hpp>
#include <Virtual/VirtualMemory.hpp>
#include <Main/TraceTags.hpp>

CDate CAlkalineServer::m_StartupDate;
CAtomic CAlkalineServer::m_ServerRestartCount;

CAlkalineServer::CAlkalineServer(unsigned int Port, const CString& Address, const CString& EquivString, const CString& AdminString) :
    CCertifiedServer(Port, Address, EquivString, AdminString)  
{
    Initialize();
  
    // initialize ping address, defaults to localhost:port, can come from global.cnf
    m_PingAddress = m_GlobalCnf.GetOption("PingUrl");
    if (! m_PingAddress.GetLength()) {
        if (Address.GetLength()) {
            m_PingAddress = "http://" + Address + ":" + CString::IntToStr(Port) + "/";
        } else {
            m_PingAddress = "http://localhost:" + CString::IntToStr(Port) + "/";  
        }
    }
    
    m_AlkalineData = new CAlkalineData(EquivString, this);
}

void CAlkalineServer::Initialize(void) {
	m_PingThread = NULL;
	m_Reindex = true;
	m_SSIEnabled = false; // server side includes
	m_Daemon = false;
#ifdef _UNIX
	m_ForceTerminate = false;
#endif
    m_ForceShutdown = false;
	m_StartupDateString = CAlkalineServer::m_StartupDate.AscTime();
    m_ServerStartupDateString = m_ServerStartupDate.AscTime();
    m_ServerRestartCount.Inc();
    
	//m_Certificate.SetCategory("asearch");
	//m_Certificate.SetExtension(".alk");
	//m_Certificate.SetScriptName("/");
	//m_Certificate.SetCertifyUrl("http://www.vestris.com/alkaline/certify.html");
    //m_Certificate.Certify();
        
    m_GlobalCnf.Read(this, true);

	if (m_AccessManager.GetSize() == 0) {
		cout << "[warning, you should define at least a root password in global.cnf]" << endl;
	}

    // ensure a relative admin path
   
    m_ServerAdminPath = m_GlobalCnf.GetOption("AdminPath");
    
   #ifdef _WIN32    
    if (CLocalPath :: IsDrive(m_ServerAdminPath)) {
        Trace(tagClient, levError, ("CAlkalineServer::ExecuteWebClient - admin path is not relative: %s.", m_ServerAdminPath.GetBuffer()));    
        m_ServerAdminPath.Empty();
    }
   #endif
    
    if (m_ServerAdminPath.GetLength() && ((m_ServerAdminPath[0] == '/') || (m_ServerAdminPath[0] == '\\') || (m_ServerAdminPath[0] == '.'))) {
        Trace(tagClient, levError, ("CAlkalineServer::ExecuteWebClient - admin path is not relative: %s.", m_ServerAdminPath.GetBuffer()));    
        m_ServerAdminPath.Empty();
    }
    
    if (! m_ServerAdminPath.GetLength())
        m_ServerAdminPath = "admin";
    
    // ensure a startup path
    
    m_ServerStartupDirectory = CLocalPath :: GetCurrentDirectory();
    
    m_ServerHttpHeader = 
        CString(RC_VERSION_PRODUCT_NAME) + "/" + 
        CString::IntToStr(RC_VERSION_MAJOR) + '.' + 
        CString::IntToStr(RC_VERSION_MINOR);
    
    CCertifiedServer :: SetLogPath(m_EquivManager.GetValue("LogPath"));

    CServer :: SetNagle(m_GlobalCnf.GetNagle());
    
	CString Realm = m_GlobalCnf.GetOption("Realm");
	if (Realm.GetLength())
        m_Realm = Realm;
	
	CString ErrorFooter = m_GlobalCnf.GetOption("ErrorFooter");
	if (ErrorFooter.GetLength())
        m_ErrorFooter = ErrorFooter;
	
    WriteLog("server initializing\tfacility=server\tcategeory=server\ttype=information\tversion=" + CString(RC_VERSION_STRING)  + "\tport=" + m_PortString);

	cout << "[Alkaline server (" << __DATE__  << ") running, binding to " << m_PortString << "]" << endl;
}
 
CAlkalineServer::~CAlkalineServer(void) {
	WriteLog("server shutdown\tfacility=server\tcategeory=server\ttype=information\tversion=" + CString(RC_VERSION_STRING)  + "\tport=" + m_PortString);  
}

bool CAlkalineServer::RetrieveTemplate(
    CHttpIo& HttpIo,
    const CString& TemplatePage, 
    CString& TemplateContents) {
    
    m_EquivManager.Read();    
    if (TemplatePage.StartsWithSame("http://")) {      
        CRemoteFile RemoteFile(TemplatePage);      
        CString Proxy;
		for (int i = 0; i < (int) m_GlobalCnf.GetForwardAlnHeaders().GetSize(); i++) {
			CString HeaderName = m_GlobalCnf.GetForwardAlnHeaders()[i];
			CStringTable HeaderValue;
			if (HttpIo.GetHttpFields().FindAndCopy(HeaderName, HeaderValue)) {
				RemoteFile.SetParameter(HeaderName, HeaderValue.GetValue(HeaderName));
			}
		}
        if (m_EquivManager.FindAndCopy("PROXY", Proxy)) 
            RemoteFile.Get(Proxy);
        else RemoteFile.Get();        
        if (RemoteFile.GetRStatusValue() == 200) {
            TemplateContents = RemoteFile.GetRData();            
            if (m_GlobalCnf.GetCacheTemplates()) {
                m_ContainersTable.Set(TemplatePage, TemplateContents);
            }            
            return true;
        } else {
            TemplateContents = RemoteFile.GetHttpRequest().GetRStatusString();
            return false;
        }        
    } else {

        CString FqTemplatePage = m_ServerStartupDirectory + TemplatePage;
#ifdef _UNIX    
        struct_stat FileStat;
        
        if (!CLocalFile::GetFileStat(FqTemplatePage, FileStat)) {
            HttpIo.SetRStatus(404);
            return false;
        }
    
        if (!(FileStat.st_mode & S_IROTH)) {
            HttpIo.SetRStatus(401);
            return false;
        }
#endif
        
        CLocalFile LocalFile(FqTemplatePage);      
        if (LocalFile.GetSize() && LocalFile.OpenReadBinary()) {
            LocalFile.Read(& TemplateContents);
            if (TemplateContents.GetLength()) {
                if (m_GlobalCnf.GetCacheTemplates()) {
                    m_ContainersTable.Set(TemplatePage, TemplateContents);
                }
                return true;
            }
        }
        
        TemplateContents = LocalFile.GetStringError();
        return false;
    }
}

bool CAlkalineServer::RetrieveTemplatePage(
    CHttpIo& HttpIo,
    const CString& TemplatePage, 
    CString& TemplateContents, 
    bool bForce) {
    
    Trace(tagClient, levInfo, ("CAlkalineServer::RetrieveTemplatePage - %s.", TemplatePage.GetBuffer()));
    
    bool bResult = true;
    if (bForce || (!m_GlobalCnf.GetCacheTemplates()) || HttpIo.GetGetFields().FindName("nocache")) {
        m_ExecuteMutex.StartWriting();
        bResult = RetrieveTemplate(HttpIo, TemplatePage, TemplateContents);
        m_ExecuteMutex.StopWriting();
    } else {
        m_ExecuteMutex.StartReading();
        bResult = m_ContainersTable.FindAndCopy(TemplatePage, TemplateContents);
        m_ExecuteMutex.StopReading();
        if (!bResult) {
            m_ExecuteMutex.StartWriting();
            bResult = m_ContainersTable.FindAndCopy(TemplatePage, TemplateContents);
            if (!bResult)
                bResult = RetrieveTemplate(HttpIo, TemplatePage, TemplateContents);
            m_ExecuteMutex.StopWriting();
        }
    }

    Trace(tagClient, levInfo, ("CAlkalineServer::RetrieveTemplatePage - %s (%s).", TemplatePage.GetBuffer(), bResult?"ok":"failed"));
    return bResult;
}

void CAlkalineServer::ExecuteNormal(CHttpIo& HttpIo) {
    
    CString GroupAlias = m_EquivManager.GetValue(HttpIo.GetGetNameAt(0));
    CString SearchConfig = HttpIo.GetGet("searchconfig");
    
    Trace(tagClient, levInfo, ("CAlkalineServer::ExecuteNormal - groupalias=%s, searchconfig=%s.", GroupAlias.GetBuffer(), SearchConfig.GetBuffer()));
    
    if ((SearchConfig.GetLength() == 0) && (GroupAlias.GetLength() == 0)) {
        WriteLog("invalid parameter\tdetail=alias " + HttpIo.GetGetNameAt(0) + " not defined in equiv.struct\tfacility=server\tcategory=client\ttype=error\tclient=" + HttpIo.GetConnection()->GetSocket().GetRemoteHost());
        ErrorPage("<h3>Invalid parameter: " + HttpIo.GetGetNameAt(0) + "</h3>This alias is not defined or the above configuration is not loaded.", HttpIo);        
        return;
    }

    CString TemplatePage = m_EquivManager.GetValue(HttpIo.GetGetNameAt(1));

    if (TemplatePage.GetLength() == 0) {
        
        // the template page is in the data directory        
        TemplatePage = HttpIo.GetGetNameAt(0) + '/' + HttpIo.GetGetNameAt(1);        
        TemplatePage.Trim();

#ifdef _WIN32
        TemplatePage.Replace('/','\\');
        if ((TemplatePage.GetLength() >= 2) && (TemplatePage[1] == ':')) {
			WriteLog("invalid parameter\tdetail=" + HttpIo.GetGetNameAt(1) + " contains drive letters\tfacility=server\tcategory=security\ttype=error\tclient=" + HttpIo.GetConnection()->GetSocket().GetRemoteHost());
            ErrorPage("<h3>Invalid parameter: " + HttpIo.GetGetNameAt(1) + "</h3>Paths cannot contain drive letters. This is a server security policy restriction.", HttpIo);
            return;
        }
#endif
#ifdef _UNIX
        TemplatePage.Replace('\\','/');
#endif
        
        if (TemplatePage.GetLength() && ((TemplatePage[0] == '\\') || (TemplatePage[0] == '/') || (TemplatePage.Pos(g_strPathParent) != -1))) {
			WriteLog("invalid parameter\tdetail=" + HttpIo.GetGetNameAt(1) + " is absolute\tfacility=server\tcategory=security\ttype=error\tclient=" + HttpIo.GetConnection()->GetSocket().GetRemoteHost());            
            ErrorPage("<h3>Invalid parameter: " + HttpIo.GetGetNameAt(1) + "</h3>Paths cannot be absolute. This is a server security policy restriction.", HttpIo);
            return;
        }
        
        if (TemplatePage.EndsWithSame(".aln")) {
            // link to an http:// location
            CLocalFile LnFile(TemplatePage);
            LnFile.OpenReadBinary();
            if (LnFile.GetSize() > 5 * KBYTE) {
				WriteLog("invalid parameter\tdetail=" + HttpIo.GetGetNameAt(1) + " points to an .aln file exceeding size limit, remote link not followed\tfacility=server\tcategory=security\ttype=error\tclient=" + HttpIo.GetConnection()->GetSocket().GetRemoteHost());                
                ErrorPage("<h3>Invalid parameter: " + HttpIo.GetGetNameAt(1) + "</h3>The .aln file is too large and the remote link will not be followed. This is a server security policy restriction.", HttpIo);
                return;
            }
            LnFile.ReadLine(&TemplatePage);
            TemplatePage.Trim32();
        }
    }
    
    if (TemplatePage.GetLength() == 0) {
		WriteLog("invalid parameter\tdetail=" + HttpIo.GetGetNameAt(1) + " is not defined in equiv.struct, aln file empty or cannot be read\tfacility=server\tcategory=client\ttype=error\tclient=" + HttpIo.GetConnection()->GetSocket().GetRemoteHost());        
        ErrorPage("<h3>Invalid parameter: " + HttpIo.GetGetNameAt(1) + "</h3>The alias is not defined or the .aln file is empty or could not be read.", HttpIo);
        return;
    }
    
    CString TemplateContents;
    if (!RetrieveTemplatePage(HttpIo, TemplatePage, TemplateContents)) {
        if (HttpIo.GetRStatus() == 200) {
			WriteLog("error retrieving template\tdetail=request for " + TemplatePage + " returned an invalid response: " + TemplateContents + "\tfacility=server\tcategory=client\ttype=error\tclient=" + HttpIo.GetConnection()->GetSocket().GetRemoteHost());            
            ErrorPage("<h3>Unable to retrieve the template:</h3>" + TemplatePage + "<br>" + TemplateContents, HttpIo);
        } else {
            HttpIo.Flush();
        }
        return;
    }
    
    CAlkalineSession * pSession = new CAlkalineSession(m_AlkalineData);
    pSession->SetSSIEnabled(m_SSIEnabled);
    pSession->SetUSFormats(m_GlobalCnf.GetUSFormats());
    pSession->SetProxy(m_EquivManager.GetValue("PROXY"));
    pSession->Execute(HttpIo, TemplateContents);	
    delete pSession;
}

void CAlkalineServer::SendASFile(const CString& Filename, CHttpIo& HttpIo) {

#ifdef _UNIX
    struct_stat FileStat;

    if (!CLocalFile::GetFileStat(Filename, FileStat)) {
        HttpIo.SetRStatus(404);
        return;
    }
    
    if (!(FileStat.st_mode & S_IROTH)) {
        HttpIo.SetRStatus(401);
        return;
    }    
#endif
    
    CLocalFile File(Filename);
    if (!File.OpenReadBinary()) {
        HttpIo.SetRStatus(400);
        return;
    }
    
    CString MapContents;
    File.Read(&MapContents);
    if (!MapContents.GetLength()) {
        HttpIo.SetRStatus(400);
        return;
    }
    
    HttpIo.SendFileContents(Filename, Map(MapContents, HttpIo));
}

bool CAlkalineServer::AnalyzePath(const CString& LocalFile) const {
	// check whether the resulting filename is under the server startup directory
	if (! LocalFile.StartsWith(m_ServerStartupDirectory)) {
        return false;
	}

	int nSeparator = LocalFile.InvPos(PATH_SEPARATOR);
	if (nSeparator < 0) {
		return false;
	}

	CString LocalFileDirectory;
	LocalFile.Left(nSeparator + 1, & LocalFileDirectory);
	
	if (LocalFileDirectory.Same(m_ServerStartupDirectory)) {
		return false;
	}

	return true;
}

void CAlkalineServer::ExecuteWebClient(CHttpIo& HttpIo) {

    CString PathAdmin(m_ServerAdminPath);
    PathAdmin.TerminateWith('/');
    
    Trace(tagClient, levInfo, ("CAlkalineServer::ExecuteWebClient - admin path: %s.", PathAdmin.GetBuffer()));

    static const CString AsProcessPrefix("as_");
    static const CString AsProcessHttpServer("Server");    

    HttpIo.SetErrorFooter(m_ErrorFooter);    
    HttpIo.SetKeepAlive(m_GlobalCnf.GetKeepAlive());
    
    // do not log localhost ping
    if (!HttpIo.GetConnection()->GetSocket().IsPeerLocalHost() || (HttpIo.GetGetSize() >= 2)) {
		WriteLog("accepted request\tfacility=server\tcategory=client\ttype=information\tclient=" + 
			HttpIo.GetConnection()->GetSocket().GetRemoteHost() + 
			"\turl=" + HttpIo.GetRequestUrl().GetBrute() +
			"\tuser-agent=" + HttpIo.GetHttpFields().FindElement(g_strHttpUserAgent).GetValue(g_strHttpUserAgent));        
    }
    
    HttpIo.GetRFields().Set(AsProcessHttpServer, m_ServerHttpHeader);

    CString HttpDirectory = CUrl::UnEscape(HttpIo.GetRequestUrl().GetHttpDirectory());
    CString HttpFilename = CUrl::UnEscape(HttpIo.GetRequestUrl().GetHttpFilename());

    HttpDirectory.Trim('/');
    HttpDirectory.Trim('\\');

	Trace(tagClient, levInfo, ("CAlkalineServer::ExecuteWebClient - http directory: %s, http filename: %s.", HttpDirectory.GetBuffer(), HttpFilename.GetBuffer()));

    if (HttpDirectory.GetLength() && HttpFilename.GetLength()) {
        HttpIo.GetGetFields().InsertAt(0, HttpDirectory, HttpDirectory);
        HttpIo.GetGetFields().InsertAt(1, HttpFilename, HttpFilename);
    }

    if ((HttpIo.GetGetSize() < 2) && (!HttpDirectory.GetLength()) && (!HttpFilename.GetLength())) {
        HttpIo.SetRStatus(301);
        HttpIo.GetRFields().Set("Location", m_GlobalCnf.GetOption("Redirect"));
        return;
    }
    
    if (!HttpDirectory.GetLength() && 
        (HttpFilename.Same(m_ServerAdminPath) || CLocalPath::DirectoryExists(HttpFilename))) {
        HttpIo.SetRStatus(301);
        HttpIo.GetRFields().Set(g_strHttpLocation, HttpFilename + '/');
        return;
    }

	// set response content-type
	if (! HttpIo.SetRContentType(HttpFilename))
		HttpIo.SetRContentType(HttpIo.GetDefaultDocument());
    
    // allow search within the admin section with a SearchConfig parameter
    CString SearchConfig = HttpIo.GetGet("searchconfig");
    
    if (!SearchConfig.GetLength() && 
        (HttpDirectory.Same(m_ServerAdminPath) || HttpDirectory.StartsWithSame(PathAdmin))) {

        Trace(tagClient, levInfo, ("CAlkalineServer::ExecuteWebClient - admin path."));

        // if (!CLocalPath::DirectoryExists(m_ServerStartupDirectory + HttpDirectory)) {
        //    ErrorPage("<h3>The " + m_ServerAdminPath + " directory does not exist.</h3>It is requred for the online administration. This is a server configuration error.", HttpIo);
        //    return;
        // }

		// fall-through when no passwords are defined
		if (m_AccessManager.GetSize() != 0) {
			if (!EnsurePassword("root,alkaline-add,alkaline-manage", HttpIo)) {
				return;
			}
		}

        ProcessPost(HttpIo);
        if (!HttpFilename.GetLength())
            HttpFilename = HttpIo.GetDefaultDocument();
            
		CString LocalFile = CLocalPath::ResolveDirectory(m_ServerStartupDirectory, HttpDirectory + PATH_SEPARATOR + HttpFilename);
		LocalFile.Replace('/', PATH_SEPARATOR);

		if (! AnalyzePath(LocalFile)) {
			HttpIo.SetRStatus(401);
			return;
		}

		if (!CLocalFile::FileExists(LocalFile)) {
            HttpIo.SetRStatus(404);
            return;
        } else if (HttpFilename.StartsWithSame(AsProcessPrefix)) {
            SendASFile(LocalFile, HttpIo);
        } else {
            HttpIo.Send(LocalFile);
        }
	} else if (m_GlobalCnf.IsDocumentPath(HttpDirectory)) {

        Trace(tagClient, levInfo, ("CAlkalineServer::ExecuteWebClient - document path."));

		if (!HttpFilename.GetLength())
            HttpFilename = HttpIo.GetDefaultDocument();
            
        CString LocalFile = CLocalPath::ResolveDirectory(m_ServerStartupDirectory, HttpDirectory + PATH_SEPARATOR + HttpFilename);
		LocalFile.Replace('/', PATH_SEPARATOR);

		if (! AnalyzePath(LocalFile)) {
			HttpIo.SetRStatus(401);
			return;
		}

        if (!CLocalFile::FileExists(LocalFile)) {
            HttpIo.SetRStatus(404);
            return;
        } else {
            HttpIo.Send(LocalFile);
        }        	
    } else if ((HttpIo.GetGetSize() >= 2) || (SearchConfig.GetLength())) {

        Trace(tagClient, levInfo, ("CAlkalineServer::ExecuteWebClient - search."));

        ExecuteNormal(HttpIo);
        //if (!m_Certificate.GetCertified())
          //  m_Certificate.CertifyFail(HttpIo);
    } else {

		Trace(tagClient, levInfo, ("CAlkalineServer::ExecuteWebClient - omitted path."));

        HttpIo.SetRStatus(301);
        HttpIo.GetRFields().Set(g_strHttpLocation, HttpFilename + '/');
        return;
    }
#ifdef _UNIX
    if (m_ForceTerminate) {
        HttpIo.Flush();
		HttpIo.GetConnection()->GetSocket().Close();
        CHandler :: Terminate(0);
    }    
#endif
    if (m_ForceShutdown || g_pHandler->GetSignalSigterm()) {
        HttpIo.Flush();
		HttpIo.GetConnection()->GetSocket().Close();
        CServer :: Shutdown();
    }    
}

bool CAlkalineServer::ProcessGet(CHttpIo& HttpIo) const {
    return CCertifiedServer::ProcessGet(HttpIo);
}

bool CAlkalineServer::RestartServerProcess(const CString& Action, CHttpIo& HttpIo) const {
    static const CString ActionRestart("RESTART");
  
    if (!Action.Same(ActionRestart))
        return false;

    if (!EnsurePassword("root,alkaline-restart", HttpIo))
        return true;

#ifdef _UNIX
    if (!m_AlkalineData->CanTerminate()) {
		WriteLog("error restarting server\tdetail=writing data\tfacility=server\tcategory=server\ttype=error\tclient=" + 
			HttpIo.GetConnection()->GetSocket().GetRemoteHost());
        HttpIo.GetRFields().Set(m_ServerResponseXml, "<response><error>1</error><message>Unable to restart server, writing data. Please try again later.</message></response>");
        return true;
    }
  
	WriteLog("restarting server\tdetail=request from console\tfacility=server\tcategory=server\ttype=information\tclient=" + 
		HttpIo.GetConnection()->GetSocket().GetRemoteHost());
    HttpIo.GetRFields().Set(m_ServerResponseXml, "<response><error>0</error><message>Server is now being restarted.</message></response>");  
    m_ForceTerminate = true;
#endif
#ifdef _WIN32
	WriteLog("cannot restart server\tdetail=unsupported on this platform\tfacility=server\tcategory=server\ttype=information\tclient=" + 
		HttpIo.GetConnection()->GetSocket().GetRemoteHost());
    HttpIo.GetRFields().Set(m_ServerResponseXml, "<response><error>1</error><message>Restarting server is unsupported on this platform.</message></response>");  
#endif
	return true;
}

bool CAlkalineServer::ShutdownServerProcess(const CString& Action, CHttpIo& HttpIo) const {
    static const CString ActionRestart("SHUTDOWN");
  
    if (!Action.Same(ActionRestart))
        return false;

    if (!EnsurePassword("root,alkaline-restart", HttpIo))
        return true;
 
    if (!m_AlkalineData->CanTerminate()) {
		WriteLog("error shutting server down\tdetail=writing data\tfacility=server\tcategory=server\ttype=error\tclient=" + 
			HttpIo.GetConnection()->GetSocket().GetRemoteHost());
        HttpIo.GetRFields().Set(m_ServerResponseXml, "<response><error>1</error><message>Unable to shutdown server, writing data. Please try again later.</message></response>");
        return true;
    }
  
	WriteLog("shutting down server\tdetail=request from console\tfacility=server\tcategory=server\ttype=information\tclient=" + 
		HttpIo.GetConnection()->GetSocket().GetRemoteHost());
    HttpIo.GetRFields().Set(m_ServerResponseXml, "<response><error>0</error><message>Server is now being shut down.</message></response>");  
    m_ForceShutdown = true;
    return true;
}
    
bool CAlkalineServer::IndexReloadProcess(const CString& Action, CHttpIo& HttpIo) const {
    static const CString ActionReload("RELOADINDEX");
    static const CString ActionConfig("CONFIG");

    if (!Action.Same(ActionReload))
        return false;

    CString ConfigPath = HttpIo.PostGet(ActionConfig);
    CString ReloadedIndexes;
    
    m_ExecuteMutex.StartReading();
    for (int i=(int)(m_AlkalineData->GetSitesList().GetSize())-1;i>=0;i--) {
        if ((!ConfigPath.GetLength()) || (m_AlkalineData->GetSitesList()[i]->GetConfigPath() == ConfigPath)) {
            if (ReloadedIndexes.GetLength())
                ReloadedIndexes += ", ";
            ReloadedIndexes += m_AlkalineData->GetSitesList()[i]->GetConfigPath();
			WriteLog("reloading configuration\tdetail=" + ConfigPath + "\tfacility=server\tcategory=server\ttype=information\tclient=" + 
				HttpIo.GetConnection()->GetSocket().GetRemoteHost());
            m_AlkalineData->GetSitesList()[i]->ReloadSiteIndex();
        }
    }
    m_ExecuteMutex.StopReading();
    
    if (ReloadedIndexes.GetLength()) {
        HttpIo.GetRFields().Set(
            m_ServerResponseXml, 
            "<response><error>0</error><message>Reloaded index(es): " + ReloadedIndexes + "</message></response>");
    } else {
        HttpIo.GetRFields().Set(
            m_ServerResponseXml, 
            "<response><error>1</error><message>No indexes reloaded. Check whether the configuration path is valid.</message></response>");        
    }
    
    return true;  
}

bool CAlkalineServer::IndexRefreshProcess(const CString& Action, CHttpIo& HttpIo) const {
    static const CString ActionRefresh("REFRESH");
    static const CString ActionTemplate("TEMPLATE");
    if (!Action.Same(ActionRefresh))
        return false;
    CString TemplatePage = HttpIo.PostGet(ActionTemplate);    
    CString Data;
    if (((CAlkalineServer&) * this).RetrieveTemplatePage(HttpIo, TemplatePage, Data, true)) {
        HttpIo.GetRFields().Set(m_ServerResponseXml, "<response><error>0</error><message>Successfully reloaded " + TemplatePage + ".</message></response>");  
    } else {
        HttpIo.GetRFields().Set(m_ServerResponseXml, "<response><error>1</error><message>Error reloading " + TemplatePage + ".</message></response>");
    }
    return true;
}

bool CAlkalineServer::IndexAddRemoveProcess(const CString& Action, CHttpIo& HttpIo) const {
    static const CString IndexAdd("INDEXADD");
    static const CString IndexDel("INDEXDEL");  
    static const CString IndexUrl("URL");
    static const CString IndexAlias("ALIAS");
  
    bool bIndexAdd = false;
    bool bIndexDel = false;

    if (Action.Same(IndexAdd))
        bIndexAdd = true;
    else if (Action.Same(IndexDel))
        bIndexDel = true;

    if (!bIndexAdd && !bIndexDel)
        return false;

    if (!EnsurePassword("root,alkaline-add", HttpIo))
        return true;

    CString Url = HttpIo.PostGet(IndexUrl);
    CUrl UrlClass(Url);
    if (!UrlClass.GetValid()) {
        HttpIo.GetRFields().Set(m_ServerResponseXml, "<response><error>1</error><message>Invalid url: " + Url + ".</message></response>");  
        return false;
    }
    CString Alias = HttpIo.PostGet(IndexAlias);
#ifdef _UNIX
    Alias.Replace('\\','/');
#endif
    bool bResult = false;
    m_ExecuteMutex.StartReading();
    for (int i=(int)(m_AlkalineData->GetSitesList().GetSize())-1;i>=0;i--) {
        if (m_AlkalineData->GetSitesList()[i]->GetConfigPath() == Alias) {
            CAdderThread * AdderThread = ::new CAdderThread;
            AdderThread->SetSite(m_AlkalineData->GetSitesList()[i]);
            AdderThread->SetUrl(Url);
            if (bIndexAdd) {
                AdderThread->SetAdderOperation(AtAdd);
				WriteLog("scheduling url for reindexing\tdetail=" + Url + "\tfacility=server\tcategory=indexing\ttype=information\tclient=" + 
					HttpIo.GetConnection()->GetSocket().GetRemoteHost());                
                HttpIo.GetRFields().Set(m_ServerResponseXml, "<response><error>0</error><message>The url " + Url + " has been scheduled for reindex.</message></response>");
            } else if (bIndexDel) {
				WriteLog("scheduling url for removal\tdetail=" + Url + "\tfacility=server\tcategory=indexing\ttype=information\tclient=" + 
					HttpIo.GetConnection()->GetSocket().GetRemoteHost());
                AdderThread->SetAdderOperation(AtRemove);
                HttpIo.GetRFields().Set(m_ServerResponseXml, "<response><error>0</error><message>The url " + Url + " has been scheduled for removal.</message></response>");
            }
            AdderThread->Launch();
            bResult = true;
            break;
        }
    }
    m_ExecuteMutex.StopReading();
    if (!bResult)
        HttpIo.GetRFields().Set(m_ServerResponseXml, "<response><error>1</error><message>The alias " + Alias + " cannot be resolved.</message></response>");
    return false;
}

bool CAlkalineServer::ProcessPost(CHttpIo& HttpIo) const {
    static const CString PostAction("ACTION");
    CString Action = HttpIo.PostGet(PostAction);

    IndexRefreshProcess(Action, HttpIo);
    IndexReloadProcess(Action, HttpIo);

    RestartServerProcess(Action, HttpIo);
    ShutdownServerProcess(Action, HttpIo);
    IndexAddRemoveProcess(Action, HttpIo);
    return CCertifiedServer::ProcessPost(HttpIo);
}

void CAlkalineServer::Launch(void) {
    m_AlkalineData->SetOptions(m_Options);
    m_AlkalineData->Load();

    Bind();
  
    int BindRetry = 5;
    while(! g_pHandler->GetSignalSigterm()) {
        
        if (GetBound())
            break;
    
		WriteLog("error binding server\tdetail=retrying " + CString::IntToStr(BindRetry) + 
			"\tport=" + CString(inet_ntoa((const in_addr&)GetAddr())) + ":" + CString::IntToStr(ntohs(GetPort())) + 
			"\tfacility=server\tcategory=server\ttype=error");                

        cerr << "Error! Unable to bind Alkaline server to " <<  inet_ntoa((const in_addr&)GetAddr()) << ":"  << ntohs(GetPort()) << ", retrying " << BindRetry-- << " ... " << endl;
    
        if (!BindRetry || g_pHandler->GetSignalSigterm()) {
#ifdef _UNIX
            kill(0, SIGTERM);
#endif
            return;
        }
        base_sleep(3);
        Bind();
    }
  
    if (g_pHandler->GetSignalSigterm())
        return;

    static const CString __SSI("SSI");
    static const CString __ENABLESSI("ENABLESSI");
    static const CString __THREAD_("THREAD_");
    static const CString __MAXTHREADS("MAXTHREADS");
    static const CString __MT("MT");
    static const CString __DISABLEPING("DISABLEPING");
    static const CString __ENABLEPING("ENABLEPING");
    static const CString __DISABLESWAP("DISABLESWAP");
    static const CString __ENABLESWAP("ENABLESWAP");
    static const CString __AI("AI");
    static const CString __ACCEPTINTERVAL("ACCEPTINTERVAL");
	static const CString __NOREINDEX("NOREINDEX");
                
    int ePos;
    
    bool bPing = m_GlobalCnf.GetPing();
  
    CString Name, Value;

    for (register int i=0;i<(int)m_Options.GetSize();i++) {
		if (m_Options[i].Same(__NOREINDEX)) {			
            m_Reindex = false;
		} else if (m_Options[i].Same(__ENABLESSI)||m_Options[i].Same(__SSI)) {
            cout << "[--enablessi] - enabling server-side includes" << endl;
            m_SSIEnabled = true;
        } else if ((ePos = m_Options[i].Pos('=')) > 0) {
            m_Options[i].Mid(0, ePos, &Name);
            if ((Name.Same(__MAXTHREADS))||(Name.Same(__MT))) {
                m_Options[i].Mid(ePos+1, m_Options[i].GetLength(), &Value);
                int vVal = CString::StrToInt(Value);                  
                if (vVal > 0) {                      
                    cout << "[--maxthreads] max thread pool: " << vVal << endl;
                    CServer::m_ThreadPool.SetMaxThreads(vVal);
                } else cout << "[--maxthreads] " << Value << " is an invalid number" << endl;
            } else if ((Name.Same(__ACCEPTINTERVAL))||(Name.Same(__AI))) {
                m_Options[i].Mid(ePos+1, m_Options[i].GetLength(), &Value);
                int vVal = CString::StrToInt(Value);                  
                if (vVal > 0) {                      
                    cout << "[--acceptinterval] accept interval: " << vVal << endl;
                    SetAcceptInterval(vVal);
                } else cout << "[--acceptinterval] " << Value << " is an invalid number" << endl;
            }
        } else if (m_Options[i].Same(__DISABLESWAP)) {
        } else if (m_Options[i].Same(__ENABLESWAP)) {
        } else if (m_Options[i].Same(__DISABLEPING)) {
            bPing = false;
            cout << "[--disableping] disabling localhost ping" << endl;
        } else if (m_Options[i].Same(__ENABLEPING)) {
            bPing = true;
            cout << "[--enableping] enabling localhost ping" << endl;
        }
#ifdef _WIN32
        else if (m_Options[i].StartsWithSame(__THREAD_)) {
            if (!m_AlkalineData->SetThreadOption(m_Options[i])) {
                cerr << "invalid option: " << m_Options[i] << endl;
            }
        }
#endif
    }

	if (m_Reindex && ! g_pHandler->GetSignalSigterm()) {
		CMutex DataMutex;
		DataMutex.Lock();
		m_AlkalineData->SetThreadStartMutex(&DataMutex);
		m_AlkalineData->Launch();
		DataMutex.Lock();
		WriteLog("server data loaded\tdetail=data thread segment initialized\tfacility=server\tcategory=server\ttype=information");
		DataMutex.UnLock();
	}

    if (bPing && ! g_pHandler->GetSignalSigterm()) {
		if (! m_PingThread) {
            Trace(tagServer, levInfo, ("CAlkalineServer::Launch - starting ping at {%s}.", m_PingAddress.GetBuffer()));
			m_PingThread = new CAlkalinePingThread(m_PingAddress, 0);
            m_PingThread->SetPingInterval(m_GlobalCnf.GetPingInterval());
            m_PingThread->SetPingRestart(m_GlobalCnf.GetPingRestart());
        }
        m_PingThread->Launch();
		WriteLog("started ping thread\tdetail=ping thread segment initialized\tfacility=server\tcategory=server\ttype=information");
    }

    if (g_pHandler->GetSignalSigterm())
        return;

    CCertifiedServer::Launch();
}

void CAlkalineServer::Bind(void) {
    
	WriteLog("server binding\tfacility=server\tcategeory=server\ttype=information\tversion=" + CString(RC_VERSION_STRING)  + "\tport=" + m_PortString);

    if (g_pHandler->GetSignalSigterm())
        return;

    if (m_PingThread)
		m_PingThread->SetbPing(true);
    CServer::Bind();
}

void CAlkalineServer::UnBind(void) {
	WriteLog("server unbinding\tfacility=server\tcategeory=server\ttype=information\tversion=" + CString(RC_VERSION_STRING)  + "\tport=" + m_PortString);
    if (m_PingThread)
		m_PingThread->SetbPing(false);
    CServer::UnBind();
}

CString CAlkalineServer::Map(const CString& Source, CHttpIo& HttpIo) const {
	CString Result;	
	MapTerm(Source, Result, 0);
    Result.Replace("serverresponsexml", HttpIo.GetRFields().GetValue(m_ServerResponseXml));
	return Result;
}

CString& CAlkalineServer::MapTerm(const CString& Source, CString& Target, int Dummy) const {
	MAP_TERM_MACRO(Source, Target, MapTerm, MapTermEach, false, Dummy);
}

CString& CAlkalineServer::MapTermEach(CString& Term, int /* Dummy */) const {

    static const CString ServerXmlMap(
        "<server>"                                 \
        " <pool>"                                  \
        "  <started></started>"                    \
        "  <running></running>"                    \
        "  <threads></threads>"                    \
        "  <uptime></uptime>"                      \
		"  <max></max>"                            \
		"  <rampup></rampup>"                      \
		"  <maxqueue></maxqueue>"                  \
		"  <maxidle></maxidle>"                    \
		"  <jobs></jobs>"                          \
		"  <waiting></waiting>"                    \
        " </pool>"                                 \
        " <started></started>"                     \
        " <running></running>"                     \
        " <date></date>"                           \
        " <release></release>"                     \
        " <restart>0</restart>"                    \
        "</server>"                                \
        "<search>"                                 \
        " <requests></requests>"                   \
        " <rpm></rpm>"                             \
        "</search>"                                \
        "<system>"                                 \
        " <traffic>"                               \
        "  <output></output>"                      \
        " </traffic>"                              \
        " <swap>"                                  \
        "  <size>0</size>"                         \
        "  <virtual>0</virtual>"                   \
        "  <used>0</used>"                         \
        "  <blocks>0</blocks>"                     \
        "  <view>not mapped</view>"                \
        "  <frag>unfragmented</frag>"              \
        " </swap>"                                 \
        " <cpu>"                                   \
        "  <system>N/A</system>"                   \
        "  <user>N/A</user>"                       \
        "  <time>N/A</time>"                       \
        "  <load>N/A</load>"                       \
        " </cpu>"                                  \
        " <uts>"                                   \
        "  <node>N/A</node>"                       \
        "  <descr>N/A</descr>"                     \
        " </uts>"
        "</system>");

    static const CString TmplCacheXmlMap(
        "<templates>"                              \
        " <size></size>"                           \
        "</templates>");

    static CString GlobalConfigXmlMap(
        "<config>"                                 \
        "</config>"); 
 
    if (Term.Same("tmplcachexml")) {  
        CXmlTree XmlTree;
        XmlTree.SetXml(TmplCacheXmlMap);
        CXmlNode TemplateXmlNode;
        CXmlNode DataNode;
        DataNode.SetType(xmlnData);
        CTreeElement< CXmlNode > * pXmlNode = XmlTree.XmlFind("/templates");
    
        m_ExecuteMutex.StartReading();

        XmlTree.SetValue("/templates/size", CString::IntToStr(m_ContainersTable.GetSize()));
        for (register int i=0; i<(int) m_ContainersTable.GetSize();i++) {
            TemplateXmlNode.SetType(xmlnOpen);
            TemplateXmlNode.SetData(CString::IntToStr(i));
            CTreeElement< CXmlNode > * pXmlNewNode = XmlTree.AddChildLast(pXmlNode, TemplateXmlNode);
            
            DataNode.SetData(m_ContainersTable.GetKeyAt(i));
            DataNode.GetData().Replace('\\', '/');
            XmlTree.AddChild(pXmlNewNode, DataNode); 
            
            TemplateXmlNode.SetType(xmlnClose);
            XmlTree.AddChildLast(pXmlNode, TemplateXmlNode);
        }
       
        m_ExecuteMutex.StopReading();
        
        XmlTree.GetXml(Term);
    } else if (Term.Same("configwml")) {        
        m_AlkalineData->GetConfigWml(Term);
	} else if (Term.Same("configwmlsearch")) {        
        m_AlkalineData->GetConfigWmlSearch(Term);
	} else if (Term.Same("configxml")) {
		m_AlkalineData->GetConfigXml(Term);
    } else if (Term.Same("globalconfigxml") || Term.Same("globalconfigwml")) {
        CXmlTree XmlTree;
        XmlTree.SetXml(GlobalConfigXmlMap);  
        m_GlobalCnf.PopulateXmlNode(XmlTree, XmlTree.XmlFind("/config"));    
		if (Term.Same("globalconfigwml")) {
			XmlTree.GetWml(Term);
		} else if (Term.Same("globalconfigxml")) {
			XmlTree.GetXml(Term); 
		}
    } else if (Term.Same("serverxml") || Term.Same("serverwml")) {  
        CXmlTree XmlTree;
        XmlTree.SetXml(ServerXmlMap);

        CDate Today;
 
        XmlTree.SetValue("/server/release", RC_VERSION_STRING);
        XmlTree.SetValue("/server/started", m_StartupDateString);
        XmlTree.SetValue("/server/running", CDate::GetElapsedTime(Today, m_StartupDate));
        XmlTree.SetValue("/server/date", Today.AscTime());
        XmlTree.SetValue("/search/requests", CString::IntToStr(CAlkalineSession::m_AlkalineSessions.Get()));
        XmlTree.SetValue("/search/rpm", CString::DoubleToStr(CAlkalineSession::m_AlkalineSessions.Get() / (BASE_MAX(1, ((double) (Today.GetTTime() - m_ServerStartupDate.GetTTime())/60))), 0, 1));
        XmlTree.SetValue("/system/traffic/output", CString::KBytesToStr(CHttpIo::m_TotalTraffic.Get()));

        XmlTree.SetValue("/server/pool/started", m_ServerStartupDateString);
        XmlTree.SetValue("/server/pool/running", CDate::GetElapsedTime(Today, m_ServerStartupDate));
        XmlTree.SetValue("/server/pool/uptime", CString::IntToStr(abs(Today.GetTTime() - m_ServerStartupDate.GetTTime())));        
		XmlTree.SetValue("/server/pool/max", CString::IntToStr(CServer::m_ThreadPool.GetMaxThreads()));
		XmlTree.SetValue("/server/pool/rampup", CString::IntToStr(CServer::m_ThreadPool.GetRampupThreads()));
		XmlTree.SetValue("/server/pool/maxqueue", CString::IntToStr(CServer::m_ThreadPool.GetMaxQueueSize()));
		XmlTree.SetValue("/server/pool/maxidle", CString::IntToStr(CServer::m_ThreadPool.GetMaxThreadIdle()) + " seconds");
		XmlTree.SetValue("/server/pool/jobs", CString::IntToStr(CServer::m_ThreadPool.GetJobsList().GetSize()));
		XmlTree.SetValue("/server/pool/threads", CString::IntToStr(CServer::m_ThreadPool.GetThreads().GetSize()));
		XmlTree.SetValue("/server/pool/waiting", CString::IntToStr(CServer::m_ThreadPool.GetWaitingThreads().Get()));

        VMAccounting AccountingInfo;
        CVirtualMemory :: m_pSwap->GetAccountingInfo(& AccountingInfo);

        if (AccountingInfo.stTotal > 0) {
            XmlTree.SetValue("/system/swap/size", CString::BytesToStr(AccountingInfo.stTotal));
            XmlTree.SetValue("/system/swap/virtual", CString::BytesToStr(AccountingInfo.stVirtual));
            XmlTree.SetValue("/system/swap/used", CString::BytesToStr(AccountingInfo.stUsed));
            XmlTree.SetValue("/system/swap/blocks", CString::IntToStr(AccountingInfo.nBlockCount));
            XmlTree.SetValue("/system/swap/frag", "unavailable"); // CString::IntToStr(AccountingInfo.nBlockCount ? (AccountingInfo.nFragCount * 100 / AccountingInfo.nBlockCount) : 0) + "%");
        }
#ifdef _OS_SunOS
        struct tms Tms; times(&Tms);
        double sTime = Tms.tms_stime/(double)CLK_TCK;
        double uTime = Tms.tms_utime/(double)CLK_TCK;    
        if (sTime)
            XmlTree.SetValue("/system/cpu/system", CDate::GetElapsedTime(0, sTime));
        if (uTime)
            XmlTree.SetValue("/system/cpu/user", CDate::GetElapsedTime(0, uTime));
        double absTime = abs(Today.GetTTime()-m_StartupDate.GetTTime());
        if ((sTime || uTime) && absTime) 
            XmlTree.SetValue("/system/cpu/load", CString::DoubleToStr((sTime + uTime)*100/absTime, 0, 2) + "%");
#endif
#ifdef _WIN32
        long ServerUser, ServerKernel, DataUser, DataKernel;
        if ((m_AlkalineData->GetTimes(DataUser, DataKernel))&&(GetTimes(ServerUser, ServerKernel))) {
            long Total = ServerKernel + DataKernel + m_KernelTime.Get() + ServerUser + DataUser + m_UserTime.Get();
            XmlTree.SetValue("/system/cpu/time", CDate::GetElapsedTime(0, Total/1000));
            XmlTree.SetValue("/system/cpu/load", CString::DoubleToStr((double)(Total/10)/abs(Today.GetTTime()-m_StartupDate.GetTTime()), 0, 2) + "%");
        }
#endif

#ifdef _UNIX
        struct utsname UTS;
        if (uname(&UTS) >= 0) {
            CString UtsDescription = CString(UTS.sysname) + " " + CString(UTS.release) + " (" + CString(UTS.version) + "/" + CString(UTS.machine) + ")";
            CString UtsNodename = CString(UTS.nodename);
            XmlTree.SetValue("/system/uts/descr", UtsDescription);
            if (UtsNodename.GetLength())
                XmlTree.SetValue("/system/uts/node", UtsNodename);
        }
#endif
#ifdef _WIN32
        XmlTree.SetValue("/system/uts/descr", "Windows NT/2000");
#endif
#ifdef _UNIX
        // is the server restartable
        if (!m_Daemon) 
            XmlTree.SetValue("/server/restart", "1");
#endif
		if (Term.Same("serverxml")) {
			XmlTree.GetXml(Term);
		} else if (Term.Same("serverwml")) {
			XmlTree.GetWml(Term);
		}

        
    } else if (Term.Same("certifxml")) {
    /*
        CXmlTree XmlTree;
        m_Certificate.PopulateXmlTree(XmlTree);
        XmlTree.GetXml(Term);

        if (m_Certificate.GetCertified()) {
            CRemoteFile RemoteCertFile(CString("http://reg.vestris.com/trump/trump?admin+certificate=") + m_Certificate.CalculateCertif());
            RemoteCertFile.SetTimeout(3);
            RemoteCertFile.Get();
            
            int lPos = RemoteCertFile.GetRData().Pos("<!--xml");
            if (lPos != -1) {
                lPos += (sizeof("<!--xml") - 1);
                int rPos = RemoteCertFile.GetRData().Pos(CString("-->"), lPos);
                if (rPos != -1) {
                    CString MidXml;
                    RemoteCertFile.GetRData().Mid(lPos, rPos - lPos, & MidXml);
                    Term += MidXml;
                }
            }
        }
        */
    }
    return Term;
}

void CAlkalineServer::Stop(void) {
    
    g_pHandler->SetSignalSigterm(true);

    if (m_PingThread)
		m_PingThread->SetTerminate(true);
	if (m_Reindex)
		m_AlkalineData->Stop();
    UnBind();
}
