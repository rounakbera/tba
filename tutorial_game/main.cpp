import tba;
import <iostream>;
import <vector>;
import <string>;
import <utility>;
import <unordered_map>;
import <string>;
import <sstream>;
import <fstream>;
import <variant>;
import <typeinfo>;
// clang modules are a bit buggy; may need to import some extra standard library modules

#include "../rapidxml/rapidxml.hpp"
#include "../rapidxml/rapidxml_print.hpp"
#include "../rapidjson/rapidjson.h"
#include "../rapidjson/document.h"
#include "../rapidjson/allocators.h"
#include "../rapidjson/ostreamwrapper.h"
#include "../rapidjson/istreamwrapper.h"
#include "../rapidjson/writer.h"

class MyGameTalker {
public:
    std::vector<std::string> history;

    std::vector<std::string> getInput();
};

std::vector<std::string> MyGameTalker::getInput()
{
    std::cout << "enter input:\n";
    std::string input;
    std::cin >> input;

    std::vector<std::string> args {input};
    return args;
}

class XMLGameState {
public:
    std::unordered_map<std::string, std::variant<bool, int, std::string>> flags;
    bool gameEnd;
    tba::RoomName currentRoom;

    bool serialize(std::ostream& myOStream, std::string format);
    bool deserialize(std::istream& myIStream, std::string format);
};

bool XMLGameState::serialize(std::ostream& out, std::string format)
{
    if (format != "xml") {
        return false;
    }
    rapidxml::xml_document<> doc;
    rapidxml::xml_node<>* flagsNode = doc.allocate_node(rapidxml::node_element, "flags");

    for (auto const& p : flags) {
        std::string val;
        std::string type;

        if (std::holds_alternative<bool>(p.second)) {
            val = (std::get<bool>(p.second)) ? "true" : "false";
            type = "bool";
        } 
        else if (std::holds_alternative<int>(p.second)) {
            val = std::to_string(std::get<int>(p.second));
            type = "int";
        } 
        else if (std::holds_alternative<std::string>(p.second)) {
            val = std::get<std::string>(p.second);
            type = "string";
        }

        rapidxml::xml_node<>* keyNode = doc.allocate_node(rapidxml::node_element, "key", doc.allocate_string(p.first.c_str()));
        rapidxml::xml_node<>* valNode = doc.allocate_node(rapidxml::node_element, "val", doc.allocate_string(val.c_str()));
        rapidxml::xml_attribute<> *typeAttr = doc.allocate_attribute("type", doc.allocate_string(type.c_str()));
        valNode->append_attribute(typeAttr);

        rapidxml::xml_node<>* flagNode = doc.allocate_node(rapidxml::node_element, "flag");
        flagNode->append_node(keyNode);
        flagNode->append_node(valNode);

        flagsNode->append_node(flagNode);
    }
    doc.append_node(flagsNode);

    std::string gameEndStr = (gameEnd) ? "true" : "false";

    rapidxml::xml_node<>* gameEndNode = doc.allocate_node(rapidxml::node_element, "gameEnd", gameEndStr.c_str());
    rapidxml::xml_node<>* currRoomNode = doc.allocate_node(rapidxml::node_element, "currentRoom", currentRoom.c_str());
    doc.append_node(gameEndNode);
    doc.append_node(currRoomNode);

    out << doc;

    return true;
}

bool XMLGameState::deserialize(std::istream& in, std::string format)
{
    if (format != "xml") {
        return false;
    }

    std::stringstream buffer;
    buffer << in.rdbuf();
    std::string str = buffer.str();

    rapidxml::xml_document<> doc;
    doc.parse<0>(str.data());

    flags.clear();
    rapidxml::xml_node<>* flagsNode = doc.first_node("flags");
    for (rapidxml::xml_node<> *flagNode = flagsNode->first_node(); flagNode; flagNode = flagNode->next_sibling()) {
        std::string key = flagNode->first_node("key")->value();
        rapidxml::xml_node<> *valNode = flagNode->first_node("val");
        std::string val = valNode->value();
        std::string type = valNode->first_attribute("type")->value();

        if (type == "bool") {
            if (val == "true") {
                flags.insert(std::make_pair(key, true));
            }
            else {
                flags.insert(std::make_pair(key, false));
            }
        }
        else if (type == "int") {
            flags.insert(std::make_pair(key, std::stoi(val)));
        }
        else if (type == "string") {
            flags.insert(std::make_pair(key, val));
        }
    }
    
    currentRoom = doc.first_node("currentRoom")->value();
    std::string gameEndStr = doc.first_node("gameEnd")->value();
    gameEnd = (gameEndStr == "true");
    return true;
}

