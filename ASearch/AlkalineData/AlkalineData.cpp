/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  _____________________________________________________
  
  written by Daniel Doubrovkine - dblock@vestris.com

  Revision history:
  
  18.03.2000: removed dumb sprintf in seconds output
  
*/

#include <alkaline.hpp>

#include "AlkalineData.hpp"
#include <Site/Site.hpp>
#include <Mv4/EquivManager.hpp>
#include <Mv4/AdminManager.hpp>
#include <AlkalineServer/AlkalineServer.hpp>
#include <Tree/XmlTree.hpp>

CAlkalineData::~CAlkalineData(void) {
	for (register int i=0;i<(int) m_SitesList.GetSize();i++)
		delete m_SitesList[i];
}

#define S_PORTION 60

void CAlkalineData::Reload(void){
    StartReading();
    for (register int i=0;i<(int) m_SitesList.GetSize();i++){
        if (m_Terminated)
            break;
        m_SitesList[i]->LoadSiteIndex();
    }
    StopReading();
}

void CAlkalineData::Execute(void * Arguments) {
	cout << "[indexing thread running - lazy mode]" << endl;
	base_sleep(1);
	time_t First; time_t FirstSite;
	time_t Second;
	long dSec;
	long dMin;
	bool Once = false;
    while(!m_Terminated && !g_pHandler->GetSignalSigterm()) {                
        
        time(&First);
        CIterator Iterator;
        
        StartReading();
        CSite ** CurrentSite = (CSite **) m_SitesList.GetFirst(Iterator);        
        while (CurrentSite && (!m_Terminated)) {            
            if (m_GlobalIndex) 
                time(&FirstSite);
            Once |= (!(* CurrentSite)->CanOnce());
            if (!m_Terminated && (* CurrentSite)->CanOnce()) {    
                // retrieve site
                (* CurrentSite)->RetrieveSite(true);
                if (m_GlobalIndex && !g_pHandler->GetSignalSigterm()) {
                    time(&Second);
                    CDate TmpDate;
                    dSec = (long) difftime(Second, FirstSite);
                    dMin = dSec/60;
                    dSec -= dMin*60;
                    cout << "[" << TmpDate.Map("$hour:$min:$sec $year-$month-$day") << 
                        "][(re)indexing of " << (* CurrentSite)->GetAlias() << " processed " << 
                        (* CurrentSite)->GetModifiedFiles() << " new/modified files in " << 
                        (int) dMin << ":" << ((dSec < 10)?"0":"") << dSec << " min]" << endl;
                }
            }
            if (!m_Terminated) {
                base_sleep((* CurrentSite)->GetSleepRoundtrip());
                CurrentSite = (CSite **) m_SitesList.GetNext(Iterator);
            }
        }
        StopReading();

        if (!m_Terminated && !g_pHandler->GetSignalSigterm()) {            
            if (m_GlobalIndex && !Once) {
                time(&Second);		  
                dSec = (long) difftime(Second, First);
                dMin = dSec/60;
                dSec -= dMin*60;
                cout << "[indexing roundtrip in " << (int) dMin << ":" << ((dSec < 10)?"0":"") << dSec << " min]" << endl;
            }
            if (ConfigHasChanged())
                m_GlobalIndex = ReloadEnginesList();
        }

        time(&Second);
        if (difftime(Second, FirstSite) < 1)
            base_sleep(1);
        
    }
    CThread::Execute(Arguments);
}

bool CAlkalineData::ConfigHasChanged(void) const {
    /*
	if (m_ParentServer->GetEquivManager().HasChanged()) 
          return true;
	if (m_ParentServer->GetAdminManager().HasChanged()) 
          return true;	
    CIterator Iterator;
    StartReading();
    CSite ** CurrentSite = (CSite **) m_SitesList.GetFirst(Iterator);
    while (CurrentSite) {			
        if ((* CurrentSite)->HasChanged()) return true;
        CurrentSite = (CSite **) m_SitesList.GetNext(Iterator);
    }
    StopReading();
    return false;
    */
    return false;
}

CAlkalineData::CAlkalineData(const CString& Root, CAlkalineServer * pParentServer) {
	m_Root = Root;
    m_Terminated = false;
    m_ParentServer = pParentServer;
}

void CAlkalineData::Load(void) {
	m_GlobalIndex = ReloadEnginesList();
}

