/*
  
  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  _____________________________________________________
  
  written by Daniel Doubrovkine - dblock@vestris.com
  
  Revision history:
  
  27.10.1999: hack for autoresize of quantity of results (SetDim($Quant))
  to avoid huge numbers that lead to an OUTOFMEMORY error
  30.10.1999: support for NoMd5=Y
  
*/
    
#include <alkaline.hpp>
#include "Index.hpp"
#include <Site/Site.hpp>
#include <Encryption/Encryption.hpp>
#include <Internet/EmailAddress.hpp>
#include <Encryption/Md5.hpp>
#include <AlkalineSession/AlkalineSession.hpp>
#include <File/LocalFile.hpp>
#include <Vector/IntVecIter.hpp>
#include <String/GStrings.hpp>
#include <Main/TraceTags.hpp>

CIndex::CIndex(void) {
    m_Searcher.SetParent(this);
    SetExts("html,htm,txt,shtml", true);
}

CIndex::CIndex(const CString& Index) {
    m_Searcher.SetParent(this);
    SetIndex(Index);
    SetExts("html,htm,txt,shtml", true);
}

bool CIndex::RetrieveFile(CSite * Parent, const CString& URL, CString * Contents) const {
    bool Result = false;
    CRemoteFile FileRemote(URL);
    
    FileRemote.SetFollowRedirections(false);
    FileRemote.SetVerbose(false);
    Parent->AppendRequestHeaders(FileRemote);
    FileRemote.SetTimeout(m_IndexOptions.m_Timeout);    
    FileRemote.SetMaxSize(m_IndexOptions.m_Limit);
    
    m_INFManager.StartReading();
    FileRemote.SetAuth(Parent->GetAuth());
    m_INFManager.StopReading();

    ((CIndex&)(* this)).IssueRequest(&FileRemote);

    if (FileRemote.GetRStatusValue() == 200) {
        Result = true;
        if (Contents)
            (* Contents) = FileRemote.GetRData();
    }  
    
    return Result;
}

void CIndex::Remove404(CSite * Parent, bool Verbose) {    
    m_URLManager.Remove404(this, Parent, Verbose);    
}

void CIndex::SetExts(const CString& Exts, bool Overwrite){
    
    if (Overwrite) {
        m_IndexOptions.m_IndexingExts.RemoveAll();
    }
    
    CVector<CString> NewExts;
    CString::StrToVector(Exts, ',', &NewExts);
    
    for (int i=0;i<(int) NewExts.GetSize();i++) {
        NewExts[i].Trim32();
        m_IndexOptions.m_IndexingExts.AddSortedUnique(NewExts[i]);
    }
}

// load indexes separately
bool CIndex::LoadIndexINF(bool bMakeMd5Tree) {
    
    if (g_pHandler->GetSignalSigterm())
        return false;

    bool bResult = true;

    // load .inf file
    if (bResult && CLocalFile::FileExists(m_IndexOptions.m_CurrentFilenameInf)) {
        m_INFManager.StartWriting();            
        m_INFManager.Grow(m_URLManager.GetSize());
        cout << "  [" << m_IndexOptions.m_CurrentFilenameInf << "]"; cout.flush();
        if (! m_INFManager.Load(m_IndexOptions.m_CurrentFilenameInf, true, bMakeMd5Tree)) {
            cout << endl;    
            bResult = false;
        }
        m_INFManager.StopWriting();    
    }

    return bResult;
}

bool CIndex::LoadIndexLNX(void) {
    
    if (g_pHandler->GetSignalSigterm())
        return false;

    bool bResult = true;

    // load .lnx file    
    if (bResult && CLocalFile::FileExists(m_IndexOptions.m_CurrentFilenameLnx)) {
        m_LNXManager.StartWriting();  
        m_LNXManager.Grow(m_URLManager.GetSize());            
        cout << "  [" << m_IndexOptions.m_CurrentFilenameLnx << "]"; cout.flush();
        if (! m_LNXManager.Load(m_IndexOptions.m_CurrentFilenameLnx, true)) {
            cout << endl;            
            bResult = false;
        }
        m_LNXManager.StopWriting();                     
    }
    
    return bResult;
}

bool CIndex::LoadIndexURT(void) {

    if (g_pHandler->GetSignalSigterm())
        return false;

    bool bResult = true;

    CString UrlListFilename_1_2 = m_IndexOptions.m_IndexFile + "1.url";

    // load .url / .urt file
    if (bResult && CLocalFile::FileExists(UrlListFilename_1_2)) {
        cout << "  [" << UrlListFilename_1_2 << "]"; cout.flush();
        cout << "[converting to url tree]"; cout.flush();
        m_URLManager.StartWriting();
        m_URLManager.Load(UrlListFilename_1_2, true);
        m_URLManager.SetFilename(m_IndexOptions.m_CurrentFilenameUrt);
        m_URLManager.Write(true);        
        m_URLManager.StopWriting();            
        base_unlink((const char *) UrlListFilename_1_2.GetBuffer());
    } else {        
        if (bResult && CLocalFile::FileExists(m_IndexOptions.m_CurrentFilenameUrt)) {
            cout << "  [" << m_IndexOptions.m_CurrentFilenameUrt << "]"; cout.flush();
            m_URLManager.StartWriting();  
            if (! m_URLManager.FastLoad(m_IndexOptions.m_CurrentFilenameUrt, true)) {
                cout << endl;
                bResult = false;
            }
            m_URLManager.StopWriting();        
        }        
    }

    return bResult;
}

bool CIndex::LoadIndexNDX(bool bMakeAlreadyIndexed) {

    if (g_pHandler->GetSignalSigterm())
        return false;

    bool bResult = true;

    // grow the already indexed pages vector, add some space for future growth
    m_Searcher.GetAlreadyIndexedPages().Grow(m_URLManager.GetSize());

    CString NdxListFilename_1_31 = m_IndexOptions.m_IndexFile + "1.ndx";
    CString NdxListFilename_1_4 = m_IndexOptions.m_IndexFile + "2.ndx";

    // load .ndx file
    if (bResult && CLocalFile :: FileExists(m_IndexOptions.m_CurrentFilenameNdx)) {
        cout << "  [" << m_IndexOptions.m_CurrentFilenameNdx << "]"; cout.flush();  
        if (! m_Searcher.LoadIndex(m_IndexOptions.m_CurrentFilenameNdx, true, bMakeAlreadyIndexed))
            bResult = false; 
    } else if (bResult && CLocalFile :: FileExists(NdxListFilename_1_4)) {
        cout << "  [" << NdxListFilename_1_4 << "]"; cout.flush();  
        cout << "[loading 1.4 ndx]";
        if (! m_Searcher.LoadIndex_1_4(NdxListFilename_1_4, true, bMakeAlreadyIndexed))
            bResult = false; 
    } else if (bResult && CLocalFile :: FileExists(NdxListFilename_1_31)) {
        cout << "  [" << NdxListFilename_1_31 << "]"; cout.flush();  
        cout << "[loading 1.31 ndx]";
        if (! m_Searcher.LoadIndex_1_31(NdxListFilename_1_31, true, bMakeAlreadyIndexed))
            bResult = false;
    }             

    return bResult;
}

void CIndex::SetIndex(const CString& Index, bool bLoad, bool bMakeAlreadyIndexed) {
    
    bool bResult = true;
    
    m_IndexOptions.m_Modified = false;
    m_IndexOptions.m_IndexFile = Index;

    /* lnx file names */
    m_IndexOptions.m_CurrentFilenameLnx = m_IndexOptions.m_IndexFile + "1.lnx";
    m_LNXManager.SetFilename(m_IndexOptions.m_CurrentFilenameLnx);

    /* urt/url file names */    
    m_IndexOptions.m_CurrentFilenameUrt = m_IndexOptions.m_IndexFile + "1.urt";
    m_URLManager.SetFilename(m_IndexOptions.m_CurrentFilenameUrt);

    /* inf file names */
    m_IndexOptions.m_CurrentFilenameInf = m_IndexOptions.m_IndexFile + "1.inf";
    m_INFManager.SetFilename(m_IndexOptions.m_CurrentFilenameInf);

    /* ndx file names */    
    m_IndexOptions.m_CurrentFilenameNdx = m_IndexOptions.m_IndexFile + "3.ndx";


    if (m_IndexOptions.m_GatherEmail)
        return;

    if (! bLoad)
        return;

    cout << "[loading indexes] {" << endl;

    if (bResult) bResult = LoadIndexURT();

    int URLManagerSize = m_URLManager.GetSize();

    if (bResult) bResult = LoadIndexINF(false);

    int INFManagerSize = m_INFManager.GetSize();
    
    if (bResult) bResult = LoadIndexLNX();

    int LNXManagerSize = m_LNXManager.GetSize();

    if ((URLManagerSize == 0) &&
        (LNXManagerSize == 0) &&
        (INFManagerSize == 0))
    {
        cout << "  [no index files found (clean index)]" << endl;
        cout << "} done." << endl;
        return;
    }
    
    if ((INFManagerSize != LNXManagerSize) || (LNXManagerSize != URLManagerSize)) {
        bResult = false;
    }

    if (bResult) bResult = LoadIndexNDX(bMakeAlreadyIndexed);

	if (g_pHandler->GetSignalSigterm()) {		
		return;
	}

    cout << "  [checking cross-references, "; cout.flush(); 

    if (! bResult) {
        
        cout << "corrupt, indexes not loaded]\n  [you should delete all siteidx*.* files and make clean indexes]" << endl;
        
        m_LNXManager.StartWriting();  
        m_INFManager.StartWriting();
        m_URLManager.StartWriting();
        m_Searcher.StartWriting();
        
        m_LNXManager.RemoveAll();
        m_INFManager.RemoveAll();
        m_URLManager.RemoveAll();
        m_Searcher.RemoveAll(false);
        
        m_Searcher.StopWriting();
        m_LNXManager.StopWriting();  
        m_INFManager.StopWriting();
        m_URLManager.StopWriting();            
    } else {
        cout << "- ok.]" << endl;
    }
    
    cout << "} done." << endl;
}

