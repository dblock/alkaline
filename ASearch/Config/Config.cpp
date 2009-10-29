/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  _____________________________________________________

  written by Daniel Doubrovkine - dblock@vestris.com

*/

#include <alkaline.hpp>

#include "Config.hpp"
#include <RegExp/RegExp.hpp>
#include <Main/bldver.hpp>
#include <File/LocalFile.hpp>
#include <Socket/Socket.hpp>
#include <Internet/Url.hpp>

CConfig::CConfig(const CConfig& Other) : CConfigBase(Other) {
    CreateConfigurationOptions();
    Copy(Other);
}

CConfig::CConfig(void) : CConfigBase() {
    m_HeaderLength = 256;
    m_SleepFile = 1;
    m_SleepRoundtrip = 60;
    m_CanRemote = false;
    m_IncludePagesAll = false;
    m_ExcludePagesAll = false;
    m_CanUpper = false;
    m_MaxFiles = -1;
    m_MaxLinks = -1;
    m_WriteIndex = 100;
    m_ExactSize = 1;
    m_CanRobots = true;
    m_CanCookies = true;
    m_CanMd5 = true;
    m_CanMetaDescription = true;
    m_CanTextDescription = true;
    m_CanReindex = true;
    m_RetryCount = 3;
    m_SizeLimit = 1000000;
    m_Depth = -1;
    m_RemoteDepth = -1;
    m_SiteDepth = -1;
    m_Exts = "html,htm,txt,shtml";
    m_FreeAlpha = false;
    m_CGI = false;
    m_SkipParseLinks = false;
    m_NSF = false;
    m_EmptyLinks = true;
    m_Regexp = false;
    m_Insens = false;
    m_SkipMeta = false;
    m_SkipText = false;
    m_SkipLinks = false;
    m_Timeout = 30;
    m_SearchCacheLife = 60;
    m_DnsTimeout = 30;
    m_Expire = false;
    m_LowerCase = false;
    m_UpperCase = false;
    m_NewOnly = false;
    m_SearchPartialLeft = false;
    m_MaxWordSize = 32;
    CreateConfigurationOptions();
}

CConfig::~CConfig(void) {    

}

