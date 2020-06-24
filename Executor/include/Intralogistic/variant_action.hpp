#pragma once

#include <behaviortree_cpp_v3/behavior_tree.h>
#include <zmq.hpp>

class VariantAction: public BT::ActionNodeBase
{
public:
    VariantAction(const std::string& instance_name,
      const BT::NodeConfiguration& config, const char *ip, 
      zmq::context_t& context);

    ~VariantAction();

    BT::NodeStatus tick() override;

    void halt() override {}

private:
    zmq::socket_t  _request_socket;
    zmq::socket_t  _reply_socket;
    unsigned        _current_uid;
    BT::NodeConfiguration _current_params;
    char _request_address[100];
    char _reply_address[100];
    std::string bb_key_;
    std::string bb_value_;

    BT::NodeStatus parseAnswer(const std::string& result_value);

    std::string generateRequest();
};