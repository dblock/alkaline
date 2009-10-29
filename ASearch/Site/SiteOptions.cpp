/*
    © Vestris Inc., Geneva, Switzerland
    http://www.vestris.com, 1994-1999 All Rights Reserved
    ______________________________________________

    written by Daniel Doubrovkine - dblock@vestris.com
  
    Revision history:
    
*/

#include <alkaline.hpp>

#include "Site.hpp"

void CSite::ReadConfig(bool Lazy) {
	
    CString Filename = m_ConfigPath;
    CLocalPath::Terminate(Filename); 
    Filename += "asearch.cnf";
    
    struct_stat NewStat; 
	
	if (base_stat((const char *) Filename.GetBuffer(), &NewStat) != 0) {
		cerr << "[error, " << Filename.GetBuffer() << " does not exist or access denied]" << endl;
		return;
	}

    if (m_Stat.st_mtime == NewStat.st_mtime) 
		return;

    m_Stat = NewStat;
    
    m_SiteConfigs.RemoveAll();
    CLocalFile ConfigFile(Filename);
    CString Line;
    
	if (!m_SiteIndex.m_IndexOptions.m_GatherEmail) 
        cout << "[" << (m_SiteConfigs.GetSize()?"re":"") << "loading " << ConfigFile.GetFullPath() << "] {" << endl;

    if (! ConfigFile.OpenReadBinary() || (ConfigFile.GetSize() <= 0)) {
		cerr << "[error, cannot open " << ConfigFile.GetFullPath() << "]" << endl;
		return;
	}

	CConfig * CurrentConfig = ::new CConfig;
	m_State = CsLoaded;
	int Pos = 0;
	CString ConfigContents; 
	ConfigFile.Read(&ConfigContents);
	while (Pos < (int) ConfigContents.GetLength()) {
		ConfigContents.GetLine(&Line, Pos);
		Line.Trim32();
		if (Line.GetLength()&&(Line[0] == '*')) {
			m_SiteConfigs += (* CurrentConfig);
			delete CurrentConfig;
			CurrentConfig = ::new CConfig;
		} else CurrentConfig->AddOption(Line, !m_SiteIndex.m_IndexOptions.m_GatherEmail);
	}
	m_SiteConfigs+=(* CurrentConfig);
	delete CurrentConfig;
    
    for (int i=0;i<(int) m_SiteConfigs.GetSize();i++) {
        if (!Lazy) {
            m_SiteConfigs[i].SetOption("SLEEPROUNDTRIP","0");
            m_SiteConfigs[i].SetOption("SLEEPFILE", "0");
        }
        m_SiteConfigs[i].Finalize();
    }

    if (m_SiteConfigs.GetSize()) {

        m_CurrentConfiguration = & m_SiteConfigs[0];

		m_CurrentConfiguration->Set("Size", CString::IntToStr(ConfigContents.GetSize()) + " bytes");

        // update global options (first configuration is the default)
        m_SiteIndex.m_IndexOptions.m_FreeAlpha = m_CurrentConfiguration->GetFreeAlpha();
        m_SiteIndex.m_IndexOptions.m_CGI = m_CurrentConfiguration->GetCGI();
        m_SiteIndex.SetExactSize(m_CurrentConfiguration->GetExactSize());
        m_SiteIndex.GetSearcher().SetRightPartialOnly(!m_CurrentConfiguration->GetSearchPartialLeft());   

        CString TmpValue;
        int TmpIntValue;

        if (m_CurrentConfiguration->GetWeightTable().FindAndCopy("Title", TmpValue) && TmpValue.IsInt(& TmpIntValue))
            m_SiteIndex.GetSearcher().SetTitleWeight(TmpIntValue);
        
        if (m_CurrentConfiguration->GetWeightTable().FindAndCopy("Keywords", TmpValue) && TmpValue.IsInt(& TmpIntValue))
            m_SiteIndex.GetSearcher().SetKeywordWeight(TmpIntValue);
        
        if (m_CurrentConfiguration->GetWeightTable().FindAndCopy("Description", TmpValue) && TmpValue.IsInt(& TmpIntValue))
            m_SiteIndex.GetSearcher().SetDescriptionWeight(TmpIntValue);

        if (m_CurrentConfiguration->GetWeightTable().FindAndCopy("Text", TmpValue) && TmpValue.IsInt(& TmpIntValue))
            m_SiteIndex.GetSearcher().SetTextWeight(TmpIntValue);


		m_SiteIndex.GetSearcher().SetMaxWordSize(m_CurrentConfiguration->GetMaxWordSize());
		m_SiteIndex.GetSearcher().SetWeakWords(m_CurrentConfiguration->GetWeakWords());
    
        m_SiteIndex.GetSearcher().SetSearchCacheLife(m_CurrentConfiguration->GetSearchCacheLife());
       
    }    

    if (!m_SiteIndex.m_IndexOptions.m_GatherEmail) cout << "} loaded " << m_SiteConfigs.GetSize() << " inline configuration" << ((m_SiteConfigs.GetSize()>1)?"s":"") << "." << endl;
}

