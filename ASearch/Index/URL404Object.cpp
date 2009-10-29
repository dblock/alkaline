/*
	© Vestris Inc., Geneva, Switzerland
	http://www.vestris.com, 1994-1999 All Rights Reserved
	______________________________________________

    written by Daniel Doubrovkine - dblock@vestris.com

*/

#include <alkaline.hpp>

#include "URL404Object.hpp"
#include <Index/URLManager.hpp>
#include <Index/Index.hpp>
#include <Site/Site.hpp>
#include <Encryption/Encryption.hpp>
#include <String/GStrings.hpp>

CURL404Object::CURL404Object(void) {
    m_Index = NULL;
    m_Site = NULL;
    m_Verbose = false;        
    m_TableIndex = -1;
}
CURL404Object::CURL404Object(const CURL404Object& Object) {
    operator=(Object);
}

CURL404Object& CURL404Object::operator=(const CURL404Object& Object) {
    m_Index = Object.m_Index;
    m_Site = Object.m_Site;
    m_Verbose = Object.m_Verbose;
    m_Url = Object.m_Url;        
    m_TableIndex = Object.m_TableIndex;
    return * this;
}

void CURL404Object::ExecuteRemoteFile(const CUrl& Url, CString& Output) {

    CString LocalName = Url.GetHttpPath();

#ifdef _WIN32
    LocalName.Replace('/', '\\');
    LocalName.TrimRight('\\');
#endif

    if (m_Verbose) { 
        Output += ("[" + LocalName + "]");
    }
    
    if (CLocalFile::FileExists(LocalName) || CLocalPath::DirectoryExists(LocalName)) {
        
        if (m_Verbose) { 
            Output += ("[verified]\n"); 
        }
        
    } else {
        
        if (m_Verbose) { 
            Output += ("[removed]\n"); 
        }
    
        RemoveFile();
    }
}

void CURL404Object::RemoveFile(void) {
    
    // mark index to write
    m_Index->SetModifiedFlag();
    
    // remove the page
    m_Index->GetSearcher().RemovePage(m_TableIndex);
    
    m_Index->GetLNXManager().StartWriting();
    m_Index->GetLNXManager().RemoveAll(m_TableIndex);
    m_Index->GetLNXManager().StopWriting();
    
    m_Index->GetINFManager().StartWriting();
    for (int t=m_Index->GetINFManager().GetSize(m_TableIndex) - 1; t>=0; t--)
        m_Index->GetINFManager().PutAt(CString::EmptyCString, m_TableIndex, t);
    m_Index->GetINFManager().StopWriting();            
    
}

void CURL404Object::ExecuteRemoteHttp(CString& Output) {
    CRemoteFile F404Remote;
    if (m_Verbose)  { Output += ("[" + m_Url + "]"); }
    F404Remote.SetUrl(m_Url);
    m_Site->AppendRequestHeaders(F404Remote);
    F404Remote.SetTimeout(10);
    F404Remote.SetMaxSize(0);
    F404Remote.SetAuth(m_Site->GetAuth());
    bool StillRetrieve = true;
    while (StillRetrieve) {
        if (g_pHandler->GetSignalSigterm())
            break;
        if (m_Index->GetProxy().GetLength()) 
            F404Remote.Get(m_Index->GetProxy()); 
        else F404Remote.Get();
        if (F404Remote.GetRStatusValue() == 404) {
            if (m_Verbose) { Output += "[removed]\n"; }
            
            RemoveFile();
            
            StillRetrieve = false;
            break;
        } else if (F404Remote.GetRStatusValue() == 200) {
            StillRetrieve = false;
            if (m_Verbose) { Output += ("[verified]\n"); }
        } else {
            StillRetrieve = false;
            if (m_Verbose) { Output += ("[" + CHttpIo::GetRStatusString(F404Remote.GetRStatusValue()) + ", keeping]\n"); }
            break;
        }            
    }
}

bool CURL404Object::ExecuteLocal(CString& Output) {    
    
    if (m_Site->GetReplaceLocalSize()) {
        
        CString LocalName;
        
        if (m_Site->ReplaceLocalUrl(m_Url, &LocalName, NULL)) {        
            
            if (CLocalFile::FileExists(LocalName)) {          
                
                if (m_Verbose) { 
                    Output += ("[" + LocalName + "]");
                    Output += ("[verified]\n"); 
                }
                
                return true;
            }
        }
    }  
    
    return false;
}

void CURL404Object::Execute(void) {

    CString Output;

    if (! ExecuteLocal(Output)) {
    
        CUrl Url(m_Url);
    
        if (Url.GetScheme().Same(g_strProto_FILE)) {
            ExecuteRemoteFile(Url, Output);
        } else {
            ExecuteRemoteHttp(Output);  
        }
    }
    
    if (m_Verbose && !g_pHandler->GetSignalSigterm()) {
        cout << Output;
        cout.flush();
    }
}


