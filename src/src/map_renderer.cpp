#include "map_renderer.hpp"
#include <utility>
#include <stdexcept>
#include "message_writer.hpp"

//<f> Constructors & operator=
MapRenderer::MapRenderer(SDL_Renderer* renderer, ControlFlags* control_flags): m_renderer{renderer}, m_control_flags{control_flags}
{
    LoadGridCells();
}

MapRenderer::~MapRenderer() noexcept
{

}

MapRenderer::MapRenderer(const MapRenderer& other)
{

}

MapRenderer::MapRenderer(MapRenderer&& other) noexcept
{

}

MapRenderer& MapRenderer::operator=(const MapRenderer& other)
{
    if(this != &other)//not same ref
    {
        auto tmp(other);
        *this = std::move(tmp);
    }

    return *this;
}

MapRenderer& MapRenderer::operator=(MapRenderer&& other) noexcept
{
    if(this != &other)//not same ref
    {
        //move here
    }
    return *this;
}
//</f> /Constructors & operator=

//<f> Getters/Setters
void MapRenderer::ResizeVector(int size)
{
    std::lock_guard<std::mutex> lock(render_mutex);

    m_grid_ui.resize(size);
    for(int i = 0; i < size; i++)
        m_grid_ui[i] = DrawData{false, false, 0};
}

void MapRenderer::SetStartNode(int index)
{
    std::lock_guard<std::mutex> lock(render_mutex);
    m_grid_ui[index].start_or_target = 1;
}

void MapRenderer::SetTargetNode(int index)
{
    std::lock_guard<std::mutex> lock(render_mutex);
    m_grid_ui[index].start_or_target = 2;
}

void MapRenderer::SetVisited(int index)
{
    std::lock_guard<std::mutex> lock(render_mutex);
    m_grid_ui[index].visited = true;
}

void MapRenderer::SetToCheck(int index)
{
    std::lock_guard<std::mutex> lock(render_mutex);
    m_grid_ui[index].to_check = true;
}
//</f> /Getters/Setters

//<f> Methods
void MapRenderer::SetMapData(MapData* data)
{
    std::lock_guard<std::mutex> lock(render_mutex);
    m_map_data = data;
}

void MapRenderer::CreateMap()
{
    if(m_map_data == nullptr)
        return;

    std::lock_guard<std::mutex> lock(render_mutex);

    //configure cell size
    m_cell_size = std::min(40, std::max(1, m_grid_size / std::max(m_map_data->map_width, m_map_data->map_height)));
    MessageWriter::Instance()->WriteLineToConsole("Map " +m_map_data->name + " expecting: " +
            std::to_string(m_map_data->benchmarks[m_map_data->selected_bechmark_index].expected_min_path_cost));

    //create the map texture only once
    CreateMapTexture();
    m_destination_rect.w = m_cell_size * m_map_data->map_width;
    m_destination_rect.h = m_cell_size * m_map_data->map_height;
    //try to centre map inits area
    m_destination_rect.x = 1050 / 2 - m_destination_rect.w / 2;
    m_destination_rect.y = 1050 / 2 - m_destination_rect.h / 2;
}

void MapRenderer::RenderMap()
{
    std::lock_guard<std::mutex> lock(render_mutex);
    SDL_RenderCopy(m_renderer, m_image_pointers.map_texture.get(), NULL, &m_destination_rect);
}

void MapRenderer::RenderPath()
{
    std::lock_guard<std::mutex> lock(render_mutex);
    DrawMap();

    if(!m_control_flags->running && m_map_data != nullptr && m_map_data->path_buffer.size() > 0)
    {
        DrawPath();
    }
}

void MapRenderer::RenderGrid()
{
    std::lock_guard<std::mutex> lock(render_mutex);
    SDL_RenderCopy(m_renderer, m_image_pointers.grid_texture.get(), NULL, &m_destination_rect);
}

void MapRenderer::ClearPath()
{
    std::lock_guard<std::mutex> lock(render_mutex);
    m_grid_ui.clear();
}
//</f> /Methods

