module tba;

std::pair<bool, std::chrono::microseconds> tba::DefaultGameState::save()
{
    std::chrono::microseconds myTime{3};
    std::pair<bool, std::chrono::microseconds> myPair(true, myTime);
    return myPair;
}