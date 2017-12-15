#ifndef MAP_UI_H
#define MAP_UI_H

#include <vector>
#include "ui_structs.hpp"
#include "utils.hpp"

void RenderBlock(MainPointers& main_pointers, GridImagePointers& cells_pointers, SDL_Rect& destination_rect, DrawData& data, SDL_Surface* nodes_surface);

/**
* \brief Draws the checked nodes and the nodes to be checked, as well as the start and target nodes
*/
void DrawGrid(std::vector<DrawData>& grid, const int map_width, const int map_height, int cell_size, MainPointers& main_pointers, GridImagePointers& imgs_pointers)
{
    if(grid.size() <= 0)
        return;
    SDL_Surface* nodes_surface = SDL_CreateRGBSurface(0, map_width * cell_size, map_height * cell_size,32,0,0,0,0);
    SDL_Rect destination_rect;
    int offset = 5;

    for( int i = 0; i < map_height; i++)//each line
    {
        DrawData first_of_sequence = grid[i*map_width];//j is 0
        int sequence_length = 0;
        int start_x = 0;
        int start_y = i;

        for(int j = 0; j < map_width; j++)//each column
        {
            int index = i*map_width+j;
            if(grid[index] == first_of_sequence)//cell is to be painted with the same elements
            {
                sequence_length++;

                if(j == map_width - 1)//is the last element of the line
                {
                    destination_rect.x = cell_size * start_x;
                    destination_rect.y = cell_size * start_y;
                    destination_rect.w = cell_size * sequence_length;
                    destination_rect.h = cell_size;

                    RenderBlock(main_pointers, imgs_pointers, destination_rect, first_of_sequence, nodes_surface);
                }
            }
            else//break sequence
            {
                destination_rect.x = cell_size * start_x;
                destination_rect.y = cell_size * start_y;
                destination_rect.w = cell_size * sequence_length;
                destination_rect.h = cell_size;

                RenderBlock(main_pointers, imgs_pointers, destination_rect, first_of_sequence, nodes_surface);

                //start new block
                first_of_sequence = grid[index];
                sequence_length = 1;//we already have a node
                start_x = j;

                if(j == map_width - 1)//is the last element of the line
                {
                    destination_rect.x = cell_size * start_x;
                    destination_rect.y = cell_size * start_y;
                    destination_rect.w = cell_size * sequence_length;
                    destination_rect.h = cell_size;

                    RenderBlock(main_pointers, imgs_pointers, destination_rect, first_of_sequence, nodes_surface);
                }
            }
        }//for width
    }//for height

    destination_rect.x = offset;
    destination_rect.y = offset;
    destination_rect.w = cell_size * map_width;
    destination_rect.h = cell_size * map_height;
    //render image
    SDL_SetColorKey( nodes_surface, SDL_TRUE, SDL_MapRGB( nodes_surface->format, 0x00, 0x00, 0x00 ) );//remove the brack from the unused area
    imgs_pointers.nodes_texture.reset( SDL_CreateTextureFromSurface(main_pointers.screen_renderer.get(), nodes_surface) );
    SDL_RenderCopy( main_pointers.screen_renderer.get(), imgs_pointers.nodes_texture.get(), nullptr, &destination_rect );
    SDL_FreeSurface(nodes_surface);
}

