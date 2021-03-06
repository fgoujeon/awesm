#Copyright Florian Goujeon 2021 - 2022.
#Distributed under the Boost Software License, Version 1.0.
#(See accompanying file LICENSE or copy at
#https://www.boost.org/LICENSE_1_0.txt)
#Official repository: https://github.com/fgoujeon/awesm

cmake_minimum_required(VERSION 3.10)

function(awesm_target_common_options TARGET)
    set_target_properties(
        ${TARGET} PROPERTIES
        CXX_STANDARD 17
    )

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(
            ${TARGET}
            PRIVATE
                -Wall -Wextra -Werror -pedantic -Werror
        )
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(
            ${TARGET}
            PRIVATE
                /W4 /WX /permissive-
        )
    endif()
endfunction()
