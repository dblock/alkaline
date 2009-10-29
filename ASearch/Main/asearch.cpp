/*

  © Vestris Inc., Geneva Switzerland
  http://www.vestris.com, 1998, All Rights Reserved
  __________________________________________________

  written by Daniel Doubrovkine - dblock@vestris.com

*/

#include <alkaline.hpp>
#include "bldver.hpp"

#include <platform/include.hpp>
#include <String/String.hpp>
#include <Mv4/AccessManager.hpp>
#include <Mv4/AdminManager.hpp>
#include <Mv4/EquivManager.hpp>
#include <File/LocalFile.hpp>
#include <AlkalineServer/AlkalineServer.hpp>
#include <Tree/UrlTree.hpp>
#include <File/LocalFile.hpp>
#include <RegExp/RegExp.hpp>
#include <AlkalineServer/AlkalineService.hpp>
#include <FileSystem/LocalSystem.hpp>
#include <Virtual/FileMapping.hpp>
#include <Main/TraceTags.hpp>

void Merge(const CString& Target, const CVector<CString>& Sites);
void Header(void);
void Usage();
void GlobalOptions(const CLocalSystem& LocalSystem);

static const CString g_strAsearchCnf("asearch.cnf");
static const CString g_strAsearchServer("asearch.server");

static const CString g_strService("SERVICE");
static const CString g_strInstall("INSTALL");
static const CString g_strRemove("REMOVE");
static const CString g_strStart("START");
static const CString g_strStop("STOP");
static const CString g_strDispatch("DISPATCH");
static const CString g_strDebug("DEBUG");

// static const CString g_strSuspend("SUSPEND");
// static const CString g_strResume("RESUME");

static const CString g_strStartDirectory("StartDirectory");
static const CString g_strStartArguments("StartArguments");
static const CString g_strStartOptions("StartOptions");

static const CString g_strOptions("options");
static const CString g_strTracing("tracing");
static const CString g_strRxRepl("rxrepl");
static const CString g_strRxMatch("rxmatch");
static const CString g_strParse("parse");

bool bNoEquivStruct = false;
bool bDaemon = false;
bool bFork = true;
char ** g_Argv;

#ifdef _UNIX
static pid_t ForkedParentPid = 0;
#endif    
    
CAlkalineServer * pServer = NULL;

void Header(void) {
    cout << "===============================================================================" << endl;
    cout << " Alkaline Search Engine, Version " << RC_VERSION_STRING;
#ifdef _UNIX
    struct utsname UTS; if (uname(&UTS) >= 0) cout << " for " << UTS.sysname;
#endif
#ifdef _WIN32
    cout << " for Windows NT";
#endif
    cout << endl << " (c) Vestris Inc., Switzerland - 1994-2002 - All Rights Reserved" << endl;
    cout << " http://alkaline.vestris.com/" << endl;
    cout << "===============================================================================" << endl;
}

void Usage() {
    Header();
    CLocalPath Executable(g_Argv[0]);
    cout << "    usage: " << Executable.GetFilename() << " [options] [host:]port path [path2 ...]\n" \
        "           " << Executable.GetFilename() << " [options] path-to-asearch.cnf command ...\n" \
        "     port: port to bind and listen to, ex: 9999\n" \
        "     path: relative path(s) to asearch.cnf files\n" \
        "  options: please refer to the users guide\n" \
        "  command: one of reindex, email, emailall, remove, rxmatch, rxrepl, etc.\n" \
        "_______________________________________________________________________________\n" \
        "  for more information, please refer to http://alkaline.vestris.com/" << endl;
#ifdef _WIN32
    CLocalSystem :: GetCh();
#endif
    CHandler :: Terminate(-1);
}

void ServiceUsage() {
    Header();
#ifdef _WIN32
    CLocalPath Executable(g_Argv[0]);
    cout << "    usage: " << Executable.GetFilename() << " service [command] <options> [port path [path2 ...]]\n" \
        "     port: port to bind and listen to, ex: 9999\n" \
        "     path: relative path(s) to asearch.cnf files\n" \
        "  command: install, remove, start, stop\n" \
		"  options: --servicename=<name>\n" \
		"           --servicedisplayname=<display name>\n" \
		"           --servicedescription=<service description>\n" \
        "           --serviceusername=<domain\\username>\n" \
		"           --servicepassword=<password>\n" \
		"_______________________________________________________________________________\n" \
        "  for more information, please refer to http://alkaline.vestris.com/" << endl;
#endif
#ifdef _UNIX
    cout << "services is a Windows NT specific feature\n" \
        "_______________________________________________________________________________\n" \
        "  for more information, please refer to http://alkaline.vestris.com/" << endl;
#endif
#ifdef _WIN32
    CLocalSystem :: GetCh();
#endif
    CHandler :: Terminate(-1);
}

