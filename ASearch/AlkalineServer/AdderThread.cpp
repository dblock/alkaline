/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  _____________________________________________________
  
  written by Daniel Doubrovkine - dblock@vestris.com

*/

#include <alkaline.hpp>

#include "AdderThread.hpp"

CAdderThread::CAdderThread(void) {
	m_Site = NULL;
	m_AdderOperation = AtAdd;
}

CAdderThread::~CAdderThread(void) {

}

void CAdderThread::Execute(void * Arguments) {
	switch(m_AdderOperation) {
	case AtAdd:
		if (m_Site) {
			m_Site->ProcessURLSingle(m_Url, true);			
		}
		break;
	case AtRemove:
		if (m_Site) {
			m_Site->RemovePage(m_Url);			
		}
		break;
	default:
		break;
	}
	
	m_Site->Write(false, true);
	CThread::Execute(Arguments);
}
