/*
    © Vestris Inc., Geneva, Switzerland
    http://www.vestris.com, 1994-1999 All Rights Reserved
    ______________________________________________

    written by Daniel Doubrovkine - dblock@vestris.com

*/

#ifndef ALKALINE_INDEX_MANAGER_HPP
#define ALKALINE_INDEX_MANAGER_HPP

#include <platform/include.hpp>
#include <String/String.hpp>
#include <Vector/IntVector.hpp>
#include <File/Progress.hpp>
#include <Mutex/RWMutex.hpp>

class CIndexManager : public CObject {
	property(CString, Filename);
	static CProgress m_Progress;
        mutable_property(CRWMutex, RwMutex);
public:
	CIndexManager(void);
	virtual ~CIndexManager(void);

	static bool WriteIntVector(const CString& FileName, const CIntVector& Index, bool Verbose = true, int DataRows = 0);
	static bool WriteStringVector(const CString& FileName, const CVector<CString>& Index, bool Verbose = true, int DataRows = 0);
	static bool WriteIntVectorVector(const CString& FileName, const CVector<CIntVector>& Index, bool Verbose = true, int DataRows = 0);
	static bool WriteStringVectorVector(const CString& FileName, const CVector< CVector<CString> >& Index, bool Verbose = true, int DataRows = 0);

	static bool LoadIntVector(const CString& FileName, CIntVector& Target, bool Verbose = true);
	static bool LoadStringVector(const CString& FileName, CVector<CString>& Target, bool Verbose = true);
	static bool LoadIntVectorVector(const CString& FileName, CVector<CIntVector>& Target, bool Verbose = true);
	static bool LoadStringVectorVector(const CString& FileName, CVector< CVector<CString> >& Target, bool Verbose = true);

	static CString ReadLine(FILE * IStream, int Max, char * buffer);

	static float GetStringVectorMemorySize(const CVector<CString>& Vector);
	static float GetStringVectorVectorMemorySize(const CVector< CVector<CString> >& Vector);
	static float GetIntVectorMemorySize(const CVector<CIntVector>&);
	static float GetIntVectorMemorySize(const CVector<int>&);
        inline void StartReading(void) const { m_RwMutex.StartReading(); }
        inline void StopReading(void) const { m_RwMutex.StopReading(); }
        inline void StartWriting(void) { m_RwMutex.StartWriting(); }
        inline void StopWriting(void) { m_RwMutex.StopWriting(); }
};

#endif
