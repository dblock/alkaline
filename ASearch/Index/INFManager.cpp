/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  _____________________________________________________
  
  written by Daniel Doubrovkine - dblock@vestris.com
  
*/

#include <alkaline.hpp>

#include "INFManager.hpp"
#include "Index.hpp"

CINFElement::CINFElement(void) {
    memset(&m_tmTime, 0, sizeof(m_tmTime));
    m_ttTime = 0;
    m_DocumentSize = 0;
}

CINFElement::~CINFElement(void) {
    
}

CINFElement::CINFElement(const CINFElement& Other) {
    operator=(Other);
}

CINFElement::CINFElement(const CVector<CString>& Vector) {
    m_Fields = Vector;    
    Update();
}

CINFElement& CINFElement::operator=(const CINFElement& Other) {
    if (this != &Other) {
        m_Fields = Other.m_Fields;
        Update();
    }
    return * this;
}

void CINFElement::Update(void) {

    if (m_Fields.GetSize() >= 2)
        m_DocumentSize = CString::StrToInt(m_Fields[1]);
    
    bool bSet = false;
    if (m_Fields.GetSize() && m_Fields[0].GetLength())
        bSet = CDate::EncodeDate(m_Fields[0], m_tmTime);
    else if ((m_Fields.GetSize() >= 2) && m_Fields[1].GetLength())
        bSet = CDate::EncodeDate(m_Fields[1], m_tmTime);        
    
    if (bSet) {
        m_ttTime = mktime(&m_tmTime);
    } else {
        memset(&m_tmTime, 0, sizeof(m_tmTime));
        m_ttTime = 0;
    }
}

bool CINFManager::Load(const CString& Filename, bool Verbose, bool bMakeMd5Tree) {
    m_Filename = Filename;
    
    if (! LoadINFVector(Verbose))
        return false;
    
    if (!m_INFVector.GetSize())
        return false;

    if (bMakeMd5Tree) {
        
        if (Verbose) {
            cout << "  [building the md5 tree]";
            cout.flush();
        }
        CProgress Progress(20, Verbose);
        for (int i = 0; i < (int) m_INFVector.GetSize(); i++) {

            if (g_pHandler->GetSignalSigterm())
                return false;

            Progress.Show(i, m_INFVector.GetSize(), Verbose);
            if (m_INFVector[i].GetSize() >= 6)
                m_Md5Tree.Add(m_INFVector[i][6], i);
        }
        Progress.Finish(Verbose);
        if (Verbose) cout << endl;
    }

    return true;
}

CINFManager::CINFManager(void) {
}

CINFManager::~CINFManager(void) {
    
}


CString CINFManager::GetModifiedSince(int UrlPos) const {
    if ((UrlPos != -1)&&((int) m_INFVector.GetSize() > UrlPos)) {
        if (m_INFVector[UrlPos].GetSize()) {
            return m_INFVector[UrlPos][0];
        }
    }
    return CString::EmptyCString;
}

void CINFManager::Append(CINFManager& Manager, const CVector<int>& DispTable, CIndex * Parent) {
    cout << "  [merging INFs]"; cout.flush();
    CProgress Progress(10);	
    bool bIdentical = false;
    
    int DispTableMax = 0;
    for (register int d=0;d<(int)DispTable.GetSize();d++)
        DispTableMax = (DispTableMax < DispTable[d])?DispTable[d]:DispTableMax;
    
    EnsureSize(DispTableMax + 1);
    
    for (register int i=0;i<(int) Manager.GetSize();i++) {	
        int DispTableIndex = DispTable[i];            
        Progress.Show(i, Manager.GetSize());
        /* if target is empty */ 
        if (!Manager.GetSize(i)) {
            /* the other one is not indexed */
        } else if (!m_INFVector[DispTableIndex].GetSize()) {	       
            /* the other one is indexed, but local is not */
            m_INFVector[DispTableIndex] = Manager[i];
        } else {
            /* both sides have indexed the page, keep it as is, should get the most recent*/			
            if (m_INFVector[DispTableIndex] != Manager[i]) {
                if (!bIdentical) {
                    cout << "{\n    [warning, identical urls are referenced in both indexes]";
                    bIdentical = true;
                }
                cout << "\n    {url: " << Parent->GetURLLink(DispTableIndex) << "}";
                cout.flush();              
            }
        }
    }
    Progress.Finish();
    cout << "[" << m_INFVector.GetSize() << " INF entries]" << endl;
}

/* write a vector of vectors of strings */
bool CINFManager::Write(bool Verbose, int DataRows) {
    
  bool bResult = true;
  
  if (DataRows == 0)
    DataRows = (int) m_INFVector.GetSize();
  
  CMMapFile Stream(m_Filename);
  
  if (! Stream.MMap(MMAP_OPENMODE, GetDataSize(DataRows))) {
    cerr << "[mmap failed, datasize is " << GetDataSize(DataRows) << " bytes]" << endl;
    return false;
  }
  
  m_Progress.Init(10, Verbose);
  
  for(int i = 0; i < DataRows; i++) {
    
    m_Progress.Show(i, DataRows, Verbose);
    
    for (int j=0;j<(int)m_INFVector[i].GetSize();j++) {
      
      if (j) {
	if (! Stream.Write((void *) "\t", sizeof(char))) {
	  bResult = false;
	  break;
	}
      }
      
      if (! Stream.Write((void *) m_INFVector[i][j].GetBuffer(), m_INFVector[i][j].GetLength())) {
	bResult = false;
	break;
      }
    }
    
    if (! Stream.Write((void *) g_strCrLf, base_strlen(g_strCrLf))) {
      bResult = false;
      break;
    }
  }
  
  m_Progress.Finish(Verbose);

  return bResult;
}

long CINFManager::GetDataSize(int DataRows) const {
  long lResult = 0;

  if (DataRows == 0) {
    DataRows = (int) m_INFVector.GetSize();
  }
  
  for(int i = 0; i < DataRows; i++) {
    
    for (int j=0;j<(int)m_INFVector[i].GetSize();j++) {            
      if (j) {
	lResult += sizeof(char); // tabulator
      }
      lResult += m_INFVector[i][j].GetLength() * sizeof(char);
    }
    
    lResult += base_strlen(g_strCrLf);
  }
  
  return lResult;
}

/* load a vector of vectors of strings */
bool CINFManager::LoadINFVector(bool Verbose) {	
    m_INFVector.RemoveAll();
    CMMapFile FastFile(m_Filename);
    if (! FastFile.MMap(MMAP_READOPENMODE))
      return false;
    long fSize = FastFile.GetSize();
    if (fSize && FastFile.GetMem()) {
        m_Progress.Init(10, Verbose);
        CString Line;        
        while (FastFile.ReadLine(&Line) >= 0) {

            if (g_pHandler->GetSignalSigterm())
                return false;

            m_Progress.Show(FastFile.GetOffset(), fSize, Verbose);        
            CINFElement INFObject;
            m_INFVector.Add(INFObject);
            CString::StrToVector(Line, '\t', & (m_INFVector[m_INFVector.GetSize()-1].GetFields()));
            m_INFVector[m_INFVector.GetSize()-1].Update();
        }
        m_Progress.Finish(Verbose);
    } else {
        return false;
    }
    cout << "[" << m_INFVector.GetSize() << " lines]" << endl;
    return true;
}

void CINFManager::RemoveAll(void) {
    m_INFVector.RemoveAll();
    m_Md5Tree.RemoveAll();
}
