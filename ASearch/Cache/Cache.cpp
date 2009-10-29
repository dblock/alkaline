/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  _____________________________________________________
  
  written by Daniel Doubrovkine - dblock@vestris.com
  
*/

#include <alkaline.hpp>
#include "Cache.hpp"

CCache::CCache(int Size) : CObject() {
    m_Size = 0;
    m_Pool = 0;
    m_Occupation = 0;
    time(&m_CreateTime);
    SetSize(Size);
}

CCache::~CCache(void) {
    if (m_Pool) delete[] m_Pool;
}

void CCache::ClearPool(void) {
    m_Occupation = 0;
    for (int i=0;i<m_Size;i++)
        m_Pool[i].m_Used = 0;
}

void CCache::SetSize(int Size) {
    if (Size < 0) 
        return;    
    m_RWMutex.StartWriting();
    if (Size == 0) {        
        if (m_Pool) delete[] m_Pool;
        m_Pool = 0;
        m_Size = 0;
        m_Occupation = 0;
    } else {
        CCacheElement * NewPool = new CCacheElement[Size];
        for (register int i=0;i<Size;i++)
            NewPool[i].m_Used = 0;
        int j = 0;
        if (m_Occupation) {
            m_Occupation = 0;
            for (register int i=0;i<m_Size;i++) {
                if (j == Size) break;
                if (m_Pool[i].m_Used) {
                    NewPool[j] = m_Pool[i];
                    j++;
                    m_Occupation++;
                }
            }
        }
        delete[] m_Pool;
        m_Pool = NewPool;
        m_Size = Size;
    }
    m_RWMutex.StopWriting();
}

CCacheElement * CCache::FindBest(void) const {
    if (!m_Size) return 0;
    CCacheElement * l_Best = &(m_Pool[0]);
    for (register int i=0;i<m_Size;i++) {
        if (m_Pool[i].m_Used == 0) 
            return &(m_Pool[i]);
        else if (m_Pool[i].m_Requests.Get() < l_Best->m_Requests.Get()) 
            l_Best = &(m_Pool[i]);
    }
    return l_Best;
}

void CCache::Set(const CStringA& Word, const CSVector& Vector) {    
    if ((m_Occupation+1) < m_Size/2) SetSize(m_Size/2);
    else if ((m_Occupation+1) > (m_Size * 0.75)) SetSize(m_Size + (m_Size / 2) + 1);
    if (!m_Size)
        return;
    m_RWMutex.StartWriting();
    CCacheElement * l_Best = &(m_Pool[0]);
    if (m_Occupation)
        for (register int i=m_Size-1;i>=0;i--) {
            if (m_Pool[i].m_Used == 0) 
                l_Best = &(m_Pool[i]);
            else {
                if ((l_Best->m_Used) &&
                    (m_Pool[i].m_Requests.Get() < l_Best->m_Requests.Get()))
                    l_Best = &(m_Pool[i]);
                if (m_Pool[i].m_Word == Word) {
                    m_Pool[i].m_Pages = Vector;
                    m_Pool[i].m_Requests.Set(1);
                    time(&m_Pool[i].m_Time);
                    m_RWMutex.StopWriting();
                    return;
                }
            }
        }
    if (!l_Best->m_Used) 
        m_Occupation++;
    l_Best->m_Used = 1;
    l_Best->m_Pages = Vector;
    l_Best->m_Word = Word;
    l_Best->m_Requests.Set(1);
    time(&l_Best->m_Time);
    m_RWMutex.StopWriting();
}

void CCache::Invalidate(int Timeout) {
    time_t c_Time;
    time(&c_Time);
    m_RWMutex.StartWriting();
    for (register int i=m_Size-1;i>=0;i--) {
        if ((m_Pool[i].m_Used)&&(difftime(c_Time, m_Pool[i].m_Time) >= Timeout)) {
            m_Pool[i].m_Used = 0;
            m_Pool[i].m_Word.Empty();
            m_Pool[i].m_Pages.RemoveAll();
            m_Occupation--;
        }
    }
    m_RWMutex.StopWriting();
}

void CCache::Invalidate(const CStringA& Word) {
    m_RWMutex.StartWriting();
    for (register int i=m_Size-1;i>=0;i--) {
        if ((m_Pool[i].m_Used)&&(m_Pool[i].m_Word == Word)) {
            m_Pool[i].m_Used = 0;
            m_Pool[i].m_Word.Empty();
            m_Pool[i].m_Pages.RemoveAll();
            m_Occupation--;
            break;
        }
    }
    m_RWMutex.StopWriting();
}


void CCache::InvalidateAll() {
    m_RWMutex.StartWriting();
    for (register int i=m_Size-1;i>=0;i--) {
            m_Pool[i].m_Used = 0;
            m_Pool[i].m_Word.Empty();
            m_Pool[i].m_Pages.RemoveAll();
            m_Occupation--;
    }
    m_RWMutex.StopWriting();
}


bool CCache::Find(const CStringA& Word, CSVector& Target) const {
    m_Requests.Inc();
    m_RWMutex.StartReading();    
    bool Result = false;
    for (register int i=0;i<m_Size;i++)
        if ((m_Pool[i].m_Used)&&(m_Pool[i].m_Word == Word)) {
            Target = m_Pool[i].m_Pages;
            m_Pool[i].m_Requests.Inc();
            m_Hits.Inc();
            Result = true;
        }
    m_RWMutex.StopReading();
    return Result;
}

#define CACHE_MOD_STAT 5

void CCache::PopulateXmlTree(CXmlTree& XmlTree, const CString& Path) const {
  CCacheElement * UsedPool = new CCacheElement[CACHE_MOD_STAT];
  int UsedOccupation = 0;
  m_RWMutex.StartReading();
  for (register int i=0;i<m_Size;i++) {
    if ((m_Pool[i].m_Used)&&(!(m_Pool[i].m_Word.GetTagPos()))) {
      if (UsedOccupation < CACHE_MOD_STAT) {
        UsedPool[UsedOccupation++] = m_Pool[i];
      } else {
        int MinElement = 0;
        for (int j=1;j<CACHE_MOD_STAT;j++) {
          if (UsedPool[MinElement].m_Requests.Get() > UsedPool[j].m_Requests.Get()) 
            MinElement = j;
        }
        if (m_Pool[i].m_Requests.Get() > UsedPool[MinElement].m_Requests.Get()) 
          UsedPool[MinElement] = m_Pool[i];
      }
    }
  }
  m_RWMutex.StopReading();

  CString Result;
  for (register int l=0;l<UsedOccupation;l++) {
    if (l)
      Result += ", ";
    Result += (UsedPool[l].m_Word + " (" + CString::IntToStr(UsedPool[l].m_Requests.Get()) + ")");
  }
  if (!Result.GetLength())
    Result = "empty";
  delete[] UsedPool;
  XmlTree.SetValue(Path, Result);
}
