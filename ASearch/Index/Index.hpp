/*
  
  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  ______________________________________________
  
  written by Daniel Doubrovkine - dblock@vestris.com
  
*/

#ifndef ALKALINE_C_INDEX_HPP
#define ALKALINE_C_INDEX_HPP

#include <platform/include.hpp>
#include <Search/Search.hpp>
#include "IndexOptions.hpp"
#include <Internet/CookieStorage.hpp>
#include <Session/Session.hpp>
#include <AlkalineParser/AlkalineParser.hpp>
#include <Tree/XmlTree.hpp>
#include <Search/SearchObject.hpp>
#include "IndexObject.hpp"

#include "LNXManager.hpp"
#include "URLManager.hpp"
#include "INFManager.hpp"

class CSite;

class CIndex : public CObject {
public:
    CIndexOptions     m_IndexOptions;
private:    
    readonly_property(CSearch, Searcher);
    readonly_property(CLNXManager, LNXManager);
    readonly_property(CURLManager, URLManager);
    readonly_property(CINFManager, INFManager);
    property(CCookieStorage, CookieStorage);    
private:
    static bool IsReservedKeyword(const CString& Word);
    inline bool AlreadyIndexed(int UrlIndex) const { return m_Searcher.AlreadyIndexed(UrlIndex); }
    int SetLNXVector(const CString&, const CIntVector&);
    int SetLNXVector(int, const CIntVector&);
    
    protected_property(CVector<CString>, AlreadyEmailVector);
    
    int GetURLIndex(const CString& Url);
public:    
    void IssueRequest(CRemoteFile * RemoteFile, CSite * ParentSite = NULL, CString * pResult = NULL);
    bool IssueLocalRequest(CRemoteFile * RemoteFile, CSite * Site, CString * pResult);
    CString GetURLLink(const int Val) const;
    void GetURLVectorIndex(const CVector<CString>& Vector, CIntVector&);
    void GetURLVectorStrings(int UrlPos, CVector<CString> * pVector) const;
private:
    void IndexAddMetaWords(
        const CString& Extension, const CString& MimeType, 
        const CAlkalineParser& HtmlParser,
        CSite * Parent, int UrlPos, CString * = NULL);  
    void IndexGatherEmail(const CString& Filename, const CVector<CString>& EmailVector);
    void IndexAddWords(
        const CString& Extension, const CString& MimeType, 
        CSite * Parent, CVector<CStringA>& AVector, 
        int UrlPos, CString *);
    void IndexFileAll(
        CRemoteFile& RemoteFile, const CString& FileName, 
        CString& Server, CVector<CString>& Links, bool Lazy, CSite * Parent, CString * = NULL);
    void IndexActiveXObjects(
        CAlkalineParser& HtmlParser, 
        const CString& FileName,
        const CString& NewBase,
        const CString& DataText,
        CSite * Parent, 
        CString * Result);    
    void IndexFileInfVector(
        CVector<CString>& FileIndexVector,
        CAlkalineParser& HtmlParser,
        CRemoteFile& RemoteFile,    
        const CString& Server,
        CString& DataText,
        CSite * Parent,
        CString * Result);    
    bool IndexFileMD5(
        const CVector<CString>& FileIndexVector, const CString& DocumentDirectory, 
        const CString& FileName, CString& URLVectorI);
    int IndexFileSave(
        const CString& FileName, const CVector<CString>& FileIndexVector, 
        const CVector<CString>& Links, CString * = NULL);    
    CString Map(const CString&);
    CString GetFileExtension(const CString& FName) const;
    // load indexes separately
    bool LoadIndexINF(bool bMakeMd5Tree = true);
    bool LoadIndexLNX(void);
    bool LoadIndexURT(void);
    bool LoadIndexNDX(bool bMakeAlreadyIndexed = true);
private:
    bool CheckIndexHTML(const CUrl&);

