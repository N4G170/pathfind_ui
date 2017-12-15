#ifndef RSR_BASED_H
#define RSR_BASED_H
/**
//
//  This file is experimental. It only works with maps from the 8room maps group and the type (in file) must be "octile_high"
//
**/
#include <algorithm>
#include <utility>

#include "structs.hpp"
#include "message_writer.hpp"
#include "clock.hpp"
#include "heuristics.hpp"
#include "finder.hpp"
#include "text.hpp"
#include "utils.hpp"

struct HighLevelNode
{
    int index = -1;

    int left_index = -1;//high level index for the node to the left
    int left_exit_index = -1;//left exit from this node

    int right_index = -1;//high level index for the node to the right
    int right_exit_index = -1;//right exit from this node

    int up_index = -1;//high level index for the node above
    int up_exit_index = -1;//up exit from this node

    int down_index = -1;//high level index for the node bellow
    int down_exit_index = -1;//down exit from this node

    const void Debug()
    {
        std::cout << "SELF: "<< index << "\n";
        std::cout <<"UP: "<< up_index<<" - "<<up_exit_index << "\n";
        std::cout <<"DOWN: "<< down_index<<" - "<<down_exit_index << "\n";
        std::cout <<"LEFT: "<< left_index<<" - "<<left_exit_index << "\n";
        std::cout <<"RIGHT: "<< right_index<<" - "<<right_exit_index << "\n";
    }

    void Clear()
    {

    }
};
//I do not like global variables, but for this test will do, for me to keep everything related to the test on this file
//global non-const vars also go against the C++ Core Guidelines by Bjarne Stroustrup and Herb Sutter https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#Ri-global

std::vector<HighLevelNode> high_level_grid(0);
std::string g_previous_map_name{""};


//functions declarations (implemented in this file)
void CreateHighLevelGraph(MapData& map_data, int grid_width, int node_size);
bool HighLevelFinderStep(SearchData& search_data, std::pair< std::vector< int >, bool >& path, const int& grid_width, const int& start_target_delta_x, const int& start_target_delta_y,  const int& target_node_x, const int& target_node_y);
std::queue< int > GetHighLevelNeighbors(const int& current_index);
std::pair< std::vector< int >, bool > FindHighLevelPath(const int& start_index, const int& target_index, const int& grid_width, const int& node_size);
int GetExitIndex(const HighLevelNode& node, const int& neigbor);
std::map<int, std::vector< int > > FindHighLevelPath(const int& start_index, const int& target_index);

