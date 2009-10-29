/*
    © Vestris Inc., Geneva, Switzerland
    http://www.vestris.com, 1994-1999 All Rights Reserved
    ______________________________________________

    written by Daniel Doubrovkine - dblock@vestris.com

*/

#ifndef _TEMPLATES_BASE_HPP
#define _TEMPLATES_BASE_HPP

#include <platform/tvector.hpp>

#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4660 4700 )
#endif

template class CVector<char *>;
template ostream& operator<<(ostream&, CVector<char *> const &);
template class CVector<long *>;
template ostream& operator<<(ostream&, CVector<long *> const &);
       
#include <SwapVector/SwapVector.hpp>
template class CVector<CSwapVector *>;
template ostream& operator<<(ostream&, CVector<CSwapVector *> const &);

#include <StringA/StringA.hpp>

typedef CVector<CStringA> CStringAVector;

template class CVector<CStringA>;
template ostream& operator<<(ostream&, CVector<CStringA> const &);

template class CVector<CStringAVector>;
template ostream& operator<<(ostream&, CVector<CStringAVector> const &);

#include <Config/Config.hpp>
template class CVector<CConfig>;
template ostream& operator<<(ostream&, CVector<CConfig> const &);

#include <Letter/Letter.hpp>
#include <Vector/SVector.hpp>
template class CVector<CLetter *>;
template class CVector<short int>;
template class CVector<unsigned short int>;

#ifdef _WIN32
#pragma warning( pop )
#endif

#endif
