@echo off
pushd %~dp0
devtools\bin\vpc.exe /da +game /mksln DoubleAction.sln
popd
pause
