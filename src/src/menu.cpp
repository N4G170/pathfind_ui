#include "menu.hpp"
#include <utility>
#include "map_loader.hpp"
#include <stdexcept>
#include "sdl_gui_constants.hpp"
#include "pathfind.hpp"
#include "high_level_search.hpp"
#include "labyrinth_search.hpp"

//<f> Constructors & operator=
Menu::Menu(sdl_gui::GuiManager* gui_manager, ControlFlags* control_flags, std::mutex* text_mutex, MapRenderer* map_renderer):m_gui_manager{gui_manager},
    m_map_renderer{map_renderer}, m_control_flags{control_flags}, m_text_mutex{text_mutex}
{
    if( !LoadMapList(m_map_list) )
        throw std::runtime_error("Failed to load map list");

    if( m_map_list.empty() )
        throw std::runtime_error("Map list is empty");

    GetCurrentMapData();
    InitMenu();
}

Menu::~Menu() noexcept
{

}

Menu::Menu(const Menu& other)
{

}

Menu::Menu(Menu&& other) noexcept
{

}

Menu& Menu::operator=(const Menu& other)
{
    if(this != &other)//not same ref
    {
        auto tmp(other);
        *this = std::move(tmp);
    }

    return *this;
}

Menu& Menu::operator=(Menu&& other) noexcept
{
    if(this != &other)//not same ref
    {
        //move here
    }
    return *this;
}
//</f> /Constructors & operator=

//<f> Methods
void Menu::Input(SDL_Event& event)
{
    if( event.type == SDL_KEYDOWN)
    {
        switch (event.key.keysym.sym)
        {
            case SDLK_r: RunSearch(); break;

            case SDLK_DOWN: ChangeBenchmark(-1); break;
            case SDLK_UP: ChangeBenchmark(1); break;

            case SDLK_RIGHT: ChangeMap(1); break;
            case SDLK_LEFT: ChangeMap(-1); break;

            case SDLK_f: SearchSpeed(m_speed_label); break;

            default:
            /* code */
            break;
        }
    }
}

