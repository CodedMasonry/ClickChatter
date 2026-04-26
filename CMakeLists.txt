cmake_minimum_required(VERSION 3.20)
project(ClickChatter)
set(CMAKE_CXX_STANDARD 17)

find_path(MINIAUDIO_INCLUDE_DIRS "miniaudio.h")

add_executable(main main.cpp audio.cpp)

target_include_directories(main PRIVATE ${MINIAUDIO_INCLUDE_DIRS})

if(UNIX AND NOT APPLE)
    target_link_libraries(main PRIVATE pthread m dl)
elseif(APPLE)
    target_link_libraries(main PRIVATE
        "-framework CoreAudio"
        "-framework AudioToolbox"
        "-framework AudioUnit"
        "-framework CoreFoundation"
        m)
elseif(WIN32)
    target_link_libraries(main PRIVATE ole32 winmm)
endif()
