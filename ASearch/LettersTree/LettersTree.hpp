/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  ______________________________________________

  written by Hassan Sultan - hsultan@vestris.com
  
*/

#ifndef ALKALINE_C_LETTERS_TREE_HPP
#define ALKALINE_C_LETTERS_TREE_HPP


#include <platform/include.hpp>
#include <Letter/Letter.hpp>
#include <Mutex/Mutex.hpp>
#include <Vector/SVector.hpp>
#include <StringA/StringA.hpp>
#include <Mutex/RWMutex.hpp>
#include <LettersHandler/LettersHandler.hpp>
#include <Sort/Sort.hpp>
#include <SwapVector/SwapVector.hpp>


#define MAX_NORMALSORT 20000
#define MINVECT_NEWSORT 10
    
class CLettersTree : public CObject
{
private:

    //  CVector<CVectorInt> m_PagesVector,m_CacheVector;
    
    CMutex m_TreeMutex;
    mutable_property(CRWMutex, RwMutex);
    property(unsigned int, OwnWordsCount);
	CLettersHandler *m_AllLetters;

public:

    CLetter m_FirstLetter;
    CLettersTree() ;
    ~CLettersTree();
    
	void SetLettersHandler(CLettersHandler *LettersHandler);

	inline void RecalcCache();

	void ShowTree() { m_FirstLetter.ShowTree(0); };
    bool Serialize(CMMapFile& IndexFile);
    inline void WriteIndex(FILE *IndexFile);
    inline bool Load(FILE *IndexFile, CAtomic& WordsCount, unsigned char Endian);
    inline bool Load(CMMapFile& IndexFile, CAtomic& WordsCount, unsigned char Endian);
    inline void AppendTo(CSearch& OtherSearcher,const CVector<int>& DispTable);
    
    inline void AddWord(const CString& Word, const int Page, bool& IsNewWord);
    inline void AddWord(const CString& Word, const CVector<int>& Pages, bool& IsNewWord);
	inline void AppendWord(const CString& Word, const CSVector& Pages, bool& IsNewWord);
	
    inline void FindExactWordSensitive(const CString& Word, CSVector& Result);
    inline void FindPartialWordSensitive(const CString& Word, CSVector& Result);
    inline void FindExactWordInsensitive(const CString& Word, CSVector& Result);
    inline void FindPartialWordInsensitive(const CString& Word, CSVector& Result);
    
    
    inline void FindPartSensitive(const CString& Word, CSVector& Result);
    inline void FindPartMetaSensitive(const CString& Word, CSVector& Result);
    
    inline void FindPartInsensitive(const CString& Word, CSVector& Result);
    inline void FindPartMetaInsensitive(const CString& Word, CSVector& Result);
    
    
    inline void RemoveAll();
    
    inline void FindSensitiveLowerThan(const CString& Word, CSVector& Result);
    inline void FindSensitiveGreaterThan(const CString& Word, CSVector& Result);
    inline void FindInsensitiveLowerThan(const CString& Word, CSVector& Result);
    inline void FindInsensitiveGreaterThan(const CString& Word, CSVector& Result);
    inline void FindEqualTo(const CString& Word, CSVector& Result);
    
    
    inline void FindExactMetaSensitive(const CString& Word, CSVector& Result);
    inline void FindPartialMetaSensitive(const CString& Word, CSVector& Result);
    inline void FindExactMetaInsensitive(const CString& Word, CSVector& Result);
    inline void FindPartialMetaInsensitive(const CString& Word, CSVector& Result);
    
    inline void RemovePages(const CVectorInt& Pages,const CVectorInt& PagesDeleted);
    inline void RemoveWordsRegExp(const CVector<CRegExp>& Words, int& DeletedWords);
    inline bool RemoveWord(const CString& Word);
    inline void RemoveWordsInMeta(const CVector<CString>& Words, int& DeletedWords);

    inline void MakeAlreadyIndexed(CBitSet *AlreadyIndexedPages);
    inline int GetWordsCount(void) const { return m_OwnWordsCount; }
    inline void StartReading(void) const { m_RwMutex.StartReading(); }
    inline void StopReading(void) const { m_RwMutex.StopReading(); }
    inline void StartWriting(void) { m_RwMutex.StartWriting(); }
    inline void StopWriting(void) { m_RwMutex.StopWriting(); }

