#pragma once

#include <behaviortree_cpp_v3/behavior_tree.h>

class CheckPositiveCondition : public BT::SimpleConditionNode
{
public:
  CheckPositiveCondition(const std::string& instance_name,
      const BT::NodeConfiguration& config);

  ~CheckPositiveCondition() = default;

  static BT::PortsList providedPorts()
  {
    return { BT::InputPort<int>("value")};
  }

private:
  int  value_;
  BT::NodeStatus CheckPositive();
};