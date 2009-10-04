#ifndef HEADER_FEATURE_UNLOCKED_HPP
#define HEADER_FEATURE_UNLOCKED_HPP

#include "guiengine/cutscene.hpp"

namespace irr { namespace scene { class ISceneNode; class ICameraSceneNode; } }

class FeatureUnlockedCutScene : public GUIEngine::CutScene
{
    FeatureUnlockedCutScene();
    void prepare();
    
    float angle;
    irr::scene::ISceneNode* sky;
    irr::scene::ICameraSceneNode* camera;
public:
    static void show();
  
    void onUpdate(float dt, irr::video::IVideoDriver*);
    void terminate();
};

#endif

