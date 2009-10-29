/*

   Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  ______________________________________________
  
  written by Hassan Sultan - hsultan@infomaniak.ch
  and Daniel Doubrovkine - dblock@vestris.com
    
*/

#include <alkaline.hpp>
#include "Search.hpp"
#include <String/GStrings.hpp>
#include <Main/TraceTags.hpp>
#include <Index/Index.hpp>
#include <File/MMapFile.hpp>

CSearch::CSearch() : 
    m_ExactSize(3),
    m_RightPartialOnly(false),
    m_TitleWeight(10),
    m_KeywordWeight(8),
    m_DescriptionWeight(6),
    m_TextWeight(4),
    m_MaxWordSize(64),
#ifdef WORDS_BIGENDIAN
    m_Endian(1),
#else
    m_Endian(0),
#endif
    m_SearchCacheLife(60)
{
	    
	unsigned char value[2];
	for(unsigned int i=0;i<CHARSET_SIZE;i++)
	{
//        m_WordsTree[(unsigned char) i].m_FirstLetter.SetLetter(i);
		value[0]=(unsigned char)i;
		m_WordsTree[(unsigned char) i].SetLettersHandler(&m_AllLetters); 
		m_WordsTree[(unsigned char) i].m_FirstLetter.SetContent(value,1,&m_AllLetters);
	}

/*
	m_WeakWords+="for";
	m_WeakWords+="and";
	m_WeakWords+="or";
	m_WeakWords+="my";
	m_WeakWords+="he";
	m_WeakWords+="you";
	m_WeakWords+="your";
	m_WeakWords+="his";
	m_WeakWords+="her";
	m_WeakWords+="our";
	m_WeakWords+="she";
	m_WeakWords+="it";
	m_WeakWords+="we";
	m_WeakWords+="not";
	m_WeakWords+="to";
	m_WeakWords+="the";
	m_WeakWords+="of";
	m_WeakWords+="a";
	m_WeakWords+="an";
	m_WeakWords+="in";
	m_WeakWords+="on";
	*/
}

void CSearch::Dequote(CStringA& StringA)
{
    if (StringA.GetLength())
    {
        switch(StringA[0])
        {
        case '\"':
        case '\'':
        case '`':
            StringA.Delete(0, 1);
        };
        
        switch(StringA[StringA.GetLength()-1])
        {
        case '\"':
        case '\'':
        case '`':
            StringA.Delete(StringA.GetLength()-1, 1);
        };
        StringA.Trim();
    }
}

void CSearch::Dequote(CString& String) {
    if (String.GetLength()) {
        switch(String[0]){
        case '\"':
        case '\'':
        case '`':
            String.Delete(0, 1);
        };
    }
    if (String.GetLength()) {
        switch(String[String.GetLength()-1]){
        case '\"':
        case '\'':
        case '`':
            String.Delete(String.GetLength()-1, 1);
        };
    }
    String.Trim();
}



/////////////////////                           ////////////////////
/////////////////////  PAGES REMOVAL METHODS    ////////////////////


/*
* Remove a page from the pages vectors, and switch the negative value to a positive one if possible
* This method does NOT REMOVE THE PAGE COMPLETELY, for this use RemovePermanently()
* This method is intended for use when reindexing pages, not removing pages
*/
void CSearch::RemovePage(int Page)
{
    Trace(tagSearch, levVerbose, ("CSearch::RemovePage [%d] - enter.", Page));

    StartWriting();
    if (m_PagesToRemove.FindSortedElt(Page)!=-1)
        PurgeBadPages();
    
    m_PagesToRemove.AddSortedUnique(Page);    
    
    if (m_PagesToRemove.GetSize()>100)
        PurgeBadPages();
    StopWriting();

    Trace(tagSearch, levVerbose, ("CSearch::RemovePage [%d] - leave.", Page));
}


void CSearch::RemovePageNonWriteLocked(int Page)
{    
    Trace(tagSearch, levVerbose, ("CSearch::RemovePageNonWriteLocked [%d] - enter.", Page));

    if (m_PagesToRemove.FindSortedElt(Page) != -1)
        PurgeBadPages();    
    
    m_PagesToRemove.AddSortedUnique(Page);    
    
    if (m_PagesToRemove.GetSize() > 100)
        PurgeBadPages();

    Trace(tagSearch, levVerbose, ("CSearch::RemovePageNonWriteLocked [%d] - leave.", Page));
}


/*
* This method removes pages from the index PERMANENTLY, the pages COMPLETELY disappear
*/
void CSearch::RemovePermanently(const  int Page)
{
	long nPTRPSize = 0;

    Trace(tagSearch, levVerbose, ("CSearch::RemovePermanently [%d] - enter.", Page));

	StartWriting();

    m_PagesToRemovePermanently.AddSortedUnique(Page);
	
	nPTRPSize = m_PagesToRemovePermanently.GetSize();
	
    if (nPTRPSize > 50)
        PurgeBadPages();

	StopWriting();

    Trace(tagSearch, levVerbose, ("CSearch::RemovePermanently [%d] - leave.", Page));    
}

/*
* Force cleaning of the index, removing pages permanently and old reindexed pages data
* Calls PurgeBadPages(), done this way so that PurgeBadPages can be called from other places
* without the need to put RWMutexes in PurgeBadPages()
*/
void CSearch::ForcePurgeOfBadPages()
{
    Trace(tagSearch, levVerbose, ("CSearch::ForcePurgeOfBadPages - enter."));
    StartWriting();
    PurgeBadPages();
    StopWriting();
    Trace(tagSearch, levVerbose, ("CSearch::ForcePurgeOfBadPages - leave."));
}

/*
* Clean the index from old pages and removed pages, this method MUST NOT BE CALLED DIRECTLY
* IT NEEDS TO BE PROTECTED BY RWMUTEXES !!!
*/
void CSearch::PurgeBadPages()
{
    Trace(tagSearch, levVerbose, ("CSearch::PurgeBadPages - enter."));

    unsigned int i;
    // _L_DEBUG(4, cout<< "CSearch :: Purging bad pages: [" << m_PagesToRemove << "] and permanently: [" << m_PagesToRemovePermanently << "]" << endl);
    
    if ((m_PagesToRemove.GetSize()) || (m_PagesToRemovePermanently.GetSize()))
    {    
        for(i=0;i<CHARSET_SIZE;i++)
        {
            m_WordsTree[(unsigned char) i].RemovePages(m_PagesToRemove, m_PagesToRemovePermanently);
        }
        
        for(i=0;i<m_PagesToRemovePermanently.GetSize();i++)
        {
            m_AlreadyIndexedPages.UnSetBit(abs(m_PagesToRemovePermanently[i]));
        }
        
        m_PagesToRemove.RemoveAll();
        m_PagesToRemovePermanently.RemoveAll();
		RefreshNodesCache();
    }

    Trace(tagSearch, levVerbose, ("CSearch::PurgeBadPages - leave."));
}

/////////////////////         END OF            ////////////////////
/////////////////////  PAGES REMOVAL METHODS    ////////////////////

/*------------------------------------------------------------------------*/

/////////////////////                                  ////////////////////
/////////////////////  INDEXED PAGES BITSET METHODS    ////////////////////



void CSearch::MakeAlreadyIndexed(bool Verbose)
{
    if (Verbose) {
        cout << "  [calculating aim "; 
        cout.flush();
    }
    
    StartWriting();
    m_AlreadyIndexedPages.RemoveAll();    
    
    // grow the already indexed vector at most size of the url manager
    // this is always the max size of urls that we can have

    m_AlreadyIndexedPages.Grow(m_Parent->GetURLManager().GetSize());
    
    CProgress Progress(10, Verbose);
    for(unsigned int i=0;i<CHARSET_SIZE;i++)
    {
        Progress.Show(i, CHARSET_SIZE, Verbose);        
        m_WordsTree[(unsigned char) i].MakeAlreadyIndexed(&m_AlreadyIndexedPages);
    }
    StopWriting();
    
    Progress.Finish(Verbose);
    if (Verbose) 
        cout << " - " << m_AlreadyIndexedPages.GetSize() << " block items, " << m_AlreadyIndexedPages.CountBits(true) << " indexed]" << endl;
    
}