bool CAlkalineData::ReloadEnginesList(void) {
    bool Result = false;
    
    StartWriting();
    
    m_ParentServer->GetEquivManager().Read();
    m_ParentServer->GetAdminManager().Read();
    
    CList<CSite *> NewIndex;
    CString::StrToVector(m_ParentServer->GetEquivManager().GetValue(m_Root), ' ', &m_EnginesList);
    m_EnginesList.QuickSortUnique();
    
    for (int k = m_EnginesList.GetSize() - 1; k >= 0; k--) {
        m_EnginesList[k].Trim32();        
        if (!m_EnginesList[k].GetLength())
            m_EnginesList.RemoveAt(k);
    }
    
    CString EachSite;
    CVector<CString> OldSites = m_SaneEnginesList;
    CVector<CString> NewSites;
    
    for (int i=0;i<(int) m_EnginesList.GetSize();i++) {
        EachSite = m_ParentServer->GetEquivManager().GetValue(m_EnginesList[i]);
        if (EachSite.GetLength()) {
            if (!m_SaneEnginesList.Contains(m_EnginesList[i])) {
                m_SaneEnginesList+=m_EnginesList[i];
                NewSites+=m_EnginesList[i];
                OldSites-=m_EnginesList[i];
                CSite * NewSite = ::new CSite(m_EnginesList[i], EachSite);
                NewSite->ReadConfig(true);
                // adjust thread parameters
                NewSite->GetIndexPool().SetRampupThreads(m_ParentServer->GetGlobalCnf().GetRampupIndexThreads());
                NewSite->GetIndexPool().SetMaxThreads(m_ParentServer->GetGlobalCnf().GetMaxIndexThreads());
                NewSite->SetOptions(m_Options);
                if (NewSite->GetCanReindex()) Result = true;
                m_SitesList += NewSite;
                NewIndex += NewSite;
            } else {
                OldSites -= m_EnginesList[i];
            }
        } else cout << "[invalid equiv: " << m_EnginesList[i] << "]" << endl;
    }
    
    int l;
    if (OldSites.GetSize()) {
        cout << "[excluded from Alkaline list: ";
        for (l=0;l<(int) OldSites.GetSize();l++) {
            for (int m=m_SaneEnginesList.GetSize()-1;m>=0;m--)
                if (m_SaneEnginesList[m] == OldSites[l]) {
                    m_SaneEnginesList.RemoveAt(m);
                    m_SitesList.RemoveAt(m);
                }
                cout << OldSites[l] << " ";
        } cout << "]" << endl;
    }
    
    StopWriting();
    
    
    if (NewSites.GetSize()) {
        cout << "[newly added to Alkaline list: ";
        for (l=0;l<(int) NewSites.GetSize();l++)
            cout << NewSites[l] << " ";
		cout << "]" << endl;
	}
	for (l=0;l<(int) NewIndex.GetSize();l++) {
        if (m_Terminated)
            break;
		cout << "[reloading " << NewSites[l] << "]" << endl;
		NewIndex[l]->LoadSiteIndex();
	}
	for (l=0;l<(int) m_SitesList.GetSize(); l++)
		if (m_SitesList[l]->GetCanReindex()) {
			Result = true;
			break;
		}
	return Result;
}

bool CAlkalineData::CanTerminate(void) const {
	for (int l=0;l<(int) m_SitesList.GetSize(); l++)
		if (m_SitesList[l]->IsWriting()) return false;
	return true;
}

void CAlkalineData::GetConfigWmlSearch(CString& Target) const {
  // the search input
  Target = "<input name=\"search\" size=\"15\"/>";
  // the configuration selector
  Target += "<select name=\"searchconfig\">"; 
  StartReading();  
  for (register int i=0;i<(int) m_SitesList.GetSize();i++) {
	  Target += "<option value=\"";
	  Target += m_SitesList[i]->GetAlias();
	  Target += "\">";
      Target += m_SitesList[i]->GetAlias();
	  Target += "</option>";
  }
  StopReading();
  Target += "</select>";   
}

void CAlkalineData::GetConfigWml(CString& Target) const {

  static CString ConfigXmlMap(
     "<configurations>"                         \
	 "</configurations>"
	 );
  
  CXmlTree XmlTree;
  XmlTree.SetXml(ConfigXmlMap);
  
  StartReading();
  for (register int i=0;i<(int) m_SitesList.GetSize();i++) {
	  m_SitesList[i]->PopulateXmlNode(XmlTree, XmlTree.XmlFind("/configurations"));
  }
  StopReading();

  XmlTree.GetWml(Target);
}

void CAlkalineData::GetConfigXml(CString& Target) const {
  static CString ConfigXmlMap(
     "<configurations>"                         \
     " <head>"                                  \
     "  <list></list>"                          \
     " </head>"                                 \
     " <config>"                                \
     " </config>"                               \
	 "</configurations>"
	 );
  
  CXmlTree XmlTree;
  XmlTree.SetXml(ConfigXmlMap);
  
  CString XmlHeadList;
 
  StartReading();
  CString ConfigPath;

  for (register int i=0;i<(int) m_SitesList.GetSize();i++) {
	  if (i) {
		  XmlHeadList += ';';
	  }
	  ConfigPath = m_SitesList[i]->GetConfigPath();

	  ConfigPath.Replace("\\", "\\\\");  
	  ConfigPath.Replace("/", "\\\\");
	  ConfigPath = CUrl::Escape(ConfigPath);

	  XmlHeadList += ConfigPath;
	  m_SitesList[i]->PopulateXmlNode(XmlTree, XmlTree.XmlFind("/configurations/config"));
  }
  StopReading();
  XmlTree.SetValue("/configurations/head/list", XmlHeadList);
  XmlTree.GetXml(Target);
}

void CAlkalineData::Stop(void) {
    
    m_Terminated = true;

    for (register int i=0;i<(int) m_SitesList.GetSize();i++)
		m_SitesList[i]->GetIndexPool().PassiveWait(true, 0);    
}
