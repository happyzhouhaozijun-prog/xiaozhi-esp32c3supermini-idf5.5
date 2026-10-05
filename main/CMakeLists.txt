set(SOURCES "audio_codecs/audio_codec.cc"
            "audio_codecs/no_audio_codec.cc"

            
            "led/circular_strip.cc"
            "display/display.cc"
            "display/no_display.cc"

            
            "protocols/protocol.cc"

            "system_info.cc"
            "application.cc"
            "ota.cc"
            "settings.cc"
            "background_task.cc"
            "main.cc"
            )

set(INCLUDE_DIRS "." "display" "audio_codecs" "protocols" )



# 添加板级公共文件
file(GLOB BOARD_COMMON_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/boards/common/*.cc)
list(APPEND SOURCES ${BOARD_COMMON_SOURCES})
list(APPEND INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/boards/common)

# 根据 BOARD_TYPE 配置添加对应的板级文件

set(BOARD_TYPE "esp32c3-supermini")    
file(GLOB BOARD_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/boards/${BOARD_TYPE}/*.cc)
list(APPEND SOURCES ${BOARD_SOURCES})


list(APPEND SOURCES "protocols/mqtt_protocol.cc")






file(GLOB ASSETS ${CMAKE_CURRENT_SOURCE_DIR}/assets/zh/*.p3)

idf_component_register(SRCS ${SOURCES}
                    EMBED_FILES ${ASSETS}
                    INCLUDE_DIRS ${INCLUDE_DIRS}
                    WHOLE_ARCHIVE
                    )

# 使用 target_compile_definitions 来定义 BOARD_TYPE
target_compile_definitions(${COMPONENT_LIB}
                    PRIVATE BOARD_TYPE=\"${BOARD_TYPE}\"
                    )
