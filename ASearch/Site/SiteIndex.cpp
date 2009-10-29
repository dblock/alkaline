/*
    © Vestris Inc., Geneva, Switzerland
    http://www.vestris.com, 1994-1999 All Rights Reserved
    ______________________________________________

    written by Daniel Doubrovkine - dblock@vestris.com
  
    Revision history:
    
*/

#include <alkaline.hpp>

#include "Site.hpp"
#include <Main/TraceTags.hpp>

CString CSite::ProcessURLSingle(const CString& Url, bool Lazy) {
    
    bool Verbose = (!Lazy && (!m_SiteIndex.m_IndexOptions.m_GatherEmail)) || m_Verbose;

    LoadSiteIndex();
    
    CString Result;

    CIndexObject IndexObject;
    IndexObject.SetIndexObjectType(iotUrl);
    IndexObject.SetUrl(Url);
    IndexObject.SetDepth(0);
    IndexObject.SetLazy(Lazy);    
    IndexObject.SetpSite(this);   

    CUrl ClassURL(Url);
    CString StringURL(ClassURL.GetHttpAll());

    m_ProcessedFiles.Inc();
    
    Result = "[" + StringURL + "] (console) - ";
    if (Verbose) {
        cout << "[" << StringURL << "] (console) - "; cout.flush();
    }

    CVector<CString> Links;    
    CRemoteFile RemoteFile;    
    CVector<CString> FinalURLs;
    CString Server;
 
    m_SiteIndex.Index(
        RemoteFile, 
        ClassURL, 
        Server, 
        Links, 
        this, 
        FinalURLs, 
        IndexObject, 
        &Result);
        
    for (int j=0;j<(int) FinalURLs.GetSize();j++) {                
        m_IndexMutex.StartWriting();
        m_UniqueLinks.Add(FinalURLs[j]);
        m_LastProcessedUrl = FinalURLs[j]; 
        m_IndexMutex.StopWriting();        
    }

    Write(Lazy, true);
    return Result;
}

bool CSite::OnNewUrl(
    CString& TranslatedUrl, 
    const CString& Server,
    const CIndexObject& IndexObject,
    CString& OutputString
    ) {


    CUrl ServerUrl(Server);
    CUrl SearchTopUrl(IndexObject.GetSearchTop());

    bool bServerLocal = 
        (ServerUrl.GetPortValue() == SearchTopUrl.GetPortValue()) &&
        (ServerUrl.GetHost().Same(SearchTopUrl.GetHost(), ! m_CurrentConfiguration->GetInsens()));
    
    m_CurrentConfiguration->RedirectUrl(
        TranslatedUrl, 
        (m_EnableExcludeVerbose?(&OutputString):(CString*)NULL));

    m_CurrentConfiguration->ReplaceUrl(
        TranslatedUrl, 
        (m_EnableExcludeVerbose?(&OutputString):(CString*)NULL));

    /* explicit exclude */
    if (IsExcluded(TranslatedUrl)) {
        if (m_EnableExcludeVerbose) {
            OutputString += ("   {URL} " + TranslatedUrl + " explicitely excluded by UrlExclude directive.\n");            
        }
        return false;
    }
    
    CUrl uUrl(TranslatedUrl);
    
    bool bStartsWithServer = 
        (uUrl.GetPortValue() == ServerUrl.GetPortValue()) &&
        (TranslatedUrl.StartsWith(Server, ! m_CurrentConfiguration->GetInsens()));
    
    /* Remote=N and remote URL */
    if ((!bStartsWithServer) && (!m_CurrentConfiguration->GetCanRemote())) {
        if (m_EnableExcludeVerbose) {
            OutputString += ("   {URL} " + TranslatedUrl + " excluded because remote from " + Server + " and Remote is set to No.\n");
        }
        return false;
    }
    /* explicit exclude with IsIncluded */
    if (!IsIncluded(TranslatedUrl)) {
        if (m_EnableExcludeVerbose) {
            OutputString += ("   {URL} " + TranslatedUrl + " explicitely excluded by UrlInclude directive.\n");
        }
        return false;
    }
    
    if (bStartsWithServer && bServerLocal) {
        /* URL is local */    
        
        bool bStartsWithSearchTop = 
            (uUrl.GetPortValue() == SearchTopUrl.GetPortValue()) &&
            (TranslatedUrl.StartsWith(IndexObject.GetSearchTop(), ! m_CurrentConfiguration->GetInsens()));
        
        if (!m_CurrentConfiguration->GetCanUpper() && (!bStartsWithSearchTop)) {
            if (m_EnableExcludeVerbose) {
                OutputString += ("   {URL} " + TranslatedUrl + " excluded because upper than the top URL " + IndexObject.GetSearchTop() + " and Upper=N.\n");
            }
            return false;
        }        
    } else if (!m_CurrentConfiguration->GetCanRemote()) {
        // the url can be explicitely included if UrlInclude has entries
        if (!m_CurrentConfiguration->GetIncludePaths().GetSize()) {          
            if (m_EnableExcludeVerbose) {
                OutputString += ("   {URL} " + TranslatedUrl + " excluded because remote from " + Server + ", Remote=N and not included by UrlInclude.\n");
            }
            return false;
        }
    }
    
    /* Depth setting */          
    if (IsExcludedDepth(uUrl)) {
        if (m_EnableExcludeVerbose) {
            OutputString += ("   {URL} " + TranslatedUrl + " excluded by a Depth setting.\n");            
        }
        return false;
    }
    
    /* URL already in the list */

    // this is the last thing that needs to be checked, or the url will be added to unique links and
    // never indexed although it has a legitimate linkage to it (dblock)

    bool bNewUrl = false;
    
    m_IndexMutex.StartWriting();
    m_UniqueLinks.Add(TranslatedUrl, &bNewUrl);
    m_IndexMutex.StopWriting();
    
    if (!bNewUrl) {
        if (m_EnableExcludeVerbose) {
            OutputString += ("   {URL} " + TranslatedUrl + " was already scheduled by a different thread.\n");            
        }
        return false;
    }
    
    // add url

    if (m_EnableExcludeVerbose) {        
        OutputString += ("   {URL} " + TranslatedUrl + " successfully added.\n");
    }
    
    return true;
}

