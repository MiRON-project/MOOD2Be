#include <fstream>
#include <behaviortree_cpp/bt_factory.h>
#include "Intralogistic/skill_interface.hpp"

SkillAction::SkillAction(SkillDefinition definition,
                         const std::string &instance_name,
                         const BT::NodeParameters &params,
                         zmq::socket_t *zmq_socket):
    BT::ActionNodeBase(instance_name, params),
    _definition(std::move(definition)),
    _socket(zmq_socket),
    _current_uid(0)
{

}

BT::NodeStatus SkillAction::tick()
{
    // read node parameters
    for(const auto& param: initializationParameters() )
    {
        const auto& key = param.first;
        _current_params[key] = getParam<std::string>(key).value();
    }

    // create the message
    _current_uid = GetUID();
    std::string msg = GenerateRequest(_definition, _current_uid, _current_params, 1);

    // send the message
    //TODO
    // wait reply
    //TODO

    std::string reply_value; //TODO

    return convertResultToStatus(reply_value);
}

BT::NodeStatus SkillAction::convertResultToStatus(const std::string &result_value)
{
    const std::string bb_result_key = _definition.ID + "::last_result";

    // convert result value in NodeStatus
    for(const auto& result: _definition.possible_results)
    {
        if( result.value == result_value)
        {
            blackboard()->set( bb_result_key, result_value );
            return result.res;
        }
    }
    // result.value did not match exactly anyone in definition.possible_results
    // this policy (throw) may chnge in the future
    throw std::runtime_error( "Result received is not recognized as a valid one");
}
