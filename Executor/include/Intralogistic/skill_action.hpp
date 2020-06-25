#ifndef SKILL_INTERFACE_HPP
#define SKILL_INTERFACE_HPP

#include <behaviortree_cpp_v3/behavior_tree.h>
#include <zmq.hpp>

struct SkillDefinition
{
  std::string ID;
  std::string skill_FQN;
  BT::PortsList ports;

  struct ReturnValue
  {
    BT::NodeStatus res;
    std::string value;
  };
  std::vector<ReturnValue> possible_results;
};

std::vector<SkillDefinition> ParseSkillFile(const std::string &filename);

class SkillAction : public BT::CoroActionNode
{
public:
  SkillAction(SkillDefinition definition,
              const std::string &instance_name,
              const BT::NodeConfiguration &config,
              const char *ip,
              zmq::context_t &context);

  ~SkillAction();

  BT::NodeStatus tick() override;

  void halt() override {
    CoroActionNode::halt();
  }

private:
  SkillDefinition _definition;
  zmq::socket_t _request_socket;
  zmq::socket_t _reply_socket;
  unsigned _current_uid;
  BT::NodeConfiguration _current_params;
  char _request_address[100];
  char _reply_address[100];

  BT::NodeStatus convertResultToStatus(const std::string &result_value);

  std::string generateRequest();
};

inline unsigned GetUID()
{
  static unsigned count = 0;
  return ++count;
}

#endif // SKILL_INTERFACE_HPP
