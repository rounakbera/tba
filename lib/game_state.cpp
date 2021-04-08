module tba;

import <vector>;

std::pair<bool, std::chrono::microseconds> tba::DefaultGameState::save(Format format)
{
    std::chrono::microseconds myTime{3};
    std::pair<bool, std::chrono::microseconds> myPair(true, myTime);
    return myPair;
}

std::pair<bool, std::chrono::microseconds> tba::DefaultGameState::load(Format format)
{
    std::chrono::microseconds myTime{3};
    std::pair<bool, std::chrono::microseconds> myPair(true, myTime);
    return myPair;
}