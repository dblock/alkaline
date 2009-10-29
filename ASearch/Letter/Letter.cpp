#include <alkaline.hpp>
#include "Letter.hpp"
#include <Search/Search.hpp>
#include <Swap/TMemory.hpp>
#include <String/GStrings.hpp>
#include <RegExp/RegExp.hpp>
#include <Main/TraceTags.hpp>

// CAtomic CLetter::s_EmptyNodeCount = 0;
// CAtomic CLetter::s_FullNodeCount = 0;
// CAtomic CLetter::s_AllNodeCount = 0;

int CLetter::s_CacheDepth = 3;


CLetter::CLetter()
{
    m_PagesVector = NULL;
	// s_AllNodeCount++;
	m_ContentSize = 0;
	m_Content = NULL;
    m_CacheVector = NULL;
}

CLetter::~CLetter()
{    
    RemoveAll();
	// s_AllNodeCount--;
}


void CLetter::RemoveAll()
{
    if (m_PagesVector != NULL) 
    {
        delete m_PagesVector;
        m_PagesVector = NULL;
    }
    
	m_ContentSize = 0;
	
	DelBuf();

	for(unsigned int i = 0;i<m_NextLetters.GetSize();i++)
	{
		delete m_NextLetters[i];
	}
	
    m_NextLetters.RemoveAll();        

	if (m_CacheVector)
	{
		delete m_CacheVector;
		m_CacheVector = NULL;
	}
}


void CLetter::WriteIndex(FILE *IndexFile, CString& Buffer)
{  
    int nBufferLen = Buffer.GetLength();
    
	if (m_ContentSize)
	{
		Buffer.Append((char*)m_Content, (int) m_ContentSize);
	}
    
    if ( (m_PagesVector != NULL) && (m_PagesVector->GetSize() != 0) )
    {
        fwrite(Buffer.GetBuffer(), sizeof(char), Buffer.GetLength(), IndexFile);
        fwrite(" ", sizeof(char), 1, IndexFile);
        m_PagesVector->DumpTo(IndexFile);
        fwrite(g_strCrLf, sizeof(char), base_strlen(g_strCrLf), IndexFile);
    }
    
    for(unsigned int j = 0;j<m_NextLetters.GetSize();j++)
    {
        m_NextLetters[j]->WriteIndex(IndexFile, Buffer);
        Buffer.SetLength(nBufferLen + 1);
    }
    
    Buffer.SetLength(nBufferLen);
}


bool CLetter::Load(FILE *IndexFile, unsigned int& WordsCount, unsigned char Endian, CLettersHandler *LettersHandler)
{
    unsigned int i;
	int j;
    unsigned int PagesSize = 0;
    unsigned int PagesTemp = 0;
    unsigned int LeafNumber = 0;
    unsigned int LeafTemp = 0;
    int result;
    
	if (fread(&m_ContentSize,1,1,IndexFile) != 1)
		return false;

	m_Content = AllocBuf(m_ContentSize);

    if (m_Content == NULL)
		return false;

	if (fread(m_Content, 1, (int)m_ContentSize, IndexFile) != (unsigned int)m_ContentSize)
	{
		DelBuf();
		return false;
	}
    
	for(j  = 0 ; j < (int) m_ContentSize; j++)
		LettersHandler->Add((unsigned char)m_Content[j],this);

			// cout<<"["<<endl<<" Loading Letter <"<<m_Letter<<">"<<endl;
    fread(&LeafNumber, sizeof(int), 1, IndexFile);
    if (Endian  ==  1) 
    {
        LeafTemp = LeafNumber;
        LeafNumber = Flip_int32(LeafTemp);
    }
    if (LeafNumber)
    {
		m_NextLetters.SetDim(LeafNumber,false);
        for(i = 0; i < LeafNumber; i++) 
        {
            m_NextLetters += new CLetter;
        }
    }
    
    fread(&PagesSize, sizeof(int), 1, IndexFile);

    if (Endian  ==  1)
    {
        PagesTemp = PagesSize;
        PagesSize = Flip_int32(PagesTemp);
    }
    
    
    if (PagesSize > 0)
    {
        
        m_PagesVector = new CSwapVector;
        result = m_PagesVector->ReadFrom(IndexFile,PagesSize);
        if (Endian  ==  1)
        {
			m_PagesVector->ConvertIndianness();
        }

		// s_FullNodeCount++;
      
        if (result != (int)PagesSize) 
		{
			DelBuf();
			return false;
		}
        
        WordsCount++;
    }
	else 
	{
		// s_EmptyNodeCount++;
	}
    
    if (LeafNumber)
    {

        for(i = 0; i < LeafNumber; i++)
        {
            if (m_NextLetters[i]->Load(IndexFile, WordsCount, Endian, LettersHandler)  ==  false)
			{
				DelBuf();
				return false;
			}
        }        
    }
    
    return true;
}

bool CLetter::Serialize(CMMapFile& IndexFile)
{
    unsigned int LeafNumber = m_NextLetters.GetSize();

	if (! IndexFile.Write(& m_ContentSize, sizeof(m_ContentSize))) {
		cerr << "[error:socz]" << endl;
		return false;
	}

	if (((int) m_ContentSize) != 0)
	{
		if (! IndexFile.Write(m_Content, (int) m_ContentSize)) {
			cerr << "[error:soct]" << endl;
			return false;
		}
	}

	if (! IndexFile.Write(& LeafNumber, sizeof(int))) {
		cerr << "[error:soln]" << endl;
		return false;
	}
	
    unsigned int PagesSize=0;
    if (m_PagesVector!=NULL)
        if (m_PagesVector->GetSize()>0) 
        {
            PagesSize=m_PagesVector->GetSize();
        }
	if (! IndexFile.Write(& PagesSize, sizeof(int))) {
		cerr << "[error:sopz]" << endl;
		return false;
	}

    if (PagesSize>0) 
    {
		if (!m_PagesVector->DumpTo(IndexFile)) {
			cerr << "[error:sopv]" << endl;
			return false;
		}
    }
    
    for(int i = 0; i < (int) m_NextLetters.GetSize(); i++)
    {
		if (! m_NextLetters[(unsigned char) i]->Serialize(IndexFile)) {
			cerr << "[error:" << LeafNumber << "(" << (int) i << ")]" << endl;
            return false;
		}
    }

    return true;
}

bool CLetter::Load(CMMapFile& IndexFile, unsigned int& WordsCount, unsigned char Endian, CLettersHandler *LettersHandler)
{
    unsigned int i;
	int j;
    unsigned int PagesSize = 0;
    unsigned int PagesTemp = 0;
    unsigned int LeafNumber = 0;
    unsigned int LeafTemp = 0;
    

	if (IndexFile.Read(&m_ContentSize,sizeof(m_ContentSize)) != sizeof(m_ContentSize))
	{
        return false;
    }

	if ((int)m_ContentSize != 0)
	{
		m_Content = AllocBuf(m_ContentSize);
	    
		if (m_Content  ==  NULL)
		{
            return false;
        }

		if (IndexFile.Read(m_Content, (int)m_ContentSize) != (int)m_ContentSize)
		{
			DelBuf();
			return false;
		}

		for(j = 0; j < (int)m_ContentSize;j++)
			LettersHandler->Add((unsigned char)m_Content[j],this);
	}

    if (IndexFile.Read(& LeafNumber, sizeof(int)) != sizeof(int))
	{
		DelBuf();
		return false;
	}
    
    if (Endian  ==  1) 
    {
        LeafTemp = LeafNumber;
        LeafNumber = Flip_int32(LeafTemp);
    }
    
    if (LeafNumber)
    {
		m_NextLetters.SetDim(LeafNumber,false);
        for(i = 0; i < LeafNumber; i++) 
        {            
			m_NextLetters += new CLetter;
        }
    }
    
    if (IndexFile.Read(& PagesSize, sizeof(int)) != sizeof(int))
 	{
		DelBuf();
		return false;
	}

    if (Endian  ==  1)
    {
        PagesTemp = PagesSize;
        PagesSize = Flip_int32(PagesTemp);
    }
    
    if (PagesSize > 0)
    {
        
        m_PagesVector = new CSwapVector;
		int Result=m_PagesVector->ReadFrom(IndexFile,PagesSize);
		if ((unsigned int)Result != PagesSize)
        {
			DelBuf();
			return false;
		}
    
        if (Endian  ==  1)
			m_PagesVector->ConvertIndianness();
        
        WordsCount++;
    }
	else 
	{
        m_PagesVector=NULL;
		// s_EmptyNodeCount++;
	}
    
    if (LeafNumber)
    {

        for(i = 0; i < LeafNumber; i++)
        {
            if (m_NextLetters[i]->Load(IndexFile, WordsCount, Endian, LettersHandler)  ==  false)
			{
				DelBuf();
				return false;
			}
        }
    }
    return true;
}