CConfig& CConfig::Copy(const CConfig& Other) {
    
    if (&Other == this) 
        return * this;
    
    m_HeaderLength = Other.m_HeaderLength;
    m_SleepFile = Other.m_SleepFile;
    m_SleepRoundtrip = Other.m_SleepRoundtrip;
    m_CanRemote = Other.m_CanRemote;
    m_CanUpper = Other.m_CanUpper;
    m_MaxFiles = Other.m_MaxFiles;
    m_MaxLinks = Other.m_MaxLinks;
    m_WriteIndex = Other.m_WriteIndex;
    m_ExactSize = Other.m_ExactSize;
    m_CanRobots = Other.m_CanRobots;
    m_CanCookies = Other.m_CanCookies;
    m_CanMd5 = Other.m_CanMd5;
    m_CanMetaDescription = Other.m_CanMetaDescription;
    m_CanTextDescription = Other.m_CanTextDescription;
    m_IncludePagesAll = Other.m_IncludePagesAll;
    m_ExcludePagesAll = Other.m_ExcludePagesAll;
    m_CanReindex = Other.m_CanReindex;
    m_IndexPaths = Other.m_IndexPaths;
    m_UrlIndexPaths = Other.m_UrlIndexPaths;
    m_UrlSkipPaths = Other.m_UrlSkipPaths;
    m_IncludePaths = Other.m_IncludePaths;
    m_ExcludePaths = Other.m_ExcludePaths;
    m_RetryCount = Other.m_RetryCount;
    m_SizeLimit = Other.m_SizeLimit;
    m_IndexHTML = Other.m_IndexHTML;
    m_Proxy = Other.m_Proxy;
    m_Exts = Other.m_Exts;
    m_RequestHeadersTable = Other.m_RequestHeadersTable;
    m_AddExts = Other.m_AddExts;
    m_ExtsAdd = Other.m_ExtsAdd;
    m_Depth = Other.m_Depth;
    m_SiteDepth = Other.m_SiteDepth;
    m_RemoteDepth = Other.m_RemoteDepth;
    m_FreeAlpha = Other.m_FreeAlpha;
    m_CGI = Other.m_CGI;
    m_SkipParseLinks = Other.m_SkipParseLinks;
    m_NSF = Other.m_NSF;
    m_EmptyLinks = Other.m_EmptyLinks;
    m_Regexp = Other.m_Regexp; 
    m_Insens = Other.m_Insens;
    m_Auth = Other.m_Auth;
    m_SkipMeta = Other.m_SkipMeta;
    m_SkipText = Other.m_SkipText;
    m_SkipLinks = Other.m_SkipLinks;
    m_RedirectTable = Other.m_RedirectTable;
    m_ReplaceLocalTable = Other.m_ReplaceLocalTable;
    m_ReplaceTable = Other.m_ReplaceTable;
    m_UrlReplaceTable = Other.m_UrlReplaceTable;
    m_Timeout = Other.m_Timeout;
    m_SearchCacheLife = Other.m_SearchCacheLife;
    m_DnsTimeout = Other.m_DnsTimeout;
    m_Expire = Other.m_Expire;
    m_LowerCase = Other.m_LowerCase;
    m_UpperCase = Other.m_UpperCase;
    m_NewOnly = Other.m_NewOnly;        
    m_WordsIndexTable = Other.m_WordsIndexTable;
    m_WordsExcludeTable = Other.m_WordsExcludeTable;    
    m_PagesIncludeTable = Other.m_PagesIncludeTable;    
    m_PagesExcludeTable = Other.m_PagesExcludeTable;
    m_ObjectFilters = Other.m_ObjectFilters;
    m_Cookies = Other.m_Cookies;
    m_CustomMetas = Other.m_CustomMetas;
    m_SearchPartialLeft = Other.m_SearchPartialLeft;
    m_ParseContent = Other.m_ParseContent;
    m_ParseMetas = Other.m_ParseMetas;
    m_WeakWords = Other.m_WeakWords;
    m_WeightTable = Other.m_WeightTable;
    m_MaxWordSize = Other.m_MaxWordSize;

    m_Regexp_IndexPaths = Other.m_Regexp_IndexPaths;
    m_Regexp_ExcludePaths = Other.m_Regexp_ExcludePaths;
    m_Regexp_IncludePaths = Other.m_Regexp_IncludePaths;
    m_Regexp_UrlIndexPaths = Other.m_Regexp_UrlIndexPaths;
    m_Regexp_UrlSkipPaths = Other.m_Regexp_UrlSkipPaths;
    
    m_Regexp_ReplaceLocalTable = Other.m_Regexp_ReplaceLocalTable;
    m_Regexp_UrlReplaceTable = Other.m_Regexp_UrlReplaceTable;
    m_Regexp_ReplaceTable = Other.m_Regexp_ReplaceTable;

    m_Regexp_WordsIndexTable = Other.m_Regexp_WordsIndexTable;
    m_Regexp_WordsExcludeTable = Other.m_Regexp_WordsExcludeTable;    
    m_Regexp_PagesIncludeTable = Other.m_Regexp_PagesIncludeTable;    
    m_Regexp_PagesExcludeTable = Other.m_Regexp_PagesExcludeTable;
    
    CConfigBase :: Copy(Other);
    return (* this);    
}

