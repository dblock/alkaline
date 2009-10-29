/*
    © Vestris Inc., Geneva, Switzerland
    http://www.vestris.com, 1994-1999 All Rights Reserved
    ______________________________________________

    written by Daniel Doubrovkine - dblock@vestris.com

    Revision history:

*/

#ifndef ALKALINE_INDEX_POOL_HPP
#define ALKALINE_INDEX_POOL_HPP

#include <platform/include.hpp>
#include <Thread/ThreadPool.hpp>
#include <Index/IndexObject.hpp>
    
class CIndexPool : public CThreadPool {
public:
  CIndexPool(void);
  virtual ~CIndexPool(void);  
  bool AddIndexJob(const CIndexObject& IndexObject);
};

#endif
