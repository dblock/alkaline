#include <baseclasses.hpp>
#include "Allocator.hpp"

template <class T>
CAllocator<T>::CAllocator(const unsigned int Dim)
{
  m_Dim=Dim;
  m_AllEltsUsed=0xFFFFFFFF;
  m_IntSize=sizeof(int)*8;
  //  cout<<"Used: ";PrintBits(m_AllEltsUsed);
  m_BitFieldSize=(m_Dim/m_IntSize);
  m_EltsNumberPerSeg=m_IntSize*m_BitFieldSize;

  if ( (m_Dim % m_IntSize )!=0) m_BitFieldSize+=1;

  AddDataSegment();
}

template <class T>
CAllocator<T>::~CAllocator()
{
  unsigned int i;

  for(i=0;i<m_DataSegments.GetSize();i++)
    {
      delete[] m_DataSegments[i];
      delete[] m_BitFields[i];
    }
}


template <class T>
void CAllocator<T>::PrintBits(int Number)
{
  int j;
  int Mask=1;
  cout<<"[";
  for(j=0;j<32;j++)
    {
     
      if ( (Number&Mask)==Mask) cout<<"1";
      else cout<<"0";
      Mask<<=1;
    }
  cout<<"]"<<endl;

}

template <class T>
T* CAllocator<T>::GetNew()
{

  
  int FreeEltPos,j;
  unsigned int i;
  unsigned int Mask=1;
  unsigned int Rest32Mask=0x0000001F;
  //cout<<"Entering GetNew"<<endl;
  m_Mutex.Lock();

  for(i=0;i<m_DataSegments.GetSize();i++)
    {

      if (m_DataSegFirstFreeElt[i]!=-1) //If there's a free place in this segment
	{

	  if (m_SegmentIsLinear[i])
	    {
	      Mask<<=(m_NextFreeElt[i]&Rest32Mask);
	      m_BitFields[i][m_NextFreeElt[i]>>5]|=Mask;
	      if (m_BitFields[i][m_NextFreeElt[i]>>5]==m_AllEltsUsed) m_DataSegFirstFreeElt[i]++;
	      m_NextFreeElt[i]++;
	      if (m_NextFreeElt[i]==m_EltsNumberPerSeg)
		{
		  m_DataSegFirstFreeElt[i]=-1;

		  unsigned int k, FreeSpaceExist=0;
		  // cout<<"No more place in segment "<<i<<", Adding a new segment"<<endl;
		  for(k=i+1;k<m_DataSegments.GetSize();k++)
		    if ( m_DataSegFirstFreeElt[k]!=-1 )
		      FreeSpaceExist=1;
		  if (!FreeSpaceExist)
		    AddDataSegment();
		}
	      
	      m_Mutex.UnLock();
	      return &(m_DataSegments[i][m_NextFreeElt[i]-1]); //return the adress of the segment
	    }

	  FreeEltPos=m_DataSegFirstFreeElt[i]; //Store the int in the bitfield which marks the place
	  for(j=0;j<m_IntSize;j++)
	    {
	      
	      if ( (m_BitFields[i][FreeEltPos]&Mask) !=0 ) //Find the bit marking the free place
		{
		  
		  m_BitFields[i][FreeEltPos]|=Mask; //Mark the place as used
		  
		  if (m_BitFields[i][FreeEltPos]==m_AllEltsUsed) //If no more place in this int
		    {
		      m_DataSegFirstFreeElt[i]=FindNextFreeElt(i,FreeEltPos); //Find the pos of next free elt
		      if (m_DataSegFirstFreeElt[i]==-1) //If no more free elt in this segment, add a new segment
			{
			  unsigned int k, FreeSpaceExist=0;
			  // cout<<"No more place in segment "<<i<<", Adding a new segment"<<endl;
			  for(k=i+1;k<m_DataSegments.GetSize();k++)
			    if ( m_DataSegFirstFreeElt[k]!=-1 )
			      FreeSpaceExist=1;
			  if (!FreeSpaceExist)
			    AddDataSegment();
			}
		    }
		  //cout<<"Found free place in segment: "<<i<<" in bitfield: "<<FreeEltPos<<" at place "<<j<<endl;
		  m_Mutex.UnLock();
		  return &(m_DataSegments[i][j+(FreeEltPos<<5)]); //return the adress of the segment
		}
	      else 
		Mask=(Mask<<1);
	      
	      
	    }
	  m_Mutex.UnLock();
	  return NULL;
	}
    }
  m_Mutex.UnLock();
  return NULL;
  
}


