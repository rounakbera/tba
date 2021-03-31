import tba;
import <iostream>;
import <vector>;
import <sstream>;

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

    // tokenize input by whitespace
    std::istringstream iss(input);
    std::vector<std::string> args{std::istream_iterator<std::string>{iss},
                    std::istream_iterator<std::string>{}};

    return args;
}

int main()
{
    tba::GameRunner<tba::DefaultGameTalker, tba::DefaultGameState> gameRunner;
    gameRunner.runGame();
    gameRunner.talker.getInput();

    tba::GameRunner<MyGameTalker, int> myGameRunner;
    myGameRunner.runGame();
    myGameRunner.talker.getInput();
}