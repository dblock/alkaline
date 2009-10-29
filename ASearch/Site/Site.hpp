/*

    © Vestris Inc., Geneva, Switzerland
    http://www.vestris.com, 1994-1999 All Rights Reserved
    ______________________________________________

    written by Daniel Doubrovkine - dblock@vestris.com

*/

#ifndef ALKALINE_SITE_HPP
#define ALKALINE_SITE_HPP

typedef enum { CsVoid, CsLoaded } CSiteState;

#include <platform/include.hpp>
#include <Config/Config.hpp>
#include <Session/Session.hpp>
#include <HashTable/HashTable.hpp>
#include <Index/Index.hpp>
#include <Index/IndexPool.hpp>
#include <Tree/XmlTree.hpp>
#include <RegExp/RegExp.hpp>
#include <Encryption/Encryption.hpp>
#include <String/GStrings.hpp>
#include <Mutex/RWNamedTable.hpp>

class CSite : public CObject {
    protected_property(int, ForceSleepFile);
    property(int, ForceSleepRoundtrip);
    
    protected_property(bool, ForceExpire);
    readonly_property(bool, ForceNoReindex);
    
    protected_property(struct_stat, Stat);
    copy_property(CConfig *, CurrentConfiguration);
    protected_property(CVector<CConfig>, SiteConfigs);
    readonly_property(CString, LastProcessedUrl);
    
    readonly_property(bool, IsWriting);
    readonly_property(CIndex, SiteIndex);
       
    protected_property(CSiteState, State);
    readonly_property(CString, ConfigPath);
    
    protected_property(CDate, Today);
    protected_property(CUrlTree, UniqueLinks);
    
    readonly_property(bool, EnableExcludeVerbose);
    readonly_property(bool, EnableVerboseParser);
    
    protected_property(bool, Verbose);
    protected_property(bool, ShowRequests);
    protected_property(bool, Enable404);
    protected_property(bool, NewOnly);
    protected_property(int, ReindexCount);
    readonly_property(bool, Once);
    
    readonly_property(CAtomic, ProcessedFiles);                          // total files processed (MaxSize)
    readonly_property(CAtomic, ProcessedLinks);                          // total links processed from top url (MaxLinks)
    protected_property(CString, IndexFile);
    readonly_property(CString, Alias);    
    protected_property(CHashTable<CStringVector>, ExcludeTable);
    mutable CRWMutex m_IndexMutex;
    mutable CRWMutex m_RobotsMutex;
    mutable CRWNamedTable m_RobotsMutexTable;

    readonly_property(CIndexPool, IndexPool);

    void Write(bool bLazy, bool bForce = false);

private:

    void Initialize(void);
    bool IsExcluded(const CString&);
    bool IsIncluded(const CString&);	
    void ProcessURL(const CString&, bool, const CString&, const CVector<CUrl>&);
    void AddIndexJob(const CString& Url, int Depth, bool Lazy, const CString& SearchTop, const CVector<CUrl>& RemoteHistory);
    void AddRobotsJob(const CString& Url, bool Lazy);
    
    void LoadRobots(const CString& Server, CVector<CString>& ExcludeVector, bool Lazy = false);
    void RetrieveRobot(const CString& Server, CVector<CString>& NewExclude, bool Lazy = false);
    bool IsExcludedDepth(const CUrl& Url) const;
    
    bool GetFinalURL(const CString& iFileName, CString& FinalUrl, bool Verbose);
public:
    void RetrieveRobots(const CUrl& Url, CVector<CString>& ExcludeVector, bool Lazy = false);
    bool IsExcludedRobots(const CUrl&, bool Lazy = false);
    void ProcessURL(const CIndexObject& IndexObject);
    void ReadConfig(bool);
    CString ProcessURLSingle(const CString& Url, bool Lazy);
    inline bool IsWriting(void) const;
    inline bool CanOnce(void) const;
    inline bool GetEnableObjects(void) const;
    inline bool GetLowerCase(void) const;
    inline bool GetUpperCase(void) const;
    inline bool GetForceExpire(void) const;
    inline int GetHeaderLength(void) const;
    inline const CVector<CStringPair>& GetAuth(void) const;
    inline void CopyReplaceTable(CStringTable& StringTable) const;
    inline void CopyRegexpReplaceTable(CStringTable& StringTable) const;
    inline int GetSleepRoundtrip(void) const;
    inline int GetSleepFile(void) const;
    inline bool GetCanRobots(void) const;    
    inline bool GetCanCookies(void) const;    
    inline bool GetCanMd5(void) const;    
    inline bool GetCanMetaDescription(void) const;
    inline bool GetCanTextDescription(void) const;
    inline bool GetSkipMeta(void) const;
    inline bool GetSkipText(void) const;
    inline bool GetSkipLinks(void) const;
    inline bool GetNewOnly(void) const;
    inline bool GetCustomMetas(CVector<CString> * pCustomMetas) const;
    inline bool GetParseMetas(CVector<CString> * pCustomMetas) const;

