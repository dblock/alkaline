#ifdef _WIN32
#ifndef ALKALINE_PCH_HPP
#define ALKALINE_PCH_HPP

#if _MSC_VER > 1000
 #pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN

#include <platform/baseclasses.hpp>
#include <AlkalineData\AlkalineData.hpp>
#include <AlkalineParser\AlkalineParser.hpp>
#include <AlkalineServer\AdderThread.hpp>
#include <AlkalineServer\AlkalineServer.hpp>
#include <AlkalineServer\AlkalineService.hpp>
#include <AlkalineSession\AlkalineSession.hpp>
#include <Cache\Cache.hpp>
#include <Config\ConfigBase.hpp>
#include <Config\Config.hpp>
#include <Config\GlobalCnf.hpp>
#include <Index\Index.hpp>
#include <Index\IndexManager.hpp>
#include <Index\INFManager.hpp>
#include <Index\LNXManager.hpp>
#include <Index\URLManager.hpp>
#include <Mv4\AccessManager.hpp>
#include <Mv4\AdminManager.hpp>
#include <Mv4\EquivManager.hpp>
#include <Search\Search.hpp>
#include <Search\SearchTypes.hpp>
#include <Search\SearchObject.hpp>
#include <Session\Certif.hpp>
#include <Session\CertifiedServer.hpp>
#include <Session\Session.hpp>
#include <Site\Site.hpp>
#include <StringA\StringA.hpp>

#endif
#endif
