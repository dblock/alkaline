
/*
    © Vestris Inc., Geneva, Switzerland
    http://www.vestris.com, 1994-1999 All Rights Reserved
    ______________________________________________

    written by Hassan Sultan - hsultan@vestris.com

*/

#ifndef CSEARCH_HPP
#define CSEARCH_HPP

#include <platform/include.hpp>
#include <Letter/Letter.hpp>
#include <LettersTree/LettersTree.hpp>
#include <Mutex/Mutex.hpp>
#include <StringA/StringA.hpp>
#include <Vector/Vector.hpp>
#include <Vector/IntVector.hpp>
#include <Vector/SVector.hpp>
#include <Cache/Cache.hpp>
#include <File/LocalFile.hpp>
#include <File/MMapFile.hpp>
#include <File/Progress.hpp>
#include <Io/Io.hpp>
#include <Search/SearchTypes.hpp>
#include <Tree/XmlTree.hpp>
#include <Rank/Rank.hpp>
#include <LettersHandler/LettersHandler.hpp>

//typedef CVector<int> CVectorInt;

typedef CVector<CStringA> CStringAVector;

#define CHARSET_SIZE 256

class CIndex;
    
class CSearch : public CObject
{
private:
  copy_property(CIndex *, Parent);  
  CLettersTree m_WordsTree[CHARSET_SIZE];
  property(unsigned int, ExactSize);
  property(CAtomic, WordsCount);
  property(bool, RightPartialOnly);

  // the following values must not exceed 15
  readonly_property(unsigned short int, TitleWeight);
  readonly_property(unsigned short int, KeywordWeight);
  readonly_property(unsigned short int, DescriptionWeight);
  readonly_property(unsigned short int, TextWeight);

  readonly_property(unsigned short int, MaxWordSize);

  inline void SetTitleWeight(unsigned short int Value);
  inline void SetTextWeight(unsigned short int Value);	
  inline void SetKeywordWeight(unsigned short int Value);	
  inline void SetDescriptionWeight(unsigned short int Value);

  inline void SetMaxWordSize(unsigned short int Value);

  property(CVector<CString>, WeakWords);

  unsigned short int GetWeight(const CString& Word); //Gives the weight of the word

  CLettersHandler m_AllLetters; //Will contain a pointer to all the nodes in the trees, sorted by letter

  unsigned char m_Endian;
  
  CVectorInt m_PagesToRemove;   
  CVectorInt m_PagesToRemovePermanently;
  property(CBitSet, AlreadyIndexedPages);

  void PurgeBadPages();
  void GetAlreadyIndexedVector(CSVector& Result);

  /* cache */
    
  property(int, SearchCacheLife);
    
  CCache m_CacheExact;
  CCache m_CacheInsensExact;
  CCache m_CachePartial;
  CCache m_CacheInsensPartial;

  void InvalidateCache(long nPeriod);

  static void Dequote(CString& iStr);
  static void Dequote(CStringA& iStr);
  
  bool FindWithCache(const CVector<CStringA>& Words, CCache& Cache, CSearchType SearchType, CVector<CSVector> *);

  bool FindExactWord(const CVector<CStringA>&, CVector<CSVector> *);
  bool FindInsensExactWord(const CVector<CStringA>&, CVector<CSVector> *);
  bool FindPartialWord(const CVector<CStringA>&, CVector<CSVector> *);
  bool FindInsensPartialWord(const CVector<CStringA>&, CVector<CSVector> *);

  bool FindPartSensitive(const CVector<CStringA>& Words, CVector<CSVector> *pvResult);
  bool FindPartInsensitive(const CVector<CStringA>& Words, CVector<CSVector> *pvResult);

  void RemovePageNonWriteLocked(int Page);

  bool LoadNode(FILE *IndexFile, CString Word, unsigned char Endian, CSVector& TempVector);
public:
  mutable_property(CRWMutex, RwMutex);

  inline void StartReading(void) const { m_RwMutex.StartReading(); }
  inline void StopReading(void) const { m_RwMutex.StopReading(); }
  inline void StartWriting(void) { m_RwMutex.StartWriting(); }
  inline void StopWriting(void) { m_RwMutex.StopWriting(); }

public:
  CSearch();
  bool Find(CSearchData& /* Data */);
  static int GetQuality(CSearchData& /* Data */, int /* Index */);
  //void AddWord(const CString& Word, const int Page);
  void AddWord(const CString& Word, const CVector<int>& Pages);
  void AddPage(const CVector<CString>& Words, const int Page);
  void AddPage(const CVector<CStringA>& Words, const int Page);
  void AddPageIndex(const int Page, int& RealPage);

  bool RemoveWords(const CVector<CString>& Words, bool bRegExp);
  bool RemoveWordsPattern(const CVector<CString>& Words);
  bool RemoveWordsRegExp(const CVector<CString>& Words);

  inline int GetWordCount(void) const { return m_WordsCount.Get(); }

  void RemovePage(int Page);

  void ForcePurgeOfBadPages();
  void RemovePermanently(const  int Page);

  void MakeAlreadyIndexed(bool Verbose);
  bool AlreadyIndexed(int Page) const;
  void ClearAlreadyIndexed();
  
  void ShowTree() {for(int i=0;i<CHARSET_SIZE;i++) m_WordsTree[i].ShowTree();} 
  bool WriteIndex(const CString& FileName, bool Verbose);

  bool LoadIndex(const CString& FileName, bool Verbose, bool bMakeAlreadyIndexed = true);

  bool LoadIndex_1_31(const CString& FileName, bool Verbose, bool bMakeAlreadyIndexed = true);
  bool LoadIndex_1_4(const CString& FileName, bool Verbose, bool bMakeAlreadyIndexed = true);
  void RefreshNodesCache();

  void PopulateXmlNode(CXmlTree& Tree, CTreeElement< CXmlNode > * pXmlNode) const;

  void AppendTo(CSearch& Searcher, const CVector<int>& DispTable);
  void RemoveAll(bool bLocked = true);

  long GetDataSize(void) const;
};

inline void CSearch::SetTitleWeight(unsigned short int Value)
{
	if (Value < 16)
		m_TitleWeight=Value;
	else m_TitleWeight = 10;
}

inline void CSearch::SetKeywordWeight(unsigned short int Value)
{
	if (Value < 16)
		m_KeywordWeight = Value;
	else m_KeywordWeight = 8;
}

inline void CSearch::SetDescriptionWeight(unsigned short int Value)
{
	if (Value < 16)
        m_DescriptionWeight = Value;
	else m_DescriptionWeight = 6;
}

inline void CSearch::SetTextWeight(unsigned short int Value)
{
	if (Value < 16)
        m_TextWeight = Value;
    else m_TextWeight = 3;
}

inline void CSearch::SetMaxWordSize(unsigned short int Value)
{
	if (Value < 5000)
        m_MaxWordSize = Value;
    else m_MaxWordSize = 64;
}
#endif
