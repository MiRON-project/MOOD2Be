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
        "String", "Int", "Double", "Integer", "Bool"
    };

    for (const auto& item: doc)
    {
        const auto& skill = item["skill"];
        SkillDefinition definition;
        definition.ID = skill["name"];
        definition.skill_FQN = skill["skill-definition-fqn"];

        //----------------
        const auto& inAttributes = skill["in-attribute"];
        for (auto it = inAttributes.begin(); it != inAttributes.end(); it++)
        {
            std::string key  = it.key();
            std::string type = it.value();

            if( type == "String"){
              definition.ports.insert( BT::InputPort<std::string>( key ));
            }
            else if( type == "Int" || type == "Integer") {
              definition.ports.insert( BT::InputPort<int>( key ) );
            }
            else if( type == "Double") {
              definition.ports.insert( BT::InputPort<double>( key ) );
            }
            else if( type == "Bool" || type == "Boolean") {
              definition.ports.insert( BT::InputPort<bool>( key ) );
            }
            else{
              throw std::runtime_error("Error in [in-attribute]: We don't support this type: " + type);
            }
        }
        //----------------
        const auto& outAttributes = skill["out-attribute"];
        for (auto it = outAttributes.begin(); it != outAttributes.end(); it++)
        {
          std::string key  = it.key();
          std::string type = it.value();

          if( type == "String"){
            definition.ports.insert( BT::OutputPort<std::string>( key ));
          }
          else if( type == "Int" || type == "Integer") {
            definition.ports.insert( BT::OutputPort<int>( key ) );
          }
          else if( type == "Double") {
            definition.ports.insert( BT::OutputPort<double>( key ) );
          }
          else if( type == "Bool" || type == "Boolean") {
            definition.ports.insert( BT::OutputPort<bool>( key ) );
          }
          else{
            throw std::runtime_error("Error in [in-attribute]: We don't support this type: " + type);
          }
        }
        //----------------
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

std::string SkillAction::generateRequest()
{
    nlohmann::json json;
    json["msg-type"] = "push-skill";
    json["id"] = _current_uid;

    auto& skill = json["skill"];
    skill["name"] = _definition.ID;
    skill["skill-definition-fqn"] = _definition.skill_FQN;
    auto& out_attribute = skill["out-attribute"] = nlohmann::json({});
    auto& in_attribute = skill["in-attribute"] = nlohmann::json({});

    // read all the inputs
    for(const auto& port_pair: _definition.ports )
    {
        if (port_pair.second.direction() == BT::PortDirection::INPUT)
        {
            const auto& key  = port_pair.first;
            const BT::PortInfo& info = port_pair.second;
            auto type = info.type();

            if( *type == typeid(std::string) )
            {
                auto val = getInput<std::string>(key);
                if( !val ){
                    throw std::runtime_error( "Invalid parameter at key: " + key );
                }
                in_attribute.push_back( {key, val.value() } );
            }
            else if( *type == typeid(double) )
            {
                auto val = getInput<double>(key);
                if( !val ){
                    throw std::runtime_error( "Invalid parameter at key: " + key );
                }
                in_attribute.push_back( {key, val.value() } );
            }
            else if( *type == typeid(int) )
            {
                auto val = getInput<int>(key);
                if( !val ){
                    throw std::runtime_error( "Invalid parameter at key: " + key );
                }
                in_attribute.push_back( {key, val.value() } );
            }
            else if( *type == typeid(bool) )
            {
                auto val = getInput<bool>(key);
                if( !val ){
                    throw std::runtime_error( "Invalid parameter at key: " + key );
                }
                in_attribute.push_back( {key, val.value() } );
            }
            else
                throw std::runtime_error( "Invalid parameter at key: " + key );
        }
    }

    // setting all the outputs
    for(const auto& port_pair: _definition.ports )
    {
        if (port_pair.second.direction() == BT::PortDirection::OUTPUT)
        {
            const auto& key  = port_pair.first;
            const BT::PortInfo& info = port_pair.second;
            auto type = info.type();

            if( *type == typeid(std::string) )
                out_attribute.push_back( {key, "" } );
            else if( *type == typeid(double) )
                out_attribute.push_back( {key, 0.0 } );
            else if( *type == typeid(int) )
                out_attribute.push_back( {key, 0 } );
            else if( *type == typeid(bool) )
                out_attribute.push_back( {key, true } );
            else 
                throw std::runtime_error( "Invalid parameter at key: " + key );
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

    if (msg_type != "skill-result")
        return  BT::NodeStatus::IDLE;

    std::string result    = json["result"]["result"];
    auto json_res_value = json["result"]["result-value"];

    if( json_res_value.is_string() )
    {
        const std::string bb_result_key = _definition.ID + "::last_result";
        std::string res_value = json_res_value;
        
//   TODO. What should I do with this string?
//        blackboard()->set( bb_result_key, res_value );
    }

    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    if( result == "SUCCESS")
    {
        return BT::NodeStatus::SUCCESS;
    }
    else if( result == "ERROR" || result == "ABORT")
    {
        return BT::NodeStatus::FAILURE;
    }
    else{
        std::cout << "Error parsing this reply:\n" << result << std::endl;
        throw std::runtime_error("Unrecognized [result]" );
    }
}