/*
* Return true if the page is in the index, or false if not
*/
bool CSearch::AlreadyIndexed(int Page) const
{
    StartReading();
    bool bValue = m_AlreadyIndexedPages.GetBit(abs(Page));
    StopReading();
    return bValue;
}



void CSearch::ClearAlreadyIndexed()
{
    StartWriting();
    m_AlreadyIndexedPages.RemoveAll();
    StopWriting();
}


void CSearch::GetAlreadyIndexedVector(CSVector& Result)
{    
    Result.SetDim(m_AlreadyIndexedPages.GetSize(),false,false);
    
    StartReading();
    
    for(int i=0;i<m_AlreadyIndexedPages.GetSize();i++)
    {
        if (m_AlreadyIndexedPages.GetBit(i))
            Result.Add(i);        
    }
    StopReading();    
}

/////////////////////         END OF                   ////////////////////
/////////////////////  INDEXED PAGES BITSET METHODS    ////////////////////

/*------------------------------------------------------------------------*/

/////////////////////                                ////////////////////
/////////////////////  METHODS TO ADD WORDS/PAGES    ////////////////////

void CSearch::AddPageIndex(const int Page, int& RealPage) 
{
    StartWriting();
    if (m_AlreadyIndexedPages.GetBit(abs(Page)))
    {
        RemovePageNonWriteLocked(Page);
        RealPage = -Page;
    }
    else 
    {
        m_AlreadyIndexedPages.SetBit(abs(Page)); 
    }
    StopWriting();  
}

void CSearch::AddPage(const CVector<CString>& Words, const int Page)
{
    
    Trace(tagSearch, levVerbose, ("CSearch::AddPage [%d]", Page));

    int nTotalNewNewWords = 0;
    int RealPage = Page;
    
    AddPageIndex(Page, RealPage);
    
    for(unsigned int i=0;i<Words.GetSize();i++)
    {
		if (Words[i].GetLength()<m_MaxWordSize)
		{
			bool bNewWord = false;
            
            Trace(tagSearch, levVerbose, ("CSearch::AddWord [%s]", Words[i].GetBuffer()));

			m_WordsTree[(unsigned char) Words[i][0]].AddWord(Words[i], RealPage, bNewWord);
			
            if (bNewWord)
                nTotalNewNewWords++;				       
		}
    }

    m_WordsCount.Inc(nTotalNewNewWords); 
}

void CSearch::AddPage(const CVector<CStringA>& Words, const int Page)
{    
    bool bNewWord;
    int RealPage = Page;
    
    AddPageIndex(Page, RealPage);
    
    for(unsigned int i=0;i<Words.GetSize();i++)
    {      
		if (Words[i].GetLength()<m_MaxWordSize)
		{
            Trace(tagSearch, levVerbose, ("CSearch::AddWord [%s]", Words[i].GetBuffer()));

			bNewWord=false;
			m_WordsTree[(unsigned char) Words[i][0]].AddWord((CString) Words[i], RealPage, bNewWord);
			if (bNewWord)
				m_WordsCount.Inc();
		}
    }
}

/*
void CSearch::AddWord(const CString& Word, const int Page)
{
bool NewWord=false;
m_WordsTree[Word[0]].AddWord(Word,Page,NewWord);
if (NewWord==true)
m_WordsCount++;
}
*/


/* 
* This method must be used ONLY WHEN LOADING THE INDEX OR MERGING DATABASES
*/
void CSearch::AddWord(const CString& Word, const CVector<int>& Pages)
{
    bool bNewWord=false;
    if ( (!Word.GetLength()) || (Word.GetLength()>=m_MaxWordSize) ) {        
        return;
    }

    Trace(tagSearch, levVerbose, ("CSearch::AddWord [%s]", Word.GetBuffer()));

    m_WordsTree[(unsigned char) Word[0]].AddWord(Word,Pages,bNewWord);
    if (bNewWord)
        m_WordsCount.Inc();
    //else cout<<"WORD: "<<Word<<" IS NOT NEW IN THE INDEX"<<endl;
}

/////////////////////            END OF              ////////////////////
/////////////////////  METHODS TO ADD WORDS/PAGES    ////////////////////

/*------------------------------------------------------------------------*/

/////////////////////                                   ////////////////////
/////////////////////  METHODS TO SEARCH WORDS/METAS    ////////////////////
bool CSearch::FindExactWord(const CVector<CStringA>& Words, CVector<CSVector> *pvResult)
{
    int NbreMots = Words.GetSize();
    pvResult->SetSize(NbreMots);
//	unsigned int j;
    for(int i=0;i<NbreMots;i++)
    {     
		if ( ((*pvResult)[i]).GetSize()!=0)
			continue;
        //      cout<<"Searched Word: "<<Words[i]<<" TagPos: "<<Words[i].GetTagPos()<<endl;
        if (Words[i].Pos('=') > 0)
        {
            m_WordsTree[(unsigned char) Words[i][0]].FindExactWordSensitive(Words[i], (*pvResult)[i] );
        }
        else
            if (Words[i].Pos('>') > 0)
            {        
                CString NewWord=Words[i];
                NewWord[NewWord.Pos('>')]='=';
                m_WordsTree[(unsigned char) Words[i][0]].FindSensitiveGreaterThan(NewWord, (*pvResult)[i] );
            }
            else if (Words[i].Pos('<') > 0)
            {
                CString NewWord=Words[i];
                NewWord[NewWord.Pos('<')]='=';
                m_WordsTree[(unsigned char) Words[i][0]].FindSensitiveLowerThan(NewWord, (*pvResult)[i] );
            }
            else
                if (Words[i] == (CStringA)g_strStar)
                {
                    GetAlreadyIndexedVector( (*pvResult)[i]);
                }
                else
                {
                    if (Words[i].GetTagPos()==0)
					{
                        m_WordsTree[(unsigned char) Words[i][0]].FindExactWordSensitive(Words[i], (*pvResult)[i] );
					}
                    else
					{
                        m_WordsTree[(unsigned char) Words[i][0]].FindExactMetaSensitive(Words[i], (*pvResult)[i] );
					}
                }
    }
    return true;
}

bool CSearch::FindInsensExactWord(const CVector<CStringA>& Words, CVector<CSVector> *pvResult)
{

    Trace(tagSearch, levVerbose, ("CSearch::FIEW - entering with %d items.", Words.GetSize()));

    unsigned char chReal, chLower, chUpper;    
    int NbreMots = Words.GetSize();
//	unsigned int j;
    pvResult->SetSize(NbreMots);
    for(int i = 0; i < NbreMots; i++)
    {
		if ( ((*pvResult)[i]).GetSize()!=0)
			continue;
        chReal = Words[i][0];
        chLower = CString::LCase(chReal);
        chUpper = CString::UCase(chReal);
        
        //     cout<<"Searched Word: "<<Words[i]<<" TagPos: "<<Words[i].GetTagPos()<<endl;
        
        if (Words[i].Pos('=') > 0)
        {
            m_WordsTree[chLower].FindExactWordInsensitive(Words[i], (*pvResult)[i] );
            m_WordsTree[chUpper].FindExactWordInsensitive(Words[i], (*pvResult)[i] );
        }
        else
        {
            if (Words[i].Pos('>') > 0)
            {        
                CString NewWord=Words[i];
                NewWord[NewWord.Pos('>')]='=';
                m_WordsTree[chLower].FindInsensitiveGreaterThan(NewWord, (*pvResult)[i] );
                m_WordsTree[chUpper].FindInsensitiveGreaterThan(NewWord, (*pvResult)[i] );
            }
            else if (Words[i].Pos('<') > 0)
            {
                CString NewWord=Words[i];
                NewWord[NewWord.Pos('<')]='=';
                m_WordsTree[chLower].FindInsensitiveLowerThan(NewWord, (*pvResult)[i] );
                m_WordsTree[chUpper].FindInsensitiveLowerThan(NewWord, (*pvResult)[i] );
            }
            else
            {
                if (Words[i].GetTagPos() == 0)
                {
                    m_WordsTree[chLower].FindExactWordInsensitive(Words[i], (*pvResult)[i] );
                    m_WordsTree[chUpper].FindExactWordInsensitive(Words[i], (*pvResult)[i] );
                }
                else
                {
                    m_WordsTree[chReal].FindExactMetaInsensitive(Words[i], (*pvResult)[i] );
                }
            }
        }
    }

    Trace(tagSearch, levVerbose, ("CSearch::FIEW - leaving with %d results.", pvResult->GetSize()));

    return true;
}