// match a regular expression
void RxMatch(const CVector<CString>& Arguments) {
    if (Arguments.GetSize() < 3)
        Usage();
    if (CRegExp::Match(Arguments[0], Arguments[2]))
        cout << "+rxmatch: [" << Arguments[0] << "] matches [" << Arguments[2] << "]" << endl;
    else cout << "-rxmatch: [" << Arguments[0] << "] does not match [" << Arguments[2] << "]" << endl;  
}

// replace a regexp
void RxRepl(const CVector<CString>& Arguments) {
    if (Arguments.GetSize() < 4)
        Usage();
    else {
        cout << "string: [" << Arguments[0] << "]" << endl;                          
        cout << "source: [" << Arguments[2] << "]" << endl;                          
        cout << "target: [" << Arguments[3] << "]" << endl;                          
        CString Result = CRegExp::SearchReplace(Arguments[0], Arguments[2], Arguments[3]);
        cout << "rxrepl: [" << Result << "]" << endl;                          
    }
}

// replace a regexp
void RxParse(const CLocalSystem& LocalSystem) {
    if (LocalSystem.GetCmdLineArguments().GetSize() <= 1) {
        Usage();
        return;
    }

    CString Data;
    CString Username;        
    CString Password;
    bool bVerbose = false;
    int i = 0;

    for (i = 0; i < (int) LocalSystem.GetCmdLineOptions().GetSize(); i++) {
        CString Option = LocalSystem.GetCmdLineOptions()[i];
        if (Option.StartsWithSame("auth:")) {
            Option.Mid(sizeof("auth:") - 1, Option.GetLength(), & Username);
            int nCol = Username.Pos(':');
            if (nCol != -1) {
                Username.Mid(nCol + 1, Username.GetLength(), & Password);
                Username.Delete(nCol, Username.GetLength());
            }            
            cout << "alkaline::adding credentials {" << Username << " (" << Password << ")" << endl;
        } else if (Option.Same("verbose")) {
            cout << "alkaline::enabling verbose mode" << endl;
            bVerbose = true;            
        }
    }
    
    for (i = 1; i < (int) LocalSystem.GetCmdLineArguments().GetSize(); i++) {
        CString Argument = LocalSystem.GetCmdLineArguments()[i];
        CAlkalineParser HtmlParser;
        HtmlParser.SetEnableObjects(true);
        HtmlParser.SetVerbose(false);
        HtmlParser.SetVerboseParser(true);        
        
        cout << "alkaline::parsing {" << Argument << "}" << endl;            
        
        if (Argument.StartsWithSame("http://")) {    
            CRemoteFile RemoteFile(Argument);
            RemoteFile.GetHttpRequest().SetDump(bVerbose);
            RemoteFile.AddAuth(Username, Password);            
            RemoteFile.Get();
            if (RemoteFile.GetRStatusValue() != 200) {
                cout << "error retrieving " << Argument << " - " << 
                    CHttpIo::GetRStatusString(RemoteFile.GetRStatusValue()) << endl;
                continue;
            }
            Data = RemoteFile.GetRData();
        } else {                
            CLocalFile FastFile(Argument);
            if (!FastFile.OpenReadBinary()) {
                cout << "error opening " << Argument << endl;
                continue;
            }                
            if (!FastFile.Read(& Data)) {
                cout << "error reading " << Argument << endl;
                continue;
            }                
        }
        HtmlParser.Parse(Data);
    }
}
    
