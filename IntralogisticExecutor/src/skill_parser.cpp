#include <fstream>
#include <unordered_set>
#include <algorithm>
#include <string>
#include "Intralogistic/skill_interface.hpp"
#include "Intralogistic/json.hpp"


std::vector<SkillDefinition> ParseSkillFile(const std::string &filename)
{
    std::vector<SkillDefinition> definitions;

    std::ifstream input_file(filename);
    if(input_file.fail())
    {
        throw std::runtime_error("The json file can't be opened");
    }

    nlohmann::json doc;
    input_file >> doc;

    const std::unordered_set<std::string> supported_types =
    {
        "String", "Int", "Double", "Integer"
    };

    for (const auto& item: doc)
    {
        const auto& skill = item["skill"];
        SkillDefinition definition;
        definition.ID = skill["name"];
        definition.skill_FQN = skill["skill-definition-fqn"];

        const auto& inAttributes = skill["in-attribute"];

        for (auto it = inAttributes.begin(); it != inAttributes.end(); it++)
        {
            std::string key   = it.key();
            std::string type = it.value();

            if( supported_types.count(type) == 0)
            {
                throw std::runtime_error("Error in [in-attribute]: We don't support this type: " + type);
            }

            definition.params.insert( { key, type} );
        }

        const auto& results = skill["results"];
        for (const auto& result: results)
        {
            std::string res   = result["result"];
            std::string value = result["result-value"];

            BT::NodeStatus status;
            if( res == "SUCCESS")
            {
                status = BT::NodeStatus::SUCCESS;
            }
            else if( res == "ERROR")
            {
                status = BT::NodeStatus::FAILURE;
            }
            else{
                throw std::runtime_error( "Error in skill->results->result: only accepted values "
                                          "are SUCCESS and ERROR");
            }
            definition.possible_results.push_back( {status,value} );
        }
        definitions.push_back( std::move(definition) );
    }

    return definitions;
}


std::string GenerateRequest(const SkillDefinition& definition,
                            unsigned msg_uid,
                            const BT::NodeParameters& current_params,
                            int indent)
{
    nlohmann::json json;
    json["msg-type"] = "push-skill";
    json["id"] = msg_uid;

    auto& skill = json["skill"];
    skill["name"] = definition.ID;
    skill["skill-definition-fqn"] = definition.skill_FQN;
    skill["out-attribute"] = nlohmann::json({});
    skill["in-attribute"] = current_params;

    return json.dump(indent);
}

std::string SkillAction::generateRequest()
{
    nlohmann::json json;
    json["msg-type"] = "push-skill";
    json["id"] = _current_uid;

    auto& skill = json["skill"];
    skill["name"] = _definition.ID;
    skill["skill-definition-fqn"] = _definition.skill_FQN;
    skill["out-attribute"] = nlohmann::json({});
    auto& in_attribute = skill["in-attribute"] = nlohmann::json({});

    // read node parameters
    for(const auto& param: initializationParameters() )
    {
        const auto& key  = param.first;
        const auto& type = _definition.params.at(key);

        if( type == "String")
        {
            auto val = getParam<std::string>(key);
            if( !val ){
                throw std::runtime_error( "Invalid parameter at key: " + key );
            }
            in_attribute.push_back( {key, val.value() } );
        }
        else if( type == "Double")
        {
            auto val = getParam<double>(key);
            if( !val ){
                throw std::runtime_error( "Invalid parameter at key: " + key );
            }
            in_attribute.push_back( {key, val.value() } );
        }
        else if( type == "Integer" || type == "Int" )
        {
            auto val = getParam<int>(key);
            if( !val ){
                throw std::runtime_error( "Invalid parameter at key: " + key );
            }
            in_attribute.push_back( {key, val.value() } );
        }
    }

    return json.dump(1);
}

BT::NodeStatus SkillAction::convertResultToStatus(const std::string &result_string)
{
    nlohmann::json json = nlohmann::json::parse(result_string);

    unsigned msg_id = json["id"];
    if( msg_id != _current_uid)
    {
        return  BT::NodeStatus::IDLE;
    }
    std::string msg_type  = json["msg-type"];
    std::string result    = json["result"]["result"];
    auto json_res_value = json["result"]["result-value"];

    if( json_res_value.is_string() )
    {
        const std::string bb_result_key = _definition.ID + "::last_result";
        std::string res_value = json_res_value;
        blackboard()->set( bb_result_key, res_value );
    }

    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    if( result == "SUCCESS")
    {
        return BT::NodeStatus::SUCCESS;
    }
    else if( result == "ERROR")
    {
        return BT::NodeStatus::FAILURE;
    }
    else{
        std::cout << "Error parsing this reply:\n" << result << std::endl;
        throw std::runtime_error("Unrecognized [result]" );
    }
}
