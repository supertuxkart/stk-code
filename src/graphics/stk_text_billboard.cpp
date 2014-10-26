#include "graphics/stk_text_billboard.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/shaders.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/stkbillboard.hpp"
#include "graphics/stkmeshscenenode.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "glwrap.hpp"
#include <SMesh.h>
#include <SMeshBuffer.h>
#include <ISceneManager.h>
#include <ICameraSceneNode.h>

using namespace irr;

STKTextBillboard::STKTextBillboard(core::stringw text, gui::ScalableFont* font,
    const video::SColor& color_top, const video::SColor& color_bottom,
    irr::scene::ISceneNode* parent,
    irr::scene::ISceneManager* mgr, irr::s32 id,
    const irr::core::vector3df& position, const irr::core::vector3df& size) :
    STKMeshSceneNode(new scene::SMesh(),
        parent, irr_driver->getSceneManager(), -1, "text_billboard",
        position, core::vector3df(0.0f, 0.0f, 0.0f), size, false)
{
    m_color_top = color_top;
    m_color_bottom = color_bottom;
    getTextMesh(text, font);
    createGLMeshes();
    Mesh->drop();
    //setAutomaticCulling(0);
    updateAbsolutePosition();
}

void STKTextBillboard::updateAbsolutePosition()
{
    if (Parent)
    {
        // Override to not use the parent's rotation
        AbsoluteTransformation = getRelativeTransformation();
        AbsoluteTransformation.setTranslation(AbsoluteTransformation.getTranslation() + Parent->getAbsolutePosition());
    }
    else
        AbsoluteTransformation = getRelativeTransformation();
}

scene::IMesh* STKTextBillboard::getTextMesh(core::stringw text, gui::ScalableFont* font)
{
    font->doDraw(text, core::rect<s32>(0, 0, 1000, 1000), video::SColor(255,255,255,255),
        false, false, NULL, this);

    const float scale = 0.018f;

    //scene::SMesh* mesh = new scene::SMesh();
    std::map<video::ITexture*, scene::SMeshBuffer*> buffers;

    int max_x = 0;
    int min_y = 0;
    int max_y = 0;
    for (unsigned int i = 0; i < m_chars.size(); i++)
    {
        int char_x = m_chars[i].m_destRect.LowerRightCorner.X;
        if (char_x > max_x)
            max_x = char_x;

        int char_min_y = m_chars[i].m_destRect.UpperLeftCorner.Y;
        int char_max_y = m_chars[i].m_destRect.LowerRightCorner.Y;
        if (char_min_y < min_y)
            min_y = char_min_y;
        if (char_max_y > min_y)
            max_y = char_max_y;
    }
    float scaled_center_x = (max_x / 2.0f) * scale;
    float scaled_y = (max_y / 2.0f) * scale; // -max_y * scale;

    for (unsigned int i = 0; i < m_chars.size(); i++)
    {
        core::vector3df char_pos((float) m_chars[i].m_destRect.UpperLeftCorner.X,
            (float) m_chars[i].m_destRect.UpperLeftCorner.Y, 0);
        char_pos *= scale;

        core::vector3df char_pos2((float)m_chars[i].m_destRect.LowerRightCorner.X,
            (float) m_chars[i].m_destRect.LowerRightCorner.Y, 0);
        char_pos2 *= scale;

        core::dimension2di char_size_i = m_chars[i].m_destRect.getSize();
        core::dimension2df char_size(char_size_i.Width*scale, char_size_i.Height*scale);

        std::map<video::ITexture*, scene::SMeshBuffer*>::iterator map_itr = buffers.find(m_chars[i].m_texture);
        scene::SMeshBuffer* buffer;
        if (map_itr == buffers.end())
        {
            buffer = new scene::SMeshBuffer();
            buffer->getMaterial().setTexture(0, m_chars[i].m_texture);
            buffer->getMaterial().setTexture(1, getUnicolorTexture(video::SColor(0, 0, 0, 0)));
            buffer->getMaterial().MaterialType = irr_driver->getShader(ES_OBJECT_UNLIT);
            buffers[m_chars[i].m_texture] = buffer;
        }
        else
        {
            buffer = map_itr->second;
        }

        float tex_width = (float) m_chars[i].m_texture->getSize().Width;
        float tex_height = (float)m_chars[i].m_texture->getSize().Height;


        video::S3DVertex vertices[] =
        {
            video::S3DVertex(char_pos.X - scaled_center_x, char_pos.Y - scaled_y, 0.0f,
                0.0f, 0.0f, 1.0f,
                m_color_bottom,
                m_chars[i].m_sourceRect.UpperLeftCorner.X / tex_width,
                m_chars[i].m_sourceRect.LowerRightCorner.Y / tex_height),

            video::S3DVertex(char_pos2.X - scaled_center_x, char_pos.Y - scaled_y, 0.0f,
                0.0f, 0.0f, 1.0f,
                m_color_bottom,
                m_chars[i].m_sourceRect.LowerRightCorner.X / tex_width,
                m_chars[i].m_sourceRect.LowerRightCorner.Y / tex_height),

            video::S3DVertex(char_pos2.X - scaled_center_x, char_pos2.Y - scaled_y, 0.0f,
                0.0f, 0.0f, 1.0f,
                m_color_top,
                m_chars[i].m_sourceRect.LowerRightCorner.X / tex_width,
                m_chars[i].m_sourceRect.UpperLeftCorner.Y / tex_height),

            video::S3DVertex(char_pos.X - scaled_center_x, char_pos2.Y - scaled_y, 0.0f,
                0.0f, 0.0f, 1.0f,
                m_color_top,
                m_chars[i].m_sourceRect.UpperLeftCorner.X / tex_width,
                m_chars[i].m_sourceRect.UpperLeftCorner.Y / tex_height)
        };

        irr::u16 indices[] = { 2, 1, 0, 3, 2, 0 };

        buffer->append(vertices, 4, indices, 6);
    }

    for (std::map<video::ITexture*, scene::SMeshBuffer*>::iterator map_itr = buffers.begin();
        map_itr != buffers.end(); map_itr++)
    {
        ((scene::SMesh*)Mesh)->addMeshBuffer(map_itr->second);

        map_itr->second->recalculateBoundingBox();
        Mesh->setBoundingBox(map_itr->second->getBoundingBox()); // TODO: wrong if several buffers

        map_itr->second->drop();
    }

    getMaterial(0).MaterialType = irr_driver->getShader(ES_OBJECT_UNLIT);

    return Mesh;
}

void STKTextBillboard::updateNoGL()
{
    scene::ICameraSceneNode* curr_cam = irr_driver->getSceneManager()->getActiveCamera();
    core::vector3df cam_pos = curr_cam->getPosition();
    core::vector3df text_pos = this->getAbsolutePosition();
    float angle = atan2(text_pos.X - cam_pos.X, text_pos.Z - cam_pos.Z);
    this->setRotation(core::vector3df(0.0f, angle * 180.0f / M_PI, 0.0f));
    updateAbsolutePosition();

    STKMeshSceneNode::updateNoGL();
}

void STKTextBillboard::collectChar(video::ITexture* texture,
    const core::rect<s32>& destRect,
    const core::rect<s32>& sourceRect,
    const video::SColor* const colors)
{
    m_chars.push_back(STKTextBillboardChar(texture, destRect, sourceRect, colors));
}