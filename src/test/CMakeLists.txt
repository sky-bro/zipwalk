add_executable(test-my_inflate test-my_inflate.c)

target_link_libraries(test-my_inflate PUBLIC zipwalk-core ZLIB::ZLIB)