bool CSite::GetCanReindex(void) const {
    if (m_ForceNoReindex) return false;
    for (int i=0;i<(int) m_SiteConfigs.GetSize();i++)
        if (m_SiteConfigs[i].GetCanReindex()) return true;
        return false;
}


void CSite::SetIndexOptions(const CConfig& Config, CIndex& Index) {
    Index.m_IndexOptions.m_RetryCount = Config.GetRetryCount();
    Index.m_IndexOptions.m_Limit = Config.GetSizeLimit();
    Index.m_IndexOptions.m_Proxy = Config.GetProxy();
    Index.SetExts(Config.GetExts());
    Index.m_IndexOptions.m_FreeAlpha = Config.GetFreeAlpha();
    Index.m_IndexOptions.m_CGI = Config.GetCGI();
    Index.m_IndexOptions.m_NSF = Config.GetNSF();
    Index.m_IndexOptions.m_SkipParseLinks = Config.GetSkipParseLinks();
    Index.m_IndexOptions.m_EmptyLinks = Config.GetEmptyLinks();
    Index.SetInsens(Config.GetInsens());
    Index.m_IndexOptions.m_Timeout = Config.GetTimeout();
    if (Config.GetExtsAdd().GetLength()) 
        Index.SetExts(Config.GetExtsAdd(), false);
    if (Config.GetAddExts().GetLength()) 
        Index.SetExts(Config.GetAddExts(), false);
    Index.GetCookieStorage().RemoveAll();
    Index.SetCookies(Config.GetCookies());
    Index.SetIndexHTML(Config.GetOption("Index.html"));
}