/**
* \brief Searches for the shortest path on a map using an high level abstraction
*/
void HighLevelSearch( MapData& map_data, std::vector<DrawData>& draw_data_grid, MainControlFlags& flags, std::map<std::string, Text>& menu_texts, int grid_width, int node_size)
{
    if(high_level_grid.size() == 0 || g_previous_map_name.compare(map_data.name) != 0)//high level grid was not created
    {
        g_previous_map_name = map_data.name;
        CreateHighLevelGraph(map_data, grid_width, node_size);
    }

    draw_data_grid.clear();

    MapBenchmark benchmark = map_data.benchmarks[map_data.selected_bechmark_index];
    int start_index = GetIndexFromCoordinate (benchmark.start_x, benchmark.start_y, map_data.map_width);
    int target_index = GetIndexFromCoordinate (benchmark.target_x, benchmark.target_y, map_data.map_width);

    //High level pathfind. Get a path from the high level grid (returns low level indeces)
    std::pair< std::vector< int >, bool>  high_level_valid_path_result = FindHighLevelPath(start_index, target_index, grid_width, node_size);
    
    if(!high_level_valid_path_result.second)//failed to find path
    {
        MessageWriter::Instance()->WriteLineToConsole("No High Level path found.");

        menu_texts["result"].SetString("Result length: -1");

        flags.new_map = true;
        return;
    }

    //for clarity
    std::vector< int >  high_level_valid_path( std::move(high_level_valid_path_result.first));

    //if high_level_valid_path.size() == 0 the start and target are nside the same node

    //number of cells analyzed
    unsigned int operations = 0;
    unsigned int thread_sleeps = 0;

    //high_level_valid_path is filled with low level indeces but FindHighLevelPath does not know the original start and target indeces, so we need to insert them
    high_level_valid_path.insert(high_level_valid_path.begin(), start_index);
    high_level_valid_path.push_back(target_index);

    //the clock will not count the time processing in high level
    unsigned long clock_id = Clock::Instance()->StartClock();

// <f> Prepare Map Render Vector (tag from editor custom fold)
    draw_data_grid.resize(map_data.map_height * map_data.map_width);

    for(int i = 0; i < map_data.map_height * map_data.map_width; i++)
        draw_data_grid[i] = DrawData{false, false, 0};

    if(start_index < map_data.map_height * map_data.map_width)
        draw_data_grid[start_index].start_or_target = 1;
    if(target_index < map_data.map_height * map_data.map_width)
        draw_data_grid[target_index].start_or_target = 2;
// </f> (tag from editor custom fold)

    //start searching for final path
    //prepare algorithm helper struct
    SearchData search_data;
    //select heuristic
    search_data.Heuristic = Octile;

    //Start low level pathfind
    map_data.min_path_cost = -1;
    map_data.path_buffer.clear();

    for(unsigned int i = 0; i < high_level_valid_path.size() - 1; i++)
    {
        search_data.to_visit.clear();

        search_data.start_index = high_level_valid_path[i];
        search_data.target_index = high_level_valid_path[i+1];

        //for each new pair we create a new benchmark
        MapBenchmark local_benchmark;
        local_benchmark.start_x = GetCoordinateX(search_data.start_index, map_data.map_width);
        local_benchmark.start_y = GetCoordinateY(search_data.start_index, map_data.map_width);
        local_benchmark.target_x = GetCoordinateX(search_data.target_index, map_data.map_width);
        local_benchmark.target_y = GetCoordinateY(search_data.target_index, map_data.map_width);

        //create first node
        Node current_cell{search_data.start_index, 0,
            search_data.Heuristic(local_benchmark.start_x - local_benchmark.target_x, local_benchmark.start_y - local_benchmark.target_y,
                map_data.line_cost, map_data.diagonal_cost, local_benchmark.start_x - local_benchmark.target_x, local_benchmark.start_y - local_benchmark.target_y )};//base cost of start is 0

        search_data.to_visit.insert(current_cell);//inserts the first cell in the set so we can start searching
        search_data.parents[search_data.start_index] = -1;//the start cell has no parent

        while(flags.pause)
            std::this_thread::sleep_for(5ms);

        if(!flags.fast)
        {
            thread_sleeps++;
            std::this_thread::sleep_for(0.2ms);
        }

        if(flags.quit || flags.stop)
            break;

        //In each step we analyze each cell as store its valid neighbors
        while(!FinderStep(search_data, map_data, draw_data_grid, local_benchmark))
        {
            operations++;

            while(flags.pause)
                std::this_thread::sleep_for(5ms);

            if(!flags.fast)
            {
                thread_sleeps++;
                std::this_thread::sleep_for(0.2ms);
            }

            if(flags.quit || flags.stop)
                break;
        }
    }

    //based on the exit mode we use different debug outputs
    if(!flags.quit)
    {
        MessageWriter::Instance()->WriteLineToConsole("Path took "+Clock::Instance()->StopAndReturnClock(clock_id)+
        " ms to process(with 0.2ms * "+std::to_string(thread_sleeps)+" of thread sleep), "+std::to_string(operations)+
        " steps with result lenght of "+std::to_string(map_data.min_path_cost)+" units ("+ std::to_string(map_data.path_buffer.size()) +" total cells)");

        menu_texts["result"].SetString("Result length: "+std::to_string(map_data.min_path_cost));
    }
    else if(flags.stop)
    {
        MessageWriter::Instance()->WriteLineToConsole("Search stopped by user.");
    }
    else
    {
        //Cannot use MessageWriter::Instance()->WriteLineToConsole();
        std::cout << "Search stopped because program terminated." << "\n";
    }

    flags.new_map = true;
}

