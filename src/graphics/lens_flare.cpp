/* TODO: copyright */

#include "graphics/lens_flare.hpp"
#include "IVideoDriver.h"
#include "ISceneManager.h"
#include "ISceneCollisionManager.h"

namespace irr
{
namespace scene
{

CLensFlareSceneNode::CLensFlareSceneNode(ISceneNode* parent, ISceneManager* mgr, s32 id) :
    scene::ISceneNode(parent, mgr, id)
{
    #ifdef _DEBUG
    setDebugName("CLensFlareSceneNode");
    #endif

    // set the bounding box
    BBox.MaxEdge.set(0,0,0);
    BBox.MinEdge.set(0,0,0);

    // set the initial Strength
    Strength = 1.0f;

    // setup the vertices
    Vertices[0] = video::S3DVertex(-1.f, -1.f, 0.f, 0.f, 0.f, 1.f, 0xffffffff, 0.f, 1.f);
    Vertices[1] = video::S3DVertex(-1.f,  1.f, 0.f, 0.f, 0.f, 1.f, 0xffffffff, 0.f, 0.f);
    Vertices[2] = video::S3DVertex( 1.f,  1.f, 0.f, 0.f, 0.f, 1.f, 0xffffffff, 1.f, 0.f);
    Vertices[3] = video::S3DVertex( 1.f, -1.f, 0.f, 0.f, 0.f, 1.f, 0xffffffff, 1.f, 1.f);

    // setup the indices
    Indices[0] = 0;
    Indices[1] = 1;
    Indices[2] = 2;
    Indices[3] = 2;
    Indices[4] = 3;
    Indices[5] = 0;

    // set the default material properties
    Material.MaterialType = video::EMT_TRANSPARENT_ADD_COLOR;
    Material.Lighting = false;
    Material.ZBuffer = video::ECFN_NEVER;

    FlareData.reallocate(30);

    // prepare the flare data array
    // circles, halos and ring behind the sun
    FlareData.push_back(SFlareData(EFT_CIRCLE, 0.5f, 0.12f, video::SColor(120, 60, 180, 35)));
    FlareData.push_back(SFlareData(EFT_HALO, 0.45f, 0.4f, video::SColor(200, 100, 200, 60)));
    FlareData.push_back(SFlareData(EFT_CIRCLE, 0.4f, 0.17f, video::SColor(240, 120, 220, 40)));
    FlareData.push_back(SFlareData(EFT_CIRCLE, 0.2f, 0.35f, video::SColor(175, 175, 255, 20)));
    FlareData.push_back(SFlareData(EFT_RING, 0.15f, 0.2f, video::SColor(120, 60, 255, 100)));

    // sun and glow effect at sun position
    FlareData.push_back(SFlareData(EFT_SUN, 0.0f, 0.75f, video::SColor(255, 255, 255, 255)));
//    FlareData.push_back(SFlareData(EFT_STREAKS, 0.0f, 2.9f, video::SColor(255, 255, 255, 255)));
    FlareData.push_back(SFlareData(EFT_GLOW, 0.0f, 3.5f, video::SColor(255, 255, 255, 255)));
//    FlareData.push_back(SFlareData(EFT_RING, 0.0f, 1.5f, video::SColor(120, 120, 120, 150)));

    // some lenses, halos and circles
    FlareData.push_back(SFlareData(EFT_LENS, -0.15f, 0.15f, video::SColor(255, 60, 60, 90)));
    FlareData.push_back(SFlareData(EFT_HALO, -0.3f, 0.3f, video::SColor(120, 60, 255, 180)));
    FlareData.push_back(SFlareData(EFT_HALO, -0.4f, 0.2f, video::SColor(220, 80, 80, 98)));
    FlareData.push_back(SFlareData(EFT_CIRCLE, -0.45f, 0.1f, video::SColor(220, 80, 80, 85)));
    FlareData.push_back(SFlareData(EFT_RING, -0.42f, 0.3f, video::SColor(180, 60, 255, 110)));

    // some small lenses, halos and rings
    FlareData.push_back(SFlareData(EFT_LENS, -0.55f, 0.2f, video::SColor(255, 60, 60, 130)));
    FlareData.push_back(SFlareData(EFT_HALO, -0.6f, 0.3f, video::SColor(120, 60, 255, 80)));
    FlareData.push_back(SFlareData(EFT_LENS, -0.7f, 0.2f, video::SColor(200, 60, 60, 130)));
    FlareData.push_back(SFlareData(EFT_LENS, -0.71f, 0.2f, video::SColor(200, 60, 130, 60)));
    FlareData.push_back(SFlareData(EFT_LENS, -0.72f, 0.2f, video::SColor(200, 130, 130, 60)));
    FlareData.push_back(SFlareData(EFT_LENS, -0.74f, 0.2f, video::SColor(200, 130, 60, 60)));

    // some polyons, lenses and circle
    FlareData.push_back(SFlareData(scene::EFT_POLY, -0.79f, 0.2f, video::SColor(200, 60, 130, 60)));
    FlareData.push_back(SFlareData(scene::EFT_POLY, -0.86f, 0.3f, video::SColor(200, 130, 130, 60)));
    FlareData.push_back(SFlareData(scene::EFT_LENS, -0.87f, 0.3f, video::SColor(180,255,192,178)));
    FlareData.push_back(SFlareData(scene::EFT_CIRCLE, -0.9f, 0.1f, video::SColor(200, 60, 60, 130)));
    FlareData.push_back(SFlareData(scene::EFT_POLY, -0.93f, 0.4f, video::SColor(200, 130, 60, 60)));

    // finally som polygons
    FlareData.push_back(SFlareData(EFT_POLY, -0.95f, 0.6f, video::SColor(120, 60, 255, 120)));
    FlareData.push_back(SFlareData(EFT_POLY, -1.0f, 0.15f, video::SColor(120, 20, 255, 85)));
}

CLensFlareSceneNode::~CLensFlareSceneNode()
{
}

u32 CLensFlareSceneNode::getMaterialCount() const
{
    // return the material count (always one in our case)
    return 1;
}

video::SMaterial& CLensFlareSceneNode::getMaterial(u32 i)
{
    // return the material
    return Material;
}

core::array<SFlareData>& CLensFlareSceneNode::getFlareData()
{
    // return the flare data array
    return FlareData;
}

void CLensFlareSceneNode::OnRegisterSceneNode()
{
    // if node is visible and Strength is greater than 0 register it for the ESNRP_TRANSPARENT_EFFECT pass
    if(IsVisible && Strength > 0.0f)
    {
        SceneManager->registerNodeForRendering(this, scene::ESNRP_TRANSPARENT_EFFECT);
    }

    // call base OnRegisterSceneNode
    ISceneNode::OnRegisterSceneNode();
}

void CLensFlareSceneNode::render()
{
    // get the videodriver and the active camera
    video::IVideoDriver* driver = SceneManager->getVideoDriver();
    ICameraSceneNode* camera = SceneManager->getActiveCamera();

    // return if we don't have a valid driver or a valid camera
    // or if we have no texture attached to the material
    if (!camera || !driver || !Material.getTexture(0))
        return;

    // get screencenter
    const core::vector2d<s32> screenCenter = core::vector2d<s32>(
        SceneManager->getVideoDriver()->getScreenSize().Width,
        SceneManager->getVideoDriver()->getScreenSize().Height)/2;

    // get screencoordinates of the node
    const core::vector2d<s32> lightPos = SceneManager->getSceneCollisionManager()->getScreenCoordinatesFrom3DPosition(
        getAbsolutePosition(),
        camera);

    // store old projection matrix
    core::matrix4 oldProjMat = driver->getTransform(video::ETS_PROJECTION);

    // store old view matrix
    core::matrix4 oldViewMat = driver->getTransform(video::ETS_VIEW);

    // clear the projection matrix
    driver->setTransform(video::ETS_PROJECTION, core::IdentityMatrix);

    // clear the view matrix
    driver->setTransform(video::ETS_VIEW, core::IdentityMatrix);

    // set the transform
    driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);