void CLetter::FindPartialWordInsensitive(const CString& Word, const unsigned int WordPos, const unsigned int Start, RESULTYPE& Result)
{
    register unsigned int i;
    unsigned int NewPos = WordPos;
//cout<<"In FindPartialWordInsens for Word :"<<Word<<" OwnWord:"<<GetWord()<<" WordPos: "<<WordPos<<" Start: "<<Start<<endl;
    if (WordPos  ==  Word.GetLength()-1)
    {
		GetSubVectors(Result);
    }
    else
    {
        unsigned char chWord, chLetter;
		unsigned char Min;
			
		if ( (unsigned char) (Word.GetLength() - WordPos + Start) < m_ContentSize ) 
		{
			Min = (unsigned char) (Word.GetLength() - WordPos + Start);
		}
		else
		{
			Min = m_ContentSize;
		}

		for(i = Start;i<(unsigned int)Min;i++)
		{
            chWord = CString::UCase((unsigned char) Word[NewPos]);
            chLetter = CString::UCase(m_Content[i]);			
			if (chLetter != chWord)
				return;
			NewPos++;
		}
		if ( (unsigned char) (Word.GetLength()-WordPos) > m_ContentSize - (unsigned char)Start)
		{
			chWord = CString::UCase((unsigned char) Word[NewPos]);
			for(i = 0; i < m_NextLetters.GetSize(); i++)
			{   
				if (! m_NextLetters[i]->m_ContentSize)
					continue;

				chLetter = CString::UCase(m_NextLetters[i]->m_Content[0]);
				if ( chWord  ==  chLetter)
				{
	                m_NextLetters[i]->FindPartialWordInsensitive(Word, NewPos, 0, Result);
				}
			}
		}
		else
		{
			GetSubVectors(Result);
		}
    }    
}

void CLetter::FindExactWordInsensitive(const CString& Word, const unsigned int WordPos, const unsigned int Start, RESULTYPE& Result)
{
    register unsigned int i;
    unsigned int NewPos = WordPos;

	// cout<<"In FindExactWordInsens for Word :"<<Word<<" OwnWord:"<<GetWord()<<endl;
    if (WordPos  ==  Word.GetLength()-1)
    {
		if (m_ContentSize>1) 
			return;
		GetOwnResults(Result);
    }
    else
    {
        unsigned char chWord, chLetter;
		unsigned char Min;
			
		if ( (unsigned char) (Word.GetLength() - WordPos + Start) < m_ContentSize ) 
		{
			Min = (unsigned char) (Word.GetLength() - WordPos + Start);
		}
		else
		{
			Min = m_ContentSize;
		}

		for(i = Start;i<(unsigned int)Min;i++)
		{
            chWord = CString::UCase((unsigned char) Word[NewPos]);
            chLetter = CString::UCase(m_Content[i]);			
			if (chLetter != chWord)
				return;
			NewPos++;
		}
		if ( (unsigned char) (Word.GetLength()-WordPos) > m_ContentSize)
		{
			chWord = CString::UCase((unsigned char) Word[NewPos]);
			for(i = 0; i < m_NextLetters.GetSize(); i++)
			{   
				if (! m_NextLetters[i]->m_ContentSize)
					continue;

				chLetter = CString::UCase(m_NextLetters[i]->m_Content[0]);
				if ( chWord  ==  chLetter)
				{
	                m_NextLetters[i]->FindExactWordInsensitive(Word, NewPos, 0, Result);
				}
			}
		}
		else if ( (unsigned char) (Word.GetLength()-WordPos)  ==  m_ContentSize )
		{
			GetOwnResults(Result);
		}
    }    
}

void CLetter::FindExactWordSensitive(const CString& Word, const unsigned int WordPos, const unsigned int Start, RESULTYPE& Result)
{
    register unsigned int i;
    unsigned int NewPos = WordPos;

	// cout<<"In FindExactWordSens for Word :"<<Word<<" OwnWord:"<<GetWord()<<endl;
    if (WordPos  ==  Word.GetLength()-1)
    {
		if (m_ContentSize>1) 
			return;
		GetOwnResults(Result);
    }
    else
    {
		unsigned char Min;
			
		if ( (unsigned char) (Word.GetLength() - WordPos + Start) < m_ContentSize ) 
			Min = (unsigned char) (Word.GetLength() - WordPos + Start);
		else
			Min = m_ContentSize;

		for(i = Start;i<(unsigned int)Min;i++)
		{
			if ((unsigned char)Word[NewPos] != m_Content[i])
				return;
			NewPos++;
		}
		if ( (unsigned char) (Word.GetLength()-WordPos) > m_ContentSize)
		{
			for(i = 0; i < m_NextLetters.GetSize(); i++)
			{   
				if (! m_NextLetters[i]->m_ContentSize)
					continue;

				if ( (unsigned char) Word[NewPos]  ==  m_NextLetters[i]->m_Content[0])
				{
	                m_NextLetters[i]->FindExactWordSensitive(Word, NewPos, 0, Result);
				}
			}
		}
		else if ( (unsigned char) (Word.GetLength()-WordPos)  ==  m_ContentSize )
		{
			GetOwnResults(Result);
		}
    }    
}
















void CLetter::FindPartialWordSensitive(const CString& Word, const unsigned int WordPos, const unsigned int Start, RESULTYPE& Result)
{
    register unsigned int i;
    unsigned int NewPos = WordPos;

	// cout<<"In FindPartialWordSens for Word :"<<Word<<" OwnWord:"<<GetWord()<<endl;
    if (WordPos  ==  Word.GetLength()-1)
    {
		GetSubVectors(Result);
    }
    else
    {
		unsigned char Min;
			
		if ( (unsigned char) (Word.GetLength() - WordPos + Start) < m_ContentSize ) 
			Min = (unsigned char) (Word.GetLength() - WordPos + Start);
		else
			Min = m_ContentSize;

		for(i = Start;i<(unsigned int)Min;i++)
		{
			if ((unsigned char)Word[NewPos] != m_Content[i])
				return;
			NewPos++;
		}
		if ( (unsigned char) (Word.GetLength()-WordPos) > m_ContentSize - (unsigned char) Start)
		{
			for(i = 0; i < m_NextLetters.GetSize(); i++)
			{   
				if (! m_NextLetters[i]->m_ContentSize)
					continue;

				if ( (unsigned char) Word[NewPos]  ==  m_NextLetters[i]->m_Content[0])
				{
	                m_NextLetters[i]->FindPartialWordSensitive(Word, NewPos, 0, Result);
				}
			}
		}
		else
		{
			GetSubVectors(Result);
		}
    }    
}

void CLetter::FindPartialMetaInsensitive(const CString& Word, const unsigned int WordPos, const unsigned int Start, RESULTYPE& Result)
{
    register unsigned int i,j;
	unsigned int NewPos = WordPos;
    unsigned char chWord, chLetter;
	unsigned char Min;

	// cout<<"In FindPartialMetaInsens for Word :"<<Word<<" OwnWord:"<<GetWord()<<endl;
	if ( (unsigned char) (Word.GetLength() - WordPos + Start) < m_ContentSize ) 
		Min = (unsigned char) (Word.GetLength() - WordPos + Start);
	else
		Min = m_ContentSize;

	for(i = Start;i<Min;i++)
	{
		if ((unsigned char)Word[NewPos] != m_Content[i]) // if the META part is not the same, we're not in the good subtree, go back
			return;
		else if ( ((unsigned char)Word[NewPos] == m_Content[i]) && ((unsigned char)Word[NewPos] == ':') ) // if ':' is found, then it's the start of the word
		{
			// cout<<"META and Word are mixed, start InsWordSearch"<<endl;
			for(j = i;j<(unsigned int)Min;j++) // We are in Insensitive mode, so we compare the letters insensitive
			{
				chWord = CString::UCase((unsigned char) Word[NewPos]);
				chLetter = CString::UCase(m_Content[j]);
				if ( chWord != chLetter) // If letters are different --> bad subtree --> go back
				{
					return;
				}
				NewPos++;
			}
			// If we end up here, then it means that up to now, the words match

			if ( (unsigned char) (Word.GetLength()-WordPos) > m_ContentSize ) // if not the end of the word, continue the search forward
			{
				// cout<<"Not the end of the word, go to next node"<<endl;
				chWord = CString::UCase((unsigned char) Word[NewPos]);
				for(j = 0; j < m_NextLetters.GetSize(); j++)
				{   
					if (! m_NextLetters[j]->m_ContentSize)
						continue;

					chLetter = CString::UCase(m_NextLetters[j]->m_Content[0]);
					if ( chWord  ==  chLetter)
					{
						m_NextLetters[j]->FindPartialWordInsensitive(Word, NewPos, 0, Result);
					}
				}
			}
			else // if it's the end of the word, then it means that we are ON the node corresponding to the word --> get the pages
			{
				GetSubVectors(Result);
			}
			return;
		}
		NewPos++;
	}

	// cout<<"Still in META, Going to next node"<<endl;
	if ( (unsigned char) (Word.GetLength()-WordPos) > m_ContentSize ) // It's not yet the end of the META part
	{
		for(i = 0; i < m_NextLetters.GetSize(); i++)
		{   
			if (! m_NextLetters[i]->m_ContentSize)
				continue;

			if ( Word[NewPos]  ==  m_NextLetters[i]->m_Content[0])
			{
				m_NextLetters[i]->FindPartialMetaInsensitive(Word, NewPos, 0, Result);
			}
		}
	}
}





