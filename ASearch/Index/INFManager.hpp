/*
    © Vestris Inc., Geneva, Switzerland
    http://www.vestris.com, 1994-1999 All Rights Reserved
    ______________________________________________

    written by Daniel Doubrovkine - dblock@vestris.com

*/

#ifndef ALKALINE_INF_MANAGER_HPP
#define ALKALINE_INF_MANAGER_HPP

#include "IndexManager.hpp"
#include <Tree/Md5Tree.hpp>
#include <Mutex/RWMutex.hpp>

class CIndex;

class CINFElement {

    property(CVector<CString>, Fields);
    property(struct tm, tmTime);
    property(time_t, ttTime);
    property(int, DocumentSize);
    
public:
    
    CINFElement(void);
    ~CINFElement(void);
    CINFElement(const CVector<CString>& Vector);
    CINFElement(const CINFElement&);
    CINFElement& operator=(const CINFElement&);
    
    inline bool operator>(const CINFElement& Other) const { return m_Fields > Other.m_Fields; }
    inline bool operator<(const CINFElement& Other) const { return m_Fields < Other.m_Fields; }
    inline bool operator>=(const CINFElement& Other) const { return m_Fields >= Other.m_Fields; }
    inline bool operator<=(const CINFElement& Other) const { return m_Fields <= Other.m_Fields; }
    inline bool operator==(const CINFElement& Other) const { return m_Fields == Other.m_Fields; }
    inline bool operator!=(const CINFElement& Other) const { return m_Fields != Other.m_Fields; }
    inline ostream& operator<<(ostream& Stream) const { return Stream << m_Fields; }
    friend inline ostream& operator<<(ostream& Stream, const CINFElement& Element);

    inline int GetSize(void) const { return m_Fields.GetSize(); }
    inline const CString& operator[](int Index) const { return m_Fields[Index]; }
    inline CString& operator[](int Index) { return m_Fields[Index]; }

    void Update(void);
};

inline ostream& operator<<(ostream& Stream, const CINFElement& Element) {
    return Element.operator <<(Stream);
}

class CINFManager : public CIndexManager {
    CVector< CINFElement > m_INFVector;
    property(CMd5Tree, Md5Tree);    
    bool CINFManager::LoadINFVector(bool Verbose);
public:
    CINFManager(void);
    virtual ~CINFManager(void);
    inline time_t GetTimeT(int Index) const { return m_INFVector[Index].GetttTime(); }
    inline int GetDocumentSize(int Index) const { return m_INFVector[Index].GetDocumentSize(); }
    inline CINFElement& GetAt(int Index) { return m_INFVector[Index]; }
    inline const CVector<CString>& Get(int Index) const { return m_INFVector[Index].GetFields(); }
    inline const CVector<CString>& operator[](int Index) const { return Get(Index); }
    inline int GetSize(void) const { return m_INFVector.GetSize(); }
    inline int GetSize(int Index) const { return m_INFVector[Index].GetSize(); }   
    bool Load(const CString& Filename, bool Verbose = false, bool bMakeMd5Tree = true);
    inline void PutAt(const CString& String, int X, int Y);
    inline void PutAt(const CVector<CString>& Vector, int X) { m_INFVector[X].SetFields(Vector); m_INFVector[X].Update(); }
    inline void Add(const CVector<CString>& Vector) { CINFElement Element(Vector); m_INFVector.Add(Element); }
    inline void Grow(int NewDimension) { m_INFVector.SetDim(NewDimension); }
    inline void EnsureSize(int NewSize) { if ((int) m_INFVector.GetSize() < NewSize) m_INFVector.SetSize(NewSize); }
    bool Write(bool Verbose = true, int DataRows = 0);
	long GetDataSize(int DataRows = 0) const;
    void RemoveAll(void);
    CString GetModifiedSince(int UrlPos) const;
    /* Append an another INF manager */
    void Append(CINFManager& Manager, const CVector<int>& DispTable, CIndex * Parent);
    inline int GetDuplicateMd5(const CString& Md5String, CIntVector& pVector) { return m_Md5Tree.FindNodeIndexes(Md5String, pVector); }
    inline void SaveMd5(const CString& Md5String, int Index) { m_Md5Tree.Add(Md5String, Index); }
};

inline void CINFManager::PutAt(const CString& String, int X, int Y) { 
    m_INFVector[X][Y] = String; 
}

#endif