// run a site
void RxSite(const CVector<CString>& Arguments, const CVector<CString>& Options) {
    CSite Site(Arguments[0], Arguments[0], false);
    Site.SetOptions(Options);
    if (Arguments[1].Same("email")) {
        Site.SetGatherEmail(true);
        Site.RetrieveSite(false);
    } else if (Arguments[1].Same("emailall")) {
        Site.SetGatherEmail(true);
        Site.SetGatherEmailAll(true);
        Site.RetrieveSite(false);			
    } else if (Arguments[1].Same("reindex")) {
        Header();
        Site.RetrieveSite(0);
    } else if (Arguments[1].Same("remove")) {
        for (int i=2;i<(int) Arguments.GetSize();i++) {
            if (Site.RemovePage(Arguments[i])) 
                cout << "[" << Arguments[i] << " removed]" << endl; 
            else cout << "[" << Arguments[i] << " not indexed]" << endl;
        }
    } else if (Arguments[1].Same("rxremove")) {
        cout << "regexp remove not implemented (yet)" << endl;
    } else if (Arguments[1].Same("merge")) {
        Trace(tagMerge, levInfo, ("Alkaline :: RxSite - merge"));
        CVector<CString> MergeSites;
        for (register int j=2;j<(int)Arguments.GetSize();j++) {
            Trace(tagMerge, levInfo, ("Alkaline :: RxSite - merge, adding [%s]", Arguments[j].GetBuffer()));
            MergeSites += Arguments[j];
        }
        if (MergeSites.GetSize()) {
            Merge(Arguments[0], MergeSites);
        } else {
            Usage();
        }
    } else if (Arguments[1].Same("excludewords")) {
        Trace(tagExcludeWords, levInfo, ("Alkaline :: RxSite - excludewords"));    
        CVector<CString> Filenames;
        bool bRegExp = Options.Contains("regexp");
        Trace(tagExcludeWords, levInfo, ("Alkaline :: RxSite - excludewords, regexp=%s", bRegExp?"yes":"no"));        
        for (int i=2;i<(int) Arguments.GetSize();i++) {
            Trace(tagExcludeWords, levInfo, ("Alkaline :: RxSite - excludewords, adding [%s]", Arguments[i].GetBuffer()));                    
            Filenames+=Arguments[i];
        }
        Site.RemoveWords(Filenames, bRegExp);
    }
}

void IntializeGlobals(int argc, char ** argv) {

    char * pSwapFilename = NULL;
    
    bool bEnableSwap = CFileMapping :: GetArgsEnableSwap(argc, argv, & pSwapFilename);

    if (bEnableSwap) {
        cout << "[--enableswap] enable swap (" << (pSwapFilename ? pSwapFilename : "automatic") << ")" << endl;
    }

	GlobalInitialize(bEnableSwap, pSwapFilename);   
}

void UnInitializeGlobals(void) {	
	GlobalTerminate();
}

bool VerifyArguments(const CString& Alias, const CString& AdminParameter) {
    CEquivManager EquivManager;
    CAccessManager AccessManager;
    CAdminManager AdminManager;
    
    CString Equiv = EquivManager.GetValue(Alias);
    CString Admin = AdminManager.Get(AdminParameter, Alias);
    
    if (!bDaemon)
        cout << "[checking 1.3 configuration] {" << endl;
    /*
    Admin
    */
    if (Admin == AdminParameter) {
        if (!bDaemon)
            cout << "  [successfully verified equiv/admin.struct]" << endl;
    } else {
        cerr << "  [error, equiv/equiv.struct does not contain\n    " << Alias << ",<list of aliases>]" << endl;
        return 0;
    }
    
    /*
    Access
    */
    if (AccessManager.GetValue("root").GetLength() == 0) {
        cerr << "  [warning, equiv/access.struct should contain\n    root, password]" << endl;
    }
    
    /*
    Alkaline,Alias1 Alias2
    */
    if (Equiv.GetLength()) {
        CVector<CString> EquivDigest; CString::StrToVector(Equiv, ' ', &EquivDigest);
        for (int i=0;i<(int) EquivDigest.GetSize();i++) {
            if (EquivDigest[i].GetLength()) {
                cout << "  [verifying if Alkaline alias (" << EquivDigest[i] << ") is valid]" << endl;
                CString AlkalineAlias = EquivManager.GetValue(EquivDigest[i]);
                if (AlkalineAlias.GetLength()) {
                    CLocalPath::Terminate(AlkalineAlias);
                    AlkalineAlias+=g_strAsearchCnf;
                    cout << "  [verifying that (" << AlkalineAlias << ") exists]" << endl;
                    CLocalFile File(AlkalineAlias);
                    if (File.Open(O_RDONLY) && File.GetSize()) {
                        cout << "  [file exists (" << File.GetSize() << " bytes)]" << endl;
                    } else {
                        cerr << "  [error, the following file has not been found or is empty:\n    " << AlkalineAlias << "]" << endl;
                        return false;
                    }
                } else {
                    cerr << "  [error, equiv/equiv.struct does not contain\n    " << EquivDigest[i] << ",<path to asearch.cnf>]" << endl;
                    return false;
                }
            }
        }
    } else {
        cerr << "  [error, equiv/equiv.struct does not contain\n    " << Alias << ",<valid Alkaline alias>]" << endl;
        return false;
    }
    cout << "} done." << endl;
    bNoEquivStruct = false;
    return true;
}


