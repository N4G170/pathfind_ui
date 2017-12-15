#include <iostream>

#include "structs.hpp"
#include "pathfind.hpp"
#include "sdl_ui.hpp"
#include "map_ui.hpp"
#include "clock.hpp"
#include "message_writer.hpp"
#include "map_loader.hpp"

#include <vector>
#include <random>
//#include <utility>
#include <string>

#include <thread>
//#include <functional>
#include <chrono>

#include "high_level_search.hpp"

/**
* \brief Generates a random number between min and max, includes both limits
*/
int RandomGenerator(int min = 0, int max = 4)
{
    std::random_device seed;
    std::default_random_engine engine( seed() );
    std::uniform_int_distribution<int> dist(min, max);

    return dist(engine);
}

/**
* \brief Loads the data of the requested map into the MapData reference that will be used to call the pathfind algorithm
*/
void RequestMap(int path_id, std::vector<std::string>& maps, std::string& previous_map_name, MapData& data, int map_id = 0)
{
    switch (path_id)
    {
    case 3://maps stored in files in the maps folder
        {
            if(previous_map_name != maps[map_id])//load new map
            {
                previous_map_name = data.name = maps[map_id];
                LoadMap(maps[map_id], data);
            }
            break;
        }

        case 1://Paradox example 1 from kattis
        {
            previous_map_name = "Paradox1";
            data.search_type = SEARCH_TYPE::QUARTILE;
            data.heuristic_type = HEURISTIC_TYPE::MANHATTAN;
            data.map = {1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1};
            data.map.shrink_to_fit();
            data.path_buffer.resize(12);//12 - value from example

            data.map_width = 4;
            data.map_height = 3;
            data.benchmarks.clear();
            //benchmark values start_x, start_y, target_x, target_y, expected length
            data.benchmarks.push_back(MapBenchmark{0, 0, 1, 2, 3 });//expected result from kattis
            data.selected_bechmark_index = 0;
            break;
        }

        case 2://Paradox example 2 from kattis
        {
            previous_map_name = "Paradox2";

            data.search_type = SEARCH_TYPE::QUARTILE;
            data.heuristic_type = HEURISTIC_TYPE::MANHATTAN;
            data.map = {0, 0, 1, 0, 1, 1, 1, 0, 1};
            data.map.shrink_to_fit();
            data.path_buffer.resize(7);//7 - value from example

            data.map_width = 3;
            data.map_height = 3;
            data.benchmarks.clear();
            //benchmark values start_x, start_y, target_x, target_y, expected length
            data.benchmarks.push_back(MapBenchmark{2, 0, 0, 2, -1 });//expected result from kattis
            data.selected_bechmark_index = 0;

            break;
        }

        case 0://Generates a random gibberish map (was used to debug the UI). The result cannot be tested for accuracy
        {
            previous_map_name = "Random1";

            data.search_type = SEARCH_TYPE::OCTILE_NOT_CORNER;
            data.heuristic_type = HEURISTIC_TYPE::OCTILE;

            int width{ 200 };
            int height{ 150 };
            data.map.resize(width*height);
            data.path_buffer.clear();

            for(unsigned int i = 0; i < data.map.size(); i++)
            {
                if(RandomGenerator() == 0)
                    data.map[i] = 0;
                else
                    data.map[i] = 1;
            }

            //generate start and target
            int s_x, s_y;
            do
            {
                s_x = RandomGenerator(0,width - 1);
                s_y = RandomGenerator(0,height - 1);

            } while (data.map[s_y*width+s_x] == 0);

            int t_x, t_y;
            do
            {
                t_x = RandomGenerator(0,width - 1);
                t_y = RandomGenerator(0,height - 1);

            } while (data.map[t_y*width+t_x] == 0 || (s_x == t_x && s_y == t_y));

            data.map.shrink_to_fit();

            data.map_width = width;
            data.map_height = height;
            data.benchmarks.clear();
            data.benchmarks.push_back(MapBenchmark{s_x, s_y, t_x, t_y, -1 });
            data.selected_bechmark_index = 0;

            break;
        }
    }
}