void CLetter::FindExactMetaInsensitive(const CString& Word, const unsigned int WordPos, const unsigned int Start, RESULTYPE& Result)
{
    register unsigned int i,j;
	unsigned int NewPos = WordPos;
    unsigned char chWord, chLetter;
	unsigned char Min;

	// cout<<"In FindExactMetaInsens for Word :"<<Word<<" OwnWord:"<<GetWord()<<endl;
	if ( (unsigned char) (Word.GetLength() - WordPos + Start) < m_ContentSize ) 
		Min = (unsigned char) (Word.GetLength() - WordPos + Start);
	else
		Min = m_ContentSize;

	for(i = Start;i<Min;i++)
	{
		if ((unsigned char)Word[NewPos] != m_Content[i]) // if the META part is not the same, we're not in the good subtree, go back
			return;
		else if ( ((unsigned char)Word[NewPos] == m_Content[i]) && (Word[NewPos] == ':') ) // if ':' is found, then it's the start of the word
		{
			// cout<<"META and Word are mixed, start InsWordSearch"<<endl;
			for(j = i;j<(unsigned int)Min;j++) // We are in Insensitive mode, so we compare the letters insensitive
			{
				chWord = CString::UCase((unsigned char) Word[NewPos]);
				chLetter = CString::UCase(m_Content[j]);
				if ( chWord != chLetter) // If letters are different --> bad subtree --> go back
				{
					return;
				}
				NewPos++;
			}
			// If we end up here, then it means that up to now, the words match

			if ( (unsigned char) (Word.GetLength()-WordPos) > m_ContentSize ) // if not the end of the word, continue the search forward
			{
				// cout<<"Not the end of the word, go to next node"<<endl;
				chWord = CString::UCase((unsigned char) Word[NewPos]);
				for(j = 0; j < m_NextLetters.GetSize(); j++)
				{   
					if (! m_NextLetters[j]->m_ContentSize)
						continue;

					chLetter = CString::UCase(m_NextLetters[j]->m_Content[0]);
					if ( chWord  ==  chLetter)
					{
						m_NextLetters[j]->FindExactWordInsensitive(Word, NewPos, 0, Result);
					}
				}
			}
			else if ( (unsigned char) (Word.GetLength()-WordPos)  ==  m_ContentSize ) // if it's the end of the word, then it means that we are ON the node corresponding to the word --> get the pages
			{
				GetOwnResults(Result);
			}
			return;
		}
		NewPos++;
	}

	// cout<<"Still in META, Going to next node"<<endl;
	if ( (unsigned char) (Word.GetLength()-WordPos) > m_ContentSize ) // It's not yet the end of the META part
	{
		for(i = 0; i < m_NextLetters.GetSize(); i++)
		{   
			if (! m_NextLetters[i]->m_ContentSize)
				continue;

			if ( Word[NewPos]  ==  m_NextLetters[i]->m_Content[0])
			{
				m_NextLetters[i]->FindExactMetaInsensitive(Word, NewPos, 0, Result);
			}
		}
	}
}



void CLetter::FindExactMetaSensitive(const CString& Word, const unsigned int WordPos, const unsigned int Start, RESULTYPE& Result)
{
    register unsigned int i,j;
	unsigned int NewPos = WordPos;
	unsigned char Min;

	// cout<<"In FindExactMetaSens for Word :"<<Word<<" OwnWord:"<<GetWord()<<endl;
	if ( (unsigned char) (Word.GetLength() - WordPos + Start) < m_ContentSize ) 
		Min = (unsigned char) (Word.GetLength() - WordPos + Start);
	else
		Min = m_ContentSize;

	for(i = Start;i<Min;i++)
	{
		if ((unsigned char)Word[NewPos] != m_Content[i]) // if the META part is not the same, we're not in the good subtree, go back
			return;
		else if ( ((unsigned char)Word[NewPos] == m_Content[i]) && (Word[NewPos] == ':') ) // if ':' is found, then it's the start of the word
		{
			// cout<<"META and Word are mixed, start SensWordSearch"<<endl;
			for(j = i;j<(unsigned int)Min;j++) // We are in Sensitive mode, so we compare the letters Sensitive
			{
				if ( (unsigned char) Word[NewPos] != m_Content[j]) // If letters are different --> bad subtree --> go back
				{
				  return;
				}
				NewPos++;
			}
			// If we end up here, then it means that up to now, the words match
			// cout<<"Words match up to now"<<endl;
			if ( (unsigned char) (Word.GetLength()-WordPos) > m_ContentSize ) // if not the end of the word, continue the search forward
			{
				// cout<<"Not the end of the word, continue forward"<<endl;
				for(j = 0; j < m_NextLetters.GetSize(); j++)
				{   
					if (! m_NextLetters[j]->m_ContentSize)
						continue;

					if ( (unsigned char) Word[NewPos]  ==  m_NextLetters[j]->m_Content[0])
					{
						m_NextLetters[j]->FindExactWordSensitive(Word, NewPos, 0, Result);
					}
				}
			}
			else if ( (unsigned char) (Word.GetLength()-WordPos)  ==  m_ContentSize ) // if it's the end of the word, then it means that we are ON the node corresponding to the word --> get the pages
			{
				GetOwnResults(Result);
			}
			return;
		}
		NewPos++;
	}

	// cout<<"Still in META, Going to next node"<<endl;
	if ( (unsigned char) (Word.GetLength()-WordPos) > m_ContentSize ) // It's not yet the end of the META part
	{
		for(i = 0; i < m_NextLetters.GetSize(); i++)
		{   
			if (! m_NextLetters[i]->m_ContentSize)
				continue;

			if ( Word[NewPos]  ==  m_NextLetters[i]->m_Content[0])
			{
				m_NextLetters[i]->FindExactMetaSensitive(Word, NewPos, 0, Result);
			}
		}
	}
}




void CLetter::FindPartialMetaSensitive(const CString& Word, const unsigned int WordPos, const unsigned int Start, RESULTYPE& Result)
{
    register unsigned int i,j;
	unsigned int NewPos = WordPos;
	unsigned char Min;

	// cout<<"In FindPartialMetaSens for Word :"<<Word<<" OwnWord:"<<GetWord()<<endl;
	if ( (unsigned char) (Word.GetLength() - WordPos + Start) < m_ContentSize ) 
		Min = (unsigned char) (Word.GetLength() - WordPos + Start);
	else
		Min = m_ContentSize;

	for(i = Start;i<Min;i++)
	{
		if (Word[NewPos] != m_Content[i]) // if the META part is not the same, we're not in the good subtree, go back
			return;
		else if ( ((unsigned char)Word[NewPos] == m_Content[i]) && (Word[NewPos] == ':') ) // if ':' is found, then it's the start of the word
		{
			// cout<<"META and Word are mixed, start SensWordSearch"<<endl;
			for(j = i;j<(unsigned int)Min;j++) // We are in Sensitive mode, so we compare the letters Sensitive
			{
				if ( (unsigned char) Word[NewPos] != m_Content[j]) // If letters are different --> bad subtree --> go back
					return;
				NewPos++;
			}
			// If we end up here, then it means that up to now, the words match
			// cout<<"Words match up to now"<<endl;
			if ( (unsigned char) (Word.GetLength()-WordPos) > m_ContentSize ) // if not the end of the word, continue the search forward
			{
				// cout<<"Not the end of the word, continue forward"<<endl;
				for(j = 0; j < m_NextLetters.GetSize(); j++)
				{   
					if (! m_NextLetters[j]->m_ContentSize)
						continue;

					if ( (unsigned char) Word[NewPos]  ==  m_NextLetters[j]->m_Content[0])
					{
						m_NextLetters[j]->FindPartialWordSensitive(Word, NewPos, 0, Result);
					}
				}
			}
			else // if it's the end of the word, then it means that we are ON the node corresponding to the word --> get the pages
			{
				GetSubVectors(Result);
			}
			return;
		}
		NewPos++;
	}

	// cout<<"Still in META, Going to next node"<<endl;
	if ( (unsigned char) (Word.GetLength()-WordPos) > m_ContentSize ) // It's not yet the end of the META part
	{
		for(i = 0; i < m_NextLetters.GetSize(); i++)
		{   
			if (! m_NextLetters[i]->m_ContentSize)
				continue;

			if ( Word[NewPos]  ==  m_NextLetters[i]->m_Content[0])
			{
				m_NextLetters[i]->FindPartialMetaSensitive(Word, NewPos, 0, Result);
			}
		}
	}
}










void CLetter::FindPartMetaInsensitive(const CString& Word, const unsigned int WordPos, const unsigned int Start, RESULTYPE& Result)
{
    register unsigned int i;
	unsigned int NewPos = WordPos;
	unsigned char Min;

	// cout<<"In FindPartMetaInsens for Word :"<<Word<<" OwnWord:"<<GetWord()<<endl;
	if ( (unsigned char) (Word.GetLength() - WordPos + Start) < m_ContentSize ) 
		Min = (unsigned char) (Word.GetLength() - WordPos + Start);
	else
		Min = m_ContentSize;

	for(i = Start;i<(unsigned int)Min;i++)
	{
		if (Word[NewPos] != m_Content[i]) // if the META part is not the same, we're not in the good subtree, go back
			return;
		else if ( ((unsigned char)Word[NewPos] == m_Content[i]) && (Word[NewPos] == ':') ) // if ':' is found, then it's the start of the word
		{
			// cout<<"META and Word are mixed, start InsWordSearch"<<endl;
			FindPartInsensitive(Word,NewPos+1,0, Result);
			return;
		}
		NewPos++;
	}

	// cout<<"Still in META, Going to next node"<<endl;
	if ( (unsigned char) (Word.GetLength()-WordPos) > m_ContentSize ) // It's not yet the end of the META part
	{
		for(i = 0; i < m_NextLetters.GetSize(); i++)
		{   
			if (! m_NextLetters[i]->m_ContentSize)
				continue;

			if ( Word[NewPos]  ==  m_NextLetters[i]->m_Content[0])
			{
				m_NextLetters[i]->FindPartMetaInsensitive(Word, NewPos, 0, Result);
			}
		}
	}
}

