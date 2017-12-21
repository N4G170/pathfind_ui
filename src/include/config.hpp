#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include "SDL.h"
#include <bitset>

//flags used to mark the status of a node
using bit2 = std::bitset<2>;
const bit2 CLOSED_NODE{2};
const bit2 OPEN_NODE{1};
const bit2 NEW_NODE{0};

//sdl window config
const std::string window_name{ "Paradox Pathfind" };
const int window_position_x{ SDL_WINDOWPOS_UNDEFINED };
const int window_position_y{ SDL_WINDOWPOS_UNDEFINED };
const int window_width{ 1400 };
const int window_height{ 1050 };
const Uint64 window_flags{ SDL_WINDOW_SHOWN };

//paths
const std::string path_data{ "data/" };
const std::string path_img{ "data/imgs/" };
const std::string path_map{ "data/maps/" };
const std::string path_font{ "data/font/" };

//extensions
const std::string file_extension_map{ ".map" };
const std::string file_extension_map_scenario{ ".map.scen" };

const std::string font_name{ "dejavusansmono.ttf" };

//benchmarks
const int max_benchmarks{10};

#endif//CONFIG_H
