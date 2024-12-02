cmake_minimum_required(VERSION 3.14)
project(scp-thread_utils-tests VERSION 1.0 LANGUAGES CXX C)

macro(run_test test)
    add_executable(${test} ${test}.cc)
    target_link_libraries(${test} scp::thread_utils SystemC::systemc)
    add_test(NAME ${test} COMMAND ${test})
    set_tests_properties(${test} PROPERTIES TIMEOUT 10)
endmacro()

run_test(realtimelimiter_test)
run_test(runonsysc_keepalive_test)