void CSite::ProcessURL(const CIndexObject& IndexObject) {

    int Depth = IndexObject.GetDepth();    

    CString OutputString;  
    CUrl ClassURL(IndexObject.GetUrl());
    CString StringURL(ClassURL.GetHttpAll());  
    bool Verbose = (!IndexObject.GetLazy() && (!m_SiteIndex.m_IndexOptions.m_GatherEmail)) || m_Verbose; 
    
    if (Verbose) {
        OutputString = "[" + StringURL + "] (" + CString::IntToStr(Depth) + "/" + CString::IntToStr(IndexObject.GetRemoteHistory().GetSize()) + ") - ";
    }

    // robots directive
    if (IsExcludedRobots(IndexObject.GetUrl(), IndexObject.GetLazy())) {
        if (Verbose) {
            cout << OutputString << "[excluded by robots directive]" << endl;
        }
        return;
    }
    
    // maximum files limit
    if (IsMaxFilesLimitReached()) {
        if (Verbose) {
            cout << OutputString << "[maximum files limit reached]" << endl;            
        }       
        return;
    }

    if (g_pHandler->GetSignalSigterm()) {
        return;
    }

    CRemoteFile RemoteFile;
    CString Server; 
    int i, j;
    
    CVector<CString> Links;
    CVector<CString> FinalURLs;
    CVector<CString> CurrentLinks;

    int l_ModifiedFiles = m_SiteIndex.m_IndexOptions.m_ModifiedCount.Get();
    
    // _L_DEBUG(7, cout << "CSite::ProcessURL() - FinalURLs() - Index Init." << endl);    
    
    m_ProcessedFiles.Inc();
    m_ProcessedLinks.Inc();
 
    // _L_DEBUG(7, cout << "CSite::ProcessURL() - FinalURLs() - Incremented processed files." << endl);    
    
    Write(IndexObject.GetLazy());    

    // _L_DEBUG(7, cout << "CSite::ProcessURL() - FinalURLs() - Write through." << endl);    
    
    m_IndexMutex.StartWriting();
    m_LastProcessedUrl = IndexObject.GetUrl();    
    m_IndexMutex.StopWriting();

    m_SiteIndex.Index(
        RemoteFile, 
        ClassURL, 
        Server, 
        Links,         
        this, 
        FinalURLs,
        IndexObject,
        & OutputString);    
   
    // _L_DEBUG(7, cout << "CSite::ProcessURL() - FinalURLs() - Index Done." << endl);
    for (j=0;j<(int) FinalURLs.GetSize();j++) {
        
        // if ((Verbose) && (j != (int) FinalURLs.GetSize() - 1)) {
        // cout << "\t[301,302/Document Moved][" << FinalURLs[j] << "]" << endl;
        // OutputString += ("\t[301,302/Document Moved][" + FinalURLs[j] + "]\n");
        // }
        
        m_IndexMutex.StartWriting();
        m_UniqueLinks.Add(FinalURLs[j]);
        m_IndexMutex.StopWriting();
        
    }

    if (GetSleepFile() && (m_SiteIndex.m_IndexOptions.m_ModifiedCount.Get() != l_ModifiedFiles)) {
        base_sleep(GetSleepFile());
    }
    
    // _L_DEBUG(7, cout << "CSite::ProcessURL() - FinalURLs() - done." << endl);
    if (Depth || (Depth == -1)) {
        
        for (i=0;i<(int) Links.GetSize();i++) {
          
            CString Filename = Links[i];

            if (! OnNewUrl(
                Filename, 
                Server,
                IndexObject,
                OutputString))
                continue;

            m_IndexMutex.StartWriting();
            CurrentLinks += Filename;
            m_IndexMutex.StopWriting();

        }        
    }
    
    for (i=((int) CurrentLinks.GetSize())-1;i>=0;i--) {
        
        if (g_pHandler->GetSignalSigterm())
            break;
        
        if (IsMaxFilesLimitReached()) 
            break;
        
        AddIndexJob(
            CurrentLinks[i], 
            Depth - 1, 
            IndexObject.GetLazy(), 
            IndexObject.GetSearchTop(), 
            IndexObject.GetRemoteHistory());
    }
    
    if (Verbose) {
      cout << OutputString;
      cout.flush();
    }
    
}

bool CSite::IsExcludedDepth(const CUrl& Url) const {
    
    /* Site Depth (actually path depth from root) */
    
    if (m_CurrentConfiguration->GetSiteDepth() >= 0) {
        int SiteDepth = 0;
        for (register int j=0;j<(int) Url.GetHttpDirectory().GetLength();j++) {
            if (Url.GetHttpDirectory()[j] == '/') {
                if (SiteDepth > m_CurrentConfiguration->GetSiteDepth()) {
                    // _L_DEBUG(7, cout << "Rejecting " << Url.GetBrute() << " because of SiteDepth." << endl);
                    return true;
                }
                SiteDepth++;
            }
        }
    }
    return false;
}

void CSite::RetrieveRobots(const CUrl& Url, CVector<CString>& ExcludeVector, bool Lazy) {
    
    CString RobotsServer = Url.GetHttpServer();
    
    if (m_CurrentConfiguration->GetCanRobots()) 
        LoadRobots(RobotsServer, ExcludeVector, Lazy);    
}

bool CSite::IsExcludedRobots(const CUrl& Url, bool Lazy) {
    
    CVector<CString> ExcludeVector;

    RetrieveRobots(Url, ExcludeVector, Lazy);

    bool bIsExcluded = false;
    for (int i=0;i<(int) ExcludeVector.GetSize();i++) {
        if (Url.GetHttpDirectory().StartsWith(ExcludeVector[i]))  {
            bIsExcluded = true;
            break;
        }
    }
    
    return bIsExcluded;
        
}

