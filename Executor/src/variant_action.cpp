#include <fstream>
#include <behaviortree_cpp_v3/bt_factory.h>
#include "Intralogistic/skill_action.hpp"
#include "Intralogistic/variant_action.hpp"
#include "Intralogistic/json.hpp"

VariantAction::VariantAction(const std::string &instance_name,
    const BT::NodeConfiguration &config, const char* ip, 
    zmq::context_t& context):
  BT::CoroActionNode(instance_name, config),
  _request_socket( context, ZMQ_REQ ),
  _reply_socket( context, ZMQ_SUB  ),
  _current_uid(GetUID())
{
  sprintf(_request_address, "tcp://%s:8274", ip );
  _request_socket.connect( _request_address );

  int timeout_ms = 1000;
  int linger_ms = 0;
  _request_socket.setsockopt(ZMQ_SNDTIMEO, timeout_ms);
  _request_socket.setsockopt(ZMQ_RCVTIMEO, timeout_ms);
  _request_socket.setsockopt(ZMQ_LINGER, linger_ms);

  timeout_ms = 1;
  sprintf(_reply_address, "tcp://%s:8275", ip);
  _reply_socket.connect(_reply_address);
  _reply_socket.setsockopt(ZMQ_SUBSCRIBE, "", 0);
  _reply_socket.setsockopt(ZMQ_RCVTIMEO, timeout_ms);
}

VariantAction::~VariantAction()
{
  _request_socket.disconnect(_request_address);
  _reply_socket.disconnect(_reply_address);
}

BT::NodeStatus VariantAction::tick()
{
  // create the message
  std::string request_msg = generateRequest();

  // send the message
  zmq::message_t zmq_request_msg(request_msg.size());
  memcpy(zmq_request_msg.data(), request_msg.c_str(), request_msg.size() );

  if(!_request_socket.send(zmq_request_msg)) {
    std::cout << "VariantAction send Fails\n";
    return BT::NodeStatus::FAILURE;
  }

  zmq::message_t ack;
  if(!_request_socket.recv(&ack))
  {
    std::cout << "VariantAction ack Fails\n";
    return BT::NodeStatus::FAILURE;
  }

  BT::NodeStatus reply_status = BT::NodeStatus::IDLE;
  while(reply_status == BT::NodeStatus::IDLE)
  {
    zmq::message_t reply;
    if (!_reply_socket.recv(&reply)) {
      setStatusRunningAndYield();
      continue;
    }

    std::string reply_json(static_cast<const char*>(reply.data()), 
      reply.size());
    reply_status = parseAnswer(reply_json);
  }
  return reply_status;
}

std::string VariantAction::generateRequest()
{
    nlohmann::json json;
    json["msg-type"] = "variant";
    json["id"] = _current_uid;
    return json.dump(1);
}

BT::NodeStatus VariantAction::parseAnswer(const std::string &result_string)
{
  nlohmann::json json = nlohmann::json::parse(result_string);

  unsigned msg_id = json["id"];
  if( msg_id != _current_uid) {
    std::cout << "message id is: " << msg_id << ", and variant id is: " <<
      _current_uid << ". Variant is in IDLE STATE!\n";
    return  BT::NodeStatus::IDLE;
  }

  std::string msg_type  = json["msg-type"];
  if (msg_type != "variant") {
      std::cout << "message type is: " << msg_type 
        << ", and variant type is variant. Variant is in IDLE STATE!\n";
    return  BT::NodeStatus::IDLE;
  }

  auto json_res_value = json["result"]["value"];

  if(json_res_value.is_string()) 
    bb_value_ = json_res_value;
  else bb_value_ = "";
  
  if (bb_value_.empty()) 
    return BT::NodeStatus::IDLE;

  try {
    setOutput<std::string>("value", bb_value_);
    return BT::NodeStatus::SUCCESS;
  }
  catch(...) {
    return BT::NodeStatus::FAILURE;
  }
}




