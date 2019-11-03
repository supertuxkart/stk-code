
#ifndef HEADER_ACHIEVEMENT_INFO_HPP
#define HEADER_ACHIEVEMENT_INFO_HPP

#include "utils/types.hpp"

#include <irrString.h>
#include <string>
#include <vector>

// ============================================================================

class Achievement;
class XMLNode;

/** This class stores an achievement definition from the xml file, including
 *  title, description, but also how to achieve this achievement.
 *  Contrast with the Achievement class, which is a player-specific instance
 *  tracking the progress of the achievement.
 * \ingroup achievements
 */
class TipSet
{
public:
    // The operations supported for a goal
    enum operationType {
        OP_NONE      = 0,
        OP_ADD       = 1,
        OP_SUBSTRACT = 2,
    };

    // We store goals in a recursive tree.
    // This structure matching the algorithms
    // we use to manipulate it simplify code.
    struct tipsTree
    {
        std::string m_texts;
        std::string m_icon_path;
        std::vector<tipsTree> children;
    };

private:
    /** The id of this tips. */
    std::string m_id;

    /** A secret achievement has its progress not shown. */
    bool m_is_secret;

    void parseGoals(const XMLNode * input, goalTree &parent);
    int  recursiveGoalCount(goalTree &parent);
    int  recursiveProgressCount(goalTree &parent);
    int  getRecursiveDepth(goalTree &parent);

    /** The tree storing all tips */
    tipsTree           m_goal_tree;

public:
             AchievementInfo(const XMLNode * input);
    virtual ~AchievementInfo() {};

    virtual irr::core::stringw goalString();
    virtual irr::core::stringw progressString();

    int                getProgressTarget()    { return recursiveProgressCount(m_goal_tree); }
    int                getGoalCount()         { return recursiveGoalCount(m_goal_tree); }
    int                getDepth()             { return getRecursiveDepth(m_goal_tree); }
    uint32_t           getID()          const { return m_id; }
    irr::core::stringw getDescription() const;
    irr::core::stringw getName()        const;
    bool               isSecret()       const { return m_is_secret; }

    // This function should not be called if copy already has children
    void copyGoalTree(goalTree &copy, goalTree &model, bool set_values_to_zero);
};   // class AchievementInfo


#endif

/*EOF*/