bool CSearch::FindPartialWord(const CVector<CStringA>& Words, CVector<CSVector> *pvResult)
{

    Trace(tagSearch, levVerbose, ("CSearch::FPW - entering with %d items.", Words.GetSize()));

    unsigned char chReal;
    int NbreMots = Words.GetSize();
//	unsigned int j;
    pvResult->SetSize(NbreMots);
    for(int i=0; i< NbreMots; i++)
    {
		if ( ((*pvResult)[i]).GetSize()!=0)
			continue;
        //     cout<<"Searched Word: "<<Words[i]<<" TagPos: "<<Words[i].GetTagPos()<<endl;
        chReal = Words[i][0];
        
        if (Words[i].Pos('=') > 0)
        {
            m_WordsTree[chReal].FindExactWordSensitive(Words[i], (*pvResult)[i] );
        }
        else
        {
            if (Words[i].Pos('>') > 0)
            {        
                CString NewWord = Words[i];
                NewWord[NewWord.Pos('>')] = '=';
                m_WordsTree[chReal].FindSensitiveGreaterThan(NewWord, (*pvResult)[i] );
            }
            else if (Words[i].Pos('<')>0)
            {
                CString NewWord=Words[i];
                NewWord[NewWord.Pos('<')]='=';
                m_WordsTree[chReal].FindSensitiveLowerThan(NewWord, (*pvResult)[i] );
            }
            else
            {
                if (Words[i].GetTagPos()==0)
                {
                    m_WordsTree[chReal].FindPartialWordSensitive(Words[i], (*pvResult)[i] );
                }
                else
                {
                    m_WordsTree[chReal].FindPartialMetaSensitive(Words[i], (*pvResult)[i] );
                }
            }
        }
    }

    Trace(tagSearch, levVerbose, ("CSearch::FPW - leaving with %d results.", pvResult->GetSize()));

    return true;
}

bool CSearch::FindPartSensitive(const CVector<CStringA>& Words, CVector<CSVector> *pvResult)
{    
    Trace(tagSearch, levVerbose, ("CSearch::FPS - entering with %d items.", Words.GetSize()));

    unsigned char chReal;
    int NbreMots = Words.GetSize();
    pvResult->SetSize(NbreMots);
    unsigned int j;
    int i;
    for(i = 0; i < NbreMots; i++)
    {
		if ( ((*pvResult)[i]).GetSize()!=0)
			continue;
        chReal = Words[i][0];        
        //cout<<"In FindPartSensitive for "<<Words[i]<<" "<<Words[i].GetTagPos()<<endl;
        //     cout<<"Searched Word: "<<Words[i]<<" TagPos: "<<Words[i].GetTagPos()<<endl;
        if (Words[i].Pos('=')>0)
        {
            m_WordsTree[chReal].FindExactWordSensitive(Words[i], (*pvResult)[i] );
        }
        else
        {
            if (Words[i].Pos('>')>0)
            {        
                CString NewWord=Words[i];
                NewWord[NewWord.Pos('>')]='=';
                m_WordsTree[chReal].FindSensitiveGreaterThan(NewWord, (*pvResult)[i] );
            }
            else if (Words[i].Pos('<')>0)
            {
                CString NewWord=Words[i];
                NewWord[NewWord.Pos('<')]='=';
                m_WordsTree[chReal].FindSensitiveLowerThan(NewWord, (*pvResult)[i] );
            }
            else if (Words[i].GetTagPos()==0)
            {
				CLetter *CurrentLetter;
				int LetterPos=0;
				unsigned int PrevResultsSize=0;

				CVector<CSwapVector*> ResultVectors;
				CSort ResSorter;
				m_AllLetters.StartReading(chReal);
				for(j=0;j<m_AllLetters.GetSize(chReal);j++)
				{
					LetterPos=-1;
					CurrentLetter=m_AllLetters.ValueNonLocked(chReal,j);
					while( (CurrentLetter->m_Content[LetterPos]!=chReal) && (LetterPos<(int)CurrentLetter->m_ContentSize) )
					{
						LetterPos++;
						PrevResultsSize=ResultVectors.GetSize();
//							cout<<endl<<"2 Trying word: "<<CurrentLetter->OwnWord()<<" Start: "<<LetterPos<<endl;
						CurrentLetter->FindPartialWordInsensitive(Words[i],0,LetterPos,ResultVectors);
						if (ResultVectors.GetSize()!=PrevResultsSize)
							break;
					}
				}
				m_AllLetters.StopReading(chReal);
				ResSorter.SetVectors(ResultVectors);
				ResSorter.AddVector(&((*pvResult)[i]));
				(*pvResult)[i]=ResSorter.GetSortedSwapVector();
            }
            else
            {
                m_WordsTree[chReal].FindPartMetaSensitive(Words[i], (*pvResult)[i] );
            }
        }
    }

    Trace(tagSearch, levVerbose, ("CSearch::FPS - leaving with %d results.", pvResult->GetSize()));

    return true;
}


bool CSearch::FindPartInsensitive(const CVector<CStringA>& Words, CVector<CSVector> *pvResult)
{
    Trace(tagSearch, levVerbose, ("CSearch::FPI - entering with %d items.", Words.GetSize()));

    unsigned char chReal, chLower, chUpper;    
    int NbreMots = Words.GetSize();
    pvResult->SetSize(NbreMots);
    unsigned int j;
    int i;
    for(i = 0; i < NbreMots; i++)
    {
		if ( ((*pvResult)[i]).GetSize()!=0)
			continue;
        chReal = Words[i][0];
        chLower = CString::LCase(chReal);
        chUpper = CString::UCase(chReal);
        
//        cout<<"In FindPartInsensitive for "<<Words[i]<<" "<<Words[i].GetTagPos()<<endl;
//        cout<<"Searched Word: "<<Words[i]<<" TagPos: "<<Words[i].GetTagPos()<<endl;
        
        if (Words[i].Pos('=')>0)
        {
            m_WordsTree[chLower].FindExactWordInsensitive(Words[i], (*pvResult)[i] );
            m_WordsTree[chUpper].FindExactWordInsensitive(Words[i], (*pvResult)[i] );
        }
        else
        {
            if (Words[i].Pos('>')>0)
            {        
                CString NewWord=Words[i];
                NewWord[NewWord.Pos('>')]='=';
                m_WordsTree[chLower].FindInsensitiveGreaterThan(NewWord, (*pvResult)[i] );
                m_WordsTree[chUpper].FindInsensitiveGreaterThan(NewWord, (*pvResult)[i] );
            }
            else if (Words[i].Pos('<')>0)
            {
                CString NewWord=Words[i];
                NewWord[NewWord.Pos('<')]='=';
                m_WordsTree[chLower].FindInsensitiveLowerThan(NewWord, (*pvResult)[i] );
                m_WordsTree[chUpper].FindInsensitiveLowerThan(NewWord, (*pvResult)[i] );
            }
            else
            {
                if (Words[i].GetTagPos()==0)
                {

					CLetter *CurrentLetter;
					int LetterPos=0;
					unsigned int PrevResultsSize=0;

					CVector<CSwapVector*> ResultVectors;
					CSort ResSorter;

					m_AllLetters.StartReading(chLower);
					for(j=0;j<m_AllLetters.GetSize(chLower);j++)
					{
						LetterPos=-1;
						CurrentLetter=m_AllLetters.ValueNonLocked(chLower,j);

						while(LetterPos<(int)CurrentLetter->m_ContentSize) 
						{
							LetterPos++;
							if (CurrentLetter->m_Content[LetterPos]==chLower)
							{
								PrevResultsSize=ResultVectors.GetSize();
//								cout<<endl<<"2 Trying word: "<<CurrentLetter->OwnWord()<<" Start: "<<LetterPos<<endl;
								CurrentLetter->FindPartialWordInsensitive(Words[i],0,LetterPos,ResultVectors);
								if (ResultVectors.GetSize()!=PrevResultsSize)
									break;
							}
						}

					}
					m_AllLetters.StopReading(chLower);
//					ResSorter.SetVectors(ResultVectors);
//					ResSorter.AddVector(&((*pvResult)[i]));
//					(*pvResult)[i]=ResSorter.GetSortedSVector();


					m_AllLetters.StartReading(chUpper);
					for(j=0;j<m_AllLetters.GetSize(chUpper);j++)
					{
						LetterPos=-1;
						CurrentLetter=m_AllLetters.ValueNonLocked(chUpper,j);

						while( (CurrentLetter->m_Content[LetterPos]!=chUpper) && (LetterPos<(int)CurrentLetter->m_ContentSize) )
						{
							LetterPos++;
							PrevResultsSize=ResultVectors.GetSize();
//							cout<<endl<<"4 Trying word: "<<CurrentLetter->OwnWord()<<" Start: "<<LetterPos<<endl;
							CurrentLetter->FindPartialWordInsensitive(Words[i],0,LetterPos,ResultVectors);
							if (ResultVectors.GetSize()!=PrevResultsSize)
								break;
						}
					}
					m_AllLetters.StopReading(chUpper);
					ResSorter.SetVectors(ResultVectors);
					ResSorter.AddVector(&((*pvResult)[i]));
					(*pvResult)[i]=ResSorter.GetSortedSwapVector();

                }
                else
                {
                    m_WordsTree[chReal].FindPartMetaInsensitive(Words[i], (*pvResult)[i] );
                }
            }
        }
    }

    Trace(tagSearch, levVerbose, ("CSearch::FPI - leaving with %d results.", pvResult->GetSize()));

    return true;
}

