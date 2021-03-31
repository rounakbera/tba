module tba;

import <iostream>;
import <vector>;
import <sstream>;
import <algorithm>;

std::vector<std::string> tba::DefaultGameTalker::getInput()
{
    std::string input;
    std::cin >> input;

    // tokenize input by whitespace
    std::istringstream iss(input);
    std::vector<std::string> args{std::istream_iterator<std::string>{iss},
                    std::istream_iterator<std::string>{}};

    return args;
}