/*
    © Vestris Inc., Geneva, Switzerland
    http://www.vestris.com, 1994-1999 All Rights Reserved
    ______________________________________________

    written by Hassan Sultan - hsultan@vestris.com

*/
   
  
 
#ifndef ALKALINE_C_FIXEDALLOCATOR_HPP
#define ALKALINE_C_FIXEDALLOCATOR_HPP

#include <platform/include.hpp>
#include <Vector/Vector.hpp>
#include <Mutex/Mutex.hpp>
    
template<class T>
class CAllocBucket;
 
template<class T>
class CAllocBucket 
{  
private: 
	unsigned long m_ulAllocatedEltNum;
	T *m_pBase;
	T *m_pEnd;    
	T *m_pNextAllocatedElt; 
	T **m_pLastFreedElt;
	CVector<T*> m_OtherFreedPointers;
	CMutex m_Mutex;
	CAllocBucket<T> *m_pNextBucket;
public:
	CAllocBucket(const unsigned long ulEltNum);
	inline CAllocBucket<T>* GetNextBucket() { return m_pNextBucket;}
	inline bool IsInRange(T* pElt) { return (pElt<m_pEnd && pElt>=m_pBase) ? true : false; }
	T* GetNew();  
	CAllocBucket<T>* AddBucket();
	void FreeElt(T *tElt);
	~CAllocBucket(); 
};



template<class T>
class CFixedAllocator;

template<class T>
class CFixedAllocator  
{
private:
	unsigned long m_ulEltNum;
	CAllocBucket<T> m_Bucket; 
public:
	CFixedAllocator(const unsigned long ulNum);
	virtual ~CFixedAllocator(); 
	T* GetNew();
	void FreeElt(T *tElt);
};







template<class T>
class CObjAllocBucket;
 
template<class T>
class CObjAllocBucket 
{ 
private: 
	unsigned long m_ulAllocatedEltNum;
	T *m_pBase;
	T *m_pEnd;    
	T *m_pNextAllocatedElt; 
	T **m_pLastFreedElt;
	CVector<T*> m_OtherFreedPointers;
	CObjAllocBucket<T> *m_pNextBucket;
	CMutex m_Mutex;
//	void ConstructElt(T *tElt);
public:
	CObjAllocBucket(const unsigned long ulEltNum);
	inline CObjAllocBucket<T>* GetNextBucket() { return m_pNextBucket;}
	inline bool IsInRange(T* pElt) { return (pElt<m_pEnd && pElt>=m_pBase) ? true : false; }
	T* GetNew();  
	CObjAllocBucket<T>* AddBucket();
	void FreeElt(T *tElt);
	~CObjAllocBucket(); 
};



template<class T>
class CObjAllocator;

template<class T>
class CObjAllocator  
{
private:
	unsigned long m_ulEltNum;
	CObjAllocBucket<T> m_Bucket;
public:
	CObjAllocator(const unsigned long ulNum);
	virtual ~CObjAllocator();
	T* GetNew();
	void FreeElt(T *tElt);
};

#endif