	inline long GetDataSize(void) const { return m_FirstLetter.GetDataSize(); }
};

inline void CLettersTree::RecalcCache()
{
	CSVector TempVector; 
	
	StartWriting(); 
	m_FirstLetter.RecalcCache(0,&TempVector); 
	StopWriting();
}

inline void CLettersTree::SetLettersHandler(CLettersHandler *LettersHandler)
{
	m_AllLetters=LettersHandler;
}

inline void CLettersTree::RemoveAll()
{
    StartWriting();
    m_FirstLetter.RemoveAll();
    StopWriting();
}

inline void CLettersTree::WriteIndex(FILE *IndexFile) 
{
    StartWriting();
    //cout<<"Writing index for letter: ["<<(int)m_FirstLetter.GetLetter()<<"]"<<endl;
    CString Buffer;  
    m_FirstLetter.WriteIndex(IndexFile, Buffer);
    StopWriting();
}

inline bool CLettersTree::Load(FILE *IndexFile, CAtomic& WordsCount,unsigned char Endian) 
{
    StartWriting();
    unsigned int OwnWordsCount=0;
    bool Result = m_FirstLetter.Load(IndexFile, OwnWordsCount, Endian, m_AllLetters);
    m_OwnWordsCount = OwnWordsCount;
    WordsCount.Set(OwnWordsCount + WordsCount.Get());
    StopWriting();
    return Result;
}

inline bool CLettersTree::Load(CMMapFile& IndexFile, CAtomic& WordsCount, unsigned char Endian) 
{
    StartWriting();
    unsigned int OwnWordsCount = 0;
    bool Result = m_FirstLetter.Load(IndexFile, OwnWordsCount, Endian, m_AllLetters);
    m_OwnWordsCount = OwnWordsCount;
    WordsCount.Set(OwnWordsCount + WordsCount.Get());
    StopWriting();
    return Result;
}

inline void CLettersTree::AppendTo(CSearch& OtherSearcher,const CVector<int>& DispTable)
{
    StartWriting();
    CString Buffer;
    m_FirstLetter.AppendTo(&OtherSearcher, DispTable, Buffer);
    StopWriting();
}

inline void CLettersTree::RemoveWordsInMeta(const CVector<CString>& Words, int& DeletedWords)
{
    StartWriting();
    m_FirstLetter.RemoveWordsInMeta(Words,DeletedWords, m_AllLetters);
    StopWriting();
}

inline bool CLettersTree::RemoveWord(const CString& Word)
{
    //cout<<"In letters tree RemoveWord: ["<<Word[0]<<"] for word: "<<Word<<endl;
    bool Result=false;
    StartWriting();
    m_FirstLetter.RemoveWord(Word,0,0,Result, m_AllLetters);
    if (Result) m_OwnWordsCount--;
    StopWriting();
    return Result;
}

inline void CLettersTree::RemoveWordsRegExp(const CVector<CRegExp>& RegWords, int& DeletedWords) {
    StartWriting();
    CString CurrentBuffer;
    m_FirstLetter.RemoveWordsRegExp(RegWords, CurrentBuffer, DeletedWords, m_AllLetters);
    StopWriting();
}

inline void CLettersTree::AddWord(const CString& Word, const int Page, bool& IsNewWord)
{
//    cout<<"In letters tree AddWord 1 page: ["<<Word[0]<<"] for word: "<<Word<<" and page : "<<Page<<endl;
    if (Word.GetLength()>MAX_WORD_SIZE) {IsNewWord=false;return;}    
    StartWriting();
//    m_FirstLetter.AddWord(Word,0,Page,IsNewWord, m_AllLetters);
    m_FirstLetter.AddNewWord(Word,0,Page,IsNewWord, m_AllLetters);
    if (IsNewWord) m_OwnWordsCount++;
    StopWriting();
}


