@echo off

:: Get the Steam install directory from the registry.
for /f "tokens=2*" %%a in ('reg query HKCU\Software\Valve\Steam /v SteamPath') do set "steaminstall=%%b"

:: Copy all mod files over.
xcopy . "%steaminstall%\steamapps\common\SourceFilmmaker\game\usermod\" /E /Y

:: Reset the gameinfo.txt to the default SFM one.
xcopy scripts\sfm\gameinfo.txt "%steaminstall%\steamapps\common\SourceFilmmaker\game\usermod\" /E /Y

:: Remove the binaries as they will interfere.
rmdir "%steaminstall%\steamapps\common\SourceFilmmaker\game\usermod\bin" /s /q

pause
