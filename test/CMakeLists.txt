project(span-ext-test)

add_executable(span-ext-test test.cxx)
target_link_libraries(span-ext-test span-ext)

add_test(NAME span-ext COMMAND span-ext-test)
