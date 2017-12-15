#ifndef HEURISTICS_H
#define HEURISTICS_H

#include <algorithm>

/**
* \brief Just returns a zero, because Dijkstra search algorithm is A* with heuristic zero
*/
float Dijkstra(const int& delta_x, const int& delta_y, const float& line_cost, const float& diagonal_cost, const int& start_delta_x, const int& start_delta_y)
{
    return 0;
}

/**
* \brief Manhattan distance with abs "2D cross product" for tie break.
*/
float Manhattan(const int& delta_x, const int& delta_y, const float& line_cost, const float& diagonal_cost, const int& start_delta_x, const int& start_delta_y)
{
    //return (delta_x + delta_y) * line_cost;

    int delta_x_abs = std::abs(delta_x);
    int delta_y_abs = std::abs(delta_y);
    //return (line_cost * (delta_x + delta_y) + (diagonal_cost - 2 * line_cost) * std::min(delta_x, delta_y));

    float heuristics = (delta_x_abs + delta_y_abs);

    //this last value addition needs to be very, but very small or A* will degrade to best-first and fail some tests
    //0.0000001 is used because the vectors are not normalized (sqrt is too expensive here)
    return heuristics + std::abs(delta_x * start_delta_y - start_delta_x * delta_y) * 0.0000001;//cross product
}

/**
* \brief Octile distance with abs "2D cross product" for tie break.
*/
float Octile(const int& delta_x, const int& delta_y, const float& line_cost, const float& diagonal_cost, const int& start_delta_x, const int& start_delta_y)
{
    int delta_x_abs = std::abs(delta_x);
    int delta_y_abs = std::abs(delta_y);
    //return (line_cost * (delta_x + delta_y) + (diagonal_cost - 2 * line_cost) * std::min(delta_x, delta_y));

    float heuristics = (line_cost * (delta_x_abs + delta_y_abs) + (diagonal_cost - 2 * line_cost) * std::min(delta_x_abs, delta_y_abs));

    //this last value addition needs to be very, but very small or A* will degrade to best-first and fail some tests
    //0.0000001 is used because the vectors are not normalized (sqrt is too expensive here)
    return heuristics + std::abs(delta_x * start_delta_y - start_delta_x * delta_y) * 0.0000001;//cross product
}


#endif //HEURISTICS_H
