#include "menu.hpp"

/**
* \brief Initialize the std::map with the Text objects that own all rendered text
*/
void  InitMenu(MainPointers& main_pointers, std::map<std::string, Text>& menu_text_objects)
{
    int y{ 80 };
    //prepare menu
    menu_text_objects["fps"] = Text(main_pointers, 1170, 10, "0");
    menu_text_objects["title"] = Text(main_pointers, 1050, 50, "Pathfinder Program");

    menu_text_objects["maps"] = Text(main_pointers, 1050, y, "Maps (8 directions no corner)"); y += 30;
    menu_text_objects["map_name"] = Text(main_pointers, 1050, y, "<-|    |->"); y += 30;
    menu_text_objects["m_note"] = Text(main_pointers, 1050, y, "L/R Arrows to change map"); y += 30;

    menu_text_objects["m_benchmark"] = Text(main_pointers, 1050, y, "<-|    |-> U/D Arrows change benchmark"); y += 30;
    menu_text_objects["m_note_2"] = Text(main_pointers, 1050, y, "'R' to run selected map benchmark"); y += 45;

    menu_text_objects["warning"] = Text(main_pointers, 1050, y, "WARNING-EXPERIMENTAL HIERARCHICAL MAP"); y += 30;
    menu_text_objects["line_2"] = Text(main_pointers, 1050, y, "------------------------------------"); y += 30;

    menu_text_objects["toggle_grid"] = Text(main_pointers, 1050, y, "'G' - Show Grid"); y += 30;
    menu_text_objects["toggle_map"] = Text(main_pointers, 1050, y, "'H' - Hide Map"); y += 30;
    menu_text_objects["toggle_path"] = Text(main_pointers, 1050, y, "'J' - Hide Path"); y += 30;

    menu_text_objects["toggle_pause"] = Text(main_pointers, 1050, y, "'P' - Pause Search"); y += 30;
    menu_text_objects["break"] = Text(main_pointers, 1050, y, "'B' - Stop Search"); y += 30;
    menu_text_objects["speed"] = Text(main_pointers, 1050, y, "'F' - Set search speed Fast"); y += 30;

    menu_text_objects["line_3"] = Text(main_pointers, 1050, y, "------------------------------------"); y += 30;

    menu_text_objects["current_map"] = Text(main_pointers, 1050, y, "Current Map: "); y += 30;
    menu_text_objects["expected"] = Text(main_pointers, 1050, y, "Expected length: "); y += 30;
    menu_text_objects["result"] = Text(main_pointers, 1050, y, "Result length: ");

    y += 50;
    menu_text_objects["l_to_check"] = Text(main_pointers, 1100, y, "Node to check "); y += 25;
    menu_text_objects["l_checked"] = Text(main_pointers, 1100, y, "Checked node"); y += 25;
    menu_text_objects["l_short_path"] = Text(main_pointers, 1100, y, "Node in short path"); y += 25;

    menu_text_objects["l_start"] = Text(main_pointers, 1100, y, "Start node"); y += 25;
    menu_text_objects["l_target"] = Text(main_pointers, 1100, y, "Target node"); y += 25;

    menu_text_objects["l_path"] = Text(main_pointers, 1100, y, "Passable node"); y += 25;
    menu_text_objects["l_wall"] = Text(main_pointers, 1100, y, "Impassable node ");

}

/**
* \brief Renders all the Text objects and the nodes color legend
*/
void RenderMenu(MainPointers& main_pointers, std::map<std::string, Text>& menu_text_objects, GridImagePointers& image_pointers, const bool& show_warning, std::mutex& text_mutex)
{
    std::lock_guard<std::mutex> lock(text_mutex);

    menu_text_objects["title"].Render(main_pointers.screen_renderer);
    menu_text_objects["maps"].Render(main_pointers.screen_renderer);
    menu_text_objects["map_name"].Render(main_pointers.screen_renderer);
    menu_text_objects["m_note"].Render(main_pointers.screen_renderer);
    menu_text_objects["m_benchmark"].Render(main_pointers.screen_renderer);
    menu_text_objects["m_note_2"].Render(main_pointers.screen_renderer);
    menu_text_objects["line_2"].Render(main_pointers.screen_renderer);
    menu_text_objects["toggle_map"].Render(main_pointers.screen_renderer);
    menu_text_objects["toggle_grid"].Render(main_pointers.screen_renderer);
    menu_text_objects["toggle_path"].Render(main_pointers.screen_renderer);
    menu_text_objects["toggle_pause"].Render(main_pointers.screen_renderer);
    menu_text_objects["break"].Render(main_pointers.screen_renderer);
    menu_text_objects["speed"].Render(main_pointers.screen_renderer);
    menu_text_objects["line_3"].Render(main_pointers.screen_renderer);
    menu_text_objects["current_map"].Render(main_pointers.screen_renderer);
    menu_text_objects["expected"].Render(main_pointers.screen_renderer);
    menu_text_objects["result"].Render(main_pointers.screen_renderer);
    menu_text_objects["fps"].Render(main_pointers.screen_renderer);

    //render legend text
    menu_text_objects["l_to_check"].Render(main_pointers.screen_renderer);
    menu_text_objects["l_checked"].Render(main_pointers.screen_renderer);
    menu_text_objects["l_short_path"].Render(main_pointers.screen_renderer);

    menu_text_objects["l_start"].Render(main_pointers.screen_renderer);
    menu_text_objects["l_target"].Render(main_pointers.screen_renderer);

    menu_text_objects["l_path"].Render(main_pointers.screen_renderer);
    menu_text_objects["l_wall"].Render(main_pointers.screen_renderer);

    //WARNING
    if(show_warning)
        menu_text_objects["warning"].Render(main_pointers.screen_renderer);

    //render legend images
    SDL_Rect destination_rect;
    destination_rect.w = 25;
    destination_rect.h = 25;
    destination_rect.x = 1070;
    destination_rect.y = 616;

    //render image
    SDL_RenderCopy( main_pointers.screen_renderer.get(), image_pointers.to_check.get(), nullptr, &destination_rect ); destination_rect.y += 26;
    SDL_RenderCopy( main_pointers.screen_renderer.get(), image_pointers.checked.get(), nullptr, &destination_rect ); destination_rect.y += 26;
    SDL_RenderCopy( main_pointers.screen_renderer.get(), image_pointers.short_path.get(), nullptr, &destination_rect ); destination_rect.y += 26;

    SDL_RenderCopy( main_pointers.screen_renderer.get(), image_pointers.start.get(), nullptr, &destination_rect ); destination_rect.y += 26;
    SDL_RenderCopy( main_pointers.screen_renderer.get(), image_pointers.target.get(), nullptr, &destination_rect ); destination_rect.y += 26;

    SDL_RenderCopy( main_pointers.screen_renderer.get(), image_pointers.path.get(), nullptr, &destination_rect ); destination_rect.y += 26;
    SDL_RenderCopy( main_pointers.screen_renderer.get(), image_pointers.wall.get(), nullptr, &destination_rect ); destination_rect.y += 26;
}
