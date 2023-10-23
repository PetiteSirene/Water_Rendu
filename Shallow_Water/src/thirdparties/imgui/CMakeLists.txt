file(GLOB imgui_SRC *.cpp *.h)
file(GLOB imgui_HEADERS *.h)
add_library(
        imgui
        ${imgui_SRC}
        backends/imgui_impl_opengl3.cpp
        backends/imgui_impl_opengl3.h
        backends/imgui_impl_glfw.cpp
        backends/imgui_impl_glfw.h
)
target_link_libraries(imgui PUBLIC glfw glad)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DIMGUI_IMPL_OPENGL_LOADER_GLAD2")
target_include_directories(
        imgui
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/backends
)

