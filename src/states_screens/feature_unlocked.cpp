
#include "io/file_manager.hpp"
#include "guiengine/engine.hpp"
#include "states_screens/feature_unlocked.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"

#include <SColor.h>

static const char* CUTSCENE_NAME = "feature_unlocked";
static FeatureUnlockedCutScene* singleton = NULL;

void FeatureUnlockedCutScene::show()
{
    if (singleton == NULL)
    {
        singleton = new FeatureUnlockedCutScene();
        addCutScene(singleton);
    }
    
    singleton->prepare();
    
    StateManager::get()->pushCutScene(CUTSCENE_NAME);
}


FeatureUnlockedCutScene::FeatureUnlockedCutScene() : CutScene(CUTSCENE_NAME)
{
}

void FeatureUnlockedCutScene::prepare()
{
    angle = 0.0f;
    
    sky = irr_driver->addSkyDome(file_manager->getTextureFile("lscales.png"), 16 /* hori_res */, 16 /* vert_res */,
                           1.0f /* texture_percent */,  2.0f /* sphere_percent */);
    
    camera = irr_driver->addCamera();
    
    /*
    for (unsigned int i=0; i<sky->getMaterialCount(); i++)
    {
        video::SMaterial &irrMaterial = sky->getMaterial(i);
        for (unsigned int j=0; j<video::MATERIAL_MAX_TEXTURES; j++)
        {
            video::ITexture* t = irrMaterial.getTexture(j);
            if(!t) continue;
            core::matrix4 *m = &irrMaterial.getTextureMatrix(j);
            m_animated_textures.push_back(new MovingTexture(m, 0.05f, 0.0f));
        }   // for j<MATERIAL_MAX_TEXTURES
    }   // for i<getMaterialCount
     */
}
void FeatureUnlockedCutScene::terminate()
{
    printf("+++++++ FeatureUnlockedCutScene:Terminate +++++++++\n");
    
    irr_driver->removeNode(sky);
    sky = NULL;
    
    irr_driver->removeCamera(camera);
    camera = NULL;
}

void FeatureUnlockedCutScene::onUpdate(float dt, irr::video::IVideoDriver* driver)
{
    angle += dt*2;
    if (angle > 360) angle -= 360;
    sky->setRotation( core::vector3df(0, angle, 0) );

    const int w = irr_driver->getFrameSize().Width;
    const int h = irr_driver->getFrameSize().Height;
    const irr::video::SColor color(255, 255, 0 ,0);
    
    static int test_y = 0;
    static int test_speed = 1;
    test_y += test_speed;
    if (test_y > h) test_speed = -1;
    else if (test_y < 0) test_speed = 1;
    
    GUIEngine::getFont()->draw(_("Feature Unlocked"),
                               core::rect< s32 >( 0, test_y, w, h/10 ),
                               color,
                               true/* center h */, true /* center v */ );
}

