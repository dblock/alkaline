/*
    © Vestris Inc., Geneva, Switzerland
    http://www.vestris.com, 1994-1999 All Rights Reserved
    ______________________________________________

    written by Daniel Doubrovkine - dblock@vestris.com

*/

#ifndef _TEMPLATES_LIST_HPP
#define _TEMPLATES_LIST_HPP

#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4660 4700 )
#endif

#include <platform/tlist.hpp>

#include <Site/Site.hpp>
template class CList<CSite *>;
template class CListElement<CSite *>;
template ostream& operator<<(ostream&, CList<CSite *> const &);

#ifdef _WIN32
#pragma warning( pop )
#endif

#endif