void CLetter::FindPartMetaSensitive(const CString& Word, const unsigned int WordPos, const unsigned int Start, RESULTYPE& Result)
{
    register unsigned int i;
	unsigned int NewPos = WordPos;
	unsigned char Min;

	// cout<<"In FindPartMetaInsens for Word :"<<Word<<" OwnWord:"<<GetWord()<<endl;
	if ( (unsigned char) (Word.GetLength() - WordPos + Start) < m_ContentSize ) 
		Min = (unsigned char) (Word.GetLength() - WordPos + Start);
	else
		Min = m_ContentSize;

	for(i = Start;i<(unsigned int)Min;i++)
	{
		if (Word[NewPos] != m_Content[i]) // if the META part is not the same, we're not in the good subtree, go back
			return;
		else if ( ((unsigned char)Word[NewPos] == m_Content[i]) && (Word[NewPos] == ':') ) // if ':' is found, then it's the start of the word
		{
			// We start directly on the same node, the META part will not match, but since we are in 'Part' mode, the search will continue
			FindPartSensitive(Word,NewPos+1,0, Result);
			return;
		}
		NewPos++;
	}

	// cout<<"Still in META, Going to next node"<<endl;
	if ( (unsigned char) (Word.GetLength()-WordPos) > m_ContentSize ) // It's not yet the end of the META part
	{
		for(i = 0; i < m_NextLetters.GetSize(); i++)
		{   
			if (! m_NextLetters[i]->m_ContentSize)
				continue;

			if ( Word[NewPos]  ==  m_NextLetters[i]->m_Content[0])
			{
				m_NextLetters[i]->FindPartMetaInsensitive(Word, NewPos, 0, Result);
			}
		}
	}
}

void CLetter::FindPartInsensitive(const CString& Word, const int WordPos, const unsigned int Start, RESULTYPE& Result)
{


    register unsigned int i;
	int j;
    int NewPos = WordPos;

	// cout<<"In FindPartInsens for Word :"<<Word<<" OwnWord:"<<GetWord()<<endl;
    if (m_ContentSize &&
		((unsigned int)WordPos  ==  Word.GetLength()-1) && 
		(CString::UCase((unsigned char)Word[WordPos])  ==  CString::UCase(m_Content[0])) )// If the first letter of the node is the last of the word, then we're done
    {
		GetSubVectors(Result);
    }
    else // if we're not yet at the end of the word, we continue searching
    {
        unsigned char chWord, chLetter;
		unsigned char Min;
		int LastFirstChar = -1;
		if ( (unsigned char) (Word.GetLength() - WordPos + Start) < m_ContentSize ) // Find the shortest string, so that we don't compare too far in the buffer
			Min = (unsigned char) (Word.GetLength() - WordPos + Start);
		else
			Min = m_ContentSize;

		for(i = Start;i<(unsigned int)m_ContentSize;i++) // compare all the letters
		{
			if (NewPos == (int)Word.GetLength())
			{
				GetSubVectors(Result);
				return;
			}
            chWord = CString::UCase((unsigned char) Word[NewPos]);
            chLetter = CString::UCase(m_Content[i]);			
			if (chLetter != chWord) // if there's a mismatch, we find the last char identical to the 1st letter of the word, or we continue with the subnodes
            {
                chWord = CString::UCase((unsigned char) Word[Word.Pos(':')+1]);
				int TempFirstChar = LastFirstChar;
				for(j = i;j>TempFirstChar;j--)
				{
					if (CString::UCase(m_Content[j]) == chWord)
					{
						i = (unsigned int)j;
						LastFirstChar = j;
					}
				}
				NewPos = WordPos-1;
            }
			NewPos++;
		}
		if ( (unsigned int)NewPos != Word.GetLength() )
		{
			for(i = 0; i < m_NextLetters.GetSize(); i++)
			{   
               m_NextLetters[i]->FindPartInsensitive(Word, NewPos, 0, Result);
			}
		}
		else
		{
			GetSubVectors(Result);
		}
    }    
}

void CLetter::FindPartSensitive(const CString& Word, const int WordPos, const unsigned int Start, RESULTYPE& Result)
{


    register unsigned int i;
	int j;
    int NewPos = WordPos;

	// cout<<"In FindPartInsens for Word :"<<Word<<" OwnWord:"<<GetWord()<<endl;
    if (m_ContentSize &&
		((unsigned int)WordPos  ==  Word.GetLength()-1) && 
		((unsigned char)Word[WordPos]  ==  m_Content[0]) )// If the first letter of the node is the last of the word, then we're done
    {
		GetSubVectors(Result);
    }
    else // if we're not yet at the end of the word, we continue searching
    {
		unsigned char Min;
		int LastFirstChar = -1;
		if ( (unsigned char) (Word.GetLength() - WordPos + Start) < m_ContentSize ) // Find the shortest string, so that we don't compare too far in the buffer
			Min = (unsigned char) (Word.GetLength() - WordPos + Start);
		else
			Min = m_ContentSize;

		for(i = Start;i<(unsigned int)m_ContentSize;i++) // compare all the letters
		{
			if (NewPos == (int)Word.GetLength())
			{
				GetSubVectors(Result);
				return;
			}
			if ((unsigned char) Word[NewPos] != m_Content[i]) // if there's a mismatch, we find the last char identical to the 1st letter of the word, or we continue with the subnodes
            {
				int TempFirstChar = LastFirstChar;
				for(j = i;j>TempFirstChar;j--)
				{
					if (m_Content[j] == (unsigned char) Word[Word.Pos(':')+1])
					{
						i = (unsigned int)j;
						LastFirstChar = j;
					}

				}
				NewPos = WordPos-1;
            }
			NewPos++;
		}
		if ( (unsigned int)NewPos != Word.GetLength() )
		{
			for(i = 0; i < m_NextLetters.GetSize(); i++)
			{   
               m_NextLetters[i]->FindPartSensitive(Word, NewPos, 0, Result);
			}
		}
		else
		{
			GetSubVectors(Result);
		}
    }    
}




void CLetter::AppendTo(CSearch * OtherSearcher,const CVector<int>& DispTable, CString& Buffer)
{
    register unsigned int i;
    
    int nBufferLen = Buffer.GetLength();
    Buffer.Append((char*)m_Content,(int)m_ContentSize);
    
    if ( m_PagesVector && m_PagesVector->GetSize() )
    {
        CVector<int> Pages;
		m_PagesVector->GetDispTable(Pages,DispTable);
        OtherSearcher->AddWord(Buffer, Pages);
    }
    
    for(i = 0; i < m_NextLetters.GetSize(); i++)
    {
        m_NextLetters[i]->AppendTo(OtherSearcher, DispTable, Buffer);
        Buffer.SetLength(nBufferLen + 1);
    }
    
    Buffer.SetLength(nBufferLen);
}


void CLetter::AddSubWord(const CString& Word, const unsigned int WordPos, const int Page, bool& IsNewWord, CLettersHandler *LettersHandler)
{
	unsigned int i;
	bool bAdded = false;

/*
	for(i = 0;i<WordPos;i++)
		cout<<" ";
	cout<<"AddSubWord Vector<int> - Adding Word :"<<&(Word[WordPos])<<" as leaf of : "<<GetWord()<<endl;
*/
	
	//cout<<"Looking for letter ["<<(unsigned char)Word[WordPos]<<"] as first letter of following nodes"<<endl;
	//cout<<"Existing nodes : "<<endl;
	//for(i = 0; i < m_NextLetters.GetSize(); i++)
	//	cout<<"["<<m_NextLetters[i]->GetWord()<<"]"<<endl;	
		
	for(i = 0; i < m_NextLetters.GetSize(); i++)
	{
		if (0 == (int) m_NextLetters[i]->m_ContentSize)
			continue;

		if (m_NextLetters[i]->m_Content[0]  ==  (unsigned char) Word[WordPos])
		{
			//cout<<"Found next node with same first letter"<<endl;
			m_NextLetters[i]->AddNewWord(Word, WordPos, Page, IsNewWord,LettersHandler);
			bAdded = true;
			break;
		}
		else
			{
				//cout<<"Next: ["<<m_NextLetters[i]->m_Content[0]<<"] Current: ["<<(unsigned char) Word[WordPos]<<"]"<<endl;
				//cout<<"Diff: "<<(int) (m_NextLetters[i]->m_Content[0] - ( char) Word[WordPos])<<endl;
			}
	}

	if (! bAdded)
	{		
		CLetter * pNewLetter = new CLetter;
        m_NextLetters += pNewLetter;
	    pNewLetter->SetContent((unsigned char*)&(Word[WordPos]),Word.GetLength()-WordPos, LettersHandler);

		CSwapVector *PagesVector = new CSwapVector;
		PagesVector->Add(Page);        
		pNewLetter->SetPagesVector(PagesVector);
		pNewLetter->AddCacheEntry(Page,WordPos);
		// s_FullNodeCount++;
		IsNewWord = true;
	}

}