void Menu::Logic(float delta_time)
{
    if(m_future_result.valid() && m_future_result.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
    {
        auto result{m_future_result.get()};//get() moves the value
        if(result != m_result_string)
        {
            m_result_string = result;

            auto result_str = m_result_string.substr(0, m_result_string.find("|"));
            auto result_time = m_result_string.substr(m_result_string.find("|") + 1);

            m_result_label->Text("<b>Result length: </b>"+result_str, sdl_gui::Colour::White);
            m_result_time_label->Text("<b>Search time: </b>"+result_time+" ms", sdl_gui::Colour::White);
        }
    }
}
//</f> /Methods

void Menu::InitMenu()
{
    auto menu_container{m_gui_manager->CreateElement<sdl_gui::GuiElement>({1050,25}, {0,0})};
    sdl_gui::Dimensions container_size{300,1000};
    auto bg{m_gui_manager->CreateElement<sdl_gui::Image>({0,0}, container_size)};
    bg->ColourModulation({12,34,56,255});
    bg->Parent(menu_container);
    bg->LocalPosition({0,0});

    float x{0}, y{10};
    float x_spacing{10}, y_spacing{10};
    float line_thickness{3};

    auto centre_element_x = [&container_size](sdl_gui::GuiElement* element, float y)->sdl_gui::Position { return {container_size.w/2 - element->Size().w/2, y}; };
    // auto centre_element_y = [&container_size](sdl_gui::GuiElement* element, float x)->sdl_gui::Position { return {x, container_size.h/2 - element->Size().h/2}; };
    // auto centre_element_xy = [&container_size](sdl_gui::GuiElement* element)->sdl_gui::Position { return {container_size.w/2 - element->Size().w/2, container_size.h/2 - element->Size().h/2}; };

    //<f> Title
    auto title{m_gui_manager->CreateElement<sdl_gui::Label>({0,0}, {})};
    title->Parent(menu_container);
    title->FontSize(25);
    title->Text("Pathfinder Program", sdl_gui::Colour::White);
    title->LocalPosition( centre_element_x(title, y) );
    //</f> /Title

    y = title->LocalPosition().y + title->Size().h + y_spacing * 2;

    //<f> First Break Line
    auto line{m_gui_manager->CreateElement<sdl_gui::Image>({0,0}, {container_size.w, line_thickness})};
    line->Parent(menu_container);
    line->LocalPosition(centre_element_x(line, y));
    //</f> /First Break Line

    y += line_thickness + y_spacing * 2;

    //<f> Maps List
    auto maps_title{m_gui_manager->CreateElement<sdl_gui::Label>({0,0}, {})};
    maps_title->Parent(menu_container);
    maps_title->FontSize(18);
    maps_title->Text("Maps list", sdl_gui::Colour::White);
    maps_title->LocalPosition( centre_element_x(maps_title, y) );

    y += maps_title->Size().h + y_spacing;

    //<f> Change map btns
    float change_btn_w{30}, change_btn_h{30};
    x = x_spacing;
    auto map_minus_btn{m_gui_manager->CreateElement<sdl_gui::Button>({0,0}, {change_btn_w, change_btn_h})};
    map_minus_btn->Parent(menu_container);
    map_minus_btn->LocalPosition({x,y});
    map_minus_btn->CreateLabel("<", sdl_gui::c_default_font_path, 25, sdl_gui::Colour::Black, {0,0});
    map_minus_btn->CentreLabel();
    map_minus_btn->MouseInteractionPtr()->MouseButtonCallback(SDL_BUTTON_LEFT, sdl_gui::InputKeyCallbackType::CLICK,
        std::bind(&Menu::ChangeMap, this, -1));

    x = container_size.w - x_spacing - change_btn_w;
    auto map_plus_btn{m_gui_manager->CreateElement<sdl_gui::Button>({0,0}, {change_btn_w, change_btn_h})};
    map_plus_btn->Parent(menu_container);
    map_plus_btn->LocalPosition({x,y});
    map_plus_btn->CreateLabel(">", sdl_gui::c_default_font_path, 25, sdl_gui::Colour::Black, {0,0});
    map_plus_btn->CentreLabel();
    map_plus_btn->MouseInteractionPtr()->MouseButtonCallback(SDL_BUTTON_LEFT, sdl_gui::InputKeyCallbackType::CLICK,
        std::bind(&Menu::ChangeMap, this, 1));
    //</f> /Change map btns

    m_map_name_label = m_gui_manager->CreateElement<sdl_gui::Label>({0,0}, {});
    m_map_name_label->Parent(menu_container);
    m_map_name_label->FontSize(16);
    m_map_name_label->Text(m_map_list[0], sdl_gui::Colour::White);

    y += change_btn_h / 2 - m_map_name_label->Size().h/2;//align with buttons
    m_map_name_label->Anchor(sdl_gui::AnchorType::TOP_CENTRE);
    m_map_name_label->LocalPosition( {container_size.w / 2, y});

    //map count
    m_map_number_label = m_gui_manager->CreateElement<sdl_gui::Label>({0,0}, {0,0});
    m_map_number_label->Parent(menu_container);
    m_map_number_label->FontSize(16);
    m_map_number_label->Text("1 / "+ std::to_string(m_map_list.size()), sdl_gui::Colour::White);

    y = map_minus_btn->LocalPosition().y + map_minus_btn->Size().h + y_spacing;
    m_map_number_label->Anchor(sdl_gui::AnchorType::TOP_CENTRE);
    m_map_number_label->LocalPosition( {container_size.w / 2, y});
    //</f> /Maps List

    y = m_map_number_label->LocalPosition().y + m_map_number_label->Size().h + y_spacing;

    //<f> Warning
    m_warning_label = m_gui_manager->CreateElement<sdl_gui::Label>({0,0}, {0,0});
    m_warning_label->Parent(menu_container);
    m_warning_label->FontSize(13);
    m_warning_label->Text("Hierarchical room search", sdl_gui::Colour::Red);
    m_warning_label->Anchor(sdl_gui::AnchorType::TOP_CENTRE);
    m_warning_label->LocalPosition({container_size.w / 2, y});
    m_warning_label->Enabled( m_map_list[0].rfind("_high") != std::string::npos );
    //</f> /Warning

    y += m_warning_label->Size().h + y_spacing * 2;

    //<f> Break Line
    line = m_gui_manager->CreateElement<sdl_gui::Image>({0,0}, {container_size.w, line_thickness});
    line->Parent(menu_container);
    line->LocalPosition(centre_element_x(line, y));
    //</f> /Break Line

    y += line_thickness + y_spacing * 2;

    //<f> Benchmarks
    auto benchmarks_title{m_gui_manager->CreateElement<sdl_gui::Label>({0,0}, {})};
    benchmarks_title->Parent(menu_container);
    benchmarks_title->FontSize(18);
    benchmarks_title->Text("Map benchmarks", sdl_gui::Colour::White);
    benchmarks_title->LocalPosition( centre_element_x(benchmarks_title, y) );

    y += benchmarks_title->Size().h + y_spacing;

    //<f> Change benchmark btns
    float benchmark_btn_w{30}, benchmark_btn_h{30};
    x = x_spacing;
    auto benchmark_minus_btn{m_gui_manager->CreateElement<sdl_gui::Button>({0,0}, {benchmark_btn_w, benchmark_btn_h})};
    benchmark_minus_btn->Parent(menu_container);
    benchmark_minus_btn->LocalPosition({x,y});
    benchmark_minus_btn->CreateLabel("<", sdl_gui::c_default_font_path, 25, sdl_gui::Colour::Black, {0,0});
    benchmark_minus_btn->CentreLabel();
    benchmark_minus_btn->MouseInteractionPtr()->MouseButtonCallback(SDL_BUTTON_LEFT, sdl_gui::InputKeyCallbackType::CLICK,
        std::bind(&Menu::ChangeBenchmark, this, -1));

    x = container_size.w - x_spacing - benchmark_btn_w;
    auto benchmark_plus_btn{m_gui_manager->CreateElement<sdl_gui::Button>({0,0}, {benchmark_btn_w, benchmark_btn_h})};
    benchmark_plus_btn->Parent(menu_container);
    benchmark_plus_btn->LocalPosition({x,y});
    benchmark_plus_btn->CreateLabel(">", sdl_gui::c_default_font_path, 25, sdl_gui::Colour::Black, {0,0});
    benchmark_plus_btn->CentreLabel();
    benchmark_plus_btn->MouseInteractionPtr()->MouseButtonCallback(SDL_BUTTON_LEFT, sdl_gui::InputKeyCallbackType::CLICK,
        std::bind(&Menu::ChangeBenchmark, this, 1));
    //</f> /Change benchmark btns

    m_benchmark_number_label = m_gui_manager->CreateElement<sdl_gui::Label>({0,0}, {});
    m_benchmark_number_label->Parent(menu_container);
    m_benchmark_number_label->FontSize(16);
    m_benchmark_number_label->Text("1 / "+ std::to_string(m_map_data.benchmarks.size()), sdl_gui::Colour::White);

    x = centre_element_x(m_benchmark_number_label, y).x;
    y += benchmark_btn_h / 2 - m_benchmark_number_label->Size().h/2;//align with buttons
    m_benchmark_number_label->LocalPosition(centre_element_x(m_benchmark_number_label, y));

    //</f> /Benchmarks

    y = m_benchmark_number_label->LocalPosition().y + m_benchmark_number_label->Size().h + y_spacing * 2;

    //<f> Break Line
    line = m_gui_manager->CreateElement<sdl_gui::Image>({0,0}, {container_size.w, line_thickness});
    line->Parent(menu_container);
    line->LocalPosition(centre_element_x(line, y));
    //</f> /Break Line

    y += line_thickness + y_spacing * 2;

    //<f> Run benchmark
    x = x_spacing;
    m_expected_label = m_gui_manager->CreateElement<sdl_gui::Label>({0,0}, {0,0});
    m_expected_label->Parent(menu_container);
    m_expected_label->FontSize(14);
    m_expected_label->Text("<b>Expected length:</b> "+ std::to_string(m_map_data.benchmarks[0].expected_min_path_cost), sdl_gui::Colour::White);
    m_expected_label->LocalPosition({x, y});

    y += m_expected_label->Size().h + y_spacing;

    m_result_label = m_gui_manager->CreateElement<sdl_gui::Label>({0,0}, {0,0});
    m_result_label->Parent(menu_container);
    m_result_label->FontSize(14);
    m_result_label->Text("<b>Result length:</b> ", sdl_gui::Colour::White);
    m_result_label->LocalPosition({x, y});

    y += m_result_label->Size().h + y_spacing;

    m_result_time_label = m_gui_manager->CreateElement<sdl_gui::Label>({0,0}, {0,0});
    m_result_time_label->Parent(menu_container);
    m_result_time_label->FontSize(14);
    m_result_time_label->Text("<b>Search time:</b> ", sdl_gui::Colour::White);
    m_result_time_label->LocalPosition({x, y});

    y += m_result_time_label->Size().h + y_spacing;

    auto run_btn{m_gui_manager->CreateElement<sdl_gui::Button>({0,0}, {container_size.w - x_spacing * 2, 30})};
    run_btn->Parent(menu_container);
    run_btn->LocalPosition({x, y});
    run_btn->CreateLabel("Find Path (R)", sdl_gui::c_default_font_path, 16, sdl_gui::Colour::Black, {0,0});
    run_btn->CentreLabel();
    run_btn->MouseInteractionPtr()->MouseButtonCallback(SDL_BUTTON_LEFT, sdl_gui::InputKeyCallbackType::CLICK,
        std::bind(&Menu::RunSearch, this ));
    //</f> /Run benchmark

    y += run_btn->Size().h + y_spacing * 2;

    //<f> Break Line
    line = m_gui_manager->CreateElement<sdl_gui::Image>({0,0}, {container_size.w, line_thickness});
    line->Parent(menu_container);
    line->LocalPosition(centre_element_x(line, y));
    //</f> /Break Line

    y += line_thickness + y_spacing * 2;

    //<f> Commands
    float btn_w{30}, btn_h{30};

    //<f> Speed
    auto speed_btn{m_gui_manager->CreateElement<sdl_gui::Button>({0,0}, {btn_w, btn_h})};
    speed_btn->Parent(menu_container);

    m_speed_label = m_gui_manager->CreateElement<sdl_gui::Label>({0,0}, {0,0});
    m_speed_label->Parent(menu_container);
    m_speed_label->FontSize(16);
    m_speed_label->Text("Search speed - Normal", sdl_gui::Colour::White);

    speed_btn->MouseInteractionPtr()->MouseButtonCallback(SDL_BUTTON_LEFT, sdl_gui::InputKeyCallbackType::CLICK,
    std::bind( &Menu::SearchSpeed, this, m_speed_label ) );

    //position
    x = x_spacing;
    speed_btn->LocalPosition({x,y});

    m_speed_label->LocalPosition( {x + btn_w + x_spacing, y + btn_h / 2 - m_speed_label->Size().h/2} );
    //</f> /Speed

    y += btn_h + y_spacing;

    //<f> Pause
    auto pause_btn{m_gui_manager->CreateElement<sdl_gui::Button>({0,0}, {btn_w, btn_h})};
    pause_btn->Parent(menu_container);

    auto pause_label{m_gui_manager->CreateElement<sdl_gui::Label>({0,0}, {0,0})};
    pause_label->Parent(menu_container);
    pause_label->FontSize(16);
    pause_label->Text("Pause search", sdl_gui::Colour::White);

    pause_btn->MouseInteractionPtr()->MouseButtonCallback(SDL_BUTTON_LEFT, sdl_gui::InputKeyCallbackType::CLICK,
    std::bind( &Menu::PauseResume, this, pause_label ) );

    //position
    x = x_spacing;
    pause_btn->LocalPosition({x,y});

    pause_label->LocalPosition( {x + btn_w + x_spacing, y + btn_h / 2 - pause_label->Size().h/2} );
    //</f> /Pause

    y += btn_h + y_spacing;

    //<f> Stop
    auto stop_btn{m_gui_manager->CreateElement<sdl_gui::Button>({0,0}, {btn_w, btn_h})};
    stop_btn->Parent(menu_container);

    auto stop_label{m_gui_manager->CreateElement<sdl_gui::Label>({0,0}, {0,0})};
    stop_label->Parent(menu_container);
    stop_label->FontSize(16);
    stop_label->Text("Stop search", sdl_gui::Colour::White);

    stop_btn->MouseInteractionPtr()->MouseButtonCallback(SDL_BUTTON_LEFT, sdl_gui::InputKeyCallbackType::CLICK,
    [this](){m_control_flags->stop = true;} );

    //position
    x = x_spacing;
    stop_btn->LocalPosition({x,y});

    stop_label->LocalPosition( {x + btn_w + x_spacing, y + btn_h / 2 - stop_label->Size().h/2} );
    //</f> /Stop

    y += btn_h + y_spacing;

    //<f> Map
    auto map_btn{m_gui_manager->CreateElement<sdl_gui::Button>({0,0}, {btn_w, btn_h})};
    map_btn->Parent(menu_container);

    auto map_label{m_gui_manager->CreateElement<sdl_gui::Label>({0,0}, {0,0})};
    map_label->Parent(menu_container);
    map_label->FontSize(16);
    map_label->Text("Hide map", sdl_gui::Colour::White);

    map_btn->MouseInteractionPtr()->MouseButtonCallback(SDL_BUTTON_LEFT, sdl_gui::InputKeyCallbackType::CLICK,
    std::bind( &Menu::ShowHideMap, this, map_label ) );

    //position
    x = x_spacing;
    map_btn->LocalPosition({x,y});

    map_label->LocalPosition( {x + btn_w + x_spacing, y + btn_h / 2 - map_label->Size().h/2} );
    //</f> /Map

    y += btn_h + y_spacing;

    //<f> Path
    auto path_btn{m_gui_manager->CreateElement<sdl_gui::Button>({0,0}, {btn_w, btn_h})};
    path_btn->Parent(menu_container);

    auto path_label{m_gui_manager->CreateElement<sdl_gui::Label>({0,0}, {0,0})};
    path_label->Parent(menu_container);
    path_label->FontSize(16);
    path_label->Text("Hide path", sdl_gui::Colour::White);

    path_btn->MouseInteractionPtr()->MouseButtonCallback(SDL_BUTTON_LEFT, sdl_gui::InputKeyCallbackType::CLICK,
    std::bind( &Menu::ShowHidePath, this, path_label ) );

    //position
    x = x_spacing;
    path_btn->LocalPosition({x,y});

    path_label->LocalPosition( {x + btn_w + x_spacing, y + btn_h / 2 - path_label->Size().h/2} );
    //</f> /Path

    y += btn_h + y_spacing;

    //<f> Grid
    auto grid_btn{m_gui_manager->CreateElement<sdl_gui::Button>({0,0}, {btn_w, btn_h})};
    grid_btn->Parent(menu_container);

    auto grid_label{m_gui_manager->CreateElement<sdl_gui::Label>({0,0}, {0,0})};
    grid_label->Parent(menu_container);
    grid_label->FontSize(16);
    grid_label->Text("Show grid", sdl_gui::Colour::White);

    grid_btn->MouseInteractionPtr()->MouseButtonCallback(SDL_BUTTON_LEFT, sdl_gui::InputKeyCallbackType::CLICK,
    std::bind( &Menu::ShowHideGrid, this, grid_label ) );

    //position
    x = x_spacing;
    grid_btn->LocalPosition({x,y});

    grid_label->LocalPosition( {x + btn_w + x_spacing, y + btn_h / 2 - grid_label->Size().h/2} );
    //</f> /Grid
    //</f> /Commands

    y += btn_h + y_spacing *2;

    //<f> Break Line
    line = m_gui_manager->CreateElement<sdl_gui::Image>({0,0}, {container_size.w, line_thickness});
    line->Parent(menu_container);
    line->LocalPosition(centre_element_x(line, y));
    //</f> /Break Line

    y += line_thickness + y_spacing * 2;

    //<f> Caption
    auto caption_title{m_gui_manager->CreateElement<sdl_gui::Label>({0,0}, {})};
    caption_title->Parent(menu_container);
    caption_title->FontSize(18);
    caption_title->Text("Caption", sdl_gui::Colour::White);
    caption_title->LocalPosition( centre_element_x(caption_title, y) );

    y += caption_title->Size().h + y_spacing;

    //<f> squares
    x = x_spacing;
    // y += y_spacing;

    std::vector<std::string> captions{"Start node", "End node", "Passable node", "Impassable node", "Node to check", "Checked node", "Shortest path node"};

    for(auto i{0}; i<captions.size(); ++i)
    {
        auto local_x{x};
        auto square{m_gui_manager->CreateElement<sdl_gui::Image>({0,0}, {22,22})};
        square->ChangeTexture("data/imgs/caption.png");
        square->SetSourceRect({25 * i,0,25,25});
        square->Parent(menu_container);
        square->LocalPosition({local_x,y});

        local_x += square->Size().w + x_spacing;
        auto square_label{m_gui_manager->CreateElement<sdl_gui::Label>({0,0}, {0,0})};
        square_label->Parent(menu_container);
        square_label->FontSize(16);
        square_label->Text(captions[i], sdl_gui::Colour::White);
        square_label->LocalPosition({local_x, y});

        y += 22 + y_spacing / 2;
    }
    //</f> /squares
    //</f> /Caption

    ChangeMap(0);//show first map
}

//<f> Navigation functions
void Menu::ChangeMap(int index_change)
{
    if(m_control_flags->running)
        return;

    m_map_index += index_change;

    if(m_map_index < 0)
        m_map_index = m_map_list.size() - 1;
    else if(m_map_index >= m_map_list.size())
        m_map_index = 0;

    m_map_name_label->Text(m_map_list[m_map_index], sdl_gui::Colour::White);
    m_map_number_label->Text(std::to_string(m_map_index + 1)+" / "+std::to_string(m_map_list.size()), sdl_gui::Colour::White);

    //if map has _high of fast in name, show warning
    if(m_map_list[m_map_index].rfind("_fast") != std::string::npos)
    {
        m_warning_label->Text("MAZE NODE SEARCH", sdl_gui::Colour::Red);
        m_warning_label->Enabled( true );
    }
    else if(m_map_list[m_map_index].rfind("_high") != std::string::npos)
    {
        m_warning_label->Text("HIERARCHICAL ROOM SEARCH", sdl_gui::Colour::Red);
        m_warning_label->Enabled( true );
    }
    else
    {
        m_warning_label->Enabled( false );
    }

    GetCurrentMapData();

    m_benchmark_index = 0;
    m_benchmark_number_label->Text(std::to_string(m_benchmark_index + 1)+" / "+std::to_string(m_map_data.benchmarks.size()), sdl_gui::Colour::White);
    m_expected_label->Text("<b>Expected length:</b> "+ std::to_string(m_map_data.benchmarks[m_benchmark_index].expected_min_path_cost), sdl_gui::Colour::White);
    m_map_data.selected_bechmark_index = m_benchmark_index;
    m_result_label->Text("<b>Result length:</b> ", sdl_gui::Colour::White);
    m_result_time_label->Text("<b>Search time:</b> ", sdl_gui::Colour::White);

    m_map_renderer->SetMapData(&m_map_data);
    m_map_renderer->CreateMap();
    m_map_renderer->ClearPath();
}

void Menu::ChangeBenchmark(int index_change)
{
    if(m_control_flags->running)
        return;

    m_benchmark_index += index_change;

    if(m_benchmark_index < 0)
        m_benchmark_index = m_map_data.benchmarks.size() - 1;
    else if(m_benchmark_index >= m_map_data.benchmarks.size())
        m_benchmark_index = 0;

    m_benchmark_number_label->Text(std::to_string(m_benchmark_index + 1)+" / "+std::to_string(m_map_data.benchmarks.size()), sdl_gui::Colour::White);
    m_expected_label->Text("<b>Expected length:</b> "+ std::to_string(m_map_data.benchmarks[m_benchmark_index].expected_min_path_cost), sdl_gui::Colour::White);
    m_result_label->Text("<b>Result length:</b> ", sdl_gui::Colour::White);
    m_result_time_label->Text("<b>Search time:</b> ", sdl_gui::Colour::White);
    m_map_data.selected_bechmark_index = m_benchmark_index;
}

void Menu::RunSearch()
{
    if(m_control_flags->running)
        return;

    //config vars & flags
    m_control_flags->running = true;
    m_control_flags->stop = false;
    m_control_flags->pause_resume = false;
    //set movement costs (should be moved to map data load as weach map could have different costs)
    m_map_data.line_cost = 1;
    // m_map_data.diagonal_cost = 1.4142135623730950488f;
    // m_map_data.diagonal_cost = 1.414213562373095048801688724209698078569671875376948073176;
    // m_map_data.diagonal_cost = 1.4142135623730950488016;
    m_map_data.diagonal_cost = std::sqrt(2);
    m_map_data.selected_bechmark_index = m_benchmark_index;

    m_map_renderer->SetMapData(&m_map_data);
    m_map_renderer->CreateMap();

    // std::thread pathfind_thread{};
    if(m_map_data.search_type == SEARCH_TYPE::OCTILE_HIGH_8)
    {
        // HighLevelSearch( map_data, grid_ui, flags, menu_text_objects, 64, 8);
        //start search thread and sends it all needed data
        // pathfind_thread = std::thread{ HighLevelSearch, std::ref(m_map_data), m_map_renderer, m_control_flags, 64, 8, m_result_label, m_text_mutex };

        m_future_result = std::async( std::launch::async, HighLevelSearch, std::ref(m_map_data), m_map_renderer, m_control_flags, 64, 8, m_result_label, m_text_mutex );
        //there is a shared flag that will stop the thread so it will not keep running after we terminate the program
        //also we need the flag as we cannot join the thread after we detach it
        // pathfind_thread.detach();
    }
    else if(m_map_data.search_type == SEARCH_TYPE::OCTILE_HIGH_64)
    {
        // HighLevelSearch(map_data, grid_ui, flags, menu_text_objects, 8, 64);
        //start search thread and sends it all needed data
        // pathfind_thread = std::thread{ HighLevelSearch, std::ref(m_map_data), m_map_renderer, m_control_flags, 8, 64, m_result_label, m_text_mutex };

        m_future_result = std::async( std::launch::async, HighLevelSearch, std::ref(m_map_data), m_map_renderer, m_control_flags, 8, 64, m_result_label, m_text_mutex );
        //there is a shared flag that will stop the thread so it will not keep running after we terminate the program
        //also we need the flag as we cannot join the thread after we detach it
        // pathfind_thread.detach();
    }
    else if(m_map_data.search_type == SEARCH_TYPE::MAZE)
    {
        // HighLevelSearch(map_data, grid_ui, flags, menu_text_objects, 8, 64);
        //start search thread and sends it all needed data
        // pathfind_thread = std::thread{ HighLevelSearch, std::ref(m_map_data), m_map_renderer, m_control_flags, 8, 64, m_result_label, m_text_mutex };

        m_future_result = std::async( std::launch::async, FindLabyrinthPath, std::ref(m_map_data), m_map_renderer, m_control_flags, m_result_label, m_text_mutex );
        //there is a shared flag that will stop the thread so it will not keep running after we terminate the program
        //also we need the flag as we cannot join the thread after we detach it
        // pathfind_thread.detach();
    }
    else
    {
        //start search thread and sends it all needed data
        // pathfind_thread = std::thread{ FindPath, std::ref(m_map_data), m_map_renderer, m_control_flags, m_result_label, m_text_mutex };

        m_future_result = std::async( std::launch::async, FindPath, std::ref(m_map_data), m_map_renderer, m_control_flags, m_result_label, m_text_mutex);
        //there is a shared flag that will stop the thread so it will not keep running after we terminate the program
        //also we need the flag as we cannot join the thread after we detach it
        // pathfind_thread.detach();
    }
}

void Menu::GetCurrentMapData()
{
    LoadMap(m_map_list[m_map_index], m_map_data);
}
//</f> /Navigation functions

//<f> Control functions
void Menu::ShowHideGrid(sdl_gui::Label* label)
{
    m_control_flags->show_grid = !m_control_flags->show_grid;

    if(m_control_flags->show_grid)
        label->Text("Hide grid", sdl_gui::Colour::White);
    else
        label->Text("Show grid", sdl_gui::Colour::White);
}

void Menu::ShowHideMap(sdl_gui::Label* label)
{
    m_control_flags->show_map = !m_control_flags->show_map;

    if(m_control_flags->show_map)
        label->Text("Hide map", sdl_gui::Colour::White);
    else
        label->Text("Show map", sdl_gui::Colour::White);
}

void Menu::ShowHidePath(sdl_gui::Label* label)
{
    m_control_flags->show_path = !m_control_flags->show_path;

    if(m_control_flags->show_path)
        label->Text("Hide path", sdl_gui::Colour::White);
    else
        label->Text("Show path", sdl_gui::Colour::White);
}

void Menu::SearchSpeed(sdl_gui::Label* label)
{
    m_control_flags->search_speed_is_fast = !m_control_flags->search_speed_is_fast;

    if(m_control_flags->search_speed_is_fast)
        label->Text("Search speed - Fast", sdl_gui::Colour::White);
    else
        label->Text("Search speed - Normal", sdl_gui::Colour::White);
}

void Menu::PauseResume(sdl_gui::Label* label)
{
    m_control_flags->pause_resume = !m_control_flags->pause_resume;

    if(m_control_flags->pause_resume)
        label->Text("Resume search", sdl_gui::Colour::White);
    else
        label->Text("Pause search", sdl_gui::Colour::White);
}
//</f> /Control functions
