/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  _____________________________________________________

  written by Daniel Doubrovkine - dblock@vestris.com

*/

#include <alkaline.hpp>

#include "IndexManager.hpp"
#include <File/LocalFile.hpp>
#include <String/GStrings.hpp>
#include <File/MMapFile.hpp>
 
CProgress CIndexManager::m_Progress(10, false);

CIndexManager::CIndexManager(void) {

}

CIndexManager::~CIndexManager(void) {

}

/* write a compressed vector of integer values */
bool CIndexManager::WriteIntVector(const CString& FileName, const CIntVector& Index, bool /* Verbose */, int /* DataRows */) {
	FILE * OStream = fopen((const char *) FileName.GetBuffer(), "wb+");
	if (OStream) {
		Index.Write(OStream);
        fwrite(g_strCrLf, base_strlen(g_strCrLf), 1, OStream);
		fclose(OStream);
		return true;
	} else return false;
}

/* write a vector of CStrings */
bool CIndexManager::WriteStringVector(const CString& FileName, const CVector<CString>& Index, bool Verbose, int DataRows) {
	FILE * OStream = fopen((const char *) FileName.GetBuffer(), "wb+");
	if (OStream) {
		m_Progress.Init(10, Verbose);
		CString Buffer;
		if (DataRows == 0) 
		  DataRows = (int) Index.GetSize();
		for(int i=0;i<DataRows;i++) {
			m_Progress.Show(i, DataRows, Verbose);
			Buffer = Index[i];
			if (Buffer.GetLength()) fwrite(Buffer.GetBuffer(), Buffer.GetLength(), 1, OStream);
            fwrite(g_strCrLf, base_strlen(g_strCrLf), 1, OStream);
		}
		fclose(OStream);
		m_Progress.Finish(Verbose);
		return true;
	} else return false;
}

/* write a vector of vectors of ints */
bool CIndexManager::WriteIntVectorVector(const CString& FileName, const CVector<CIntVector>& Index, bool Verbose, int DataRows) {
	FILE * OStream = fopen((const char *) FileName.GetBuffer(), "wb+");
	if (OStream) {
		m_Progress.Init(10, Verbose);
		if (DataRows == 0)
		  DataRows = (int) Index.GetSize();
		for(int i=0;i<DataRows;i++) {
			m_Progress.Show(i, DataRows, Verbose);
			Index[i].Write(OStream);
            fwrite(g_strCrLf, base_strlen(g_strCrLf), 1, OStream);
		}
		fclose(OStream);
		m_Progress.Finish(Verbose);
		return true;
	} else return false;
}

/* write a vector of vectors of strings */
bool CIndexManager::WriteStringVectorVector(const CString& FileName, const CVector< CVector<CString> >& Index, bool Verbose, int DataRows) {
	FILE * OStream = fopen((const char *) FileName.GetBuffer(), "wb+");
	if (OStream) {
		m_Progress.Init(10, Verbose);
		if (DataRows == 0)
		  DataRows = (int) Index.GetSize();
		for(int i=0;i<DataRows;i++) {
			m_Progress.Show(i, DataRows, Verbose);
			CString Buffer;
			for (int j=0;j<(int)Index[i].GetSize();j++) {
				if (j) fprintf(OStream, "\t");
				Buffer = Index[i][j];
				if (Buffer.GetLength()) 
                    fwrite(Buffer.GetBuffer(), Buffer.GetLength(), 1, OStream);
            }
            fwrite(g_strCrLf, base_strlen(g_strCrLf), 1, OStream);
		}
		fclose(OStream);
		m_Progress.Finish(Verbose);
		return true;
	} else return false;
}

/* load a vector of integers */
bool CIndexManager::LoadIntVector(const CString& FileName, CIntVector& Target, bool Verbose){	
  Target.RemoveAll();
  CMMapFile FastFile(FileName);
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
      Target += CString::StrToInt(Line);
    }
    m_Progress.Finish(Verbose);
  }	
  cout << "[" << Target.GetSize() << " lines]" << endl;
  return true;
}

