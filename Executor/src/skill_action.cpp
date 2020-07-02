#include <fstream>
#include <behaviortree_cpp_v3/bt_factory.h>
#include "Intralogistic/skill_action.hpp"
#include "Intralogistic/json.hpp"

SkillAction::SkillAction(SkillDefinition definition,
                         const std::string &instance_name,
                         const BT::NodeConfiguration &config,
                         const char *ip,
                         zmq::context_t &context) : 
  BT::CoroActionNode(instance_name, config),
  _definition(std::move(definition)),
  _request_socket(context, ZMQ_REQ),
  _reply_socket(context, ZMQ_SUB),
  _current_uid(0)
{

  sprintf(_request_address, "tcp://%s:5557", ip);
  _request_socket.connect(_request_address);
  int timeout_ms = 1000;
  int linger_ms = 0;
  _request_socket.setsockopt(ZMQ_SNDTIMEO, timeout_ms);
  _request_socket.setsockopt(ZMQ_RCVTIMEO, timeout_ms);
  _request_socket.setsockopt(ZMQ_LINGER, linger_ms);

  timeout_ms = 1;
  sprintf(_reply_address, "tcp://%s:5558", ip);
  _reply_socket.connect(_reply_address);
  _reply_socket.setsockopt(ZMQ_SUBSCRIBE, "", 0);
  _reply_socket.setsockopt(ZMQ_RCVTIMEO, timeout_ms);
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
  zmq::message_t zmq_request_msg(request_msg.size());
  memcpy(zmq_request_msg.data(), request_msg.c_str(), request_msg.size());

  //std::cout << request_msg << std::endl;

  if (!_request_socket.send(zmq_request_msg))
  {
    std::cout << "SkillAction send Fails\n";
    return BT::NodeStatus::FAILURE;
  }

  zmq::message_t ack;
  if (!_request_socket.recv(&ack))
  {
    std::cout << "SkillAction ack Fails\n";
    return BT::NodeStatus::FAILURE;
  }

  BT::NodeStatus reply_status = BT::NodeStatus::IDLE;
  while (reply_status == BT::NodeStatus::IDLE)
  {
    zmq::message_t reply;
    if (!_reply_socket.recv(&reply)) {
      setStatusRunningAndYield();
      continue;
    }

    std::string reply_json(static_cast<const char *>(reply.data()), 
      reply.size());
    reply_status = convertResultToStatus(reply_json);
  }
  return reply_status;
}

BT::NodeStatus SkillAction::convertResultToStatus(const std::string &result_string)
{
  nlohmann::json json = nlohmann::json::parse(result_string);

  unsigned msg_id = json["id"];
  if(msg_id != _current_uid)
  {
    return BT::NodeStatus::IDLE;
  }
  std::string msg_type = json["msg-type"];

  if (msg_type != "skill-result")
    return BT::NodeStatus::IDLE;

  std::string result = json["result"]["result"];
  auto json_res_value = json["result"]["result-value"];

  if (json_res_value.is_string())
  {
    const std::string bb_result_key = _definition.ID + "::last_result";
    std::string res_value = json_res_value;

    //   TODO. What should I do with this string?
    //        blackboard()->set( bb_result_key, res_value );
  }

  std::transform(result.begin(), result.end(), result.begin(), ::toupper);
  if (result == "SUCCESS")
  {
    return BT::NodeStatus::SUCCESS;
  }
  else if (result == "ERROR" || result == "ABORT")
  {
    return BT::NodeStatus::FAILURE;
  }
  else
  {
    std::cout << "Error parsing this reply:\n"
              << result << std::endl;
    throw std::runtime_error("Unrecognized [result]");
  }
}

std::string SkillAction::generateRequest()
{
  nlohmann::json json;
  json["msg-type"] = "push-skill";
  json["id"] = _current_uid;

  auto &skill = json["skill"];
  skill["name"] = _definition.ID;
  skill["skill-definition-fqn"] = _definition.skill_FQN;
  auto &out_attribute = skill["out-attribute"] = nlohmann::json({});
  auto &in_attribute = skill["in-attribute"] = nlohmann::json({});

  // read all the inputs
  for (const auto &port_pair : _definition.ports)
  {
    if (port_pair.second.direction() == BT::PortDirection::INPUT)
    {
      const auto &key = port_pair.first;
      const BT::PortInfo &info = port_pair.second;
      auto type = info.type();

      if (*type == typeid(std::string))
      {
        auto val = getInput<std::string>(key);
        if (!val)
        {
          throw std::runtime_error("Invalid parameter at key: " + key);
        }
        in_attribute.push_back({key, val.value()});
      }
      else if (*type == typeid(double))
      {
        auto val = getInput<double>(key);
        if (!val)
        {
          throw std::runtime_error("Invalid parameter at key: " + key);
        }
        in_attribute.push_back({key, val.value()});
      }
      else if (*type == typeid(int))
      {
        auto val = getInput<int>(key);
        if (!val)
        {
          throw std::runtime_error("Invalid parameter at key: " + key);
        }
        in_attribute.push_back({key, val.value()});
      }
      else if (*type == typeid(bool))
      {
        auto val = getInput<bool>(key);
        if (!val)
        {
          throw std::runtime_error("Invalid parameter at key: " + key);
        }
        in_attribute.push_back({key, val.value()});
      }
      else
        throw std::runtime_error("Invalid parameter at key: " + key);
    }
  }

  // setting all the outputs
  for (const auto &port_pair : _definition.ports)
  {
    if (port_pair.second.direction() == BT::PortDirection::OUTPUT)
    {
      const auto &key = port_pair.first;
      const BT::PortInfo &info = port_pair.second;
      auto type = info.type();

      if (*type == typeid(std::string))
        out_attribute.push_back({key, ""});
      else if (*type == typeid(double))
        out_attribute.push_back({key, 0.0});
      else if (*type == typeid(int))
        out_attribute.push_back({key, 0});
      else if (*type == typeid(bool))
        out_attribute.push_back({key, true});
      else
        throw std::runtime_error("Invalid parameter at key: " + key);
    }
  }
  return json.dump(1);
}
