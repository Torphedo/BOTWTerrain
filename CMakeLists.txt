cmake_minimum_required(VERSION 3.16)
project(hght
	VERSION 0.0.2
	LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
add_subdirectory(lib/bobtail)

add_executable(hght
	"main.c"
	"terrain.c"
)

target_link_libraries(hght bobtail)
