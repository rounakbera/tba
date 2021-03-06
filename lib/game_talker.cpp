module tba;

import <iostream>;
import <vector>;
import <sstream>;
import <algorithm>;
import <string>;

std::vector<std::string> tba::DefaultGameTalker::getInput()
{
    std::cout << "> ";
    std::string input = "";
    while (input == "") {
        std::getline(std::cin, input);
    }

    history.emplace_back(input);

    // tokenize input by whitespace
    std::istringstream iss(input);
    std::vector<std::string> args{std::istream_iterator<std::string>{iss},
                    std::istream_iterator<std::string>{}};

    return args;
}