/**
* \brief Initialize the std::map with the Text objects that own all rendered text
*/
void  InitMenu(MainPointers& main_pointers, std::map<std::string, Text>& menu_text_objects)
{
    int y{ 290 };
    //prepare menu
    menu_text_objects["fps"] = Text(main_pointers, 1170, 10, "0");
    menu_text_objects["title"] = Text(main_pointers, 1050, 50, "Pathfinder Program");

    menu_text_objects["p_problems"] = Text(main_pointers, 1050, 100, "Paradox Examples");
    menu_text_objects["p_1"] = Text(main_pointers, 1050, 150, "-> Example 1 (press '1' to run)");
    menu_text_objects["p_2"] = Text(main_pointers, 1050, 175, "-> Example 2 (press '2' to run)");
    menu_text_objects["p_note"] = Text(main_pointers, 1050, 220, "Examples use 4 directions A* algorithm");

    menu_text_objects["line_1"] = Text(main_pointers, 1050, 240, "------------------------------------");

    menu_text_objects["maps"] = Text(main_pointers, 1050, y, "Extra maps (8 directions no corner)"); y += 25;
    menu_text_objects["map_name"] = Text(main_pointers, 1050, y, "<-|    |->"); y += 25;
    menu_text_objects["m_note"] = Text(main_pointers, 1050, y, "L/R Arrows to change map"); y += 25;

    menu_text_objects["m_benchmark"] = Text(main_pointers, 1050, y, "<-|    |-> U/D Arrows change benchmark"); y += 25;
    menu_text_objects["m_note_2"] = Text(main_pointers, 1050, y, "'R' to run selected map benchmark"); y += 45;

    menu_text_objects["warning"] = Text(main_pointers, 1050, y, "WARNING-EXPERIMENTAL HIERARCHICAL MAP"); y += 30;
    menu_text_objects["line_2"] = Text(main_pointers, 1050, y, "------------------------------------"); y += 25;

    menu_text_objects["toggle_grid"] = Text(main_pointers, 1050, y, "'G' - Show Grid"); y += 25;
    menu_text_objects["toggle_map"] = Text(main_pointers, 1050, y, "'H' - Hide Map"); y += 25;
    menu_text_objects["toggle_path"] = Text(main_pointers, 1050, y, "'J' - Hide Path"); y += 30;

    menu_text_objects["toggle_pause"] = Text(main_pointers, 1050, y, "'P' - Pause Search"); y += 25;
    menu_text_objects["break"] = Text(main_pointers, 1050, y, "'B' - Stop Search"); y += 25;
    menu_text_objects["speed"] = Text(main_pointers, 1050, y, "'F' - Set search speed Fast"); y += 25;

    menu_text_objects["line_3"] = Text(main_pointers, 1050, y, "------------------------------------"); y += 25;

    menu_text_objects["current_map"] = Text(main_pointers, 1050, y, "Current Map: "); y += 25;
    menu_text_objects["expected"] = Text(main_pointers, 1050, y, "Expected length: "); y += 25;
    menu_text_objects["result"] = Text(main_pointers, 1050, y, "Result length: ");

    y += 50;
    menu_text_objects["l_to_check"] = Text(main_pointers, 1100, y, "Node to check "); y += 25;
    menu_text_objects["l_checked"] = Text(main_pointers, 1100, y, "Checked node"); y += 25;
    menu_text_objects["l_short_path"] = Text(main_pointers, 1100, y, "Node in short path"); y += 25;

    menu_text_objects["l_start"] = Text(main_pointers, 1100, y, "Start node"); y += 25;
    menu_text_objects["l_target"] = Text(main_pointers, 1100, y, "Target node"); y += 25;

    menu_text_objects["l_path"] = Text(main_pointers, 1100, y, "Passable node"); y += 25;
    menu_text_objects["l_wall"] = Text(main_pointers, 1100, y, "Impassable node ");

}

