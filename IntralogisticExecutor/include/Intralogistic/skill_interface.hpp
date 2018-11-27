#ifndef SKILL_INTERFACE_HPP
#define SKILL_INTERFACE_HPP

#include <behaviortree_cpp/behavior_tree.h>

struct SkillDefinition
{
    std::string ID;
    std::string skill_FQN;
    BT::NodeParameters params;

    struct ReturnValue{
        BT::NodeStatus res;
        std::string value;
    };
    std::vector<ReturnValue> possible_results;
};

std::vector<SkillDefinition> ParseSkillFile( const std::string& filename );


#endif // SKILL_INTERFACE_HPP
