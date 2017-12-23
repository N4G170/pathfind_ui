#include "labyrinth_search.hpp"
#include "utils.hpp"
#include <map>
#include <algorithm>
#include "heuristics.hpp"

std::map<int, LabyrinthNode> FindNodes(MapData& map_data)
{
    std::map<int, LabyrinthNode> nodes;

    //<f> Find Nodes
    //run throught the map and find all corners and intersections
    for(int i = 0; i < map_data.map.size(); ++i)
    {
        if(map_data.map[i] <= 0)//invalid position
            continue;

        int node_x = GetCoordinateX( i, map_data.map_width );
        int node_y = GetCoordinateY( i, map_data.map_width );

        //check if not border position
        if(node_x <= 0)//node is at left
            continue;
        if(node_y <= 0)//node is at top
            continue;

        //node at least one position in
        //check NSWE positions for valid node
        int n{0}, s{0}, w{0}, e{0};

        if(map_data.map[i - map_data.map_width] > 0)//N, valid one line up
            n = i - map_data.map_width;
        if(map_data.map[i + map_data.map_width] > 0)//S, valid one line down
            s = i + map_data.map_width;

        if(map_data.map[i - 1] > 0)//L, valid one column left
            w = i - 1;
        if(map_data.map[i + 1] > 0)//E, valid one column right
            e = i + 1;

        //check if corner or intersection
        if((n > 0 || s > 0) && (w > 0 || e > 0))
        {
            //we store the node XY to avoid recalculating the in the future
            //nodes[i] = {i, node_x, node_y, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
        }
    }
    //</f> /Find Nodes

    //<f> Create Links
    //for each node, find its neigbours to the right and down, as the nodes are stored row major
    for(auto& node : nodes)
    {
        auto node_index{node.first};
        int node_x = GetCoordinateX( node_index, map_data.map_width );
        int node_y = GetCoordinateY( node_index, map_data.map_width );

        //move right until we find a node, a wall or the end of map
        if(map_data.map[node_index + 1] > 0)//we have a node to the right
        {
            int move_cost{0};
            for( auto j{node_index + 1}; j<map_data.map.size() ; ++j)
            {
                ++move_cost;
                //check if position has a valid nodes
                if(nodes.find(j) != std::end(nodes))//node exists
                {
                    //store connection
                    nodes[node_index].right_index = j;
                    nodes[node_index].right_cost = move_cost;
                    nodes[j].left_index = node_index;
                    nodes[j].left_cost = move_cost;

                    break;//we found the node so we continue
                }
            }
        }

        //move down until we find a node, a wall or the end of map
        if(map_data.map[node_index + map_data.map_width] > 0)//we have a node down
        {
            int move_cost{0};
            for( auto j{node_index + map_data.map_width}; j<map_data.map.size() ; j += map_data.map_width )
            {
            std::cout<<node_index<<" - "<<j<<std::endl;

                ++move_cost;
                //check if position has a valid nodes
                if(nodes.find(j) != std::end(nodes))//node exists
                {
                    //store connection
                    nodes[node_index].bottom_index = j;
                    nodes[node_index].bottom_cost = move_cost;
                    nodes[j].top_index = node_index;
                    nodes[j].top_cost = move_cost;

                    break;//we found the node so we continue
                }
            }
        }
    }
    //</f> /Create Links

    return nodes;
}

