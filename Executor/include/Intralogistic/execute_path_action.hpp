#pragma once

#include <behaviortree_cpp_v3/behavior_tree.h>

class ExecutePathAction : public BT::SyncActionNode
{
public:
  ExecutePathAction(const std::string& instance_name,
      const BT::NodeConfiguration& config);

  ~ExecutePathAction() = default;

  BT::NodeStatus tick() override;

  static BT::PortsList providedPorts()
  {
    return { BT::InputPort<std::string>("waypoints"),
        BT::OutputPort<double>("w_x"),
        BT::OutputPort<double>("w_y")};
  }

private:
  bool read_parameter;
  std::deque<std::pair<double, double>> waypoints;
  void parseWaypoints(std::string& _waypoints);
};