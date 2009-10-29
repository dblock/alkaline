/*

    © Vestris Inc., Geneva, Switzerland
    http://www.vestris.com, 1994-1999 All Rights Reserved
    ______________________________________________

    written by Daniel Doubrovkine - dblock@vestris.com

*/

#include <alkaline.hpp>

#ifdef _WIN32
#include "AlkalineService.hpp"

CAlkalineService::CAlkalineService(void) {
    m_ServiceName = "AlkalineSE";
    m_ServiceDisplayName = "Alkaline Search Engine";    
	m_ServiceDescription = "Provides web and local searching and indexing capabilities.";
}

CAlkalineService::~CAlkalineService(void) {

}

#endif
