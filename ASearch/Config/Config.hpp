/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  ______________________________________________

  written by Daniel Doubrovkine - dblock@vestris.com

*/

#ifndef ALKALINE_C_CONFIGURATION_HPP
#define ALKALINE_C_CONFIGURATION_HPP

#include <platform/include.hpp>
#include <Config/ConfigBase.hpp>

class CConfig : public CConfigBase {	
    property(bool, LowerCase);    
    property(bool, UpperCase);    
    property(bool, Expire);
    property(bool, NewOnly);
    property(bool, SkipLinks);
    property(bool, SkipMeta);
    property(bool, SkipText);
    property(int, Timeout);
    property(int, DnsTimeout);
    property(int, SearchCacheLife);
    property(int, HeaderLength);
    property(bool, FreeAlpha);
    property(bool, CGI);
    property(bool, SkipParseLinks);
    property(bool, NSF);
    property(bool, EmptyLinks);
    property(bool, Regexp);
    property(bool, Insens);
    property(int, Depth);
    property(int, RemoteDepth);
    property(int, SiteDepth);
    property(int, SleepFile);
    property(int, SleepRoundtrip);
    property(bool, CanRemote);
    property(bool, CanUpper);
    property(int, MaxFiles);
    property(int, MaxLinks);
    property(int, WriteIndex);
    property(int, ExactSize);
    property(bool, CanRobots);
    property(bool, CanReindex);
    property(bool, CanMd5);
    property(bool, CanMetaDescription);
    property(bool, CanTextDescription);
    property(bool, CanCookies);
    
    property(CVector<CString>, IndexPaths);
    property(CVector<CString>, ExcludePaths);
    property(CVector<CString>, IncludePaths);
    property(CVector<CString>, UrlIndexPaths);
    property(CVector<CString>, UrlSkipPaths);

    property(CVector<CString>, Regexp_IndexPaths);
    property(CVector<CString>, Regexp_ExcludePaths);
    property(CVector<CString>, Regexp_IncludePaths);
    property(CVector<CString>, Regexp_UrlIndexPaths);
    property(CVector<CString>, Regexp_UrlSkipPaths);
    
	property(int, MaxWordSize);
	property(CVector<CString>, WeakWords);
	property(CStringTable, WeightTable);

    property(int, RetryCount);
    property(int, SizeLimit);
    property(CString, IndexHTML);
    property(CString, Proxy);
    property(CString, Exts);
    property(CString, AddExts);
    property(bool, IncludePagesAll);
    property(bool, ExcludePagesAll);
    property(CString, ExtsAdd);
    
    property(CStringTable, RedirectTable);                // Redirect - equivalent sites
    property(CStringTable, ReplaceLocalTable);            // ReplaceLocal - local path
    property(CStringTable, UrlReplaceTable);              // UrlReplace - all text url replace
    property(CStringTable, ReplaceTable);                 // Replace - in search results
    
    property(CStringTable, Regexp_ReplaceLocalTable);     // ReplaceLocal - local path
    property(CStringTable, Regexp_UrlReplaceTable);       // UrlReplace - all text url replace
    property(CStringTable, Regexp_ReplaceTable);          // Replace - in search results
    
    property(CStringTable, RequestHeadersTable);
    property(CVector<CStringPair>, Auth);
    
    readonly_property(CHashTable<CStringVector>, WordsExcludeTable);
    readonly_property(CHashTable<CStringVector>, WordsIndexTable);
    readonly_property(CHashTable<CStringVector>, PagesIncludeTable);
    readonly_property(CHashTable<CStringVector>, PagesExcludeTable);

    readonly_property(CHashTable<CStringVector>, Regexp_WordsExcludeTable);
    readonly_property(CHashTable<CStringVector>, Regexp_WordsIndexTable);
    readonly_property(CHashTable<CStringVector>, Regexp_PagesIncludeTable);
    readonly_property(CHashTable<CStringVector>, Regexp_PagesExcludeTable);
    
    readonly_property(CHashTable<CString>, ObjectFilters);
    readonly_property(CVector<CString>, Cookies);
    readonly_property(CVector<CString>, CustomMetas);
    property(bool, SearchPartialLeft);
    readonly_property(CVector<CString>, ParseContent);
    readonly_property(CVector<CString>, ParseMetas);
	readonly_property(CVector<CString>, SearchDomains);
    
    virtual void CreateConfigurationOptions(void);
    
public:	
    
    void Finalize(void);
    bool ReplaceLocalUrl(const CString& Url, CString * pResultUrl, CString * pResult) const;
    bool RedirectUrl(CString& Url, CString * pResult) const;
    bool ReplaceUrl(CString& Url, CString * pResult) const;
    inline void CopyReplaceTable(CStringTable& StringTable) const { StringTable = m_ReplaceTable; }
    inline void CopyRegexpReplaceTable(CStringTable& StringTable) const { StringTable = m_Regexp_ReplaceTable; }
    inline void CopyUrlReplaceTable(CStringTable& StringTable) const { StringTable = m_UrlReplaceTable; }
    inline CString GetLogFile(void) const { return GetValue("LOGFILE"); }
    void AddOption(const CString& Line, bool Verbose = true, bool bRegExp = false);
    
    static void AddFromFilesToVector(const CString& iStr, CVector<CString>&, const CVector<CString>&);

    CConfig(void);
    CConfig(const CConfig&);
    virtual CConfig& Copy(const CConfig& Other);
    virtual ~CConfig(void);
    inline CConfig& operator=(const CConfig& Other) { return Copy(Other); }
};

#endif