void CConfig::AddOption(const CString& Line, bool Verbose, bool bRegExp) {  
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

	CString Name;
    
    static const CString OptionAuth("Auth ");
    static const CString OptionRegexp("Regexp ");
    static const CString OptionReplace("Replace ");
    static const CString OptionReplaceLocal("ReplaceLocal ");
    static const CString OptionUrlReplace("UrlReplace ");
    static const CString OptionRedirect("Redirect ");
    static const CString OptionFilter("Filter");
    static const CString OptionObject("Object");
    static const CString OptionCookie("Cookie ");
    static const CString OptionCustomMetas("CustomMetas");
    static const CString OptionHttp("http://");
    static const CString OptionRequestHeader("RequestHeader ");
    static const CString OptionWeight("Weight ");

    if (Values[0].StartsWithSame(OptionAuth)) {
		Values[0].Mid(OptionAuth.GetLength(), Values[0].GetLength(), & Name);        
        if (Name.GetLength()) {
            CStringPair AuthPair(Name, Values[1]);
            m_Auth += AuthPair;
            if (Verbose) cout << "  [Auth:" << AuthPair.GetName() << "=" << AuthPair.GetValue() << "]" << endl;
            return;
        } else {
            cout << "  [missing name in auth: " << Values[1] << "]" << endl;
            return;
        }        
    } else if (Values[0].StartsWithSame(OptionRegexp)) {
        
		Values[0].Mid(OptionRegexp.GetLength(), Values[0].GetLength(), & Name);		
          
        if (Name.GetLength()) {
            // add the same option with a regular expression marker
            AddOption(Name + '=' + Values[1], Verbose, true);
            return;
        } else if (Verbose) {
            cout << "  [" << Name << "=" << Values[1] << "]" << endl;
        }
        
    } else if (Values[0].StartsWithSame(OptionRequestHeader)) {

		Values[0].Mid(OptionRequestHeader.GetLength(), Values[0].GetLength(), & Name);
        
        if (Verbose) cout << "  [" << OptionRequestHeader << Name << "=" << Values[1] << "]" << endl;
        m_RequestHeadersTable.Add(Name, Values[1]);
    } else if (Values[0].StartsWithSame(OptionWeight)) {
		
		Values[0].Mid(OptionWeight.GetLength(), Values[0].GetLength(), & Name);
        
        if (Verbose) cout << "  [" << OptionWeight << Name << "=" << Values[1] << "]" << endl;
        m_WeightTable.Add(Name, Values[1]);
    } else if (Values[0].StartsWithSame(OptionReplaceLocal)) {
        
		Values[0].Mid(OptionReplaceLocal.GetLength(), Values[0].GetLength(), & Name);
		
        if (Verbose) cout << "  [" << OptionReplaceLocal << Name << "=" << Values[1] << "]" << (bRegExp?"(regexp)":"(plain)") << endl;
        if (bRegExp) {
            m_Regexp_ReplaceLocalTable.Add(Name, Values[1]);
        } else {
            m_ReplaceLocalTable.Add(Name, Values[1]);
        }
    } else if (Values[0].StartsWithSame(OptionReplace)) {
        
		Values[0].Mid(OptionReplace.GetLength(), Values[0].GetLength(), & Name);
		
        if (Verbose) cout << "  [" << OptionReplace << Name << "=" << Values[1] << "]" << (bRegExp?"(regexp)":"(plain)") << endl;
        if (bRegExp) {
            m_Regexp_ReplaceTable.Add(Name, Values[1]);
        } else {
            m_ReplaceTable.Add(Name, Values[1]);
        }
    } else if (Values[0].StartsWithSame(OptionUrlReplace)) {
        
		Values[0].Mid(OptionUrlReplace.GetLength(), Values[0].GetLength(), & Name);
		
        if (Verbose) cout << "  [" << OptionUrlReplace << Name << "=" << Values[1] << "]" << (bRegExp?"(regexp)":"(plain)") << endl;
        if (bRegExp) {
            m_Regexp_UrlReplaceTable.Add(Name, Values[1]);
        } else {
            m_UrlReplaceTable.Add(Name, Values[1]);
        }
    } else if (Values[0].StartsWithSame(OptionRedirect)) {
        
		Values[0].Mid(OptionRedirect.GetLength(), Values[0].GetLength(), & Name);
		
        if (Verbose) cout << "  [" << OptionRedirect << Name << "=" << Values[1] << "]" << (bRegExp?"(regexp)":"(plain)") << endl;
        m_RedirectTable.Add(Name, Values[1]);
    } else if (Values[0].StartsWithSame(OptionFilter)) {
        CString MidValue;
        Values[0].Mid(OptionFilter.GetLength(), Values[0].GetLength(), &MidValue);
        MidValue.Trim();
        if (Verbose) {
            if (MidValue.GetLength())
                cout << "  [" << OptionFilter << "{" << MidValue << "}]" << endl;
            else cout << "  [" << OptionFilter << "{*}]" << endl;
        }
        // hack: Filterhtml is the same as Filter html (legacy)
        if (MidValue.GetLength()) {
            Values[0] = OptionFilter + ' ' + MidValue;                        
        } else Values[0] = OptionFilter;
    } else if (Values[0].StartsWithSame(OptionObject)) {
        CString FilterClass; 
        Values[0].Mid(OptionObject.GetLength(), Values[0].GetLength(), &FilterClass);
        FilterClass.Trim();
        if (Verbose) cout << "  [" << OptionObject << " {" << FilterClass << "}]" << endl;
        m_ObjectFilters.Set(FilterClass, Values[1]);
        return;
    } else if (Values[0].StartsWithSame(OptionCookie)) {
        CString MidValue;                       
        Line.Mid(OptionCookie.GetLength(), Line.GetLength(), &MidValue);
        CString Cookie = "Cookie: ";
        Cookie += MidValue;
        if (Verbose) 
            cout << "  [" << OptionCookie << " {" << MidValue << "}]" << endl;
        m_Cookies += Cookie;
        return;
    } else if (Values[0].Same(OptionCustomMetas)) {
        CString::StrToVector(Values[1], ',', &m_CustomMetas);
        if (Verbose) {
            if (Verbose) cout << "  [" << OptionCustomMetas << "{";
            for (register int i=0;i<(int)m_CustomMetas.GetSize();i++)
                cout << "{" << m_CustomMetas[i] << "}";
            cout << "]" << endl;
        }
    } else if (bRegExp) {
        Set("RegExp " + Values[0], Values[1]);
        cout << "  [RegExp " << Values[0] << "=" << Values[1] << "]" << endl;
        return;
    } else if (Verbose) {
        cout << "  [" << Values[0] << "=" << Values[1] << "]" << endl;
    }
    
    Set(Values[0], Values[1]);
}