bool CSearch::FindInsensPartialWord(const CVector<CStringA>& Words, CVector<CSVector> *pvResult)
{
    Trace(tagSearch, levVerbose, ("CSearch::FIPW - entering with %d items.", Words.GetSize()));

    unsigned char chReal, chLower, chUpper;    
    int NbreMots = Words.GetSize();
//	unsigned int j;
    pvResult->SetSize(NbreMots);
    for(int i=0;i<NbreMots;i++)
    {
		if ( ((*pvResult)[i]).GetSize()!=0)
			continue;

        chReal = Words[i][0];
        chLower = CString::LCase(chReal);
        chUpper = CString::UCase(chReal);
        
//        cout<<"Searched Word: "<<Words[i]<<" TagPos: "<<Words[i].GetTagPos()<<endl;
        if (Words[i].Pos('=') > 0)
        {
            m_WordsTree[chLower].FindExactWordInsensitive(Words[i], (*pvResult)[i] );
            m_WordsTree[chUpper].FindExactWordInsensitive(Words[i], (*pvResult)[i] );
        }
        else
        {
            if (Words[i].Pos('>')>0)
            {        
                CString NewWord=Words[i];
                NewWord[NewWord.Pos('>')]='=';
                m_WordsTree[chLower].FindInsensitiveGreaterThan(NewWord, (*pvResult)[i] );
                m_WordsTree[chUpper].FindInsensitiveGreaterThan(NewWord, (*pvResult)[i] );
            }
            else if (Words[i].Pos('<')>0)
            {
                CString NewWord=Words[i];
                NewWord[NewWord.Pos('<')]='=';
                m_WordsTree[chLower].FindInsensitiveLowerThan(NewWord, (*pvResult)[i] );
                m_WordsTree[chUpper].FindInsensitiveLowerThan(NewWord, (*pvResult)[i] );
            }
            else
            {
                if (Words[i].GetTagPos()==0)
                {
                    m_WordsTree[chLower].FindPartialWordInsensitive(Words[i], (*pvResult)[i] );
                    m_WordsTree[chUpper].FindPartialWordInsensitive(Words[i], (*pvResult)[i] );

                }
                else
                {
                    m_WordsTree[chReal].FindPartialMetaInsensitive(Words[i], (*pvResult)[i] );    
                }
            }
        }
    }

    Trace(tagSearch, levVerbose, ("CSearch::FIPW - leaving with %d results.", pvResult->GetSize()));

    return true;
}


/////////////////////              END OF               ////////////////////
/////////////////////  METHODS TO SEARCH WORDS/METAS    ////////////////////

/*------------------------------------------------------------------------*/


/* OLD VERSION
int CSearch::GetQuality(CSearchData& Data, int Index) {    
    if (!Data.m_ResultsQuality.GetSize()) return 0;
    unsigned int i;
    int TotalResults=0;
    float NotWords=0;
    
    for(i=0;i<Data.m_Words.GetSize();i++) {
        if (Data.m_Words[i].GetLength() && (Data.m_Words[i][0] == '-'))
            NotWords++;        
    }
    
    float RealWords=Data.m_SearchedWords-NotWords;
    
    int Sum=Data.m_ResultsQuality[Data.m_ResultsQuality.GetSize()-1];
    for( i=Data.m_ResultsQuality.GetSize()-1;i>0;i--) {
        if (Index>Sum-1) Sum+=Data.m_ResultsQuality[i];
        else break; 
    }
    float WordsCount=(float) i+1;
    for( i=0;i<Data.m_ResultsQuality.GetSize();i++) {
        TotalResults+=Data.m_ResultsQuality[i];
    }
    srand(TotalResults+Index); 
    float Random=(float) (0.5*rand()/(RAND_MAX+1.0));
    float Fragment=(66000/(float)TotalResults);
    srand(TotalResults);
    float SecondRandom=(float)(5*(rand()/(RAND_MAX+1.0)));
    float ReturnValue=(float)( Fragment*(TotalResults-Index) + Fragment*Random )/1000+1;
    float Better=WordsCount-((float)RealWords-NotWords);
    if (Better>0) ReturnValue*=(1+(Better/10));
    ReturnValue+=SecondRandom;    
    ReturnValue-=(float) (6*SecondRandom* ( (float)(Index - Index%5 ) / (float)TotalResults ));
    if (ReturnValue>100) ReturnValue=100;
    if (ReturnValue<1) ReturnValue=1; 
    return (int)ReturnValue;
}


*/


int CSearch::GetQuality(CSearchData& Data, int Index) {    

    Trace(tagSearch, levVerbose, ("CSearch::GetQuality - index %d.", Index));

    int Divisor=5;	
	int GlobalQuality=0;
	bool Finished=false;
	int Step=0;

	int WeakPoints=10;
	if (Data.m_Results.GetSize()>5000)
		WeakPoints=20;
	if (Data.m_Results.GetSize()>10000)
		WeakPoints=30;

	WeakPoints=(int) ( (float)WeakPoints*( (float)Index /  (float)Data.m_Results.GetSize() ) );
	if (WeakPoints>10) 
		WeakPoints=10;
	float Proportion;
	Proportion=(float)100/(float)Data.m_MaxPossibleQuality;

	if (0==Index) 
		return (int)((Data.m_ResultsQuality[0]-Data.m_QualityThreshold)*Proportion);

	int PosCounter=Index;
	while(!Finished)
	{
		PosCounter--;
		Step++;
		GlobalQuality+=(Data.m_ResultsQuality[PosCounter]-Data.m_QualityThreshold);
		Divisor--;
		if ( (0==PosCounter) || (Index-PosCounter>2) )
			Finished=true;
	}

	GlobalQuality/=Step;
	GlobalQuality-=(Data.m_ResultsQuality[Index]-Data.m_QualityThreshold);
	GlobalQuality/=Divisor;


//Proportion=1;
//	printf("Index : %d Value : %d Proportion: %f MaxQuality : %d\n",Index,GlobalQuality, Proportion, Data.m_MaxPossibleQuality);
    int ReturnValue=(int)((Data.m_ResultsQuality[Index]-Data.m_QualityThreshold+GlobalQuality)*Proportion)-WeakPoints;
	if (ReturnValue<=0) ReturnValue=1;
//	printf("Return : %d InitialQuality: %d\n",ReturnValue,(Data.m_ResultsQuality[Index]-Data.m_QualityThreshold));
	return ReturnValue;
}



