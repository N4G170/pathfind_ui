#include "text.hpp"

void Text::SetString(const std::string& text )
{
    std::lock_guard<std::mutex> lock(m_string_mutex);

    if(m_text == text)
        return;

    m_text = text;

    if(text.empty())
    {
        if(m_text_texture)
            m_text_texture.reset(nullptr);
        return;
    }

    // SDL_Surface* textSurface = nullptr;

    /*if(m_multiline)
        textSurface = TTF_RenderText_Blended_Wrapped( m_font, text.c_str(), m_color, m_line_width);//function not working correctly
    else*/
    SDL_Surface* textSurface = TTF_RenderText_Blended( m_font, text.c_str(), m_color);

    m_text_texture.reset( SDL_CreateTextureFromSurface( m_main_pointers->screen_renderer.get(), textSurface ));

    if( m_text_texture == nullptr )
    {
        std::cout<< "Unable to create texture from rendered text! SDL Error: "<< SDL_GetError()<<std::endl;
    }
    else
    {
        //TTF_SizeText(m_font, text.c_str(), &m_transform.m_rect.w, &m_transform.m_rect.h);
        //Get image dimensions
        m_transform.m_rect.w = textSurface->w;
        m_transform.m_rect.h = textSurface->h;
    }
    //Get rid of old surface
    SDL_FreeSurface( textSurface );
}

void Text::AppendString(const std::string& text )
{
    this->SetString(m_text + text);
}
