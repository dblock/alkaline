/*
	© Vestris Inc., Geneva, Switzerland
	http://www.vestris.com, 1994-1999 All Rights Reserved
	______________________________________________

    written by Daniel Doubrovkine - dblock@vestris.com

*/

#ifndef ALKALINE_URL_404_OBJECT_HPP
#define ALKALINE_URL_404_OBJECT_HPP

#include <platform/include.hpp>
#include <String/String.hpp>
#include <Internet/Url.hpp>

class CSite;
class CIndex;
class CURLManager;

class CURL404Object : public CObject {
    property(CString, Url);
    property(int, TableIndex);
    copy_property(CIndex *, Index);
    copy_property(CSite *, Site);
    copy_property(bool, Verbose);
    copy_property(CURLManager *, UrlManager);
private:
    void ExecuteRemoteHttp(CString& Output);
    bool ExecuteLocal(CString& Output);
    void ExecuteRemoteFile(const CUrl& Url, CString& Output);
    void RemoveFile(void);
public:
    CURL404Object(void);
    CURL404Object(const CURL404Object& Object);
    CURL404Object& operator=(const CURL404Object& Object);
    void Execute(void);
};

#endif