/**
* \brief Create the high level representation of the original map (rooms only)
*/
void CreateHighLevelGraph(MapData& map_data, int grid_width, int node_size)
{
    int grid_height = grid_width;//its a square map
    high_level_grid.resize(grid_width*grid_height);

    for(unsigned int i = 0; i < high_level_grid.size(); i++)
    {
        //create connections
        int node_x = GetCoordinateX( i, grid_width );
        int node_y = GetCoordinateY( i, grid_width );
        int origin_x = node_x * node_size;
        int origin_y = node_y * node_size;

        high_level_grid[i].index = i;

        for(int j = 0; j < node_size; j++)
        {
            int index = GetIndexFromCoordinate(origin_x + j, origin_y, map_data.map_width);
            //Calculate up link
            if(map_data.map[index] == 1 && node_y > 0)//valid position and has up
            {
                high_level_grid[i].up_index = i - grid_width;
                high_level_grid[i].up_exit_index = index;
            }

            index = GetIndexFromCoordinate(origin_x + j, origin_y + node_size, map_data.map_width);
            //Calculate down link
            if(map_data.map[index] == 1 && node_y < grid_width - 1)//valid position and has up
            {
                high_level_grid[i].down_index = i + grid_width;
                high_level_grid[i].down_exit_index = index;
            }

            index = GetIndexFromCoordinate(origin_x, origin_y + j, map_data.map_width);
            //Calculate left link
            if(map_data.map[index] == 1 && node_x > 0)//valid position and has up
            {
                high_level_grid[i].left_index = i - 1;
                high_level_grid[i].left_exit_index = index;
            }

            index = GetIndexFromCoordinate(origin_x + node_size, origin_y + j, map_data.map_width);
            //Calculate right link
            if(map_data.map[index] == 1 && node_x < grid_width - 1)//valid position and has up
            {
                high_level_grid[i].right_index = i + 1;
                high_level_grid[i].right_exit_index = index;
            }
        }
    }//for(unsigned int i = 0; i < high_level_grid.size(); i++)
}

std::pair< std::vector< int >, bool > FindHighLevelPath(const int& start_index, const int& target_index, const int& grid_width, const int& node_size)
{
    std::pair<std::vector< int >, bool> path;
    path.second = false;

    //we find in which node the start and the target are
    int high_level_start_index = GetIndexFromCoordinate( GetCoordinateX( start_index, 512 )/node_size, GetCoordinateY( start_index, 512 )/node_size, grid_width );
    int high_level_target_index = GetIndexFromCoordinate( GetCoordinateX( target_index, 512 )/node_size, GetCoordinateY( target_index, 512 )/node_size, grid_width );

    if(high_level_start_index == high_level_target_index)
    {
        path.second = true;
        return path;
    }

    int target_node_x = GetCoordinateX( high_level_target_index, grid_width );
    int target_node_y = GetCoordinateY( high_level_target_index, grid_width );
    int start_target_delta_x = GetCoordinateX( high_level_start_index, grid_width ) - target_node_x;
    int start_target_delta_y = GetCoordinateY( high_level_start_index, grid_width ) - target_node_y;

    SearchData search_data;
    search_data.Heuristic = Manhattan;
    search_data.start_index = high_level_start_index;
    search_data.target_index = high_level_target_index;

    //Get first node
    Node current_node{high_level_grid[high_level_start_index].index, 0,
        search_data.Heuristic(start_target_delta_x, start_target_delta_y, 1, 0, start_target_delta_x, start_target_delta_y)};

    search_data.to_visit.insert(current_node);

    while(search_data.to_visit.size() > 0)//we have elements to check
    {
        //In each step we analyze each cell as store its valid neighbors
        if(HighLevelFinderStep(search_data, path, grid_width, start_target_delta_x, start_target_delta_y, target_node_x, target_node_y))
            break;//found path
    }

    return path;
}