/* Lookup a word in one of the cache levels, if not found, add it to cache */
bool CSearch::FindWithCache(const CVector<CStringA>& Words, CCache& Cache, CSearchType SearchType, CVector<CSVector> * pvResult) 
{
    Trace(tagSearch, levVerbose, ("CSearch::FindWithCache - %d words.", Words.GetSize()));

    assert(pvResult);
    pvResult->SetSize(Words.GetSize());

	CVector<int> WordsFoundInCache;
	WordsFoundInCache.SetSize(Words.GetSize());

    // _L_DEBUG(4, cout << "CSearch ::  FindWithCache() - " << Words.GetSize() << " words." << endl);
    CVector<CStringA> WordsTmp(Words);
    int i,j, WordsGetSize = WordsTmp.GetSize();
        
    for (i = WordsTmp.GetSize()-1; i>= 0; i--) 
    {

        if (Cache.Find(Words[i], (* pvResult)[i])) 
        {            
            Trace(tagSearch, levVerbose, ("CSearch::FindWithCache - found [%s] in cache with %d result arrays.", Words[i].GetBuffer(), (* pvResult)[i].GetSize()));
			WordsFoundInCache[i]=1;
            WordsGetSize--;
        } 
        else
        {
			WordsFoundInCache[i]=0;
            Trace(tagSearch, levVerbose, ("CSearch::FindWithCache - couldn't find [%s] in cache.", Words[i].GetBuffer()));
        }
        
    }
    
	//The Find<bla> methods rely on the size of pvResult[i] being different from 0 to discern words which were not found in the cache
    if (WordsGetSize) 
    {
        StartReading();

        switch(SearchType) 
        {
        case st_Exact:
            FindExactWord(WordsTmp, pvResult);
            break;
        case st_InsensExact:
            FindInsensExactWord(WordsTmp, pvResult);
            break;
        case st_Partial:
            if (false==m_RightPartialOnly)
                FindPartSensitive(WordsTmp, pvResult);
            else
                FindPartialWord(WordsTmp, pvResult);
            break;
        case st_InsensPartial:
            if (false==m_RightPartialOnly)
                FindPartInsensitive(WordsTmp, pvResult);
            else
                FindInsensPartialWord(WordsTmp, pvResult);
            break;
        default:
            break;
        }
        StopReading();
        
		for (i = Words.GetSize()-1;i>= 0;i--)
		{
			if (Words[i].GetTagPos()!=0)
			{
				for(j = Words.GetSize()-1;j>= 0;j--)
				{//if word[i] is a meta, and word[j] is the same word but NOT in a meta, and word[j] does not come from the cache, then add the results 
					//of the meta to the results of the word
					if ( (Words[j].GetTagPos()==0) && (WordsFoundInCache[j]==0) && (((CString)Words[i]).Compare(Words[j],Words[i].GetTagPos()+1,false)==0) )
					{
						//cout<<"Word "<<Words[j]<<" is the base word for Word :"<<Words[i]<<endl;
						//cout<<"Init: "<<(*pvResult)[j].GetSize()<<" Other : "<<(*pvResult)[i].GetSize()<<endl;
						((* pvResult)[j]).Add(((* pvResult)[i]));
						//cout<<"Total :"<<(*pvResult)[j].GetSize()<<endl;
					}
				}
			}
		}




        for (i = WordsTmp.GetSize()-1;i>= 0;i--) 
        {
            if (WordsTmp[i].GetLength()) 
            {        
                Cache.Set(WordsTmp[i], (* pvResult)[i]);
            }
        }  
    }

    Trace(tagSearch, levVerbose, ("CSearch::FindWithCache - %d results.", pvResult->GetSize()));

    for (i = 0; i < (int) pvResult->GetSize(); i++) 
	{
        Trace(tagSearch, levVerbose, ("CSearch::FindWithCache - [%d] result arrays.", (* pvResult)[i].GetSize()));
    }

    return true;
}

unsigned short int CSearch::GetWeight(const CString& Word)
{
    Trace(tagSearch, levVerbose, ("CSearch::GetWeight - [%s].", Word.GetBuffer()));

	int MetaPos=Word.Pos(':');
	if (MetaPos<0) MetaPos=-1;
	CString TempWord;
	Word.Mid(MetaPos+1,&TempWord);
//	cout<<"WORD : " << TempWord<<endl;
	TempWord.LowerCase();
	if (m_WeakWords.FindSortedElt(TempWord)>-1)
		return 1;
	if (Word.StartsWith("TITLE:"))
		return GetTitleWeight();
	else if (Word.StartsWith("KEYWORDS:"))
		return GetKeywordWeight();
	else if (Word.StartsWith("DESCRIPTION:"))
		return GetDescriptionWeight();
	else return GetTextWeight();

}

