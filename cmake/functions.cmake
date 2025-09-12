function(enable_warnings target_name)
    if(MSVC)
        target_compile_definitions("${target_name}" PRIVATE NOMINMAX)
        target_compile_options("${target_name}" PRIVATE "/W4")
        target_compile_options("${target_name}" PRIVATE "/Zc:__cplusplus") # Use the correct value for the __cplusplus macro
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
            "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR
            "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
        target_compile_options("${target_name}" PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)

        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            # Enable extra runtime checks in debug mode
            target_compile_definitions("${target_name}" PRIVATE
                    _GLIBCXX_ASSERTIONS _LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_EXTENSIVE)
        endif()
    endif()
endfunction()

function(enable_lto target_name)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result)
    if(result AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        set_target_properties("${target_name}" PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endfunction()