/**
* \brief Renders all the Text objects and the nodes color legend
*/
void RenderMenu(MainPointers& main_pointers, std::map<std::string, Text>& menu_text_objects, GridImagePointers& image_pointers, const bool& show_warning)
{
    menu_text_objects["title"].Render(main_pointers.screen_renderer);
    menu_text_objects["p_problems"].Render(main_pointers.screen_renderer);
    menu_text_objects["p_1"].Render(main_pointers.screen_renderer);
    menu_text_objects["p_2"].Render(main_pointers.screen_renderer);
    menu_text_objects["p_note"].Render(main_pointers.screen_renderer);
    menu_text_objects["line_1"].Render(main_pointers.screen_renderer);
    menu_text_objects["maps"].Render(main_pointers.screen_renderer);
    menu_text_objects["map_name"].Render(main_pointers.screen_renderer);
    menu_text_objects["m_note"].Render(main_pointers.screen_renderer);
    menu_text_objects["m_benchmark"].Render(main_pointers.screen_renderer);
    menu_text_objects["m_note_2"].Render(main_pointers.screen_renderer);
    menu_text_objects["line_2"].Render(main_pointers.screen_renderer);
    menu_text_objects["toggle_map"].Render(main_pointers.screen_renderer);
    menu_text_objects["toggle_grid"].Render(main_pointers.screen_renderer);
    menu_text_objects["toggle_path"].Render(main_pointers.screen_renderer);
    menu_text_objects["toggle_pause"].Render(main_pointers.screen_renderer);
    menu_text_objects["break"].Render(main_pointers.screen_renderer);
    menu_text_objects["speed"].Render(main_pointers.screen_renderer);
    menu_text_objects["line_3"].Render(main_pointers.screen_renderer);
    menu_text_objects["current_map"].Render(main_pointers.screen_renderer);
    menu_text_objects["expected"].Render(main_pointers.screen_renderer);
    menu_text_objects["result"].Render(main_pointers.screen_renderer);
    menu_text_objects["fps"].Render(main_pointers.screen_renderer);

    //render legend text
    menu_text_objects["l_to_check"].Render(main_pointers.screen_renderer);
    menu_text_objects["l_checked"].Render(main_pointers.screen_renderer);
    menu_text_objects["l_short_path"].Render(main_pointers.screen_renderer);

    menu_text_objects["l_start"].Render(main_pointers.screen_renderer);
    menu_text_objects["l_target"].Render(main_pointers.screen_renderer);

    menu_text_objects["l_path"].Render(main_pointers.screen_renderer);
    menu_text_objects["l_wall"].Render(main_pointers.screen_renderer);

    //WARNING
    if(show_warning)
        menu_text_objects["warning"].Render(main_pointers.screen_renderer);

    //render legend images
    SDL_Rect destination_rect;
    destination_rect.w = 25;
    destination_rect.h = 25;
    destination_rect.x = 1070;
    destination_rect.y = 765;

    //render image
    SDL_RenderCopy( main_pointers.screen_renderer.get(), image_pointers.to_check.get(), nullptr, &destination_rect ); destination_rect.y += 25;
    SDL_RenderCopy( main_pointers.screen_renderer.get(), image_pointers.checked.get(), nullptr, &destination_rect ); destination_rect.y += 25;
    SDL_RenderCopy( main_pointers.screen_renderer.get(), image_pointers.short_path.get(), nullptr, &destination_rect ); destination_rect.y += 25;

    SDL_RenderCopy( main_pointers.screen_renderer.get(), image_pointers.start.get(), nullptr, &destination_rect ); destination_rect.y += 25;
    SDL_RenderCopy( main_pointers.screen_renderer.get(), image_pointers.target.get(), nullptr, &destination_rect ); destination_rect.y += 25;

    SDL_RenderCopy( main_pointers.screen_renderer.get(), image_pointers.path.get(), nullptr, &destination_rect ); destination_rect.y += 25;
    SDL_RenderCopy( main_pointers.screen_renderer.get(), image_pointers.wall.get(), nullptr, &destination_rect ); destination_rect.y += 25;

}


