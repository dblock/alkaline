/*

    © Vestris Inc., Geneva, Switzerland
    http://www.vestris.com, 1994-1999 All Rights Reserved
    ______________________________________________

    written by Daniel Doubrovkine - dblock@vestris.com

*/

#include <platform/include.hpp>

#ifndef ALKALINE_SERVICE_HPP
#define ALKALINE_SERVICE_HPP

#ifdef _WIN32
#include <Server/NTService.hpp>

class CAlkalineService : public CNTService {    
public:
    CAlkalineService(void);
    virtual ~CAlkalineService(void);
};


#endif
#endif
