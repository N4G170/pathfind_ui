#ifndef MAP_RENDERER_HPP
#define MAP_RENDERER_HPP

#include <vector>
#include "ui_structs.hpp"
#include "utils.hpp"
#include "structs.hpp"
#include <mutex>

class MapRenderer
{
    public:
        //<f> Constructors & operator=
        /** brief Default constructor */
        MapRenderer(SDL_Renderer* renderer, ControlFlags* control_flags);
        /** brief Default destructor */
        virtual ~MapRenderer() noexcept;

        /** brief Copy constructor */
        MapRenderer(const MapRenderer& other);
        /** brief Move constructor */
        MapRenderer(MapRenderer&& other) noexcept;

        /** brief Copy operator */
        MapRenderer& operator= (const MapRenderer& other);
        /** brief Move operator */
        MapRenderer& operator= (MapRenderer&& other) noexcept;
        //</f> /Constructors & operator=

        //<f> Methods
        void CreateMap();
        void RenderMap();
        void RenderPath();
        void RenderGrid();

        void ClearPath();
        //</f> /Methods

        //<f> Getters/Setters
        //need to be a ref
        void SetMapData(MapData* data);
        void ResizeVector(int size);
        void SetStartNode(int index);
        void SetTargetNode(int index);

        void SetVisited(int index);
        void SetToCheck(int index);
        //</f> /Getters/Setters

    protected:
        /**
        * \brief Blits a continuous line of nodes of the same type on a surface that will be used to render the nodes
        * \brief This method avoids the rendering of individual nodes, its some sort of batch rendering
        */
        void RenderBlock(SDL_Rect& destination_rect, DrawData& data, SDL_Surface* nodes_surface);

        /**
        * \brief Draws the checked nodes and the nodes to be checked, as well as the start and target nodes
        */
        void DrawMap();

        /**
        * \brief Draw the shortest path found by the serach algorithm
        */
        void DrawPath();


        /**
        * \brief Creates a texture with the map walls and passable areas. Only one is created when the serach algorithm is called.
        */
        void CreateMapTexture();

        void LoadGridCells();

        SDL_Renderer* m_renderer;
        GridImagePointers m_image_pointers;
        std::mutex render_mutex;
        SDL_Rect m_destination_rect;
        MapData* m_map_data{nullptr};
        ControlFlags* m_control_flags{nullptr};

        std::vector<DrawData> m_grid_ui;
        int m_cell_size{ 20 };
        int m_grid_size{ 1024 };
    private:
};

#endif //MAP_RENDERER_HPP
