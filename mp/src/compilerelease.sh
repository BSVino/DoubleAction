#!/bin/bash
pushd "$(dirname "$0")" > /dev/null
"/c/Program Files (x86)/MSBuild/12.0/Bin/MSBuild.exe" DoubleAction.sln -p:Configuration=Release
popd > /dev/null
