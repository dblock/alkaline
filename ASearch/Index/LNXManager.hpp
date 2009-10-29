/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  ______________________________________________

  written by Daniel Doubrovkine - dblock@vestris.com

*/

#ifndef ALKALINE_LNX_MANAGER_HPP
#define ALKALINE_LNX_MANAGER_HPP

#include "IndexManager.hpp"
#include <Vector/IntVector.hpp>

class CIndex;

class CLNXManager : public CIndexManager {
	property(CVector<CIntVector>, LNXVector);    
public:
	bool Load(const CString& Filename, bool Verbose = true);
	CLNXManager(void);
	virtual ~CLNXManager(void);
	inline void RemoveAll(void) { m_LNXVector.RemoveAll(); }
	inline void RemoveAll(int Index) { m_LNXVector[Index].RemoveAll(); }
	inline int GetSize(void) const { return m_LNXVector.GetSize(); }
	inline void EnsureSize(int NewSize) { if ((int) m_LNXVector.GetSize() < NewSize) m_LNXVector.SetSize(NewSize); }
    inline void Grow(int NewSize) { m_LNXVector.SetDim(NewSize); }
    inline bool Write(bool Verbose = true, int DataRows = 0) { return WriteIntVectorVector(m_Filename, m_LNXVector, Verbose, DataRows); }
	/* append an LNX manager using a Disptable */
	void Append(CLNXManager& Manager, const CVector<int>& DispTable, CIndex * Parent);    
};



#endif
