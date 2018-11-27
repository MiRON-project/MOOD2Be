#include <fstream>
#include <behaviortree_cpp/bt_factory.h>
#include "Intralogistic/skill_interface.hpp"


void RegisterSkill(BT::BehaviorTreeFactory& factory,
                   const SkillDefinition& definition
                   )
{
    using namespace BT;
    SimpleActionNode::TickFunctor tick = [definition](TreeNode& self) -> NodeStatus
    {
        // read node parameters
        // create the message
        // send the message
        // wait reply

        std::string reply_value; //TODO

        const std::string bb_result_key = definition.ID + "::last_result";

        // convert result value in NodeStatus
        for(const auto& result: definition.possible_results)
        {
            if( result.value == reply_value)
            {
                self.blackboard()->set( bb_result_key, reply_value );
                return result.res;
            }
        }

        // result.value did not match exactly anyone in definition.possible_results
        // this policy (throw) may chnge in the future
        throw std::runtime_error( "Result received is not recognized as a valid one");
    };

    factory.registerSimpleAction(definition.ID, tick);
}