void CConfig::CreateConfigurationOptions(void) {
    
    //
    //        ccoDigit: a number [reset value] [ignored]
    //                  if the number read is invalid, or if it's smaller than the reset value, 
    //                  it will be defaulted to the reset value
    //   ccoDigitBound: a value with bounds [lower limit] [default]
    //                  reset to default if (lower limit != default) and (value < lower limit)
    //     ccoDigitPos: positive number [default] [ignored]
    // ccoTranslateSet: a size in bytes, kbytes or megabytes [ignored] [ignored]
    //         ccoBool: a boolean value (Y, N, true, false) [ignored] [ignored]
    // ccoBoolInverted: inverted result, for example Robots option setting a NoRobots value [ignored] [ignored]
    //      ccoVirtual: caller's non-built-in type, in CGlobalCnf this looks up a value in equivs [caller] [caller]
    //       ccoString: a string value [ignored] [ignored]
    //    ccoStringPos: non-empty string [ignored] [ignored]
    //        ccoArray: a comma separated array [ignored] [ignored]
    //     ccoEvalPair: a non-parsed option, just for information [ignored] [ignored]
    // 

    //
    // look in CConfigBase for detailed evaluation
    //

    CConfigOption LocalConfigurationOptions[] = 
    {
            { "WriteIndex",         ccoDigit,        & m_WriteIndex,            -1,    0,     "database write interval" },
            { "Retry",              ccoDigitBound,   & m_RetryCount,             1,    3,     "retry count for a timed-out connection" },
            { "Timeout",            ccoDigitBound,   & m_Timeout,                1,    30,    "network timeout period" },
            { "DnsTimeout",         ccoDigitBound,   & m_DnsTimeout,             1,    30,    "dns query timeout period" },
            { "MaxSize",            ccoTranslateSet, & m_SizeLimit,             -1,    0,     "maximum document size" },
            { "ExactSize",          ccoDigitPos,     & m_ExactSize,              1,    0,     "exact search word length" },
            { "Depth",              ccoDigit,        & m_Depth,                 -1,    0,     "relative url indexing depth" },
            { "SiteDepth",          ccoDigit,        & m_SiteDepth,             -1,    0,     "maximum paths depth of urls to follow" },
            { "RemoteDepth",        ccoDigit,        & m_RemoteDepth,           -1,    0,     "maximum remote depth of urls to follow" },
            { "MaxFiles",           ccoDigit,        & m_MaxFiles,              -1,    0,     "maximum number of files to index" },
            { "MaxLinks",           ccoDigit,        & m_MaxLinks,              -1,    0,     "maximum number of links to follow" },
            { "Expire",             ccoBool,         & m_Expire,                 0,    0,     "treat all documents as out of date" },
            { "LowerCase",          ccoBool,         & m_LowerCase,              0,    0,     "convert to lowercase" },
            { "UpperCase",          ccoBool,         & m_UpperCase,              0,    0,     "convert to uppercase" },
            { "NewOnly",            ccoBool,         & m_NewOnly,                0,    0,     "index only new documents" },
            { "SkipMeta",           ccoBool,         & m_SkipMeta,               0,    0,     "do not index meta tags" },
            { "SkipText",           ccoBool,         & m_SkipText,               0,    0,     "do not index plain text" },
            { "SkipLinks",          ccoBool,         & m_SkipLinks,              0,    0,     "do not follow links" },
            { "Remote",             ccoBool,         & m_CanRemote,              0,    0,     "index or exclude urls from a remote site" },
            { "Upper",              ccoBool,         & m_CanUpper,               0,    0,     "follow or ignore parent server paths" },
            { "Reindex",            ccoBool,         & m_CanReindex,             0,    0,     "enable background indexing" },
            { "Robots",             ccoBool,         & m_CanRobots,              0,    0,     "respect server robots directives" },
            { "Cookies",            ccoBool,         & m_CanCookies,             0,    0,     "send, receive and store cookies" },
            { "Md5",                ccoBool,         & m_CanMd5,                 0,    0,     "enable the MD5 document matching mechanism" },
            { "MetaDescription",    ccoBool,         & m_CanMetaDescription,     0,    0,     "store meta descriptions" },
            { "TextDescription",    ccoBool,         & m_CanTextDescription,     0,    0,     "store plain text descriptions" },
            { "IncludePagesAll",    ccoBool,         & m_IncludePagesAll,        0,    0,     "define the behavior of IncludePages" },
            { "ExcludePagesAll",    ccoBool,         & m_ExcludePagesAll,        0,    0,     "define the behavior of ExcludePages" },
            { "Index.html",         ccoString,       & m_IndexHTML,              0,    0,     "default for urls without a filename" },
            { "HeaderLength",       ccoDigitPos,     & m_HeaderLength,           256,  0,     "maximum $header length" },
            { "FreeCharset",        ccoBool,         & m_FreeAlpha,              0,    0,     "disable character decoding" },
            { "Cgi",                ccoBool,         & m_CGI,                    0,    0,     "schedule cgi urls for indexing" },
            { "Nsf",                ccoBool,         & m_NSF,                    0,    0,     "Lotus Notes Domino support" },
            { "SkipParseLinks",     ccoBool,         & m_SkipParseLinks,         0,    0,     "index links in skip sections" },
            { "EmptyLinks",         ccoBool,         & m_EmptyLinks,             0,    0,     "process and queue invisible links" },
            { "Regexp",             ccoBool,         & m_Regexp,                 0,    0,     "enable regular expressions" },
            { "Insens",             ccoBool,         & m_Insens,                 0,    0,     "case-insensitive url parsing and MD5" },
            { "Proxy",              ccoString,       & m_Proxy,                  0,    0,     "define an HTTP proxy to use for url retrieval" },
            { "Exts",               ccoStringPos,    & m_Exts,                   0,    0,     "valid extensions for files to spider" },
            { "AddExts",            ccoString,       & m_AddExts,                0,    0,     "add extensions to the Exts directive" },
            { "ExtsAdd",            ccoString,       & m_ExtsAdd,                0,    0,     "add extensions to the Exts directive" },
            { "SleepFile",          ccoDigit,        & m_SleepFile,              0,    0,     "lazy mode delay between files" },
            { "SleepRoundtrip",     ccoDigit,        & m_SleepRoundtrip,         0,    0,     "lazy mode delay between roundtrips" },
            { "UrlExclude",         ccoArray,        & m_ExcludePaths,           0,    0,     "exclude urls from indexing" },
            { "Regexp UrlExclude",  ccoArray,        & m_Regexp_ExcludePaths,    0,    0,     "exclude urls from indexing" },
            { "UrlInclude",         ccoArray,        & m_IncludePaths,           0,    0,     "define a global url include scope" },
            { "Regexp UrlInclude",  ccoArray,        & m_Regexp_IncludePaths,    0,    0,     "define a global url include scope" },
            { "UrlIndex",           ccoArray,        & m_UrlIndexPaths,          0,    0,     "follow links and index" },
            { "Regexp UrlIndex",    ccoArray,        & m_Regexp_UrlIndexPaths,   0,    0,     "follow links and index" },
            { "UrlSkip",            ccoArray,        & m_UrlSkipPaths,           0,    0,     "follow links, but do not index" },
            { "Regexp UrlSkip",     ccoArray,        & m_Regexp_UrlSkipPaths,    0,    0,     "follow links, but do not index" },
            { "SearchPartialLeft",  ccoBool,         & m_SearchPartialLeft,      0,    0,     "constraint search to left/right wildcards" },
            { "ParseContent",       ccoArray,        & m_ParseContent,           0,    0,     "define html content-types" },
            { "Auth",               ccoEvalPair,     0,                          0,    0,     "supply basic or ntlm (NT only) authentication credentials" },
            { "Replace",            ccoEvalPair,     0,                          0,    0,     "absolute url string replacement" },
            { "Regexp Replace",     ccoEvalPair,     0,                          0,    0,     "absolute url string replacement" },
            { "ReplaceLocal",       ccoEvalPair,     0,                          0,    0,     "local url string replacement" },
            { "Regexp ReplaceLocal",ccoEvalPair,     0,                          0,    0,     "local url string replacement" },
            { "UrlReplace",         ccoEvalPair,     0,                          0,    0,     "indexed urls text replacements" },
            { "Regexp UrlReplace",  ccoEvalPair,     0,                          0,    0,     "indexed urls text replacements" },
            { "Redirect",           ccoEvalPair,     0,                          0,    0,     "define equivalent or redirected urls" },
            { "Filter",             ccoEvalPair,     0,                          0,    0,     "define a document type filter" },
            { "Object",             ccoEvalPair,     0,                          0,    0,     "define an embedded objects filter" },
            { "Cookie",             ccoEvalPair,     0,                          0,    0,     "set a global cookie to be set to servers" },
            { "CustomMetas",        ccoEvalPair,     0,                          0,    0,     "define custom meta tags for search results" },
            { "ParseMetas",         ccoArray,        & m_ParseMetas,             0,    0,     "define meta tags to parse" },
            { "RequestHeader",      ccoEvalPair,     0,                          0,    0,     "define an http request header" } ,  
            { "MaxWordSize",        ccoDigitBound,   & m_MaxWordSize,            1,  255,     "define the maximum size of indexed words" },
            { "WeakWords",          ccoArray,        & m_WeakWords,              0,    0,     "define weak words in rankings" },
            { "Weight",             ccoEvalPair,     0,                          0,    0,     "define a weight value in rankings" },
            { "SearchCacheLife",    ccoDigitPos,     & m_SearchCacheLife,       60,    0,     "search cache record lifetime" },
            // depricated            
            { "NoReindex",          ccoBoolInverted, & m_CanReindex,             0,    0,     "disable background indexing" },
            { "NoRobots",           ccoBoolInverted, & m_CanRobots,              0,    0,     "ignore server robots directives" },
            { "NoCookies",          ccoBoolInverted, & m_CanCookies,             0,    0,     "disable sending and receiving cookies" },
            { "NoMd5",              ccoBoolInverted, & m_CanMd5,                 0,    0,     "disable the MD5 mechanism" },
            { "NoMetaDescription",  ccoBoolInverted, & m_CanMetaDescription,     0,    0,     "store plain text descriptions" },
            { "NoTextDescription",  ccoBoolInverted, & m_CanTextDescription,     0,    0,     "do not store plain text descriptions" },
            { "NoEmptyLinks",       ccoBoolInverted, & m_EmptyLinks,             0,    0,     "skip invisible links" },            
    };

    CConfigBase :: CreateConfigurationOptions(
        LocalConfigurationOptions, 
        (int) (sizeof(LocalConfigurationOptions) / sizeof(CConfigOption)));
}
    