/**
* \brief Blits a continuous line of nodes of the same type on a surface that will be used to render the nodes
* \brief This method avoids the rendering of individual nodes, its some sort of batch rendering
*/
void MapRenderer::RenderBlock(SDL_Rect& destination_rect, DrawData& data, SDL_Surface* nodes_surface)
{
    if(data.visited)
        SDL_BlitScaled(m_image_pointers.checked_surface.get(), nullptr, nodes_surface, &destination_rect);
    else if(data.to_check)
        SDL_BlitScaled(m_image_pointers.to_check_surface.get(), nullptr, nodes_surface, &destination_rect);

    if(data.start_or_target == 1)//startshort_path
        SDL_BlitScaled(m_image_pointers.start_surface.get(), nullptr, nodes_surface, &destination_rect);
    else if(data.start_or_target == 2)//target
        SDL_BlitScaled(m_image_pointers.target_surface.get(), nullptr, nodes_surface, &destination_rect);
}

/**
* \brief Draws the checked nodes and the nodes to be checked, as well as the start and target nodes
*/
void MapRenderer::DrawMap()
{
    if(m_grid_ui.size() <= 0 || m_map_data == nullptr)
        return;

    auto map_width{m_map_data->map_width};
    auto map_height{m_map_data->map_height};

    SDL_Surface* nodes_surface = SDL_CreateRGBSurface(0, map_width * m_cell_size, map_height * m_cell_size,32,0,0,0,0);
    SDL_Rect destination_rect;

    for( int i = 0; i < map_height; ++i)//each line
    {
        DrawData first_of_sequence = m_grid_ui[i*map_width];//j is 0
        int sequence_length = 0;
        int start_x = 0;
        int start_y = i;

        for(int j = 0; j < map_width; j++)//each column
        {
            int index = i*map_width+j;
            if(m_grid_ui[index] == first_of_sequence)//cell is to be painted with the same elements
            {
                sequence_length++;

                if(j == map_width - 1)//is the last element of the line
                {
                    destination_rect.x = m_cell_size * start_x;
                    destination_rect.y = m_cell_size * start_y;
                    destination_rect.w = m_cell_size * sequence_length;
                    destination_rect.h = m_cell_size;

                    RenderBlock(destination_rect, first_of_sequence, nodes_surface);
                }
            }
            else//break sequence
            {
                destination_rect.x = m_cell_size * start_x;
                destination_rect.y = m_cell_size * start_y;
                destination_rect.w = m_cell_size * sequence_length;
                destination_rect.h = m_cell_size;

                RenderBlock(destination_rect, first_of_sequence, nodes_surface);

                //start new block
                first_of_sequence = m_grid_ui[index];
                sequence_length = 1;//we already have a node
                start_x = j;

                if(j == map_width - 1)//is the last element of the line
                {
                    destination_rect.x = m_cell_size * start_x;
                    destination_rect.y = m_cell_size * start_y;
                    destination_rect.w = m_cell_size * sequence_length;
                    destination_rect.h = m_cell_size;

                    RenderBlock(destination_rect, first_of_sequence, nodes_surface);
                }
            }
        }//for width
    }//for height

    //render image
    SDL_SetColorKey( nodes_surface, SDL_TRUE, SDL_MapRGB( nodes_surface->format, 0x00, 0x00, 0x00 ) );//remove the brack from the unused area
    m_image_pointers.nodes_texture.reset( SDL_CreateTextureFromSurface(m_renderer, nodes_surface) );
    SDL_RenderCopy( m_renderer, m_image_pointers.nodes_texture.get(), nullptr, &m_destination_rect );
    SDL_FreeSurface(nodes_surface);
}

/**
* \brief Draw the shortest path found by the serach algorithm
*/
void MapRenderer::DrawPath()
{
    auto map_width{m_map_data->map_width};
    auto map_height{m_map_data->map_height};

    int* path{m_map_data->path_buffer.data()};
    auto node_count{m_map_data->path_buffer.size()};

    SDL_Surface* path_surface = SDL_CreateRGBSurface(0, map_width * m_cell_size, map_height * m_cell_size,32,0,0,0,0);//create empty surface

    SDL_Rect destination_rect;
    for(int i = 1; i < node_count; i++)//we jump the first node as it is the target, so we do not render over it
    {
        destination_rect.x = m_cell_size * GetCoordinateX(path[i], map_width);
        destination_rect.y = m_cell_size * GetCoordinateY(path[i], map_width);
        destination_rect.w = m_cell_size;
        destination_rect.h = m_cell_size;

        SDL_BlitScaled(m_image_pointers.short_path_surface.get(), nullptr, path_surface, &destination_rect);
    }

    //render image
    SDL_SetColorKey( path_surface, SDL_TRUE, SDL_MapRGB( path_surface->format, 0x00, 0x00, 0x00 ) );//remove the brack from the unused area
    m_image_pointers.path_texture.reset( SDL_CreateTextureFromSurface(m_renderer, path_surface) );
    SDL_RenderCopy( m_renderer, m_image_pointers.path_texture.get(), nullptr, &m_destination_rect );
    SDL_FreeSurface(path_surface);
}

