import tba;
import <iostream>;
import <vector>;
import <string>;
import <utility>;
import <unordered_map>;
import <string>;
import <sstream>;
import <fstream>;
// clang modules are a bit buggy; may need to import some extra standard library modules
import <variant>;

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

int main()
{
    std::ios_base::sync_with_stdio(false); // iostream optimization

    tba::GameRunner<tba::DefaultGameTalker, tba::DefaultGameState> gameRunner {};
    gameRunner.setSaveFormat("json");
    gameRunner.state.flags.insert(std::make_pair("testInt", 12));
    gameRunner.state.flags.insert(std::make_pair("testBool", true));
    gameRunner.state.flags.insert(std::make_pair("testString", "testing"));

    /*gameRunner.addStartingRoom("start");
    gameRunner.runGame();
    std::cout << "Game has quit\n";*/
    
    // test code for event/action/room
    tba::Room<tba::DefaultGameState> mainHold {};
    mainHold.setDescription("You are sitting in a passenger "
            "chair in a dingy space freighter. As you look out the viewport, "
            "you see the bright starlines of hyperspace flow around you.");
    
    tba::EventFunc<tba::DefaultGameState> descriptionEvent = [](auto& r, auto& s) {
        return std::make_pair(true, "You look around and see two other people. "
            "One appears to be a man, and the other appears to be a woman.");
    };
    mainHold.events.emplace("description2", tba::Event{descriptionEvent});

    tba::ActionFunc<tba::DefaultGameState> talkAction =
        [](auto& room, auto& state, std::vector<std::string> args) {
            if (args.empty()) {
                return std::make_pair(true, "Who do you want to greet?");
            }
            else if (args[0] == "man") {
                tba::ActionFunc<tba::DefaultGameState> nodAction =
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

    tba::Room<tba::DefaultGameState> cockpit {};
    cockpit.setDescription("You enter the cockpit. The pilot is leaning back at her chair.");
    cockpit.setTextAction("Who would you like to greet?", "greet",
        {{"pilot", "\"Heya,\" the pilot waves back at you. \"Have you checked the shields yet?\""}});

    tba::Room<tba::DefaultGameState> engineRoom {};
    engineRoom.setDescription("The engine room is hot and filled with steam. "
        "You see the hyperdrive and the shields. "
        "A droid pokes his optical lens up at you.");
    engineRoom.setTextAction("Who would you like to greet?", "greet",
        {{"droid", "\"Beep beep boop boop,\" the droid says."}});
    engineRoom.setTextAction("What would you like to inspect?", "inspect",
        {{"droid", "The droid jumps back, startled. \"Beep beep beep boop boop boop!\" he exclaims."},
        {"shields", "You run a diagnostic on the shields. They are damaged."},
        {"hyperdrive", "You run a diagnostic on the hyperdrive. It is perfectly functional."}});
    
    tba::Room<tba::DefaultGameState> cargoHold {};
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