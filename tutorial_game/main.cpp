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

using GameTalker = tba::DefaultGameTalker;
using GameState = tba::DefaultGameState;

int main()
{
    std::ios_base::sync_with_stdio(false); // iostream optimization

    tba::GameRunner<GameTalker, GameState> gameRunner {};
    gameRunner.setSaveState("simple", false);
    
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