import tba;
import <iostream>;
import <vector>;
import <string>;
// clang modules are a bit buggy; may need to import some extra standard library modules
import <variant>;
import <unordered_map>;

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
    tba::GameRunner<tba::DefaultGameTalker, tba::DefaultGameState> gameRunner;
    gameRunner.runGame();
    gameRunner.talker.getInput();

    tba::GameRunner<MyGameTalker, tba::DefaultGameState> myGameRunner;
    myGameRunner.runGame();
    myGameRunner.talker.getInput();
}