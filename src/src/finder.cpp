#include "finder.hpp"

using namespace std::chrono_literals;

/**
* \brief Process a node and checks its neighbors storing them if valid
*/
bool FinderStep(SearchData& search_data, MapData& map_data, std::vector<DrawData>& draw_data_grid, MapBenchmark& benchmark)
{
    //get a copy of the node with lowest cost
    Node current_node = *search_data.to_visit.begin();
    //remove the first cell as we are processing it
    search_data.to_visit.erase(search_data.to_visit.begin());

    search_data.node_status[current_node.index] = CLOSED_NODE;

    //update ui
    draw_data_grid[current_node.index].visited = true;

    //we found the destination
    if(current_node.index == search_data.target_index)
    {
        map_data.path_buffer.push_back( current_node.index );//store target node
        int parent_index = search_data.parents[current_node.index];//get current node parent from paretns map
        map_data.min_path_cost = search_data.g_costs[current_node.index];//get cost to get to target

        while(parent_index != search_data.start_index)//valid index
        {
            map_data.path_buffer.push_back( parent_index );//store parent node
            parent_index = search_data.parents[parent_index];//get next parent
        }

        return true;//found path
    }

    //go search for neighbors
    std::queue< std::pair<int,DIRECTION> > neighbors_queue = GetNeighbors(current_node.index, map_data);

    while(neighbors_queue.size() > 0)
    {
        auto neighbor = neighbors_queue.front();
        neighbors_queue.pop();//remove first

        if( IsClosed(search_data.node_status[neighbor.first]) )//node already processed in a previous step
            continue;

        int neighbor_index = neighbor.first;//this var is used for clarity only
        int x = GetCoordinateX(neighbor_index, map_data.map_width);
        int y = GetCoordinateY(neighbor_index, map_data.map_width);

        //calculate the g_cost to neighbor from current node
        float new_neighbor_g_cost = search_data.g_costs[current_node.index] + ((neighbor.second == DIRECTION::ORTHOGONAL) ? map_data.line_cost : map_data.diagonal_cost);

        if(!IsOpen(search_data.node_status[neighbor_index]) || new_neighbor_g_cost < GetMapValue<int, float>(search_data.g_costs, neighbor_index, map_data.map.size()))//new node or one with lower g_cost
        {
            search_data.g_costs[neighbor_index] = new_neighbor_g_cost;//set new cost
            search_data.parents[neighbor_index] = current_node.index;

            Node new_node{ neighbor_index, new_neighbor_g_cost,
                search_data.Heuristic(x - benchmark.target_x, y - benchmark.target_y, map_data.line_cost, map_data.diagonal_cost,
                                        benchmark.start_x - benchmark.target_x, benchmark.start_y - benchmark.target_y) };

            if(!IsOpen(search_data.node_status[neighbor_index]))//add new to open set
            {
                search_data.to_visit.insert(new_node);
                search_data.node_status[neighbor_index] = OPEN_NODE;
                //UI update
                draw_data_grid[neighbor_index].to_check = true;
            }
            else//update node in the set
            {
                //custom condition find
                auto search_result = std::find_if(search_data.to_visit.begin(), search_data.to_visit.end(),
                [&neighbor_index](const Node& stored_node)
                {
                    return stored_node.index == neighbor_index;
                });

                if(search_result != search_data.to_visit.end())//found item
                {
                    search_data.to_visit.erase(search_result);//remove old
                    search_data.to_visit.insert(new_node);//insert updated node
                }
            }
        }
    }//while(neighbors_queue.size() > 0)

    return false;
}