inline void CLettersTree::AddWord(const CString& Word, const CVector<int>& Pages, bool& IsNewWord)
{
//    cout<<"In letters tree AddWord Vector for word: "<<Word<<endl;
    if (Word.GetLength()>MAX_WORD_SIZE) {IsNewWord=false;return;}
    StartWriting();
//    m_FirstLetter.AddWord(Word,0,Pages,IsNewWord, m_AllLetters);
    m_FirstLetter.AddNewWord(Word,0,Pages,IsNewWord, m_AllLetters);
    if (IsNewWord) m_OwnWordsCount++;
    StopWriting();
}


inline void CLettersTree::AppendWord(const CString& Word, const CSVector& Pages, bool& IsNewWord)
{
//    cout<<"In letters tree AddWord Vector for word: "<<Word<<endl;
	if (Word.GetLength()>MAX_WORD_SIZE) {IsNewWord=false;return;}
    StartWriting();
    m_FirstLetter.AddNewWord(Word,0,Pages,IsNewWord, m_AllLetters);
    if (IsNewWord) m_OwnWordsCount++;
    StopWriting();
}

inline void CLettersTree::RemovePages(const CVectorInt& Pages,const CVectorInt& PagesDeleted)
{
    StartWriting();
    //cout<<"In letters tree RemovePages ["<<(int)m_FirstLetter.GetLetter()<<"]"<<endl;
    m_FirstLetter.RemovePages(Pages, PagesDeleted,m_OwnWordsCount);
    StopWriting();
    
}


inline void CLettersTree::MakeAlreadyIndexed(CBitSet *AlreadyIndexedPages)
{
  StartWriting();
  m_FirstLetter.MakeAlreadyIndexed(AlreadyIndexedPages);
  StopWriting();
}



inline void CLettersTree::FindSensitiveGreaterThan(const CString& Word, CSVector& Result)
{

  StartReading();

  CVector<CSwapVector*> ResultVectors;
  unsigned int i;
  m_FirstLetter.FindSensitiveGreaterThan(Word, 0, 0, ResultVectors);
  int Total=0;
  for(i=0;i<ResultVectors.GetSize();i++)
	  Total+=(*(ResultVectors[i])).GetSize();
  CSort ResSorter;
  ResSorter.SetVectors(ResultVectors);
  Result.Add(ResSorter.GetSortedSwapVector());
  StopReading();

}

inline void CLettersTree::FindSensitiveLowerThan(const CString& Word, CSVector& Result)
{
  StartReading();

  CVector<CSwapVector*> ResultVectors;
  unsigned int i;
  m_FirstLetter.FindSensitiveLowerThan(Word, 0, 0, ResultVectors);
  int Total=0;
  for(i=0;i<ResultVectors.GetSize();i++)
	  Total+=(*(ResultVectors[i])).GetSize();
  CSort ResSorter;
  ResSorter.SetVectors(ResultVectors);
  Result.Add(ResSorter.GetSortedSwapVector());
  StopReading();
}

    

inline void CLettersTree::FindInsensitiveGreaterThan(const CString& Word, CSVector& Result)
{

  StartReading();

  CVector<CSwapVector*> ResultVectors;
  unsigned int i;
  m_FirstLetter.FindInsensitiveGreaterThan(Word, 0, 0, ResultVectors);
  int Total=0;
  for(i=0;i<ResultVectors.GetSize();i++)
	  Total+=(*(ResultVectors[i])).GetSize();
  CSort ResSorter;
  ResSorter.SetVectors(ResultVectors);
  Result.Add(ResSorter.GetSortedSwapVector());
  StopReading();

}

inline void CLettersTree::FindInsensitiveLowerThan(const CString& Word, CSVector& Result)
{

  StartReading();

  CVector<CSwapVector*> ResultVectors;
  unsigned int i;
  m_FirstLetter.FindInsensitiveLowerThan(Word, 0, 0, ResultVectors);
  int Total=0;
  for(i=0;i<ResultVectors.GetSize();i++)
	  Total+=(*(ResultVectors[i])).GetSize();
	CSort ResSorter;
	ResSorter.SetVectors(ResultVectors);
	Result.Add(ResSorter.GetSortedSwapVector());
  StopReading();

}

    
    
