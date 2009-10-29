/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  _____________________________________________________

  written by Daniel Doubrovkine - dblock@vestris.com

*/

#include <alkaline.hpp>
#include "GlobalCnf.hpp"
#include <File/LocalFile.hpp>
#include <AlkalineServer/AlkalineServer.hpp>

CGlobalCnf::CGlobalCnf(const CGlobalCnf& Other) : CConfigBase(Other) {
    Initialize();
    CreateConfigurationOptions();
    Copy(Other);
}

CGlobalCnf::CGlobalCnf(void) : CConfigBase() {
    Initialize();
    CreateConfigurationOptions();
}

void CGlobalCnf :: Initialize(void) {
    m_RampupSearchThreads = THREADPOOL_RAMPUP_DEFAULT;
    m_RampupIndexThreads = THREADPOOL_RAMPUP_DEFAULT;
    m_MaxSearchThreads = THREADPOOL_MAXTHREADS_DEFAULT;
    m_MaxIndexThreads = THREADPOOL_MAXTHREADS_DEFAULT;
    m_MaxSearchQueueSize = THREADPOOL_QUEUESIZE_DEFAULT;
    m_MaxSearchThreadIdle = THREADPOOL_IDLETHREAD_LIFE;
    
    m_AlkalineServer = NULL;
    
    m_Filename = "global.cnf";
    memset(&m_Stat, 0, sizeof(m_Stat));
    
    m_CacheTemplates = true;
    m_USFormats = false;
    m_KeepAlive = false;
    m_Nagle = true;
    m_Verbose = false;
    m_Ssi = false;

    Set("Redirect", "/admin/");
    Set("AdminPath", "admin");
    
    m_PingInterval = 30;
    m_PingRestart = 3;
    m_Ping = false;
    Set("PingUrl", "");
    
    m_SearchCacheLife = 60;
}
    
CGlobalCnf::~CGlobalCnf(void) {
  
}

CGlobalCnf& CGlobalCnf::Copy(const CGlobalCnf& Other) {
    if (&Other == this) 
        return * this;

    m_Stat = Other.m_Stat;
    m_Filename = Other.m_Filename;  
    m_CacheTemplates = Other.m_CacheTemplates;
    m_USFormats = Other.m_USFormats;
    m_Verbose = Other.m_Verbose;
    m_Ssi = Other.m_Ssi;
    m_KeepAlive = Other.m_KeepAlive;
    m_Nagle = Other.m_Nagle;
    
    m_RampupSearchThreads = Other.m_RampupSearchThreads;
    m_RampupIndexThreads = Other.m_RampupIndexThreads;
    m_MaxSearchThreads = Other.m_MaxSearchThreads;
    m_MaxIndexThreads = Other.m_MaxIndexThreads;
    m_MaxSearchQueueSize = Other.m_MaxSearchQueueSize;
    m_MaxSearchThreadIdle = Other.m_MaxSearchThreadIdle;
    
    m_AlkalineServer = Other.m_AlkalineServer;
    
    m_PingInterval = Other.m_PingInterval;
    m_PingRestart = Other.m_PingRestart;
    m_Ping = Other.m_Ping;
    
    m_SearchCacheLife = Other.m_SearchCacheLife;

	m_DocumentPaths = Other.m_DocumentPaths;
	m_ForwardAlnHeaders = Other.m_ForwardAlnHeaders;
    
    CConfigBase :: Copy(Other);
    return (* this);
}