void CLetter::AddSubWord(const CString& Word, const unsigned int WordPos, const CVector<int>& Pages, bool& IsNewWord, CLettersHandler *LettersHandler)
{
	unsigned int i;
	bool bAdded = false;

/*
	for(i = 0;i<WordPos;i++)
		cout<<" ";
	cout<<"AddSubWord Vector<int> - Adding Word :"<<&(Word[WordPos])<<" as leaf of : "<<GetWord()<<endl;
*/
	
	for(i = 0; i < m_NextLetters.GetSize(); i++)
	{
		if (! m_NextLetters[i]->m_ContentSize)
			continue;

		if (m_NextLetters[i]->m_Content[0] == (unsigned char)Word[WordPos])
		{
			m_NextLetters[i]->AddNewWord(Word, WordPos, Pages, IsNewWord,LettersHandler);
			bAdded = true;
			break;
		}
	}			
	if (!bAdded)
	{		
		CLetter * pNewLetter = new CLetter;
        m_NextLetters += pNewLetter;
	    pNewLetter->SetContent((unsigned char*)&(Word[WordPos]),Word.GetLength()-WordPos,LettersHandler);

		CSwapVector *PagesVector = new CSwapVector;
        for(i = 0; i < Pages.GetSize(); i++)
        {            
            PagesVector->Add(Pages[i]);        
        }

		pNewLetter->SetPagesVector(PagesVector);
		pNewLetter->AddCacheEntry(Pages,WordPos);
		// s_FullNodeCount++;
		IsNewWord = true;
	}
}

void CLetter::AddSubWord(const CString& Word, const unsigned int WordPos, const CSVector& Pages, bool& IsNewWord, CLettersHandler *LettersHandler)
{
	unsigned int i;
	bool bAdded = false;

/*
	for(i = 0;i<WordPos;i++)
		cout<<" ";
	cout<<"AddSubWord Vector<int> - Adding Word :"<<&(Word[WordPos])<<" as leaf of : "<<GetWord()<<endl;
*/
	
	for(i = 0; i < m_NextLetters.GetSize(); i++)
	{
		if (! m_NextLetters[i]->m_ContentSize)
			continue;

		if (m_NextLetters[i]->m_Content[0]  ==  (unsigned char)Word[WordPos])
		{
			m_NextLetters[i]->AddNewWord(Word, WordPos, Pages, IsNewWord,LettersHandler);
			bAdded = true;
			break;
		}
	}			
	if (!bAdded)
	{		
		CLetter * pNewLetter = new CLetter;
        m_NextLetters += pNewLetter;
	    pNewLetter->SetContent((unsigned char*)&(Word[WordPos]),Word.GetLength()-WordPos,LettersHandler);

		CSwapVector *PagesVector = new CSwapVector;
		PagesVector->Add(Pages);

		pNewLetter->SetPagesVector(PagesVector);
		pNewLetter->AddCacheEntry(*PagesVector,WordPos);
		// s_FullNodeCount++;
		IsNewWord = true;
	}
}


void CLetter::ShowTree(unsigned int Depth)
{
	unsigned int i;
	for(i = 0;i<Depth*2;i++)
		cout<<" ";
	cout<<"Word: "<<GetWord()<<" Pages:[";
	if (m_PagesVector != NULL)
	{
		m_PagesVector->Display();
	}
	cout<<"]"<<endl;
	for(i = 0; i < m_NextLetters.GetSize(); i++)
		m_NextLetters[i]->ShowTree(Depth+1);

}


// We need to cut the part of the node which is different and place it in a subnode
void CLetter::MoveOwnSubWord(const int StartPos, CLettersHandler *LettersHandler, unsigned int WordPos)
{
	register int i;
	// cout<<"MoveOwnSubWord - StartPos: "<<StartPos<<" Moving subword:";

	CLetter * pNewLetter = new CLetter;

	pNewLetter->SetContent(&(m_Content[StartPos]),((int)m_ContentSize)-StartPos,LettersHandler);

	for(i = StartPos;i<(int)m_ContentSize;i++)
	{
		LettersHandler->Remove((unsigned char)m_Content[i],this);
	}

	// We need to register again the current node, because if the same letters were in the beginning and the end of the node, the
	// current node will have been unregistered by the previous loop
	for(i = 0;i<StartPos;i++)
	{
		LettersHandler->Add((unsigned char)m_Content[i],this); 
	}

	pNewLetter->SetPagesVector(m_PagesVector);
//	pNewLetter->AddCacheEntry(*m_PagesVector,WordPos);
	pNewLetter->m_NextLetters = m_NextLetters;
	m_PagesVector = NULL;
	m_NextLetters.SetSize(0);
	m_NextLetters += pNewLetter;
	m_ContentSize = StartPos;  // We do not unallocate, but we cut the current string to the max. identical substring
}

void CLetter::AddNewWord(const CString& Word, unsigned int WordPos, const CSVector& Pages, bool& IsNewWord, CLettersHandler *LettersHandler)
{

	int CutPos = -1;
	unsigned int i;
	unsigned int InitPos = WordPos;
	
	CSwapVector Temp;
	Temp.Add(Pages);
	AddCacheEntry(Temp,WordPos);
	if ( (Word.GetLength()-InitPos) > (unsigned int)m_ContentSize)  // case where new word is bigger than current subword
	{
		for(i = 0;i<(unsigned int)m_ContentSize;i++)
		{
			if (m_Content[i] != (unsigned char)Word[WordPos])
			{
				CutPos = i;
				break;
			}
			WordPos++;
		}
		if (CutPos != -1)  // case where current node is smaller, and different
		{
			MoveOwnSubWord(CutPos,LettersHandler, WordPos);
			AddSubWord(Word,WordPos,Pages,IsNewWord,LettersHandler);
		}
		else  // case where current node is smaller and identical
		{
			AddSubWord(Word,WordPos,Pages,IsNewWord,LettersHandler);
		}
	}
	else if ( (Word.GetLength()-InitPos)  ==  (unsigned int)m_ContentSize)
	{
		// Need to verify that words are the same
		for(i = 0;i<(unsigned int)m_ContentSize;i++)
		{
			if (m_Content[i] != (unsigned char)Word[WordPos])
			{
				CutPos = i;
				break;
			}
			WordPos++;
		}
		if (CutPos != -1) // Words have same size, but are different
		{
			MoveOwnSubWord(CutPos,LettersHandler, WordPos);
			AddSubWord(Word,WordPos,Pages,IsNewWord,LettersHandler);
		}
        else
		{
			if (m_PagesVector  ==  NULL)
			{
				m_PagesVector = new CSwapVector;
				assert(m_PagesVector->GetSize()  ==  0);
				IsNewWord = true;
				// s_FullNodeCount++;
			}
			m_PagesVector->Add(Pages);
		}
	}
	else // m_ContentSize is bigger than new word
	{
		for(i = 0;i<(Word.GetLength()-InitPos);i++)
		{
			if (m_Content[i] != (unsigned char)Word[WordPos])
			{
				CutPos = i;
				break;
			}
			WordPos++;
		}
		if (CutPos != -1)  // case where current node is smaller, and different
		{
			MoveOwnSubWord(CutPos,LettersHandler, WordPos);
			AddSubWord(Word,WordPos,Pages,IsNewWord,LettersHandler);
		}
		else  // case where current node is smaller and identical
		{
			MoveOwnSubWord(Word.GetLength()-InitPos,LettersHandler, WordPos);
			if (m_PagesVector  ==  NULL)
			{
				m_PagesVector = new CSwapVector;
				assert(m_PagesVector->GetSize()  ==  0);
				IsNewWord = true;
				// s_FullNodeCount++;
			}
			m_PagesVector->Add(Pages);
		}
	}
}

void CLetter::AddNewWord(const CString& Word, unsigned int WordPos, const CVector<int>& Pages, bool& IsNewWord, CLettersHandler *LettersHandler)
{

	int CutPos = -1;
	unsigned int i;
	unsigned int InitPos = WordPos;

	AddCacheEntry(Pages,WordPos);

	if ( (Word.GetLength()-InitPos) > (unsigned int)m_ContentSize)  // case where new word is bigger than current subword
	{
		for(i = 0;i<(unsigned int)m_ContentSize;i++)
		{
			if (m_Content[i] != (unsigned char)Word[WordPos])
			{
				CutPos = i;
				break;
			}
			WordPos++;
		}
		if (CutPos != -1)  // case where current node is smaller, and different
		{
			MoveOwnSubWord(CutPos,LettersHandler, WordPos);
			AddSubWord(Word,WordPos,Pages,IsNewWord,LettersHandler);
		}
		else  // case where current node is smaller and identical
		{
			AddSubWord(Word,WordPos,Pages,IsNewWord,LettersHandler);
		}
	}
	else if ( (Word.GetLength()-InitPos)  ==  (unsigned int)m_ContentSize)
	{
		// Need to verify that words are the same
		
		for(i = 0;i<(unsigned int)m_ContentSize;i++)
		{
			if (m_Content[i] != (unsigned char)Word[WordPos])
			{
				CutPos = i;
				break;
			}
			WordPos++;
		}
		if (CutPos != -1) // Words have same size, but are different
		{
			MoveOwnSubWord(CutPos,LettersHandler, WordPos);
			AddSubWord(Word,WordPos,Pages,IsNewWord,LettersHandler);
		}
        else
		{
			if (m_PagesVector  ==  NULL)
			{
				m_PagesVector = new CSwapVector;
				assert(m_PagesVector->GetSize()  ==  0);
				IsNewWord = true;
				// s_FullNodeCount++;
			}
	        for(i = 0; i < Pages.GetSize(); i++)
		    {            
			    m_PagesVector->Add(Pages[i]);        
			}
		}
	}
	else // m_ContentSize is bigger than new word
	{
		for(i = 0;i<(Word.GetLength()-InitPos);i++)
		{
			if (m_Content[i] != (unsigned char)Word[WordPos])
			{
				CutPos = i;
				break;
			}
			WordPos++;
		}
		if (CutPos != -1)  // case where current node is smaller, and different
		{
			MoveOwnSubWord(CutPos,LettersHandler, WordPos);
			AddSubWord(Word,WordPos,Pages,IsNewWord,LettersHandler);
		}
		else  // case where current node is smaller and identical
		{
			MoveOwnSubWord(Word.GetLength()-InitPos,LettersHandler, WordPos);
			if (m_PagesVector  ==  NULL)
			{
				m_PagesVector = new CSwapVector;
				assert(m_PagesVector->GetSize()  ==  0);
				IsNewWord = true;
				// s_FullNodeCount++;
			}
	        for(i = 0; i < Pages.GetSize(); i++)
		    {            
			    m_PagesVector->Add(Pages[i]);        
			}
		}
	}
}


