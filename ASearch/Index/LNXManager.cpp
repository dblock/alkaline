/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  _____________________________________________________

  written by Daniel Doubrovkine - dblock@vestris.com

*/

#include <alkaline.hpp>

#include "LNXManager.hpp"
#include "Index.hpp"

CLNXManager::CLNXManager(void) {

}

CLNXManager::~CLNXManager(void) {

}

bool CLNXManager::Load(const CString& Filename, bool Verbose) {
	m_Filename = Filename;
	return LoadIntVectorVector(m_Filename, m_LNXVector, Verbose);
}

void CLNXManager::Append(CLNXManager& Manager, const CVector<int>& DispTable, CIndex * Parent) {
	// CVector<CIntVector>, LNXVector
	cout << "  [merging LNXs]"; cout.flush();
	CProgress Progress(10);	
        bool bIdentical = false;
        int * pVector = NULL;
        CIntVector NewVector;
	for (register int i=0;i<(int) Manager.GetSize();i++) {		
          int DispTableIndex = DispTable[i];
          Progress.Show(i, Manager.GetSize());		
          NewVector.RemoveAll();
          EnsureSize(DispTableIndex + 1);
          if ((pVector = Manager.m_LNXVector[i].GetVector())) {
            /* displace the related URLs */
            for (register int j=0;j<(int)Manager.m_LNXVector[i].GetSize();j++)
              pVector[j] = DispTable[pVector[j]];
            /* merge into the LNXVector */
            /* two choices, we already have something in DispTableIndex or it's new */
            for (register int k=0;k<(int)Manager.m_LNXVector[i].GetSize();k++)
              NewVector += pVector[k];
            if (m_LNXVector[DispTableIndex].GetSize()) {
              if ((m_LNXVector[DispTableIndex] != NewVector)&&(m_LNXVector[DispTableIndex].GetSize())&&(NewVector.GetSize())) {
                if (!bIdentical) {
                  cout << "{\n    [warning, identical urls are referenced in both indexes]";
                  bIdentical = true;
                }
                cout << "\n    {url: " << Parent->GetURLLink(DispTableIndex) << "}";
              }
            }
            delete[] pVector;
          }
          m_LNXVector[DispTableIndex] += NewVector;
	}
	Progress.Finish();
	cout << "[" << m_LNXVector.GetSize() << " LNX entries]" << endl;
}