CIndex::~CIndex(void) {
    Write(m_IndexOptions.m_Verbose);
}

void CIndex::Write(bool ShowWrite) {    
    if (!m_IndexOptions.m_Modified || m_IndexOptions.m_GatherEmail)
        return;
    
    if (g_pHandler->GetSignalSigterm())
        return;
    
    m_URLManager.StartReading();
    m_INFManager.StartReading();
    m_LNXManager.StartReading();            
    m_Searcher.StartReading();
  
    if (m_IndexOptions.m_Modified) {

        m_IndexOptions.m_Modified = false;
    
        // figure out the minimal data rows to write
        int DataRows = (int) m_INFManager.GetSize();
        if ((int) m_LNXManager.GetSize() < DataRows)
            DataRows = m_LNXManager.GetSize();
        if ((int) m_URLManager.GetSize() < DataRows)
            DataRows = m_URLManager.GetSize();
    
        void * PTerminate = (void *) signal(SIGTERM, SIG_IGN);
        void * PInterupt = (void *) signal(SIGINT, SIG_IGN);
#ifdef _UNIX
        void * PAlarm = (void *) signal(SIGALRM, SIG_IGN);
#endif
    
        if (ShowWrite) cout << "[writing databases] {" << endl;
    
        bool bError = false;
    
        if (! bError) {
            if (ShowWrite) cout << "  [inf]"; 
            bError = ! m_INFManager.Write(ShowWrite, DataRows); 
            if (bError) {
                cerr << "[system error writing .inf file]" << endl;                
                CHandler::ShowLastError();
            }
            if (ShowWrite) cout << endl;
        }
        
        if (! bError) {
            if (ShowWrite) cout << "  [lnx]"; 
            bError = ! m_LNXManager.Write(ShowWrite, DataRows); 
            if (bError) {
                cerr << "[system error writing .lnx file]" << endl;
                CHandler::ShowLastError();
            }
            if (ShowWrite) cout << endl;
        }
        
        if (! bError) {
            if (ShowWrite) cout << "  [url]"; 
            bError = ! m_URLManager.Write(ShowWrite, DataRows); 
            if (bError) {
                cerr << "[system error writing .urt file]" << endl;
                CHandler::ShowLastError();
            }
            if (ShowWrite) cout << endl;
        }
        
        if (! bError) {
            if (ShowWrite) cout << "  [ndx]"; 
            bError = ! m_Searcher.WriteIndex(m_IndexOptions.m_CurrentFilenameNdx, ShowWrite);     
            if (bError) {
                cerr << "[system error writing .ndx file]" << endl;
                CHandler::ShowLastError();
            }
            if (ShowWrite) cout << endl << "} done." << endl;
        }
        
        if (bError) {
            m_IndexOptions.m_Modified = true;
        }
        
        signal(SIGTERM, (void (*)(int)) PTerminate);
        signal(SIGINT, (void (*)(int)) PInterupt);
#ifdef _UNIX
        signal(SIGALRM, (void (*)(int)) PAlarm);
#endif            
    }
  
    m_Searcher.StopReading();
    m_INFManager.StopReading();            
    m_LNXManager.StopReading();
    m_URLManager.StopReading();                   
}

bool CIndex::CheckIndexHTML(const CUrl& FileURL){
    if (!FileURL.GetHttpFilename().GetLength() && m_IndexOptions.m_IndexHTML.GetLength()) {
        ((CUrl&) FileURL).SetHttpFilename(m_IndexOptions.m_IndexHTML);
        return true;
    } else return false;
}

bool CIndex::SetIndexHTML(const CString& IndexHTML){
    if (m_IndexOptions.m_IndexHTML != IndexHTML) {
        if (IndexHTML.Same("(none)"))
            m_IndexOptions.m_IndexHTML.Empty();
        else m_IndexOptions.m_IndexHTML = IndexHTML;
        return true;
    } else return false;
}

bool CIndex::ProcessLocalRequest(const CString& LocalName, CRemoteFile * RemoteFile, CString * pResult) {

    Trace(tagIndex, levInfo, ("CIndex::Index - ProcessLocalRequest [%s]", LocalName.GetBuffer()));    

    // support If-Modified-Since
    struct_stat LocalFileStat;
    if (!CLocalFile::GetFileStat(LocalName, LocalFileStat)) {
        RemoteFile->GetHttpRequest().SetRStatusValue(400);
        * pResult += "[stat-error]";
        return false;
    }

	// zero or -1 MaxSize is ignored
    if (RemoteFile->GetHttpRequest().GetRequestSizeLimit() >= 0 && 		
        LocalFileStat.st_size > RemoteFile->GetHttpRequest().GetRequestSizeLimit()) {
         RemoteFile->GetHttpRequest().SetRStatusValue(206);
         CStringTable Table;
         Table.Set("Content-Length", CString::IntToStr(LocalFileStat.st_size));
         RemoteFile->GetHttpRequest().GetRFields().Set("Content-Length", Table);
         return false;
    }

    struct tm LastModifiedStruct;
    // get the current file modification time (GMT)
    base_gmtime(LocalFileStat.st_mtime, LastModifiedStruct);    
    CDate FileModificationDate(&LastModifiedStruct);
    CString LastModifiedString = FileModificationDate.Map("$dayeng, $day $montheng $year $hour:$min:$sec GMT");
    // get the current if-Modified-Since set as a request field
    CString IfModifiedSinceHeader = RemoteFile->GetParameter(g_strHttpIfModifiedSince);
  
    if (IfModifiedSinceHeader.GetLength()) {
        if (CDate::CompareDates(IfModifiedSinceHeader, LastModifiedString) >= 0) {      
            RemoteFile->GetHttpRequest().SetRStatusValue(304);
            RemoteFile->GetHttpRequest().SetRStatusString("Not Modified");
            return false;
        }
    }
  
    CLocalFile FastFile(LocalName);
    if (!FastFile.OpenReadBinary()) {
        * pResult += "[open-error]";
        RemoteFile->GetHttpRequest().SetRStatusValue(400);        
        return false;
    }
  
    if (!FastFile.Read((CString *) & RemoteFile->GetRData())) {
        * pResult += "[read-error]";
        RemoteFile->GetHttpRequest().SetRStatusValue(400);        
        return false;
    }
  
    // set the last-modified entry in the remote file
    RemoteFile->SetValue(g_strHttpLastModified, LastModifiedString);  
    RemoteFile->SetValue(g_strHttpDate, LastModifiedString);  
    RemoteFile->GetHttpRequest().SetRStatusValue(200);
    RemoteFile->GetHttpRequest().SetRStatusString("OK");

    return true;
}

bool CIndex::ProcessLocalDirectory(const CString& Directory, CRemoteFile * RemoteFile, CString * pResult) {

  CString TerminatedDirectory(Directory);
  CLocalPath::Terminate(TerminatedDirectory);
      
  if (! CLocalPath::DirectoryExists(TerminatedDirectory)) {

    // check whether this is a relative path
    
    CString PartialDirectory(TerminatedDirectory);
    PartialDirectory.TrimLeft(PATH_SEPARATOR, PATH_SEPARATOR);
    PartialDirectory.Insert(0, CLocalPath::GetCurrentDirectory());		

    if (CLocalPath::DirectoryExists(PartialDirectory)) {
      CLocalPath::Terminate(PartialDirectory);
      RemoteFile->GetHttpRequest().SetRStatusValue(301);
      RemoteFile->GetHttpRequest().GetRRedirections().Add(Directory);
      RemoteFile->GetHttpRequest().GetRRedirections().Add("file:///" + PartialDirectory);
      RemoteFile->GetHttpRequest().SetRField("Location", "file:///" + PartialDirectory);
      return true;
    }
    
    return false;
  }
  
  // check whether index.html is defined and return the document if present
  if (m_IndexOptions.m_IndexHTML.GetLength()) {
    CString IndexHtmlDocument(Directory);
    CLocalPath::Terminate(IndexHtmlDocument);
    IndexHtmlDocument += m_IndexOptions.m_IndexHTML;
    
    if (CLocalFile::FileExists(IndexHtmlDocument)) {
      if (ProcessLocalRequest(IndexHtmlDocument, RemoteFile, pResult))
	return true;
    }
  }
	
  // do a directory listing otherwise
  
  CVector<CString> PathContents;
  
  CLocalPath::PopulatePathContents(false, true, true, PathContents, TerminatedDirectory);
  
  ((CString &) RemoteFile->GetRData()) = ("<title>" + Directory + "</title>");
  
  for (register int i = 0; i < (int) PathContents.GetSize(); i++) {
    // collected links are relative to Directory, but include it
    // so base href is one level up for relative links
    CString File(PathContents[i]);
    if (File.StartsWith(TerminatedDirectory)) {
      File.Delete(0, TerminatedDirectory.GetLength());
    }
    Trace(tagIndex, levInfo, ("CIndex::Index - Adding (file) %s", File.GetBuffer()));          
    ((CString &) RemoteFile->GetRData()) += ("<a href=\"" + File + "\">" + File + "</a>\n");
  }
  
  RemoteFile->GetHttpRequest().SetRStatusValue(200);
  return true;
}

