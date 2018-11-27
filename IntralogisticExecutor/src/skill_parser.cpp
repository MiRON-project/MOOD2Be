#include <fstream>
#include "Intralogistic/skill_interface.hpp"
#include "Intralogistic/json.hpp"

using namespace BT;

/*
{"skill" :
        {
                "name" : "moverobot_approach_region_purepursuit",
                "skill-definition-fqn" : "CommNavigationObjects.CdlSkills.moverobot_approach_region_purepursuit",
                "in-attribute" : {
                        "location" : "String"
                },
                "out-attribute" : {
                },
                "results" : [
                        { "result" : "ERROR", "result-value" : "UNKNOWN LOCATION" },
                        { "result" : "ERROR", "result-value" : "ROBOT BLOCKED" },
                        { "result" : "SUCCESS", "result-value" : "OK" },
                        { "result" : "SUCCESS", "result-value" : "" }
                ]
        }
}*/

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


    for (const auto& item: doc)
    {
        const auto& skill = item["skill"];
        SkillDefinition definition;
        definition.ID = skill["name"];
        definition.skill_FQN = skill["skill-definition-fqn"];
        //std::cout << definition.ID << std::endl;

        const auto& inAttributes = skill["in-attribute"];

        for (auto it = inAttributes.begin(); it != inAttributes.end(); it++)
        {
            std::string key   = it.key();
            std::string value = it.value();

            //std::cout << key << " = " << value << std::endl;

            definition.params.insert( { key, value} );
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

/*
{
    "msg-type" : "push-skill" ,
    "id" : 1,
    "skill" : {
        "name" : "moverobot_approach_region_purepursuit",
        "skill-definition-fqn" : "CommNavigationObjects.CdlSkills.moverobot_approach_region_purepursuit",
        "in-attribute" : {
            "location" : "Lcoation1-Value"
        },
        "out-attribute" : { }
    }
}
*/
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
    skill["in-attribute"] = current_params;
    skill["out-attribute"] = nlohmann::json({});

    return json.dump(indent);
}
