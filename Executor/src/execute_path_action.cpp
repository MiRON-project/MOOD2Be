#include "Intralogistic/execute_path_action.hpp"

ExecutePathAction::ExecutePathAction(const std::string& instance_name,
    const BT::NodeConfiguration& config) :
  SyncActionNode(instance_name, config),
  read_parameter(false) {}

BT::NodeStatus ExecutePathAction::tick() {
  if (!read_parameter) {
    std::string waypoints_str;
    if(!getInput("waypoints", waypoints_str))
    {
      throw BT::RuntimeError("Missing parameter [waypoints]");
    }
    parseWaypoints(waypoints_str);
    read_parameter = true;
  }

  if (!waypoints.empty()) {
    auto xy = waypoints.front();
    waypoints.pop_front();
    setOutput("w_x", xy.first);
    setOutput("w_y", xy.second);
    if (waypoints.empty()) {
      read_parameter = false;
    }
    return BT::NodeStatus::SUCCESS;
  }
  return BT::NodeStatus::SUCCESS;
}

void ExecutePathAction::parseWaypoints(std::string& _waypoints) {
  _waypoints.erase(std::remove_if(_waypoints.begin(), _waypoints.end(), 
    [](char ch)
    {
      return ch=='(' || ch ==')'; 
    }), _waypoints.end() );

  std::vector<double> result; 
	std::istringstream iss(_waypoints); 
	for(std::string s; iss >> s; ) 
		result.push_back(std::stod(s)); 
  if (result.size() % 2)
    throw BT::LogicError("Waypoints must be pairs x, y");
  
  waypoints.clear();
  for (size_t i = 0; i < result.size(); i+=2 ) {
    waypoints.push_back(std::make_pair(result[i]*1000, -result[i+1]*1000));
  }
}