bool VerifyArgumentsEx(const CVector<CString> & Arguments) {
  CEquivManager EquivManager;
  // check for new syntax, not in equiv/equiv.struct but a directory
  if (!bDaemon)
      cout << "[checking post 1.3 configuration] {" << endl;
  for (register int i=1;i<(int) Arguments.GetSize();i++) {
    CString Alias = Arguments[i];    
    if (EquivManager.GetValue(Alias).GetLength()) {
        cout << "  [equiv/equiv.struct contains the (" << Alias << ") alias]" << endl;
        return false;
    }
    if (! Alias.EndsWithSame(g_strAsearchCnf)) {
      if (! CLocalPath::DirectoryExists(Alias)) {
        cerr << " [the path (" << Alias << ") could not be found]" << endl;
        return false;
      }
      CLocalPath::Terminate(Alias);            
      Alias += g_strAsearchCnf;
      if (!bDaemon)
          cout << "  [verifying that (" << Alias << ") exists]" << endl;
      CLocalFile File(Alias);
      if (File.OpenReadBinary() && File.GetSize()) {
          if (!bDaemon)
              cout << "  [file exists (" << File.GetSize() << " bytes)]" << endl;
      } else {
          cerr << "  [error, the following file has not been found or is empty:\n    " << Alias << "]" << endl;
        return false;
      }
	}
  }
  bNoEquivStruct = true;
  return true;
}

void ExecuteServer(const CLocalSystem& LocalSystem) {
    CString EquivString;
    
    if (bNoEquivStruct) {
        EquivString = "AlkalineAuto";    
    } else {
        EquivString = LocalSystem.GetCmdLineArguments()[1];
    }
        
    CString BindAddress, BindPort;
    CServer::GetBindAddressPort(LocalSystem.GetCmdLineArguments()[0], &BindAddress, &BindPort);
    pServer = new CAlkalineServer(CString::StrToInt(BindPort),
                                  BindAddress,
                                  EquivString, 
                                  g_strAsearchServer);
    
#ifdef _UNIX
    pServer->SetForkedParentPid(ForkedParentPid);
#endif
    
    if (bNoEquivStruct) {
        CString PathToAsearchCnf;
        CString EquivStringValue;
        
        for (register int i=1;i<(int) LocalSystem.GetCmdLineArguments().GetSize();i++) {
            if (EquivStringValue.GetLength())
                EquivStringValue += ' ';
            
			PathToAsearchCnf = LocalSystem.GetCmdLineArguments()[i];
			
			if (! CLocalPath::DirectoryExists(PathToAsearchCnf) && PathToAsearchCnf.EndsWithSame(g_strAsearchCnf))
                PathToAsearchCnf.SetLength(PathToAsearchCnf.GetLength() - g_strAsearchCnf.GetLength());

			PathToAsearchCnf.Replace('\\', '/');
            PathToAsearchCnf.TrimRight('/', '/');
    
            EquivStringValue += PathToAsearchCnf;
            pServer->GetEquivManager().Set(PathToAsearchCnf, PathToAsearchCnf);
        }
        
        pServer->GetAdminManager().Set(EquivString, g_strAsearchServer);
        pServer->GetEquivManager().Set(EquivString, EquivStringValue);

    }

    pServer->SetOptions(LocalSystem.GetCmdLineOptions());
    pServer->SetDaemon(bDaemon);    
    pServer->Launch();
    pServer->PassiveWait();    
}

void RunServer(const CLocalSystem& LocalSystem) {
#ifdef _UNIX
    if (bFork) {     
        // get the parent's process ID
        ForkedParentPid = getpid();
        switch (fork()) {
        case -1:
            perror("Alkaline Server");		
            cerr  << "[unable to fork daemon]" << endl;
            CHandler :: Terminate(1);
        case 0:
    
            // descriptors when running as a daemon
            if (bDaemon) {
                if (! CServer::DaemonForkedProcess()) {
                    cerr  << "[unable to detach daemon]" << endl;
                    CHandler :: Terminate(1);
                }
            }
            
            ExecuteServer(LocalSystem);
            CHandler :: Terminate(0);
        default:
            if (bDaemon) {
                /* running as a daemon */			
                CHandler :: Terminate(0);
            } else {
                int nProcessStatus = 0;
                cout << "[surveillance thread running]" << endl;
                wait(& nProcessStatus);
                if (WIFEXITED(nProcessStatus)) cout << "[main thread terminated (normally " << WEXITSTATUS(nProcessStatus) << ")]" << endl;
                else if (WIFSIGNALED(nProcessStatus)) cout << "[main thread terminated (uncaught signal " << WTERMSIG(nProcessStatus) << ")]" << endl;
                else if (WIFSTOPPED(nProcessStatus)) cout << "[main thread terminated (stop signal " << WSTOPSIG(nProcessStatus) << ")]" << endl;
                else cout << "[main thread terminated]" << endl;
            }
              
            break;
        }
    } else {
        ExecuteServer(LocalSystem);
    }
#endif
#ifdef _WIN32
    ExecuteServer(LocalSystem);	
#endif
}

