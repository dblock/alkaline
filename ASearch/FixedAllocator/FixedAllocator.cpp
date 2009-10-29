// Allocator.cpp: implementation of the CAllocator class.
//
//////////////////////////////////////////////////////////////////////

#include <alkaline.hpp>
#include "FixedAllocator.hpp"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
template <class T>
CAllocBucket<T>::CAllocBucket(const unsigned long ulEltNum)
{
	m_ulAllocatedEltNum=ulEltNum;
	if (m_ulAllocatedEltNum<4)
		m_ulAllocatedEltNum=1024;

	m_pBase=new T[m_ulAllocatedEltNum];
//	cout<<"Allocated "<<m_ulAllocatedEltNum<<" Elts"<<endl;
	m_pEnd=m_pBase+m_ulAllocatedEltNum;
	m_pLastFreedElt=(T**)m_pEnd;
//	cout<<"m_pBase: "<<(void*)m_pBase<<" m_pEnd: "<<(void*)m_pEnd<<" Difference: "<<(m_pEnd-m_pBase)<<" EltSize: "<<sizeof(T)<<endl;
	m_pNextAllocatedElt=(T*)m_pBase;
	m_OtherFreedPointers.SetDim(128);
	m_pNextBucket=NULL;
}

template <class T>
CAllocBucket<T>::~CAllocBucket()
{
	delete[] m_pBase;
	m_pBase=NULL;
	m_pEnd=NULL;
	delete m_pNextBucket;
	m_pNextBucket=NULL;
}

template <class T>
T* CAllocBucket<T>::GetNew()
{

	T * pReturnValue = NULL;

	m_Mutex.Lock();

	if ((void*)m_pLastFreedElt<(void*)m_pEnd) //case where we have stored addresses of freed elts in the block
	{
		pReturnValue = *m_pLastFreedElt;	
		m_pLastFreedElt++;
		//cout<<"Gave back freed Elt from block: "<<(void*)pReturnValue<<" m_pLastFreedElt: "<<(void*)m_pLastFreedElt<<endl;
	}
	else if (m_OtherFreedPointers.GetSize()!=0) //case where we still have freed elts in the vector
	{
		pReturnValue = m_OtherFreedPointers[m_OtherFreedPointers.GetSize()-1];
		m_OtherFreedPointers.RemoveAt(m_OtherFreedPointers.GetSize()-1);
		//cout<<"Gave back freed Elt from Vector: "<<(void*)pReturnValue<<" Vector Size: "<<m_OtherFreedPointers.GetSize()<<endl;
	}
	else //case where there's no freed elt to give back, we allocate a fresh new elt
	{
		if (m_pNextAllocatedElt!=m_pEnd)
		{
			//cout<<"Giving fresh new Elt: "<<(void*)m_pNextAllocatedElt<<endl;
			pReturnValue = m_pNextAllocatedElt;
			m_pNextAllocatedElt++;
		}
		else 
		{
			//cout<<"No more Elts, m_pBase: "<<(void*)m_pBase<<" m_pEnd: "<<(void*)m_pEnd<<" m_pLastFreedElt: "<<(void*)m_pLastFreedElt<<" Vector Size: "<<m_OtherFreedPointers.GetSize()<<endl;
			pReturnValue = NULL;
		}
	}

	m_Mutex.UnLock();
	return pReturnValue;
}


template <class T>
void CAllocBucket<T>::FreeElt(T *tElt)
{
	m_Mutex.Lock();
	if ((void*)m_pNextAllocatedElt<(void*)(m_pLastFreedElt-1)) //we still have room at the end of the block to store the freed elt info
	{
		//cout<<"Adding freed Elt: "<<(void*) tElt <<" to the end of the block"<<endl;
		//cout<<"m_pLastFreedElt was : "<<(void*)m_pLastFreedElt<<endl;
		m_pLastFreedElt--;
		//cout<<"m_pLastFreedElt is now : "<<(void*)m_pLastFreedElt<<endl;
		*m_pLastFreedElt=tElt;

	}
	else //we need to store it in the freed elts vector
	{
		//cout<<"Adding freed Elt: "<<(void*) tElt <<" to the freed elts vector (m_pNextAllocatedElt: "<<(void*)m_pNextAllocatedElt<<" m_pLastFreedElt: "<<(void*)m_pLastFreedElt<<endl;
		m_OtherFreedPointers+=tElt;
	}
	m_Mutex.UnLock();
}

