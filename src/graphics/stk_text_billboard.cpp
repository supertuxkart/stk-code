#include "graphics/stk_text_billboard.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/shaders.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/stkbillboard.hpp"
#include "graphics/stkmeshscenenode.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"

#include <SMesh.h>
#include <SMeshBuffer.h>
#include <ISceneManager.h>

using namespace irr;


STKTextBillboard::STKTextBillboard(core::stringw text, gui::ScalableFont* font,
    const video::SColor& color, irr::scene::ISceneNode* parent,
    irr::scene::ISceneManager* mgr, irr::s32 id,
    const irr::core::vector3df& position, const irr::core::dimension2d<irr::f32>& size) : // TODO: use size or remove
    IBillboardSceneNode(parent, mgr, id, position),
    CBillboardSceneNode(parent, mgr, id, position, size, video::SColor(255, 255, 255, 255), video::SColor(255, 255, 255, 255))
{
    font->doDraw(text, core::rect<s32>(0, 0, 1000, 1000), color, false, false, NULL, this);

    const float scale = 0.05f;

    scene::SMesh* mesh = new scene::SMesh();
    std::map<video::ITexture*, scene::SMeshBuffer*> buffers;

    for (unsigned int i = 0; i < m_chars.size(); i++)
    {
        core::vector3df char_pos(m_chars[i].m_destRect.UpperLeftCorner.X,
            m_chars[i].m_destRect.UpperLeftCorner.Y, 0);
        char_pos *= scale;
        char_pos += position;

        core::vector3df char_pos2(m_chars[i].m_destRect.LowerRightCorner.X,
            m_chars[i].m_destRect.LowerRightCorner.Y, 0);
        char_pos2 *= scale;
        char_pos2 += position;

        core::dimension2di char_size_i = m_chars[i].m_destRect.getSize();
        core::dimension2df char_size(char_size_i.Width*scale, char_size_i.Height*scale);

        std::map<video::ITexture*, scene::SMeshBuffer*>::iterator map_itr = buffers.find(m_chars[i].m_texture);
        scene::SMeshBuffer* buffer;
        if (map_itr == buffers.end())
        {
            buffer = new scene::SMeshBuffer();
            buffer->getMaterial().setTexture(0, m_chars[i].m_texture);
            buffer->getMaterial().MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF;
            buffers[m_chars[i].m_texture] = buffer;
        }
        else
        {
            buffer = map_itr->second;
        }

        float tex_width = m_chars[i].m_texture->getSize().Width;
        float tex_height = m_chars[i].m_texture->getSize().Height;


        video::S3DVertex vertices[] =
        {
            video::S3DVertex(char_pos.X, char_pos.Y, 0.0f,
                0.0f, 0.0f, 1.0f,
                video::SColor(255, 255, 255, 255),
                m_chars[i].m_sourceRect.UpperLeftCorner.X / tex_width,
                m_chars[i].m_sourceRect.LowerRightCorner.Y / tex_height),

           video::S3DVertex(char_pos2.X, char_pos.Y, 0.0f,
                0.0f, 0.0f, 1.0f,
                video::SColor(255, 255, 255, 255),
                m_chars[i].m_sourceRect.LowerRightCorner.X / tex_width,
                m_chars[i].m_sourceRect.LowerRightCorner.Y / tex_height),

            video::S3DVertex(char_pos2.X, char_pos2.Y, 0.0f,
                0.0f, 0.0f, 1.0f,
                video::SColor(255, 255, 255, 255),
                m_chars[i].m_sourceRect.LowerRightCorner.X / tex_width,
                m_chars[i].m_sourceRect.UpperLeftCorner.Y / tex_height),

            video::S3DVertex(char_pos.X, char_pos2.Y, 0.0f,
                0.0f, 0.0f, 1.0f,
                video::SColor(255, 255, 255, 255),
                m_chars[i].m_sourceRect.UpperLeftCorner.X / tex_width,
                m_chars[i].m_sourceRect.UpperLeftCorner.Y / tex_height)
        };

        irr::u16 indices[] = { 2, 1, 0, 3, 2, 0 };

        buffer->append(vertices, 4, indices, 6);
    }

    for (std::map<video::ITexture*, scene::SMeshBuffer*>::iterator map_itr = buffers.begin();
        map_itr != buffers.end(); map_itr++)
    {
        mesh->addMeshBuffer(map_itr->second);
        map_itr->second->drop();
    }

    STKMeshSceneNode* stk_mesh = new STKMeshSceneNode(mesh,
        parent, irr_driver->getSceneManager(), -1,
        position, core::vector3df(0.0f, 0.0f, 0.0f), core::vector3df(1.0f, 1.0f, 1.0f));
    stk_mesh->setAutomaticCulling(0);
}

void STKTextBillboard::render()
{
}

void STKTextBillboard::collectChar(video::ITexture* texture,
    const core::rect<s32>& destRect,
    const core::rect<s32>& sourceRect,
    const video::SColor* const colors)
{
    m_chars.push_back(STKTextBillboardChar(texture, destRect, sourceRect, colors));
}