void CSite::SetOptions(const CVector<CString>& Options){
    static const CString __THREAD("THREAD_");
    static const CString __NEWONLY("NEWONLY");
    static const CString __NO404("NO404");
    static const CString __ONCE("ONCE");
    static const CString __EXPIRE("EXPIRE");
    static const CString __NOREINDEX("NOREINDEX");
    static const CString __LOG("LOG");
    static const CString __VERBOSE("VERBOSE");
    static const CString __SLEEPROUNDTRIP("SLEEPROUNDTRIP");
    static const CString __SLEEPROUND("SLEEPROUND");
    static const CString __SLEEPFILE("SLEEPFILE");
    static const CString __V("V");
    static const CString __L("L");
    static const CString __SF("SF");
    static const CString __SR("SR");
    static const CString __SSI("SSI");
    static const CString __ENABLESSI("ENABLESSI");
    static const CString __EXV("EXV");
    static const CString __EXX("EXX");
    static const CString __MAXTHREADS("MAXTHREADS=");
    static const CString __MAXINDEXTHREADS("MAXINDEXTHREADS");
    static const CString __ACCEPTINTERVAL("ACCEPTINTERVAL");
    static const CString __MT("MT=");
    static const CString __MI("MI");
    static const CString __AI("AI");
    static const CString __DISABLEPING("DISABLEPING");
    static const CString __ENABLEPING("ENABLEPING");
    static const CString __DISABLESWAP("DISABLESWAP");
    static const CString __ENABLESWAP("ENABLESWAP");
    static const CString __ENABLESWAPWF("ENABLESWAP=");
    static const CString __REGEXP("REGEXP");
    static const CString __TRACETAGS("TRACETAGS=");
    static const CString __TRACELEVELS("TRACELEVELS=");
    
    CString Name, Value;
    int vVal;
    int ePos = -1;
    
    for (int i=0;i<(int) Options.GetSize();i++){
        if (Options[i].StartsWith(__THREAD)) continue;
        if ((Options[i].Same(__SSI))||
            (Options[i].Same(__ENABLESSI))||
            (Options[i].Same(__DISABLEPING)) ||
            (Options[i].Same(__ENABLEPING)) ||
            (Options[i].Same(__DISABLESWAP)) ||
            (Options[i].Same(__ENABLESWAP)) ||
            (Options[i].Same(__REGEXP)) ||
            (Options[i].StartsWithSame(__AI)) ||
           #ifdef _UNIX
            (Options[i].StartsWithSame(__ENABLESWAPWF)) ||
           #endif
            (Options[i].StartsWithSame(__TRACETAGS)) ||
            (Options[i].StartsWithSame(__TRACELEVELS)) ||
            (Options[i].StartsWithSame(__ACCEPTINTERVAL)) ||
            (Options[i].StartsWithSame(__MT))||
            (Options[i].StartsWithSame(__MAXTHREADS))) continue;
        cout << "[" << m_Alias << "][--" << Options[i] << "] - ";
        if (Options[i].Same(__EXV)) {
            cout << "verbose of include/exclude paths" << endl;
            m_EnableExcludeVerbose = true;
        } else if (Options[i].Same(__EXX)) {
            cout << "verbose parser" << endl;
            m_EnableVerboseParser = true;
        } else if (Options[i].Same(__NEWONLY)) {
            cout << "indexing only new documents" << endl;
            m_NewOnly = true;
        } else if (Options[i].Same(__ONCE)) {
            cout << "setting single reindex run" << endl;
            m_Once = true;
        } else if (Options[i].Same(__NO404)) {
            cout << "disabling 404/Not Found stage" << endl;
            m_Enable404 = false;
        } else if (Options[i].Same(__EXPIRE)) {
            cout << "disabling if-Modified-Since" << endl;
            m_ForceExpire = true;
        } else if (Options[i].Same(__NOREINDEX)) {
            cout << "disabling background indexing" << endl;
            m_ForceNoReindex = true;
        } else if ((Options[i].Same(__LOG))||(Options[i].Same(__L))) {
            m_ShowRequests = true;
            cout << "enabling log" << endl;
        } else if ((Options[i].Same(__VERBOSE))||(Options[i].Same(__V))) {
            cout << "enabling verbose" << endl;
            m_Verbose = true;
            // m_SiteIndex.SetVerbose(true);
        } else if ((ePos = Options[i].Pos('=')) > 0) {
            Options[i].Mid(0, ePos, &Name);
            Options[i].Mid(ePos+1, Options[i].GetLength(), &Value);
            vVal = CString::StrToInt(Value);
            if (vVal >= 0) {
                if ((Name.Same(__MAXINDEXTHREADS))||(Name.Same(__MI))){
                    cout << "indexing thread pool: " << vVal << endl;
                    m_IndexPool.SetMaxThreads(vVal);
                } else if ((Name.Same(__SLEEPFILE))||(Name.Same(__SF))){
                    cout << "file sleep interval: " << vVal << " sec" << endl;
                    m_ForceSleepFile = vVal;
                } else if ((Name.Same(__SLEEPROUNDTRIP))||(Name.Same(__SR))||(Name.Same(__SLEEPROUND))) {
                    cout << "roundtrip sleep interval: " << vVal << " sec" << endl;
                    m_ForceSleepRoundtrip = vVal;
                } else cout << "invalid option" << endl;
            } else cout << "invalid value: " << Value << endl;
        } else cout << "invalid option" << endl;
    }
}