    // set the material
    driver->setMaterial(Material);

    // calculate some handy constants
    const f32 texPos = 1.0f/EFT_COUNT;
    const s32 texHeight = s32(Material.getTexture(0)->getSize().Height*0.5f);
    const f32 screenWidth = f32(driver->getScreenSize().Width);
    const f32 screenHeight = f32(driver->getScreenSize().Height);

    // render the flares
    for (u32 i=0; i<FlareData.size(); ++i)
    {
        // get the flare element
        SFlareData& flare = FlareData[i];

        // calculate center of the flare
        core::vector2d<s32>flarePos = screenCenter.getInterpolated(lightPos, -2.0*flare.Position);

        // calculate flareposition in vertex coordinates using the scalefactor of the flare
        s32 flareScale = s32((texHeight*flare.Scale));
        core::rect<f32> flareRect = core::rect<f32>(
            -1.f + 2.f * f32(flarePos.X-flareScale) / screenWidth,
            -1.f + 2.f * f32(screenHeight-flarePos.Y-flareScale) / screenHeight,
            -1.f + 2.f * f32(flarePos.X+flareScale) / screenWidth,
            -1.f + 2.f * f32(screenHeight-flarePos.Y+flareScale) / screenHeight);

        // calculate flarecolor in dependence of occlusion
        f32 flareAlpha = f32(flare.Color.getAlpha()) / 255.f;
        video::SColor flareColor(255,
            (u32)(Strength * flareAlpha * flare.Color.getRed()),
            (u32)(Strength * flareAlpha * flare.Color.getGreen()),
            (u32)(Strength * flareAlpha * flare.Color.getBlue()));

        // set vertex colors
        Vertices[0].Color = flareColor;
        Vertices[1].Color = flareColor;
        Vertices[2].Color = flareColor;
        Vertices[3].Color = flareColor;

        // set texture coordinates
        Vertices[0].TCoords.set(    flare.Type * texPos, 1.0f);
        Vertices[1].TCoords.set(    flare.Type * texPos, 0.0f);
        Vertices[2].TCoords.set((flare.Type+1) * texPos, 0.0f);
        Vertices[3].TCoords.set((flare.Type+1) * texPos, 1.0f);

        // set vertex positions
        Vertices[0].Pos.set(flareRect.UpperLeftCorner.X, flareRect.UpperLeftCorner.Y, 0);
        Vertices[1].Pos.set(flareRect.UpperLeftCorner.X, flareRect.LowerRightCorner .Y, 0);
        Vertices[2].Pos.set(flareRect.LowerRightCorner.X, flareRect.LowerRightCorner.Y, 0);
        Vertices[3].Pos.set(flareRect.LowerRightCorner.X, flareRect.UpperLeftCorner.Y, 0);

        //draw the mesh
        driver->drawIndexedTriangleList(Vertices, 4, Indices, 2);
    }

