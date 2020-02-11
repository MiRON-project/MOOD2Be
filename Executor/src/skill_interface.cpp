#include <fstream>
#include <behaviortree_cpp_v3/bt_factory.h>
#include "Intralogistic/skill_interface.hpp"

SkillAction::SkillAction(SkillDefinition definition,
                         const std::string &instance_name,
                         const BT::NodeConfiguration &config,
                         const char* ip,
                         zmq::context_t& context):
    BT::ActionNodeBase(instance_name, config),
    _definition(std::move(definition)),
    _request_socket( context, ZMQ_REQ ),
    _reply_socket( context, ZMQ_SUB  ),
    _current_uid(0)
{

    sprintf(_request_address, "tcp://%s:5557", ip );
    _request_socket.connect( _request_address );

    int timeout_ms = 500;
    int linger_ms = 0;
    _request_socket.setsockopt(ZMQ_SNDTIMEO, timeout_ms);
    _request_socket.setsockopt(ZMQ_RCVTIMEO, timeout_ms);
    _request_socket.setsockopt(ZMQ_LINGER, linger_ms);

    sprintf( _reply_address, "tcp://%s:5558", ip );
    _reply_socket.connect( _reply_address );
    _reply_socket.setsockopt(ZMQ_SUBSCRIBE, "", 0);
}

SkillAction::~SkillAction()
{
    _request_socket.disconnect(_request_address);
    _reply_socket.disconnect(_reply_address);
}

BT::NodeStatus SkillAction::tick()
{
    // create the message
    _current_uid = GetUID();
    std::string request_msg = generateRequest();

    // send the message
    zmq::message_t zmq_request_msg( request_msg.size() );
    memcpy( zmq_request_msg.data(), request_msg.c_str(), request_msg.size() );

    //std::cout << request_msg << std::endl;

    if( !_request_socket.send( zmq_request_msg ) )
    {
       // timeout
       std::cout << "send TIMEOUT" << std::endl;
       return BT::NodeStatus::FAILURE;
    }
    std::cout << "message sent" << std::endl;

    zmq::message_t ack;
    if( !_request_socket.recv( &ack ) )
    {
        std::cout << "ack TIMEOUT" << std::endl;
        return BT::NodeStatus::FAILURE;
    }

    std::cout << "ack received" << std::endl;


    std::cout << "wait reply" << std::endl;
    BT::NodeStatus reply_status = BT::NodeStatus::IDLE;

    while( reply_status == BT::NodeStatus::IDLE )
    {
        zmq::message_t reply;
        _reply_socket.recv( &reply );

        std::string reply_json( static_cast<const char*>(reply.data()), reply.size() );
        reply_status = convertResultToStatus(reply_json);
    }

    return reply_status;
}




