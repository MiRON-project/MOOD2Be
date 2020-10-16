#include "Intralogistic/txt_to_waypoints_action.hpp"

TxtToWaypointsAction::TxtToWaypointsAction(const std::string& instance_name,
    const BT::NodeConfiguration& config) :
  SyncActionNode(instance_name, config),
  read_parameter(false) {}

BT::NodeStatus TxtToWaypointsAction::tick() {
  if (!read_parameter) {
    std::string waypoints_str;
    if(!getInput("goal_path", waypoints_str))
    {
      throw BT::RuntimeError("Missing parameter [goal_path]");
    }
    parsePath(waypoints_str);
    read_parameter = true;
  }

  if (!waypoints.empty()) {
    auto xy = waypoints.front();
    waypoints.pop_front();
    std::cout << "x: " << xy.first << ", y: " << xy.second << "\n";
    setOutput("x", xy.first);
    setOutput("y", xy.second);
    if (waypoints.empty()) {
      read_parameter = false;
    }
    return BT::NodeStatus::SUCCESS;
  }
  return BT::NodeStatus::FAILURE;
}

void TxtToWaypointsAction::parsePath(std::string& _waypoints) {
  const char* env_p = std::getenv("MOOD2BE_DIR");
  std::string path = std::string(env_p);
  path = path.substr(0, path.find("MOOD2Be") + 7) + 
    "/Executor/include/Intralogistic/paths/" + _waypoints;
  std::ifstream infile(path);
  double x, y;
  while (infile >> x >> y)
  {
    waypoints.push_back(std::make_pair(x,y));
  }
}