bool CIndex::IssueLocalRequest(CRemoteFile * RemoteFile, CSite * Site, CString * pResult) {
    
    if (!Site)
        return false;  

    CString LocalName;

    // file://url
    if (RemoteFile->GetUrl().GetScheme().Same("file")) {        

        LocalName = RemoteFile->GetUrl().GetHttpPath();

	Trace(tagIndex, levInfo, ("CIndex::Index - IssueLocalRequest (file) %s", LocalName.GetBuffer()));    


       #ifdef _WIN32
        LocalName.Replace('/', '\\');
       #endif
       #ifdef _UNIX
        LocalName.Replace('\\', '/');
       #endif

        LocalName.TrimRight(PATH_SEPARATOR, PATH_SEPARATOR);

        if (ProcessLocalDirectory(LocalName, RemoteFile, pResult))
			return true;
        
        ProcessLocalRequest(LocalName, RemoteFile, pResult);
        return true;

    } else {

        if (!Site->GetReplaceLocalSize())
            return false;    
  
        CString RequestUrl = RemoteFile->GetUrl().GetHttpAll();
    
        if (!Site->ReplaceLocalUrl(RequestUrl, & LocalName, Site->GetEnableExcludeVerbose()?pResult:(CString *)NULL))
            return false;
  
        if (pResult)
            * pResult += "\n  [" + LocalName + "]";

        return ProcessLocalRequest(LocalName, RemoteFile, pResult);
    }
    
}

void CIndex::IssueRequest(CRemoteFile * RemoteFile, CSite * Site, CString * pResult) {

    if (IssueLocalRequest(RemoteFile, Site, pResult))
        return;
    
    if (Site->GetCanCookies()) {
        CString CookieString;
        CookieString = m_CookieStorage.GetCookieString(RemoteFile->GetUrl());
        // _L_DEBUG(3, cout << "CIndex::CookieString: [" << CookieString << "]" << endl);
        if (CookieString.GetLength()) 
            RemoteFile->SetParameter("Cookie", CookieString);
    }
    
    if (m_IndexOptions.m_Proxy.GetLength()) 
        RemoteFile->SetProxy(m_IndexOptions.m_Proxy);

    RemoteFile->Get();

    if (Site->GetCanCookies()) {
        m_CookieStorage.ProcessHttpRequest(RemoteFile->GetHttpRequest());
    }    
}

void CIndex::Index(
    CRemoteFile& RemoteFile, 
    const CUrl& FileURL, 
    CString& Server, 
    CVector<CString>& Links,     
    CSite * Parent, 
    CVector<CString>& Redirections, 
    const CIndexObject& IndexObject,
    CString * Result) {
    
    CString FileName;  
    int StillRetrieve;
    int r;
    
    Trace(tagIndex, levInfo, ("CIndex::Index - entering for %s (%d)", FileURL.GetHttpFilename().GetBuffer(), Server.GetBuffer()));    
    
    if (CheckIndexHTML(FileURL)) {
        (* Result)+=("[+" + FileURL.GetHttpFilename() + "]");    
    }
    
    FileName = FileURL.GetHttpAll();
    // lookup for the file, in case it cas already been indexed
    
    int UrlPos = FindURLIndex(FileName);
    CString FIfModifiedSince;
    
    StillRetrieve = m_IndexOptions.m_RetryCount+1;
    if (StillRetrieve <= 0)
        StillRetrieve = 1;
    
    // check if the file should be expired (forced)
    if (!Parent->GetForceExpire()) {
        m_INFManager.StartReading();
        FIfModifiedSince = m_INFManager.GetModifiedSince(UrlPos);
        m_INFManager.StopReading();
    }
    
    RemoteFile.SetUrl(FileURL);

    Server = RemoteFile.GetUrl().GetHttpServer();

    RemoteFile.SetFollowRedirections(false);
    RemoteFile.SetMaxSize(m_IndexOptions.m_Limit);
    RemoteFile.SetTimeout(m_IndexOptions.m_Timeout);
    Parent->AppendRequestHeaders(RemoteFile);
    RemoteFile.SetVerbose(false);    

    m_INFManager.StartReading();
    RemoteFile.SetAuth(Parent->GetAuth());
    m_INFManager.StopReading();

    while (StillRetrieve && !g_pHandler->GetSignalSigterm()) {

        Trace(tagIndex, levInfo, ("CIndex::Index - retrieving %s.", RemoteFile.GetUrl().GetHttpAll().GetBuffer()));            
        
        if (FIfModifiedSince.GetLength()) {
            // check if we are inexing new documents only
            if (Parent->GetNewOnly()) {

	        Trace(tagIndex, levInfo, ("CIndex::Index - %s not modified.", RemoteFile.GetUrl().GetHttpAll().GetBuffer()));            

                (* Result) += ("[--NewOnly/Not Modified]\n");
                
                GetURLVectorStrings(UrlPos, &Links);
                
                m_INFManager.StartReading();
                if (m_INFManager.GetSize(UrlPos) > 5) 
                    Server = m_INFManager[UrlPos][5];        
                m_INFManager.StopReading();
                
                return;
            }
	    
	    Trace(tagIndex, levInfo, ("CIndex::Index - %s, setting %s.", RemoteFile.GetUrl().GetHttpAll().GetBuffer(), FIfModifiedSince.GetBuffer()));
            RemoteFile.SetParameter(g_strHttpIfModifiedSince, FIfModifiedSince);
        }
        
        IssueRequest(&RemoteFile, Parent, Result);
        
        // HACKHACK: Win32 quering localhost might have a select/recv inconsistency
        if ((RemoteFile.GetRStatusValue() == -1) && FIfModifiedSince.GetLength())  {
            RemoteFile.RemoveParameter(g_strHttpIfModifiedSince);
            IssueRequest(&RemoteFile, Parent, Result);
        }
        
        bool bCircularRedirection = false;
        
	Trace(tagIndex, levInfo, ("CIndex::Index - %s has %d redirection(s)", FileURL.GetHttpFilename().GetBuffer(), Redirections.GetSize()));            

        if (RemoteFile.GetRRedirections().GetSize() > 0) {
            
            for (r = 0; r < (int) RemoteFile.GetRRedirections().GetSize(); r++) {
                if ((RemoteFile.GetRRedirections()[r] == FileName) ||
                    (Redirections.Contains(RemoteFile.GetRRedirections()[r]))) {
                    bCircularRedirection = true;
                    break;
                }
            }
    
            if (bCircularRedirection) {    
                
                * Result += "[circ]";
                
            } else {
                
                Redirections += RemoteFile.GetRRedirections();
    
                FileName = Redirections[Redirections.GetSize() - 1];                

                RemoteFile.SetUrl(FileName);

                Server = RemoteFile.GetUrl().GetHttpServer();
                
                // check for other properties of this url
                if (Parent->OnNewUrl(
                    FileName, 
                    Server, 
                    IndexObject,
                    * Result)) {
                    
                    Parent->AppendRequestHeaders(RemoteFile);                    
                    
                    if (CheckIndexHTML(RemoteFile.GetUrl())) {
                        (* Result)+=("[+" + RemoteFile.GetUrl().GetHttpFilename() + "]");    
                    }
                
                    UrlPos = FindURLIndex(FileName);
                    
                    if (!Parent->GetForceExpire()) {
                        m_INFManager.StartReading();
                        FIfModifiedSince = m_INFManager.GetModifiedSince(UrlPos);
                        m_INFManager.StopReading();
                    }
                    
                    (* Result) += ("[" + CHttpIo::GetRStatusString(RemoteFile.GetRStatusValue()) + "]\n  [" + FileName + "]");
                    
                    continue;
                }

                (* Result) += ("[" + CHttpIo::GetRStatusString(RemoteFile.GetRStatusValue()) + "]\n  [" + FileName + "]");
                (* Result) += ("[nfaq]");
                
            }
            
        }
        
        // _L_DEBUG(4, cout << "CIndex:: reindex returned:[" << RemoteFile.GetRStatusValue() << "]" << endl);
        
        // index words and file reference

        if (RemoteFile.GetRStatusValue() == 200) {

            AppendSucceededAuthState(Parent, RemoteFile, Result);

            (* Result)+=("[" + CString::IntToStr(RemoteFile.GetRData().GetLength()) + " bytes]");
            IndexFileAll(RemoteFile, FileName, Server, Links, IndexObject.GetLazy(), Parent, Result);
            
            m_IndexOptions.m_ModifiedCount++;
            
            break;
            
        } else {
            
            IndexProcessErrors(RemoteFile, Server, Links, UrlPos, Result);
            
            if (!StillRetrieve) 
                break;
            
            if (! RemoteFile.GetUnrecoverableError() &&
                ((RemoteFile.GetRStatusValue() == 204) || // partial content
                 (RemoteFile.GetRStatusValue() == 207) || // partial content-length
                 (RemoteFile.GetRStatusValue() == 922)    // timed out
                    )) {
                
                StillRetrieve--;
                
                if (StillRetrieve > 1)
                    (* Result) += ("[retrying (" + CString::IntToStr(StillRetrieve) + " attempt" + CString(((StillRetrieve>1)?"s ":" ")) + "left)]");
                else if (StillRetrieve) 
                    (* Result) += "[retrying (last attempt)]";
                else (* Result) += "[quit]";
                
            } else break;
        }
    }
  
    Trace(tagIndex, levInfo, ("CIndex::Index - done %s (%d redirections)", FileURL.GetHttpFilename().GetBuffer(), Redirections.GetSize()));            
}

