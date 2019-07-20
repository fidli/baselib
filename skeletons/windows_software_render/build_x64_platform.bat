@echo off

set FILES="%cd%\sources\windows64_platform.cpp"
set LIBS="Kernel32.lib" "Advapi32.lib" "Gdi32.lib" "Shell32.lib" "User32.lib"
set BASELIB="%cd%\baselib"
set LIBDIR="%cd%\lib"
pushd build


rem CANNOT USE SOME C++ FEATURES, std lib is ripped off (https://hero.handmade.network/forums/code-discussion/t/94)
call cl.exe /nologo /W2 /WX /EHa- /GS- /GR- /Od /Zi /FS /I %BASELIB% /I %LIBDIR% /Fdplatform64.pdb /Feplatform64.exe %FILES%  /link /INCREMENTAL:NO /NODEFAULTLIB /SUBSYSTEM:CONSOLE %LIBS%

POPD