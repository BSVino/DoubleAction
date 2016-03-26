@echo off
pushd %~dp0
devtools\bin\vpc.exe /da +game /mksln DoubleAction.sln /2013
popd
pause
