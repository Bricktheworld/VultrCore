/usr/bin/cmake --no-warn-unused-cli -H. -Brelease -G "Unix Makefiles"

/usr/bin/cmake --build release --config RelWithDebInfo --target all -- -j 10