void CConfig::Finalize(void) {
    
    CConfigBase :: Finalize();
    
    // user-agent must be present
    
    CString AlkalineUserAgent;
    if (!m_RequestHeadersTable.FindAndCopy(g_strHttpUserAgent, AlkalineUserAgent) || 
        !AlkalineUserAgent.GetLength()) {
        AlkalineUserAgent =
            "AlkalineBOT/" + 
            CString::IntToStr(RC_VERSION_MAJOR) + "." + 
            CString::IntToStr(RC_VERSION_MINOR) + " (" + CString(RC_VERSION_STRING) + ")";        
        m_RequestHeadersTable.Set(g_strHttpUserAgent, AlkalineUserAgent);
    }

    CVector<CString> FVector;
    
    // UrlExcludeFile
    CString::StrToVector(GetOption("UrlExcludeFile"),',',&FVector);
    AddFromFilesToVector("UrlExcludeFile", m_ExcludePaths, FVector);
    
    CString::StrToVector(GetOption("Regexp UrlExcludeFile"),',',&FVector);
    AddFromFilesToVector("Regexp UrlExcludeFile", m_Regexp_ExcludePaths, FVector);
    
    // UrlIncludeFile
    CString::StrToVector(GetOption("UrlIncludeFile"),',',&FVector);
    AddFromFilesToVector("UrlIncludeFile", m_IncludePaths, FVector);
    
    CString::StrToVector(GetOption("Regexp UrlIncludeFile"),',',&FVector);
    AddFromFilesToVector("Regexp UrlIncludeFile", m_Regexp_IncludePaths, FVector);

    // UrlIndexFile
    CString::StrToVector(GetOption("UrlIndexFile"),',',&FVector);
    AddFromFilesToVector("UrlIndexFile", m_IndexPaths, FVector);
    
    CString::StrToVector(GetOption("Regexp UrlIndexFile"),',',&FVector);
    AddFromFilesToVector("Regexp UrlIndexFile", m_Regexp_IndexPaths, FVector);
        
    // UrlSkipFile
    CString::StrToVector(GetOption("UrlSkipFile"),',',&FVector);
    AddFromFilesToVector("UrlSkipFile", m_UrlSkipPaths, FVector);
    
    CString::StrToVector(GetOption("Regexp UrlSkipFile"),',',&FVector);
    AddFromFilesToVector("Regexp UrlSkipFile", m_Regexp_UrlSkipPaths, FVector);
        
    // if global regexp is enabled, move data from tables
    if (m_Regexp) {

        m_Regexp_IndexPaths.Add(m_IndexPaths);                 m_IndexPaths.RemoveAll();        
        m_Regexp_ExcludePaths.Add(m_ExcludePaths);             m_ExcludePaths.RemoveAll();    
        m_Regexp_IncludePaths.Add(m_IncludePaths);             m_IncludePaths.RemoveAll();    
        m_Regexp_UrlIndexPaths.Add(m_UrlIndexPaths);           m_UrlIndexPaths.RemoveAll();    
        m_Regexp_UrlSkipPaths.Add(m_UrlSkipPaths);             m_UrlSkipPaths.RemoveAll();   
        m_Regexp_ReplaceLocalTable.Add(m_ReplaceLocalTable);   m_ReplaceLocalTable.RemoveAll();    
        m_Regexp_UrlReplaceTable.Add(m_UrlReplaceTable);       m_UrlReplaceTable.RemoveAll();    
        m_Regexp_ReplaceTable.Add(m_ReplaceTable);             m_ReplaceTable.RemoveAll();        
    }

    cout << "  [RegExp][enabled for " << 
        m_Regexp_IndexPaths.GetSize() + 
        m_Regexp_ExcludePaths.GetSize() +
        m_Regexp_IncludePaths.GetSize() +
        m_Regexp_UrlIndexPaths.GetSize() +
        m_Regexp_UrlSkipPaths.GetSize() +
        m_Regexp_ReplaceLocalTable.GetSize() +
        m_Regexp_UrlReplaceTable.GetSize() +
        m_Regexp_ReplaceTable.GetSize()
        << " element(s)]" << endl;
    
    // adjust DNS timeout
    CSocket :: m_pDNService->SetDnsTimeout(m_DnsTimeout);
}

