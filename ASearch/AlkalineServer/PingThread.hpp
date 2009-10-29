/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  _____________________________________________________
  
  written by Daniel Doubrovkine - dblock@vestris.com
  
*/

#ifndef ALKALINE_PING_THREAD_HPP
#define ALKALINE_PING_THREAD_HPP

#include <platform/include.hpp>
#include <Server/PingClient.hpp>

class CAlkalinePingThread : public CPingClient {
    property(int, Pid);
    property(int, PingRestart);
public:
    CAlkalinePingThread(const CString& PingUrl, int Pid);
    virtual void Execute(void * Arguments);
    virtual ~CAlkalinePingThread(void);
    virtual void PingFailed(int Count) const;
    virtual void PingSucceeded(void) const;
};

#endif