bool CSite::IsExcluded(const CString& iUrl){

    /* Exclusion Paths */
    
    int i;
    
    for (i=0;i<(int) m_CurrentConfiguration->GetExcludePaths().GetSize();i++) {
        if (iUrl.StartsWith(m_CurrentConfiguration->GetExcludePaths()[i])) 
            return true;
    }
    
    for (i=0;i<(int) m_CurrentConfiguration->GetRegexp_ExcludePaths().GetSize();i++) {
        if (CRegExp::Match(iUrl, m_CurrentConfiguration->GetRegexp_ExcludePaths()[i]))
            return true;            
    }
    
    return false;
}

bool CSite::IsIncluded(const CString& iUrl) {

    /* Inclusion Definitions */
    int i;
    
    unsigned int TotalSize = 
        m_CurrentConfiguration->GetIncludePaths().GetSize() + 
        m_CurrentConfiguration->GetRegexp_IncludePaths().GetSize();
    
    if (! TotalSize)
        return true;
    
    CUrl iUrlStructure(iUrl);
    for (i=0;i<(int) m_CurrentConfiguration->GetIncludePaths().GetSize();i++) {
        CUrl iUrlInclude(m_CurrentConfiguration->GetIncludePaths()[i]);      
        if (iUrlStructure.Includes(iUrlInclude)) {
            return true;
        }
    }
    
    for (i=0;i<(int) m_CurrentConfiguration->GetRegexp_IncludePaths().GetSize();i++) {
        if (CRegExp::Match(iUrl, m_CurrentConfiguration->GetRegexp_IncludePaths()[i])) {          
            return true;
        }
    }
    
    return false;
}

void CSite::AddIndexJob(
    const CString& Url, 
    int Depth, 
    bool Lazy, 
    const CString& SearchTop,
    const CVector<CUrl>& RemoteHistory
    ) {
    
    CIndexObject IndexObject;
    IndexObject.SetIndexObjectType(iotUrl);
    IndexObject.SetUrl(Url);
    IndexObject.SetDepth(Depth);
    IndexObject.SetLazy(Lazy);
    IndexObject.SetSearchTop(SearchTop);
    IndexObject.SetpSite(this);
    
    // build the remote history of this url being indexed and check whether it's beyond our limits    
    
    CUrl ThisUrl(Url);
    
    for (int i = 0; i < (int) RemoteHistory.GetSize(); i++) {
        if (RemoteHistory[i].GetHost().Same(ThisUrl.GetHost(), ! m_CurrentConfiguration->GetInsens()) &&
            RemoteHistory[i].GetPort() == ThisUrl.GetPort() &&
            RemoteHistory[i].GetScheme().Same(ThisUrl.GetScheme())) {
            // we are back to a hop within the list
            break;
        } else {
            IndexObject.GetRemoteHistory().Add(RemoteHistory[i]);
        }
    }
    
    if (IsMaxRemoteDepthReached(IndexObject.GetRemoteHistory())) {
        Trace(tagIndex, levInfo, ("CSite :: IsMaxRemoteDepthReached() for %s.", Url.GetBuffer()));
        return;
    }
    
    IndexObject.GetRemoteHistory().Add(ThisUrl);

    Trace(tagIndex, levInfo, ("CSite :: adding job for %s.", Url.GetBuffer()));
    
    m_IndexPool.AddIndexJob(IndexObject);  
}

void CSite::AddRobotsJob(const CString& Url, bool Lazy) {
    CIndexObject IndexObject;
    IndexObject.SetIndexObjectType(iotRobot);
    IndexObject.SetUrl(Url);    
    IndexObject.SetLazy(Lazy);    
    IndexObject.SetpSite(this);
    m_IndexPool.AddIndexJob(IndexObject);  
}

void CSite::ProcessURL(const CString& Url, bool Lazy, const CString& SearchTop, const CVector<CUrl>& RemoteHistory) {
    if (IsMaxFilesLimitReached()) 
        return;
    if (g_pHandler->GetSignalSigterm())
        return;

    m_IndexMutex.StartWriting();
    m_LastProcessedUrl = Url;
    m_IndexMutex.StopWriting();

    // _L_DEBUG(7, cout << "CSite::ProcessURL(A) - [" << Url << "][" << SearchTop << "]" << endl);
    int Depth = m_CurrentConfiguration->GetDepth();
    AddIndexJob(Url, Depth, Lazy, SearchTop, RemoteHistory);    
}

bool CSite::GetFinalURL(const CString& FileName, CString& FinalUrl, bool Verbose) {
    
    CString Result;
    CRemoteFile RemoteFile(FileName);
    RemoteFile.SetMaxSize(0);
    AppendRequestHeaders(RemoteFile);
    RemoteFile.SetTimeout(m_SiteIndex.m_IndexOptions.m_Timeout);
    RemoteFile.SetMaxSize(0);
    RemoteFile.SetVerbose(false);    
    RemoteFile.SetAuth(GetAuth());

    if (Verbose)
        cout << "[retrieving " << FileName << "]" << endl;

    if (g_pHandler->GetSignalSigterm())
        return true;

    m_SiteIndex.IssueRequest(& RemoteFile, this, & Result);

    CVector<CString> Redirections;
    
    Redirections = RemoteFile.GetRRedirections();
    
    if (Redirections.GetSize() > 1) {
        FinalUrl = Redirections[Redirections.GetSize() - 1];
    }

    if (Verbose) {
        cout << Result << "[saved as " << FinalUrl << "]" << endl;
    }
    
    return true;
}

void CSite::LoadRobots(const CString& Server, CVector<CString>& ExcludeVector, bool Lazy){

    bool bFound = false;

    m_RobotsMutex.StartReading();
    bFound = m_ExcludeTable.FindAndCopy(Server, ExcludeVector);
    m_RobotsMutex.StopReading();

    if (bFound)
        return;
    
    if (g_pHandler->GetSignalSigterm())
        return;
        
    m_RobotsMutexTable.StartWriting(Server);

    m_RobotsMutex.StartReading();
    bFound = m_ExcludeTable.FindAndCopy(Server, ExcludeVector);
    m_RobotsMutex.StopReading();

    if (! bFound) {
        
        RetrieveRobot(Server, ExcludeVector, Lazy);

        m_RobotsMutex.StartWriting();
        m_ExcludeTable.Set(Server, ExcludeVector);
        m_RobotsMutex.StopWriting();

    }

    m_RobotsMutexTable.StopWriting(Server);

}