void FindLabyrinthPath(MapData& map_data, std::vector<DrawData>& draw_data_grid, MainControlFlags& flags, std::map<std::string, Text>& menu_texts, std::mutex& text_mutex)
{
    //get selected benchmark
    MapBenchmark benchmark = map_data.benchmarks[map_data.selected_bechmark_index];

    //prepare algorithm helper struct
    SearchData search_data;
    search_data.start_index = GetIndexFromCoordinate (benchmark.start_x, benchmark.start_y, map_data.map_width);
    search_data.target_index = GetIndexFromCoordinate (benchmark.target_x, benchmark.target_y, map_data.map_width);
    //select heuristic
    search_data.Heuristic = Manhattan;


    // <f> Prepare Map Render Vector (tag from editor custom fold)
    draw_data_grid.resize(map_data.map_height * map_data.map_width);

    for(int i = 0; i < map_data.map_height * map_data.map_width; i++)
        draw_data_grid[i] = DrawData{false, false, 0};

    if(search_data.start_index < map_data.map_height * map_data.map_width)
        draw_data_grid[search_data.start_index].start_or_target = 1;
    if(search_data.target_index < map_data.map_height * map_data.map_width)
        draw_data_grid[search_data.target_index].start_or_target = 2;
    // </f> (tag from editor custom fold)

    auto nodes{FindNodes(map_data)};

    std::cout<<nodes.size()<<std::endl;
    for(auto& node : nodes)
    {
        // if(node.second.bottom_index > 0)
        {
            // std::cout<<node.second.bottom_cost<<std::endl;
            draw_data_grid[node.first].visited = true;
        }
    }
    std::cout<<"END"<<std::endl;

    // auto start_closest{FindClosestNode(search_data.start_index, nodes, map_data.map_width, map_data.map_height)};
    // auto target_closest{FindClosestNode(search_data.target_index, nodes, map_data.map_width, map_data.map_height)};

    // draw_data_grid[nodes[start_closest].index].visited = true;
    // draw_data_grid[nodes[target_closest].index].visited = true;
    //
    // std::cout<<start_closest<<std::endl;
}

int FindClosestNode(int position_index, std::vector<LabyrinthNode>& nodes, const int map_width, const int map_height)
{
    //starting at map_position we move NSWE until we find a node or a wall, and then we return the closest node
    int node_index{-1};
    auto position_x{GetCoordinateX(position_index, map_width)};
    auto position_y{GetCoordinateY(position_index, map_width)};

    //<f> Find Valid Nodes

    //first get all node indices in same line and column
    struct node_data
    {
        int index;
        //store xy to avoid recalculating them
        int node_x;
        int node_y;
    };

    std::vector<node_data> line;
    std::vector<node_data> column;

    for(auto i{0}; i<nodes.size(); ++i)
    {
        auto node_x{GetCoordinateX(nodes[i].index, map_width)};
        auto node_y{GetCoordinateY(nodes[i].index, map_width)};

        //same line
        if(node_x == position_x)
            line.push_back({ i, node_x, node_y });
        else if(node_y == position_y)
            column.push_back({ i, node_x, node_y });
    }
    //</f> /Find Valid Nodes

    //<f> Check Inline Nodes
    //check all nodes in same line for the one closest
    int line_closest{-1};
    int line_distance{map_width};

    for(auto i{0}; i < line.size(); ++i)
    {
        auto distance{ std::abs(line[i].node_x - position_x) };

        if(distance < line_distance)
        {
            line_distance = distance;
            line_closest = static_cast<int>(i);
        }
    }
    //</f> /Check Inline Nodes

    //<f> Find Incolumn Nodes
    //check all nodes in same line for the one closest
    int column_closest{-1};
    int column_distance{map_height};
    for(auto i{0}; i<column.size(); ++i)
    {
        auto distance{ std::abs(column[i].node_y - position_y) };

        if(distance < column_distance)
        {
            column_distance = distance;
            column_closest = static_cast<int>(i);
        }
    }
    //</f> /Find Incolumn Nodes

    //check if closest is in line on column
    if(line_closest >= 0)//found valid line node
    {
        if(column_closest >= 0)//found both column and line valid nodes
        {
            if(line_distance < column_distance)
                node_index = line[line_closest].index;
            else
                node_index = column[column_closest].index;
        }
        else//only valid line node
        {
            node_index = line[line_closest].index;
        }
    }
    else if(column_closest >= 0)//found valid column node
    {
        node_index = column[column_closest].index;
    }

    //return closest node index, -1if it fails to find it
    return node_index;
}
