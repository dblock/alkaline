#include <alkaline.hpp>
#include "LettersTree.hpp"

CLettersTree::CLettersTree()
{
	m_OwnWordsCount=0;
}

CLettersTree::~CLettersTree()
{
  
}

bool CLettersTree::Serialize(CMMapFile& IndexFile) 
{
	// assume global lock
	bool fResult = m_FirstLetter.Serialize(IndexFile);

	if (! fResult) {
		cerr << "[error:ltsr]" << endl;
	}

	return fResult;
}
