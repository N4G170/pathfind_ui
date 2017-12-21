#include "utils.hpp"

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
        return neighbors;

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