bool CConfig::ReplaceLocalUrl(const CString& Url, CString * pResultUrl, CString * pResult) const {
    
    * pResultUrl = CUrl::UnEscape(Url);

    CString UrlCopy(* pResultUrl);
    
    int i;
    
    for (i=0;i<(int) m_ReplaceLocalTable.GetSize();i++) {
        if (pResult) { 
            * pResult += "\n  ReplaceLocal {";
            * pResult += * pResultUrl;
            * pResult += "} => {";
        }
        
        pResultUrl->Replace(m_ReplaceLocalTable.GetNameAt(i), m_ReplaceLocalTable.GetValueAt(i));
        
        if (pResult) {
            * pResult += * pResultUrl;
            * pResult += "}";
        }
    }
    
    for (i=0;i<(int) m_Regexp_ReplaceLocalTable.GetSize();i++) {
        if (pResult) { 
            * pResult += "\n  Regexp ReplaceLocal {";
            * pResult += * pResultUrl;
            * pResult += "} => {";
        }
        
        CString RegexpError;
        
        * pResultUrl = CRegExp::SearchReplace(
            * pResultUrl, 
            m_Regexp_ReplaceLocalTable.GetNameAt(i), 
            m_Regexp_ReplaceLocalTable.GetValueAt(i), 
            pResult?(CString *)&RegexpError:(CString *)NULL);
        
        if (pResult && RegexpError.GetLength()) {
            * pResult += "{[regexp error: ";
            * pResult += RegexpError;
            * pResult += "]";
        }
        
        if (pResult) {
            * pResult += * pResultUrl;
            * pResult += "}";
        }
    }
    
    return ((* pResultUrl) != UrlCopy);
}

