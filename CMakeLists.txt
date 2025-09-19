cmake_minimum_required(VERSION 3.16)
project(web-scraper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(TBB REQUIRED)
find_package(CURL REQUIRED)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/url/Url.cpp
)

# Bolje je spojiti u jednu liniju ili dodati PRIVATE
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
    TBB::tbb
    CURL::libcurl
)