void Merge(const CString& Target, const CVector<CString>& Sites) {
	CSite TargetSite(Target, Target, false);
	cout << "Initializing merge subsystem for [" << TargetSite.GetConfigPath() << "]" << endl;
	TargetSite.LoadSiteIndex(false);
	for (register int i=0;i<(int)Sites.GetSize();i++) {		
		CSite AppendSite(Sites[i], Sites[i], false);
		cout << "Merging with [" << AppendSite.GetConfigPath() << "]" << endl;
		AppendSite.LoadSiteIndex(false);
		TargetSite.Append(AppendSite);
	}
	TargetSite.GetSiteIndex().Write(true);
}

void RxRun(const CLocalSystem& LocalSystem, bool AutoRestart) {
    cout.flush();
    if (bDaemon) {        
        // base_close(base_fileno(stdout));        
    } else Header();
    if (VerifyArgumentsEx(LocalSystem.GetCmdLineArguments()) ||
        VerifyArguments(LocalSystem.GetCmdLineArguments()[1], 
        g_strAsearchServer)) {    
        do {
            RunServer(LocalSystem);
			if (g_pHandler->GetSignalSigterm())
				break;			
			if (! bDaemon) {
                cout << "[server stopped, restarting]" << endl;
			}
            GlobalOptions(LocalSystem);
        } while (AutoRestart);
    }   
}


//------------------------------------------------------------------------------------------
#ifdef _WIN32

void ServiceStart(void) {

	CAlkalineService::g_NTService->ReportStatusToSCMgr(SERVICE_START_PENDING, NO_ERROR, 3000);

	CLocalSystem LocalSystem;
	
    CString::StrToVector(CLocalSystem::RegGetString(
		HKEY_LOCAL_MACHINE, 
		CAlkalineService::g_NTService->GetRegParametersPath(),
		g_strStartArguments), ' ', & LocalSystem.GetCmdLineArguments());

    CString::StrToVector(CLocalSystem::RegGetString(
		HKEY_LOCAL_MACHINE, 
		CAlkalineService::g_NTService->GetRegParametersPath(),
		g_strStartOptions), ' ', & LocalSystem.GetCmdLineOptions());

	GlobalInitialize(LocalSystem.GetCmdLineOptions().Contains("enableswap"), NULL);   

    CString StartupDirectory = CLocalSystem::RegGetString(
        HKEY_LOCAL_MACHINE, 
        CAlkalineService::g_NTService->GetRegParametersPath(),
        g_strStartDirectory);

    if (!CLocalPath::ChDir(StartupDirectory)) {
        CAlkalineService::g_NTService->ReportStatusToSCMgr(SERVICE_STOPPED, ERROR_INVALID_PARAMETER, 0);
        return;
    }

    GlobalOptions(LocalSystem);

    if (LocalSystem.GetCmdLineArguments().GetSize() < 2) {
        CAlkalineService::g_NTService->ReportStatusToSCMgr(SERVICE_STOPPED, ERROR_INVALID_PARAMETER, 0);
        return;
    }
        
    // get the bind address and port
    CString BindAddress, BindPort;        
    bool bArgZeroInt = CServer::GetBindAddressPort(LocalSystem.GetCmdLineArguments()[0], &BindAddress, &BindPort);
    if (!bArgZeroInt) {
      CAlkalineService::g_NTService->ReportStatusToSCMgr(SERVICE_STOPPED, ERROR_INVALID_PARAMETER, 0);
      return;
    }
    
    CAlkalineService::g_NTService->ReportStatusToSCMgr(SERVICE_RUNNING, NO_ERROR, 0);
    
    // run the daemon
    RxRun(LocalSystem, false);
    
    CAlkalineService::g_NTService->ReportStatusToSCMgr(SERVICE_STOPPED, NO_ERROR, 0);

    CHandler :: Terminate(0);

	UnInitializeGlobals();
}