int main(int argc, char* argv[])
{
    MainPointers main_pointers;
    GridImagePointers image_pointers;
    std::map< std::string, Text > menu_text_objects;

    //start SDL
    if(!InitSDL(main_pointers))
    {
        //terminate any sdl subsystem that was initialized
        TerminateSDL(main_pointers);

        std::cout << "InitSDL ERROR "<<SDL_INIT_FAIL << "\n";

        exit(SDL_INIT_FAIL);
    }
    //sdl started ok

    if(!LoadGridCells(image_pointers, main_pointers))
    {
        //terminate any sdl subsystem that was initialized
        TerminateSDL(main_pointers);

        std::cout << "LoadGridCells ERROR "<<CELL_IMG_FAIL << "\n";

        exit(CELL_IMG_FAIL);
    }

    MainControlFlags flags;
    flags.quit = false;
    flags.pause = false;
    flags.stop = false;
    flags.fast = false;
    //thread that will run the search algorithm in parallel with the render thread(the main thread)
    std::thread pathfind_thread;

    //Create Event handler
    SDL_Event event;

    //grid related vars
    MapData map_data;
    std::string previous_map_name{""};

    //UI related vars
    flags.new_map = true;
    std::vector<DrawData> grid_ui;
    int cell_size{ 20 };
    int grid_size{ 1024 };
    bool show_grid{ false };
    bool show_map{ true };
    bool show_path{ true };
    bool show_warning{ true };

    std::vector<std::string> map_list;
    LoadMapList(map_list);//loads the list of available maps from the map_list file

    SDL_SetRenderDrawColor( main_pointers.screen_renderer.get(), 0xFF, 0xFF, 0xFF, 0xFF );
    SDL_Rect destination_rect;
    destination_rect.x = 1025;
    destination_rect.y = 0;
    destination_rect.w = 1050;

    int map_index{ 0 };
    int prev_map_index{ -1 };
    int benchmark_index{ 0 };
    int previous_bechmark_index{ -1 };

    //update show_warning to be in sync with the first map
    if(map_list[map_index].rfind("_high") != std::string::npos)//found word
        show_warning = true;
    else
        show_warning = false;

    //start Text objects
    InitMenu(main_pointers, menu_text_objects);

    //cap frame rate to 60 fps
    float frame_cap{1.f / 60 * 1000};
    float fps{0};

    //main loop
    while(!flags.quit)
    {
        //starts frame time counter
        auto start_time(std::chrono::high_resolution_clock::now());

        //Handle events on queue
        while( SDL_PollEvent( &event ) != 0 )
        {
            //User requests quit
            if( event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
                flags.quit = true;
            else if(event.type == SDL_KEYDOWN)
            {
                switch(event.key.keysym.sym)
                {
                    //changes the map, from the loaded list, to be run
                    case SDLK_LEFT: map_index = std::max(0, map_index-1); benchmark_index = 0;
                        if(map_list[map_index].rfind("_high") != std::string::npos)//found word
                            show_warning = true;
                        else
                            show_warning = false;
                    break;

                    case SDLK_RIGHT: map_index = std::min((int)(map_list.size()-1), map_index+1); benchmark_index = 0;
                        if(map_list[map_index].rfind("_high") != std::string::npos)//found word
                            show_warning = true;
                        else
                            show_warning = false;
                    break;

                    //changes the benchmark to be run in the current selected map
                    case SDLK_DOWN: benchmark_index = std::max(0, benchmark_index-1); break;
                    case SDLK_UP: benchmark_index = std::min((int)(max_benchmarks-1), benchmark_index+1); break;

                    //changes the search speed of the algorithm (if true the search thread does not sleep after every step)
                    case SDLK_f: flags.fast = !flags.fast;
                                if(flags.fast)
                                    menu_text_objects["speed"].SetString("'F' - Set search speed Normal");
                                else
                                    menu_text_objects["speed"].SetString("'F' - Set search speed Fast");
                    break;

                    //shows/hides a grig that shows the delimitations of each cell (off by default as it is very hard on the eyes on big maps)
                    case SDLK_g: show_grid = !show_grid;
                                if(show_grid)
                                    menu_text_objects["toggle_grid"].SetString("'G' - Hide Grid");
                                else
                                    menu_text_objects["toggle_grid"].SetString("'G' - Show Grid");
                    break;

                    //shows/hides the maps, just for the fun of watching the algorithm run on a blank area (I like doing this with the maze)
                    case SDLK_h: show_map = !show_map;
                                if(show_map)
                                    menu_text_objects["toggle_map"].SetString("'H' - Hide Map");
                                else
                                    menu_text_objects["toggle_map"].SetString("'H' - Show Map");
                    break;

                    //shows/hides the nodes searched and the final path(if found). I used it to debug a UI bug and kept it
                    case SDLK_j: show_path = !show_path;
                                if(show_path)
                                    menu_text_objects["toggle_path"].SetString("'J' - Hide Path");
                                else
                                    menu_text_objects["toggle_path"].SetString("'J' - Show Path");
                    break;

                    //stops/resumes the algorithm
                    case SDLK_p: flags.pause = !flags.pause;
                                if(flags.pause)
                                    menu_text_objects["toggle_pause"].SetString("'P' - Pause Search");
                                else
                                    menu_text_objects["toggle_pause"].SetString("'P' - Resume Search");
                    break;

                    //breaks the current algorithm allowing you to run a new test, as each run blocks the start of a new one(there is only one map draw area)
                    case SDLK_b: flags.stop = !flags.stop; break;
                }

                if(flags.new_map)//the last called search ended (on its own or we stopped it)
                {
                    bool map_set = false;

                    switch(event.key.keysym.sym)
                    {
                        case SDLK_1: case SDLK_KP_1://Paradox example 1
                        {
                            RequestMap(1, map_list, previous_map_name, map_data);
                            map_set = true;
                            flags.new_map = false;
                            break;
                        }
                        case SDLK_2: case SDLK_KP_2://Paradox example 2
                        {
                            RequestMap(2, map_list, previous_map_name, map_data);
                            map_set = true;
                            flags.new_map = false;
                            break;
                        }
                        case SDLK_KP_0: case SDLK_0://random gibberish map
                        {
                            RequestMap(0, map_list, previous_map_name, map_data);
                            map_set = true;
                            flags.new_map = false;
                            break;
                        }

                        case SDLK_r://starts a benchmark from the loaded list
                        {
                            RequestMap(3, map_list, previous_map_name, map_data, map_index);
                            map_data.selected_bechmark_index = benchmark_index;
                            map_set = true;
                            flags.new_map = false;
                            break;
                        }
                        break;


                    }//switch

                    if(map_set)//we want to run a new benchmark
                    {
                        //reset pause flag
                        flags.pause = false;
                        flags.stop = false;

                        //configure cell size
                        cell_size = std::min(40, std::max(1, grid_size / std::max(map_data.map_width, map_data.map_height)));
                        MessageWriter::Instance()->WriteLineToConsole("Map " + map_data.name + " expecting: " +
                                std::to_string(map_data.benchmarks[map_data.selected_bechmark_index].expected_min_path_cost));

                        //update some UI text to refer to our current benchmark
                        menu_text_objects["current_map"].SetString("Current Map: "+map_data.name);
                        menu_text_objects["expected"].SetString("Expected length:" + std::to_string(map_data.benchmarks[map_data.selected_bechmark_index].expected_min_path_cost));
                        menu_text_objects["result"].SetString("Result length: ");

                        //create the map texture only once
                        CreateMapTexture(main_pointers, image_pointers, map_data.map.data(), map_data.map_width, map_data.map_height, cell_size);
                        destination_rect.x = 5; destination_rect.y = 5;
                        destination_rect.w = cell_size * map_data.map_width; destination_rect.h = cell_size * map_data.map_height;

                        //set movement costs (should be moved to map data load as weach map could have different costs)
                        map_data.line_cost = 1;
                        map_data.diagonal_cost = std::sqrt(2);


                        if(map_data.search_type == SEARCH_TYPE::OCTILE_HIGH_8)
                        {
                            //start search thread and sends it all needed data
                            pathfind_thread = std::thread{ HighLevelSearch, std::ref(map_data), std::ref(grid_ui), std::ref(flags), std::ref(menu_text_objects), 64, 8 };

                            //there is a shared flag that will stop the thread so it will not keep running after we terminate the program
                            //also we need the flag as we cannot join the thread after we detach it
                            pathfind_thread.detach();
                        }
                        else if(map_data.search_type == SEARCH_TYPE::OCTILE_HIGH_64)
                        {
                            //start search thread and sends it all needed data
                            pathfind_thread = std::thread{ HighLevelSearch, std::ref(map_data), std::ref(grid_ui), std::ref(flags), std::ref(menu_text_objects), 8, 64 };

                            //there is a shared flag that will stop the thread so it will not keep running after we terminate the program
                            //also we need the flag as we cannot join the thread after we detach it
                            pathfind_thread.detach();
                        }
                        else
                        {
                            //start search thread and sends it all needed data
                            pathfind_thread = std::thread{ FindPath, std::ref(map_data), std::ref(grid_ui), std::ref(flags), std::ref(menu_text_objects) };

                            //there is a shared flag that will stop the thread so it will not keep running after we terminate the program
                            //also we need the flag as we cannot join the thread after we detach it
                            pathfind_thread.detach();
                        }
                    }
                }//if with new_map
            }//else if
        }

        //Render
        SDL_RenderClear( main_pointers.screen_renderer.get() );//clear screen

        //draw bg
        if(show_map)
            SDL_RenderCopy( main_pointers.screen_renderer.get(), image_pointers.map_texture.get(), NULL, &destination_rect );

        if(grid_ui.size() == (unsigned int)(map_data.map_width * map_data.map_height) && show_path)//the grid was updated to the new size
            DrawGrid(grid_ui, map_data.map_width, map_data.map_height, cell_size, main_pointers, image_pointers);

        if(show_path && flags.new_map && map_data.path_buffer.size() > 0)//has a path to draw
            DrawPath(map_data.path_buffer.data(), map_data.path_buffer.size(), map_data.map_width, map_data.map_height, cell_size, main_pointers, image_pointers);

        //draw grid
        if(show_grid)
            SDL_RenderCopy( main_pointers.screen_renderer.get(), image_pointers.grid_texture.get(), NULL, &destination_rect );

        //print menu
        menu_text_objects["fps"].SetString("FPS:" + std::to_string(fps));

        if(prev_map_index != map_index)//update current map Text (we need the if as Text updates are expensive)
        {
            prev_map_index = map_index;
            menu_text_objects["map_name"].SetString("<-|  "+map_list[map_index]+" "+std::to_string(map_index+1)+"/"+std::to_string(map_list.size())+"  |->");
        }

        if(previous_bechmark_index != benchmark_index)//update current benchmark Text (we need the if as Text updates are expensive)
        {
            previous_bechmark_index = benchmark_index;
            menu_text_objects["m_benchmark"].SetString("<-|  "+std::to_string(benchmark_index + 1)+"/"+std::to_string(max_benchmarks)+"  |->");
        }

        RenderMenu(main_pointers, menu_text_objects, image_pointers, show_warning);
        //end print menu

        //Update screen
        SDL_RenderPresent( main_pointers.screen_renderer.get() );

        //calculate frame time
        auto delta_time(std::chrono::high_resolution_clock::now() - start_time);
        //convert to float
        float frame_time = std::chrono::duration_cast< std::chrono::duration<float, std::milli> >(delta_time).count();

        //fps cap
        if(frame_time < frame_cap)//we wait
        {
            SDL_Delay(frame_cap - frame_time);
            frame_time = frame_cap;
        }
        frame_time /= 1000.f;
        fps = 1.f / frame_time;
        //change window title to show FPS
        SDL_SetWindowTitle(main_pointers.window.get(), (window_name +" - "+ std::to_string(fps)+" FPS").c_str() );
    }//while(!quit)

    // //as pathfind_thread is a default constructed thread (initially) we need to check if we can join it or not
    // //otherwise we get a std::system_error
    // if(pathfind_thread.joinable())//false if default constructed thread (will be false when we close the program without running a single search)
    //     pathfind_thread.join();

    return 0;
}
