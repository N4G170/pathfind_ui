#ifndef HEURISTICS_H
#define HEURISTICS_H

#include <algorithm>

/**
* \brief Just returns a zero, because Dijkstra search algorithm is A* with heuristic zero
*/
float Dijkstra(const int& delta_x, const int& delta_y, const float& line_cost, const float& diagonal_cost, const int& start_delta_x, const int& start_delta_y);

/**
* \brief Manhattan distance with abs "2D cross product" for tie break.
*/
float Manhattan(const int& delta_x, const int& delta_y, const float& line_cost, const float& diagonal_cost, const int& start_delta_x, const int& start_delta_y);

/**
* \brief Octile distance with abs "2D cross product" for tie break.
*/
float Octile(const int& delta_x, const int& delta_y, const float& line_cost, const float& diagonal_cost, const int& start_delta_x, const int& start_delta_y);


#endif //HEURISTICS_H