    CString GetMapTerm(const CString& Source, const CIndexMapV& Parameters) const;
    CString& MapTerm(const CString& Source, CString& Target, const CIndexMapV& Parameters) const;
    CString& MapTermEach(CString& Term, const CIndexMapV& Parameters) const;	
	
	CString Highlight(
		const CString& String, 
		const CVector<CString>& Terms, 
		const CString&, 
		const CString&, 
		const CSearchOptions& SearchOptions) const;
    
    bool CreateDateObjects(CSession& Parent, CIndexMapV& IndexMapStructure, struct tm& fDate, int curDayCount);
    bool CreateDateObject(CSession& Parent, const CString& HashDate, struct tm& fDate, int curDayCount, CIndexMapV& IndexMapStructure, CDate& FSDateObject);
    void PrepareKeywords(CSearchObject& /* SearchObject */) const;
    void PrepareResults(CSearchObject& /* SearchObject */) const;
    bool GetMostRecentDate(int PageUID, time_t& Date) const;
    int CompareResults(CSortType SortType, int First, int Second);
    void SortResults(CSearchObject& /* SearchData */, CSortType SortType, int, int);
    void SortResults(CSearchObject& /* SearchData */, const CString& SortType);
	void SortResultsByDomain(CSearchObject& SearchData);
    void AdjustKeywords(CSearchObject& /* SearchObject */) const;
    void AdjustMetas(CSearchObject& SearchObject) const;
    void IndexProcessErrors(CRemoteFile& RemoteFile, CString& Server, CVector<CString>& Links, int UrlPos, CString * Result, bool = true);
private:
    bool ProcessLocalRequest(
        const CString& LocalName, 
        CRemoteFile * RemoteFile, 
        CString * pResult);
    bool ProcessLocalDirectory(
        const CString& Directory, 
        CRemoteFile * RemoteFile, 
        CString * pResult);
public:
    void SetCookies(const CVector<CString>&);
    inline void SetInsens(bool Value);
    inline void SetExactSize(int Value);
    inline const CString& GetProxy(void) const;
    int FindURLIndex(const CString& Url) const;
    inline int GetWordCount(void) const { return m_Searcher.GetWordCount(); }
    void Write(bool);
    void Remove404(CSite *, bool);
    bool RemovePage(const CString&);
    bool RemovePages(const CString&);
    bool RemovePage(int Index);
    bool RemoveWords(const CVector<CString>& Words, bool bRegExp);
    bool RetrieveFile(CSite * /* Parent */, const CString& /* URL */, CString * /* Contents */ = NULL) const;

    CIndex(void);
    CIndex(const CString&);
    virtual ~CIndex(void);
    
    void SetIndex(const CString&, bool bLoad = true, bool bMakeAlreadyIndexed = true);
    void SetExts(const CString&, bool Overwrite = true);
    
    void Index(
        CRemoteFile&, 
        const CUrl&, 
        CString& Server, 
        CVector<CString>& Links,         
        CSite * Parent, 
        CVector<CString>&, 
        const CIndexObject&,
        CString * = NULL);

    bool Search(CSearchObject& /* SearchObject */);	
    bool SetIndexHTML(const CString&);
    void SetModifiedFlag(void);
    
    CIndex& Append(CIndex& Index);
    bool AppendIndex(CIndex& Index);
    inline CIndex& operator+=(CIndex& Index) { return Append(Index); }
    
    void PopulateXmlNode(CXmlTree& Tree, CTreeElement< CXmlNode > * pXmlNode) const; 
    void AppendSucceededAuthState(CSite * Parent, const CRemoteFile& RemoteFile, CString * Result);
};

inline void CIndex :: SetInsens(bool Value) { 
    m_IndexOptions.m_Insens = Value; 
    m_URLManager.SetInsens(Value); 
}

inline void CIndex :: SetExactSize(int Value) { 
    m_Searcher.SetExactSize(Value); 
}

inline const CString& CIndex :: GetProxy(void) const {
    return m_IndexOptions.m_Proxy;
}
    
#endif
