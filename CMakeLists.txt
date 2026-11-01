cmake_minimum_required(VERSION 3.16)
project(xcloud_client CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

add_library(xcloud STATIC
    src/http_client.cpp
    src/auth.cpp
    src/stream.cpp
)
target_include_directories(xcloud PUBLIC include)
target_link_libraries(xcloud PUBLIC CURL::libcurl nlohmann_json::nlohmann_json)

add_executable(xcloud_cli main.cpp)
target_link_libraries(xcloud_cli PRIVATE xcloud)

# ---- Switch (libnx) 集成说明 ----
# 在 libnx 工程里，把 src/*.cpp 加入构建，并提供一个实现 xcloud::HttpClient
# 的 libnx HTTP 后端（替换 libcurl），鉴权 / 信令逻辑无需改动。
# 典型 devkitA64 编译：
#   add_compile_options(-D__SWITCH__)
#   target_link_libraries(xcloud nx)
