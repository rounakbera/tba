module tba;

import <vector>;
import <string>;
import <iostream>;
import <unordered_map>;
import <variant>;

bool tba::DefaultGameState::serializeSimple(std::ostream& out)
{
    out << flags.size() << "\n";
    for (auto const& p : flags) 
    {
        if (std::holds_alternative<bool>(p.second)) 
        {
            auto val = std::get<bool>(p.second);
            out << p.first << " : " << val << "\n";
        } 
        else if (std::holds_alternative<int>(p.second)) 
        {
            auto val = std::get<int>(p.second);
            out << p.first << " : " << val << "\n";
        } 
        else if (std::holds_alternative<std::string>(p.second)) 
        {
            auto val = std::get<std::string>(p.second);
            out << p.first << " : " << val << "\n";
        }
    }
    out << gameEnd << "\n";
    out << currentRoom;
    return true;
}

bool tba::DefaultGameState::deserializeSimple(std::istream& in)
{
    flags.clear();
    std::string line;
    std::getline(in, line);
    for (int i=0; i<std::stoi(line); i++)
    {
        std::string flagEntry;
        std::getline(in, flagEntry);

        std::string delimiter = " : ";
        auto key = flagEntry.substr(0, flagEntry.find(delimiter));
        flagEntry.erase(0, flagEntry.find(delimiter) + delimiter.length());
        auto val = flagEntry.substr(0, flagEntry.find(delimiter));

        std::cout << key << " : " << val << "\n";

        flags.insert(std::make_pair(key, val));
    }

    std::getline(in, line);
    gameEnd = std::stoi(line);

    std::getline(in, line);
    currentRoom = line;

    return true;
}

bool tba::DefaultGameState::serializeBinary(std::ostream& out)
{
    auto num_entries = std::to_string(flags.size());

    out.write(num_entries.c_str(), num_entries.size());
    // out << flags.size() << std::endl;
    // for (auto const& p : flags) 
    // {
    //     if (std::holds_alternative<bool>(p.second)) 
    //     {
    //         auto val = std::get<bool>(p.second);
    //         out << p.first << " : " << val << std::endl;
    //     } 
    //     else if (std::holds_alternative<int>(p.second)) 
    //     {
    //         auto val = std::get<int>(p.second);
    //         out << p.first << " : " << val << std::endl;
    //     } 
    //     else if (std::holds_alternative<std::string>(p.second)) 
    //     {
    //         auto val = std::get<std::string>(p.second);
    //         out << p.first << " : " << val << std::endl;
    //     }
    // }
    // out << gameEnd << std::endl;
    // out << currentRoom;
    return true;
}

bool tba::DefaultGameState::deserializeBinary(std::istream& in)
{
    flags.clear();
    std::string line;
    std::getline(in, line);
    for (int i=0; i<std::stoi(line); i++)
    {
        std::string flagEntry;
        std::getline(in, flagEntry);

        std::string delimiter = " : ";
        auto key = flagEntry.substr(0, flagEntry.find(delimiter));
        flagEntry.erase(0, flagEntry.find(delimiter) + delimiter.length());
        auto val = flagEntry.substr(0, flagEntry.find(delimiter));

        std::cout << key << " : " << val << std::endl;

        flags.insert(std::make_pair(key, val));
    }

    std::getline(in, line);
    gameEnd = stoi(line);

    std::getline(in, line);
    currentRoom = line;

    return true;
}

bool tba::DefaultGameState::serializeJson(std::ostream& out)
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
    return true;
}

bool tba::DefaultGameState::deserializeJson(std::istream& in)
{
    flags.clear();
    flags.insert(std::make_pair("newTest", 777));
    currentRoom = "main hold";
    gameEnd = false;
    return true;
}

bool tba::DefaultGameState::serialize(std::ostream& out, std::string format) 
{
    if (format == "simple")
    {
        return serializeSimple(out);
    }
    else if (format == "binary")
    {
        return serializeBinary(out);
    }
    else if (format == "json") 
    {
        return serializeJson(out);
    }
    else return false;
}

bool tba::DefaultGameState::deserialize(std::istream& in, std::string format)
{
    if (format == "simple")
    {
        return deserializeSimple(in);
    }
    else if (format == "binary")
    {
        return deserializeBinary(in);
    }
    else if (format == "json") 
    {
        return deserializeJson(in);
    }
    else return false;
}