void CIndex::IndexProcessErrors(
    CRemoteFile& RemoteFile,
    CString& Server, 
    CVector<CString>& Links, 
    int UrlPos,
    CString * Result,
    bool bAppendStatus) {
    
    if (bAppendStatus) {        
        (*Result)+=("[" + CHttpIo::GetRStatusString(RemoteFile.GetRStatusValue()) + "]\n");
    }
    
    m_INFManager.StartReading();
    int INFManagerSize = (int) m_INFManager.GetSize();
    m_INFManager.StopReading();            
  
    if ((UrlPos != -1)&&(INFManagerSize > UrlPos)) {
        switch(RemoteFile.GetRStatusValue()) {
            int t;
            bool bAlreadyIndexed;
        case 404:
            // not found
       
    
            bAlreadyIndexed = m_Searcher.AlreadyIndexed(UrlPos);
                     
            if (bAlreadyIndexed) {
       
                m_Searcher.RemovePermanently(UrlPos);      
        
                m_LNXManager.StartWriting();
                m_LNXManager.GetLNXVector()[UrlPos].RemoveAll();
                m_LNXManager.StopWriting();
        
                m_INFManager.StartWriting();
                for (t=0;t<(int) m_INFManager[UrlPos].GetSize();t++) {
                    m_INFManager.PutAt(CString::EmptyCString, UrlPos, t);
                }
                m_INFManager.StopWriting();
            }
      
            Links.RemoveAll();
            break;
        case 206:
            if (RemoteFile.GetRStatusValue() == 206)
                (* Result) += 
                    "  [MaxSize (" + CString::IntToStr(RemoteFile.GetHttpRequest().GetRequestSizeLimit()) + " bytes) is smaller than the content length (" +
                    RemoteFile.GetHttpRequest().GetRFields().FindElement("Content-Length").GetValue("Content-Length") + " bytes).]\n";
            // process default with 206
        default:
            // _L_DEBUG(4, cout << "Retrieving links from LNXVector at " << UrlPos << endl);
            // _L_DEBUG(4, 
            //         m_LNXManager.StartReading(); 
            //         for(int n=0;n<(int) m_LNXManager.GetLNXVector()[UrlPos].GetSize();n++) 
            //         cout << "  " << m_LNXManager.GetLNXVector()[UrlPos][n]; 
            //         cout << endl; 
            //         m_LNXManager.StopReading();
            //    );
      
            GetURLVectorStrings(UrlPos, &Links);
            // _L_DEBUG(4, for(int n=0;n<(int)Links.GetSize();n++) cout << "  " << Links[n] << endl;);
      
            m_INFManager.StartReading();
            if (m_INFManager.GetSize(UrlPos) > 5) 
                Server = m_INFManager[UrlPos][5];
            m_INFManager.StopReading();
      
            break;
        }
    }
}

CString CIndex::GetFileExtension(const CString& FName) const {
    // _L_DEBUG(4, cout << "CIndex::GetFileExtension() - " << FName << endl);
    if ((FName.GetLength())&&(FName[FName.GetLength()-1]!='/')) {
        int pPos = FName.InvPos('.');
        if (pPos >= 0) {
            CString pExt;
            FName.Mid(pPos+1, FName.GetLength(), &pExt);
            for (int j=0;j<(int) pExt.GetLength();j++)
                if (!isalnum(pExt[j])) {
                    pExt.Delete(j, pExt.GetLength());
                    break;
                }
            // _L_DEBUG(4, cout << "CIndex::GetFileExtension() - return " << pExt << endl);
            return pExt;
        }
    }
    // _L_DEBUG(4, cout << "CIndex::GetFileExtension() - return none." << endl);
    return CString::EmptyCString;
}

void CIndex::IndexAddWords(const CString& Extension,
                           const CString& MimeType,
                           CSite * Parent,
                           CVector<CStringA>& AVector,
                           int UrlPos, CString * Result) {
    
    // ExcludeWords directive, exclude several words from indexing
    Parent->ExecutePluginWords(Extension, MimeType, AVector);
    // IncludePages directive, index pages containing IncludePages only
    if (!(Parent->IncludePluginPages(Extension, MimeType, AVector, Result))) {
        * Result += "[ipexcl]";
        return;
    }
    // ExcludePages directive, do not index pages containing ExcludePages words
    if (!(Parent->ExcludePluginPages(Extension, MimeType, AVector, Result))) {
        * Result += "[epexcl]";
        return;
    }
    // IndexWords directive, include only words to the index
    Parent->ExecuteIndexWords(Extension, MimeType, AVector);
        
    m_Searcher.AddPage(AVector, UrlPos);

    // _L_DEBUG(2, cout << "CIndex::IndexAddWords() - (completed)" << endl);    
}

void CIndex::IndexGatherEmail(const CString& Filename, const CVector<CString>& EmailVector) {
    unsigned int OldSize;
    for (int i=0;i<(int) EmailVector.GetSize();i++) {
        CEmailAddress EmailVerify(EmailVector[i]);
        if (EmailVerify.GetValid()) {
            OldSize = m_AlreadyEmailVector.GetSize();
            m_AlreadyEmailVector.AddSortedUnique(EmailVector[i]);
            if (m_IndexOptions.m_GatherEmailAll || (OldSize != m_AlreadyEmailVector.GetSize())) {
                cout << EmailVector[i] << "  " << Filename << endl;
            }
        }
    }
}

void CIndex::IndexAddMetaWords(const CString& Extension,
                               const CString& MimeType,
                               const CAlkalineParser& HtmlParser,
                               CSite * Parent,
                               int UrlPos,
                               CString * Result) {

    if (HtmlParser.GetMetaRobotsNoindex() && Parent->GetCanRobots()) {
        (* Result)+="[mta rskip][ndx rskip]";
        return;
    }

    int i;

    static const CString MetaAlkalineSkipMeta("ALKALINE:SKIPMETA");
    static const CString MetaAlkalineSkipText("ALKALINE:SKIPTEXT");
  
    CVector<CStringA> AVector;    
    CStringA CurrentWord;
  
    if (MetaAlkalineSkipText.InVector(HtmlParser.GetMetaData(), false) || Parent->GetSkipText()) {
        (* Result)+="[ndx skip]";
    } else {
        CVector<CString> Words; 
        HtmlParser.GetWords(Words);

        // this is the most likely dimension
        AVector.SetDim(Words.GetSize() + HtmlParser.GetMetaData().GetSize());
    
        for (i=((int)Words.GetSize()) - 1;i>=0;i--) {          
            CurrentWord = Words[i];
            if (!CurrentWord.GetLength())
                continue;        
            // convert to upper/lowercase on demand
            if (Parent->GetLowerCase())
                CurrentWord.LowerCase();
            else if (Parent->GetUpperCase())
                CurrentWord.UpperCase();
            AVector.Add(CurrentWord);
        }      
        (* Result)+="[ndx]";
        // IndexAddWords(Extension, MimeType, Parent, Words, UrlPos, Verbose);
    }
  
    if (MetaAlkalineSkipMeta.InVector(HtmlParser.GetMetaData(), false) || Parent->GetSkipMeta()) {
        (* Result)+="[mta skip]";
    } else {
        (* Result)+="[mta]";
        for (i=((int)HtmlParser.GetMetaData().GetSize()) - 1;i>=0;i--) {
            CurrentWord = HtmlParser.GetMetaData()[i];
            if (!CurrentWord.GetLength())
                continue;
            // convert to upper/lowercase on demand
            if (Parent->GetLowerCase())
                CurrentWord.LowerCase();          
            else if (Parent->GetUpperCase())
                CurrentWord.UpperCase();
            AVector.Add(CurrentWord);
        }
        // IndexAddWords(Extension, MimeType, Parent, HtmlParser.GetMetaData(), UrlPos, Verbose);
    }
    
    // we might have introduced duplicates when merging meta data and words
    // and/or converting to upper/lower case
    AVector.QuickSort();
    for (i=(int) AVector.GetSize()-1;i>0;i--) {
        if (AVector[i-1] == AVector[i])
            AVector.RemoveAt(i);        
    }
  

    IndexAddWords(Extension, MimeType, Parent, AVector, UrlPos, Result);


}