void ServiceStop(void) {
    if (pServer) {        
        pServer->Stop();
    }
}

void RxService(const CLocalSystem& LocalSystem) {    
    if (LocalSystem.GetCmdLineArguments().GetSize() < 2) {
        ServiceUsage();
        return;
    }

	CAlkalineService::g_NTService = new CAlkalineService;

	CVector<CString> ArgumentsVector = LocalSystem.GetCmdLineArguments();
	CVector<CString> OptionsVector = LocalSystem.GetCmdLineOptions();
        
	for (register int i=0; i < (int) OptionsVector.GetSize(); i++) {
		
		static const CString g_strServiceName("SERVICENAME=");
		static const CString g_strServiceDisplayName("SERVICEDISPLAYNAME=");
		static const CString g_strServiceUsername("SERVICEUSERNAME=");
		static const CString g_strServicePassword("SERVICEPASSWORD=");
		static const CString g_strServiceDescription("SERVICEDESCRIPTION=");
		
		CString CurrentArgument = OptionsVector[i];
		
		if (CurrentArgument.StartsWithSame(g_strServiceName)) {
			CString NewServiceName;
			CurrentArgument.Mid(g_strServiceName.GetLength(), CurrentArgument.GetLength(), & NewServiceName);			
			if (!NewServiceName.GetLength()) {
				ServiceUsage();
				return;
			}
			OptionsVector.RemoveAt(i--);
			cout << "Service name: " << NewServiceName << endl;
			CAlkalineService::g_NTService->SetServiceName(NewServiceName);
		} else if (CurrentArgument.StartsWithSame(g_strServiceDisplayName)) {
			CString NewServiceDisplayName;
			CurrentArgument.Mid(g_strServiceDisplayName.GetLength(), CurrentArgument.GetLength(), & NewServiceDisplayName);
			if (!NewServiceDisplayName.GetLength()) {
				ServiceUsage();
				return;
			}
			OptionsVector.RemoveAt(i--);
			cout << "Service display name: " << NewServiceDisplayName << endl;
			CAlkalineService::g_NTService->SetServiceDisplayName(NewServiceDisplayName);
		} else if (CurrentArgument.StartsWithSame(g_strServiceUsername)) {
			CString NewServiceUsername;
			CurrentArgument.Mid(g_strServiceUsername.GetLength(), CurrentArgument.GetLength(), & NewServiceUsername);
			if (!NewServiceUsername.GetLength()) {
				ServiceUsage();
				return;
			}
			OptionsVector.RemoveAt(i--);
			cout << "Run as (username): " << NewServiceUsername << endl;
			CAlkalineService::g_NTService->SetRunasUsername(NewServiceUsername);
		} else if (CurrentArgument.StartsWithSame(g_strServicePassword)) {
			CString NewServicePassword;
			CurrentArgument.Mid(g_strServicePassword.GetLength(), CurrentArgument.GetLength(), & NewServicePassword);
			if (!NewServicePassword.GetLength()) {
				ServiceUsage();
				return;
			}
			OptionsVector.RemoveAt(i--);
			cout << "Run as (password): " << NewServicePassword << endl;
			CAlkalineService::g_NTService->SetRunasPassword(NewServicePassword);
		} else if (CurrentArgument.StartsWithSame(g_strServiceDescription)) {
			CString NewServiceDescription;
			CurrentArgument.Mid(g_strServiceDescription.GetLength(), CurrentArgument.GetLength(), & NewServiceDescription);
			OptionsVector.RemoveAt(i--);
			cout << "Service description: " << NewServiceDescription << endl;
			CAlkalineService::g_NTService->SetServiceDescription(NewServiceDescription);
		}
	}
    
    if (LocalSystem.GetCmdLineArguments()[1].Same(g_strInstall)) {

        ArgumentsVector.RemoveAt(0);
        ArgumentsVector.RemoveAt(0);

        if (ArgumentsVector.GetSize() < 2) {
            ServiceUsage();
            return;
        }
		
        if (VerifyArgumentsEx(ArgumentsVector) ||
            VerifyArguments(ArgumentsVector[1], 
            g_strAsearchServer)) {    
            
            CAlkalineService::g_NTService->Install();
            // set registry keys for service startup --------------------------------------
            if (!LocalSystem.RegSetString(HKEY_LOCAL_MACHINE, 
                CAlkalineService::g_NTService->GetRegParametersPath(), 
                g_strStartDirectory,
                CLocalPath::GetCurrentDirectory())) {
                
                cerr << "Error setting " << CAlkalineService::g_NTService->GetRegParametersPath() << "\\" << g_strStartDirectory << endl;
            }
            
            CString Arguments, Options;
            CString::VectorToStr(ArgumentsVector, ' ', &Arguments);
            CString::VectorToStr(OptionsVector, ' ', &Options);
            
            if (!LocalSystem.RegSetString(HKEY_LOCAL_MACHINE, 
                CAlkalineService::g_NTService->GetRegParametersPath(), 
                g_strStartArguments, Arguments)) {
                
                cerr << "Error setting " << CAlkalineService::g_NTService->GetRegParametersPath() << "\\" << g_strStartArguments << endl;
            }

            if (!LocalSystem.RegSetString(HKEY_LOCAL_MACHINE, 
                CAlkalineService::g_NTService->GetRegParametersPath(), 
                g_strStartOptions,
                Options)) {
                
                cerr << "Error setting " << CAlkalineService::g_NTService->GetRegParametersPath() << "\\" << g_strStartOptions << endl;
            }
            // ----------------------------------------------------------------------------
        }
    } else if (LocalSystem.GetCmdLineArguments()[1].Same(g_strRemove)) {
        CAlkalineService::g_NTService->Remove();
    } else if (LocalSystem.GetCmdLineArguments()[1].Same(g_strStart)) {
        CAlkalineService::g_NTService->Start();
	} else if (LocalSystem.GetCmdLineArguments()[1].Same(g_strDebug)) {
		CAlkalineService::g_NTService->Debug();
    } else if (LocalSystem.GetCmdLineArguments()[1].Same(g_strStop)) {
        CAlkalineService::g_NTService->Control(SERVICE_CONTROL_STOP);
    // } else if (LocalSystem.GetCmdLineArguments()[1].Same(g_strResume)) {
    //    CAlkalineService::g_NTService->Control(SERVICE_CONTROL_CONTINUE);
    // } else if (LocalSystem.GetCmdLineArguments()[1].Same(g_strSuspend)) {
    //    CAlkalineService::g_NTService->Control(SERVICE_CONTROL_PAUSE);
    } else if (LocalSystem.GetCmdLineArguments()[1].Same(g_strDispatch)) {
        CAlkalineService::g_NTService->Dispatch();
    } else {
        ServiceUsage();        
    }
    delete CAlkalineService::g_NTService;
}

