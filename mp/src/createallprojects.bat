@echo off
pushd %~dp0
devtools\bin\vpc.exe /da +everything /mksln Everything.sln /2010
popd
pause