void CIndex::IndexFileAll(CRemoteFile& RemoteFile, 
                          const CString& FileName, 
                          CString& Server, 
                          CVector<CString>& Links, 
                          bool /* Lazy */, 
                          CSite * Parent, 
                          CString * Result) {
    
    // _L_DEBUG(4, cout << "IndexFileAll(" << FileName << ")" << endl);

    Result->SetSize(256);

    (*Result)+=("[" + CString::IntToStr(m_Searcher.GetWordCount()) + "]\n  ");
 

    // parse through plugins for data retrieval
    CString Extension = GetFileExtension(FileName);
    // new server base
    CString EmptyText;
    
    CString NewBase(Server);

    CString Doc(RemoteFile.GetUrl().GetHttpDirectory());
    
    NewBase+=Doc;
    CString DocumentDirectory(NewBase);

    CString DataText;
    // _L_DEBUG(4, cout << "CIndex::IndexFileAll() - executing Plugin" << endl);
    
//     cout << "Data length: " << RemoteFile.GetRData().GetLength() << endl;
//     cout << "Extension: " << Extension << endl;
//     cout << "Content-type: " << RemoteFile.GetValue("Content-Type") << endl;
//     cout << "Filename: " << FileName << endl;
//     cout << "Parent: " << (long) Parent << endl;
    
    if (!Parent->ExecutePlugin(RemoteFile.GetRData(), Extension, RemoteFile.GetValue("Content-Type"), FileName, DataText, Result)) {
        // _L_DEBUG(4, cout << "CIndex::IndexFileAll() - no plugin available, executing Get()" << endl);
        DataText = RemoteFile.GetRData();
    }


    // _L_DEBUG(4, cout << "CIndex::IndexFileAll() - initializing HTML consumer" << endl);

    CAlkalineParser HtmlParser;
    HtmlParser.SetEnableObjects(Parent->GetEnableObjects());
    HtmlParser.SetVerbose(false);
    HtmlParser.SetVerboseParser(Parent->GetEnableVerboseParser());
    HtmlParser.SetFreeAlpha(m_IndexOptions.m_FreeAlpha);
    HtmlParser.SetCGI(m_IndexOptions.m_CGI);
    HtmlParser.SetNSF(m_IndexOptions.m_NSF);
    HtmlParser.SetSkipParseLinks(m_IndexOptions.m_SkipParseLinks);
    HtmlParser.SetEmptyLinks(m_IndexOptions.m_EmptyLinks);
    HtmlParser.SetHeaderLength(Parent->GetHeaderLength());  

    // _L_DEBUG(4, cout << "CIndex::IndexFileAll() - CHtmlParser::Parse" << endl);
    
    // parseable-content
    bool bParseContent = Parent->GetParseContent(RemoteFile.GetValue("Content-Type"));
    if (!bParseContent)
        (*Result)+=("[np]");    
    HtmlParser.SetParseHtmlContent(bParseContent);
    
    // parse-metas (meta tags to parse)
    CVector<CString> ParseMetas;
    if (Parent->GetParseMetas(& ParseMetas)) {
        HtmlParser.SetParseMetas(ParseMetas);
    }
    
    // parse the document
    HtmlParser.Parse(DataText);

    // add more meta data
    HtmlParser.AddMetaData("FILENAME", RemoteFile.GetUrl().GetHttpFilename());

    IndexActiveXObjects(HtmlParser, FileName, NewBase, DataText, Parent, Result);
    
    // gather email
    // _L_DEBUG(4, cout << "CIndex::IndexFileAll() - consumer finished" << endl);
    if (m_IndexOptions.m_GatherEmail) {
        HtmlParser.GetLinks(m_IndexOptions.m_IndexingExts, Links, NewBase, Parent->GetEnableExcludeVerbose());
        IndexGatherEmail(FileName, HtmlParser.GetEmail());
        return;
    }

    if (HtmlParser.GetMetaData().Contains("ALKALINE:SKIP")) {
        Links.RemoveAll();
        // remove the url in case the mskip was added recently
        int UrlPosMskip = FindURLIndex(FileName);
        if (UrlPosMskip >= 0) {       
            bool bAlreadyIndexed = m_Searcher.AlreadyIndexed(UrlPosMskip);       
            if (bAlreadyIndexed) {          
                m_Searcher.RemovePermanently(UrlPosMskip);                   
                (*Result)+="[remove]";      
            }
        }
        (*Result)+="[mskip]\n";    
        return;
    }
    
    // file reference
    (*Result)+="[inf]";  

    CVector<CString> FileIndexVector;
    
    IndexFileInfVector(FileIndexVector, HtmlParser, RemoteFile, Server, DataText, Parent, Result);    
    
    // remote links
    if (HtmlParser.GetMetaRobotsNofollow() && Parent->GetCanRobots()) {
        (*Result)+="[links rskip]";    
        Links.RemoveAll();
    } else if (HtmlParser.GetMetaData().Contains("ALKALINE:SKIPLINKS") || Parent->GetSkipLinks()) {
        (*Result)+="[links mskip]";    
        Links.RemoveAll();
    } else {
        (*Result)+="[lnx]";
        HtmlParser.GetLinks(m_IndexOptions.m_IndexingExts, Links, NewBase, Parent->GetEnableExcludeVerbose());
    }
    
    // skip MD5 if NoMd5=Y
    if (!Parent->GetCanMd5()) {
        (*Result)+="[md5-skip]";    
    } else {
        (*Result)+="[md5]";
        CString URLVectorI;

        Trace(tagIndex, levInfo, ("CIndex::IndexFileMD5 - {%s}, md5=%s.", FileName.GetBuffer(), FileIndexVector[6].GetBuffer()));

        if (IndexFileMD5(FileIndexVector, DocumentDirectory, FileName, URLVectorI)) {
            if (FileName != URLVectorI) {

                Trace(tagIndex, levInfo, ("CIndex::IndexFileMD5 - {%s} and {%s} matched, md5=%s.", 
                      FileName.GetBuffer(), 
                      URLVectorI.GetBuffer(),
                      FileIndexVector[6].GetBuffer()));

                (*Result)+=("[md5-copy]\n[MD5 reported identical file with " + URLVectorI + " - skipping]\n");        
            } else {
                // now also reject same file reindexed twice
                (*Result)+=("[md5-unch]\n");
            }
            return;
        }
    } 

    int UrlPos = IndexFileSave(FileName, FileIndexVector, Links, Result);

    // check whether we have an UrlIndex or an UrlSkip directive
    if (!Parent->IsIncludedUrlIndex(FileName)) {
        (*Result)+="[excluded per UrlIndex]\n";
        return;
    }
    if (Parent->IsExcludedUrlSkip(FileName)) {
        (*Result)+="[excluded per UrlSkip]\n";
        return;
    }

    IndexAddMetaWords(Extension, RemoteFile.GetValue("Content-Type"), HtmlParser, Parent, UrlPos, Result);
    (*Result)+="[ok]\n";  
}

void CIndex::IndexActiveXObjects(
    CAlkalineParser& HtmlParser, 
    const CString& FileName,
    const CString& NewBase,
    const CString& DataText,
    CSite * Parent, 
    CString * Result) {
    
    // _L_DEBUG(4, cout << "CIndex::IndexFileAll() - looking at ActiveX objects" << endl);
    
    for (register int i=0;i<(int)HtmlParser.GetActiveXObjects().GetSize();i++) {
        // we have ActiveX objects, execute the appropriate filter
        CString ObjectData;
        CStringTable ObjectTable = HtmlParser.GetActiveXObjects()[i];
        ObjectTable.Add("URL", FileName);
        ObjectTable.Set("BASE", NewBase);
        if (Parent->ExecuteObject(ObjectTable, ObjectData, false, Result)) {      
            HtmlParser.Parse(DataText + ObjectData);
        }
    }
}

