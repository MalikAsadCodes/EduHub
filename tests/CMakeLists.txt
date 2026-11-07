# Each tst_*.cpp is its own Qt Test program, registered with CTest.
#   cmake --build --preset debug
#   ctest --preset debug

function(edu_add_test name)
    cmake_parse_arguments(ARG "GUI" "" "" ${ARGN})
    qt_add_executable(${name} ${name}.cpp)
    if(ARG_GUI)
        target_link_libraries(${name} PRIVATE edu_ui Qt6::Test)
    else()
        target_link_libraries(${name} PRIVATE edu_core Qt6::Test)
    endif()
    add_test(NAME ${name} COMMAND ${name})
    if(ARG_GUI)
        # Runs without opening real windows; screenshots go to build/<preset>/screenshots
        set_tests_properties(${name} PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
        target_compile_definitions(${name} PRIVATE
            EDU_SCREENSHOT_DIR="${CMAKE_BINARY_DIR}/screenshots")
    endif()
endfunction()

edu_add_test(tst_sqlscript)
edu_add_test(tst_database)
edu_add_test(tst_migrator)
edu_add_test(tst_settings)
edu_add_test(tst_gui GUI)
