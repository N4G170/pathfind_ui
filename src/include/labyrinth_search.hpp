#ifndef LABYRINTH_SEARCH
#define LABYRINTH_SEARCH

#include "structs.hpp"
#include "ui_structs.hpp"
#include <vector>
#include <mutex>
#include "sdl_gui_label.hpp"
#include "utils.hpp"
#include "map_renderer.hpp"

struct LabyrinthNode
{
    int index{-1};
    int node_x;
    int node_y;

    //<f> neigbours
    int top_index{-1};
    float top_cost{-1.f};
    int bottom_index{-1};
    float bottom_cost{-1.f};
    int left_index{-1};
    float left_cost{-1.f};
    int right_index{-1};
    float right_cost{-1.f};
    //</f> /neigbours

    //<f> Start/Target
    //if the cost is > -1 means it knows the target
    float cost_to_target{-1.f};
    //</f> /Start/Target
};

/**
 * \brief Find junction/corner nodes | (index, node)
 */
std::map<int, LabyrinthNode> FindNodes(MapData& map_data);

/**
 * \brief find the shortest path in th labyrinth
 */
std::string FindLabyrinthPath(MapData& map_data, MapRenderer* map_renderer, ControlFlags* flags, sdl_gui::Label* result_label, std::mutex* text_mutex);

/**
 * \brief Find the node closest to the given labyrinth position
 * \n Returns the index for the node in the nodes array
 */
int FindClosestNode(int position_index, std::vector<LabyrinthNode>& nodes, const int map_width, const int map_height);

void DrawLabyrinthPath(const std::vector<int>& path, std::map<int, LabyrinthNode>& nodes, MapRenderer* map_renderer, int map_width);
#endif //LABYRINTH_SEARCH