void CIndex::IndexFileInfVector(
    CVector<CString>& FileIndexVector,
    CAlkalineParser& HtmlParser,
    CRemoteFile& RemoteFile,    
    const CString& Server,
    CString& DataText,
    CSite * Parent,
    CString * Result) {

    CVector<CString> CustomMetas;
    Parent->GetCustomMetas(&CustomMetas);
    
    FileIndexVector.SetDim(7 + CustomMetas.GetSize());
    
    // [last modified]
    CString DateLastModified = RemoteFile.GetValue(g_strHttpLastModified);
    CString DateServer = RemoteFile.GetValue(g_strHttpDate);
    
    Trace(tagIndex, levInfo, ("CIndex::Index() - Last-Modified: [%s]", DateLastModified.GetBuffer()));
    Trace(tagIndex, levInfo, ("CIndex::Index() - Date: [%s]", DateServer.GetBuffer()));
    
    if (DateLastModified.GetLength()) {
        FileIndexVector += DateLastModified;
    } else {
        FileIndexVector += DateServer;
    }

    // [size] 
    Trace(tagIndex, levInfo, ("CIndex::Index() - Size: [%d]", RemoteFile.GetRData().GetLength()));
    FileIndexVector += CString::IntToStr(RemoteFile.GetRData().GetLength());
    
    // [date]
    FileIndexVector += DateServer;

    // [title]
    Trace(tagIndex, levInfo, ("CIndex::Index() - Title: [%s]", HtmlParser.GetTitle().GetBuffer()));
    FileIndexVector += HtmlParser.GetTitle();                                                            
    
    // document short description text
    if (Parent->GetCanMetaDescription() && (HtmlParser.GetMetaDescription().GetLength())) {
        FileIndexVector+=HtmlParser.GetMetaDescription();
    } else if (Parent->GetCanTextDescription()) {
        FileIndexVector += HtmlParser.GetHeaderText();
    } else {
        FileIndexVector += "";
    }

    // [server]
    Trace(tagIndex, levInfo, ("CIndex::Index() - Server: [%s]", Server.GetBuffer()));    
    FileIndexVector += Server;                                                                           
    
    if (m_IndexOptions.m_Insens) {
        DataText.UpperCase();
    }
    
    // md5
    CString Md5Value;

    CMd5::Md5Calculate(DataText, & Md5Value);

    Trace(tagIndex, levInfo, ("CIndex::IndexFileMD5 - md5len=%d.", Md5Value.GetLength()));

    assert(Md5Value.GetLength() == 16);

    for (register int j=((int)Md5Value.GetLength()) - 1;j>=0;j--)
        if (((unsigned char)Md5Value[j]) < 32) {
            Md5Value.Insert(j + 1, CString::IntToStr((unsigned int)Md5Value[j]));
            Md5Value[j] = '\\';
        }
    
    FileIndexVector += Md5Value;                                                                         // _L_DEBUG(4, cout << "CIndex::Index() - MD5: [" << FileIndexVector[FileIndexVector.GetSize()-1] << "]" << endl);
    
    // custom meta tags ---------------------------------------------------------------------------------------------
    CString MetaCandidate;
    if (CustomMetas.Contains(g_strStar)) {
        for (register int CustomMetaIndex=0; CustomMetaIndex < (int) HtmlParser.GetMetaRawData().GetSize(); CustomMetaIndex++) {
            MetaCandidate = HtmlParser.GetMetaRawData().GetValueAt(CustomMetaIndex);
            MetaCandidate.Replace(0, 31, ' ');
            MetaCandidate.RemoveDuplicate(0, ' ');            
            FileIndexVector += (HtmlParser.GetMetaRawData().GetNameAt(CustomMetaIndex) + ':' + MetaCandidate);
        }
        (*Result)+=("[all cm:" + CString::IntToStr(HtmlParser.GetMetaRawData().GetSize()) + "]");
    } else {        
        for (register int CustomMetaIndex=0; CustomMetaIndex < (int) CustomMetas.GetSize(); CustomMetaIndex++) {            
            if (HtmlParser.GetMetaData(CustomMetas[CustomMetaIndex], &MetaCandidate)) {
                (*Result)+=("[cm:" + CustomMetas[CustomMetaIndex] + "]");
                // cout << CustomMetas[CustomMetaIndex] << "=>" << MetaCandidate << endl;
                FileIndexVector += (CustomMetas[CustomMetaIndex] + ':' + MetaCandidate);
            }
        }    
    }
}
    
bool CIndex::IndexFileMD5(const CVector<CString>& FileIndexVector, 
                          const CString& DocumentDirectory, 
                          const CString& /* FileName */, 
                          CString& URLVectorI) {

    bool bResult = false;
    
    // get the list of url indexes with the same MD5
    CIntVector DuplicateMd5;

    m_INFManager.StartReading();
    if (m_INFManager.GetDuplicateMd5(FileIndexVector[6], DuplicateMd5)) {

        Trace(tagIndex, levInfo, ("CIndex::IndexFileMD5 - %d identical md5 elements.", DuplicateMd5.GetSize()));

        CIntVectorIterator VectorIterator(DuplicateMd5);
        while (* VectorIterator) {            
            URLVectorI = GetURLLink(VectorIterator++);
            if (m_IndexOptions.m_NSF || 
                ((URLVectorI.GetLength() >= DocumentDirectory.GetLength()) 
                  && URLVectorI.StartsWith(DocumentDirectory))) {
                bResult = true;
                break;
            }
        }
    }
    m_INFManager.StopReading();
    
    return bResult;
}

int CIndex::IndexFileSave(const CString& FileName, 
                          const CVector<CString>& FileIndexVector, 
                          const CVector<CString>& Links, 
                          CString * Result) {    

    CIntVector VectorIndex;
    (*Result)+="[vix]";
    GetURLVectorIndex(Links, VectorIndex);    
    int UrlPos = SetLNXVector(FileName, VectorIndex);    
    CVector<CString> Dummy;

    m_INFManager.StartWriting();
    m_INFManager.EnsureSize(UrlPos + 1);
    m_INFManager.PutAt(FileIndexVector, UrlPos);
    m_INFManager.SaveMd5(FileIndexVector[6], UrlPos);
    m_IndexOptions.m_Modified = true;    
    m_INFManager.StopWriting();

    (*Result)+="[keys]";

    bool bAlreadyIndexed = m_Searcher.AlreadyIndexed(UrlPos);
    if (bAlreadyIndexed) {
        m_Searcher.RemovePage(UrlPos);    
    }

    return UrlPos;
}

void CIndex::GetURLVectorIndex(const CVector<CString>& Vector, CIntVector& Target) {
    // fast version of GetURLIndex
    // _L_DEBUG(4, cout << "CIndex::GetURLVectorIndex" << endl);
    for (int i=0;i<(int) Vector.GetSize();i++) {    
        Target += GetURLIndex(Vector[i]);
    }
    // _L_DEBUG(4, cout << "CIndex::GetURLVectorIndex - returning vector of size " << Target.GetSize() << endl);
}

void CIndex::GetURLVectorStrings(int UrlPos, CVector<CString> * pVector) const {
    CIntVector LinksIntVector;
    
    m_LNXManager.StartReading();
    LinksIntVector = m_LNXManager.GetLNXVector()[UrlPos];
    m_LNXManager.StopReading();

    pVector->RemoveAll();
    CIntVectorIterator VectorIterator(LinksIntVector);
    while (* VectorIterator)
        (* pVector) += GetURLLink(VectorIterator++);    
}

CString CIndex::GetURLLink(const int Val) const {    
    CString Result;
    m_URLManager.StartReading();    
    if ((Val < (int) m_URLManager.GetUrlTree().GetSize())&&(Val >= 0))
        Result = m_URLManager.GetUrlTree()[Val];  
    m_URLManager.StopReading();
    return Result;
}

int CIndex::GetURLIndex(const CString& Url){
    
    bool Added = false;
    CVector<CString> Dummy;
    CIntVector EmptyIntVector;    
    
    // the lock must hold at least for one manage,
    // otherwise a write may kick in and have different sizes of the three managers
    m_URLManager.StartWriting(); 
    // ensure that we never reuse the zero value (for CSVector)
    if (!m_URLManager.GetSize())
        m_URLManager.Add("", &Added);
    int Res = m_URLManager.Add(Url, &Added);

    Trace(tagIndex, levInfo, ("CIndex::GetURLIndex - added/found %s at %d", Url.GetBuffer(), Res));
    
    m_URLManager.StopWriting();
    
    if (Added) {
        m_INFManager.StartWriting();
        m_INFManager.Add(Dummy);
        m_INFManager.StopWriting();
        
        m_LNXManager.StartWriting();
        m_LNXManager.GetLNXVector()+=EmptyIntVector;
        m_LNXManager.StopWriting();
    }
        
    return Res;
}

int CIndex::FindURLIndex(const CString& Url) const {
    int nResult;

    // _L_DEBUG(4, cout << "CIndex::FindURLIndex() - Looking for URLIndex of " << Url << endl);

    m_URLManager.StartReading(); 
    // _L_DEBUG(4, cout << "CIndex::FindURLIndex() - mutex acquired for " << Url << endl);
    nResult = m_URLManager.GetUrlTree().FindNodeIndex(Url);
    m_URLManager.StopReading(); 

    // _L_DEBUG(4, cout << "CIndex::FindURLIndex() - Returning " << nResult<< endl);

    return nResult;
}

// set the LNXVector at FileName with new links
int CIndex::SetLNXVector(const CString& FileName, const CIntVector& Links){
    // _L_DEBUG(4, cout << "CIndex::SetLNXVector - [" << FileName << "][" << Links << "]" << endl);
    bool Added = false;   

    // the lock must hold at least for one manage,
    // otherwise a write may kick in and have different sizes of the three managers

    m_URLManager.StartWriting();    
    // ensure that we never reuse the zero value (for CSVector)
    if (!m_URLManager.GetSize())
        m_URLManager.Add("", &Added);
    int URLManagerOldSize = m_URLManager.GetUrlTree().GetSize();
    int UrlPos = m_URLManager.Add(FileName, &Added);

    Trace(tagIndex, levInfo, ("CIndex::GetURLIndex - added/found %s at %d", FileName.GetBuffer(), UrlPos));

    assert(UrlPos <= URLManagerOldSize);
    m_URLManager.StopWriting();

    if (Added) {                
        m_INFManager.StartWriting();
        m_INFManager.EnsureSize(UrlPos + 1);
        m_INFManager.StopWriting();
    }
    
    SetLNXVector(UrlPos, Links);
    // _L_DEBUG(4, cout << "CIndex::SetLNXVector - [" << FileName << "] returning [" << UrlPos << "]" << endl);
    
    return UrlPos;
}

