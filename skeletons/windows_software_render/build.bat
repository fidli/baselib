mkdir build

pushd build

del *.pdb

popd

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\amd64\vcvars64.bat"

call build_x64_platform.bat

call build_x64_domain.bat




