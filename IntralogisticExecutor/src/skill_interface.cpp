#include <fstream>
#include <behaviortree_cpp/bt_factory.h>
#include "Intralogistic/skill_interface.hpp"

SkillAction::SkillAction(SkillDefinition definition,
                         const std::string &instance_name,
                         const BT::NodeParameters &params,
                         const char* ip,
                         zmq::context_t& context):
    BT::ActionNodeBase(instance_name, params),
    _definition(std::move(definition)),
    _request_socket( context, ZMQ_REQ ),
    _reply_socket( context, ZMQ_SUB  ),
    _current_uid(0)
{
    char address[100];
    sprintf(address, "tcp://%s:5557", ip );
    _request_socket.connect( address );

    sprintf(address, "tcp://%s:5558", ip );
    _reply_socket.connect( address );
    _reply_socket.setsockopt(ZMQ_SUBSCRIBE, "", 0);
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
    std::string request_msg = GenerateRequest(_definition, _current_uid, _current_params, 1);

    // send the message
    zmq::message_t zmq_request_msg( request_msg.size() );
    memcpy( zmq_request_msg.data(), request_msg.c_str(), request_msg.size() );

    _request_socket.send( zmq_request_msg );
    //TODO timeout
    std::cout << "message sent" << std::endl;

    zmq::message_t ack;
    _request_socket.recv( &ack );

    std::cout << "ack received" << std::endl;

    // wait reply
    //TODO
    zmq::message_t reply;
    std::cout << "wait reply" << std::endl;
    _reply_socket.recv( &reply );

    std::string reply_value( static_cast<const char*>(reply.data()), reply.size() );

  /*  { "msg-type" : "skill-result" ,
        "id" : 1 ,
        "result" : {"result" : "SUCCESS", "result-value" : "OK" }
      }*/

    std::cout << "REPLY:\n" <<reply_value << std::endl;

 //   return convertResultToStatus(reply_value);
    return BT::NodeStatus::RUNNING;
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
    // this policy (throw) may change in the future
    throw std::runtime_error( "Result received is not recognized as a valid one");
}