int CIndex::SetLNXVector(int UrlPos, const CIntVector& Links){
    // _L_DEBUG(4, cout << "CIndex::SetLNXVector - [" << UrlPos << "][" << Links << "]" << endl);
    
    m_LNXManager.StartWriting(); 
    m_LNXManager.EnsureSize(UrlPos + 1);
    m_LNXManager.GetLNXVector()[UrlPos] = Links;
    m_LNXManager.StopWriting(); 
    
    // _L_DEBUG(4, cout << "CIndex::SetLNXVector - returning [" << UrlPos << "]" << endl);
    return UrlPos;
}

CString& CIndex::MapTermEach(CString& Term, const CIndexMapV& Parameters) const {
    if (Parameters.m_HashInfo.GetSize() < 6) {
        Term.Empty();
        return Term;
    }

    static const CString __IDX_RECENT("RECENT");
    static const CString __IDX_RECENT_COUNT("RECENT.COUNT");
    static const CString __IDX_EXPIRED("EXPIRED");
    static const CString __IDX_AGE("AGE");
    static const CString __IDX_URL("URL");
    static const CString __IDX_MODIF("MODIF");
    static const CString __IDX_DATE("DATE");
    static const CString __IDX_MODIF_FRENCH("MODIF.FRENCH");
    static const CString __IDX_SIZE("SIZE");
    static const CString __IDX_DATE_FRENCH("DATE.FRENCH");
    static const CString __IDX_TITLE("TITLE");
	static const CString __IDX_TITLE_HL("HLTITLE");
    static const CString __IDX_HEADER("HEADER");
	static const CString __IDX_HEADER_HL("HLHEADER");
    static const CString __IDX_SERVER("SERVER");
    static const CString __IDX_INDEX("INDEX");
    static const CString __IDX_QUALITY("QUALITY");
	static const CString __IDX_GROUPSIZE("GROUP.SIZE");
    static const CString __IDX_RELEVANCE("RELEVANCE");
	static const CString __IDX_SORT("SORT");

    if (Term.Same(__IDX_RECENT)) Term = Parameters.m_Recent;
    else if (Term.Same(__IDX_RECENT_COUNT)) Term = Parameters.m_RecentCount;
    else if (Term.Same(__IDX_EXPIRED)) Term = Parameters.m_Expired;
    else if (Term.Same(__IDX_AGE)) Term = Parameters.m_RecentCount;
    else if (Term.Same(__IDX_URL)) Term = Parameters.m_Url;
    //    [last-modified][content-length][date][title][header][server]
    else if (Term.Same(__IDX_MODIF)) { Term = Parameters.m_ModifDateObject.Map(Parameters.m_DateMap); }
    else if (Term.Same(__IDX_DATE)) { Term = Parameters.m_DateObject.Map(Parameters.m_DateMap); }
    else if (Term.Same(__IDX_MODIF_FRENCH)) { Term = Parameters.m_ModifDateObject.Map("$DAYFRENCH, $DAY $MONTHFRENCH $YEAR ($HOUR:$MIN)"); }
    else if (Term.Same(__IDX_DATE_FRENCH)) { Term = Parameters.m_DateObject.Map("$DAYFRENCH, $DAY $MONTHFRENCH $YEAR ($HOUR:$MIN)"); }
    else if (Term.Same(__IDX_SIZE)) Term = Parameters.m_HashInfo[1];
    else if (Term.Same(__IDX_SORT)) Term = Parameters.m_Parent->GetSearchObject().m_SortType;
    else if (Term.Same(__IDX_TITLE)) { Term = Parameters.m_HashInfo[3]; if (!Term.GetLength()) Term = Parameters.m_Url; }
	else if (Term.Same(__IDX_TITLE_HL)) { 
		Term = Parameters.m_HashInfo[3]; 
		if (!Term.GetLength()) Term = Parameters.m_Url; 
		Term = Highlight(
			Term,			
			Parameters.m_Parent->GetSearchObject().m_DigestedSearchTerms, 
			Parameters.m_Parent->GetValue("HIGHLIGHT-OPEN"),
			Parameters.m_Parent->GetValue("HIGHLIGHT-CLOSE"),
			Parameters.m_Parent->GetSearchObject().m_SearchOptions
			);
	} else if (Term.Same(__IDX_HEADER)) Term = Parameters.m_HashInfo[4];
	else if (Term.Same(__IDX_HEADER_HL)) { 
		Term = Highlight(
			Parameters.m_HashInfo[4], 
			Parameters.m_Parent->GetSearchObject().m_DigestedSearchTerms, 
			Parameters.m_Parent->GetValue("HIGHLIGHT-OPEN"),
			Parameters.m_Parent->GetValue("HIGHLIGHT-CLOSE"),
			Parameters.m_Parent->GetSearchObject().m_SearchOptions
			);
	} else if (Term.Same(__IDX_SERVER)) Term = Parameters.m_HashInfo[5];
    else if (Term.Same(__IDX_QUALITY) || Term.Same(__IDX_RELEVANCE)) Term = CString::IntToStr(CSearch::GetQuality(* Parameters.m_SearchData, Parameters.m_ResultPosition));
	// quality is used for group size
	else if (Term.Same(__IDX_GROUPSIZE)) Term = CString::IntToStr(Parameters.m_SearchData->m_ResultsQuality[Parameters.m_Index]);
    else if (Term.Same(__IDX_INDEX)) Term = CString::IntToStr(Parameters.m_Index);  
    else {
        // custom meta tags
        if (Parameters.m_HashInfo.GetSize() > 6) {
            CString TermMeta(Term);
            TermMeta += ':';
            for (register int PIndex=6;PIndex < (int) Parameters.m_HashInfo.GetSize(); PIndex++) {
                if (Parameters.m_HashInfo[PIndex].StartsWithSame(TermMeta)) {
                    Parameters.m_HashInfo[PIndex].Right(Parameters.m_HashInfo[PIndex].GetLength() - TermMeta.GetLength(), &Term);
                    return Term;
                }
            }
        }
        return Parameters.m_Parent->MapTermEach(Term, 0);
    }

    return Term;
}

CString& CIndex::MapTerm(const CString& Source, CString& Target, const CIndexMapV& Parameters) const {
    MAP_TERM_MACRO(Source, Target, MapTerm, MapTermEach, Parameters.m_ForceQuote, Parameters);
}

CString CIndex::GetMapTerm(const CString& Source, const CIndexMapV& Parameters) const {
    CString Result;	
    MapTerm(Source, Result, Parameters);
	CHttpIo Io;
	Parameters.m_Parent->TraverseTags(Io, Result);
	Result.MoveFrom(Io.GetBuffer());
    return Result;
}

bool CIndex::CreateDateObjects(
    CSession& Parent,
    CIndexMapV& IndexMapStructure,
    struct tm& fDate,
    int curDayCount) {
    
    bool bResult = false;
    
    bResult |= CreateDateObject(
        Parent, 
        IndexMapStructure.m_HashInfo[2],
        fDate, 
        curDayCount,
        IndexMapStructure,
        IndexMapStructure.m_DateObject);
    
    bResult |= CreateDateObject(
        Parent, 
        IndexMapStructure.m_HashInfo[0],
        fDate, 
        curDayCount,
        IndexMapStructure,
        IndexMapStructure.m_ModifDateObject);
    
    return bResult;
}

 
bool CIndex::CreateDateObject(
    CSession& Parent,
    const CString& HashDate,
    struct tm& fDate,
    int curDayCount,
    CIndexMapV& IndexMapStructure,
    CDate& FSDateObject) {
    
    static const CString g_strRecentCount("RECENT-COUNT");
    static const CString g_strExpiredCount("EXPIRED-COUNT");
    static const CString g_strRecent("RECENT");
    static const CString g_strExpired("EXPIRED");
    
    if (HashDate.GetLength()) {
        
        CDate::EncodeDate(HashDate, fDate);
        
        int dCount = curDayCount - CDate::DayCount(fDate.tm_mon, fDate.tm_mday, fDate.tm_year);
        int ParentRecentCount = CString::StrToInt(Parent.GetValue(g_strRecentCount));
        int ParentExpiredCount = CString::StrToInt(Parent.GetValue(g_strExpiredCount));

        Trace(tagSearch, levInfo, ("CIndex::Search - CreateDateObject [%s][dcount: %d]", HashDate.GetBuffer(), dCount));
    
        IndexMapStructure.m_RecentCount = CString::IntToStr(dCount);
        
        if (dCount < ParentRecentCount) {
            IndexMapStructure.m_Recent = Parent.GetValue(g_strRecent);
        } else {
            IndexMapStructure.m_Recent.Empty();
        }
        
        if (dCount > ParentExpiredCount) {
            IndexMapStructure.m_Expired = Parent.GetValue(g_strExpired);
        } else {
            IndexMapStructure.m_Expired.Empty();
        }
    
        FSDateObject.SetTmTime(fDate);
        return true;
    } else return false;
}