/* find */
bool CSearch::Find(CSearchData& Data) 
{

    Trace(tagSearch, levVerbose, ("CSearch::Find - entering."));

    InvalidateCache(m_SearchCacheLife);

    register int i;
    
    int nbreAND = 0, nbreNOT = 0;
    
    // _L_DEBUG(4, cout << "CSearch :: Find() - entering." << endl);
    
    CVector<CStringA> Word2;    
    Word2.SetDim(Data.m_Words.GetSize());

    CVector<CVectorInt> WhichTable;    
    WhichTable.SetSize(Data.m_Words.GetSize());
    
	CVector<unsigned short int> WordsWeight;

    int NbreMots = Data.m_Words.GetSize();
    CVector<CSVector> Results;
    CVector<CSVector> TempRes1, TempRes2, TempRes3, TempRes4 ;
    CVector<CStringA> WordSensExact, WordInsensExact, WordSensPartial, WordInsensPartial;
    bool Quoted = false;
    CString MidString;
    
    for(i = 0;i<(int)Data.m_Words.GetSize();i++ ) 
    {
        
        // _L_DEBUG(4, cout << "CSearch :: Find() - parsing " << Data.m_Words[i] << endl);
//        cout<<"SEARCHED WORD : "<<Data.m_Words[i]<<endl;
        if (Data.m_Words[i].GetLength())
            switch(Data.m_Words[i][0]) 
            {
            case '+':
                Data.m_Words[i].Mid(1, &MidString);
                Word2.InsertAt(0, MidString);
                nbreAND++;
                break;
            case '-':
                Data.m_Words[i].Mid(1, &MidString);
                Word2.InsertAt(nbreAND,MidString);
                nbreNOT++;
                break;
            default:
                Word2 += Data.m_Words[i];
                break;
            }
        
        // _L_DEBUG(4, cout << "CSearch :: Find() - done parsing " << Data.m_Words[i] << endl);        
    }
    
    // _L_DEBUG(4, cout << "CSearch :: Find() - expanding and unquoting.");
    
    assert((int)  Word2.GetSize() == NbreMots);
    
    for(i = 0; i < NbreMots; i++) 
    {
        CStringA WordCopy(Word2[i]);
        Dequote(WordCopy);
        
        if (WordCopy != Word2[i]) 
            Quoted = true;
        else Quoted = false;
 
        if (WordCopy.GetFakeSize() <= m_ExactSize) 
        {
            // When the word is very small, force the search to be exact
            WordSensExact.Add(WordCopy);
            WhichTable[i] += 1;
            WhichTable[i] += WordSensExact.GetSize()-1;
			WordsWeight.Add(GetWeight(WordCopy));
        }
        else if (
            // case-insensitive search forced ?
            (Data.m_SearchCaseType != sct_CaseInsensitive)
            && (
                (Data.m_SearchCaseType == sct_CaseSensitive) || 
                (WordCopy.HasUpperCase(WordCopy.GetTagPos()))
                )
            )
        {
            // Cases where search must be case sensitive
            if (Quoted) 
            {
                WordSensExact.Add(WordCopy);
                WhichTable[i] += 1;
                WhichTable[i] += WordSensExact.GetSize()-1;
				WordsWeight.Add(GetWeight(WordCopy));
            }
            else 
            {
                WordSensPartial.Add(WordCopy);
                WhichTable[i] += 2;
                WhichTable[i] += WordSensPartial.GetSize()-1;
				WordsWeight.Add(GetWeight(WordCopy));
            }
        }
        else 
        {
            if (Quoted) 
            {
                WordInsensExact.Add(WordCopy);
                WhichTable[i] += 3;
                WhichTable[i] += WordInsensExact.GetSize()-1;
				WordsWeight.Add(GetWeight(WordCopy));
            }
            else 
            {
                WordInsensPartial.Add(WordCopy);
                WhichTable[i] += 4;
                WhichTable[i] += WordInsensPartial.GetSize()-1;
				WordsWeight.Add(GetWeight(WordCopy));
            }
        }
    }
    
    // _L_DEBUG(4, cout << "CSearch :: Find() - searching.");
    
	// _L_DEBUG(4,cout<<"Starting to search words"<<endl);
	// _L_DEBUG(4,cout.flush());

    Trace(tagSearch, levVerbose, ("CSearch::Find - Starting searching words."));

    if (WordInsensPartial.GetSize()) 
    {
        // _L_DEBUG(4, cout << "CSearch :: FindInsensPartialWord() - ");
        // _L_DEBUG(4, cout.flush());
        FindWithCache(WordInsensPartial, m_CacheInsensPartial, st_InsensPartial, &TempRes4);
        // _L_DEBUG(4, cout << "(done)" << endl);
    }
    
    if (WordInsensExact.GetSize()) 
    {
        // _L_DEBUG(4, cout << "CSearch :: FindInsensExactWord() - ");
        // _L_DEBUG(4, for (i = 0;i<(int) WordInsensExact.GetSize();i++ ) cout << "[" << WordInsensExact[i] << "]");
        // _L_DEBUG(4, cout.flush());
        FindWithCache(WordInsensExact, m_CacheInsensExact, st_InsensExact, &TempRes3);
        // _L_DEBUG(4, cout << "(done)" << endl);
    }
    
    if (WordSensPartial.GetSize()) 
    {
        // _L_DEBUG(4, cout << "CSearch :: FindSensPartialWord() - ");
        // _L_DEBUG(4, for (i = 0;i<(int) WordSensPartial.GetSize();i++ ) cout << "[" << WordSensPartial[i] << "]");
        // _L_DEBUG(4, cout.flush());
        FindWithCache(WordSensPartial, m_CachePartial, st_Partial, &TempRes2);
        // _L_DEBUG(4, cout << "(done)" << endl);
    }
    
    if (WordSensExact.GetSize()) 
    {
        // _L_DEBUG(4, cout << "CSearch :: FindSensExactWord() - ");
        // _L_DEBUG(4, for (i = 0;i<(int) WordSensExact.GetSize();i++ ) cout << "[" << WordSensExact[i] << "]");
        // _L_DEBUG(4, cout.flush());
        FindWithCache(WordSensExact, m_CacheExact, st_Exact, &TempRes1);
        // _L_DEBUG(4, cout << "(done)" << endl);
    }

    Trace(tagSearch, levVerbose, ("CSearch::Find - Finished searching words."));

	// _L_DEBUG(4,cout<<"Starting to sort Vectors"<<endl);
	// _L_DEBUG(4,cout.flush());

	CRank Ranker;

	bool Mandatory, ShouldBeMissing;

    for(i = 0;i<NbreMots;i++ ) 
    {
		Mandatory=false;
		ShouldBeMissing=false;

		if (i<nbreAND) 
			Mandatory=true;
		else if (i<nbreAND+nbreNOT) 
			ShouldBeMissing=true;

		//cout<<"WEIGHT FOR "<<Word2[i]<<" is ["<<WordsWeight[i]<<"]["<<Mandatory<<"]["<<ShouldBeMissing<<"]"<<endl;
        if (WhichTable[i][0] == 1) 
			Ranker.AddVector(&(TempRes1[WhichTable[i][1]]),WordsWeight[i],Mandatory,ShouldBeMissing);
        else if (WhichTable[i][0] == 2) 
			Ranker.AddVector(&(TempRes2[WhichTable[i][1]]),WordsWeight[i],Mandatory,ShouldBeMissing);
        else if (WhichTable[i][0] == 3) 
			Ranker.AddVector(&(TempRes3[WhichTable[i][1]]),WordsWeight[i],Mandatory,ShouldBeMissing);
        else if (WhichTable[i][0] == 4)
			Ranker.AddVector(&(TempRes4[WhichTable[i][1]]),WordsWeight[i],Mandatory,ShouldBeMissing);
    } 

    Ranker.GetSortedVector(&Data);

    Trace(tagSearch, levVerbose, ("CSearch::Find - Finished sorting results."));

	Data.m_MaxPossibleQuality = 0;

	// _L_DEBUG(4,cout<<"Finished sorting vectors"<<endl);
	// _L_DEBUG(4,cout.flush());

	for(i=0 ; i< (int)WordsWeight.GetSize();i++)
		Data.m_MaxPossibleQuality += WordsWeight[i];

    /* original positions of results (displaced at sorting) */
    Data.m_ResultsPositions.SetSize(Data.m_Results.GetSize());
    for (i=0;i<(int) Data.m_Results.GetSize();i++) 
        Data.m_ResultsPositions[i] = i;
    
    // _L_DEBUG(4, cout << "CSearch :: Find() - leaving.");

    Trace(tagSearch, levVerbose, ("CSearch::Find - Finished doing misc work."));

    return true;
}

long CSearch::GetDataSize(void) const
{
	long lFileSize = 0;

	lFileSize = 
		1 + // m_Endian
		sizeof(int) * CHARSET_SIZE; // words count for each node

	for(unsigned int i = 0; i < CHARSET_SIZE; i++)
    {
		if (m_WordsTree[(unsigned char) i].GetWordsCount())
		{
			lFileSize += m_WordsTree[(unsigned char) i].GetDataSize();
		}
	}

	return lFileSize;
}

bool CSearch::WriteIndex(const CString& FileName, bool Verbose) 
{ 
    bool bError = false;
    unsigned int i;    
	int Count = 0;
	int DataSize;
    

    PurgeBadPages();

	for(i = 0; i < CHARSET_SIZE; i++)
		m_WordsTree[(unsigned char) i].StartReading();
		

	DataSize = GetDataSize();
    
    if (Verbose) {
		cout << "[" << DataSize << " byte(s)]";
		cout.flush();
	}


    CMMapFile IndexMappedFile(FileName);
	
	if (! IndexMappedFile.MMap(MMAP_OPENMODE, DataSize)) {
		cerr << "[mmap failed]" << endl;
		return false;
	}

    CProgress Progress(10, Verbose);

    
    if (! IndexMappedFile.Write(& m_Endian, 1))
    {
		cerr << "[endian write failed]" << endl;
		bError = true;
    }

	for(i = 0; i < CHARSET_SIZE; i++)
	{
		if (bError)
			break;
		Count = (int) m_WordsTree[(unsigned char) i].GetWordsCount();
		if (! IndexMappedFile.Write(& Count, sizeof(int))) 
		{
			cerr << "[tree count write failed]" << endl;
			bError = true;
			break;
		}
		if (Count) 
		{
			if (! m_WordsTree[(unsigned char) i].Serialize(IndexMappedFile)) 
			{
				cerr << "[serialize failed at index " << (int) i << ", count=" << Count << "]" << endl;
				bError = true;
				break;
			}
		}
		Progress.Show(i, CHARSET_SIZE, Verbose);
	}

	for(i = 0; i < CHARSET_SIZE; i++)
		m_WordsTree[(unsigned char) i].StopReading();

	Progress.Finish(Verbose);
    return ! bError;
}

