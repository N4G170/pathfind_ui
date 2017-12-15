#ifndef PATHFIND_H
#define PATHFIND_H

#include <vector>
#include <map>
#include <queue>
#include <set>
#include <cstdlib>
#include <iostream>
#include <utility>
#include <algorithm>
#include "structs.hpp"
#include "message_writer.hpp"
#include "clock.hpp"
#include "heuristics.hpp"
#include "finder.hpp"
#include "text.hpp"


using namespace std::chrono_literals;

//forward declaration (implemented in the end of this file)
std::function<float(const int&, const int&, const float&, const float&, const int&, const int&)> SelectHeuristic(HEURISTIC_TYPE type);

/**
* \brief Searches for the shortest path on a map for a given benchmark
*/
void FindPath(MapData& map_data, std::vector<DrawData>& draw_data_grid, MainControlFlags& flags, std::map<std::string, Text>& menu_texts)
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
    search_data.Heuristic = SelectHeuristic(map_data.heuristic_type);


// <f> Prepare Map Render Vector (tag from editor custom fold)
    draw_data_grid.resize(map_data.map_height * map_data.map_width);

    for(int i = 0; i < map_data.map_height * map_data.map_width; i++)
        draw_data_grid[i] = DrawData{false, false, 0};

    if(search_data.start_index < map_data.map_height * map_data.map_width)
        draw_data_grid[search_data.start_index].start_or_target = 1;
    if(search_data.target_index < map_data.map_height * map_data.map_width)
        draw_data_grid[search_data.target_index].start_or_target = 2;
// </f> (tag from editor custom fold)

    //Start path find
    //create first node
    Node current_cell{search_data.start_index, 0,
        search_data.Heuristic(benchmark.start_x - benchmark.target_x, benchmark.start_y - benchmark.target_y,
            map_data.line_cost, map_data.diagonal_cost, benchmark.start_x - benchmark.target_x, benchmark.start_y - benchmark.target_y )};//base cost of start is 0

    search_data.to_visit.insert(current_cell);//inserts the first cell in the set so we can start searching
    search_data.parents[search_data.start_index] = -1;//the start cell has no parent

    map_data.min_path_cost = -1;
    map_data.path_buffer.clear();

    while(search_data.to_visit.size() > 0 && !flags.quit && !flags.stop)//we have elements to check and we are not closing
    {
        while(flags.pause)
            std::this_thread::sleep_for(5ms);

        //In each step we analyze each cell as store its valid neighbors
        if(FinderStep(search_data, map_data, draw_data_grid, benchmark))
            break;//found path
        operations++;

        if(!flags.fast)
        {
            thread_sleeps++;
            std::this_thread::sleep_for(0.2ms);
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
* \brief Returns the std::function that matches the selected heuristic type
*/
std::function<float(const int&, const int&, const float&, const float&, const int&, const int&)> SelectHeuristic(HEURISTIC_TYPE type)
{
    switch(type)
    {
        case HEURISTIC_TYPE::MANHATTAN: return Manhattan;
        case HEURISTIC_TYPE::OCTILE: return Octile;
        default: return Manhattan;
    }
}

#endif //PATHFIND_H
