set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(target ${PROJECT_NAME})
add_library(${target})
target_sources(${target} PRIVATE 
    imgui_impl_qt_opengl3.h 
    imgui_impl_qt_opengl3.cpp
    imgui_impl_qt.h 
    imgui_impl_qt.cpp
)

# 设置预处理器定义
if(MSVC)
    target_compile_definitions(${target}
        PRIVATE UNICODE NOMINMAX
    )
endif()

# 设置target属性
set_target_properties(${target} PROPERTIES
    AUTOMOC ON #自动moc
)

target_include_directories(${target}
    PUBLIC
    "$<INSTALL_INTERFACE:include>"
    "$<BUILD_INTERFACE:${SOURCE_DIR}>"
)

target_link_libraries(${target} PUBLIC
    Qt5::Widgets imgui
)
