set(TOOL_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/tool_base.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tool_crop.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tool_ellipse_select.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tool_eraser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tool_eyedropper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tool_lasso.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tool_magic_wand.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tool_move.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tool_pen.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tool_poly_lasso.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tool_rect_select.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tool_rotate_canvas.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tool_text.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tool_zoom.cpp
)

set(TOOL_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/tool_base.h
    ${CMAKE_CURRENT_LIST_DIR}/tool_crop.h
    ${CMAKE_CURRENT_LIST_DIR}/tool_ellipse_select.h
    ${CMAKE_CURRENT_LIST_DIR}/tool_eraser.h
    ${CMAKE_CURRENT_LIST_DIR}/tool_eyedropper.h
    ${CMAKE_CURRENT_LIST_DIR}/tool_lasso.h
    ${CMAKE_CURRENT_LIST_DIR}/tool_magic_wand.h
    ${CMAKE_CURRENT_LIST_DIR}/tool_move.h
    ${CMAKE_CURRENT_LIST_DIR}/tool_pen.h
    ${CMAKE_CURRENT_LIST_DIR}/tool_poly_lasso.h
    ${CMAKE_CURRENT_LIST_DIR}/tool_rect_select.h
    ${CMAKE_CURRENT_LIST_DIR}/tool_rotate_canvas.h
    ${CMAKE_CURRENT_LIST_DIR}/tool_text.h
    ${CMAKE_CURRENT_LIST_DIR}/tool_zoom.h
)

add_library(tool STATIC ${TOOL_SOURCES} ${TOOL_HEADERS})

target_include_directories(tool PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(tool PUBLIC
    base
    sys
)

