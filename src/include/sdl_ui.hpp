#ifndef SDL_UI_H
#define SDL_UI_H
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <memory>
#include <thread>
#include <mutex>
#include <future>

//#include "config.hpp"
#include "ui_structs.hpp"
#include "structs.hpp"

enum TextAlignment
{
    TOP_LEFT = 0,
    CENTER = 1,
    TOP_RIGHT = 2
};

/**
 * \brief Initializes ALL SDL subsystems that will be used
 * \details Initializes SDL, SDL_image, SDL_ttf, if it fails "kills" the application
 * \return Whether all subsystems initialized correctly or at least one failed
 */
bool InitSDL(MainPointers& main_pointers)
{
    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        std::cout<<"SDL could not initialize! SDL_Error: "<<SDL_GetError()<<std::endl;

        return false;
    }
    else
    {
        //Create window
        SDL_Window* window_ptr = SDL_CreateWindow( window_name.c_str(), window_position_x, window_position_y, window_width, window_height, window_flags );
        if( window_ptr == nullptr )
        {
            std::cout<<"Window could not be created! SDL_Error: "<<SDL_GetError()<<std::endl;

            return false;
        }
        else
        {
            main_pointers.window.reset(window_ptr);

            SDL_Renderer* screen_renderer_ptr = SDL_CreateRenderer( main_pointers.window.get(), -1, SDL_RENDERER_ACCELERATED );

            if(screen_renderer_ptr == nullptr)
            {
                std::cout<<"Main Renderer could not be created! SDL_Error: "<<SDL_GetError()<<std::endl;

                return false;
            }
            else
            {
                main_pointers.screen_renderer.reset(screen_renderer_ptr);

                //Initialize PNG loading
                int imgFlags = IMG_INIT_PNG;
                if( !( IMG_Init( imgFlags ) & imgFlags ) )
                {
                    std::cout<<"SDL_image could not initialize! SDL_image Error: "<<IMG_GetError()<<std::endl;

                    return false;
                }
                else
                {
                    //Initialize SDL_ttf
                    if( TTF_Init() == -1 )
                    {
                       std::cout<<"SDL_ttf could not initialize! SDL_ttf Error: "<< TTF_GetError()<<std::endl;
                       return false;
                    }
                    else
                    {
                        //load font BIG
                        TTF_Font* font = TTF_OpenFont( (path_font + font_name).c_str(), 15 );
                        main_pointers.main_font.reset(font);
                    }
                }
            }
        }

    }

    return true;
}

/**
 * \brief Terminates all SDL subsystems and clears "global" pointers
 * \details Tries to reset (clear) all "global" pointers and terminates TTF IMG and MAIN SDL subsystems
 */
void TerminateSDL(MainPointers& main_pointers)
{
    //clear unique_ptr before terminating sdl subsystems. reset(nullptr) will call the respective deleter
    main_pointers.window.reset(nullptr);
    main_pointers.main_font.reset(nullptr);

    //Quit SDL subsystems
    TTF_Quit();
    SDL_Quit();
}

bool LoadGridCells(GridImagePointers& pointers, MainPointers& main_pointers)
{
    SDL_Surface* wall = IMG_Load( (path_img+"wall.png").c_str() );
    SDL_Surface* path = IMG_Load( (path_img+"path.png").c_str() );

    SDL_Surface* start = IMG_Load( (path_img+"start.png").c_str() );
    SDL_Surface* target = IMG_Load( (path_img+"target.png").c_str() );

    SDL_Surface* to_check = IMG_Load( (path_img+"to_check.png").c_str() );
    SDL_Surface* checked = IMG_Load( (path_img+"checked.png").c_str() );
    SDL_Surface* short_path = IMG_Load( (path_img+"short_path.png").c_str() );

    SDL_Surface* frame = IMG_Load( (path_img+"frame.png").c_str() );

    if(wall == nullptr || path == nullptr || start == nullptr || target == nullptr ||
        to_check == nullptr || checked == nullptr || short_path == nullptr || frame == nullptr)
        return false;

    //save surfaces
    pointers.wall_surface.reset( wall );
    pointers.path_surface.reset( path );
    pointers.frame_surface.reset( frame );

    pointers.to_check_surface.reset( to_check );
    pointers.checked_surface.reset( checked );
    pointers.short_path_surface.reset( short_path );
    pointers.start_surface.reset( start );
    pointers.target_surface.reset( target );

    //create textures
    pointers.wall.reset(SDL_CreateTextureFromSurface( main_pointers.screen_renderer.get(), wall ));
    pointers.path.reset(SDL_CreateTextureFromSurface( main_pointers.screen_renderer.get(), path ));

    pointers.start.reset(SDL_CreateTextureFromSurface( main_pointers.screen_renderer.get(), start ));
    pointers.target.reset(SDL_CreateTextureFromSurface( main_pointers.screen_renderer.get(), target ));

    pointers.to_check.reset(SDL_CreateTextureFromSurface( main_pointers.screen_renderer.get(), to_check ));
    pointers.checked.reset(SDL_CreateTextureFromSurface( main_pointers.screen_renderer.get(), checked ));
    pointers.short_path.reset(SDL_CreateTextureFromSurface( main_pointers.screen_renderer.get(), short_path ));

    if(pointers.wall == nullptr || pointers.path == nullptr || pointers.start == nullptr || pointers.target == nullptr ||
        pointers.to_check == nullptr || pointers.checked == nullptr || pointers.short_path == nullptr)
        return false;

    //free surfaces
    /*SDL_FreeSurface(start);
    SDL_FreeSurface(target);

    SDL_FreeSurface(to_check);
    SDL_FreeSurface(checked);
    SDL_FreeSurface(short_path);*/
    return true;
}

#endif //SDL_UI_H