bool CSearch::LoadIndex_1_4(const CString& FileName, bool Verbose, bool bMakeAlreadyIndexed) 
{
	bool ReturnValue=true;
	unsigned char IndexEndian=6;
    unsigned char EndianChange=0;
//    cout<<"In LoadIndex_1_4"<<endl;
	CSVector TempVector;
    TempVector.SetDim(50000,false);

    FILE *IndexFile = fopen((const char *)FileName.GetBuffer(), "rb");
    if (IndexFile) 
    {
        fread(&IndexEndian,1,1,IndexFile);
        
        if ( (IndexEndian!=0) && (IndexEndian!=1) )
        {cout<<"Corrupted index or not a siteidx2.ndx file"<<endl;return false;}
        if (IndexEndian!=m_Endian) 
        {
            cout << "[byte order changed, converting]";
            EndianChange = 1;
        }
        
        CProgress Progress(10, Verbose);

        for(unsigned int i=0;i<CHARSET_SIZE;i++)
        {
            if (g_pHandler->GetSignalSigterm())
            return false;
    
            int Count = 0, CountTemp;
            fread(&Count,1,sizeof(int),IndexFile);
            if (EndianChange == 1) 
            {
                CountTemp = Count;
                Count = Flip_int32(CountTemp);
            }
            CString CompleteWord="";
            if (Count>0)
                if (LoadNode(IndexFile, CompleteWord,EndianChange,TempVector)==false)
					ReturnValue=false;
            Progress.Show(i, CHARSET_SIZE, Verbose);
//            cout<<"Loading letter: ["<<(char)i<<"]"<<endl;
        }
        Progress.Finish(Verbose);

		if (Verbose) {
			cout << "[" << m_WordsCount.Get() << " words]" << endl;
		}

		if (bMakeAlreadyIndexed)
			MakeAlreadyIndexed(Verbose);

		InvalidateCache(0);
    } 
	RefreshNodesCache();
/*
	cout<<"EMPTY NODES COUNT ="<<CLetter::s_EmptyNodeCount<<endl;
	cout<<"FULL NODES COUNT  ="<<CLetter::s_FullNodeCount<<endl;
	cout<<"ALL NODES COUNT   ="<<CLetter::s_AllNodeCount<<endl;
*/
	return ReturnValue;
}

bool CSearch::LoadNode(FILE *IndexFile, CString Word, unsigned char Endian, CSVector& TempVector)
{
    unsigned int i;
    unsigned int PagesSize=0,PagesTemp=0;
    unsigned int LeafNumber,LeafTemp=0;
    unsigned char m_Letter;
	int result;
    fread(&m_Letter,1,1,IndexFile);
    Word += m_Letter;

    fread(&LeafNumber,sizeof(int),1,IndexFile);
    if (Endian==1) 
	{
        LeafTemp=LeafNumber;
        LeafNumber=Flip_int32(LeafTemp);
    }
    
    fread(&PagesSize,sizeof(int),1,IndexFile);
    if (Endian==1) 
    {
        PagesTemp=PagesSize;
        PagesSize=Flip_int32(PagesTemp);
    }
    
    
    if (PagesSize > 0)
    {
        TempVector.SetSize(PagesSize,false,false);
        result = fread(&TempVector[0], sizeof(int), PagesSize, IndexFile);

        if (result != (int)PagesSize) 
			return false;

        if (Endian == 1)
        {
            register int TempVal;
            for(i = 0; i < PagesSize; i++)
            {
                TempVal = TempVector[i];
                TempVector[i] = Flip_int32(TempVal);
            }
        }


        bool IsNewWord=false;
//        cout<<"Adding word: ["<<Word<<"]"<<endl;
        m_WordsTree[(unsigned char)Word[0]].AppendWord(Word,TempVector,IsNewWord);
//        cout<<"Word added"<<endl;
        if (IsNewWord==true)
            m_WordsCount++;      

    }
    
    if (LeafNumber)
    {
        for(i=0;i<LeafNumber;i++)
        {
            LoadNode(IndexFile,Word,Endian,TempVector);
        }        
    }
    
    return true;
}    


bool CSearch::LoadIndex(const CString& FileName, bool Verbose, bool bMakeAlreadyIndexed)
{  
    unsigned char IndexEndian;
    unsigned char EndianChange = 0;
    unsigned int i;

    CMMapFile FastFile(FileName);
    if (! FastFile.MMap(MMAP_READOPENMODE))
      return false;
    long fSize = FastFile.GetSize();
    if (!fSize || !FastFile.GetMem())
        return false;
        
    if (FastFile.Read(&IndexEndian, 1) != 1)
        return false;
    
    if (IndexEndian != m_Endian) {
        cout << "[byte order changed, converting]";
        EndianChange = 1;
    }
        
    CProgress Progress(10, Verbose);
    
    for(i = 0; i < CHARSET_SIZE; i++) {
        int Count = 0, CountTemp;
    
        if (g_pHandler->GetSignalSigterm())
			return false;
        if (FastFile.Read(& Count, sizeof(int)) != sizeof(int))
            return false;
        
        if (EndianChange == 1) {
            CountTemp = Count;
            Count = Flip_int32(CountTemp);
        }
        
        if (Count > 0) {
            if (m_WordsTree[(unsigned char) i].Load(FastFile, m_WordsCount, EndianChange) == false) {
                Progress.Error(Verbose);
                if (Verbose) cout << endl;
                return false;
            }
        }
        
        Progress.Show(FastFile.GetOffset(), fSize, Verbose);
    }
    
    Progress.Finish(Verbose);
        
    if (Verbose) {
        cout << "[" << m_WordsCount.Get() << " words]" << endl;
    }

    if (bMakeAlreadyIndexed)
        MakeAlreadyIndexed(Verbose);

    InvalidateCache(0);

	RefreshNodesCache();
    return true;
}

/* Load un index de mots et leurs pages */

void CSearch::RefreshNodesCache()
{
	for(unsigned int i = 0; i < CHARSET_SIZE; i++)
	{
		m_WordsTree[(unsigned char) i].RecalcCache();
	}
}

bool CSearch::LoadIndex_1_31(const CString& FileName, bool Verbose, bool bMakeAlreadyIndexed) {
//cout<<"SIZE IS :"<<sizeof(CLetter)<<endl;    
    StartWriting();
    CMMapFile FastFile(FileName);
    if (! FastFile.MMap(MMAP_READOPENMODE))
      return false;
    long fSize = FastFile.GetSize();
    int NumberOfWords=0;
    if (fSize && FastFile.GetMem()) {
        int PrevPos;
        register int IntervalPos;
        bool Intervalled = false;
        CStringA Word;
        CString Line;
        CIntVector LineVector;
        CVector<int> PagesVector;
        
        CProgress Progress(10, Verbose);
        while (FastFile.ReadLine(&Line) >= 0) {
            Progress.Show(FastFile.GetOffset(), fSize, Verbose);            
            if (!Line.GetLength()) 
                continue;
            PrevPos = 0;
            IntervalPos = -1;
            Word.Empty();
            LineVector.RemoveAll();
            PagesVector.RemoveAll();
            for (register int Pos = 0; Pos <= (int) Line.GetLength(); Pos++ ) {
                if ((Pos == (int) Line.GetLength())||(Line[Pos] == ' ')) {
                    if (Word.GetLength()) {
                        if (IntervalPos >= 0) {
                            
                            //LineVector._AppendInt(CString::StrToInt(Line.Mid(PrevPos, IntervalPos - PrevPos)), CString::StrToInt(Line.Mid(IntervalPos + 1, Pos - IntervalPos-1)));
                            LineVector._AppendInt(Line.GetInt(PrevPos, IntervalPos - PrevPos), Line.GetInt(IntervalPos + 1, Pos - IntervalPos-1));
                            IntervalPos = -1;
                        } else {
                            // cout << "\tNumber:[" << Line.Mid(PrevPos, Pos - PrevPos) << "]" << endl;
                            if (Intervalled) LineVector._AppendElt(Line.GetInt(PrevPos, Pos - PrevPos));
                            else LineVector.AddElt(Line.GetInt(PrevPos, Pos - PrevPos));
                        }
                        PrevPos = Pos + 1;
                    } else {
                        Line.Left(Pos, &Word);
                        PrevPos = Pos + 1;
                        // cout << "Word:[" << Word << "]" << endl;
                    }
                } else {
                    if (Word.GetLength()) {
                        if (Line[Pos] == '-') {
                            if (!Intervalled) Intervalled = true;
                            IntervalPos = Pos;
                        } else if (!isdigit(Line[Pos])) {
                            LineVector.RemoveAll();
                            break;
                        }
                    }
                }
            }
            // cout << "Done." << endl;
            // cout << "[" << Word << "][" << LineVector << "]" << endl;
            if (Word.GetLength() && LineVector.GetSize()) {
//                cout << "AppendWord [" << Word << "][" << LineVector << "]" << endl;
                LineVector.AppendTo(PagesVector);
                AddWord(Word, PagesVector);
                NumberOfWords++;
            }
        }
    
        Progress.Finish(Verbose);
    }    
    cout << "[" << m_WordsCount.Get() << " words]" << endl;
    StopWriting();
    if (bMakeAlreadyIndexed)
        MakeAlreadyIndexed(Verbose);
/*
	cout<<"EMPTY NODES COUNT ="<<CLetter::s_EmptyNodeCount<<endl;
	cout<<"FULL NODES COUNT  ="<<CLetter::s_FullNodeCount<<endl;
	cout<<"ALL NODES COUNT   ="<<CLetter::s_AllNodeCount<<endl;
*/

	InvalidateCache(0);
	RefreshNodesCache();
//	for(unsigned char z='S';z<'z';z++)
//		m_WordsTree[z].ShowTree();
    return NumberOfWords;
}

