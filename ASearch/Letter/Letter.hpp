/*
    © Vestris Inc., Geneva, Switzerland
    http://www.vestris.com, 1994-1999 All Rights Reserved
    ______________________________________________

    written by Hassan Sultan - hsultan@vestris.com

*/



#ifndef ALKALINE_C_LETTER_HPP
#define ALKALINE_C_LETTER_HPP

#include <platform/include.hpp>
#include <String/String.hpp>
#include <Vector/SVector.hpp>
#include <StringA/StringA.hpp>
#include <File/MMapFile.hpp>
#include <BitSet/BitSet.hpp>
#include <LettersHandler/LettersHandler.hpp>
#include <RegExp/RegExp.hpp>
#include <SwapVector/SwapVector.hpp>

typedef CVector<CSwapVector*> RESULTYPE;

#define MAX_WORD_SIZE 255
    
#define Flip_int32(type) (((type >>24) & 0x000000ff) | ((type >> 8) & 0x0000ff00) | ((type << 8) & 0x00ff0000) | ((type <<24) & 0xff000000))

class CSearch;

class CLettersHandler;

class CLetter 
{
public:
	
	// static CAtomic s_AllNodeCount;
	// static CAtomic s_EmptyNodeCount;
	// static CAtomic s_FullNodeCount;

	static int s_CacheDepth;

	void ShowTree(unsigned int Depth);
	inline CString GetWord(void) const { return CString((char*)m_Content, m_ContentSize); }
	long GetDataSize(void) const;

	unsigned char * m_Content;
	unsigned char m_ContentSize;

private:

    property(CVector<CLetter *>, NextLetters);

	CSwapVector * m_PagesVector;
	CSVector * m_CacheVector;

	inline void AddCacheEntry(int Page, unsigned int WordPos);
	inline void AddCacheEntry(const CVector<int>& Pages, unsigned int WordPos);
	inline void AddCacheEntry(const CSwapVector& Pages, unsigned int WordPos);

	inline void GetOwnResults(RESULTYPE& Result);
    inline void GetSubVectors(RESULTYPE& Result);
	inline unsigned char* AllocBuf(const unsigned char Size);
	inline void DelBuf();
    void GetLowerThan( long CurValue, const long FinalValue, const bool IsNegative, unsigned int Start,RESULTYPE& Result);
    void GetGreaterThan( long CurValue, const long FinalValue,const bool IsNegative, unsigned int Start,RESULTYPE& Result);

	void AddSubWord(const CString& Word, const unsigned int WordPos, const int Page, bool& IsNewWord, CLettersHandler *LettersHandler);
	void AddSubWord(const CString& Word, const unsigned int WordPos, const CVector<int>& Pages, bool& IsNewWord, CLettersHandler *LettersHandler);
	void AddSubWord(const CString& Word, const unsigned int WordPos, const CSVector& Pages, bool& IsNewWord, CLettersHandler *LettersHandler);

	void MoveOwnSubWord(const int StartPos, CLettersHandler *LettersHandler, unsigned int WordPos);

	void SetContent(unsigned char *SubWord, const int SubWordSize, CLettersHandler * LettersHandler);
	void SetPagesVector(CSwapVector *NewPagesVector);
public:

    /* Add Words */
    void AddNewWord(const CString& Word, unsigned int WordPos, const int Page, bool& IsNewWord, CLettersHandler *LettersHandler);
	void AddNewWord(const CString& Word, unsigned int WordPos, const CVector<int>& Pages, bool& IsNewWord, CLettersHandler *LettersHandler);
	void AddNewWord(const CString& Word, unsigned int WordPos, const CSVector& Pages, bool& IsNewWord, CLettersHandler *LettersHandler);

    void AppendTo(CSearch * OtherSearcher, const CVector<int>& DispTable, CString& Buffer);

	void RecalcCache(int Depth, CSVector *LeafPages);
	
	/* Remove Words */
    bool RemoveWordsRegExp(const CVector<CRegExp>& Words, CString& CurrentBuffer, int& DeletedWords, CLettersHandler * LettersHandler);
    bool RemoveWord(const CString& Word, const unsigned int WordPos, const unsigned int Start, bool &WasDeleted, CLettersHandler *LettersHandler);
    bool RemoveWordsInMeta(const CVector<CString>& Words, int& DeletedWords, CLettersHandler *LettersHandler);
        