void CSite::RetrieveRobot(const CString& Server, CVector<CString>& NewExclude, bool Lazy) {

    if (! Server.StartsWithSame("http://"))
        return;

    bool Verbose = (!Lazy && (!m_SiteIndex.m_IndexOptions.m_GatherEmail)) || m_Verbose;
    CString RobotsFilename = Server;
    RobotsFilename += "/robots.txt";
    
    CRemoteFile RobotsFile(RobotsFilename);

    NewExclude.RemoveAll();
    
    if (Verbose)
        cout << "[robots request for " << RobotsFilename << "]" << endl;
    
    RobotsFile.SetVerbose(false);
    RobotsFile.SetAuth(GetAuth());
    AppendRequestHeaders(RobotsFile);
    RobotsFile.SetTimeout(m_SiteIndex.m_IndexOptions.m_Timeout);

    // RobotsFile.Get(m_SiteIndex.m_IndexOptions.m_Proxy);

    CString Result;
    m_SiteIndex.IssueRequest(& RobotsFile, this, & Result);

    if (RobotsFile.GetRStatusValue() == 200) {
        
        CString VerboseResult;
        
        CString RobotsData = RobotsFile.GetRData();
        CString Line;
        CString Name;
        CString Value;
        bool RobotInside = false;
        while (RobotsData.GetLength()) {
            RobotsData.ExtractLine(&Line);
            int mPos = Line.Pos('#');
            if (mPos >= 0) Line.Delete(mPos, Line.GetLength());
            int cPos = Line.Pos(':');
            if (cPos > 0) {
                Line.Mid(0, cPos, &Name); Name.Trim32();
                Line.Mid(cPos+1, Line.GetLength(), &Value); 
                Value.Trim32();
                if (Name.Same(g_strHttpUserAgent)){
                    if ((Value == g_strStar)||(Value.StartsWithSame("AlkalineBOT")))
                        RobotInside = true;
                    else RobotInside = false;
                } else if (RobotInside && (Name.Same(g_strHttpDisallow)) && (Value.GetLength())) {
                    if (Value[0] != '/') {
                        CString NewValue("/");
                        NewValue += Value;
                        Value = NewValue;
                    }                        
                    if (Verbose) 
                        VerboseResult += "[" + Value + "]";
                    NewExclude += Value;
                }
            }
        }

        if (Verbose && NewExclude.GetSize()) {
                cout << "[" << RobotsFilename << "]" << Result << "[" 
                     << CString::IntToStr(RobotsFile.GetRData().GetLength()) + " bytes]" << endl
                     << "  " << VerboseResult << endl;
        }

    } else {            
        if (Verbose) {
            cout << "[" << RobotsFilename << "][" << CHttpIo::GetRStatusString(RobotsFile.GetRStatusValue()) << "]" << endl
                << "  [" << RobotsFile.GetHttpRequest().GetRStatusString() << "]" << endl;
        }        
    }
}

// returns true when done
bool CSite::ProcessPages(
    const CString& ProcessType, // "IncludePages" or "ExcludePages"
    CHashTable<CStringVector>& Table, // m_CurrentConfiguration->GetPagesIncludeTable()
    CHashTable<CStringVector>& RegexpTable, // m_CurrentConfiguration->GetRegexp_PagesIncludeTable()
    bool bOverride, // m_CurrentConfiguration->GetIncludePagesAll()
    const CString& Type, // *, an extension or a mime type
    const CVector<CStringA>& Words, // words to process
    int * pTotalSize, // total result set
    CString * Result, // result messages
    bool * pbResult // continue processing
    ) {
    
    int i,j;

    CVector<CString> WordsAll;
    CVector<CString> RegexpWordsAll;

    CollectLoadArray(
        ProcessType,
        Table,
        Type,
        WordsAll);

    CollectLoadArray(
        ProcessType,
        RegexpTable,
        Type,
        RegexpWordsAll);

    (* pTotalSize) += WordsAll.GetSize();
    (* pTotalSize) += RegexpWordsAll.GetSize();
    
    // includewords and regexp includewords
    
    {
        bool Found = false;        
        bool bRegExp;
        CString Word;
        
        for (j = 0; j < (int) WordsAll.GetSize() + (int) RegexpWordsAll.GetSize(); j++) {
            for (i=((int)Words.GetSize())-1;i>=0;i--) {
                
                bRegExp = (j < (int) WordsAll.GetSize()) ? false : true;
                
                Word = 
                    bRegExp ? 
                    RegexpWordsAll[j - (int) WordsAll.GetSize()] :
                    WordsAll[j];
                
                if (Words[i].IsEqualMeta(Word, bRegExp)) {
                    if (! bOverride) {
                        if (m_EnableExcludeVerbose) {
                            * Result += "\n   {IW} Explicitly included because " + ProcessType + "All=N and found or matched {" + Word + "}\n";
                        }
                        * pbResult = true;
                        return true;
                    }
                    Found = true;
                    break;
                }
            }
            
            if (!Found && bOverride) {
                if (m_EnableExcludeVerbose) {
                    * Result += "\n   {IW} Explicitly excluded because " + ProcessType + "All=Y and unable to find or match {" + Word + "}\n";
                }
                * pbResult = false;
                return true;
            }
        }
    }
    
    return false;
}

