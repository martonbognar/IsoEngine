set(inc )
set(src main.cpp)
add_executable(souls ${inc} ${src})
target_include_directories(souls PUBLIC include)
target_link_libraries (souls isoengine)
