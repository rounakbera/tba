import tba;
import <iostream>;
import <vector>;
import <string>;
import <utility>;
import <unordered_map>;
import <string>;
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
    /*gameRunner.runGame();
    std::cout << "Game has quit\n";*/
    
    // test code for event/action
    gameRunner.currentRoom.setDescription("You are sitting in a passenger chair in "
            "a dingy space freighter. As you look out the viewport, you see the "
            "bright starlines of hyperspace flow around you.");
    
    tba::EventFunc<tba::DefaultGameState> descriptionEvent = [](auto& r, auto& s) {
        return std::make_pair(true, "You look around and see two other people. "
            "One appears to be a man, and the other appears to be a woman.");
    };
    gameRunner.currentRoom.events.emplace("description2", tba::Event{descriptionEvent});

    tba::ActionFunc<tba::DefaultGameState> talkAction = [](auto& r, auto& s, std::vector<std::string> args) {
        if (args.size() == 0) {
            return std::make_pair(true, "Who do you want to greet?");
        }
        else if (args[0] == "man") {
            return std::make_pair(true, "The man looks up and smiles at you. \"Bored yet?\"");
        }
        else if (args[0] == "woman") {
            return std::make_pair(true, "The woman makes eye contact with you but does not respond.");
        }
        return std::make_pair(true, "You can't greet this!");
    };
    gameRunner.currentRoom.actions.insert_or_assign("greet", tba::Action{talkAction});

    gameRunner.runGame();

    /*tba::GameRunner<MyGameTalker, tba::DefaultGameState> myGameRunner {};
    myGameRunner.runGame();*/
}