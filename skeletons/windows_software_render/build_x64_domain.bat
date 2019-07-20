@echo off

mkdir build
set FILES="%cd%\sources\domain.cpp"
set LIBS=""
set BASELIB="%cd%\baselib"
set LIBDIR="%cd%\lib"
pushd build

set preunique=%time: =0%
set unique=%preunique:~0,2%%preunique:~3,2%%preunique:~6,2%%preunique:~9,2%

rem CANNOT USE SOME C++ FEATURES, std lib is ripped off (https://hero.handmade.network/forums/code-discussion/t/94)
call cl.exe /nologo /LD /EHsc /FI "Windows.h" /FI "%BASELIB%\platform\windows_types.h" /W2 /WX /EHa- /GS- /GR- /Od /Zi /FS /I %BASELIB%  /I %LIBDIR% /Fd"domain_%unique%.pdb" /Fedomain.dll %FILES% /link /INCREMENTAL:NO /NODEFAULTLIB %LIBS% /PDB:"domain_%unique%.pdb"


popd