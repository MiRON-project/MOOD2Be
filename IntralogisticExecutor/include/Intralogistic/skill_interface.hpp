#ifndef SKILL_INTERFACE_HPP
#define SKILL_INTERFACE_HPP

#include <behaviortree_cpp/behavior_tree.h>
#include <zmq.hpp>

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


class SkillAction: public BT::ActionNodeBase
{
public:
    SkillAction(SkillDefinition definition,
                const std::string& instance_name,
                const BT::NodeParameters& params,
                zmq::socket_t* zmq_socket );

    BT::NodeStatus tick() override;

private:
    SkillDefinition _definition;
    zmq::socket_t* _socket;
    unsigned _current_uid;
    BT::NodeParameters _current_params;

    BT::NodeStatus convertResultToStatus(const std::string& result_value);
};

inline unsigned GetUID()
{
    static unsigned count = 0;
    return ++count;
}

std::string GenerateRequest(const SkillDefinition& definition, unsigned msg_uid ,
                            const BT::NodeParameters &current_params, int indent = -1);

#endif // SKILL_INTERFACE_HPP