// BUGBUG: if (m_Content != NULL) there's a memory leak !!! but this should NOT happen since we never replace a word by another one
void CLetter::SetContent(unsigned char *SubWord, const int SubWordSize, CLettersHandler *LettersHandler)
{
	assert(m_Content  ==  NULL);

	/*
	cout<<"Added Word :";
	for(i = 0;i<SubWordSize;i++)
		cout<<SubWord[i];
	cout<<" SubWordSize:"<<SubWordSize<<endl;
	*/

	m_ContentSize = SubWordSize;
	m_Content = AllocBuf(m_ContentSize);

	assert(m_Content != NULL);

	for(register int i = 0; i < SubWordSize; i++)
	{
		m_Content[i] = SubWord[i];
		LettersHandler->Add((unsigned char)m_Content[i],this);
	}

// 	cout<<"Registered word: ["<<GetWord()<<"]"<<endl;
}

void CLetter::SetPagesVector(CSwapVector *NewPagesVector)
{
	if (m_PagesVector)
	{
		delete m_PagesVector;
        m_PagesVector=NULL;
	}
	m_PagesVector=NewPagesVector;
}


void CLetter::RecalcCache(int Depth, CSVector *LeafPages)
{
// 	cout<<"In RecalcCache for Depth "<<Depth<<endl;
	if ( ((Depth%s_CacheDepth) == 0) && (m_CacheVector == NULL) )
	{
		m_CacheVector = new CSVector;
		assert(m_CacheVector->GetSize()  ==  0);
	}
	

	for(unsigned int i = 0;i<m_NextLetters.GetSize();i++)
	{
		m_NextLetters[i]->RecalcCache(Depth+1,LeafPages);
		if ( ((Depth%s_CacheDepth) == 0) && (Depth != 0) )
			m_CacheVector->Add(*LeafPages);
	}
	if (m_CacheVector != NULL)
		LeafPages = m_CacheVector;
}

inline void CLetter::AddCacheEntry(int Page, unsigned int WordPos)
{
	/*
	if ((WordPos%s_CacheDepth) == 0)
	{
		cout<<"Adding int Cache entry at level "<<WordPos<<endl;
		if (m_CacheVector == NULL)
		{
			m_CacheVector = new CSVector;
			assert(m_CacheVector->GetSize()  ==  0);
		}
		m_CacheVector->Add(Page);
	}
	*/
}

inline void CLetter::AddCacheEntry(const CVector<int>& Pages, unsigned int WordPos)
{
/*
	
	if ((WordPos%s_CacheDepth) == 0)
	{
		cout<<"Adding CVector<int> Cache entry at level "<<WordPos<<endl;
		if (m_CacheVector == NULL)
		{
			m_CacheVector = new CSVector;
			assert(m_CacheVector->GetSize()  ==  0);
		}
		
		for(unsigned int i = 0;i<Pages.GetSize();i++)
			m_CacheVector->Add(Pages[i]);
	}
	*/
}

inline void CLetter::AddCacheEntry(const CSwapVector& Pages, unsigned int WordPos)
{
/*
	if ((WordPos%s_CacheDepth) == 0)
	{
		cout<<"Adding CSVector Cache entry at level "<<WordPos<<endl;
		if (m_CacheVector == NULL)
		{
			m_CacheVector = new CSVector;
			assert(m_CacheVector->GetSize()  ==  0);
		}
		
		m_CacheVector->Add(Pages);
	}
	*/
}


void CLetter::AddNewWord(const CString& Word, unsigned int WordPos, const int Page, bool& IsNewWord, CLettersHandler *LettersHandler)
{

	int CutPos = -1;
	unsigned int i;
	unsigned int InitPos = WordPos;

	AddCacheEntry(Page,WordPos);
	//cout<<"Adding word ["<<Word<<"] to current node, InitPos:"<<InitPos<<" ContentSize: "<<(unsigned int)m_ContentSize<<" Content: ["<<GetWord()<<"]"<<" Word Length: "<<Word.GetLength()<<endl;

	if ( (Word.GetLength()-InitPos) > (unsigned int)m_ContentSize)  // case where new word is bigger than current subword
	{
		//cout<<"Case 1"<<endl;
		for(i = 0;i<(unsigned int)m_ContentSize;i++)
		{
			if (m_Content[i] != (unsigned char)Word[WordPos])
			{
				CutPos = i;
				break;
			}
			WordPos++;
		}
		if (CutPos != -1)  // case where current node is smaller, and different
		{
			MoveOwnSubWord(CutPos,LettersHandler, WordPos);
			AddSubWord(Word,WordPos,Page,IsNewWord,LettersHandler);
		}
		else  // case where current node is smaller and identical
		{
			//cout<<"Current word is smaller and identical to beginning of word being added"<<endl;
			AddSubWord(Word,WordPos,Page,IsNewWord,LettersHandler);
		}
	}
	else if ( (Word.GetLength()-InitPos)  ==  (unsigned int)m_ContentSize)
	{
		//cout<<"Case 2"<<endl;
		// Need to verify that words are the same
		for(i = 0;i<(unsigned int)m_ContentSize;i++)
		{
			if (m_Content[i] != (unsigned char)Word[WordPos])
			{
				CutPos = i;
				break;
			}
			WordPos++;
		}
		if (CutPos != -1) // Words have same size, but are different
		{
			MoveOwnSubWord(CutPos,LettersHandler, WordPos);
			AddSubWord(Word,WordPos,Page,IsNewWord,LettersHandler);
		}
        else
		{
			//cout<<"Word is already present"<<endl;
			if (m_PagesVector  ==  NULL)
			{
				m_PagesVector = new CSwapVector;
				assert(m_PagesVector->GetSize()  ==  0);
				IsNewWord = true;
				// s_FullNodeCount++;
			}
		    m_PagesVector->Add(Page); 
			AddCacheEntry(Page,WordPos);
		}
	}
	else // m_ContentSize is bigger than new word
	{
		//cout<<"Case 3"<<endl;
		for(i = 0;i<(Word.GetLength()-InitPos);i++)
		{
			if (m_Content[i] != (unsigned char)Word[WordPos])
			{
				CutPos = i;
				break;
			}
			WordPos++;
		}
		if (CutPos != -1)  // case where current node is smaller, and different
		{	
			MoveOwnSubWord(CutPos,LettersHandler, WordPos);
			AddSubWord(Word,WordPos,Page,IsNewWord,LettersHandler);
		}
		else  // case where current node is smaller and identical
		{	
			MoveOwnSubWord(Word.GetLength()-InitPos,LettersHandler, WordPos);
			if (m_PagesVector  ==  NULL)
			{
				m_PagesVector = new CSwapVector;
				assert(m_PagesVector->GetSize()  ==  0);
				IsNewWord = true;
				// s_FullNodeCount++;
			}
		    m_PagesVector->Add(Page);        
			AddCacheEntry(Page,WordPos);
		}
	}
}



void CLetter::RemovePages(const CVectorInt& Pages,const  CVectorInt& PagesDeleted, unsigned int& WordsCount)
{
    register unsigned int i;  
    
    for(i = 0; i < m_NextLetters.GetSize(); i++) 
    {
        m_NextLetters[i]->RemovePages(Pages, PagesDeleted, WordsCount);
    }
    
    if (m_PagesVector != NULL)
    {
       
        for(i = 0; i < Pages.GetSize(); i++)
        {
            m_PagesVector->RemoveEx(Pages[i]);
        }
        for(i = 0; i < PagesDeleted.GetSize(); i++)
        {
            m_PagesVector->Remove(PagesDeleted[i]); 
        }
        
        if (m_PagesVector->GetSize()  ==  0)
        {            
			delete m_PagesVector;
            m_PagesVector = NULL;
            WordsCount--;
        }
    }
    
}


void CLetter::MakeAlreadyIndexed(CBitSet *AlreadyIndexedPages)
{
    register unsigned int i;
    if (m_PagesVector !=  NULL)
		m_PagesVector->SetAlreadyIndexed(AlreadyIndexedPages);
	    
    for(i = 0; i < m_NextLetters.GetSize(); i++)
    {
        m_NextLetters[i]->MakeAlreadyIndexed(AlreadyIndexedPages); 
    }
}