    bool GetCanReindex(void) const;
    inline bool ReplaceLocalUrl(CString& Url, CString * pReplaceUrl, CString * pResult) const;
    inline bool GetReplaceLocalSize(void) const;

    void AppendRequestHeaders(CRemoteFile& RemoteFile) const;
    
    CSite(void);
    CSite(const CString& Alias, const CString& Path, bool Lazy = true);
    virtual ~CSite(void);
    
    void RetrieveSite(bool Lazy);
    //        void MakeAlreadyIndexedOnce(bool Verbose);
    bool RemovePage(const CString&);
    bool RemoveWords(const CVector<CString>& Filenames, bool bRegExp);
    
    bool SearchSite(CSearchObject& /* SearchObject */);
    bool SimpleSearch(CSearchObject& /* SearchObject */);

    void ReloadSiteIndex(void);    
    void LoadSiteIndex(bool bLoadFiles = true);
    inline int GetModifiedFiles(void) const { return m_SiteIndex.m_IndexOptions.m_ModifiedCount.Get(); }
    inline void SetGatherEmail(const bool Value) { m_SiteIndex.m_IndexOptions.m_GatherEmail = Value; }
    inline void SetGatherEmailAll(const bool Value) { m_SiteIndex.m_IndexOptions.m_GatherEmailAll = Value; }
    void SetOptions(const CVector<CString>&);
    bool ExecuteObject(const CStringTable& /* ActiveXObject */, CString& /* TargetData */, bool /* Verbose */, CString * /* Result */) const;
    bool ExecutePlugin(const CString& Data, const CString& Extension, const CString& MimeType, const CString& FileName, CString& Target, CString * Result) const;
    bool ExecuteIndexWords(const CString& Extension, const CString& MimeType, CVector<CStringA>& Words);
    bool ExecutePluginWords(const CString& Extension, const CString& MimeType, CVector<CStringA>& Words);
    bool IncludePluginPages(const CString& Extension, const CString& MimeType, const CVector<CStringA>& Words, CString * Result);
    bool ExcludePluginPages(const CString& Extension, const CString& MimeType, const CVector<CStringA>& Words, CString * Result);

	bool ExecuteAndReadFilter(
		const CString& CommandLine,
		const CString& TargetFilename,
		CString& TargetData,
		CString * pResult,
		bool Verbose) const;

    bool ProcessPages(
        const CString& ProcessType, // "IncludePages" or "ExcludePages"
        CHashTable<CStringVector>& Table, // m_CurrentConfiguration->GetPagesIncludeTable()
        CHashTable<CStringVector>& RegexpTable, // m_CurrentConfiguration->GetRegexp_PagesIncludeTable()
        bool bOverride, // m_CurrentConfiguration->GetIncludePagesAll()
        const CString& Type, // *, an extension or a mime type
        const CVector<CStringA>& Words, // words to process
        int * pTotalSize, // total result set
        CString * Result, // result messages
        bool * pbResult // continue processing
        );
    
    bool PluginPages(
        const CString& Directive, // "IncludePages",
        CHashTable<CStringVector>& Table, // m_CurrentConfiguration->GetPagesIncludeTable(), 
        CHashTable<CStringVector>& RegexpTable, // m_CurrentConfiguration->GetRegexp_PagesIncludeTable(), 
        bool bOverride, // m_CurrentConfiguration->GetIncludePagesAll(),
        const CString& Extension, 
        const CString& MimeType, 
        const CVector<CStringA>& Words, 
        CString * Result
        );    

    void CollectLoadArray(
        const CString& ProcessType,
        CHashTable<CStringVector>& Table, // m_CurrentConfiguration->GetWordsExcludeTable()
        const CString& Type, // g_strStar
        CVector<CString>& Vector // WordsExcludeAll
        );

    void ExcludeWords(
        const CString& Directive, // "ExcludeWords"
        const CString& Type, // g_strStar
        CHashTable<CStringVector> & Table, // m_CurrentConfiguration->GetWordsExcludeTable()
        bool bRegExp,
        CVector<CStringA>& Words
        );
        
    bool IndexWords(
        const CString& Directive, // "IndexWords"
        const CString& Type, // g_strStar
        CHashTable<CStringVector> & Table, // m_CurrentConfiguration->GetWordsIndexTable()
        bool bRegExp,
        CVector<CStringA>& Words,
        CVector<CStringA>& NewWords
        );
    
    bool GetParseContent(const CString& ContentType) const;
    
