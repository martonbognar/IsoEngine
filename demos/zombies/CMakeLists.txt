set(inc include/entity.h include/game.h)
set(src src/entity.cpp src/game.cpp main.cpp)
add_executable(zombies ${inc} ${src})
target_include_directories(zombies PUBLIC include)
target_link_libraries (zombies isoengine)