inline void CLettersTree::FindEqualTo(const CString& Word, CSVector& Result)
{
    StartReading();

  CVector<CSwapVector*> ResultVectors;
  unsigned int i;
  m_FirstLetter.FindExactWordSensitive(Word, 0, 0, ResultVectors);
  int Total=0;
  for(i=0;i<ResultVectors.GetSize();i++)
	  Total+=(*(ResultVectors[i])).GetSize();
	CSort ResSorter;
	ResSorter.SetVectors(ResultVectors);
	Result.Add(ResSorter.GetSortedSwapVector());
    StopReading();
}



inline void CLettersTree::FindExactWordSensitive(const CString& Word, CSVector& Result)
{
    StartReading();

  CVector<CSwapVector*> ResultVectors;
  unsigned int i;
  m_FirstLetter.FindExactWordSensitive(Word, 0, 0, ResultVectors);
  int Total=0;
  for(i=0;i<ResultVectors.GetSize();i++)
	  Total+=(*(ResultVectors[i])).GetSize();
	CSort ResSorter;
	ResSorter.SetVectors(ResultVectors);
	Result.Add(ResSorter.GetSortedSwapVector());
    StopReading();
}


inline void CLettersTree::FindPartialWordSensitive(const CString& Word, CSVector& Result)
{
    StartReading();

  CVector<CSwapVector*> ResultVectors;
  unsigned int i;
  m_FirstLetter.FindPartialWordSensitive(Word, 0, 0, ResultVectors);
  int Total=0;
  for(i=0;i<ResultVectors.GetSize();i++)
	  Total+=(*(ResultVectors[i])).GetSize();
	CSort ResSorter;
	ResSorter.SetVectors(ResultVectors);
	Result.Add(ResSorter.GetSortedSwapVector());
    StopReading();
}



inline void CLettersTree::FindPartSensitive(const CString& Word, CSVector& Result)
{
    StartReading();

  CVector<CSwapVector*> ResultVectors;
  unsigned int i;
    if (m_FirstLetter.m_Content[0] == (unsigned char) Word[0]) {
        m_FirstLetter.FindPartSensitive(Word, 0, 0, ResultVectors);
    } else {
        m_FirstLetter.FindPartSensitive(Word,-1,0, ResultVectors);
    }
  int Total=0;
  for(i=0;i<ResultVectors.GetSize();i++)
	  Total+=(*(ResultVectors[i])).GetSize();
	CSort ResSorter;
	ResSorter.SetVectors(ResultVectors);
	Result.Add(ResSorter.GetSortedSwapVector());
    StopReading();
}

inline void CLettersTree::FindPartMetaSensitive(const CString& Word, CSVector& Result)
{
    StartReading();

  CVector<CSwapVector*> ResultVectors;
  unsigned int i;
  m_FirstLetter.FindPartMetaSensitive(Word, 0, 0, ResultVectors);
  int Total=0;
  for(i=0;i<ResultVectors.GetSize();i++)
	  Total+=(*(ResultVectors[i])).GetSize();
	CSort ResSorter;
	ResSorter.SetVectors(ResultVectors);
	Result.Add(ResSorter.GetSortedSwapVector());
    StopReading();
}

inline void CLettersTree::FindPartInsensitive(const CString& Word, CSVector& Result)
{
    StartReading();

  CVector<CSwapVector*> ResultVectors;
  unsigned int i;
    if ( (m_FirstLetter.m_Content[0] == (unsigned char) Word[0]) 
        || (m_FirstLetter.m_Content[0] == (unsigned char) toupper((unsigned char) Word[0])) )
        m_FirstLetter.FindPartInsensitive(Word, 0, 0, ResultVectors);
    else
        m_FirstLetter.FindPartInsensitive(Word, -1, 0, ResultVectors);
  int Total=0;
  for(i=0;i<ResultVectors.GetSize();i++)
	  Total+=(*(ResultVectors[i])).GetSize();
	CSort ResSorter;
	ResSorter.SetVectors(ResultVectors);
	Result.Add(ResSorter.GetSortedSwapVector());
    StopReading();
}