    void RemoveAll();
    void RemovePages(const CVectorInt& Pages,const CVectorInt& PagesDeleted, unsigned int& WordsCount);

    void MakeAlreadyIndexed(CBitSet *AlreadyIndexedPages);
    /* Find Words */
    void FindExactWordSensitive(const CString& Word, const unsigned int WordPos, const unsigned int Start, RESULTYPE& Result);
    void FindPartialWordSensitive(const CString& Word, const unsigned int WordPos, const unsigned int Start, RESULTYPE& Result);

    void FindPartSensitive(const CString& Word, const int WordPos, const unsigned int Start, RESULTYPE& Result);
    void FindPartMetaSensitive(const CString& Word, const unsigned int WordPos, const unsigned int Start, RESULTYPE& Result);

    void FindPartInsensitive(const CString& Word, const int WordPos, const unsigned int Start, RESULTYPE& Result);
    void FindPartMetaInsensitive(const CString& Word, const unsigned int WordPos, const unsigned int Start, RESULTYPE& Result);

    void FindExactWordInsensitive(const CString& Word, const unsigned int WordPos, const unsigned int Start, RESULTYPE& Result);
    void FindPartialWordInsensitive(const CString& Word, const unsigned int WordPos, const unsigned int Start, RESULTYPE& Result);
    
    /* Find Meta */
    void FindExactMetaSensitive(const CString& Word, const unsigned int WordPos, const unsigned int Start, RESULTYPE& Result);
    void FindPartialMetaSensitive(const CString& Word, const unsigned int WordPos, const unsigned int Start, RESULTYPE& Result);
    
    void FindExactMetaInsensitive(const CString& Word, const unsigned int WordPos, const unsigned int Start, RESULTYPE& Result);
    void FindPartialMetaInsensitive(const CString& Word, const unsigned int WordPos, const unsigned int Start, RESULTYPE& Result);
    
    void FindSensitiveGreaterThan(const CString& Word, unsigned int WordPos, const unsigned int Start, RESULTYPE& Result);
    void FindSensitiveLowerThan(const CString& Word, unsigned int WordPos, const unsigned int Start, RESULTYPE& Result);
    void FindInsensitiveGreaterThan(const CString& Word, unsigned int WordPos, const unsigned int Start, RESULTYPE& Result);
    void FindInsensitiveLowerThan(const CString& Word, unsigned int WordPos, const unsigned int Start, RESULTYPE& Result);
   
    /* Serialization, Loading */
    bool Serialize(CMMapFile& IndexFile);
    void WriteIndex(FILE *IndexFile, CString& Buffer); //temporary, write the index the same way as the old version
    bool Load(FILE *IndexFile, unsigned int& WordsCount, unsigned char Endian, CLettersHandler *LettersHandler);   
    bool Load(CMMapFile& IndexFile, unsigned int& WordsCount, unsigned char Endian, CLettersHandler *LettersHandler);   

    /* Class inits, destruction */    
    CLetter();
    ~CLetter();
};

inline unsigned char* CLetter::AllocBuf(const unsigned char Size)
{	
	return new unsigned char[Size];
}

inline void CLetter::DelBuf()
{
	if (m_Content != NULL)
	{
		delete[] m_Content;
		m_Content = NULL;
	}
    m_ContentSize=0;
}

inline void CLetter::GetSubVectors(RESULTYPE& Result)
{
    register unsigned int i;
    
//	if ( (m_CacheVector==NULL) || (m_CacheVector->GetSize()==0) )
//	{
	
		if (m_PagesVector != NULL)
		{
//		cout<<"Pages found(GetSubVectors): [";m_PagesVector->Display();cout<<"]"<<endl;
			Result.Add(m_PagesVector);
		}

		for(i = 0; i < m_NextLetters.GetSize(); i++)
		{
			m_NextLetters[i]->GetSubVectors(Result);
		}
/*	}
	else
	{
		Result.Add(m_CacheVector);
	}
*/
}

inline void CLetter::GetOwnResults(RESULTYPE& Result)
{
	if (m_PagesVector != NULL)
	{
//		cout<<"Pages found(GetOwnResults): [";m_PagesVector->Display();cout<<"]"<<endl;
		Result.Add(m_PagesVector);
	}
}

#endif
