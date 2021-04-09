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
// clang modules are a bit buggy; may need to import some extra standard library modules

#include "../rapidxml/rapidxml.hpp"
#include "../rapidxml/rapidxml_print.hpp"

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

using GameTalker = tba::DefaultGameTalker;
using GameState = XMLGameState;

int main()
{
    std::ios_base::sync_with_stdio(false); // iostream optimization

    tba::GameRunner<GameTalker, GameState> gameRunner {};
    gameRunner.setSaveState("xml", false);
    gameRunner.state.flags.insert(std::make_pair("testInt", 12));
    gameRunner.state.flags.insert(std::make_pair("testBool", true));
    gameRunner.state.flags.insert(std::make_pair("testString", "testing"));

    /*gameRunner.addStartingRoom("start");
    gameRunner.runGame();
    std::cout << "Game has quit\n";*/
    
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

    gameRunner.addStartingRoom("main hold", mainHold);
    gameRunner.addConnectingRoom("up", "cockpit", cockpit, "down");
    gameRunner.addConnectingRoom("left", "engine room", engineRoom, "right");
    gameRunner.addConnectingRoom("left", "cargo hold", cargoHold, "back", "engine room");

    gameRunner.runGame();

    /*tba::GameRunner<MyGameTalker, tba::DefaultGameState> myGameRunner {};
    myGameRunner.runGame();*/
}