void CGlobalCnf::AddOption(const CString& Line, bool Verbose) {  
    if (!Line.GetLength() || (Line[0] == '#'))
        return;

    CVector<CString> Values;
    CString MidValue;
    int EqualPos = 0;
    while (EqualPos < (int) Line.GetLength()) {
        if (Line[EqualPos] == '=')
            break;
        if (Line[EqualPos] == '\\')
            EqualPos++;
        EqualPos++;
    }
  
    if (EqualPos != (int) Line.GetLength()) { 
        Line.Left(EqualPos, &MidValue);
        Values.Add(MidValue);
        Line.Mid(EqualPos+1, Line.GetLength(), &MidValue);
        Values.Add(MidValue);
    } else {
        cout << "  [ignoring line: " << Line << "]" << endl;
        return;
    }
    Values[0].Trim();
    Values[1].Trim();

    static const CString OptionPass("Pass ");
	static const CString DocumentPath("DocumentPath");
	static const CString ForwardAlnHeaders("ForwardAlnHeaders");

	if (Values[0].Same(DocumentPath)) {
		// document path defines all paths that can be served as plain documents		
		CString::StrToVector(Values[1], ',', & m_DocumentPaths);
	} else if (Values[0].Same(ForwardAlnHeaders)) {
		CString::StrToVector(Values[1], ',', & m_ForwardAlnHeaders);
	} else if (Values[0].StartsWithSame(OptionPass)) {
        Values[0].Delete(0, OptionPass.GetLength());
        if (Values[0].Same("ROOT") ||
            Values[0].Same("ALKALINE-RESTART") ||
            Values[0].Same("ALKALINE-ADD") ||
            Values[0].Same("ALKALINE-MANAGE")) {
            m_AlkalineServer->GetAccessManager().Set(Values[0], Values[1]);
            Set("Pass " + Values[0], "password hidden");
            cout << "  [setting password for {" << Values[0] << "}]" << endl;
        } else {
            Set("Pass " + Values[0], "password hidden (unknown username)");
            cout << "  [invalid Pass value {" << Values[0] << "}, must be one of root, alkaline-<restart/add/manage>]" << endl;
        }
    } else {
        Set(Values[0], Values[1]);
    }
}

void CGlobalCnf :: DumpVirtual(CConfigOption& ConfigurationOption) const {
    
    cout << "string, default=" << 
        ((m_AlkalineServer != NULL) ? 
         m_AlkalineServer->GetEquivManager().GetValue(ConfigurationOption.csName).GetBuffer() :
         "(not set)");
}

void CGlobalCnf::CreateConfigurationOptions(void) {
    
    CConfigOption LocalConfigurationOptions[] = 
    {
            { "CacheTemplates",      ccoBool,         & m_CacheTemplates,        -1,    0,     "cache search templates" },
            { "USFormats",           ccoBool,         & m_USFormats,             -1,    0,     "us-formatted dates" },
            { "KeepAlive",           ccoBool,         & m_KeepAlive,             -1,    0,     "allow to keep-alive clients" },
            { "Nagle",               ccoBool,         & m_Nagle,                 -1,    0,     "disable nagle algorithm" },
            { "Proxy",               ccoVirtual,      NULL,                      -1,    0,     "define an HTTP proxy" },
            { "LogPath",             ccoVirtual,      NULL,                      -1,    0,     "location of the log files" },
            { "Realm",               ccoVirtual,      NULL,                      -1,    0,     "basic auth realm" },
            { "Redirect",            ccoVirtual,      NULL,                      -1,    0,     "default redirect" },
            { "AdminPath",           ccoVirtual,      NULL,                      -1,    0,     "administrative section path" },
			{ "DocumentPath",        ccoArray,        & m_DocumentPaths,          0,    0,     "plain document paths" },
			{ "ForwardAlnHeaders",   ccoArray,        & m_ForwardAlnHeaders,      0,    0,     "http headers to forward for templates" },
            { "ErrorFooter",         ccoVirtual,      NULL,                      -1,    0,     "error footer string" },
            { "Pass",                ccoEvalPair,     NULL,                       0,    0,     "server passwords" },
            { "Ssi",                 ccoBool,         & m_Ssi,                   -1,    0,     "enable server-side includes" },
            { "RampupSearchThreads", ccoDigitPos,     & m_RampupSearchThreads,    
                                                      THREADPOOL_RAMPUP_DEFAULT,        0,     "search thread pool rampup" },
            { "RampupIndexThreads",  ccoDigitPos,     & m_RampupIndexThreads,     
                                                      THREADPOOL_RAMPUP_DEFAULT,        0,     "index thread pool rampup" },
            { "MaxSearchThreads",    ccoDigitPos,     & m_MaxSearchThreads,       
                                                      THREADPOOL_MAXTHREADS_DEFAULT,    0,     "maximum search thread pool threads" },
            { "MaxIndexThreads",     ccoDigitPos,     & m_MaxIndexThreads,       
                                                      THREADPOOL_MAXTHREADS_DEFAULT,    0,     "maximum index thread pool threads" },
            { "MaxSearchQueueSize",  ccoDigitPos,     & m_MaxSearchQueueSize,       
                                                      THREADPOOL_QUEUESIZE_DEFAULT,     0,     "maximum search thread pool queue" },
            { "MaxSearchThreadIdle", ccoDigitPos,     & m_MaxSearchThreadIdle,       
                                                      THREADPOOL_IDLETHREAD_LIFE,       0,     "maximum search thread idle time" },
            { "PingInterval",        ccoDigitPos,     & m_PingInterval,          30,    0,     "self ping interval" },
            { "PingRestart",         ccoDigitPos,     & m_PingRestart,            3,    0,     "failed ping restart count" },
            { "Ping",                ccoBool,         & m_Ping,                  -1,    0,     "enable self ping thread" },
            { "PingUrl",             ccoVirtual,      NULL,                      -1,    0,     "url to ping periodically" },
     };
    
    CConfigBase :: CreateConfigurationOptions(    
        LocalConfigurationOptions, 
        (int) (sizeof(LocalConfigurationOptions) / sizeof(CConfigOption)));
}
    