#endif
//------------------------------------------------------------------------------------------

void GlobalOptionsTrace(const CString& Command, const CString& Name, bool * pTraceTagsLevels) {
#ifdef BASE_TRACE_ENABLED
    CString TraceCommand;
    Command.Mid(Name.GetLength(), Command.GetLength(), & TraceCommand);
    CVector<CString> TraceNumbers;
    CString::StrToVector(TraceCommand, ',', & TraceNumbers);
    for (int i = 0; i < (int) TraceNumbers.GetSize(); i++) {
        long nCommand = CString::StrToLong(TraceNumbers[i]);
        if (nCommand >= 0 && nCommand < MAX_TRACE)
            pTraceTagsLevels[nCommand] = true;            
    }
#endif
}

// global arguments, daemon and swap
void GlobalOptions(const CLocalSystem& LocalSystem) {

    static const CString CmdDaemon("DAEMON");
    static const CString CmdNFork("NF");

   #ifdef BASE_TRACE_ENABLED
    static const CString CmdTraceLevels("TRACELEVELS=");
    static const CString CmdTraceTags("TRACETAGS=");
   #endif

    // static const CString CmdEnableSwap("ENABLESWAP");
    // static const CString CmdDisableSwap("DISABLESWAP");

    for (register int i = 0; i < (int) LocalSystem.GetCmdLineOptions().GetSize() ; i++) {
        if (LocalSystem.GetCmdLineOptions()[i].Same("d") || LocalSystem.GetCmdLineOptions()[i].Same(CmdDaemon)) {
            bDaemon = true;
        } else if (LocalSystem.GetCmdLineOptions()[i].Same(CmdNFork)) {
            bFork = false;
       #ifdef BASE_TRACE_ENABLED
        } else if (LocalSystem.GetCmdLineOptions()[i].StartsWithSame(CmdTraceLevels)) {
            GlobalOptionsTrace(LocalSystem.GetCmdLineOptions()[i], CmdTraceLevels, s_TraceLevels);
            bool bCurrent = false;
            for (int j = MAX_TRACE - 1; j >= 0; j--) {
                bCurrent |= s_TraceLevels[j];
                s_TraceLevels[j] = bCurrent;                
            }
        } else if (LocalSystem.GetCmdLineOptions()[i].StartsWithSame(CmdTraceTags)) {
            GlobalOptionsTrace(LocalSystem.GetCmdLineOptions()[i], CmdTraceTags, s_TraceTags);
       #endif
        }
        
        // } else if (LocalSystem.GetCmdLineOptions()[i].Same(CmdEnableSwap)) {
        // cout << "[--enableswap] enable swap" << endl;
        // CMMSwap::m_pSwap->SetEnabled(true);
        // } else if (LocalSystem.GetCmdLineOptions()[i].Same(CmdDisableSwap)) {
        // cout << "[--disableswap] disabling swap" << endl;
        // CMMSwap::m_pSwap->SetEnabled(false);
        // }
    }
}

