

#ifndef LETTERSHANDLER_HPP
#define LETTERSHANDLER_HPP

#include <platform/include.hpp>
#include <Vector/Vector.hpp>
#include <Mutex/RWMutex.hpp>

#define CHARSET_SIZE 256

class CLetter;

class CLettersHandler  
{
private:
	CVector<CLetter *> m_AllLetters[CHARSET_SIZE];
	CRWMutex m_LetterMutex[256];
public:
	CLettersHandler();
	~CLettersHandler();

	inline bool Add(const unsigned char Letter, CLetter *Node);
	inline void Remove(const unsigned char Letter,  CLetter *Node);
	inline CLetter* Value(const unsigned char Letter, const int Number);
	inline CLetter* ValueNonLocked(const unsigned char Letter, const int Number);
	inline const unsigned int GetSize(const unsigned char Letter);
	void RemoveAll();
	inline void StartWriting(const unsigned char Letter);
	inline void StopWriting(const unsigned char Letter);
	inline void StartReading(const unsigned char Letter);
	inline void StopReading(const unsigned char Letter);
};

inline void CLettersHandler::StartWriting(const unsigned char Letter)
{
	m_LetterMutex[Letter].StartWriting();
}
inline void CLettersHandler::StopWriting(const unsigned char Letter)
{
	m_LetterMutex[Letter].StopWriting();
}
inline void CLettersHandler::StartReading(const unsigned char Letter)
{
	m_LetterMutex[Letter].StartReading();
}
inline void CLettersHandler::StopReading(const unsigned char Letter)
{
	m_LetterMutex[Letter].StopReading();
}


inline void CLettersHandler::RemoveAll()
{
	for(int i=0;i<CHARSET_SIZE;i++)
	{
		m_LetterMutex[i].StartWriting();
		m_AllLetters[(unsigned char) i].RemoveAll();
		m_LetterMutex[i].StopWriting();
	}
}

bool CLettersHandler::Add(const unsigned char Letter, CLetter *Node)
{
	bool ReturnValue;
	m_LetterMutex[Letter].StartWriting();
	if (m_AllLetters[Letter].AddSortedUnique(Node))
		ReturnValue=true;
	else ReturnValue=false;
	m_LetterMutex[Letter].StopWriting();
	return ReturnValue;
}

void CLettersHandler::Remove(const unsigned char Letter,  CLetter *Node)
{
	int Pos;
	m_LetterMutex[Letter].StartWriting();
	if ( (Pos=m_AllLetters[Letter].FindSortedElt(Node)) != -1)
		m_AllLetters[Letter].RemoveAt(Pos);
	m_LetterMutex[Letter].StopWriting();
}
 

CLetter* CLettersHandler::ValueNonLocked(const unsigned char Letter, const int Number)
{
	return m_AllLetters[Letter][Number];
}

CLetter* CLettersHandler::Value(const unsigned char Letter, const int Number)
{
	CLetter* Value;
	m_LetterMutex[Letter].StartReading();
	Value=m_AllLetters[Letter][Number];
	m_LetterMutex[Letter].StopReading();
	return Value;
}

const unsigned int CLettersHandler::GetSize(const unsigned char Letter)
{
	unsigned int Value;
	m_LetterMutex[Letter].StartReading();
	Value=m_AllLetters[Letter].GetSize();
	m_LetterMutex[Letter].StopReading();
	return Value;
}



#endif
