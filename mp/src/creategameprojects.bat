@echo off
pushd %~dp0
::sed -ri '/PLACEHOLDER_COMPILER_ARGS/ s/^.*$/git rev-parse HEAD/e; T; s/^.*$/$PreprocessorDefinitions "$BASE;DA_GIT_VERSION=\0"/' game/client/client_da.vpc game/server/server_da.vpc
devtools\bin\vpc.exe /da +game /mksln DoubleAction.sln /2013
call :append_global DoubleAction.sln
popd
pause
goto :eof

:append_global
findstr /c:"GlobalSection" %1 >nul 2>&1 && goto :eof
echo Global>> %1
echo 	GlobalSection(SolutionConfigurationPlatforms) = preSolution>> %1
echo 		Debug^|Win32 = Debug^|Win32>> %1
echo 		Release^|Win32 = Release^|Win32>> %1
echo 	EndGlobalSection>> %1
echo 	GlobalSection(ProjectConfigurationPlatforms) = postSolution>> %1
for %%G in ({D7D97049-1C31-93A3-06DC-BD9899FAC989} {97110F3D-F33B-B887-FFEF-A4F9CCC0ED9D} {01BCD5C5-3BBB-7F2F-4185-9ADBF4AD8C0C} {BAB92FF0-D72A-D7E5-1988-74628D39B94F} {95D67225-8415-236F-9128-DCB171B7DEC6} {1F48B55A-3BBB-9D49-9B29-6A461ED483CE} {EC1C516D-E1D9-BC0A-F79D-E91E954ED8EC} {F69B3672-C5E8-CD1A-257F-253A25B5B939}) do (
echo 		%%G.Debug^|Win32.ActiveCfg = Debug^|Win32>> %1
echo 		%%G.Debug^|Win32.Build.0 = Debug^|Win32>> %1
echo 		%%G.Release^|Win32.ActiveCfg = Release^|Win32>> %1
echo 		%%G.Release^|Win32.Build.0 = Release^|Win32>> %1
)
echo 	EndGlobalSection>> %1
echo 	GlobalSection(SolutionProperties) = preSolution>> %1
echo 		HideSolutionNode = FALSE>> %1
echo 	EndGlobalSection>> %1
echo EndGlobal>> %1
goto :eof
