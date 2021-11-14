/usr/bin/cmake --no-warn-unused-cli -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_BUILD_TYPE:STRING=Debug -H. -Bbuild -G "Unix Makefiles"

if /usr/bin/cmake --build build --config Debug --target all -- -j 10 ; then
  ClangBuildAnalyzer --all build/CMakeFiles/VultrShared.dir build/capture_file
  ClangBuildAnalyzer --analyze build/capture_file
fi
