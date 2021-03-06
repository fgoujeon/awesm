#Copyright Florian Goujeon 2021 - 2022.
#Distributed under the Boost Software License, Version 1.0.
#(See accompanying file LICENSE or copy at
#https://www.boost.org/LICENSE_1_0.txt)
#Official repository: https://github.com/fgoujeon/awesm

cmake_minimum_required(VERSION 3.10)

include(awesm)

set(TARGET awesm_test)

file(GLOB_RECURSE SOURCE_FILES *)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${SOURCE_FILES})
add_executable(${TARGET} ${SOURCE_FILES})

awesm_target_common_options(${TARGET})

target_link_libraries(
    ${TARGET}
    PRIVATE
        awesm
)

if(TARGET Catch2::Catch2WithMain) #v3
    target_compile_definitions(
        ${TARGET}
        PRIVATE
            AWESM_CATCH2_VERSION=3
    )
    target_link_libraries(
        ${TARGET}
        PRIVATE
            Catch2::Catch2WithMain
    )
else() #v2
    target_compile_definitions(
        ${TARGET}
        PRIVATE
            AWESM_CATCH2_VERSION=2
    )
    target_link_libraries(
        ${TARGET}
        PRIVATE
            Catch2::Catch2
    )
endif()

add_test(
    NAME ${TARGET}
    COMMAND ${TARGET}
)
