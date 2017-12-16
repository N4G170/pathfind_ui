#ifndef TEXT_H
#define TEXT_H

#include <memory>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <mutex>

#include "structs.hpp"

/**
 * \brief Class to render text on screen
 */
class Text
{
    public:
        /** Default constructor */
        Text(){};

        /** Copy constructor */
        Text(const Text& t)
        {
            m_main_pointers = t.m_main_pointers;
            m_font = t.m_font;

            m_color = t.m_color;

            m_transform.m_parent = t.m_transform.m_parent;
            m_transform.m_rect.x = t.m_transform.m_rect.x;
            m_transform.m_rect.y = t.m_transform.m_rect.y;

            m_multiline = t.m_multiline;
            m_line_width = t.m_line_width;

            this->SetString(t.m_text);//needs to be called after all render related configurations
        }

        //"Main" Text constructor
        Text(MainPointers& main_pointers, int position_x, int position_y, std::string text)
        {
            m_main_pointers = &main_pointers;
            m_font = main_pointers.main_font.get();
            m_color = {0,0,0};

            m_transform.m_rect.x = position_x;
            m_transform.m_rect.y = position_y;

            this->SetString(text);//needs to be called after all render related configurations
        }

        /** Copy constructor */
        Text& operator= (const Text& t)
        {
            m_main_pointers = t.m_main_pointers;
            m_font = t.m_font;

            m_color = t.m_color;

            m_transform.m_parent = t.m_transform.m_parent;
            m_transform.m_rect.x = t.m_transform.m_rect.x;
            m_transform.m_rect.y = t.m_transform.m_rect.y;

            m_multiline = t.m_multiline;
            m_line_width = t.m_line_width;

            SetString(t.m_text);//needs to be called after all render related configurations

            return *this;
        }

        /** Default destructor */
        virtual ~Text()
        {
            m_main_pointers = nullptr;
        }

        /**
         * \brief Returns the std::string with the text being rendered
         */
        std::string GetText() { return m_text; };
        /**
         * \brief Change the current string being rendered to the new one
         */
        void SetString(const std::string& text);
        /**
         * \brief Appends the new string to the one being rendered
         */

         void SetPosition(int position_x, int position_y);
         /**
          * \brief Appends the new string to the one being rendered
          */
        void AppendString(const std::string& text);
        /**
         * \brief Sets text rendering color (Not yet in use)
         */
        void SetColor(const  SDL_Color& color);

        /**
         * \brief Renders the text
         */
        virtual void Render(std::unique_ptr<SDL_Renderer, Deleters>& screen_renderer)
        {
            std::lock_guard<std::mutex> lock(m_string_mutex);
            
            if(m_text_texture)
            {
                //Set rendering space and render to screen
                int x = m_transform.m_rect.x;
                int y = m_transform.m_rect.y;

                SDL_Rect renderQuad = { x, y, m_transform.m_rect.w, m_transform.m_rect.h };//texture position
                SDL_RenderCopy( screen_renderer.get(), m_text_texture.get(), NULL, &renderQuad );
            }
        }

    protected:

    private:
        /**
         * \brief Texture with the text to be rendered
         */
        std::unique_ptr<SDL_Texture, Deleters> m_text_texture;
        /**
         * \brief Pointer to the "global" unique_ptrs
         */
        MainPointers* m_main_pointers;

        /**
         * \brief The std::string being rendered
         */
        std::string m_text;

        /**
         * \brief Color of the text
         */
        SDL_Color m_color;

        /**
         * \brief Pointer to the TTF font being used
         */
        TTF_Font* m_font;

        /**
         * \see Trasnform
         */
        Transform m_transform;

        /**
         * \brief Is the text multiline (currently not working because the sdl_ttf wrap function is not working correctly)
         */
        bool m_multiline;
        /**
         * \see m_multiline
         */
        unsigned int m_line_width;

        std::mutex m_string_mutex;
};



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

#endif //TEXT_H
