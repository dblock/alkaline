@echo off

if "%~1"=="" goto :Usage

setlocal

rem
rem Change current path to the target (bin)
rem

echo Running in %~1 ...

cd "%~1"

for /F "tokens=1,2,*" %%i in ( ..\ASearch\Main\bldver.hpp ) do (
 if not "%%j"=="" (
  set %%j=%%k
 )
)

set pbAsearchPath=asearch.%RC_VERSION_MAJOR%.%RC_VERSION_MINOR%
set pbTargetPath=%~1\%pbAsearchPath%
echo Building distribution of version %RC_VERSION_STRING% in %pbTargetPath%

if exist "%pbTargetPath%" ( rd /s/q "%pbTargetPath%" )
mkdir "%pbTargetPath%"

for %%i in ( README CONTRIBUTIONS COPYRIGHT asearch.exe ) do (
 echo Copying %%i to %pbTargetPath% ...
 if exist "%pbTargetPath%\%%i" attrib -r -s -h "%pbTargetPath%\%%i"
 copy %%i "%pbTargetPath%" /y
)

rem
rem Admin
rem

mkdir "%pbTargetPath%\Admin"
echo Copying ..\admin\*.* to "%pbTargetPath%\Admin" ...
xcopy /y/q ..\admin\*.* "%pbTargetPath%\Admin" /s

rem
rem Demos
rem

mkdir "%pbTargetPath%\Demos"
echo Copying ..\Demos\*.* to "%pbTargetPath%\Demos" ...
xcopy /y/q ..\demos\*.* "%pbTargetPath%\Demos" /s
move "%pbTargetPath%\Demos\global.cnf" "%pbTargetPath%"

rem 
rem AlkalineTools
rem

echo Copying tools to "%pbTargetPath%\tools"
mkdir "%pbTargetPath%\Tools\Perl"
xcopy /y/q/s ..\..\AlkalineTools\perl\*.* "%pbTargetPath%\Tools\Perl"
xcopy /y/q ..\..\AlkalineTools\bin\*.exe "%pbTargetPath%\Tools"

rem 
rem Cleanup
rem

echo Cleaning up "%pbTargetPath%\Admin\as-images-src" ...
rd /s/q "%pbTargetPath%\Admin\as-images-src"

dir /s/b/ad "%pbTargetPath%\Admin\*cvs" > %temp%\alkaline-tmp.dir
dir /s/b/ad "%pbTargetPath%\Tools\*cvs" >> %temp%\alkaline-tmp.dir
dir /s/b/ad "%pbTargetPath%\Demos\*cvs" >> %temp%\alkaline-tmp.dir
for /F "delims=" %%i in ( %temp%\alkaline-tmp.dir ) do (
 echo Cleaning up %%i ...
 rd /s/q "%%i"
)
del %temp%\alkaline-tmp.dir

if exist "%pbTargetPath%\asearch.opt" ( 
 del /q "%pbTargetPath%\asearch.opt" > nul 
)

if exist "%pbTargetPath%\asearch.ilk" ( 
 del /q "%pbTargetPath%\asearch.ilk" > nul 
)

rem
rem Documentation and distribution
rem

set TZ=PST8PDT

if exist ..\..\SgmlDocs\alkaline\alkaline-distrib.zip (
 echo Uncompressing documentation ...
 unzip32.exe -q ..\..\SgmlDocs\alkaline\alkaline-distrib.zip
 move alkaline-distrib "%pbTargetPath%\Docs"
)

if exist ..\..\SgmlDocs\alkaline-faq\alkaline-faq-distrib.zip (
 echo Uncompressing faqs ...
 unzip32.exe -q ..\..\SgmlDocs\alkaline-faq\alkaline-faq-distrib.zip
 move alkaline-faq-distrib "%pbTargetPath%\Faqs"
)

echo Building archive asearch.%RC_VERSION_MAJOR%.%RC_VERSION_MINOR%.WindowsNT-x86.zip ...
if exist asearch.%RC_VERSION_MAJOR%.%RC_VERSION_MINOR%.WindowsNT-x86.zip (
 del asearch.%RC_VERSION_MAJOR%.%RC_VERSION_MINOR%.WindowsNT-x86.zip
)

zip32.exe -q -r asearch.%RC_VERSION_MAJOR%.%RC_VERSION_MINOR%.WindowsNT-x86.zip "%pbAsearchPath%\*.*"

endlocal
goto :EOF

:Usage
echo postbuild.cmd [target path]
goto :EOF