bool CSite::PluginPages(
    const CString& Directive, // "IncludePages",
    CHashTable<CStringVector>& Table, // m_CurrentConfiguration->GetPagesIncludeTable(), 
    CHashTable<CStringVector>& RegexpTable, // m_CurrentConfiguration->GetRegexp_PagesIncludeTable(), 
    bool bOverride, // m_CurrentConfiguration->GetIncludePagesAll(),
    const CString& Extension, 
    const CString& MimeType, 
    const CVector<CStringA>& Words, 
    CString * Result
    ) {
    
    int TotalSize = 0;    
    bool bResult = false;
    
    if (
        ProcessPages(
            Directive, // "IncludePages", 
            Table, // m_CurrentConfiguration->GetPagesIncludeTable(), 
            RegexpTable, // m_CurrentConfiguration->GetRegexp_PagesIncludeTable(),
            bOverride, // m_CurrentConfiguration->GetIncludePagesAll(),
            g_strStar, 
            Words, 
            & TotalSize, 
            Result, 
            & bResult))
        return bResult;

    if (ProcessPages(
            Directive, // "IncludePages", 
            Table, // m_CurrentConfiguration->GetPagesIncludeTable(), 
            RegexpTable, // m_CurrentConfiguration->GetRegexp_PagesIncludeTable(),
            bOverride, // m_CurrentConfiguration->GetIncludePagesAll(),
            Extension, 
            Words, 
            & TotalSize, 
            Result, 
            & bResult))
        return bResult;

    if (ProcessPages(
            Directive, // "IncludePages", 
            Table, // m_CurrentConfiguration->GetPagesIncludeTable(), 
            RegexpTable, // m_CurrentConfiguration->GetRegexp_PagesIncludeTable(),
            bOverride, // m_CurrentConfiguration->GetIncludePagesAll(),
            MimeType, 
            Words,
            & TotalSize, 
            Result, 
            & bResult))
        return bResult;
    
    if (TotalSize) {
        if (! bOverride) {
            if (m_EnableExcludeVerbose) {
                * Result += "\n   {IP} Explicitly excluded because none of the " + CString::IntToStr(TotalSize) + " " + Directive + " found.\n";
            }
            return false;
        } else {
            if (m_EnableExcludeVerbose) {
                * Result += "\n   {IP} Explicitly included because all of the " + CString::IntToStr(TotalSize) + " " + Directive + " found.\n";
            }
            return true;
        }
    } else {
        if (m_EnableExcludeVerbose) {
            * Result += "\n   {IP} Implicitly included because no " + Directive + " have been defined.\n";
        }
        return true;
    }    
}
    
    
/* index pages containing Words only */
bool CSite::IncludePluginPages(
    const CString& Extension, 
    const CString& MimeType, 
    const CVector<CStringA>& Words, 
    CString * Result) {
    
    return 
        PluginPages(
            "IncludePages",
            m_CurrentConfiguration->GetPagesIncludeTable(), 
            m_CurrentConfiguration->GetRegexp_PagesIncludeTable(), 
            m_CurrentConfiguration->GetIncludePagesAll(),
            Extension, 
            MimeType, 
            Words, 
            Result);
}

/* exclude pages containing one or all exclude words */
bool CSite::ExcludePluginPages(
    const CString& Extension, 
    const CString& MimeType, 
    const CVector<CStringA>& Words, 
    CString * Result) {
    
    return 
        PluginPages(
            "ExcludePages", 
            m_CurrentConfiguration->GetPagesExcludeTable(), 
            m_CurrentConfiguration->GetRegexp_PagesExcludeTable(),
            m_CurrentConfiguration->GetExcludePagesAll(),
            Extension,
            MimeType,
            Words,
            Result);
}

bool CSite::IndexWords(
    const CString& Directive, // "IndexWords"
    const CString& Type, // g_strStar
    CHashTable<CStringVector> & Table, // m_CurrentConfiguration->GetWordsIndexTable()
    bool bRegExp,
    CVector<CStringA>& Words,
    CVector<CStringA>& NewWords
    ) {
    
    CVector<CString> Vector;
    
    CollectLoadArray(
        Directive,
        Table,
        Type,
        Vector
        );
    
    if (! Vector.GetSize())
        return false;
    
    int i,j;
    for (i = ((int)Words.GetSize()) - 1; i >= 0; i--) {
        for (j = 0; j < (int) Vector.GetSize(); j++) {
            if (Words[i].IsEqualMeta(Vector[j], bRegExp)) {
                NewWords += Words[i];
                Words.RemoveAt(i);
                break;
            }
        }
    }
    
    return true;
}
    
    
bool CSite::ExecuteIndexWords(const CString& Extension, const CString& MimeType, CVector<CStringA>& Words){

    CVector<CStringA> NewWords;
    
    bool bResult = false;
    
    bResult |= IndexWords("IndexWords", g_strStar, m_CurrentConfiguration->GetWordsIndexTable(), false, Words, NewWords);
    bResult |= IndexWords("Regexp IndexWords", g_strStar, m_CurrentConfiguration->GetRegexp_WordsIndexTable(), true, Words, NewWords);
    
    bResult |= IndexWords("IndexWords", Extension, m_CurrentConfiguration->GetWordsIndexTable(), false, Words, NewWords);
    bResult |= IndexWords("Regexp IndexWords", Extension, m_CurrentConfiguration->GetRegexp_WordsIndexTable(), true, Words, NewWords);

    bResult |= IndexWords("IndexWords", MimeType, m_CurrentConfiguration->GetWordsIndexTable(), false, Words, NewWords);
    bResult |= IndexWords("Regexp IndexWords", MimeType, m_CurrentConfiguration->GetRegexp_WordsIndexTable(), true, Words, NewWords);
    
    // true if there were IndexWords directives that lead to a list of words
    if (bResult) {
        Words = NewWords;    
    }
    
    return bResult;
}
    
