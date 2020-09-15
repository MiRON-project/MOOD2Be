#include "Intralogistic/check_positive_condition.hpp"

CheckPositiveCondition::CheckPositiveCondition(const std::string& instance_name,
    const BT::NodeConfiguration& config) : 
  SimpleConditionNode(instance_name, 
    std::bind(&CheckPositiveCondition::CheckPositive, this), config)
{}

BT::NodeStatus CheckPositiveCondition::CheckPositive() {
  if(!getInput("value", value_))
  {
    throw BT::RuntimeError("Missing parameter [value]");
  }
  if (value_ > 0)
    return BT::NodeStatus::SUCCESS;
  return BT::NodeStatus::FAILURE;
}