template <class T>
void CAllocator<T>::FreeAll()
{
  unsigned int i;
  m_Mutex.Lock();
  for(i=0;i<m_DataSegments.GetSize();i++)
    {
      delete[] m_DataSegments[i];
      delete[] m_BitFields[i];
    }
  m_DataSegments.RemoveAll();
  m_DataSegFirstFreeElt.RemoveAll();
  m_BitFields.RemoveAll();
  m_SegmentIsLinear.RemoveAll();
  m_NextFreeElt.RemoveAll();
  m_Mutex.UnLock();
}




template <class T>
void CAllocator<T>::Free(T* Pointer)
{
  unsigned int i;

  T* LastPos;

  //cout<<"Freeing Elt"<<endl;
  fflush(NULL);
  unsigned int Number;
  m_Mutex.Lock();
  for(i=0;i<m_DataSegments.GetSize();i++)
    {
      LastPos=m_DataSegments[i]+((m_BitFieldSize<<5)-1);
      //cout<<"First: "<<(int)m_DataSegments[i]<<" Pointer: "<<(int)Pointer<<" Last: "<<(int)LastPos<<endl;
      	fflush(NULL);
      if ( (Pointer<=LastPos) && (Pointer>=m_DataSegments[i]) )
	{
	  Number=Pointer-m_DataSegments[i];
	  
	  //cout<<"Found Pointer to remove as being in segment: "<<i<<" Number: "<<Number<<" Sizeof(): "<<sizeof(T)<<endl;fflush(NULL);
	  RemoveElt(i,Number);
	  //cout<<"Finished Freeing Elt"<<endl;fflush(NULL);
	  //m_Mutex.UnLock();
	  //return;
	  //break;
	}
    }
  //  cout<<"WARNING: PASSING INVALID POINTER REFERENCE TO CAllocator.Free()"<<endl;fflush(NULL);
  m_Mutex.UnLock();
  
}

template <class T>
void CAllocator<T>::RemoveElt(int Segment, unsigned int Elt)
{
  unsigned int Mask=1;
  unsigned int FFFFMask=0xFFFFFFFF;
  unsigned int Rest32Mask=0x0000001F;
  int BitFieldNumber=Elt>>5;
  unsigned int BitNumber=Elt&Rest32Mask;
  Mask<<=BitNumber;
  Mask^=FFFFMask;
  //cout<<"Elt "<<Elt<<" is in Segment: "<<Segment<<" Field: "<<BitFieldNumber<<" Bit: "<<BitNumber<<endl;
  fflush(NULL);
  m_DataSegments[Segment][Elt].RemoveAll();
  m_SegmentIsLinear[Segment]=0;
  m_BitFields[Segment][BitFieldNumber]&=Mask;
  if ( (m_DataSegFirstFreeElt[Segment]>BitFieldNumber) || (m_DataSegFirstFreeElt[Segment]==-1) )
    m_DataSegFirstFreeElt[Segment]=BitFieldNumber;
  

}

template <class T>
bool CAllocator<T>::AddDataSegment()
{


  
 

  //    cout<<"m_Dim is: "<<m_Dim<<endl;
  //   cout<<"Sizeof(int) is: "<<sizeof(int)<<endl;
  //   cout<<"m_BitFieldSize is: "<<m_BitFieldSize<<endl;
  //   cout<<"Allocated Elements per segment is: "<<m_BitFieldSize*sizeof(int)*8<<endl;

  T *DataSegmentPointer=new T[m_BitFieldSize<<5];
  m_DataSegments+=DataSegmentPointer;
  m_DataSegFirstFreeElt+=0;
  m_SegmentIsLinear+=1;
  m_NextFreeElt+=0;

  int *BitFieldPointer=new int[m_BitFieldSize];
  m_BitFields+=BitFieldPointer;

  for(int i=0;i<m_BitFieldSize;i++)
    m_BitFields[m_BitFields.GetSize()-1][i]=0;
  return true;
}