void CSite::CollectLoadArray(
    const CString& ProcessType,
    CHashTable<CStringVector>& Table, // m_CurrentConfiguration->GetWordsExcludeTable()
    const CString& Type, // g_strStar
    CVector<CString>& Vector // WordsExcludeAll
    ) {
    
    CString PagesType(ProcessType);
    
    if (Type != CString(g_strStar)) {
        PagesType += ' ';
        PagesType += Type;
    }
    
    m_IndexMutex.StartReading();
    bool bFound = Table.FindAndCopy(Type, Vector);
    m_IndexMutex.StopReading();

    if (! bFound) {
        CString Filename = m_CurrentConfiguration->GetOption(PagesType);
        if (Filename.GetLength()) {            
            m_IndexMutex.StartWriting();
            if (! Table.FindAndCopy(Type, Vector)) {
                CVector<CString> FVector;
                CString::StrToVector(Filename, ',', &FVector);
                CConfig :: AddFromFilesToVector(PagesType, Vector, FVector);
                Table.Set(Type, Vector);
            }
            m_IndexMutex.StopWriting();
        }
    }
}

void CSite::ExcludeWords(
    const CString& Directive, // "ExcludeWords"
    const CString& Type, // g_strStar
    CHashTable<CStringVector> & Table, // m_CurrentConfiguration->GetWordsExcludeTable()
    bool bRegExp,
    CVector<CStringA>& Words
    ) {
    
    CVector<CString> Vector;
    
    CollectLoadArray(
        Directive,
        Table,
        Type,
        Vector
        );
    
    int i,j;
    for (i = ((int)Words.GetSize()) - 1; i >= 0; i--) {
        for (j = 0; j < (int) Vector.GetSize(); j++) {
            if (Words[i].IsEqualMeta(Vector[j], bRegExp)) {
                Words.RemoveAt(i);
                break;
            }
        }
    }    
}

/* exclude words from indexing */
bool CSite::ExecutePluginWords(const CString& Extension, const CString& MimeType, CVector<CStringA>& Words) {

    ExcludeWords("ExcludeWords", g_strStar, m_CurrentConfiguration->GetWordsExcludeTable(), false, Words);
    ExcludeWords("Regexp ExcludeWords", g_strStar, m_CurrentConfiguration->GetRegexp_WordsExcludeTable(), true, Words);
    
    ExcludeWords("ExcludeWords", Extension, m_CurrentConfiguration->GetWordsExcludeTable(), false, Words);
    ExcludeWords("Regexp ExcludeWords", Extension, m_CurrentConfiguration->GetRegexp_WordsExcludeTable(), true, Words);

    ExcludeWords("ExcludeWords", MimeType, m_CurrentConfiguration->GetWordsExcludeTable(), false, Words);
    ExcludeWords("Regexp ExcludeWords", MimeType, m_CurrentConfiguration->GetRegexp_WordsExcludeTable(), true, Words);
    
    return true;
}

bool CSite::ExecuteObject(const CStringTable& ActiveXObject, CString& TargetData, bool Verbose, CString * Result) const {
	
    CString ClassID = ActiveXObject.GetValue("CLASSID");
    
    CString ObjectCmdline;
    if (!m_CurrentConfiguration->GetObjectFilters().FindAndCopy(ClassID, ObjectCmdline)) {
        // no Object CLSID= defined
        return false;
    }
    if (Verbose) { cout << "[" << ClassID << "]"; cout.flush(); }
    if (Result) { * Result += ("[" + ClassID + "]"); }
    /* retrieve the documented on demand, if a parameters such as ObjectDocument CLSID= exists */
    CString ObjectDocument = m_CurrentConfiguration->GetOption("ObjectDocument " + ClassID);

	bool bResult = false;

    if (ObjectDocument.GetLength()) {
        // cout << "Retrieving document defined by " << ObjectDocument << " => " << ActiveXObject.GetValue(ObjectDocument) << endl;
        CUrl BaseUrl(ActiveXObject.GetValue("BASE"));
        CUrl ResolvedUrl(BaseUrl.Resolve(ActiveXObject.GetValue(ObjectDocument)));
        CString ObjectData;
        if (m_SiteIndex.RetrieveFile((CSite *) this, ResolvedUrl.GetHttpAll(), &ObjectData)) {
            // cout << "Retrieved " << ObjectData.GetLength() << " bytes." << endl;
            if (Verbose) { cout << "[" << ObjectData.GetLength() << " bytes]"; cout.flush(); }
            if (Result) { * Result += "[" + CString::IntToStr(ObjectData.GetLength()) + " bytes]"; }
            /* create a temporary source and target */
            
            CString SourceFilename = CLocalFile::GetTmpName();
            CString TargetFilename = CLocalFile::GetTmpName();

            /* write temporary data */

			CMMapFile SourceStream(SourceFilename);
  
			if (! SourceStream.MMap(MMAP_OPENMODE, ObjectData.GetLength())) {
				if (Verbose) { cerr << "[error creating " << SourceFilename << ", " << ObjectData.GetLength() << " bytes]"; cout.flush(); }
				if (Result) { * Result += "[error creating " + SourceFilename + ", " + CString::IntToStr(ObjectData.GetLength()) + " bytes]"; }
				return false;
			}

            SourceStream.Write((void *) ObjectData.GetBuffer(), ObjectData.GetLength());
            SourceStream.MMUnmap();

            /* map command line */
            ((CStringTable&)ActiveXObject).Set("SOURCEFILE", SourceFilename);
            ((CStringTable&)ActiveXObject).Set("TARGETFILE", TargetFilename);
            
			bResult = ExecuteAndReadFilter(
				ActiveXObject.Map(ObjectCmdline),
				TargetFilename,
				TargetData,
				Result,
				Verbose
				);

			CLocalFile::Delete(SourceFilename);
        }
    }

    return bResult;
}

bool CSite::ExecuteAndReadFilter(
	 const CString& CommandLine,
	 const CString& TargetFilename,
	 CString& TargetData,
	 CString * Result,
	 bool Verbose) const {

	bool bResult = false;

	Trace(tagIndex, levInfo, ("CSite :: Filter - [%s].", CommandLine.GetBuffer()));

	int SysRes = system((const char *) CommandLine.GetBuffer());

	if (SysRes != -1) {
		
		CMMapFile TargetStream(TargetFilename);

		if (TargetStream.MMap()) {

			Trace(tagIndex, levInfo, ("CSite :: Filter - target data is %d bytes.", TargetStream.GetSize()));

			TargetStream.Read(& TargetData, TargetStream.GetSize());			
			TargetStream.MMUnmap();

			bResult = true;
		} else {
			if (Verbose) { cout << "\t[filter error: target file is empty or system error]" << endl; }
			if (Result) { * Result += "\t[filter error: target file is empty or system error]\n"; }
			bResult = false;
		}


	} else {
		if (Verbose) { cout << "\t[filter error: system call has failed]" << endl; }
		if (Result) { * Result += "\t[filter error: system call has failed]\n"; }
		bResult = false;
	}

	CLocalFile::Delete(TargetFilename);

	return bResult;
}

