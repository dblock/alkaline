/*
    © Vestris Inc., Geneva, Switzerland
    http://www.vestris.com, 1994-1999 All Rights Reserved
    ______________________________________________

    written by Daniel Doubrovkine - dblock@vestris.com
  
    Revision history:
    
      27.08.1999: in IsIncluded return Remote=Y option when not explicitely forced to index
      18.03.2000: fixed non-metatags parsing, eg. url:server:port/path that was treated as a metatag
      13.04.2001: broken into parts
      13.04.2001: added NoCookies support
      
*/

#include <alkaline.hpp>

#include "Site.hpp"
#include <Main/TraceTags.hpp>

CSite::CSite(void) {    
    Initialize();
}

CSite::CSite(const CString& Alias, const CString& ConfigPath, bool /* Lazy */) {
    m_Alias = Alias;
    m_ConfigPath = ConfigPath;
    Initialize();
}

void CSite::Initialize(void) {
    m_IsWriting = false;
    m_State = CsVoid;
    m_ShowRequests = false;
    m_Enable404 = true;
    m_NewOnly = false;
    m_Once = false;
    m_ReindexCount = 0;
    m_Verbose = false;
    m_EnableExcludeVerbose = false;
    m_EnableVerboseParser = false;
    m_ForceSleepFile = -1;
    m_ForceSleepRoundtrip = -1;
    m_ForceExpire = false;
    m_ForceNoReindex = false;
    m_CurrentConfiguration = NULL;
    memset(& m_Stat, 0, sizeof(struct_stat));
}
    
CSite::~CSite(void){
    
}

bool CSite::HasChanged(void) const {
    CString Filename = m_ConfigPath; 
    CLocalPath::Terminate(Filename); 
    Filename += "asearch.cnf";
    
    struct_stat NewStat; 
    base_stat((const char *) Filename.GetBuffer(), &NewStat);

    if (m_Stat.st_mtime == NewStat.st_mtime) 
        return false;
    return true;
}


void CSite::LoadSiteIndex(bool bLoadFiles){
    if (!m_IndexFile.GetLength()) {
        m_IndexFile = m_ConfigPath;
        CLocalPath::Terminate(m_IndexFile);
        CLocalPath FullIndexPath(m_IndexFile);        
        m_IndexFile = FullIndexPath.GetFullPath();
        
        m_IndexFile += "siteidx";
        m_SiteIndex.SetIndex(m_IndexFile, bLoadFiles);        
    }
}

void CSite::ReloadSiteIndex(void) {
  m_SiteIndex.SetIndex(m_IndexFile);
}

void CSite::Write(bool Lazy, bool bForce) {  
	if (m_IsWriting)
		return;
    m_IndexMutex.StartWriting();
    if (bForce || 
        (m_SiteIndex.m_IndexOptions.m_ModifiedCount.Get()) &&
        (
            ((m_CurrentConfiguration->GetWriteIndex() > 0) &&
             (m_SiteIndex.m_IndexOptions.m_ModifiedCount.Get() % m_CurrentConfiguration->GetWriteIndex() == 0))
            ||
            ! m_CurrentConfiguration->GetWriteIndex()
            ) &&
        (!m_SiteIndex.m_IndexOptions.m_GatherEmail)) {
        m_IsWriting = true;
        m_SiteIndex.Write((!Lazy  && (!m_SiteIndex.m_IndexOptions.m_GatherEmail)) || (m_Verbose));
        m_IsWriting = false;
    }
    m_IndexMutex.StopWriting();
}

void CSite::WriteLog(const CString& Filename, const CString& String, CSession& Session) const {
    Session.WriteLog(Filename, String);
}

CString CSite::Map(const CString& String, const CVector<CString>& Vector) {
    CString Result;
    int DPos = String.Pos('$');
    int i, PPos=0;
    CString MidValue;
    while (DPos >= 0) {
        String.Mid(PPos, DPos - PPos, &MidValue);
        Result += MidValue;
        i = ++DPos;
        while ((i < (int) String.GetLength()) && isdigit(String[i])) 
            i++;
        PPos = i;
        if (i != DPos) {
            i = String.GetInt(DPos, i - DPos);
            if (i>=0) {
                if ((int) Vector.GetSize() > i) 
                    Result+=Vector[i];
            }
        }
        DPos = String.Pos('$', DPos);
    }
    String.Mid(PPos, String.GetLength(), &MidValue);
    Result+=MidValue;
    return Result;
    
}

bool CSite::RemovePage(const CString& Page) {    
    Trace(tagRemovePages, levInfo, ("CSite::RemovePage [%s]", Page.GetBuffer()));
    LoadSiteIndex();
    return m_SiteIndex.RemovePage(Page);
}

CSite& CSite::Append(CSite& Site) {
    m_SiteIndex.Append(Site.GetSiteIndex());
    return * this;
}

// void CSite::MakeAlreadyIndexedOnce(bool Verbose) {
//   m_SiteIndex.GetSearcher().MakeAlreadyIndexedOnce(Verbose);
// }

