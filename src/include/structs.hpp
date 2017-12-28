#ifndef STRUCTS_H
#define STRUCTS_H

#include <vector>
#include <random>
#include <utility>
#include <iostream>
#include <set>
#include <map>
#include <functional>
#include <atomic>

#include <thread>
#include <string>
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "config.hpp"
#include <mutex>

enum ERROR_CODE
{
    SDL_INIT_FAIL = -1,
    CELL_IMG_FAIL = -2,
};

enum SEARCH_TYPE
{
    QUARTILE = 1,
    OCTILE_NOT_CORNER = 2,
    OCTILE_HIGH_8 = 3,
    OCTILE_HIGH_64 = 4,
    MAZE = 5,
};

enum HEURISTIC_TYPE
{
    MANHATTAN = 1,
    OCTILE = 2,
};

enum DIRECTION
{
    ORTHOGONAL = 1,
    DIAGONAL = 2,
};

struct MapBenchmark
{
    int start_x;
    int start_y;

    int target_x;
    int target_y;

    float expected_min_path_cost;
};

struct MapData
{
    std::string name;
    SEARCH_TYPE search_type;
    std::vector<unsigned char> map;
    std::vector<int> path_buffer;
    std::vector<MapBenchmark> benchmarks;//each map can have multiple benchmarks

    int map_width;
    int map_height;

    int selected_bechmark_index;

    HEURISTIC_TYPE heuristic_type;

    //each map could have diferent movement costs
    float line_cost;
    float diagonal_cost;
    float min_path_cost;
};

/**
* \brief Basic node structure with the bare minimum amount of data needed by the search algorithm
*/
struct Node
{
    int index = 0;
    float g_cost;
    float f_cost;
    float h_cost;

    Node(){}

    Node(int new_index, float new_g_cost, float new_h_cost):index{new_index}, g_cost{new_g_cost}, h_cost{new_h_cost}
    {
        f_cost = g_cost + new_h_cost;
    }

    bool operator() (const Node& left_node, const Node& right_node) const//used by multiset
    {
        return left_node.f_cost < right_node.f_cost;// && left_node.h_cost < right_node.h_cost;// || left_node.insertion_id > right_node.insertion_id;
    }
};

/**
* \brief Struct used internally by the search algorithm to store data relevant to the current run
*/
struct SearchData
{
    std::multiset<Node, Node> to_visit;//cost is the key amd the value is the index
    std::map<int, bit2> node_status;//keep record of the status of each node we touch (open, closed, new)
    std::map<int, int> parents;//parent cells for each cell
    std::map<int, float> g_costs;//cost of moving to a cell

    int start_index;
    int target_index;

    std::function<float(const int&, const int&, const float&, const float&, const int&, const int&)> Heuristic;
    //std::function<float(float, float, float, float)> Heuristic;
};

struct Transform
{
    SDL_Rect m_rect;
    Transform* m_parent = nullptr;
};

//{ Region Custom unique_ptr deleters
/**
* \brief Struct defining custom destruction rules that are needed by some unique_ptrs
*/
struct Deleters
{
    void operator() (SDL_Window* window)
    {
        //MessageWriter::Instance()->WriteLineToConsole("Calling destroy for SDL_window object pointer...");
        std::cout << "Calling destroy for SDL_window object pointer... \n";
        SDL_DestroyWindow(window);
    }

    void operator() (SDL_Renderer* screen_renderer)
    {
        //MessageWriter::Instance()->WriteLineToConsole("Calling destroy for SDL_Renderer object pointer...");
        std::cout << "Calling destroy for SDL_Renderer object pointer... \n";
        SDL_DestroyRenderer(screen_renderer);
    }

    void operator() (TTF_Font* font)
    {
        //MessageWriter::Instance()->WriteLineToConsole("Calling destroy for TTF_Font object pointer...");
        std::cout << "Calling destroy for TTF_Font object pointer... \n";
        TTF_CloseFont( font );
    }

    void operator() (SDL_Texture* texture)
    {
        //MessageWriter::Instance()->WriteLineToConsole("Calling destroy for SDL_DestroyTexture object pointer...");
        //std::cout << "Calling destroy for SDL_DestroyTexture object pointer... \n";
        SDL_DestroyTexture( texture );
    }

    void operator() (SDL_Surface* surface)
    {
        //MessageWriter::Instance()->WriteLineToConsole("Calling destroy for SDL_FreeSurface object pointer...");
        //std::cout << "Calling destroy for SDL_FreeSurface object pointer... \n";
        SDL_FreeSurface( surface );
    }
};


//{ "Global" pointers struct
/**
* \brief Struct holding "global" pointers that are used by many parts of the application
*/
struct MainPointers
{
    //The window we'll be rendering to
    std::unique_ptr<SDL_Window, Deleters> window;

    // std::unique_ptr<TTF_Font, Deleters> main_font;

    std::unique_ptr<SDL_Renderer, Deleters> screen_renderer;
};

struct ControlFlags
{
    std::atomic<bool> show_grid{false};
    std::atomic<bool> show_map{true};
    std::atomic<bool> show_path{true};
    std::atomic<bool> search_speed_is_fast{false};
    std::atomic<bool> pause_resume{false};
    std::atomic<bool> stop{false};
    std::atomic<bool> quit{false};
    std::atomic<bool> running{false};
};

#endif //STRUCTS_H