class JSONGameState {
public:
    std::unordered_map<std::string, std::variant<bool, int, std::string>> flags;
    bool gameEnd;
    tba::RoomName currentRoom;

    bool serialize(std::ostream& myOStream, std::string format);
    bool deserialize(std::istream& myIStream, std::string format);
};

bool JSONGameState::serialize(std::ostream& out, std::string format)
{
    if (format != "json") {
        return false;
    }

    rapidjson::Document d;
    rapidjson::Document::AllocatorType& allocator = d.GetAllocator(); 
    rapidjson::Value root(rapidjson::kObjectType); 
    rapidjson::Value flag(rapidjson::kObjectType); 
    rapidjson::Value key(rapidjson::kStringType); 
    rapidjson::Value value(rapidjson::kStringType); 

    std::string val;

    for (auto const& p : flags) 
    {
        key.SetString(p.first.c_str(), allocator);
        if (std::holds_alternative<bool>(p.second)) 
        {
            auto boolVal = std::get<bool>(p.second);
            if (boolVal)
            {
                val = "true";
            }
            else
            {
                val = "false";
            }
        } 
        else if (std::holds_alternative<int>(p.second)) 
        {
            auto intVal = std::get<int>(p.second);
            val = std::to_string(intVal);
        } 
        else if (std::holds_alternative<std::string>(p.second)) 
        {
            auto stringVal = std::get<std::string>(p.second);
            val = stringVal;
        }
        value.SetString(val.c_str(), allocator);
        flag.AddMember(key, value, allocator);
    }
    root.AddMember("flags", flag, allocator);
    root.AddMember("gameEnd", gameEnd, allocator);
    value.SetString(currentRoom.c_str(), allocator);
    root.AddMember("currentRoom", value, allocator);

    rapidjson::OStreamWrapper osw(out);
    rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);

    root.Accept(writer);

    return true;
}

bool JSONGameState::deserialize(std::istream& in, std::string format)
{
    rapidjson::IStreamWrapper isw(in);
    // rapidjson::Reader<rapidjson::IStreamWrapper> reader(isw);
    rapidjson::Document d;
    d.ParseStream(isw);
    if (d.HasParseError())
    {
        return false;
    }

    flags.clear();
    
    for (rapidjson::Value::ConstMemberIterator it = d["flags"].MemberBegin(); it != d["flags"].MemberEnd(); ++it){
        auto val = it;
        if(val->value.IsBool())
        {
            flags[val->name.GetString()] = val->value.GetBool();
        }
        else if (val->value.IsInt())
        {
            flags[val->name.GetString()] = val->value.GetInt();
        }
        else if (val->value.IsString())
        {
            flags[val->name.GetString()] = val->value.GetString();
        }
    }

    currentRoom = d["currentRoom"].GetString();
    gameEnd = d["gameEnd"].GetBool();

    return true;
}

using GameTalker = tba::DefaultGameTalker;
using GameState = JSONGameState;

