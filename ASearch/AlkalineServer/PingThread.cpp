/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  _____________________________________________________
  
  written by Daniel Doubrovkine - dblock@vestris.com

*/

#include <alkaline.hpp>
#include <Main/TraceTags.hpp>
#include "PingThread.hpp"

CAlkalinePingThread::CAlkalinePingThread(const CString& PingUrl, int Pid) : 
    CPingClient(PingUrl),
    m_Pid(Pid)
{
    m_PingInterval = 15;
    m_PingRestart = 3;
}

CAlkalinePingThread::~CAlkalinePingThread(void) {
    
}

void CAlkalinePingThread::Execute(void * Arguments) {
    CPingClient::Execute(Arguments);
}

void CAlkalinePingThread::PingFailed(int Count) const {
    cout << "[ping: failed (url=" << m_PingUrl.GetHttpAll() << ", count=" << Count << "/" << m_PingRestart << ")]" << endl;
    if ((Count >= m_PingRestart) && m_bPing) {
#ifdef _UNIX
        if (! m_Pid) {
            cout << "[ping: attempting a clean restart]" << endl;
            kill(base_getpid(), SIGTERM); 
            kill(base_getpid(), SIGKILL); 
            _exit(0);
        } else {
            cout << "[ping: attempting signal SIGKILL " << m_Pid << "]" << endl;
            kill(m_Pid, SIGTERM); 
            kill(m_Pid, SIGKILL); 
        }
#endif
    }
}

void CAlkalinePingThread::PingSucceeded(void) const {
    Trace(tagServer, levInfo, ("CAlkalinePingThread::PingSucceeded - ping-pong."));    
}

