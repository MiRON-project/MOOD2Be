#include <fstream>
#include "Intralogistic/skill_interface.hpp"
#include "Intralogistic/json.hpp"

using namespace BT;

/*
{"skill" :
        {
                "name" : "moverobot_approach_region_purepursuit",
                "skillDefinitionFQN" : "CommNavigationObjects.CdlSkills.moverobot_approach_region_purepursuit",
                "inAttribute" : {
                        "location" : "String"
                },
                "outAttribute" : {
                },
                "results" : [
                        { "result" : "ERROR", "resultValue" : "UNKNOWN LOCATION" },
                        { "result" : "ERROR", "resultValue" : "ROBOT BLOCKED" },
                        { "result" : "SUCCESS", "resultValue" : "OK" },
                        { "result" : "SUCCESS", "resultValue" : "" }
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
        definition.skill_FQN = skill["skillDefinitionFQN"];
        //std::cout << definition.ID << std::endl;

        const auto& inAttributes = skill["inAttribute"];

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
            std::string value = result["resultValue"];
           // std::cout << res << " = " << value << std::endl;

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