template <class T>
CAllocBucket<T>* CAllocBucket<T>::AddBucket()
{
	m_Mutex.Lock();
	if (m_pNextBucket == NULL)
	{
//		cout<<"Adding new Bucket"<<endl;
		CAllocBucket<T>* pReturnValue;
		m_pNextBucket = new CAllocBucket<T>(m_ulAllocatedEltNum);
		pReturnValue = m_pNextBucket;
		m_Mutex.UnLock();
		return pReturnValue;
	}
	else
	{
		m_Mutex.UnLock();
		return m_pNextBucket->AddBucket();
	}
}









template <class T>
CFixedAllocator<T>::CFixedAllocator(const unsigned long ulEltNumber) : m_Bucket(ulEltNumber)
{
	m_ulEltNum=ulEltNumber;
}

template <class T>
CFixedAllocator<T>::~CFixedAllocator()
{

}


template <class T>
T* CFixedAllocator<T>::GetNew()
{
	T* pRetValue;
	CAllocBucket<T> *CurrentBucket=&m_Bucket;
	while ( ( pRetValue=CurrentBucket->GetNew() ) == NULL )
	{
		if (CurrentBucket->GetNextBucket()==NULL)
			CurrentBucket->AddBucket();
		CurrentBucket=CurrentBucket->GetNextBucket();
	}
	return pRetValue;
}

template <class T>
void CFixedAllocator<T>::FreeElt(T *tElt)
{
	CAllocBucket<T> *CurrentBucket=&m_Bucket;
	while ( false==CurrentBucket->IsInRange(tElt) ) 
	{
		//cout<<"Elt not in this bucket, going to the next one"<<endl;
		CurrentBucket=CurrentBucket->GetNextBucket();
	}
	if (CurrentBucket!=NULL)
		CurrentBucket->FreeElt(tElt);
}






///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////


template <class T>
CObjAllocBucket<T>::CObjAllocBucket(const unsigned long ulEltNum)
{
	m_ulAllocatedEltNum=ulEltNum;
	if (m_ulAllocatedEltNum<4)
		m_ulAllocatedEltNum=1024;

	//m_pBase=new T[m_ulAllocatedEltNum];
	m_pBase=(T*)malloc(sizeof(T)*m_ulAllocatedEltNum);
//	cout<<"Allocated "<<m_ulAllocatedEltNum<<" Elts"<<endl;
	m_pEnd=m_pBase+m_ulAllocatedEltNum;
	m_pLastFreedElt=(T**)m_pEnd;
//	cout<<"m_pBase: "<<(void*)m_pBase<<" m_pEnd: "<<(void*)m_pEnd<<" Difference: "<<(m_pEnd-m_pBase)<<" EltSize: "<<sizeof(T)<<endl;
	m_pNextAllocatedElt=(T*)m_pBase;
	m_OtherFreedPointers.SetDim(128);
	m_pNextBucket=NULL;
}

template <class T>
CObjAllocBucket<T>::~CObjAllocBucket()
{
	//delete[] m_pBase;
	free(m_pBase);
	m_pBase=NULL;
	m_pEnd=NULL;
	delete m_pNextBucket;
	m_pNextBucket=NULL;
}

