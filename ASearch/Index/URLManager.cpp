/*
	© Vestris Inc., Geneva, Switzerland
	http://www.vestris.com, 1994-1999 All Rights Reserved
	______________________________________________

    written by Daniel Doubrovkine - dblock@vestris.com

*/

#include <alkaline.hpp>

#include "URLManager.hpp"
#include <Index/Index.hpp>
#include <Site/Site.hpp>
#include <Encryption/Encryption.hpp>
#include <Index/URL404Object.hpp>

CURLManager::CURLManager(void) {

}

CURLManager::~CURLManager(void) {

}

void CURLManager::Load(const CString& Filename, bool Verbose) {    
	m_Filename = Filename;
	m_UrlTree.Load(Filename, Verbose);
}

bool CURLManager::FastLoad(const CString& Filename, bool Verbose) {
	m_Filename = Filename;
	return m_UrlTree.FastLoad(Filename, Verbose);
}

void UrlManagerExecute404(void * Argument) {    
    ((CURL404Object *) Argument)->Execute();
    delete (CURL404Object *) Argument;    
}

void CURLManager::Remove404(CIndex * Index, CSite * Site, bool Verbose) {
	if (Verbose) {
		cout << "[removing HTTP 404/Not Found]";
		cout.flush();
	}

    // calculated at load for search of *
	// Index->GetSearcher().MakeAlreadyIndexed(Verbose);

    CThreadPool URL404Pool;
    URL404Pool.SetMaxThreads(Site->GetIndexPool().GetMaxThreads());
    URL404Pool.SetMaxQueueSize(-1);
    
	int UrlVectorIndexed = 0;
	for (int i=(m_UrlTree.GetSize()-1);i>=0;i--) {
        
        if (g_pHandler->GetSignalSigterm())
            break;
        
		if (!Index->GetSearcher().AlreadyIndexed(i)) 
            continue;
        
		UrlVectorIndexed++;
		CString UrlVectorI = m_UrlTree[i];
        
        CURL404Object * Url404Object = new CURL404Object;
        Url404Object->SetUrl(UrlVectorI);
        Url404Object->SetVerbose(Verbose);
        Url404Object->SetIndex(Index);
        Url404Object->SetSite(Site);
        Url404Object->SetUrlManager(this);
        Url404Object->SetTableIndex(i);

        CThreadPoolJob URL404Job((void *) &UrlManagerExecute404, (void *) Url404Object);
        if (!URL404Pool.AddJob(URL404Job)) {
            cout << "URLManager :: Error :: Thread pool is full." << endl;
            delete Url404Object;
            continue;
        }        
	}
    
    if (Verbose && !g_pHandler->GetSignalSigterm())
        cout << "[scheduled " << UrlVectorIndexed << "/" << m_UrlTree.GetSize() << " resource locators]" << endl;

    URL404Pool.PassiveWait();
    //
    // although it's an interesting optimization, it breaks the * search
    //
	// Index->GetSearcher().ClearAlreadyIndexed();
    //
	if (Verbose && !g_pHandler->GetSignalSigterm())
        cout << "[processed " << UrlVectorIndexed << "/" << m_UrlTree.GetSize() << " resource locators]" << endl;
}

void CURLManager::AppendGetDispTable(const CURLManager& Manager, CVector<int>& DispTable) {
	cout << "  [merging URLs]"; cout.flush();
	/* append a manager and return a disp table for Manager's URLs */
	/* initialize a clean disptable, each URL maps to itself */
    DispTable.RemoveAll();
    DispTable.SetSize(Manager.GetSize());
    /* for each URL in the Manager, add it into the local URLManager
	   and adjust the disptable */
    int DispCounter = 0;
	CProgress Progress(10);
	for (register int j=0;j<(int) Manager.GetSize();j++) {
		Progress.Show(j, Manager.GetSize());
		bool Added = false;                
		DispTable[j] = m_UrlTree.Add(Manager.GetUrlTree()[j], &Added);
		if (Added)
            DispCounter++;		
	}
	Progress.Finish();
	cout << "[" << DispCounter << "/" << Manager.GetSize() << " new URLs]" << endl;	
}
