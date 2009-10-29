/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  ______________________________________________
  
  written by Daniel Doubrovkine - dblock@vestris.com
  
*/

#ifndef ALKALINE_C_CACHE_HPP
#define ALKALINE_C_CACHE_HPP

#include <platform/include.hpp>
#include <String/String.hpp>
#include <StringA/StringA.hpp>
#include <Vector/Vector.hpp>
#include <Vector/SVector.hpp>
#include <Io/Io.hpp>
#include <Mutex/Mutex.hpp>
#include <Tree/XmlTree.hpp>
#include <Mutex/Atomic.hpp>
#include <Mutex/RWMutex.hpp>

typedef struct _CCacheElement {
    time_t m_Time;
    long m_Used;
    CAtomic m_Requests;
    CStringA m_Word;
    CSVector m_Pages;
} CCacheElement;

class CCache : public CObject {
    readonly_property(int, Size);
    property(time_t, CreateTime);
    mutable_property(CAtomic, Requests);
    mutable_property(CAtomic, Hits);
    mutable_property(CRWMutex, RWMutex);
private:
    int m_Occupation;
    CCacheElement * m_Pool;
    CCacheElement * FindBest(void) const;
    void ClearPool(void);
    void SetSize(int Size);
    inline static double MaxDouble(const double& First, const double& Second) { return ((First>Second)?(First):(Second)); }
public:
    void Invalidate(const CStringA& Word);
    void Invalidate(int Timeout = 60);
    void InvalidateAll();
    CCache(int Size = 0);
    ~CCache(void);
    void Set(const CStringA& Word, const CSVector& Vector);
    bool Find(const CStringA& Word, CSVector& Target) const;
    inline int GetRequestsPerMinute(void) const { return (int) ((double) m_Requests.Get() / (MaxDouble(1, (difftime(time(0), m_CreateTime))/60))); }
    inline int GetHitRate(void) const { if (m_Requests.Get()) return (int) ((double) (m_Hits.Get() * 100) / m_Requests.Get()); else return 100; }
    inline int GetOccupation(void) const { if (m_Size) return (int) ((double)(m_Occupation * 100) / m_Size); else return 100; }    
    void PopulateXmlTree(CXmlTree& XmlTree, const CString& Path) const ;
};

#endif