    // restore view matrix
    driver->setTransform(video::ETS_VIEW, oldViewMat);

    // restore projection matrix
    driver->setTransform(video::ETS_PROJECTION, oldProjMat);
}

const core::aabbox3df& CLensFlareSceneNode::getBoundingBox() const
{
    // return the bounding box
    return BBox;
}

ESCENE_NODE_TYPE CLensFlareSceneNode::getType() const
{
    // return type of the scene node
    // (important when using with a custom scene node factory)
    return scene::ESNT_UNKNOWN; //(ESCENE_NODE_TYPE) ECSNT_LENSFLARE;
}

void CLensFlareSceneNode::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options) const
{
    // write attributes of the scene node.
    ISceneNode::serializeAttributes(out, options);
}

void CLensFlareSceneNode::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options)
{
    // read attributes of the scene node.
    ISceneNode::deserializeAttributes(in, options);
}

ISceneNode* CLensFlareSceneNode::clone(ISceneNode* newParent, ISceneManager* newManager)
{
    if (!newParent)
        newParent = Parent;
    if (!newManager)
        newManager = SceneManager;

    CLensFlareSceneNode* nb = new CLensFlareSceneNode(newParent, newManager, ID);

    nb->cloneMembers(this, newManager);
    nb->Material = Material;

    nb->drop();
    return nb;
}

}// end irr namespace
}// end scene namespace

