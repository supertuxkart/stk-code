#ifndef HEADER_FEATURE_UNLOCKED_HPP
#define HEADER_FEATURE_UNLOCKED_HPP

#include "guiengine/cutscene.hpp"

namespace irr { namespace scene { class ISceneNode; } }

class FeatureUnlockedCutScene : public GUIEngine::CutScene
{
    FeatureUnlockedCutScene();
    void prepare();
    
    irr::scene::ISceneNode* sky;
public:
    static void show();
  
    void onUpdate(float dt, irr::video::IVideoDriver*);
    void terminate();
};

#endif

