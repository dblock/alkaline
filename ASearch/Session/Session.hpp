/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  ______________________________________________

  written by Daniel Doubrovkine - dblock@vestris.com

*/

#ifndef C_SESSION_HPP
#define C_SESSION_HPP

#include <platform/include.hpp>
#include <String/StringTable.hpp>
#include <Io/HttpIo.hpp>
#include <Mutex/Mutex.hpp>

class CSession : public CStringTable {
	property(bool, SSIEnabled);
    property(CString, Proxy);
public:
	CSession(void);
	virtual ~CSession(void);
	virtual CSession& Execute(CHttpIo&, const CString&);
	CString& MapTerm(const CString& Source, CString& Target, int) const;
	CString Map(const CString&) const;
	virtual void WriteLog(const CString&, const CString&) const;
	void ErrorPage(const CString& Error, CHttpIo& HttpIo) const;
	virtual void TraverseTags(CHttpIo& HttpIo, const CString& Buffer);	
protected:
	static CMutex m_LogMutex;
	virtual void TraverseOptions(CHttpIo& HttpIo, const CString& Buffer);
	virtual void TraverseSSI(const CString SSIString, CHttpIo& HttpIo);
	virtual bool ProcessTag(CHttpIo& HttpIo, const CString& Tag);
	virtual void TraversePost(CHttpIo& HttpIo, const CString& Buffer);
	virtual CString& MapTermEach(CString&, int) const;
	int ExecuteTag(const CString& TagString, int nPos, CString& Result, CHttpIo& HttpIo);
};

#endif
