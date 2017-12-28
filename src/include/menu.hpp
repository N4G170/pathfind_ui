#ifndef MENU_HPP
#define MENU_HPP

#include <map>
#include "ui_structs.hpp"
#include "sdl_gui_manager.hpp"
#include "sdl_gui_label.hpp"
#include "structs.hpp"
#include <mutex>
#include "map_renderer.hpp"
#include <future>

class Menu
{
    public:
        //<f> Constructors & operator=
        /** brief Default constructor */
        Menu(sdl_gui::GuiManager* gui_manager, ControlFlags* control_flags, std::mutex* text_mutex, MapRenderer* map_renderer);
        /** brief Default destructor */
        virtual ~Menu() noexcept;

        /** brief Copy constructor */
        Menu(const Menu& other);
        /** brief Move constructor */
        Menu(Menu&& other) noexcept;

        /** brief Copy operator */
        Menu& operator= (const Menu& other);
        /** brief Move operator */
        Menu& operator= (Menu&& other) noexcept;
        //</f> /Constructors & operator=

        //<f> Virtual Methods

        //</f> /Virtual Methods

        //<f> Methods
        void Logic(float delta_time);
        //</f> /Methods

        //<f> Getters/Setters

        //</f> /Getters/Setters

    protected:
        void InitMenu();

        sdl_gui::GuiManager* m_gui_manager;
        MapRenderer* m_map_renderer;
        std::future<std::string> m_future_result;
        std::string m_result_string{};

        std::vector<std::string> m_map_list;
        //grid related vars
        MapData m_map_data;
        std::string m_previous_map_name{""};

        //<f> Navigation variables
        int m_map_index{ 0 };
        int m_prev_map_index{ -1 };

        int m_benchmark_index{ 0 };
        int m_previous_bechmark_index{ -1 };
        //</f> /Navigation variables

        //<f> Navigation functions
        void ChangeMap(int index_change);
        void ChangeBenchmark(int index_change);
        void RunSearch();
        void GetCurrentMapData();
        //</f> /Navigation functions

        //<f> gui vars
        sdl_gui::Label* m_map_name_label;
        sdl_gui::Label* m_map_number_label;
        sdl_gui::Label* m_benchmark_number_label;
        sdl_gui::Label* m_expected_label;
        sdl_gui::Label* m_result_label;
        sdl_gui::Label* m_warning_label;
        //</f> /gui vars

        //<f> Control Variables
        ControlFlags* m_control_flags;
        std::mutex* m_text_mutex;
        //</f> /Control Variables

        //<f> Control functions
        void ShowHideGrid(sdl_gui::Label*);
        void ShowHideMap(sdl_gui::Label*);
        void ShowHidePath(sdl_gui::Label*);
        void SearchSpeed(sdl_gui::Label*);
        void PauseResume(sdl_gui::Label*);
        //</f> /Control functions
    private:
};



#endif//MENU_HPP
