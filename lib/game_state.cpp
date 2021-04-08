module tba;

import <vector>;
import <string>;
import <iostream>;
import <unordered_map>;
import <variant>;

void tba::DefaultGameState::serializeSimple(std::ostream& out)
{
    out << flags.size() << std::endl;
    for (auto const& p : flags) 
    {
        if (std::holds_alternative<bool>(p.second)) 
        {
            auto val = std::get<bool>(p.second);
            out << p.first << " : " << val << std::endl;
        } 
        else if (std::holds_alternative<int>(p.second)) 
        {
            auto val = std::get<int>(p.second);
            out << p.first << " : " << val << std::endl;
        } 
        else if (std::holds_alternative<std::string>(p.second)) 
        {
            auto val = std::get<std::string>(p.second);
            out << p.first << " : " << val << std::endl;
        }
    }
    out << gameEnd << std::endl;
    out << currentRoom;
}

tba::DefaultGameState* tba::DefaultGameState::deserializeSimple(std::istream& in)
{
    tba::DefaultGameState *newGS = new tba::DefaultGameState();

    std::string line;
    std::getline(in, line);

    for (int i=0; i<std::stoi(line); i++)
    {
        std::string flagEntry;
        std::getline(in, flagEntry);

        auto delimiter = " : ";
        auto key = flagEntry.substr(0, flagEntry.find(delimiter));
        auto val = flagEntry.substr(1, flagEntry.find(delimiter));

        newGS->flags.insert(std::make_pair(key, val));
    }

    std::getline(in, line);
    newGS->gameEnd = stoi(line);

    std::getline(in, line);
    newGS->currentRoom = line;

    return newGS;
}

void tba::DefaultGameState::serializeJson(std::ostream& out)
{
    out << "{ flags: { ";

    for (auto const& p : flags) 
    {
        if (std::holds_alternative<bool>(p.second)) 
        {
            auto val = std::get<bool>(p.second);
            out << p.first << ": " << val << ", ";
        } 
        else if (std::holds_alternative<int>(p.second)) 
        {
            auto val = std::get<int>(p.second);
            out << p.first << ": " << val << ", ";
        } 
        else if (std::holds_alternative<std::string>(p.second)) 
        {
            auto val = std::get<std::string>(p.second);
            out << p.first << ": " << val << ", ";
        }
    }

    out << "}, gameEnd: " << gameEnd << ", ";
    out << "currentRoom: " << currentRoom << " }";
}

tba::DefaultGameState* tba::DefaultGameState::deserializeJson(std::istream& in)
{
    tba::DefaultGameState *newGS = new tba::DefaultGameState;
    newGS->flags.insert(std::make_pair("newTest", 777));
    newGS->currentRoom = "main hold";
    newGS->gameEnd = false;
    return newGS;
}

void tba::DefaultGameState::serialize(std::ostream& mystream, std::string format) 
{
    if (format == "simple")
    {
        serializeSimple(mystream);
    }
    else if (format == "json") 
    {
        serializeJson(mystream);
    }
    else return;
}

tba::DefaultGameState* tba::DefaultGameState::deserialize(std::istream& in, std::string format)
{
    if (format == "simple")
    {
        return deserializeSimple(in);
    }
    else if (format == "json") 
    {
        return deserializeJson(in);
    }
}
