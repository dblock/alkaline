# Microsoft Developer Studio Project File - Name="AlkalineNT" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=AlkalineNT - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "AlkalineNT.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "AlkalineNT.mak" CFG="AlkalineNT - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "AlkalineNT - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "AlkalineNT - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "AlkalineNT - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /O2 /Ob2 /I "../ASearch" /I "../../BaseClasses" /I "../../BaseClasses/platform" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FR /Yu"alkaline.hpp" /FD /c
# SUBTRACT CPP /X
# ADD BASE RSC /l 0x100c /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 libcimt.lib libcmt.lib kernel32.lib wsock32.lib BaseClasses.lib Advapi32.lib Secur32.lib /nologo /subsystem:console /machine:I386 /nodefaultlib /out:"..\bin\Alkaline.1.7\asearch.exe" /libpath:"..\..\BaseClasses\MSVC\Release" /libpath:"..\..\Rockall\MSVC\Release" /libpath:"..\MSVC\Release"
# SUBTRACT LINK32 /debug
# Begin Special Build Tool
TargetDir=\CVS\Alkaline\bin\Alkaline.1.7
SOURCE="$(InputPath)"
PostBuild_Cmds=..\bin\postbuild.cmd $(TargetDir)
# End Special Build Tool

!ELSEIF  "$(CFG)" == "AlkalineNT - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /ZI /Od /I "../ASearch" /I "../../BaseClasses" /I "../../BaseClasses/platform" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR /Yu"alkaline.hpp" /FD /GZ /c
# SUBTRACT CPP /X
# ADD BASE RSC /l 0x100c /d "_DEBUG"
# ADD RSC /l 0x100c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 libcimtd.lib libcmtd.lib kernel32.lib wsock32.lib BaseClasses.lib Advapi32.lib /nologo /subsystem:console /debug /machine:I386 /nodefaultlib /out:"..\bin\Alkaline.1.7\asearch.exe" /libpath:"..\..\BaseClasses\MSVC\Debug" /libpath:"..\..\Rockall\MSVC\Debug" /libpath:"..\MSVC\Debug"
# Begin Special Build Tool
TargetDir=\CVS\Alkaline\bin\Alkaline.1.7
SOURCE="$(InputPath)"
PostBuild_Cmds=..\bin\postbuild.cmd $(TargetDir)
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "AlkalineNT - Win32 Release"
# Name "AlkalineNT - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\ASearch\Mv4\AccessManager.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\AlkalineServer\AdderThread.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Mv4\AdminManager.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Main\alkaline.cpp
# ADD CPP /Yc"alkaline.hpp"
# End Source File
# Begin Source File

SOURCE=..\ASearch\AlkalineData\AlkalineData.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\AlkalineParser\AlkalineParser.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\AlkalineServer\AlkalineServer.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\AlkalineServer\AlkalineService.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\AlkalineSession\AlkalineSession.cpp
# End Source File
# Begin Source File

SOURCE="..\ASearch\Main\asearch-base.cpp"
# End Source File
# Begin Source File

SOURCE="..\ASearch\Main\asearch-ext.cpp"
# End Source File
# Begin Source File

SOURCE="..\ASearch\Main\asearch-list.cpp"
# End Source File
# Begin Source File

SOURCE=..\ASearch\Main\asearch.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Cache\Cache.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Session\Certif.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Session\CertifiedServer.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Config\Config.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Config\ConfigBase.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Mv4\EquivManager.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\FixedAllocator\FixedAllocator.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Config\GlobalCnf.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Index\Index.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Index\IndexManager.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Index\IndexObject.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Index\IndexOptions.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Index\IndexPool.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Index\IndexSearch.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Index\INFManager.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Letter\Letter.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\LettersHandler\LettersHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\LettersTree\LettersTree.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Index\LNXManager.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\AlkalineServer\PingThread.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Rank\Rank.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Search\Search.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Search\SearchObject.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Session\Session.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Site\Site.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Site\SiteIndex.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Site\SiteOptions.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Site\SiteSearch.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Sort\Sort.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\StringA\StringA.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Index\URL404Object.cpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Index\URLManager.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\ASearch\Mv4\AccessManager.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\AlkalineServer\AdderThread.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Mv4\AdminManager.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Main\alkaline.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\AlkalineData\AlkalineData.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\AlkalineParser\AlkalineParser.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\AlkalineServer\AlkalineServer.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\AlkalineServer\AlkalineService.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\AlkalineSession\AlkalineSession.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Main\bldver.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Cache\Cache.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Session\Certif.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Session\CertifiedServer.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Config\Config.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Config\ConfigBase.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Mv4\EquivManager.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\FixedAllocator\FixedAllocator.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Config\GlobalCnf.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Index\Index.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Index\IndexManager.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Index\IndexObject.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Index\IndexOptions.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Index\IndexPool.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Index\INFManager.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Letter\Letter.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\LettersHandler\LettersHandler.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\LettersTree\LettersTree.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Index\LNXManager.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\AlkalineServer\PingThread.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Rank\Rank.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Search\Search.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Search\SearchObject.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Search\SearchTypes.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Session\Session.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Site\Site.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Sort\Sort.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\StringA\StringA.hpp
# End Source File
# Begin Source File

SOURCE=..\..\BaseClasses\Swap\SwapVector.hpp
# End Source File
# Begin Source File

SOURCE="..\ASearch\templates\templates-base.hpp"
# End Source File
# Begin Source File

SOURCE="..\ASearch\templates\templates-ext.hpp"
# End Source File
# Begin Source File

SOURCE="..\ASearch\templates\templates-list.hpp"
# End Source File
# Begin Source File

SOURCE=..\ASearch\Main\TraceTags.hpp
# End Source File
# Begin Source File

SOURCE=..\..\BaseClasses\platform\tvector.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Index\URL404Object.hpp
# End Source File
# Begin Source File

SOURCE=..\ASearch\Index\URLManager.hpp
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\ASearch\Main\bldver.rc
# End Source File
# Begin Source File

SOURCE=..\ASearch\Main\icon.ico
# End Source File
# End Group
# End Target
# End Project