bool CSite::ExecutePlugin(
	const CString& Data, 
	const CString& Extension, 
	const CString& MimeType, 
	const CString& FileName, 
	CString& TargetData, 
	CString * Result) const {

    CString FilterName, FilterCmdline;
    
    // get the filter command line
    
    bool Mime = false;
    
    FilterCmdline = m_CurrentConfiguration->GetOption("Filter " + MimeType);
    if (FilterCmdline.GetLength()) {
        if (Result) (* Result) += "[mime filter: " + MimeType + "]";
        Mime = true;
    }
    
    if (!FilterCmdline.GetLength()) {
        FilterCmdline = m_CurrentConfiguration->GetOption("Filter " + Extension);                     
        if (FilterCmdline.GetLength()) {
            if (Result) (* Result) += "[ext filter: " + Extension + "]";
        }
    }
    
    if (!FilterCmdline.GetLength()) {
        FilterCmdline = m_CurrentConfiguration->GetOption("Filter");        
        if (FilterCmdline.GetLength()) {
            if (Result) (* Result) += "[generic filter: " + Extension + " (" + MimeType + ")]";
        }
    }
    
    if (!FilterCmdline.GetLength())
        return false;
    
    // produce the source file for processing
        
	CString SourceFilename = CLocalFile::GetTmpName();
    CString TargetFilename = CLocalFile::GetTmpName();

	CMMapFile SourceStream(SourceFilename);
  
	if (! SourceStream.MMap(MMAP_OPENMODE, Data.GetLength())) {
		if (Result) { * Result += "[error creating " + SourceFilename + ", " + CString::IntToStr(Data.GetLength()) + " bytes]"; }
		return false;
	}

	SourceStream.Write((void *) Data.GetBuffer(), Data.GetLength());
    SourceStream.MMUnmap();
    
    // produce the command line, simple map
       
	CVector<CString> Parameters; 
    Parameters += Extension; 
    Parameters += SourceFilename; 
    Parameters += TargetFilename; 
    Parameters += FileName; 
    Parameters += Data; 
    Parameters += MimeType;
    
	bool bResult = ExecuteAndReadFilter(
		Map(FilterCmdline, Parameters), 
		TargetFilename,
		TargetData,
		Result,
		false);

	CLocalFile::Delete(SourceFilename);

	return bResult;
}