void CLetter::FindSensitiveGreaterThan(const CString& Word, unsigned int WordPos, const unsigned int Start, RESULTYPE& Result)
{
    register unsigned int i;
	unsigned int NewPos = WordPos;
	unsigned char Min;
// 	bool IsNegative = false;
	if ( (unsigned char) (Word.GetLength() - WordPos) < m_ContentSize ) 
		Min = (unsigned char) (Word.GetLength() - WordPos);
	else
		Min = m_ContentSize;

	for(i = Start;i<Min;i++)
	{
		if ((unsigned char)Word[NewPos] != m_Content[i]) // if the META part is not the same, we're not in the good subtree, go back
			return;
		else if (Word[NewPos] == '=')  // if '=' is found, then it's the start of the word
		{
			long Value = atoi(&Word[NewPos+1]);
			if ( (Value == 0) && (Word[NewPos+1] != '0') )
				return;
			GetGreaterThan(0,Value,false, i+1,Result);
			return;
		}
		NewPos++;
	}

	if ( (unsigned char) (Word.GetLength()-NewPos) > 0 ) // It's not yet the end of the META part
	{
		for(i = 0; i < m_NextLetters.GetSize(); i++)
		{   
			if (! m_NextLetters[i]->m_ContentSize)
				continue;

			if ( (unsigned char)Word[NewPos]  ==  m_NextLetters[i]->m_Content[0])
			{
				m_NextLetters[i]->FindSensitiveGreaterThan(Word, NewPos, 0, Result);
			}
		}
	}
}    
    



void CLetter::FindInsensitiveGreaterThan(const CString& Word, unsigned int WordPos, const unsigned int Start, RESULTYPE& Result)
{
    register unsigned int i;
	unsigned int NewPos = WordPos;
    unsigned char chWord, chLetter;
	unsigned char Min;
// 	bool IsNegative = false;

	if ( (unsigned char) (Word.GetLength() - WordPos) < m_ContentSize ) 
		Min = (unsigned char) (Word.GetLength() - WordPos);
	else
		Min = m_ContentSize;

	for(i = Start;i<Min;i++)
	{
		chWord = CString::UCase((unsigned char) Word[NewPos]);
		chLetter = CString::UCase(m_Content[i]);
		if (chWord != chLetter) // if the META part is not the same, we're not in the good subtree, go back
			return;
		else if (Word[NewPos] == '=')  // if '=' is found, then it's the start of the word
		{
			long Value = atoi(&Word[NewPos+1]);
			if ( (Value == 0) && (Word[NewPos+1] != '0') )
				return;
			GetGreaterThan(0,Value,false, i+1,Result);
			return;
		}
		NewPos++;
	}

	if ( (unsigned char) (Word.GetLength()-NewPos) > 0 ) // It's not yet the end of the META part
	{
		chWord = CString::UCase((unsigned char) Word[NewPos]);
		for(i = 0; i < m_NextLetters.GetSize(); i++)
		{   
			if (! m_NextLetters[i]->m_ContentSize)
				continue;
			chLetter = CString::UCase(m_NextLetters[i]->m_Content[0]);
			if ( chWord  ==  chLetter)
			{
				m_NextLetters[i]->FindInsensitiveGreaterThan(Word, NewPos, 0, Result);
			}
		}
	}
}





void CLetter::FindInsensitiveLowerThan(const CString& Word, unsigned int WordPos, const unsigned int Start, RESULTYPE& Result)
{
    register unsigned int i;
	unsigned int NewPos = WordPos;
    unsigned char chWord, chLetter;
	unsigned char Min;
// 	bool IsNegative = false;

	if ( (unsigned char) (Word.GetLength() - WordPos) < m_ContentSize ) 
		Min = (unsigned char) (Word.GetLength() - WordPos);
	else
		Min = m_ContentSize;

	for(i = Start;i<Min;i++)
	{
		chWord = CString::UCase((unsigned char) Word[NewPos]);
		chLetter = CString::UCase(m_Content[i]);
		if (chWord != chLetter) // if the META part is not the same, we're not in the good subtree, go back
			return;
		else if (Word[NewPos] == '=')  // if '=' is found, then it's the start of the word
		{
			long Value = atoi(&Word[NewPos+1]);
			if ( (Value == 0) && (Word[NewPos+1] != '0') )
				return;
			GetLowerThan(0,Value,false,i+1,Result);
			return;
		}
		NewPos++;
	}

	if ( (unsigned char) (Word.GetLength()-NewPos) > 0 ) // It's not yet the end of the META part
	{

		chWord = CString::UCase((unsigned char) Word[NewPos]);
		for(i = 0; i < m_NextLetters.GetSize(); i++)
		{   
			if (! m_NextLetters[i]->m_ContentSize)
				continue;

			chLetter = CString::UCase(m_NextLetters[i]->m_Content[0]);
			if ( chWord  ==  chLetter)
			{
				m_NextLetters[i]->FindInsensitiveLowerThan(Word, NewPos, 0, Result);
			}
		}
	}
}









void CLetter::FindSensitiveLowerThan(const CString& Word, unsigned int WordPos, const unsigned int Start, RESULTYPE& Result)
{
    register unsigned int i;
	unsigned int NewPos = WordPos;
	unsigned char Min;
// 	bool IsNegative = false;

	if ( (unsigned char) (Word.GetLength() - WordPos) < m_ContentSize ) 
		Min = (unsigned char) (Word.GetLength() - WordPos);
	else
		Min = m_ContentSize;

	for(i = Start;i<Min;i++)
	{
		if ((unsigned char)Word[NewPos] != m_Content[i]) // if the META part is not the same, we're not in the good subtree, go back
			return;
		else if (Word[NewPos] == '=')  // if '=' is found, then it's the start of the word
		{
			long Value = atoi(&Word[NewPos+1]);
			if ( (Value == 0) && (Word[NewPos+1] != '0') )
				return;
			GetLowerThan(0,Value,false,i+1,Result);
			return;
		}
		NewPos++;
	}

	if ( (unsigned char) (Word.GetLength()-NewPos) > 0 ) // It's not yet the end of the META part
	{
		for(i = 0; i < m_NextLetters.GetSize(); i++)
		{   
			if (! m_NextLetters[i]->m_ContentSize)
				continue;

			if ( (unsigned char)Word[NewPos]  ==  m_NextLetters[i]->m_Content[0])
			{
				m_NextLetters[i]->FindSensitiveLowerThan(Word, NewPos, 0, Result);
			}
		}
	}
}
    
    


void CLetter::GetLowerThan(long CurValue, const long FinalValue, const bool IsNegative, unsigned int Start, RESULTYPE& Result)
{
    register unsigned int i;

	bool bIsNegative = false;
	if (IsNegative == true)
		bIsNegative = true;

	if(CurValue<0)
		bIsNegative = true;

	if (Start >= (unsigned int)m_ContentSize)
	{
		for( i = 0; i < m_NextLetters.GetSize(); i++) 
        {
			if (! m_NextLetters[i]->m_ContentSize)
				continue;

            if ( (isdigit(m_NextLetters[i]->m_Content[0])) || (m_NextLetters[i]->m_Content[0] == '-') )
			{
				m_NextLetters[i]->GetLowerThan(0, FinalValue, bIsNegative, 0,Result);
			}
        }
		return;
	}
    else if (m_Content[Start]  ==  '-') 
    {
        CurValue = 0;
		bIsNegative = true;
    }
    else  if (isdigit(m_Content[Start]))
    {
		if (bIsNegative)
			CurValue = CurValue * 10 - (m_Content[Start] - 48);
		else
			CurValue = CurValue * 10 + (m_Content[Start] - 48);
    }    
	else return;

	for(i = Start+1;i<(unsigned int)m_ContentSize;i++)
	{
		if (isdigit(m_Content[i]))
		{
			if (bIsNegative)
				CurValue = CurValue * 10 - (m_Content[i] - 48);
			else
				CurValue = CurValue * 10 + (m_Content[i] - 48);
		}
		else return;
	}

    if  ( (CurValue < FinalValue) && (bIsNegative) )
    {
		GetSubVectors(Result);
	}
	else
	{
		if (CurValue < FinalValue) 
			GetOwnResults(Result);

		for( i = 0; i < m_NextLetters.GetSize(); i++) 
		{
			if (! m_NextLetters[i]->m_ContentSize)
				continue;

			if (isdigit(m_NextLetters[i]->m_Content[0]))
			{
				m_NextLetters[i]->GetLowerThan(CurValue, FinalValue, bIsNegative, 0,Result);
			}
		}	
	}

}




    
void CLetter::GetGreaterThan(long CurValue, const long FinalValue, const bool IsNegative, unsigned int Start, RESULTYPE& Result)
{
    register unsigned int i;
	bool bIsNegative = false;
	if (IsNegative == true)
		bIsNegative = true;

	if(CurValue<0)
		bIsNegative = true;

	if (Start >= (unsigned int)m_ContentSize)
	{
		for( i = 0; i < m_NextLetters.GetSize(); i++) 
        {
			if (! m_NextLetters[i]->m_ContentSize)
				continue;

			if ( (isdigit(m_NextLetters[i]->m_Content[0])) || (m_NextLetters[i]->m_Content[0]  ==  '-') )
			{
                m_NextLetters[i]->GetGreaterThan(0, FinalValue, bIsNegative, 0,Result);
			}
        }
		return;
	}
    else if (m_Content[Start]  ==  '-') 
    {
        CurValue = 0;
		bIsNegative = true;
    }
    else  if (isdigit(m_Content[Start]))
    {
		if (bIsNegative)
			CurValue = CurValue * 10 - (m_Content[Start] - 48);
		else
			CurValue = CurValue * 10 + (m_Content[Start] - 48);
    }    
	else return;

	for(i = Start+1;i<(unsigned int)m_ContentSize;i++)
	{
		if (isdigit(m_Content[i]))
		{
			if (bIsNegative)
				CurValue = CurValue * 10 - (m_Content[i] - 48);
			else
				CurValue = CurValue * 10 + (m_Content[i] - 48);
		}
		else return;
	}

    if ( (CurValue > FinalValue) && (!bIsNegative) )
    {
		GetSubVectors(Result);
    }
    else
    {
		if (CurValue > FinalValue) 
			GetOwnResults(Result);

        for( i = 0; i < m_NextLetters.GetSize(); i++) 
        {
			if (! m_NextLetters[i]->m_ContentSize)
				continue;

            if (isdigit(m_NextLetters[i]->m_Content[0]))
			{
                m_NextLetters[i]->GetGreaterThan(CurValue, FinalValue, bIsNegative, 0,Result);
			}
        }
    }
}





