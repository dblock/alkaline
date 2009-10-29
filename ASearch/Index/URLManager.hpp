/*
	© Vestris Inc., Geneva, Switzerland
	http://www.vestris.com, 1994-1999 All Rights Reserved
	______________________________________________

    written by Daniel Doubrovkine - dblock@vestris.com

	*/

#ifndef ALKALINE_URL_MANAGER_HPP
#define ALKALINE_URL_MANAGER_HPP

#include "IndexManager.hpp"
#include <File/RemoteFile.hpp>
#include <Tree/UrlTree.hpp>

class CSite;
class CIndex;

class CURLManager : public CIndexManager {
	CUrlTree m_UrlTree;
public:
	CURLManager(void);
	virtual ~CURLManager(void);
	void Remove404(CIndex * Index, CSite * Site, bool Verbose = true);
	void Load(const CString&, bool Verbose = true);
	bool FastLoad(const CString&, bool Verbose = true);
	inline void RemoveAll(void) { m_UrlTree.RemoveAll(); }
	inline int GetSize(void) const { return m_UrlTree.GetSize(); }
	inline bool Write(bool Verbose = true, int DataRows = 0) { return m_UrlTree.Save(m_Filename, Verbose, DataRows); }
	void AppendGetDispTable(const CURLManager&, CVector<int>&);        
    inline void SetInsens(bool bInsens) { m_UrlTree.SetInsens(bInsens); }
    inline const CUrlTree& GetUrlTree(void) const { return m_UrlTree; }
    inline int Add(const CString& Url, bool * bAdded) { return m_UrlTree.Add(Url, bAdded); }
};


#endif
