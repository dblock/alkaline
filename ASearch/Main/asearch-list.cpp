#include <alkaline.hpp>

#include <templates/templates-list.hpp>

#ifdef _UNIX
template class CVector<CINFElement>;

#include <RegExp/RegExp.hpp>
template class CVector<CRegExp>;
#endif


