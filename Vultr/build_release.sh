/usr/bin/cmake --no-warn-unused-cli -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_C_COMPILER:FILEPATH=/usr/bin/clang-12 -DCMAKE_CXX_COMPILER:FILEPATH=/usr/bin/clang++ -H. -Brelease -G "Unix Makefiles"

/usr/bin/cmake --build release --config RelWithDebInfo --target all -- -j 10