void CSite::RetrieveSite(bool Lazy){
    
    Trace(tagIndex, levInfo, ("CSite :: RetrieveSite()"));
    
    ReadConfig(Lazy);
    LoadSiteIndex();    

    bool bRemoved404 = false;
    m_ProcessedFiles.Set(0);
    m_SiteIndex.m_IndexOptions.m_ModifiedCount.Set(0);
    
    // m_SiteIndex.GetSearcher().MakeAlreadyIndexed(!m_SiteIndex.GetGatherEmail());            
    
    for (int SIndex = 0; SIndex < (int) m_SiteConfigs.GetSize(); SIndex++) {

        if (g_pHandler->GetSignalSigterm())
            break;
        
        m_CurrentConfiguration = & m_SiteConfigs[SIndex];

        if ((
            (m_State == CsLoaded) &&
            (m_CurrentConfiguration->GetCanReindex()) &&
            (!m_ForceNoReindex)
            ) || (!Lazy)) {

            bool bVerbose = !Lazy || m_Verbose;
            
            SetIndexOptions(* m_CurrentConfiguration, m_SiteIndex);
            
            CDate StartDate;
            cout << "[" << StartDate.Map("$hour:$min:$sec $year-$month-$day") << "][building index in " << m_ConfigPath << "]" << endl;
            
            if (bVerbose && !m_SiteIndex.m_IndexOptions.m_GatherEmail) {
                CString Extensions;
                CString::VectorToStr(m_SiteIndex.m_IndexOptions.m_IndexingExts, ',', &Extensions);
                cout << "[parsing extensions: " + Extensions << "]" << endl;
            }

            static const CString UrlList("UrlList");
            static const CString UrlListFile("UrlListFile");
                
            CVector<CString> URLs;            
            CVector<CString> FVector;
    
            // UrlListFile
            CString::StrToVector(m_CurrentConfiguration->GetOption(UrlList),',', &URLs);
            CString::StrToVector(m_CurrentConfiguration->GetOption(UrlListFile),',',&FVector);
            CConfig :: AddFromFilesToVector(UrlListFile, URLs, FVector);        
            
            int CurrentDepth = m_CurrentConfiguration->GetDepth();
    
            if (!bRemoved404 && CurrentDepth && m_Enable404 && (!m_SiteIndex.m_IndexOptions.m_GatherEmail)) {
                bRemoved404 = true;
                m_SiteIndex.Remove404(this, bVerbose);
            }
            
            CVector<CString> TopIndexedVector;
            
            for (int i=0;i<(int) URLs.GetSize();i++) {
                
                m_ProcessedLinks.Set(0);
                
                //if (!Lazy || m_Verbose) cout << "[starting from " << URLs[i] << "]" << endl;
                CString TranslatedUrl;        
                // if that is a unique URL from a list, we can safely index it in parallel, otherwise take
                // care of the real url, the server passed through depends on that
                if (CurrentDepth) { 
                    TranslatedUrl = URLs[i];
                    
                    AddRobotsJob(TranslatedUrl, Lazy);

                    CString OutputString;
                    m_CurrentConfiguration->RedirectUrl(TranslatedUrl, m_EnableExcludeVerbose?(CString*)&OutputString:(CString*)NULL);
                    m_CurrentConfiguration->ReplaceUrl(TranslatedUrl, m_EnableExcludeVerbose?(CString*)&OutputString:(CString*)NULL);
                    if (m_EnableExcludeVerbose && OutputString.GetLength())
                        cout << OutputString;
                    
                    if (! GetFinalURL(TranslatedUrl, TranslatedUrl, !Lazy || m_Verbose)) {
                        continue;
                    }

                    if (!TranslatedUrl.GetLength()) {
                        cout << "\t[warning: " << URLs[i] << " was translated into an empty url and will be ignored!" << endl;
                        cout << "\t          (if this is unexpected, check your configuration sytax and try to run with -exv)]" << endl;
                    }

                    if (TopIndexedVector.FindSortedElt(TranslatedUrl) != -1) {
                        if (TranslatedUrl != URLs[i]) {
                            cout << "[warning: " << URLs[i] << " redirects to " << TranslatedUrl << ", which has already been indexed]" << endl;
                        } else {
                            cout << "[warning: " << TranslatedUrl << " has already been indexed]" << endl;
                        }
                        continue;
                    }

                    TopIndexedVector.AddSortedUnique(TranslatedUrl);        
                } else {
                    TranslatedUrl = URLs[i];
                }
                
                CString SearchTop = TranslatedUrl;
                SearchTop.Trim32();

				Trace(tagIndex, levInfo, ("CSite :: RetrieveSite - searchtop [%s].", SearchTop.GetBuffer()));

                if (SearchTop.GetLength()) {
                    CUrl STUrl(SearchTop);
                    if (STUrl.GetValid()) {                      
                      SearchTop = STUrl.GetHttpServer();
                      if (!IsExcludedDepth(STUrl)&&(!IsExcludedRobots(STUrl, Lazy))) {
				
					Trace(tagIndex, levInfo, ("CSite :: RetrieveSite - searchtop x [%s].", SearchTop.GetBuffer()));
					Trace(tagIndex, levInfo, ("CSite :: RetrieveSite - searchtop y [%s].", STUrl.GetHttpDirectory().GetBuffer()));

                        SearchTop += STUrl.GetHttpDirectory();
                        if (!IsExcluded(TranslatedUrl)) {
                          // _L_DEBUG(7, cout << "CSite::ProcessURL(B-entry) - [" << TranslatedUrl << "][" << SearchTop << "]" << endl);			  
                          CVector<CUrl> RemoteHistory;
                          RemoteHistory.Add(CUrl(TranslatedUrl));    
                          ProcessURL(TranslatedUrl, Lazy, SearchTop, RemoteHistory);
                          if (CurrentDepth)
                            m_IndexPool.PassiveWait();                        
                        } else cerr << "[invalid or excluded url: " << TranslatedUrl << "]" << endl;                      
                      } else cerr << "[excluded by robots directive: " << TranslatedUrl << "]" << endl;
                    } else cerr << "[malformed url: " << TranslatedUrl << "]" << endl;                      
                }
            }

            CDate EndDate;
            cout << "[" << EndDate.Map("$hour:$min:$sec $year-$month-$day") << "][done building index in " << m_ConfigPath << "]" << endl;

            if (!CurrentDepth)
                m_IndexPool.PassiveWait();
            if (Lazy) 
                Write(Lazy, true);
        }
    }
    m_UniqueLinks.RemoveAll();			
    m_ExcludeTable.RemoveAll();    
    m_ReindexCount++;
}

bool CSite::IsIncludedUrlIndex(const CString& Url) const {
    
    int i;
    
    unsigned int Total = 
        m_CurrentConfiguration->GetUrlIndexPaths().GetSize() +
        m_CurrentConfiguration->GetRegexp_UrlIndexPaths().GetSize();
    
    if (! Total)
        return true;
    
    /* Index inclusion paths. */

    for (i=0;i<(int) m_CurrentConfiguration->GetUrlIndexPaths().GetSize();i++) {
        if (Url.StartsWith(m_CurrentConfiguration->GetUrlIndexPaths()[i])) 
            return true;
    }

    for (i=0;i<(int) m_CurrentConfiguration->GetRegexp_UrlIndexPaths().GetSize();i++) {
        if (CRegExp::Match(Url, m_CurrentConfiguration->GetRegexp_UrlIndexPaths()[i]))
            return true;            
    }
    
    return false;  
}

bool CSite::IsExcludedUrlSkip(const CString& Url) const {

    int i;

    /* Index exclusion paths. */
    
    for (i=0;i<(int) m_CurrentConfiguration->GetUrlSkipPaths().GetSize();i++) {
        if (Url.StartsWith(m_CurrentConfiguration->GetUrlSkipPaths()[i])) 
            return true;
    }
    
    for (i=0;i<(int) m_CurrentConfiguration->GetRegexp_UrlSkipPaths().GetSize();i++) {
        if (CRegExp::Match(Url, m_CurrentConfiguration->GetRegexp_UrlSkipPaths()[i]))
            return true;            
    }
    
    return false;  
}

bool CSite::IsMaxRemoteDepthReached(const CVector<CUrl>& RemoteHistory) const {

    if (! RemoteHistory.GetSize())
        return false;
    
    if (m_CurrentConfiguration->GetRemoteDepth() == -1)
        return false;
    
    if ((int) RemoteHistory.GetSize() > m_CurrentConfiguration->GetRemoteDepth())
        return true;
    
    return false;
}
    
bool CSite::IsMaxFilesLimitReached(void) const {
    if ((m_CurrentConfiguration->GetMaxFiles() > 0) && 
        (m_ProcessedFiles.Get() > m_CurrentConfiguration->GetMaxFiles()))
        return true; 
    
    if ((m_CurrentConfiguration->GetMaxLinks() > 0) && 
        (m_ProcessedLinks.Get() > m_CurrentConfiguration->GetMaxLinks()))
        return true; 
    
    return false;
}
