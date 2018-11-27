#include <behaviortree_cpp/blackboard/blackboard_local.h>
#include <behaviortree_cpp/bt_factory.h>

#include "Intralogistic/args.hpp"

using namespace BT;


int main(int argc, char** argv)
{
    // Read more about args here: https://github.com/Taywee/args

    args::ArgumentParser parser("BehaviorTree.CPP Executor", "Load one or multiple plugins and the XML with the tree definition.");
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});

    args::Group arguments(parser, "arguments", args::Group::Validators::DontCare, args::Options::Global);
    args::ValueFlag<std::string> tree_path(arguments, "path", "The XML containing the BehaviorTree ", {'t', "tree"});
    args::ValueFlag<std::string> skills_path(arguments, "path", "JSON file containing the list of SmartSoft skills", {'s', "skills"});
    args::ValueFlag<std::string> plugin_path(arguments, "path", "Plugin to load (default is 'SkillInterface.so')", {'p', "plugin"}, "SkillInterface.so" );

    try
    {
        parser.ParseCLI(argc, argv);
    }
    catch (const args::Completion& e)
    {
        std::cout << e.what();
        return 0;
    }
    catch (const args::Help&)
    {
        std::cout << parser;
        return 0;
    }
    catch (const args::ParseError& e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }

    std::cout << "\tplugins path: " << args::get(plugin_path) << std::endl;

    if (tree_path) {
        std::cout << "\ttree file: " << args::get(tree_path) << std::endl;
    }

    if (skills_path) {
        std::cout << "\tskills file: " << args::get(skills_path) << std::endl;
    }



    BehaviorTreeFactory factory;

    return 0;
}
