/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  ______________________________________________

  written by Daniel Doubrovkine - dblock@vestris.com

*/

#ifndef ALKALINE_C_GLOBAL_CONFIGURATION_HPP
#define ALKALINE_C_GLOBAL_CONFIGURATION_HPP

#include <platform/include.hpp>
#include <String/String.hpp>
#include <String/StringTable.hpp>
#include <Io/Io.hpp>
#include <Config/ConfigBase.hpp>

class CAlkalineServer;
 
class CGlobalCnf : public CConfigBase {
    
    copy_property(CAlkalineServer *, AlkalineServer);
    
    property(bool, CacheTemplates);
    property(bool, USFormats);
    property(CString, Filename);
    protected_property(struct_stat, Stat);
    property(bool, KeepAlive);
    property(bool, Nagle);
    property(bool, Verbose);
    property(bool, Ssi);
    property(int, DnsTimeout);
    
    property(int, RampupSearchThreads);
    property(int, RampupIndexThreads);
    property(int, MaxSearchThreads);    
    property(int, MaxIndexThreads);
    property(int, MaxSearchQueueSize);
    property(int, MaxSearchThreadIdle);
    
    property(int, PingInterval);
    property(int, PingRestart);
    property(bool, Ping);
    
    property(int, SearchCacheLife);

	property(CVector<CString>, DocumentPaths);
	property(CVector<CString>, ForwardAlnHeaders);
    
private:
    
    void Initialize(void);
    
public:	
    
    virtual void CreateConfigurationOptions(void);
    virtual void FinalizeVirtual(CConfigOption& ConfigurationOption);
    virtual void DumpVirtual(CConfigOption& ConfigurationOption) const;
    
    void AddOption(const CString& Line, bool Verbose = true);
    
    CGlobalCnf(void);
    CGlobalCnf(const CGlobalCnf&);
    virtual CGlobalCnf& Copy(const CGlobalCnf& Other);
    virtual ~CGlobalCnf(void);
    inline CGlobalCnf& operator=(const CGlobalCnf& Other) { return Copy(Other); }
  
    bool HasChanged(void) const;
	bool IsDocumentPath(const CString&) const;
    void Read(CAlkalineServer * Server, bool Verbose);
};

#endif
