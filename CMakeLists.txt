cmake_minimum_required(VERSION 3.16)
project(hght
	VERSION 0.0.1
	LANGUAGES C
)

set(CMAKE_C_STANDARD 99)

add_executable(hght
	"main.c"
	"logging.c"
)

