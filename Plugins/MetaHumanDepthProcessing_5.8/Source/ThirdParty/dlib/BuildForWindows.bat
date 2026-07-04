REM *** x64 ***

if exist Build (rmdir Build /s/q)
mkdir Build
pushd Build

cmake -G "Visual Studio 16 2019" -T version=14.29 -DCMAKE_INSTALL_PREFIX=Install -DDLIB_USE_MKL_FFT=0 -DDLIB_USE_BLAS=0 -DDLIB_USE_LAPACK=0 -DDLIB_USE_CUDA=0 -DDLIB_JPEG_SUPPORT=0 -DDLIB_PNG_SUPPORT=0 -DDLIB_GIF_SUPPORT=0 -DDLIB_LINK_WITH_SQLITE3=0 ../Source

"%_msbuild%msbuild.exe" dlib_project.sln /t:build /p:Configuration=Release

"%_msbuild%msbuild.exe" INSTALL.vcxproj /t:build /p:Configuration=Release

md ..\Lib\Win64\Release\
copy /y Install\lib\dlib19.23.0_release_64bit_msvc1929.lib ..\Lib\Win64\Release\dlib19.23.0_release_64bit_msvc1929.lib

if exist ..\Include\dlib (rmdir ..\Include\dlib /s/q)
xcopy /y/s/i Install\include\dlib ..\Include\dlib

popd






REM *** ARM64 ***

if exist Build (rmdir Build /s/q)
mkdir Build
pushd Build

REM @fixme: USE_NEON_INSTRUCTIONS isn't implemented in dlib for MSVC. 
cmake -G "Visual Studio 16 2019" -T version=14.29 -A arm64 -DUSE_SSE2_INSTRUCTIONS=0 -DCMAKE_INSTALL_PREFIX=Install -DDLIB_USE_MKL_FFT=0 -DDLIB_USE_BLAS=0 -DDLIB_USE_LAPACK=0 -DDLIB_USE_CUDA=0 -DDLIB_JPEG_SUPPORT=0 -DDLIB_PNG_SUPPORT=0 -DDLIB_GIF_SUPPORT=0 -DDLIB_LINK_WITH_SQLITE3=0 ../Source

"%_msbuild%msbuild.exe" dlib_project.sln /t:build /p:Configuration=Release

"%_msbuild%msbuild.exe" INSTALL.vcxproj /t:build /p:Configuration=Release

md ..\Lib\WinArm64\Release\
copy /y Install\lib\dlib19.23.0_release_64bit_msvc1929.lib ..\Lib\WinArm64\Release\dlib19.23.0_release_64bit_msvc1929.lib

if exist ..\Include\dlib (rmdir ..\Include\dlib /s/q)
xcopy /y/s/i Install\include\dlib ..\Include\dlib

popd
