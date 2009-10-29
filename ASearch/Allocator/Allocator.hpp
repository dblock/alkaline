/*
  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  ______________________________________________

  written by Daniel Doubrovkine - dblock@vestris.com


  Revision history:

  28.08.2000: First release

*/

#ifndef ALLOCATOR_HPP
#define ALLOCATOR_HPP

#include <platform/include.hpp>
#include <Vector/Vector.hpp>
#include <Mutex/Mutex.hpp>

template <class T>
class CAllocator 
{
public:
  CAllocator(const unsigned int Dim);
  ~CAllocator();
  T* GetNew();
  void Free(T* Pointer);
  void PrintBits(int Number);
  void FreeAll();
private:
  int m_IntSize;
  int m_EltsNumberPerSeg;
  CMutex m_Mutex;
  unsigned int m_Dim;
  int m_AllEltsUsed;
  int m_BitFieldSize;
  CVector<T*> m_DataSegments;
  CVector<int> m_DataSegFirstFreeElt;
  CVector<int*> m_BitFields;
  CVector<int> m_SegmentIsLinear;
  CVector<int> m_NextFreeElt;
  bool AddDataSegment();
  void RemoveElt(int Segment, unsigned int Elt);
  inline int FindNextFreeElt(const int Number, const int BasePos);
};


template <class T>
inline int CAllocator<T>::FindNextFreeElt(const int Segment, const int BasePos)
{
  register int i;
  for(i=BasePos;i<m_BitFieldSize;i++)
    if ( (m_BitFields[Segment][i]&m_AllEltsUsed) != m_AllEltsUsed)
      return i;

  return -1;
}

#endif
