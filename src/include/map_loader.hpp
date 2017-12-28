#ifndef MAP_LOADER_H
#define MAP_LOADER_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <system_error>
#include <string>
#include <vector>

#include "structs.hpp"

/**
 * \brief Tries to load the map list
 */
bool LoadMapList( std::vector<std::string>& map_list );

/**
* \brief Load a map from its file
*/
bool LoadMap(const std::string& map_name, MapData& data);

//string manipulation


std::string Trim(const std::string& str);

std::string TrimLeft(const std::string& str);

std::string TrimRight(const std::string& str);

/**
* \brief Returns a string vector with the given string separated by the separator
*/
std::vector< std::string > Explode(const std::string& str, const std::string& separator);

#endif //MAP_LOADER_H
