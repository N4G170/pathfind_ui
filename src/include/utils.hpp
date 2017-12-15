#ifndef UTILS_H
#define UTILS_H

#include <queue>
#include "structs.hpp"
#include "config.hpp"

bool IsNew(bit2 status) { return status == NEW_NODE; }
bool IsOpen(bit2 status) { return status == OPEN_NODE; }
bool IsClosed(bit2 status) { return status == CLOSED_NODE; }

/**
    \brief Extract the X coordinate from the row-major array index
*/
int GetCoordinateX(const int& index, const int& map_width){ return index % map_width; }
/**
    \brief Extract the Y coordinate from the row-major array index
*/
int GetCoordinateY(const int& index, const int& map_width){ return index / map_width; }
/**
    \brief Calculate the row-major array index of a given x, y pair
*/
int GetIndexFromCoordinate(const int& x, const int& y, const int& map_width){ return y * map_width + x; }

/**
    \brief Checks if a given node position is passable or not (invalid positions are marked as impassable)
*/
const bool IsPassable(const int& x, const int& y, MapData& map_data)
{
    int index = y * map_data.map_width + x;
    return x >= 0 && x < (map_data.map_width) && y >= 0 && y < (map_data.map_height) && map_data.map[index] == 1;//passable
}

/**
\brief Return a dictionary value of the user defined default value. This function can be made generic for other container types.
*/
template< typename T, typename U>
U GetMapValue(std::map<T,U>& container, const T& key, const U& default_value)
{
    auto search = container.find(key);
    if(search == container.end())//if the key does not exist, create it and set default value
        container[key] = default_value;
    return container[key];
}

/**
* \brief Gets the neighbors of a given node. Can process 4 and 8 directions, depending on the map_data var for selection.
*/
std::queue< std::pair<int,DIRECTION> > GetNeighbors(const int& current_index, MapData& map_data)
{
    std::queue< std::pair<int,DIRECTION> > neighbors;

    //check adjacent cells
    unsigned short current_node_y = GetCoordinateY(current_index, map_data.map_width);
    unsigned short current_node_x = GetCoordinateX(current_index, map_data.map_width);

    bool has_north = false, has_south = false;
    bool has_east = false, has_west = false;

    bool has_nw = false, has_ne = false;
    bool has_sw = false, has_se = false;

    //up
    if(IsPassable(current_node_x, current_node_y - 1, map_data))//has row above
    {
        neighbors.push( std::make_pair(GetIndexFromCoordinate(current_node_x, current_node_y - 1, map_data.map_width), DIRECTION::ORTHOGONAL) );//move one up
        has_north = true;
    }
    //down
    if(IsPassable(current_node_x, current_node_y + 1, map_data))//has row bellow
    {
        neighbors.push( std::make_pair(GetIndexFromCoordinate(current_node_x, current_node_y + 1, map_data.map_width), DIRECTION::ORTHOGONAL) );//move one down
        has_south = true;
    }
    //left
    if(IsPassable(current_node_x - 1, current_node_y, map_data))//has left
    {
        neighbors.push( std::make_pair(GetIndexFromCoordinate(current_node_x - 1, current_node_y, map_data.map_width), DIRECTION::ORTHOGONAL) );//move one left
        has_west = true;
    }
    //right
    if(IsPassable(current_node_x + 1, current_node_y, map_data))//has right
    {
        neighbors.push( std::make_pair(GetIndexFromCoordinate(current_node_x + 1, current_node_y, map_data.map_width), DIRECTION::ORTHOGONAL) );//move one right
        has_east = true;
    }

    if(map_data.search_type == SEARCH_TYPE::QUARTILE) //return only NSEW
        return std::move(neighbors);

    //get diagonals with no corner cut
    has_ne = has_north && has_east;
    has_nw = has_north && has_west;
    has_se = has_south && has_east;
    has_sw = has_south && has_west;

    if(has_ne && IsPassable(current_node_x + 1, current_node_y - 1, map_data))
        neighbors.push( std::make_pair(GetIndexFromCoordinate(current_node_x + 1, current_node_y - 1, map_data.map_width), DIRECTION::DIAGONAL) );

    if(has_nw && IsPassable(current_node_x - 1, current_node_y - 1, map_data))
        neighbors.push( std::make_pair(GetIndexFromCoordinate(current_node_x - 1, current_node_y - 1, map_data.map_width), DIRECTION::DIAGONAL) );

    if(has_se && IsPassable(current_node_x + 1, current_node_y + 1, map_data))
        neighbors.push( std::make_pair(GetIndexFromCoordinate(current_node_x + 1, current_node_y + 1, map_data.map_width), DIRECTION::DIAGONAL) );

    if(has_sw && IsPassable(current_node_x - 1, current_node_y + 1, map_data))
        neighbors.push( std::make_pair(GetIndexFromCoordinate(current_node_x - 1, current_node_y + 1, map_data.map_width), DIRECTION::DIAGONAL) );

    return std::move(neighbors);
}
#endif //UTILS_H
