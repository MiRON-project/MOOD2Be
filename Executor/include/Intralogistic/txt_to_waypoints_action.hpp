#pragma once

#include <behaviortree_cpp_v3/behavior_tree.h>
#include <cstdlib>
#include <fstream>

class TxtToWaypointsAction : public BT::SyncActionNode
{
public:
  TxtToWaypointsAction(const std::string& instance_name,
      const BT::NodeConfiguration& config);

  ~TxtToWaypointsAction() = default;

  BT::NodeStatus tick() override;

  static BT::PortsList providedPorts()
  {
    return { BT::InputPort<std::string>("goal_path"),
        BT::OutputPort<double>("w_x"),
        BT::OutputPort<double>("w_y")};
  }

private:
  bool read_parameter;
  std::deque<std::pair<double, double>> waypoints;
  void parsePath(std::string& _waypoints);
};