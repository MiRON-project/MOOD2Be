#include <fstream>
#include <unordered_set>
#include <algorithm>
#include <string>
#include "Intralogistic/skill_action.hpp"
#include "Intralogistic/json.hpp"

std::vector<SkillDefinition> ParseSkillFile(const std::string &filename)
{
  std::vector<SkillDefinition> definitions;

  std::ifstream input_file(filename);
  if (input_file.fail())
  {
    throw std::runtime_error("The json file can't be opened");
  }

  nlohmann::json doc;
  input_file >> doc;

  const std::unordered_set<std::string> supported_types =
      {
          "String", "Int", "Double", "Integer", "Bool"};

  for (const auto &item : doc)
  {
    const auto &skill = item["skill"];
    SkillDefinition definition;
    definition.ID = skill["name"];
    definition.skill_FQN = skill["skill-definition-fqn"];

    //----------------
    const auto &inAttributes = skill["in-attribute"];
    for (auto it = inAttributes.begin(); it != inAttributes.end(); it++)
    {
      std::string key = it.key();
      std::string type = it.value();

      if (type == "String")
      {
        definition.ports.insert(BT::InputPort<std::string>(key));
      }
      else if (type == "Int" || type == "Integer")
      {
        definition.ports.insert(BT::InputPort<int>(key));
      }
      else if (type == "Double")
      {
        definition.ports.insert(BT::InputPort<double>(key));
      }
      else if (type == "Bool" || type == "Boolean")
      {
        definition.ports.insert(BT::InputPort<bool>(key));
      }
      else
      {
        throw std::runtime_error("Error in [in-attribute]: We don't support this type: " + type);
      }
    }
    //----------------
    const auto &outAttributes = skill["out-attribute"];
    for (auto it = outAttributes.begin(); it != outAttributes.end(); it++)
    {
      std::string key = it.key();
      std::string type = it.value();

      if (type == "String")
      {
        definition.ports.insert(BT::OutputPort<std::string>(key));
      }
      else if (type == "Int" || type == "Integer")
      {
        definition.ports.insert(BT::OutputPort<int>(key));
      }
      else if (type == "Double")
      {
        definition.ports.insert(BT::OutputPort<double>(key));
      }
      else if (type == "Bool" || type == "Boolean")
      {
        definition.ports.insert(BT::OutputPort<bool>(key));
      }
      else
      {
        throw std::runtime_error("Error in [in-attribute]: We don't support this type: " + type);
      }
    }
    //----------------
    const auto &results = skill["results"];
    for (const auto &result : results)
    {
      std::string res = result["result"];
      std::string value = result["result-value"];

      BT::NodeStatus status;
      if (res == "SUCCESS")
      {
        status = BT::NodeStatus::SUCCESS;
      }
      else if (res == "ERROR")
      {
        status = BT::NodeStatus::FAILURE;
      }
      else
      {
        throw std::runtime_error("Error in skill->results->result: only "
          "accepted values are SUCCESS and ERROR");
      }
      definition.possible_results.push_back({status, value});
    }
    definitions.push_back(std::move(definition));
  }

  return definitions;
}
