set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(target HelloQtImGui)

add_executable(${target})
target_sources(${target} PRIVATE 
    HelloQtImGui.cpp
)

# 设置预处理器定义
if(MSVC)
    target_compile_definitions(${target}
        PRIVATE UNICODE NOMINMAX
    )
endif()

set_target_properties(${target} PROPERTIES
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(${target} PRIVATE
    Qt5::Widgets ${PROJECT_NAME}
)

# 拷贝资源文件
add_custom_command(TARGET ${target} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${SOURCE_DIR}/data $<TARGET_FILE_DIR:${target}>/
)

DeployQtRuntime(TARGET ${target})
