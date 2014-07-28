#ifndef STKBILLBOARD_HPP
#define STKBILLBOARD_HPP

#include "../lib/irrlicht/source/Irrlicht/CBillboardSceneNode.h"
#include <IBillboardSceneNode.h>
#include <irrTypes.h>
#include "utils/cpp2011.h"

class STKBillboard : public irr::scene::CBillboardSceneNode
{
public:
    STKBillboard(irr::scene::ISceneNode* parent, irr::scene::ISceneManager* mgr, irr::s32 id,
        const irr::core::vector3df& position, const irr::core::dimension2d<irr::f32>& size,
        irr::video::SColor colorTop = irr::video::SColor(0xFFFFFFFF),
        irr::video::SColor colorBottom = irr::video::SColor(0xFFFFFFFF));

    virtual void OnRegisterSceneNode() OVERRIDE;

    virtual void render() OVERRIDE;
};

#endif