int main()
{
    std::ios_base::sync_with_stdio(false); // iostream optimization

    tba::GameRunner<GameTalker, GameState> gameRunner {};
    gameRunner.setSaveState("json", false);
    gameRunner.state.flags.insert(std::make_pair("testInt", 12));
    gameRunner.state.flags.insert(std::make_pair("testBool", true));
    gameRunner.state.flags.insert(std::make_pair("testString", "testing"));
    
    // test code for event/action/room
    tba::Room<GameState> mainHold {};
    mainHold.setDescription("You are sitting in a passenger "
            "chair in a dingy space freighter. As you look out the viewport, "
            "you see the bright starlines of hyperspace flow around you.");
    
    tba::EventFunc<GameState> descriptionEvent = [](auto& r, auto& s) {
        return std::make_pair(true, "You look around and see two other people. "
            "One appears to be a man, and the other appears to be a woman.");
    };
    mainHold.events.emplace("description2", tba::Event{descriptionEvent});

    tba::ActionFunc<GameState> talkAction =
        [](auto& room, auto& state, std::vector<std::string> args) {
            if (args.empty()) {
                return std::make_pair(true, "Who do you want to greet?");
            }
            else if (args[0] == "man") {
                tba::ActionFunc<GameState> nodAction =
                    [](auto& room, auto& state, std::vector<std::string> args) {
                        room.actions.erase("nod");
                        return std::make_pair(true, "You nod. \"I knew it would happen eventually,\" he chuckles.");
                    };
                room.actions.insert_or_assign("nod", tba::Action{nodAction});
                return std::make_pair(true, "The man looks up and smiles at you. \"Bored yet?\"");
            }
            else if (args[0] == "woman") {
                room.setTextAction("Who would you like to poke?", "poke", {{"woman", "The woman glares at you."}});
                return std::make_pair(true, "The woman makes eye contact with you but does not respond.");
            }
            return std::make_pair(true, "You can't greet this!");
        };
    mainHold.actions.insert_or_assign("greet", tba::Action{talkAction});

    std::unordered_map<std::string, std::string> holdGoTexts = {
        {"up", "You climb up the ladder to the cockpit."},
        {"left", "You walk down the hallway to the engine room."}
    };
    mainHold.setTextAction("", "go", holdGoTexts);

    tba::Room<GameState> cockpit {};
    cockpit.setDescription("You enter the cockpit. The pilot is leaning back at her chair.");
    cockpit.setTextAction("Who would you like to greet?", "greet",
        {{"pilot", "\"Heya,\" the pilot waves back at you. \"Have you checked the shields yet?\""}});

    tba::Room<GameState> engineRoom {};
    engineRoom.setDescription("The engine room is hot and filled with steam. "
        "You see the hyperdrive and the shields. "
        "A droid pokes his optical lens up at you.");
    engineRoom.setTextAction("Who would you like to greet?", "greet",
        {{"droid", "\"Beep beep boop boop,\" the droid says."}});
    engineRoom.setTextAction("What would you like to inspect?", "inspect",
        {{"droid", "The droid jumps back, startled. \"Beep beep beep boop boop boop!\" he exclaims."},
        {"shields", "You run a diagnostic on the shields. They are damaged."},
        {"hyperdrive", "You run a diagnostic on the hyperdrive. It is perfectly functional."}});
    
    tba::Room<GameState> cargoHold {};
    cargoHold.setDescription("You enter the cargo hold. "
        "It is currently empty, as you and your crew have just sold the remaining stock.");

    cargoHold.connections.insert_or_assign("forward", "cockpit");

    gameRunner.state.flags.insert(std::make_pair("is stowaway alive", true));
    gameRunner.state.flags.insert(std::make_pair("is stowaway friend", false));

    tba::ActionFunc<GameState> cargoGoAction = [](auto& room, auto& state, std::vector<std::string> args) {
        if (!args.empty() && args[0] == "forward") {
            bool isStowawayAlive = std::get<bool>(state.flags.at("is stowaway alive"));
            bool isStowawayFriend = std::get<bool>(state.flags.at("is stowaway friend"));
            if (isStowawayAlive && !isStowawayFriend) {
                tba::ActionFunc<GameState> attackAction =
                    [](auto& room, auto& state, std::vector<std::string> args) {
                        room.actions.erase("attack");
                        room.actions.erase("befriend");
                        state.flags.insert_or_assign("is stowaway alive", false);
                        return std::make_pair(true, "You blast the droid in its face. It falls over and dies.");
                    };
                tba::ActionFunc<GameState> befriendAction =
                    [](auto& room, auto& state, std::vector<std::string> args) {
                        room.actions.erase("attack");
                        room.actions.erase("befriend");
                        state.flags.insert_or_assign("is stowaway friend", true);
                        return std::make_pair(true, "You pat the droid. It beeps quietly and gets out of the way.");
                    };
                room.actions.insert_or_assign("attack", tba::Action{attackAction});
                room.actions.insert_or_assign("befriend", tba::Action{befriendAction});
                return std::make_pair(false, "You try to climb forward, but you see a stowaway droid blocking the path!");
            }
            if (!isStowawayAlive) {
                return std::make_pair(true, "You step past the burnt remains of the droid and make your way forward.");
            }
            if (isStowawayFriend) {
                return std::make_pair(true, "You wave at the droid as you make your way forward. It waves back shyly.");
            }
        }
        return std::make_pair(true, "You exit the cargo hold.");
    };
    cargoHold.actions.insert_or_assign("go", tba::Action{cargoGoAction});

    gameRunner.addStartingRoom("main hold", mainHold);
    gameRunner.addConnectingRoom("up", "cockpit", cockpit, "down");
    gameRunner.addConnectingRoom("left", "engine room", engineRoom, "right");
    gameRunner.addConnectingRoom("left", "cargo hold", cargoHold, "back", "engine room");

    gameRunner.runGame();
}