bool CLetter::RemoveWordsRegExp(const CVector<CRegExp>& Words, CString& CurrentBuffer, int& DeletedWords, CLettersHandler * LettersHandler) {
    CString * pBuffer = & CurrentBuffer;
    CString MetaBuffer;
	int i;
    CurrentBuffer.Append((char*)m_Content,(int)m_ContentSize);

    if ((m_NextLetters.GetSize()  ==  0) && (! m_PagesVector))
        return false;

    if (m_PagesVector != NULL) {
        
        // check whether the word matches
        
        for ( register unsigned int j = 0; j < Words.GetSize(); j++ ) {
            
            // match the individual pattern to the word
            
            bool bMatch = (((CRegExp&)Words[j]).RegFind(CurrentBuffer.GetBuffer())  ==  0) && 
                (((CRegExp&)Words[j]).GetFindLen()  ==  (int) CurrentBuffer.GetLength());
            
            if (bMatch) {
                // cout << "removing " << CurrentBuffer << ", matched with " << Words[j] << endl;
                delete m_PagesVector;
                m_PagesVector = NULL;
                DeletedWords++;                    
                break;
            }
        }
    }

	for( i = 0;i<(int)m_ContentSize;i++)
	{
		if (m_Content[i]  ==  ':')  
		{
			// this is a meta tag, ignore the name and continue with the rest of the words
			pBuffer = & MetaBuffer;
			if (i<m_ContentSize-1)
				pBuffer->Append((char*)&(m_Content[i+1]),(int)m_ContentSize-(i+1));
		} 
	}
	int j;
    for( i = (int) m_NextLetters.GetSize() - 1; i >= 0; i--) {
        // recurse for each letter
        
        if (! m_NextLetters[i]->RemoveWordsRegExp(Words, * pBuffer, DeletedWords, LettersHandler)) {
			for( j = 0; j<(int)m_NextLetters[i]->m_ContentSize;j++)
            LettersHandler->Remove(m_NextLetters[i]->m_Content[j], m_NextLetters[i]);
			delete m_NextLetters[i];
            m_NextLetters.RemoveAt(i);                
        }
        
    }

    CurrentBuffer.Delete(CurrentBuffer.GetLength() - (int)m_ContentSize, (int)m_ContentSize);

    return ((m_NextLetters.GetSize() != 0) || (m_PagesVector)) ? true : false;
}




bool CLetter::RemoveWord(const CString& Word, const unsigned int WordPos, const unsigned int Start, bool & bWasDeleted, CLettersHandler *LettersHandler)
{    

    bWasDeleted = false;

    register unsigned int i;
    unsigned int NewPos = WordPos;

    if (WordPos  ==  Word.GetLength()-1)
    {
		if (m_ContentSize>1) 
			return ((m_NextLetters.GetSize() != 0) || (m_PagesVector)) ? true : false;
        
		Trace(tagExcludeWords, levInfo, ("CLetter::RemoveWord - found word to remove [%s][%d][%s].", Word.GetBuffer(), WordPos, GetWord().GetBuffer()));

        if (m_PagesVector != NULL)
        {
            delete m_PagesVector;
            m_PagesVector = NULL;
            bWasDeleted = true;            
        }
    }
    else
    {
		unsigned char Min;
			
		if ( (unsigned char) (Word.GetLength() - WordPos) < m_ContentSize ) 
			Min = (unsigned char) (Word.GetLength() - WordPos);
		else
			Min = m_ContentSize;

		for(i = Start;i<Min;i++)
		{
			if ((unsigned char)Word[NewPos] != m_Content[i])
				return ((m_NextLetters.GetSize() != 0) || (m_PagesVector)) ? true : false;
			NewPos++;
		}
		if ( (unsigned char) (Word.GetLength()-WordPos) > m_ContentSize )
		{
			int j;
			for(i = 0; i < m_NextLetters.GetSize(); i++)
			{   
				if (! m_NextLetters[i]->m_ContentSize)
					continue;

				if ( (unsigned char) Word[NewPos]  ==  m_NextLetters[i]->m_Content[0])
				{
					if (! m_NextLetters[i]->RemoveWord(Word, WordPos + 1, 0, bWasDeleted, LettersHandler)) 
					{
						for( j = 0; j<(int)m_NextLetters[i]->m_ContentSize;j++)
							LettersHandler->Remove(m_NextLetters[i]->m_Content[j], m_NextLetters[i]);
						delete m_NextLetters[i];
						m_NextLetters.RemoveAt(i);  
					}
					break;
				}
			}
		}
		else if ( (unsigned char) (Word.GetLength()-WordPos)  ==  m_ContentSize )
		{
			if (m_PagesVector != NULL)
			{
	            delete m_PagesVector;
				m_PagesVector = NULL;
				bWasDeleted = true;            
			}
		}
    }    

	return ((m_NextLetters.GetSize() != 0) || (m_PagesVector)) ? true : false;
}


bool CLetter::RemoveWordsInMeta(const CVector<CString>& Words, int& DeletedWords, CLettersHandler *LettersHandler)
{
    int i, j;    
        
	i = 0;
	while( (i<m_ContentSize) && (m_Content[i] != ':') ) // try to find ':' in the current node
	{
		i++;
	}

	if ( (m_Content[i] == ':') && (i ==  ((int)m_ContentSize) - 1) ) // if ':' is the last letter of the node, we start with the leafs, not the current node
	{
		int k;
        for(j = 0; j < (int) Words.GetSize(); j++)
        {
            bool bWasDeleted = false;
            
            for(i = (int) m_NextLetters.GetSize() - 1; i >= 0; i--) 
            {
				if (! m_NextLetters[i]->m_ContentSize)
					continue;

				if (! Words[j].GetLength())
					continue;

                if (m_NextLetters[i]->m_Content[0]  ==  Words[j][0])
                {
                    if (! m_NextLetters[i]->RemoveWord(Words[j], 0, 0, bWasDeleted, LettersHandler)) 
                    {
						for( k = 0; k<(int)m_NextLetters[i]->m_ContentSize;k++)
						{
							LettersHandler->Remove(m_NextLetters[i]->m_Content[k], m_NextLetters[i]);
						}
						delete m_NextLetters[i];
                        m_NextLetters.RemoveAt(i);
                        break;
                    }
                }
            }
            
            if (bWasDeleted  ==  true) {
                DeletedWords++;
            }
        }
	}
	else if (m_Content[i] == ':') // if ':' is in the middle of the node, we start in the middle of the node, not with the leafs
	{
// 		bool NowEmpty = false;
        for(j = 0; j < (int) Words.GetSize(); j++)
        {
            bool bWasDeleted = false;
			RemoveWord(Words[j],0, i+1,bWasDeleted,LettersHandler);
            if (bWasDeleted  ==  true) {
                DeletedWords++;
            }
        }
	}
	else // if we didn't find ':' in the current node, go search in the leafs for it
	{
        for(i = (int) m_NextLetters.GetSize() - 1; i >= 0; i--)
        {
            if (! m_NextLetters[i]->RemoveWordsInMeta(Words, DeletedWords, LettersHandler)) 
            {
				for( j = 0; j<(int)m_NextLetters[i]->m_ContentSize;j++)
					LettersHandler->Remove(m_NextLetters[i]->m_Content[j], m_NextLetters[i]);
				delete m_NextLetters[i];
                m_NextLetters.RemoveAt(i);                
            }
        }
	}

    return ((m_NextLetters.GetSize() != 0) || (m_PagesVector)) ? true : false;
}

long CLetter::GetDataSize(void) const
{
	long lResult =
		sizeof(m_ContentSize) + // size of m_ContentSize
		m_ContentSize + // size of m_Content
		sizeof(int) + // sizeof(LeafNumber)
		sizeof(int) + // sizeof(PagesSize)
		sizeof(int) * (m_PagesVector ? m_PagesVector->GetSize() : 0) // pages vector size
		;

	for(int i = 0; i < (int) m_NextLetters.GetSize(); i++)
    {
        lResult += m_NextLetters[i]->GetDataSize();
    }

	return lResult;
}
