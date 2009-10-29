/*
    © Vestris Inc., Geneva, Switzerland
    http://www.vestris.com, 1994-1999 All Rights Reserved
    ______________________________________________

    written by Daniel Doubrovkine - dblock@vestris.com

*/

#ifndef ALKALINE_SESSION_HPP
#define ALKALINE_SESSION_HPP

#include <platform/include.hpp>
#include <Session/Session.hpp>
#include <AlkalineData/AlkalineData.hpp>
#include <Mutex/Atomic.hpp>

class CAlkalineSession : public CSession {
	property(bool, Searching);
	property(CSearchObject, SearchObject);
	property(CStringTable, UrlReplacements);
	property(CStringTable, Regexp_UrlReplacements);
	copy_property(CAlkalineData *, AlkalineData);
	copy_property(CHttpIo *, HttpIoPointer);
public:
	static CAtomic m_AlkalineSessions;
	CAlkalineSession(CAlkalineData *);
	virtual ~CAlkalineSession(void);
	virtual void WriteLog(const CString& m_LogFilename, const CString& String) const;
	bool ProcessResultsURL(CString&) const;
	virtual CString& MapTermEach(CString& Term, int) const;
    inline void SetUSFormats(bool bValue) { m_SearchObject.m_SearchOptions.m_USFormats = bValue; }
protected:	
	virtual void TraverseSSI(const CString SSIString, CHttpIo& HttpIo);
	virtual bool ProcessTag(CHttpIo& HttpIo, const CString& Tag);
	virtual void TraversePost(CHttpIo& HttpIo, const CString& Buffer);
	virtual void TraverseOptions(CHttpIo& HttpIo, const CString& Buffer);	
private:
	CString GetNextInherit(CHttpIo& HttpIo) const;
	CString MakeSearchString(CHttpIo& HttpIo);
	void MakeSearchField(const CString& FieldName, const CString& FieldValue);
private:
	CVector<CString> m_OptionHosts;
	CVector<CString> m_OptionPaths;
	CVector<CString> m_OptionUrls;
	CVector<CString> m_OptionOther;
	CVector<CString> m_OptionBefore;
	CVector<CString> m_OptionAfter;
    CVector<CString> m_OptionMeta;
};

#endif
