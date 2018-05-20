###########################
#         vulkan         #
###########################

set(INCLUDE_DIRECTORIES src/include)

set(SOURCE_FILES "")
set(FILES_PATH "src/src/")#file path from root
list(APPEND SOURCE_FILES "${FILES_PATH}main.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}clock.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}heuristics.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}menu.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}message_writer.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}utils.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}finder.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}sdl_init.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}map_renderer.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}map_loader.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}labyrinth_search.cpp")

set(HEADER_FILES "")
set(FILES_PATH "src/include/")#file path from root
list(APPEND HEADER_FILES "${FILES_PATH}clock.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}heuristics.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}config.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}map_loader.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}map_renderer.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}high_level_search.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}pathfind.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}ui_structs.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}menu.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}message_writer.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}utils.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}finder.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}structs.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}sdl_init.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}labyrinth_search.hpp")
