# BrushEngine
set(BASE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/color_light.cpp
    ${CMAKE_CURRENT_LIST_DIR}/geometry.cpp
    ${CMAKE_CURRENT_LIST_DIR}/image_process.cpp
    ${CMAKE_CURRENT_LIST_DIR}/interpolation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/log.cpp
    ${CMAKE_CURRENT_LIST_DIR}/random.cpp
    ${CMAKE_CURRENT_LIST_DIR}/stroke.cpp
)

set(BASE_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/base_math.h
    ${CMAKE_CURRENT_LIST_DIR}/base_types.h
    ${CMAKE_CURRENT_LIST_DIR}/gaussRand.h
    ${CMAKE_CURRENT_LIST_DIR}/log.h
)

add_library(base STATIC ${BASE_SOURCES} ${BASE_HEADERS})

target_include_directories(base PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(base PUBLIC
    spdlog
)

