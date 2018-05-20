#include "labyrinth_search.hpp"
#include "utils.hpp"
#include <map>
#include <algorithm>
#include "heuristics.hpp"
#include "clock.hpp"
#include "message_writer.hpp"

using namespace std::chrono_literals;

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

        //node at least one position in
        //check NSWE positions for valid node
        unsigned short n{0}, s{0}, w{0}, e{0};

        //check up and down
        if((node_y > 0) && map_data.map[i - map_data.map_width] > 0)//N, valid one line up
            n = 1;
        if((node_y < map_data.map_height - 1) && map_data.map[i + map_data.map_width] > 0)//S, valid one line down
            s = 1;

        if((node_x > 0) && map_data.map[i - 1] > 0)//L, valid one column left
            w = 1;
        if((node_x < map_data.map_width - 1) && map_data.map[i + 1] > 0)//E, valid one column right
            e = 1;

        //check if corner or intersection or end dead end((n + s + e + w) == 1)
        if( ((n + s + e + w) == 1) || ((n || s) && (w || e)))
        {
            nodes[i] = {i, node_x, node_y, -1, -1, -1, -1, -1, -1, -1, -1, -1};
        }
    }
    //</f> /Find Nodes

    //<f> Create Links
    //for each node, find its neigbours to the right and down, as the nodes are stored row major
    for(auto& node : nodes)
    {
        auto node_index{node.first};
        int node_x = node.second.node_x;
        int node_y = node.second.node_y;

        //move right until we find a node, a wall or the end of map
        if((node_x < map_data.map_width -1) && map_data.map[node_index + 1] > 0)//we have a node to the right
        {
            int move_cost{0};
            for( auto j{node_index + 1}; j < map_data.map.size() ; ++j)
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
        if((node_y < map_data.map_height - 1) && map_data.map[node_index + map_data.map_width] > 0)//we have a node down
        {
            int move_cost{0};
            for( auto j{node_index + map_data.map_width}; j<map_data.map.size() ; j += map_data.map_width )
            {
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

std::string FindLabyrinthPath(MapData& map_data, MapRenderer* map_renderer, ControlFlags* flags, sdl_gui::Label* result_label, std::mutex* text_mutex)
{
    //number of cells analyzed
    unsigned int operations = 0;
    unsigned int thread_sleeps = 0;
    //clock to count algorithm time cost
    unsigned long clock_id = Clock::Instance()->StartClock();

    //get selected benchmark
    MapBenchmark benchmark = map_data.benchmarks[map_data.selected_bechmark_index];

    //prepare algorithm helper struct
    SearchData search_data;
    search_data.start_index = GetIndexFromCoordinate (benchmark.start_x, benchmark.start_y, map_data.map_width);
    search_data.target_index = GetIndexFromCoordinate (benchmark.target_x, benchmark.target_y, map_data.map_width);
    //select heuristic
    search_data.Heuristic = Manhattan;

    auto nodes{FindNodes(map_data)};

    //<f> Prepare Map Render Vector (tag from editor custom fold)
    //resize and reset vector
    map_renderer->ResizeVector(map_data.map_height * map_data.map_width);

    if(search_data.start_index < map_data.map_height * map_data.map_width)
        map_renderer->SetStartNode(search_data.start_index);
    if(search_data.target_index < map_data.map_height * map_data.map_width)
        map_renderer->SetTargetNode(search_data.target_index);
    //</f> (tag from editor custom fold)

    //<f> Register start and target nodes

    //<f> Register start node or neighbours to be visited
    //check if start node exists as a labirinth node
    if(nodes.find(search_data.start_index) != std::end(nodes))//start index has a labirinth node
    {
        //start node
        Node first_node{search_data.start_index, 0,
            search_data.Heuristic(benchmark.start_x - benchmark.target_x, benchmark.start_y - benchmark.target_y,
                map_data.line_cost, map_data.diagonal_cost, benchmark.start_x - benchmark.target_x, benchmark.start_y - benchmark.target_y )};//base cost of start is 0

        search_data.to_visit.insert(first_node);//inserts the first cell in the set so we can start searching
        search_data.parents[search_data.start_index] = -1;//the start cell has no parent
    }
    else//we need to find the nodes that neighbour the starting index
    {
        //search up
        auto index{search_data.start_index - map_data.map_width};//we start in the position above
        auto cost{map_data.line_cost};//path cost (we start with map_data.line_cost because we start in the position above)

        while(index >= 0)
        {
            if(map_data.map[index] <= 0)//we found a wall
                break;

            //we at at a valid position so we check it there is a node here
            if(nodes.find(index) != std::end(nodes))//there is, so we add it as a neighbour
            {
                Node neighbour{index, cost,
                    search_data.Heuristic(nodes[index].node_x - benchmark.target_x, nodes[index].node_y - benchmark.target_y,
                    map_data.line_cost, map_data.diagonal_cost, benchmark.start_x - benchmark.target_x, benchmark.start_y - benchmark.target_y )};

                search_data.to_visit.insert(neighbour);//inserts start index neighbours in the set so we can start searching
                search_data.parents[index] = search_data.start_index;
                search_data.g_costs[index] = cost;
                break;
            }
            index -= map_data.map_width;//move another position up
            ++cost;
        }

        //search down
        index = search_data.start_index + map_data.map_width;//we start in the position bellow
        cost = map_data.line_cost;

        while(index < (map_data.map_height * map_data.map_width))
        {
            if(map_data.map[index] <= 0)//we found a wall
                break;

            //we at at a valid position so we check it there is a node here
            if(nodes.find(index) != std::end(nodes))//there is, so we add it as a neighbour
            {
                Node neighbour{index, cost,
                    search_data.Heuristic(nodes[index].node_x - benchmark.target_x, nodes[index].node_y - benchmark.target_y,
                    map_data.line_cost, map_data.diagonal_cost, benchmark.start_x - benchmark.target_x, benchmark.start_y - benchmark.target_y )};

                search_data.to_visit.insert(neighbour);//inserts start index neighbours in the set so we can start searching
                search_data.parents[index] = search_data.start_index;
                search_data.g_costs[index] = cost;
                break;
            }
            index += map_data.map_width;//move another position down
            ++cost;
        }

        //search left
        index = search_data.start_index - 1;
        cost = map_data.line_cost;

        while(index >= 0)
        {
            if(map_data.map[index] <= 0)//we found a wall
                break;

            //we at at a valid position so we check it there is a node here
            if(nodes.find(index) != std::end(nodes))//there is, so we add it as a neighbour
            {
                Node neighbour{index, cost,
                    search_data.Heuristic(nodes[index].node_x - benchmark.target_x, nodes[index].node_y - benchmark.target_y,
                    map_data.line_cost, map_data.diagonal_cost, benchmark.start_x - benchmark.target_x, benchmark.start_y - benchmark.target_y )};

                search_data.to_visit.insert(neighbour);//inserts start index neighbours in the set so we can start searching
                search_data.parents[index] = search_data.start_index;
                search_data.g_costs[index] = cost;
                break;
            }
            index -= 1;
            ++cost;
        }

        //search right
        index = search_data.start_index + 1;
        cost = map_data.line_cost;

        while(index < (map_data.map_height * map_data.map_width))
        {
            if(map_data.map[index] <= 0)//we found a wall
                break;

            //we at at a valid position so we check it there is a node here
            if(nodes.find(index) != std::end(nodes))//there is, so we add it as a neighbour
            {
                Node neighbour{index, cost,
                    search_data.Heuristic(nodes[index].node_x - benchmark.target_x, nodes[index].node_y - benchmark.target_y,
                    map_data.line_cost, map_data.diagonal_cost, benchmark.start_x - benchmark.target_x, benchmark.start_y - benchmark.target_y )};

                search_data.to_visit.insert(neighbour);//inserts start index neighbours in the set so we can start searching
                search_data.parents[index] = search_data.start_index;
                search_data.g_costs[index] = cost;
                break;
            }
            index += 1;
            ++cost;
        }
    }
    //</f> /Register start node or neighbours to be visited

    //<f> Register target
    //check if start node exists as a labirinth node
    if(nodes.find(search_data.target_index) != std::end(nodes))//start index has a labirinth node
    {
        nodes[search_data.target_index].cost_to_target = 0;
    }
    else//we need to find the nodes that neighbour the starting index
    {
        //search up
        auto index{search_data.target_index - map_data.map_width};//we start in the position above
        auto cost{map_data.line_cost};

        while(index >= 0)
        {
            if(map_data.map[index] <= 0)//we found a wall
                break;
            //we at at a valid position so we check it there is a node here
            if(nodes.find(index) != std::end(nodes))//there is, so we add it as a neighbour
            {
                nodes[index].cost_to_target = cost;
                break;
            }

            index -= map_data.map_width;//move another position up
            ++cost;
        }

        //search down
        index = search_data.target_index + map_data.map_width;//we start in the position bellow
        cost = map_data.line_cost;

        while(index < (map_data.map_height * map_data.map_width))
        {
            if(map_data.map[index] <= 0)//we found a wall
                break;

            //we at at a valid position so we check it there is a node here
            if(nodes.find(index) != std::end(nodes))//there is, so we add it as a neighbour
            {
                nodes[index].cost_to_target = cost;
                break;
            }

            index += map_data.map_width;//move another position down
            ++cost;
        }

        //search left
        index = search_data.target_index - 1;
        cost = map_data.line_cost;

        while(index >= 0)
        {
            if(map_data.map[index] <= 0)//we found a wall
                break;

            //we at at a valid position so we check it there is a node here
            if(nodes.find(index) != std::end(nodes))//there is, so we add it as a neighbour
            {
                nodes[index].cost_to_target = cost;
                break;
            }

            index -= 1;
            ++cost;
        }

        //search right
        index = search_data.target_index + 1;
        cost = map_data.line_cost;

        while(index < (map_data.map_height * map_data.map_width))
        {
            if(map_data.map[index] <= 0)//we found a wall
                break;

            //we at at a valid position so we check it there is a node here
            if(nodes.find(index) != std::end(nodes))//there is, so we add it as a neighbour
            {
                nodes[index].cost_to_target = cost;
                break;
            }

            index += 1;
            ++cost;
        }
    }
    //</f> /Register target

    //</f> /Register start and target nodes

    //<f> Pathfind algorithm

    map_data.min_path_cost = -1;
    map_data.path_buffer.clear();

    while(search_data.to_visit.size() > 0 && !flags->quit && !flags->stop)//we have elements to check and we are not closing
    {
        // std::this_thread::sleep_for(std::chrono::milliseconds(500));
        while(flags->pause_resume)
            std::this_thread::sleep_for(5ms);

        //In each step we analyze each cell as store its valid neighbors
        auto current_node {*search_data.to_visit.begin()};
        //remove node from set
        search_data.to_visit.erase(search_data.to_visit.begin());

        //mark node as closed
        search_data.node_status[current_node.index] = CLOSED_NODE;
        //mark node as visited (for rendering)
        map_renderer->SetVisited(current_node.index);

        //<f> Found Target
        //we found the destination
        if(nodes[current_node.index].cost_to_target > -1)
        {
            //we store the target node first as the first node
            map_data.path_buffer.push_back( search_data.target_index );//store target node

            if(nodes[current_node.index].cost_to_target > 0)//only store if target is not this node
                map_data.path_buffer.push_back( current_node.index );//store this neighbouring node

            int parent_index = search_data.parents[current_node.index];//get current node parent from paretns map
            map_data.min_path_cost = search_data.g_costs[current_node.index];//get cost to get to target
            map_data.min_path_cost += nodes[current_node.index].cost_to_target;//ajust cost to account for last jump

            while(parent_index != search_data.start_index)//valid index
            {
                map_data.path_buffer.push_back( parent_index );//store parent node
                parent_index = search_data.parents[parent_index];//get next parent
            }

            // flags->running = false;
            // return std::to_string(map_data.min_path_cost);//found path
            break;
        }
        //</f> /Found Target

        //<f> Neighbours
        auto node_data {nodes[current_node.index]};
        std::queue< std::pair<int, float> > neighbours_queue;

        //top
        if(node_data.top_index > -1)//exists
            neighbours_queue.push({node_data.top_index, node_data.top_cost});
        //down
        if(node_data.bottom_index > -1)//exists
            neighbours_queue.push({node_data.bottom_index, node_data.bottom_cost});
        //left
        if(node_data.left_index > -1)//exists
            neighbours_queue.push({node_data.left_index, node_data.left_cost});
        //right
        if(node_data.right_index > -1)//exists
            neighbours_queue.push({node_data.right_index, node_data.right_cost});

        //<f> Process Neighbours
        while(neighbours_queue.size() > 0)
        {
            //get first
            auto neighbour_index{neighbours_queue.front().first};
            auto neighbour_cost{neighbours_queue.front().second};
            neighbours_queue.pop();//remove first

            if( IsClosed(search_data.node_status[neighbour_index]) )//node already processed in a previous step
                continue;

            auto neigbours_node{nodes[neighbour_index]};

            //calculate the g_cost to neighbours from current node
            float new_neighbor_g_cost = search_data.g_costs[current_node.index] + neighbour_cost;

            //if node was never registered or we now have a faster way to reach it
            if(!IsOpen(search_data.node_status[neighbour_index]) || new_neighbor_g_cost < GetMapValue<int, float>(search_data.g_costs, neighbour_index, map_data.map.size()))
            {
                //save cost and parent used
                search_data.g_costs[neighbour_index] = new_neighbor_g_cost;
                search_data.parents[neighbour_index] = current_node.index;

                //create new search node
                Node new_node{ neighbour_index, new_neighbor_g_cost,
                    search_data.Heuristic(neigbours_node.node_x - benchmark.target_x, neigbours_node.node_y - benchmark.target_y, map_data.line_cost, map_data.diagonal_cost,
                                            benchmark.start_x - benchmark.target_x, benchmark.start_y - benchmark.target_y) };

                if(!IsOpen(search_data.node_status[neighbour_index]))//add new to open set
                {
                    search_data.to_visit.insert(new_node);
                    search_data.node_status[neighbour_index] = OPEN_NODE;
                    //UI update
                    map_renderer->SetToCheck(neighbour_index);
                }
                else//update node in the set
                {
                    //custom condition find
                    auto search_result = std::find_if(search_data.to_visit.begin(), search_data.to_visit.end(),
                    [&neighbour_index](const Node& stored_node)
                    {
                        return stored_node.index == neighbour_index;
                    });

                    if(search_result != search_data.to_visit.end())//found item
                    {
                        search_data.to_visit.erase(search_result);//remove old
                        search_data.to_visit.insert(new_node);//insert updated node
                    }
                }
            }
        }
        //</f> /Process Neighbours

        //</f> /Neighbours

        if(!flags->search_speed_is_fast)
        {
            ++thread_sleeps;
            std::this_thread::sleep_for(0.2ms);
        }
        ++operations;
    }
    //</f> /Pathfind algorithm

    //based on the exit mode we use different debug outputs
    if(!flags->quit)
    {
        std::string time_str{Clock::Instance()->StopAndReturnClock(clock_id)};
        MessageWriter::Instance()->WriteLineToConsole("Path took "+time_str+
        " ms to process(with 0.2ms * "+std::to_string(thread_sleeps)+" of thread sleep), "+std::to_string(operations)+
        " steps with result lenght of "+std::to_string(map_data.min_path_cost)+" units ("+ std::to_string(map_data.path_buffer.size()) +" total nodes)");

        // std::lock_guard<std::mutex> lock(*text_mutex);
        // result_label->Text("<b>Result length: </b>"+std::to_string(map_data.min_path_cost), {255,255,255,255});
        flags->running = false;
        return std::to_string(map_data.min_path_cost)+"|"+time_str;
    }
    else if(flags->stop)
    {
        MessageWriter::Instance()->WriteLineToConsole("Search stopped by user.");
    }
    else
    {
        //Cannot use MessageWriter::Instance()->WriteLineToConsole();
        std::cout << "Search stopped because program terminated." << "\n";
    }

    flags->running = false;
    return "";
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

void DrawLabyrinthPath(const std::vector<int>& path, std::map<int, LabyrinthNode>& nodes, MapRenderer* map_renderer, int map_width)
{
    // for( auto i{0}; i < path.size()-1; ++i)
    // {
    //     auto current_index{path[i]};
    //     auto current_node{nodes[current_index]};
    //     auto next_index{path[i + 1]};
    //
    //     //paint right
    //     if(current_node.right_index == next_index)
    //         for(auto i{current_index}; i<=next_index; ++i)
    //         {
    //             map_renderer->(i);
    //         }
    //     //paint left
    //     if(current_node.left_index >= 0)
    //         for(auto i{current_index}; i>=next_index; --i)
    //         {
    //             map_renderer->(i);
    //         }
    //     //paint up
    //     if(current_node.top_index >= 0)
    //         for(auto i{current_index}; i>=next_index; i -= map_width)
    //         {
    //             map_renderer->(i);
    //         }
    //     //paint left
    //     if(current_node.bottom_index >= 0)
    //         for(auto i{current_index}; i<=next_index; i += map_width)
    //         {
    //             map_renderer->(i);
    //         }
    // }
}