template <class T>
T* CObjAllocBucket<T>::GetNew()
{

	T * pReturnValue = NULL;
	m_Mutex.Lock();

	if ((void*)m_pLastFreedElt<(void*)m_pEnd) //case where we have stored addresses of freed elts in the block
	{
		pReturnValue = * m_pLastFreedElt;	
		m_pLastFreedElt++;
//		cout<<"Gave back freed Elt from block: "<<(void*)pReturnValue<<" m_pLastFreedElt: "<<(void*)m_pLastFreedElt<<endl;		
	}
	else if (m_OtherFreedPointers.GetSize()!=0) //case where we still have freed elts in the vector
	{
		pReturnValue = m_OtherFreedPointers[m_OtherFreedPointers.GetSize()-1];
		m_OtherFreedPointers.RemoveAt(m_OtherFreedPointers.GetSize()-1);
//		cout<<"Gave back freed Elt from Vector: "<<(void*)pReturnValue<<" Vector Size: "<<m_OtherFreedPointers.GetSize()<<endl;
	}
	else //case where there's no freed elt to give back, we allocate a fresh new elt
	{
		if (m_pNextAllocatedElt!=m_pEnd)
		{
			pReturnValue = m_pNextAllocatedElt;
			m_pNextAllocatedElt++;
//			cout<<"Giving fresh new Elt: "<<(void*)m_pNextAllocatedElt<<endl;
		}
		else 
		{
			//cout<<"No more Elts, m_pBase: "<<(void*)m_pBase<<" m_pEnd: "<<(void*)m_pEnd<<" m_pLastFreedElt: "<<(void*)m_pLastFreedElt<<" Vector Size: "<<m_OtherFreedPointers.GetSize()<<endl;
			pReturnValue = NULL; //no more room in this bucket
		}
	}

	m_Mutex.UnLock();
	
	if (pReturnValue)
	{		
		::new((void*)pReturnValue) T;
	}
	
	return pReturnValue;
}

/*
template<class T>
void CObjAllocBucket<T>::ConstructElt(T *tElt)
{
	tElt->T();
}

  */

template <class T>
void CObjAllocBucket<T>::FreeElt(T *tElt)
{

	m_Mutex.Lock();
	if ((void*)m_pNextAllocatedElt<(void*)(m_pLastFreedElt-1)) //we still have room at the end of the block to store the freed elt info
	{
//		cout<<"Adding freed Elt: "<<(void*) tElt <<" to the end of the block"<<endl;
//		cout<<"m_pLastFreedElt was : "<<(void*)m_pLastFreedElt<<endl;
		m_pLastFreedElt--;
//		cout<<"m_pLastFreedElt is now : "<<(void*)m_pLastFreedElt<<endl;
		*m_pLastFreedElt=tElt;
		tElt->~T();
	}
	else //we need to store it in the freed elts vector
	{
//		cout<<"Adding freed Elt: "<<(void*) tElt <<" to the freed elts vector (m_pNextAllocatedElt: "<<(void*)m_pNextAllocatedElt<<" m_pLastFreedElt: "<<(void*)m_pLastFreedElt<<endl;
		m_OtherFreedPointers+=tElt;
		tElt->~T();
	}
	m_Mutex.UnLock();
}

template <class T>
CObjAllocBucket<T>* CObjAllocBucket<T>::AddBucket()
{
	m_Mutex.Lock();
	if (m_pNextBucket==NULL)
	{
//		cout<<"Adding new Bucket"<<endl;
		m_pNextBucket=new CObjAllocBucket<T>(m_ulAllocatedEltNum);
		m_Mutex.UnLock();
		return m_pNextBucket;
	}
	else
	{
		m_Mutex.UnLock();
		return m_pNextBucket->AddBucket();
	}
}

template <class T>
CObjAllocator<T>::CObjAllocator(const unsigned long ulEltNumber) : m_Bucket(ulEltNumber)
{
	m_ulEltNum=ulEltNumber;
}

template <class T>
CObjAllocator<T>::~CObjAllocator()
{

}


template <class T>
T* CObjAllocator<T>::GetNew()
{
	T* pRetValue;
	CObjAllocBucket<T> *CurrentBucket=&m_Bucket;
	while ( ( pRetValue=CurrentBucket->GetNew() ) == NULL )
	{
		if (CurrentBucket->GetNextBucket()==NULL)
			CurrentBucket->AddBucket();
		CurrentBucket=CurrentBucket->GetNextBucket();
	}
	return pRetValue;
}

template <class T>
void CObjAllocator<T>::FreeElt(T *tElt)
{
	CObjAllocBucket<T> *CurrentBucket=&m_Bucket;
	while ( false==CurrentBucket->IsInRange(tElt) ) 
	{
//		cout<<"Elt not in this bucket, going to the next one"<<endl;
		CurrentBucket=CurrentBucket->GetNextBucket();
	}
	if (CurrentBucket!=NULL)
		CurrentBucket->FreeElt(tElt);
}
