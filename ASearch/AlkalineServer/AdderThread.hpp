/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  _____________________________________________________
  
  written by Daniel Doubrovkine - dblock@vestris.com

  URL Adder Thread, part of the Alkaline Search Engine
  adds addresses posted online asynchronously from the
  reindexing thread, allows to return control to the
  client immediately and schedule a longer term
  operation

*/

#ifndef ALKALINE_ADDER_THREAD
#define ALKALINE_ADDER_THREAD

#include <platform/include.hpp>
#include <String/String.hpp>
#include <Site/Site.hpp>
#include <Thread/Thread.hpp>

typedef enum { AtAdd, AtRemove } CAdderOperation;

class CAdderThread : public CThread {
	property(CAdderOperation, AdderOperation);
	copy_property(CSite *, Site);
	property(CString, Url);
public:
	CAdderThread(void);
	virtual ~CAdderThread(void);
	virtual void Execute(void * Arguments);
};

#endif
