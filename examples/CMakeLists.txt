cmake_minimum_required(VERSION 3.14)
project(scp-tests VERSION 1.0 LANGUAGES CXX C)

macro(run_test test)
    add_executable(${test} ${test}.cc)
    target_link_libraries(${test} scp::tlm_extensions::initiator_id scp::tlm_extensions::path_trace scp::reporting SystemC::systemc SystemC::cci-config)
    add_test(NAME ${test} COMMAND ${test})
    set_tests_properties(${test} PROPERTIES TIMEOUT 10)
endmacro()

run_test(smoke)
run_test(smoke_report)
