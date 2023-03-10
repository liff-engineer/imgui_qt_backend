include(FetchContent)
include(CMakePrintHelpers)

if(NOT TARGET imgui)
    FetchContent_Declare(
        imgui
        URL "ftp://yourhost/imgui-docking.zip"
        URL_HASH SHA512=4dae3e090f3c68148de1b37084047e0b985d8d0c92d07a0b0d05c18c714eaef273f0be7e87135aadf5da64bc7918343b5578097c480d8eef0a87e64be04a5c1f
        DOWNLOAD_NAME "imgui-docking.zip"
        DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/external/downloads/imgui  
    )
    FetchContent_GetProperties(imgui)

    if(NOT imgui_POPULATED)
        #下载并解压imgui源码包
        FetchContent_Populate(imgui)

        set(TARGET_NAME "imgui")
        set(SOURCE_DIR  ${imgui_SOURCE_DIR})
        file(GLOB SOURCE_FILES ${SOURCE_DIR}/*.h ${SOURCE_DIR}/*.cpp)
        add_library(${TARGET_NAME} ${SOURCE_FILES})
        target_include_directories(${TARGET_NAME} PUBLIC ${SOURCE_DIR})
        set_target_properties(${TARGET_NAME} PROPERTIES 
            FOLDER "external"
        )

        #设置imgui源码路径为全局变量
        set(IMGUI_SOURCE_DIR ${SOURCE_DIR} CACHE INTERNAL "imgui source directory")
    endif()
endif()
