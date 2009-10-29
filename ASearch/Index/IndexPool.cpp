/*
    © Vestris Inc., Geneva, Switzerland
    http://www.vestris.com, 1994-1999 All Rights Reserved
    ______________________________________________

    written by Daniel Doubrovkine - dblock@vestris.com

    Revision history:

*/

#include <alkaline.hpp>
#include "IndexPool.hpp"
    
CIndexPool::CIndexPool(void) {
  // _L_DEBUG(4, cout << "CIndexPool :: Starting Index Pool - Jobs list of " << m_JobsList.GetSize() << " elements." << endl);  
  m_MaxThreads = 10;
  m_MaxQueueSize = -1;  
}
    
CIndexPool::~CIndexPool(void) {
  // _L_DEBUG(4, cout << "CIndexPool :: Stopping Index Pool" << endl);
}
    
static void IndexPoolExecuteThreadFunction(void * IndexObjectPointer) {
  ((CIndexObject *) IndexObjectPointer)->Execute();
  delete (CIndexObject *) IndexObjectPointer;
}
    
bool CIndexPool::AddIndexJob(const CIndexObject& IndexObject) {
  // _L_DEBUG(4, cout << "CIndexPool :: adding index job: [" << IndexObject.GetUrl() << "]" << endl);
  CIndexObject * NewIndexObject = new CIndexObject(IndexObject);
  // _L_DEBUG(4, cout << "CIndexPool :: new index object allocated]" << endl);  
  CThreadPoolJob ThreadPoolJob((void *) IndexPoolExecuteThreadFunction, (void *) NewIndexObject);
  // _L_DEBUG(4, cout << "CIndexPool :: new job object reserved]" << endl);  
  while (!CThreadPool::AddJob(ThreadPoolJob)) {
    // _L_DEBUG(4, cout << "CIndexPool :: queue is full, sleeping." << endl);    
    base_sleep(1);
  }
  // _L_DEBUG(4, cout << "CIndexPool :: added [" << IndexObject.GetUrl() << "]" << endl);        
  return true;
}