bool CConfig::ReplaceUrl(CString& Url, CString * pResult) const {  
    
    int i;
    
    for (i=0;i<(int) m_UrlReplaceTable.GetSize();i++) {
        if (pResult) { 
            * pResult += "  UrlReplace {";
            * pResult += Url;
            * pResult += "} => {";
        }
        
        Url.Replace(m_UrlReplaceTable.GetNameAt(i), m_UrlReplaceTable.GetValueAt(i));
        
        if (pResult) {
            * pResult += Url;
            * pResult += "}\n";
        }     
    }
    
    for (i=0;i<(int) m_Regexp_UrlReplaceTable.GetSize();i++) {
        if (pResult) { 
            * pResult += "  Regexp UrlReplace {";
            * pResult += Url;
            * pResult += "} => {";
        }

        CString RegexpError;
        Url = CRegExp::SearchReplace(
            Url, 
            m_Regexp_UrlReplaceTable.GetNameAt(i), 
            m_Regexp_UrlReplaceTable.GetValueAt(i), 
            pResult?(CString *)&RegexpError:(CString *)NULL);
        
        if (pResult && RegexpError.GetLength()) {
            * pResult += "{[regexp error: ";
            * pResult += RegexpError;
            * pResult += "]\n";
        }

        if (pResult) {
            * pResult += Url;
            * pResult += "}\n";
        }     
    }
    
    return true;
}