/**
* \brief Process a node and checks its neighbors storing them if valid
*/
bool HighLevelFinderStep(SearchData& search_data, std::pair<std::vector< int >, bool>& path, const int& grid_width, const int& start_target_delta_x, const int& start_target_delta_y,  const int& target_node_x, const int& target_node_y)
{
    //get a copy of the node with lowest cost
    Node current_node = *search_data.to_visit.begin();
    //remove the first cell as we are processing it
    search_data.to_visit.erase(search_data.to_visit.begin());

    search_data.node_status[current_node.index] = CLOSED_NODE;

    //we found the destination
    if(current_node.index == search_data.target_index)
    {
        //target is stored ouside
        //path.push_back( search_data.target_index );//store target index

        int parent_index = search_data.parents[current_node.index];//get current node parent from paretns map

        path.first.push_back( GetExitIndex(high_level_grid[current_node.index], parent_index));//store connection between current node and its parent

        while(parent_index != search_data.start_index)//valid index
        {
            path.first.push_back( GetExitIndex(high_level_grid[parent_index], search_data.parents[parent_index]));//store connection between current node and its parent
            parent_index = search_data.parents[parent_index];//get next parent
        }
        //start is stored outside
        //path.push_back( search_data.start_index );//store start index

        std::reverse(path.first.begin(), path.first.end());
        path.second = true;

        return true;//found path
    }

    //go search for neighbors
    std::queue< int > neighbors_queue = GetHighLevelNeighbors(current_node.index);

    while(neighbors_queue.size() > 0)
    {
        int neighbor_index = neighbors_queue.front();
        neighbors_queue.pop();//remove first

        if( IsClosed(search_data.node_status[neighbor_index]) )//node already processed in a previous step
            continue;

        int x = GetCoordinateX(neighbor_index, grid_width);
        int y = GetCoordinateY(neighbor_index, grid_width);

        //calculate the g_cost to neighbor from current node
        float new_neighbor_g_cost = search_data.g_costs[current_node.index] + 1;

        if(!IsOpen(search_data.node_status[neighbor_index]) || new_neighbor_g_cost < GetMapValue<int, float>(search_data.g_costs, neighbor_index, high_level_grid.size()))//new node or one with lower g_cost
        {
            search_data.g_costs[neighbor_index] = new_neighbor_g_cost;//set new cost
            search_data.parents[neighbor_index] = current_node.index;

            Node new_node{ neighbor_index, new_neighbor_g_cost,
                search_data.Heuristic(x - target_node_x, y - target_node_y, 1, 0,
                                        start_target_delta_x, start_target_delta_y) };

            if(!IsOpen(search_data.node_status[neighbor_index]))//add new to open set
            {
                search_data.to_visit.insert(new_node);
                search_data.node_status[neighbor_index] = OPEN_NODE;
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

    path.second = false;
    return false;
}

std::queue< int > GetHighLevelNeighbors(const int& current_index)
{
    std::queue< int >neighbors;

    //up
    if(high_level_grid[current_index].up_index > -1)//has row above
    {
        neighbors.push( high_level_grid[current_index].up_index );//move one up
    }
    //down
    if(high_level_grid[current_index].down_index > -1)//has row bellow
    {
        neighbors.push( high_level_grid[current_index].down_index );//move one down
    }
    //left
    if(high_level_grid[current_index].left_index > -1)//has left
    {
        neighbors.push( high_level_grid[current_index].left_index );//move one left
    }
    //right
    if(high_level_grid[current_index].right_index > -1)//has right
    {
        neighbors.push( high_level_grid[current_index].right_index );//move one right
    }

    return std::move(neighbors);
}

int GetExitIndex(const HighLevelNode& node, const int& neigbor)
{
    if(neigbor == node.up_index)
        return node.up_exit_index;
    if(neigbor == node.down_index)
        return node.down_exit_index;
    if(neigbor == node.left_index)
        return node.left_exit_index;
    if(neigbor == node.right_index)
        return node.right_exit_index;

    return -1;
}

#endif //RSR_BASED_H
