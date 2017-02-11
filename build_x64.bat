@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
cls
set FILES="%cd%\sources\windows64_platform.cpp"
set LIBS="Kernel32.lib" "Advapi32.lib" "Shell32.lib"
pushd build
del *.pdb > NUL
rem CANNOT USE SOME C++ FEATURES, std lib is ripped off (https://hero.handmade.network/forums/code-discussion/t/94)
call cl.exe /nologo  /W0 /WX /EHa- /GS- /GR- /Od /Zi /FS /Fdbuild64.pdb /Febuild64.exe %FILES%  /link /INCREMENTAL:NO /NODEFAULTLIB /SUBSYSTEM:CONSOLE %LIBS%
POPD