#ifndef UTILS_H
#define UTILS_H

#include <queue>
#include "structs.hpp"
#include "config.hpp"

inline bool IsNew(bit2 status) { return status == NEW_NODE; }
inline bool IsOpen(bit2 status) { return status == OPEN_NODE; }
inline bool IsClosed(bit2 status) { return status == CLOSED_NODE; }

/**
    \brief Extract the X coordinate from the row-major array index
*/
inline int GetCoordinateX(const int& index, const int& map_width){ return index % map_width; }
/**
    \brief Extract the Y coordinate from the row-major array index
*/
inline int GetCoordinateY(const int& index, const int& map_width){ return index / map_width; }
/**
    \brief Calculate the row-major array index of a given x, y pair
*/
inline int GetIndexFromCoordinate(const int& x, const int& y, const int& map_width){ return y * map_width + x; }

/**
    \brief Checks if a given node position is passable or not (invalid positions are marked as impassable)
*/
inline const bool IsPassable(const int& x, const int& y, MapData& map_data)
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
std::queue< std::pair<int,DIRECTION> > GetNeighbors(const int& current_index, MapData& map_data);

/**
* \brief Generates a random number between min and max, includes both limits
*/
inline int RandomGenerator(int min = 0, int max = 4)
{
    std::random_device seed;
    std::default_random_engine engine( seed() );
    std::uniform_int_distribution<int> dist(min, max);

    return dist(engine);
}

#endif //UTILS_H
