@echo off
pushd %~dp0
devtools\bin\vpc.exe /da +everything /mksln Everything.sln
popd
pause