    void WriteLog(const CString& Filename, const CString& String, CSession& Session) const;
    static void SetIndexOptions(const CConfig& Config, CIndex& Index);
    static CString Map(const CString&, const CVector<CString>&);
    
    bool IsMaxFilesLimitReached(void) const;
    bool IsMaxRemoteDepthReached(const CVector<CUrl>& RemoteHistory) const;
    
    /* merge, append functions */
    CSite& Append(CSite& Site);
    inline CSite& operator+=(CSite& Site) { return Append(Site); }
    bool HasChanged(void) const;

    /* UrlIndex and UrlSkip */
    bool IsIncludedUrlIndex(const CString& Url) const;
    bool IsExcludedUrlSkip(const CString& Url) const; 
  
    void PopulateXmlNode(CXmlTree& Tree, CTreeElement< CXmlNode > * pXmlNode) const;

    bool OnNewUrl(
        CString& TranslatedUrl, 
        const CString& Server,
        const CIndexObject& IndexObject,
        CString& OutputString
        );

};

inline bool CSite::IsWriting(void) const { 
  return GetIsWriting(); 
}

inline bool CSite::CanOnce(void) const { 
  return ((!m_Once) || (!m_ReindexCount)); 
}

inline bool CSite::GetEnableObjects(void) const {
  return (m_CurrentConfiguration->GetObjectFilters().GetSize() != 0);
}

inline bool CSite::GetLowerCase(void) const {
  return m_CurrentConfiguration->GetLowerCase();
}

inline bool CSite::GetUpperCase(void) const {
  return m_CurrentConfiguration->GetUpperCase();
}

inline bool CSite::GetForceExpire(void) const {
  if (m_ForceExpire) 
    return true; 
  return m_CurrentConfiguration->GetExpire();
}
    
inline int CSite::GetHeaderLength(void) const { 
  return m_CurrentConfiguration->GetHeaderLength();
}

inline void CSite::CopyReplaceTable(CStringTable& StringTable) const {  
  m_CurrentConfiguration->CopyReplaceTable(StringTable);   
}

inline void CSite::CopyRegexpReplaceTable(CStringTable& StringTable) const {  
  m_CurrentConfiguration->CopyRegexpReplaceTable(StringTable);   
}

inline int CSite::GetSleepRoundtrip(void) const { 
  if (m_ForceSleepRoundtrip != -1)
    return m_ForceSleepRoundtrip; 
  return m_CurrentConfiguration->GetSleepRoundtrip();
}

inline int CSite::GetSleepFile(void) const { 
  if (m_ForceSleepFile != -1) 
    return m_ForceSleepFile; 
  return m_CurrentConfiguration->GetSleepFile();
}

inline bool CSite::GetCanRobots(void) const {
  return m_CurrentConfiguration->GetCanRobots();
}

inline bool CSite::GetCanCookies(void) const {
  return m_CurrentConfiguration->GetCanCookies();
}
    
inline bool CSite::GetCanMd5(void) const { 
  return m_CurrentConfiguration->GetCanMd5();
}
    
inline bool CSite::GetCanMetaDescription(void) const { 
  return m_CurrentConfiguration->GetCanMetaDescription();
}

inline bool CSite::GetCanTextDescription(void) const { 
  return m_CurrentConfiguration->GetCanTextDescription();
}

inline bool CSite::GetSkipMeta(void) const { 
  return m_CurrentConfiguration->GetSkipMeta();
}

inline bool CSite::GetSkipText(void) const {
  return m_CurrentConfiguration->GetSkipText();
}

inline bool CSite::GetSkipLinks(void) const { 
  return m_CurrentConfiguration->GetSkipLinks();
}

inline bool CSite::GetNewOnly(void) const {
  if (m_NewOnly) 
    return true;
  return m_CurrentConfiguration->GetNewOnly();
}

inline bool CSite::GetParseMetas(CVector<CString> * pParseMetas) const {
  * pParseMetas = m_CurrentConfiguration->GetParseMetas(); 
  return (pParseMetas->GetSize() > 0);
}
    
inline bool CSite::GetCustomMetas(CVector<CString> * pCustomMetas) const {
  * pCustomMetas = m_CurrentConfiguration->GetCustomMetas(); 
  return (pCustomMetas->GetSize() > 0);
}

inline bool CSite::ReplaceLocalUrl(CString& Url, CString * pReplaceUrl, CString * pResult) const {
  return m_CurrentConfiguration->ReplaceLocalUrl(Url, pReplaceUrl, pResult);
}

inline bool CSite::GetReplaceLocalSize(void) const {
  return (m_CurrentConfiguration->GetReplaceLocalTable().GetSize() > 0);
}

inline const CVector<CStringPair>& CSite::GetAuth(void) const {
  return m_CurrentConfiguration->GetAuth();
}   

#endif