inline void CLettersTree::FindPartMetaInsensitive(const CString& Word, CSVector& Result)
{
    StartReading();

  CVector<CSwapVector*> ResultVectors;
  unsigned int i;
  m_FirstLetter.FindPartMetaInsensitive(Word, 0, 0, ResultVectors);
  int Total=0;
  for(i=0;i<ResultVectors.GetSize();i++)
	  Total+=(*(ResultVectors[i])).GetSize();
	CSort ResSorter;
	ResSorter.SetVectors(ResultVectors);
	Result.Add(ResSorter.GetSortedSwapVector());
    StopReading();
}



inline void CLettersTree::FindExactWordInsensitive(const CString& Word, CSVector& Result)
{
    StartReading();

  CVector<CSwapVector*> ResultVectors;
  unsigned int i;
  m_FirstLetter.FindExactWordInsensitive(Word, 0, 0, ResultVectors);
  int Total=0;
  for(i=0;i<ResultVectors.GetSize();i++)
	  Total+=(*(ResultVectors[i])).GetSize();
	CSort ResSorter;
	ResSorter.SetVectors(ResultVectors);
	Result.Add(ResSorter.GetSortedSwapVector());
    StopReading();
}

inline void CLettersTree::FindPartialWordInsensitive(const CString& Word, CSVector& Result)
{
    StartReading();

  CVector<CSwapVector*> ResultVectors;
  unsigned int i;
  m_FirstLetter.FindPartialWordInsensitive(Word, 0, 0, ResultVectors);
  int Total=0;

  for(i=0;i<ResultVectors.GetSize();i++)
	  Total+=(*(ResultVectors[i])).GetSize();

  CSort ResSorter;
  ResSorter.SetVectors(ResultVectors);
  Result.Add(ResSorter.GetSortedSwapVector());
  StopReading();
}

inline void CLettersTree::FindExactMetaSensitive(const CString& Word, CSVector& Result)
{
    StartReading();

  CVector<CSwapVector*> ResultVectors;
  unsigned int i;
  m_FirstLetter.FindExactMetaSensitive(Word, 0, 0, ResultVectors);
  int Total=0;
  for(i=0;i<ResultVectors.GetSize();i++)
	  Total+=(*(ResultVectors[i])).GetSize();
	CSort ResSorter;
	ResSorter.SetVectors(ResultVectors);
	Result.Add(ResSorter.GetSortedSwapVector());
    StopReading();
}

inline void CLettersTree::FindPartialMetaSensitive(const CString& Word, CSVector& Result)
{
    StartReading();

  CVector<CSwapVector*> ResultVectors;
  unsigned int i;
  m_FirstLetter.FindPartialMetaSensitive(Word, 0, 0, ResultVectors);
  int Total=0;
  for(i=0;i<ResultVectors.GetSize();i++)
	  Total+=(*(ResultVectors[i])).GetSize();
	CSort ResSorter;
	ResSorter.SetVectors(ResultVectors);
	Result.Add(ResSorter.GetSortedSwapVector());
    StopReading();
}

inline void CLettersTree::FindExactMetaInsensitive(const CString& Word, CSVector& Result)
{
    StartReading();

  CVector<CSwapVector*> ResultVectors;
  unsigned int i;
  m_FirstLetter.FindExactMetaInsensitive(Word, 0, 0, ResultVectors);
  int Total=0;
  for(i=0;i<ResultVectors.GetSize();i++)
	  Total+=(*(ResultVectors[i])).GetSize();
	CSort ResSorter;
	ResSorter.SetVectors(ResultVectors);
	Result.Add(ResSorter.GetSortedSwapVector());
    StopReading();
}

inline void CLettersTree::FindPartialMetaInsensitive(const CString& Word, CSVector& Result)
{
    StartReading();

  CVector<CSwapVector*> ResultVectors;
  unsigned int i;
  m_FirstLetter.FindPartialMetaInsensitive(Word, 0, 0, ResultVectors);
  int Total=0;
  for(i=0;i<ResultVectors.GetSize();i++)
	  Total+=(*(ResultVectors[i])).GetSize();
	CSort ResSorter;
	ResSorter.SetVectors(ResultVectors);
	Result.Add(ResSorter.GetSortedSwapVector());
    StopReading();
}

#endif