/**
* \brief Draw the shortest path found by the serach algorithm
*/
void DrawPath(int* path, int node_count, const int map_width, const int map_height, int cell_size, MainPointers& main_pointers, GridImagePointers& imgs_pointers)
{
    SDL_Surface* path_surface = SDL_CreateRGBSurface(0, map_width * cell_size, map_height * cell_size,32,0,0,0,0);//create empty surface
    int offset = 5;

    SDL_Rect destination_rect;
    for(int i = 1; i < node_count; i++)//we jump the first node as it is the target, so we do not render over it
    {
        destination_rect.x = cell_size * GetCoordinateX(path[i], map_width);
        destination_rect.y = cell_size * GetCoordinateY(path[i], map_width);
        destination_rect.w = cell_size;
        destination_rect.h = cell_size;

        SDL_BlitScaled(imgs_pointers.short_path_surface.get(), nullptr, path_surface, &destination_rect);
    }

    destination_rect.x = offset;
    destination_rect.y = offset;
    destination_rect.w = cell_size * map_width;
    destination_rect.h = cell_size * map_height;

    //render image
    SDL_SetColorKey( path_surface, SDL_TRUE, SDL_MapRGB( path_surface->format, 0x00, 0x00, 0x00 ) );//remove the brack from the unused area
    imgs_pointers.path_texture.reset( SDL_CreateTextureFromSurface(main_pointers.screen_renderer.get(), path_surface) );
    SDL_RenderCopy( main_pointers.screen_renderer.get(), imgs_pointers.path_texture.get(), nullptr, &destination_rect );
    SDL_FreeSurface(path_surface);
}

/**
* \brief Blits a continuous line of nodes of the same type on a surface that will be used to render the nodes
* \brief This method avoids the rendering of individual nodes, its some sort of batch rendering
*/
void RenderBlock(MainPointers& main_pointers, GridImagePointers& img_pointers, SDL_Rect& destination_rect, DrawData& data, SDL_Surface* nodes_surface)
{
    if(data.visited)
        SDL_BlitScaled(img_pointers.checked_surface.get(), nullptr, nodes_surface, &destination_rect);
    else if(data.to_check)
        SDL_BlitScaled(img_pointers.to_check_surface.get(), nullptr, nodes_surface, &destination_rect);

    if(data.start_or_target == 1)//startshort_path
        SDL_BlitScaled(img_pointers.start_surface.get(), nullptr, nodes_surface, &destination_rect);
    else if(data.start_or_target == 2)//target
        SDL_BlitScaled(img_pointers.target_surface.get(), nullptr, nodes_surface, &destination_rect);

}

/**
* \brief Creates a texture with the map walls and passable areas. Only one is created when the serach algorithm is called.
*/
void CreateMapTexture(MainPointers& main_pointers, GridImagePointers& image_pointers, unsigned char* map, const int map_width, const int map_height, int cell_size)
{
    SDL_Surface* map_surface = SDL_CreateRGBSurface(0, map_width * cell_size, map_height * cell_size,32,0,0,0,0);//create empty surface
    SDL_Surface* grid_surface = SDL_CreateRGBSurface(0, map_width * cell_size, map_height * cell_size,32,0,0,0,0);//create empty surface

    SDL_Rect destination_rect;

    for(int i=0; i < map_height; i++)
    {
        for(int j=0; j<map_width; j++)
        {
            destination_rect.x = j * cell_size;
            destination_rect.y = i * cell_size;
            destination_rect.w = cell_size;
            destination_rect.h = cell_size;

            if(map[i * map_width + j] == 1)
                SDL_BlitScaled(image_pointers.path_surface.get(), nullptr, map_surface, &destination_rect);
            else
                SDL_BlitScaled(image_pointers.wall_surface.get(), nullptr, map_surface, &destination_rect);

            SDL_BlitScaled(image_pointers.frame_surface.get(), nullptr, grid_surface, &destination_rect);
        }
    }

    image_pointers.map_texture.reset( SDL_CreateTextureFromSurface( main_pointers.screen_renderer.get(), map_surface ) );
    //remove inner white from frame
    SDL_SetColorKey( grid_surface, SDL_TRUE, SDL_MapRGB( grid_surface->format, 0xFF, 0xFF, 0xFF ) );
    image_pointers.grid_texture.reset( SDL_CreateTextureFromSurface( main_pointers.screen_renderer.get(), grid_surface ) );

    //release tmp surfaces
    SDL_FreeSurface(map_surface);
    SDL_FreeSurface(grid_surface);
}

#endif //MAP_UI_H
