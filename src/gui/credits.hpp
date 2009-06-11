#ifndef HEADER_CREDITS_HPP
#define HEADER_CREDITS_HPP

#include "irrlicht.h"
using namespace irr;

#include "utils/ptr_vector.hpp"

namespace GUIEngine
{
    class CreditsSection;
    
    class Credits
        {
            float m_time_element;
            
            ptr_vector<CreditsSection, HOLD> m_sections;
            CreditsSection* getCurrentSection();
            
            int x, y, w, h;
            core::rect< s32 > m_section_rect;
            
            int m_curr_section;
            int m_curr_element;
            
            float time_before_next_step;
            
        public:
            Credits();
            
            static Credits* getInstance();
            void setArea(const int x, const int y, const int w, const int h);
            
            // start from beginning again
            void reset();
            
            void render(const float elapsed_time);
        };
}
#endif