void CGlobalCnf::FinalizeVirtual(CConfigOption& ConfigurationOption) {
    
    assert(m_AlkalineServer);
    
    CString Value = GetValue(ConfigurationOption.csName);
    
    if (m_Verbose && Value.GetLength()) {
        cout << "  [" << 
            ConfigurationOption.csName << "=" << 
            Value << "]" << endl;
    }
    
    m_AlkalineServer->GetEquivManager().Set(
        ConfigurationOption.csName,
        Value);    
}

void CGlobalCnf::Read(CAlkalineServer * Server, bool Verbose) {
    
    m_AlkalineServer = Server;
    
    m_Verbose = Verbose;
    
    struct_stat NewStat; 
    base_stat((const char *) m_Filename.GetBuffer(), &NewStat);
    if (GetSize() && (m_Stat.st_mtime == NewStat.st_mtime)) 
        return;
 
    CLocalFile LocalFile(m_Filename);
    CVector<CString> Lines;

    if (LocalFile.OpenReadBinary()) {
        if (m_Verbose) {
			cout << "[" << ((GetValue("Loaded") == "Yes")?"re":"") << "loading " << m_Filename << "] {" << endl;
        }
        LocalFile.ReadLines(&Lines);
        for (int i=0; i < (int) Lines.GetSize(); i++) {
            AddOption(Lines[i], m_Verbose);
        }
        Set("Size", CString::IntToStr(LocalFile.GetSize()) + " bytes");
        CConfigBase :: Finalize();
    
        // adjust threading for the search server
        
        m_AlkalineServer->GetThreadPool().SetMaxThreads(m_MaxSearchThreads);
        m_AlkalineServer->GetThreadPool().SetMaxQueueSize(m_MaxSearchQueueSize);
        m_AlkalineServer->GetThreadPool().SetRampupThreads(m_RampupSearchThreads);
        m_AlkalineServer->GetThreadPool().SetMaxThreadIdle(m_MaxSearchThreadIdle);
        m_AlkalineServer->SetSSIEnabled(m_Ssi);
        
		m_DocumentPaths += "docs";
		m_DocumentPaths += "faqs";

        if (m_Verbose) {
			CString strDocumentPaths;
			CString::VectorToStr(m_DocumentPaths, ',', & strDocumentPaths);
			cout << "  [DocumentPath=" << strDocumentPaths << "]" << endl;
			CString strForwardAlnHeaders;
			CString::VectorToStr(m_ForwardAlnHeaders, ',', & strForwardAlnHeaders);
			cout << "  [ForwardAlnHeaders=" << strForwardAlnHeaders << "]" << endl;
            cout << "} loaded " << LocalFile.GetSize() << " bytes." << endl;
        }

		Set("Loaded", "Yes");
    } else {
        Set("Loaded", "No");
    }

    m_Stat = NewStat;    
}

bool CGlobalCnf::HasChanged(void) const {  
    struct_stat NewStat; 
    base_stat((const char *) m_Filename.GetBuffer(), &NewStat);
    return (m_Stat.st_mtime != NewStat.st_mtime);	
}

bool CGlobalCnf::IsDocumentPath(const CString& HttpDirectory) const {
	CString HttpPath(HttpDirectory);
	HttpPath.TerminateWith('/');
	for (int i = 0; i < (int) m_DocumentPaths.GetSize(); i++) {
		CString DocumentPath(m_DocumentPaths[i]);
		DocumentPath.Trim();
		DocumentPath.Replace('\\', '/');
		DocumentPath.TerminateWith('/');
       #ifdef _WIN32
		if (HttpPath.StartsWithSame(DocumentPath)) {
       #endif
       #ifdef _UNIX
		if (HttpPath.StartsWith(DocumentPath)) {
       #endif
			return true;
		}
	}
	return false;
}