void CSearch::InvalidateCache(long nPeriod)
{
    if (! nPeriod)
    {
        m_CacheExact.InvalidateAll();
        m_CacheInsensExact.InvalidateAll();
        m_CachePartial.InvalidateAll();
        m_CacheInsensPartial.InvalidateAll();
    }
    else
    {
        m_CacheExact.Invalidate(nPeriod);
        m_CacheInsensExact.Invalidate(nPeriod);
        m_CachePartial.Invalidate(nPeriod);
        m_CacheInsensPartial.Invalidate(nPeriod);
    }
}

void CSearch::RemoveAll(bool bLocked) 
{    
    if (bLocked)
        StartWriting();

    for (unsigned int i = 0; i < CHARSET_SIZE; i++)
	{
        m_WordsTree[(unsigned char) i].RemoveAll();
	}

    m_WordsCount.Set(0);

    InvalidateCache(0);

    m_PagesToRemove.RemoveAll();
    m_PagesToRemovePermanently.RemoveAll();

    m_AlreadyIndexedPages.RemoveAll();

	m_AllLetters.RemoveAll();

	m_WeakWords.RemoveAll();

    if (bLocked)
        StopWriting();
}    
    
    
    
    
    
    
    
    
bool CSearch::RemoveWords(const CVector<CString>& Words, bool bRegExp) {
    if (bRegExp) {
        Trace(tagExcludeWords, levInfo, ("CSearch::RemoveWords - regexp remove.", Words.GetSize()));
        return RemoveWordsRegExp(Words);
    } else {
        Trace(tagExcludeWords, levInfo, ("CSearch::RemoveWords - pattern remove.", Words.GetSize()));
        return RemoveWordsPattern(Words);
    }
}

bool CSearch::RemoveWordsPattern(const CVector<CString>& Words)
{
    bool Result;
    unsigned int i;
    StartWriting();

    CProgress Progress;
    
    for(i=0;i<Words.GetSize();i++)
    {
        
        Progress.Show(i, Words.GetSize());
        
        Trace(tagExcludeWords, levInfo, ("CSearch::RemoveWordsPattern - [%s].", Words[i].GetBuffer()));
        
        Result = m_WordsTree[(unsigned char) Words[i][0]].RemoveWord(Words[i]);
        
        if (Result==true) 
            m_WordsCount.Dec();
    }
    
    Trace(tagExcludeWords, levInfo, ("CSearch::RemoveWordsPattern - removing metas.", Words[i].GetBuffer()));
    
    int DeletedWords;

    Progress.Finish();
    Progress.Init();
    
    for(i=0;i<CHARSET_SIZE;i++)
    {
    
        Progress.Show(i, CHARSET_SIZE);
        
        DeletedWords = 0;
        m_WordsTree[(unsigned char) i].RemoveWordsInMeta(Words, DeletedWords);
        m_WordsCount.Dec(DeletedWords);
    }
    
    Progress.Finish();
    
    Trace(tagExcludeWords, levInfo, ("CSearch::RemoveWordsPattern - done.", Words[i].GetBuffer()));    
    
    StopWriting();
    return true;
}

bool CSearch::RemoveWordsRegExp(const CVector<CString>& Words) {
    unsigned int i;

    StartWriting();

    CProgress Progress;
    
    CVector<CRegExp> RegexpWords;
    RegexpWords.SetSize(Words.GetSize());

    for (i=0;i<Words.GetSize();i++) {
        ((CRegExp&) RegexpWords[i]).RegComp(Words[i].GetBuffer());
    }
    
    for(i=0;i<CHARSET_SIZE;i++)
    {
        Progress.Show(i, CHARSET_SIZE);

        int DeletedWords = 0;
        m_WordsTree[(unsigned char) i].RemoveWordsRegExp(RegexpWords, DeletedWords);
        m_WordsCount.Dec(DeletedWords);
    }

    Progress.Finish();

    StopWriting();
    return true;
}

void CSearch::AppendTo(CSearch& Searcher, const CVector<int>& DispTable)
{
    unsigned int i;
    cout << "[merging NDXs]"; cout.flush();
    StartWriting();
    CProgress Progress(10, true);    
    for(i=0;i<CHARSET_SIZE;i++)
    {
        Progress.Show(i, CHARSET_SIZE);
        m_WordsTree[(unsigned char) i].AppendTo(Searcher,DispTable);                
    }
    StopWriting();
    Progress.Finish();
    cout << "[" << m_WordsCount.Get() << " words]" << endl;
}

void CSearch::PopulateXmlNode(CXmlTree& Tree, CTreeElement< CXmlNode > * pXmlNode) const {  
    
    // populate the search xml
    static const CString SearchXml(
        "<size></size>"                             
        "<requests></requests>"                     
        "<hits></hits>"                             
        "<occupation></occupation>"                 
        "<hitrate></hitrate>"                       
        "<rpm></rpm>"
        "<life></life>"             
        "<words>"                                   
        " <l1></l1>"                                
        " <l2></l2>"                                
        " <l3></l3>"                                
        " <l4></l4>"                                
        "</words>");
    
    CXmlTree XmlTree;
    XmlTree.SetXml(SearchXml); 
    StartReading();
    XmlTree.SetValue("/size", CString::IntToStr(m_CacheExact.GetSize() + m_CacheInsensExact.GetSize() + m_CachePartial.GetSize() + m_CacheInsensPartial.GetSize()) + " record(s)");
    XmlTree.SetValue("/requests", CString::IntToStr(m_CacheExact.GetRequests().Get() + m_CacheInsensExact.GetRequests().Get() + m_CachePartial.GetRequests().Get() + m_CacheInsensPartial.GetRequests().Get()));
    XmlTree.SetValue("/hits", CString::IntToStr(m_CacheExact.GetHits().Get() + m_CacheInsensExact.GetHits().Get() + m_CachePartial.GetHits().Get() + m_CacheInsensPartial.GetHits().Get()));
    XmlTree.SetValue("/occupation", CString::IntToStr((int) ((m_CacheExact.GetOccupation() + m_CacheInsensExact.GetOccupation() + m_CachePartial.GetOccupation() + m_CacheInsensPartial.GetOccupation())/4)) + "%");
    XmlTree.SetValue("/hitrate", CString::IntToStr((int) ((m_CacheExact.GetHitRate() + m_CacheInsensExact.GetHitRate() + m_CachePartial.GetHitRate() + m_CacheInsensPartial.GetHitRate())/4)) + "%");
    XmlTree.SetValue("/rpm", CString::IntToStr(m_CacheExact.GetRequestsPerMinute() + m_CacheInsensExact.GetRequestsPerMinute() + m_CachePartial.GetRequestsPerMinute() + m_CacheInsensPartial.GetRequestsPerMinute()));
    XmlTree.SetValue("/life", CString::IntToStr(m_SearchCacheLife) + " second(s)");
    
    m_CacheExact.PopulateXmlTree(XmlTree, "/words/l1");
    m_CacheInsensExact.PopulateXmlTree(XmlTree, "/words/l2");
    m_CachePartial.PopulateXmlTree(XmlTree, "/words/l3");
    m_CacheInsensPartial.PopulateXmlTree(XmlTree, "/words/l4");
    StopReading();
    // insert the search stats tree
    Tree.MoveAsChildLast(XmlTree, XmlTree.GetHead(), pXmlNode);
}

