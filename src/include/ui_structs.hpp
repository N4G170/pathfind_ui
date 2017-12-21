#ifndef UI_STRUCTS_H
#define UI_STRUCTS_H

#include <memory>
#include "SDL.h"

struct Deleters;

/**
* \brief Struct storing the rendering status for a node
*/
struct DrawData
{
    bool visited;
    bool to_check;
    int start_or_target;

    bool operator==(const DrawData& other)
    {
        return visited == other.visited && to_check == other.to_check
                && start_or_target == other.start_or_target;
    }
};

/**
* \brief Pointers to several SDL_surfaces and SDL_Textures, to be used by the rendering functions in order to avoid multiple memory loads of the same asset
*/
struct GridImagePointers
{
    std::unique_ptr<SDL_Surface, Deleters> wall_surface;
    std::unique_ptr<SDL_Surface, Deleters> path_surface;
    std::unique_ptr<SDL_Surface, Deleters> frame_surface;
    std::unique_ptr<SDL_Texture, Deleters> map_texture;
    std::unique_ptr<SDL_Texture, Deleters> grid_texture;
    std::unique_ptr<SDL_Texture, Deleters> nodes_texture;
    std::unique_ptr<SDL_Texture, Deleters> path_texture;

    std::unique_ptr<SDL_Texture, Deleters> wall;
    std::unique_ptr<SDL_Texture, Deleters> path;

    std::unique_ptr<SDL_Surface, Deleters> start_surface;
    std::unique_ptr<SDL_Surface, Deleters> target_surface;

    std::unique_ptr<SDL_Surface, Deleters> to_check_surface;
    std::unique_ptr<SDL_Surface, Deleters> checked_surface;
    std::unique_ptr<SDL_Surface, Deleters> short_path_surface;


    std::unique_ptr<SDL_Texture, Deleters> start;
    std::unique_ptr<SDL_Texture, Deleters> target;

    std::unique_ptr<SDL_Texture, Deleters> to_check;
    std::unique_ptr<SDL_Texture, Deleters> checked;
    std::unique_ptr<SDL_Texture, Deleters> short_path;
};

#endif //UI_STRUCTS_H
