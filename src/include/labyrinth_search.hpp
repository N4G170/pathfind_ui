#ifndef LABYRINTH_SEARCH
#define LABYRINTH_SEARCH

#include "structs.hpp"
#include "ui_structs.hpp"
#include <mutex>
#include <vector>

struct LabyrinthNode
{
    int index;
    int node_x;
    int node_y;

    //<f> neigbours
    int top_index{-1};
    int top_cost{-1};
    int bottom_index{-1};
    int bottom_cost{-1};
    int left_index{-1};
    int left_cost{-1};
    int right_index{-1};
    int right_cost{-1};
    //</f> /neigbours

    //<f> Start/Target
    //if the node is the close to the start or the target we store it and the cost to it
    int start_index{-1};
    int start_cost{-1};
    int target_index{-1};
    int target_cost{-1};
    //</f> /Start/Target
};

/**
 * \brief Find junction/corner nodes
 */
std::map<int, LabyrinthNode> FindNodes(MapData& map_data);

/**
 * \brief find the shortest path in th labyrinth
 */
void FindLabyrinthPath(MapData& map_data, std::vector<DrawData>& draw_data_grid, MainControlFlags& flags, std::map<std::string, Text>& menu_texts, std::mutex& text_mutex);

/**
 * \brief Find the node closest to the given labyrinth position
 * \n Returns the index for the node in the nodes array
 */
int FindClosestNode(int position_index, std::vector<LabyrinthNode>& nodes, const int map_width, const int map_height);
#endif //LABYRINTH_SEARCH
