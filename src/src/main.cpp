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
// #include "labyrinth_search.hpp"
#include "menu.hpp"
#include "utils.hpp"

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

int main(int argc, char* argv[])
{
    MainPointers main_pointers;
    GridImagePointers image_pointers;
    std::map< std::string, Text > menu_text_objects;
    std::mutex text_mutex;

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
                            // HighLevelSearch( map_data, grid_ui, flags, menu_text_objects, 64, 8);
                            //start search thread and sends it all needed data
                            pathfind_thread = std::thread{ HighLevelSearch, std::ref(map_data), std::ref(grid_ui), std::ref(flags), std::ref(menu_text_objects), 64, 8, std::ref(text_mutex) };

                            //there is a shared flag that will stop the thread so it will not keep running after we terminate the program
                            //also we need the flag as we cannot join the thread after we detach it
                            pathfind_thread.detach();
                        }
                        else if(map_data.search_type == SEARCH_TYPE::OCTILE_HIGH_64)
                        {
                            // HighLevelSearch(map_data, grid_ui, flags, menu_text_objects, 8, 64);
                            //start search thread and sends it all needed data
                            pathfind_thread = std::thread{ HighLevelSearch, std::ref(map_data), std::ref(grid_ui), std::ref(flags), std::ref(menu_text_objects), 8, 64, std::ref(text_mutex) };

                            //there is a shared flag that will stop the thread so it will not keep running after we terminate the program
                            //also we need the flag as we cannot join the thread after we detach it
                            pathfind_thread.detach();
                        }
                        // else if(map_data.search_type == SEARCH_TYPE::MAZE)
                        // {
                        //     //start search thread and sends it all needed data
                        //     pathfind_thread = std::thread{ FindLabyrinthPath, std::ref(map_data), std::ref(grid_ui), std::ref(flags), std::ref(menu_text_objects), std::ref(text_mutex) };
                        //
                        //     //there is a shared flag that will stop the thread so it will not keep running after we terminate the program
                        //     //also we need the flag as we cannot join the thread after we detach it
                        //     pathfind_thread.detach();
                        // }
                        else
                        {
                            // FindPath(map_data, grid_ui, flags, menu_text_objects);
                            //start search thread and sends it all needed data
                            pathfind_thread = std::thread{ FindPath, std::ref(map_data), std::ref(grid_ui), std::ref(flags), std::ref(menu_text_objects), std::ref(text_mutex) };

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

        RenderMenu(main_pointers, menu_text_objects, image_pointers, show_warning, text_mutex);
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