int RunAlkaline(int argc, char ** argv) {

	CLocalSystem LocalSystem;
    g_Argv = argv;

    LocalSystem.ParseCmdLine(argc, argv);
    GlobalOptions(LocalSystem);

    if ((LocalSystem.GetCmdLineArguments().GetSize() == 1) && 
        LocalSystem.GetCmdLineArguments()[0].Same(g_strOptions)) {
        cout << "[global.cnf options]" << endl;
        CGlobalCnf GlobalConfiguration;
        GlobalConfiguration.DumpOptions();
        cout << "[asearch.cnf options]" << endl;
        CConfig Configuration;
        Configuration.DumpOptions();
        return 0;
    }

#ifdef BASE_TRACE_ENABLED

	if ((LocalSystem.GetCmdLineArguments().GetSize() == 1) &&
		LocalSystem.GetCmdLineArguments()[0].Same(g_strTracing)) {
        
		cout << "[tracing tags (--tracetags=list)]" << endl;
		
		CInternalTrace :: ShowTagInfo(
			tagBuiltinMin,
			tagBuiltinMax,
			s_TraceTagsDescriptions,
			sizeof(s_TraceTagsDescriptions) / sizeof(CInternalTraceTagDesc));

		CInternalTrace :: ShowTagInfo(
			tagAlkalineMin + 1,
			tagAlkalineMax,
			s_TraceAlkalineDescriptions,
			sizeof(s_TraceAlkalineDescriptions) / sizeof(CInternalTraceTagDesc));

		cout << "[tracing levels (--tracelevels=max)]" << endl;

		CInternalTrace :: ShowTagInfo(
			levCrash,
			levVerbose,
			s_TraceLevelsDescriptions,
			sizeof(s_TraceLevelsDescriptions) / sizeof(CInternalTraceTagDesc));

		return 0;
	}

#endif

    if ((LocalSystem.GetCmdLineArguments().GetSize() == 1) && 
        LocalSystem.GetCmdLineArguments()[0].Same(g_strService)) {
        ServiceUsage();
        return -1;
    }

    // not enough arguments
    if (LocalSystem.GetCmdLineArguments().GetSize() < 2) {
        Usage();
        return -1;
    }
    
    CString BindAddress, BindPort;        
    bool bArgZeroInt = CServer::GetBindAddressPort(LocalSystem.GetCmdLineArguments()[0], &BindAddress, &BindPort);

    if (!bArgZeroInt && LocalSystem.GetCmdLineArguments()[0].Same(g_strParse)) {            
        RxParse(LocalSystem);
        return 0;
    } 
        
    if (!bArgZeroInt && LocalSystem.GetCmdLineArguments()[1].Same(g_strRxMatch)) {
        if (LocalSystem.GetCmdLineArguments().GetSize() < 3) {
            Usage();
            return -1;
        }
        RxMatch(LocalSystem.GetCmdLineArguments());
        return 0;
    }        
    
    if (!bArgZeroInt && LocalSystem.GetCmdLineArguments()[1].Same(g_strRxRepl)) {            
        RxRepl(LocalSystem.GetCmdLineArguments());
        return 0;
    } 
    
#ifdef _WIN32
    if (LocalSystem.GetCmdLineArguments()[0].Same(g_strService)) {
        RxService(LocalSystem);
        return 0;
    }
#endif
    
    if (!bArgZeroInt) {
        RxSite(LocalSystem.GetCmdLineArguments(), LocalSystem.GetCmdLineOptions());
        return 0;
    }
    
    RxRun(LocalSystem, true);
    return 0;
}

int main(int argc, char ** argv) {	
	IntializeGlobals(argc, argv);
	int nResult = RunAlkaline(argc, argv);  
	UnInitializeGlobals();
	return nResult;
}


