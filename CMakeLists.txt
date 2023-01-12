# ============================================================== 1. Visual Code 설치 
# C/C++
# C/C++ Extension Pack
# CMake
# CMake Tool

# ============================================================== 2. 플랫폼별 컴파일러 설치 
# for windows : visual studio 20xx c++ 설치
# for mac : xcode 설치, sudo xcode-select --install / sudo xcode-select --reset
# for linux : sudo apt-get update && sudo apt-get install gcc-9

# ============================================================== 3. cmake 설치
# for windows : 사이트에서 설치 https://cmake.org 
# for mac : brew install cmake
# brew 설치 명령 : /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
# for linux : sudo apt-get install cmake

# ============================================================== 4. git 설치
# https://git-scm.com/ (윈도우는 설치해야함)
# https://git-fork.com/
#  (ubuntu) 
# sudo apt-get install git
# sudo apt-get install libxrandr-dev
# sudo apt-get install libxinerama-dev
# sudo apt-get install libxcursor-dev
# sudo apt-get install libxi-dev
# sudo apt-get install libmesa-dev
# sudo apt-get install libgl-dev

# ============================================================== 5. glad 설치
# 파이썬 설치(https://www.python.org/downloads/)

# ============================================================== 주의 사항
# 3군데 플랫폼별
# target_link_libraries
# target_compile_definitions OS=${OS_WINDOW}
# set(DEP_LIBS ${DEP_LIBS} spdlog 

# ==============================================================
# 파일열기 처리시 현재 경로를 터미널 경로로 실행(.vscode/settings.json)
#{
#    "cmake.debugConfig": {
#        "cwd": "${workspaceFolder}"
#    }
#}

# ============================================================== 탭 변경
# 셋팅 열어서 ctrl(cmd) + ,
#{
#    "editor.tabSize": 4,
#    "editor.insertSpaces": true,
#    "editor.detectIndentation": false
#}

cmake_minimum_required(VERSION 3.13)

set(PROJECT_NAME opengl_example)
set(CMAKE_CXX_STANDARD 17)

set(WINDOW_NAME "Hello, OpenGL!")
set(WINDOW_WIDTH 960)
set(WINDOW_HEIGHT 540)
set(OS_WINDOW 0)
set(OS_MAC 1)
set(OS_UBUNTU 2)

project(${PROJECT_NAME})
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/common.cpp src/common.h
    src/shader.cpp src/shader.h
    src/program.cpp src/program.h
    src/context.cpp src/context.h
	src/buffer.cpp src/buffer.h
	src/vertex_layout.cpp src/vertex_layout.h
	src/image.cpp src/image.h
	src/texture.cpp src/texture.h
    )

include(Dependency.cmake)

# 우리 프로젝트에 include / lib 관련 옵션 추가
target_include_directories(${PROJECT_NAME} PUBLIC ${DEP_INCLUDE_DIR})
target_link_directories(${PROJECT_NAME} PUBLIC ${DEP_LIB_DIR})
# for windows
target_link_libraries(${PROJECT_NAME} PUBLIC ${DEP_LIBS})
# for mac
#target_link_libraries(${PROJECT_NAME} PUBLIC ${DEP_LIBS}
#    "-framework OpenGL"
#    "-framework Cocoa"
#    "-framework IOKit"
#    "-framework CoreVideo"
#    )
# for ubuntu
#target_link_libraries(${PROJECT_NAME} PUBLIC ${DEP_LIBS} X11)

target_compile_definitions(${PROJECT_NAME} PUBLIC
    WINDOW_NAME="${WINDOW_NAME}"
    WINDOW_WIDTH=${WINDOW_WIDTH}
    WINDOW_HEIGHT=${WINDOW_HEIGHT}
    OS_WINDOW=${OS_WINDOW}
    OS_MAC=${OS_MAC}
    OS_UBUNTU=${OS_UBUNTU}
    OS=${OS_WINDOW}
#    OS=${OS_MAC}
#    OS=${OS_UBUNTU}
    )

# Dependency들이 먼저 build 될 수 있게 관계 설정
add_dependencies(${PROJECT_NAME} ${DEP_LIST})
