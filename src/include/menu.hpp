#ifndef MENU_HPP
#define MENU_HPP

#include <map>
#include "text.hpp"
#include "ui_structs.hpp"
#include <SDL.h>

/**
* \brief Initialize the std::map with the Text objects that own all rendered text
*/
void  InitMenu(MainPointers& main_pointers, std::map<std::string, Text>& menu_text_objects);

/**
* \brief Renders all the Text objects and the nodes color legend
*/
void RenderMenu(MainPointers& main_pointers, std::map<std::string, Text>& menu_text_objects, GridImagePointers& image_pointers, const bool& show_warning, std::mutex& text_mutex);

#endif//MENU_HPP