bool CConfig::RedirectUrl(CString& Url, CString * pResult) const {
    
    for (int i=0;i<(int) m_RedirectTable.GetSize();i++) {
        if (Url.StartsWith(m_RedirectTable.GetNameAt(i))) {
            if (pResult) { 
                * pResult += "  Redirect {";
                * pResult += Url;
                * pResult += "} => {";
            }
            Url.Delete(0, m_RedirectTable.GetNameAt(i).GetLength());
            Url.Insert(0, m_RedirectTable.GetValueAt(i));
            if (pResult) {
                * pResult += Url;
                * pResult += "}\n";
            }
            return true;
        }
    }
    
    return false;
}

void CConfig::AddFromFilesToVector(const CString& iStr, CVector<CString>& Vector, const CVector<CString>& FileNames) {
    
    CString Contents; 
    CString Line;
    for (int i=0;i<(int) FileNames.GetSize();i++) {
        if (! FileNames[i].GetLength())
            continue;

        cout << "  [" << iStr << "][loading " << FileNames[i] << "]"; cout.flush();
        CLocalFile FileHandle(FileNames[i]);
        if (! FileHandle.OpenReadBinary()) {
            cout << "[error opening file]" << endl;
            return;
        }
        
        if (FileHandle.GetSize() <= 0) {
            cout << "[empty file]" << endl;
            return;
        }
        
        cout << "[" << FileHandle.GetSize() << " bytes]"; cout.flush();
        FileHandle.Read(&Contents);
        Vector.SetDim(Contents.GetCount((char) 10));
        int Pos = 0;
        while (Pos < (int) Contents.GetLength()) {
            Contents.GetLine(&Line, Pos);
            Line.Trim32();
            if (Line.GetLength())
                Vector += Line;
        }
        
        cout << "[" << Vector.GetSize() << " element" << ((Vector.GetSize() != 1)?"s]":"]") << endl;
        
    }
}