/**
* \brief Creates a texture with the map walls and passable areas. Only one is created when the serach algorithm is called.
*/
void MapRenderer::CreateMapTexture()
{
    auto map_width{m_map_data->map_width};
    auto map_height{m_map_data->map_height};

    SDL_Surface* map_surface = SDL_CreateRGBSurface(0, map_width * m_cell_size, map_height * m_cell_size,32,0,0,0,0);//create empty surface
    SDL_Surface* grid_surface = SDL_CreateRGBSurface(0, map_width * m_cell_size, map_height * m_cell_size,32,0,0,0,0);//create empty surface

    SDL_Rect destination_rect;

    for(int i=0; i < map_height; i++)
    {
        for(int j=0; j<map_width; j++)
        {
            destination_rect.x = j * m_cell_size;
            destination_rect.y = i * m_cell_size;
            destination_rect.w = m_cell_size;
            destination_rect.h = m_cell_size;

            if(m_map_data->map[i * map_width + j] == 1)
                SDL_BlitScaled(m_image_pointers.path_surface.get(), nullptr, map_surface, &destination_rect);
            else
                SDL_BlitScaled(m_image_pointers.wall_surface.get(), nullptr, map_surface, &destination_rect);

            SDL_BlitScaled(m_image_pointers.frame_surface.get(), nullptr, grid_surface, &destination_rect);
        }
    }

    m_image_pointers.map_texture.reset( SDL_CreateTextureFromSurface( m_renderer, map_surface ) );
    //remove inner white from frame
    SDL_SetColorKey( grid_surface, SDL_TRUE, SDL_MapRGB( grid_surface->format, 0xFF, 0xFF, 0xFF ) );
    m_image_pointers.grid_texture.reset( SDL_CreateTextureFromSurface( m_renderer, grid_surface ) );

    //release tmp surfaces
    SDL_FreeSurface(map_surface);
    SDL_FreeSurface(grid_surface);
}

void MapRenderer::LoadGridCells()
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
        throw std::runtime_error("Failed to load image pointers");

    //save surfaces
    m_image_pointers.wall_surface.reset( wall );
    m_image_pointers.path_surface.reset( path );
    m_image_pointers.frame_surface.reset( frame );

    m_image_pointers.to_check_surface.reset( to_check );
    m_image_pointers.checked_surface.reset( checked );
    m_image_pointers.short_path_surface.reset( short_path );
    m_image_pointers.start_surface.reset( start );
    m_image_pointers.target_surface.reset( target );

    //create textures
    m_image_pointers.wall.reset(SDL_CreateTextureFromSurface( m_renderer, wall ));
    m_image_pointers.path.reset(SDL_CreateTextureFromSurface( m_renderer, path ));

    m_image_pointers.start.reset(SDL_CreateTextureFromSurface( m_renderer, start ));
    m_image_pointers.target.reset(SDL_CreateTextureFromSurface( m_renderer, target ));

    m_image_pointers.to_check.reset(SDL_CreateTextureFromSurface( m_renderer, to_check ));
    m_image_pointers.checked.reset(SDL_CreateTextureFromSurface( m_renderer, checked ));
    m_image_pointers.short_path.reset(SDL_CreateTextureFromSurface( m_renderer, short_path ));

    if(m_image_pointers.wall == nullptr || m_image_pointers.path == nullptr || m_image_pointers.start == nullptr || m_image_pointers.target == nullptr ||
        m_image_pointers.to_check == nullptr || m_image_pointers.checked == nullptr || m_image_pointers.short_path == nullptr)
        throw std::runtime_error("Failed to load image pointers");
}