/* load a vector of strings */
bool CIndexManager::LoadStringVector(const CString& FileName, CVector<CString>& Target, bool Verbose) {	
  Target.RemoveAll();
  CMMapFile FastFile(FileName);
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
      Target += Line;
    }
    m_Progress.Finish(Verbose);
  }
  cout << "[" << Target.GetSize() << " lines]" << endl;
  return true;
}

/* load a vector of vectors of integers */
CString CIndexManager::ReadLine(FILE * IStream, int Max, char * buffer) {
	CString Line;
	int iPos = 0; char c;
	while (!feof(IStream)){
		if (iPos == Max-1){
			buffer[iPos] = 0;
			Line+=buffer;
			iPos = 0;
		}
		c = fgetc(IStream);
		if (c == 10) {
			buffer[iPos] = 0;
			Line+=buffer;
			break;
		} else if (c != 13) {
			buffer[iPos] = c;
			iPos++;
		}
	}
	return Line;
}

/* load a matrix of integer values */
bool CIndexManager::LoadIntVectorVector(const CString& FileName, CVector<CIntVector>& Target, bool Verbose){
    Target.RemoveAll();
    static CIntVector EmptyVector;	
    bool Intervalled = false;

    CMMapFile FastFile(FileName);
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
        if (!Line.GetLength()) {
          Target += EmptyVector;
          continue;
        }
        int PrevPos = 0;
        int IntervalPos = -1;
        CIntVector LineVector;
        for (register int Pos = 0; Pos <= (int) Line.GetLength(); Pos++) {
          if ((Pos == (int) Line.GetLength())||(Line[Pos] == ' ')) {
            if (IntervalPos >= 0) {
              LineVector._AppendInt(Line.GetInt(PrevPos, IntervalPos - PrevPos), Line.GetInt(IntervalPos+1, Pos - IntervalPos-1));
              IntervalPos = -1;
            } else {
              if (Intervalled) LineVector._AppendElt(Line.GetInt(PrevPos, Pos - PrevPos));
              else LineVector.AddElt(Line.GetInt(PrevPos, Pos - PrevPos));
            }
            PrevPos = Pos + 1;
          } else if (Line[Pos] == '-') {
            if (!Intervalled) Intervalled = true;
            IntervalPos = Pos;
          }
        }
        Target += LineVector;
      }
      m_Progress.Finish(Verbose);
    }
    cout << "[" << Target.GetSize() << " lines]" << endl;
    return true;
}

/* load a vector of vectors of strings */
bool CIndexManager::LoadStringVectorVector(const CString& FileName, CVector< CVector<CString> >& Target, bool Verbose) {	
    Target.RemoveAll();
    CMMapFile FastFile(FileName);
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
        CVector<CString> TmpVector;
        CString::StrToVector(Line, '\t', &TmpVector);
        Target.Add(TmpVector);
      }
      m_Progress.Finish(Verbose);
    }
    cout << "[" << Target.GetSize() << " lines]" << endl;
    return true;
}

float CIndexManager::GetIntVectorMemorySize(const CVector<CIntVector>& Vector) {
	float MemorySize = sizeof(CVector<CIntVector>);
	for (int i=0;i<(int) Vector.GetSize();i++)
		MemorySize += Vector[i].GetMemorySize();
	return MemorySize;
}

float CIndexManager::GetStringVectorVectorMemorySize(const CVector< CVector<CString> >& Vector) {
	float MemorySize = sizeof(CVector< CVector<CString> >);
	for (int i=0;i<(int)Vector.GetSize();i++)
		MemorySize+=GetStringVectorMemorySize(Vector[i]);
	return MemorySize;
}

float CIndexManager::GetStringVectorMemorySize(const CVector<CString>& Vector) {
	float MemorySize = sizeof(CVector<CString>);
	for (int i=0;i<(int)Vector.GetSize();i++) {
		MemorySize+=(sizeof(CString) + Vector[i].GetLength());
	}
	return MemorySize;
}

float CIndexManager::GetIntVectorMemorySize(const CVector<int>& Vector) {
	float MemorySize = sizeof(CVector<int>);
	MemorySize+=(Vector.GetSize() * sizeof(int));
	return MemorySize;
}
