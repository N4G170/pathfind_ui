#include <iostream>

#include "structs.hpp"

#include "sdl_init.hpp"
#include "map_renderer.hpp"
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


#include "menu.hpp"
#include "utils.hpp"

#include "sdl_gui_manager.hpp"
#include "sdl_gui_resource_manager.hpp"

int main(int argc, char* argv[])
{
    SDL_Window* window{nullptr};
    SDL_Renderer* renderer{nullptr};

    //init SDL subsystems
    SDLInitConfig sdl_config{};//load default values
    bool init_result = InitSDL(window, renderer, sdl_config);

    if(!init_result)//failed to initialize sdl subsystems
    {
        //terminate any initialized sdl subsystems
        TerminateSDL();
        DeleteSDLPointers(window, renderer);//they are deleted in a sepecific way
        window = nullptr;
        renderer = nullptr;

        return -2;
    }
    else
    {
        //control access to result label
        std::mutex text_mutex;

        ControlFlags control_flags{};

        sdl_gui::ResourceManager resource_manager{ renderer };
        sdl_gui::GuiManager gui_manager{renderer, &resource_manager};
        MapRenderer map_renderer{renderer, &control_flags};
        Menu menu{&gui_manager, &control_flags, &text_mutex, &map_renderer};

        //Create Event handler
        SDL_Event event;
        float frame_cap {1.f / 60 * 1000};
        float last_frame_time {0};

        float fixed_frame_time {0.03};
        float accumulated_time {0};

        while(!control_flags.quit)
        {
            auto start_time(std::chrono::high_resolution_clock::now());
            float fps{0};

            accumulated_time += last_frame_time;

            //Handle events on queue
            while( SDL_PollEvent( &event ) != 0 )
            {
                gui_manager.Input(event);
                //User requests quit
                if( event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
                    control_flags.quit = true;
            }

            //Fixed time step Logic
            while(accumulated_time >= fixed_frame_time)
            {
                // state_machine.Logic();
                accumulated_time -= fixed_frame_time;
            }

            //Logic
            gui_manager.Logic(last_frame_time);
            menu.Logic(last_frame_time);

            //Render
            //Clear screen
            SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0x00, 0x00 );
            SDL_RenderClear( renderer );

            {
                // std::lock_guard<std::mutex> lock(global_text_mutex);
                std::lock_guard<std::mutex> lock(text_mutex);
                // global_text_mutex.lock();
                gui_manager.Render(last_frame_time);
                // global_text_mutex.unlock();
            }

            if(control_flags.show_map)
                map_renderer.RenderMap();
            if(control_flags.show_path)
                map_renderer.RenderPath();
            if(control_flags.show_grid)
                map_renderer.RenderGrid();

            //Update screen
			SDL_RenderPresent( renderer );

            //Update frame timers
            auto delta_time(std::chrono::high_resolution_clock::now() - start_time);
            float frame_time = std::chrono::duration_cast< std::chrono::duration<float, std::milli> >(delta_time).count();

            //fps cap
            if(frame_time < frame_cap)
            {
                SDL_Delay(frame_cap - frame_time);
                frame_time = frame_cap;
            }
            frame_time /= 1000.f;
            fps = 1.f / frame_time;

            last_frame_time = frame_time;

            SDL_SetWindowTitle(window, ( sdl_config.window_name +" - "+ std::to_string(fps)+" FPS").c_str() );
        }//while(!quit)
    }

    return 0;
}