bool CIndex::RemovePages(const CString& Root) {

    Trace(tagRemovePages, levInfo, ("CIndex::RemovePages from Root [%s]", Root.GetBuffer()));

    CIntVector IntVector;
    
    m_URLManager.StartReading(); 
    m_URLManager.GetUrlTree().FindNodeIndexes(Root, IntVector);
    m_URLManager.StopReading(); 

    Trace(tagRemovePages, levInfo, ("CIndex::RemovePages [%s] returned a vector of %d elements.", Root.GetBuffer(), IntVector.GetSize()));

    if (!IntVector.GetSize())
        return false;
    
    bool Result = false; 
    CIntVectorIterator VectorIterator(IntVector);
    int PageIndex;
    while (* VectorIterator) {    
        PageIndex = VectorIterator++;
    
        if (RemovePage(PageIndex)) {
            Result = true;
        }

    }

    return Result;
}

void CIndex::SetModifiedFlag(void) {
    if (!m_IndexOptions.m_Modified) {
        m_INFManager.StartWriting();
        m_IndexOptions.m_Modified = true;
        m_INFManager.StopWriting();
    }
}

bool CIndex::RemovePage(const CString& Page) {  
    if (Page.EndsWith("/*"))
    {
        CString MidString;
        Page.Mid(0, Page.GetLength() - 2, &MidString); 

        Trace(tagRemovePages, levInfo, ("CIndex::RemovePages [%s]", MidString.GetBuffer()));

        return RemovePages(MidString);
    }
    
    int UrlPos = FindURLIndex(Page);
    
    Trace(tagRemovePages, levInfo, ("CSite::FindUrlIndex [%s] returned %d.", Page.GetBuffer(), UrlPos));

    if (UrlPos == -1) {        
        return RemovePages(Page);
    }
    
    return RemovePage(UrlPos);
}

bool CIndex :: RemovePage(int UrlPos) {

    bool bAlreadyIndexed = m_Searcher.AlreadyIndexed(UrlPos);
    
    Trace(tagRemovePages, levInfo, ("CSite::AreadyIndexed returned [%s].", bAlreadyIndexed ? "yes" : "no"));
    
    if (! bAlreadyIndexed) {        
        return false;
    }
    
    SetModifiedFlag();
    
    m_Searcher.RemovePermanently(UrlPos);
    cout << "[removing " << GetURLLink(UrlPos) << "]" << endl;
    
    m_LNXManager.StartWriting(); 
    m_LNXManager.GetLNXVector()[UrlPos].RemoveAll();
    m_LNXManager.StopWriting(); 
    
    m_INFManager.StartWriting();
    for (int t = (int) m_INFManager.GetSize(UrlPos) - 1; t >= 0;t--)
        m_INFManager.PutAt(CString::EmptyCString, UrlPos, t);
    m_INFManager.StopWriting();

    return true;
}

CIndex& CIndex::Append(CIndex& Index) {

    if (! AppendIndex(Index)) {
        cout << "[error occurred within the merge subsystem]" << endl;
    }

    return * this;
}

bool CIndex::AppendIndex(CIndex& Index) {
    // first, merge URL trees and construct the URL displacements table
    // that contains which URL id becomes which new URL id

    if (! LoadIndexURT())
        return false;
    if (! Index.LoadIndexURT())
        return false;

    m_URLManager.StartWriting(); 
    CVector<int> IndexURLDispTable;
    m_URLManager.AppendGetDispTable(Index.GetURLManager(), IndexURLDispTable);
    m_URLManager.StopWriting(); 

    Index.GetURLManager().RemoveAll();

    if (! LoadIndexLNX())
        return false;
    if (! Index.LoadIndexLNX())
        return false;

    // merge LNX table, links from the URLs
    m_LNXManager.StartWriting(); 
    m_LNXManager.Append(Index.GetLNXManager(), IndexURLDispTable, this);
    m_LNXManager.StopWriting(); 

    Index.GetLNXManager().RemoveAll();

    if (! LoadIndexINF(false))
        return false;
    if (! Index.LoadIndexINF(false))
        return false;

    // merge the INF table
    m_INFManager.StartWriting();
    m_INFManager.Append(Index.GetINFManager(), IndexURLDispTable, this);
    m_INFManager.StopWriting();

    Index.GetINFManager().RemoveAll();
    
    if (! LoadIndexNDX(false))
        return false;
    if (! Index.LoadIndexNDX(false))
        return false;

    // merge the search table
    Index.GetSearcher().AppendTo(m_Searcher, IndexURLDispTable);

    Index.GetSearcher().RemoveAll();

    // write the index
    SetModifiedFlag();
    return true;
}

void CIndex::SetCookies(const CVector<CString>& Cookies) {
    for (register int i=0;i<(int) Cookies.GetSize();i++) {
        if (!m_CookieStorage.Add(Cookies[i]))
            cerr << "[Error! Malformed cookie {" << Cookies[i] << "}]" << endl;
    }
}

void CIndex::PopulateXmlNode(CXmlTree& Tree, CTreeElement< CXmlNode > * pXmlNode) const {
    m_URLManager.StartReading(); 
    Tree.SetValue("/processed/modified", CString::IntToStr(m_IndexOptions.m_ModifiedCount.Get()));
    Tree.SetValue("/processed/words", CString::IntToStr(m_Searcher.GetWordCount()));
    Tree.SetValue("/processed/urls", CString::IntToStr(m_URLManager.GetUrlTree().GetSize()));  
    m_URLManager.StopReading();
    m_Searcher.PopulateXmlNode(Tree, pXmlNode);
}

bool CIndex::RemoveWords(const CVector<CString>& Words, bool bRegExp) {
    
    Trace(tagExcludeWords, levInfo, ("CIndex::RemoveWords - loading ndx.", Words.GetSize()));
    
    if (! LoadIndexNDX(false))
        return false;
    
    int nWordsCount = m_Searcher.GetWordCount();
    
    Trace(tagExcludeWords, levInfo, ("CIndex::RemoveWords - processing %d words.", nWordsCount));    
    
    if (m_Searcher.RemoveWords(Words, bRegExp)) {
        cout << "[writing ndx]";
        m_Searcher.StartReading();
        m_Searcher.WriteIndex(m_IndexOptions.m_CurrentFilenameNdx, true); 
        m_Searcher.StopReading();

        if (nWordsCount) {
            float nReduction = (1 - (float) m_Searcher.GetWordCount() / nWordsCount) * 100;
            cout << "[index reduced by " << CString::FloatToStr(nReduction, 0, 2) << "%]";
        }

        cout << endl;
        return true;
    } else return false;
}

void CIndex::AppendSucceededAuthState(CSite * Parent, const CRemoteFile& RemoteFile, CString * Result) {

    int nSucceededAuthIndex = RemoteFile.GetHttpRequest().GetServerAuthState().GetSucceededAuthIndex();
    
    if (nSucceededAuthIndex >= 0) {

        if (nSucceededAuthIndex != 0) {
            m_INFManager.StartWriting();
            if (nSucceededAuthIndex < (int) Parent->GetAuth().GetSize()) {
                // exchange succeeded authentication to be on top                
                CStringPair SucceededPair = Parent->GetAuth()[nSucceededAuthIndex];
                ((CStringPair&) Parent->GetAuth()[nSucceededAuthIndex]) = Parent->GetAuth()[0];
                ((CStringPair&) Parent->GetAuth()[0]) = SucceededPair;
            }
            m_INFManager.StopWriting();
        }

        (* Result) += "[auth:";
        CString Username;
        CString Domain;
        if (RemoteFile.GetHttpRequest().GetServerAuthState().GetCredentialsAt(nSucceededAuthIndex, & Username, NULL, & Domain)) {
            if (Domain.GetLength()) {
                (* Result) += Domain;
                (* Result) += "\\";
            }
            * Result += Username;
        } else {
            (* Result) += "<local>";
        }
        (* Result) += "]";
    }

}

CString CIndex :: Highlight(
	const CString& String, 
	const CVector<CString>& QueryArray, 
	const CString& Left, 
	const CString& Right,
	const CSearchOptions& SearchOptions) const {	

	CString Result = String;		
	for (register int i = 0; i < (int) QueryArray.GetSize(); i++) {
		
		int curPos = 0;
		
		do {

			if (SearchOptions.m_OptCase)
				curPos = Result.Pos(QueryArray[i], curPos);
			else curPos = Result.SamePos(QueryArray[i], curPos);

			if (curPos == -1)
				break;

			if (SearchOptions.m_OptWhole) {
				if ((curPos > 1) && isalnum(Result[curPos - 1])) {
					curPos += QueryArray[i].GetLength();
					continue;
				}
				if ((curPos + QueryArray[i].GetLength() + 1 < Result.GetLength()) && 
					isalnum(Result[curPos + QueryArray[i].GetLength()])) {
					curPos += QueryArray[i].GetLength();
					continue;
				}
			}

			Result.Insert(curPos + QueryArray[i].GetLength(), Right);
			Result.Insert(curPos, Left);

			curPos += (Right.GetLength() + Left.GetLength() + QueryArray[i].GetLength());
		} while (curPos != -1 && curPos < (int) Result.GetLength());
	}
	return Result;
}