void CSite::PopulateXmlNode(CXmlTree& Tree, CTreeElement< CXmlNode > * pXmlNode) const {
  CXmlNode ConfigNode;
  ConfigNode.SetType(xmlnOpen);
  CString RConfigPath(m_ConfigPath);  
  RConfigPath.Replace('/', '\\');
  RConfigPath.Replace("\\", "%5C");
  RConfigPath.Replace(":", "%3A");
  
  ConfigNode.SetData(RConfigPath);
  
  CTreeElement< CXmlNode > * NewXmlNodePtr = Tree.AddChildLast(pXmlNode, ConfigNode);
    
  static CString SiteXml(
    "<path></path>"                             \
    "<vsize></vsize>"                           \
    "<alias></alias>"                           \
    "<processed>"                               \
    " <size></size>"                            \
    " <modified></modified>"                    \
    " <words></words>"                          \
    " <urls></urls>"                            \
    " <last>N/A</last>"                         \
    "</processed>"                              \
	"<threads>"                                 \
	" <max></max>"                              \
	" <rampup></rampup>"                        \
	" <maxqueue></maxqueue>"                    \
	" <maxidle></maxidle>"                      \
	" <jobs></jobs>"                            \
	" <threads></threads>"                      \
	" <waiting></waiting>"                      \
    "</threads>"                                \
    "<cnf>"                                     \
    "</cnf>"                                    \
    "<search>"                                  \
    "</search>");
  
  CXmlTree XmlTree;
  XmlTree.SetXml(SiteXml);
  
  m_IndexMutex.StartReading();
  XmlTree.SetValue("/path", RConfigPath);
  XmlTree.SetValue("/vsize", CString::IntToStr(m_SiteConfigs.GetSize()));
  XmlTree.SetValue("/alias", m_Alias);
  XmlTree.SetValue("/processed/size", CString::IntToStr(m_ProcessedFiles.Get()));
  if (m_LastProcessedUrl.GetLength()) {
    XmlTree.SetValue("/processed/last", m_LastProcessedUrl);
  }
  
  XmlTree.SetValue("/threads/max", CString::IntToStr(m_IndexPool.GetMaxThreads()));
  XmlTree.SetValue("/threads/rampup", CString::IntToStr(m_IndexPool.GetRampupThreads()));
  XmlTree.SetValue("/threads/maxqueue", CString::IntToStr(m_IndexPool.GetMaxQueueSize()));
  XmlTree.SetValue("/threads/maxidle", CString::IntToStr(m_IndexPool.GetMaxThreadIdle()) + " seconds");
  XmlTree.SetValue("/threads/jobs", CString::IntToStr(m_IndexPool.GetJobsList().GetSize()));
  XmlTree.SetValue("/threads/threads", CString::IntToStr(m_IndexPool.GetThreads().GetSize()));
  XmlTree.SetValue("/threads/waiting", CString::IntToStr(m_IndexPool.GetWaitingThreads().Get()));

  CTreeElement< CXmlNode > * XmlCnfNodePtr = XmlTree.XmlFind("/cnf");
  for (int i=0;i<(int) m_SiteConfigs.GetSize();i++) {
    CXmlNode ConfigNode;
    ConfigNode.SetType(xmlnOpen);
    ConfigNode.SetData("vcnf-" + CString::IntToStr(i));  
    m_SiteConfigs[i].PopulateXmlNode(XmlTree, XmlTree.AddChildLast(XmlCnfNodePtr, ConfigNode)); 
    ConfigNode.SetType(xmlnClose);
    XmlTree.AddChildLast(XmlCnfNodePtr, ConfigNode);
  }

  m_SiteIndex.PopulateXmlNode(XmlTree, XmlTree.XmlFind("/search")); 

  // move the full tree
  Tree.MoveAsChildLast(XmlTree, XmlTree.GetHead(), NewXmlNodePtr);
  
  m_IndexMutex.StopReading();
  
  ConfigNode.SetType(xmlnClose);
  XmlTree.AddChildLast(pXmlNode, ConfigNode);  
}

// remove words that come from an ExcludeWords file
bool CSite::RemoveWords(const CVector<CString>& Filenames, bool bRegExp) {
    CVector<CString> Words;
    
    if (bRegExp) {
        CConfig::AddFromFilesToVector("Regexp ExcludeWords", Words, Filenames);
    } else {
        CConfig::AddFromFilesToVector("ExcludeWords", Words, Filenames);
    }
    
    Trace(tagExcludeWords, levInfo, ("CSite::RemoveWords - loaded %d words.", Words.GetSize()));
    
    if (!Words.GetSize()) {
        cout << "[No words to remove from the current database.]" << endl;
        return false;
    }
    
    LoadSiteIndex(false);
    
    Trace(tagExcludeWords, levInfo, ("CSite::RemoveWords - index locations set, removing words ..."));
    
    return m_SiteIndex.RemoveWords(Words, bRegExp);
}

bool CSite::GetParseContent(const CString& ContentType) const {
    
    if (!ContentType.GetLength())
        return true;    

    if (m_CurrentConfiguration->GetParseContent().GetSize() == 0)
        return true;   
    
    bool bParseContent = false;
    for (register int i = 0; i < (int) m_CurrentConfiguration->GetParseContent().GetSize(); i++) {
        CString CompContentType = m_CurrentConfiguration->GetParseContent()[i];
        if (!CompContentType.GetLength())
            continue;
        
        if (CompContentType == g_strStar) {
            bParseContent = true;
        } else if (CompContentType[0] == '-') {
            CompContentType.Delete(0, 1);
            if (CompContentType.Same(ContentType))
                return false;
        } else {
            if (CompContentType[0] == '+')
                CompContentType.Delete(0, 1);
            if (CompContentType.Same(ContentType))
                return true;
        }        
    }
    
    return bParseContent;    
}

void CSite::AppendRequestHeaders(CRemoteFile& RemoteFile) const {
    for (int i = 0; i < (int) m_CurrentConfiguration->GetRequestHeadersTable().GetSize(); i++) {
        RemoteFile.SetParameter(
            m_CurrentConfiguration->GetRequestHeadersTable().GetNameAt(i),
            m_CurrentConfiguration->GetRequestHeadersTable().GetValueAt(i));
    }
}
