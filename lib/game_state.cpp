module tba;

import <vector>;
import <string>;
import <iostream>;
import <unordered_map>;
import <variant>;


/*
format is:
# of Entries in flags
every kv pair in the map
gameEnd
currentRoom
*/
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

        flags.insert(std::make_pair(key, val));
    }

    std::getline(in, line);
    gameEnd = std::stoi(line);

    std::getline(in, line);
    currentRoom = line;

    return true;
}

/*
# of kv pairs (int)
each kv pair (strlen|str|strlen|str)
gameEnd (bool)
currentRoom (strlen|str)
*/
bool tba::DefaultGameState::serializeBinary(std::ostream& out)
{
    int num = flags.size();
    out.write((char *) (&num), sizeof(num));

    for (auto const& p : flags) 
    {
        writeString(out, p.first);
        writeVariant(out, p.second);
    }

    if (gameEnd) {
        writeString(out, "true");
    } else {
        writeString(out, "false");
    }
    writeString(out, currentRoom);
    return true;
}

void tba::DefaultGameState::writeString(std::ostream& out, std::string str)
{
    size_t strSize = str.size();
    out.write((char *) (&strSize), sizeof(strSize));
    out.write(str.c_str(), strSize);
}

void tba::DefaultGameState::writeVariant(std::ostream& out, std::variant<bool, int, std::string> vals)
{
    if (std::holds_alternative<bool>(vals)) 
    {
        auto val = std::get<bool>(vals);
        std::string bool_val;
        if (val) bool_val = "true"; else bool_val = "false";
        writeString(out, bool_val);
    } 
    else if (std::holds_alternative<int>(vals)) 
    {
        auto val = std::get<int>(vals);
        writeString(out, std::to_string(val));
    } 
    else if (std::holds_alternative<std::string>(vals)) 
    {
        auto val = std::get<std::string>(vals);
        val = "'" + val + "'";
        writeString(out, val);
    }
}

bool tba::DefaultGameState::deserializeBinary(std::istream& in)
{
    // flags.clear();
    // std::string line;
    // std::getline(in, line);
    // for (int i=0; i<std::stoi(line); i++)
    // {
    //     std::string flagEntry;
    //     std::getline(in, flagEntry);

    //     std::string delimiter = " : ";
    //     auto key = flagEntry.substr(0, flagEntry.find(delimiter));
    //     flagEntry.erase(0, flagEntry.find(delimiter) + delimiter.length());
    //     auto val = flagEntry.substr(0, flagEntry.find(delimiter));

    //     std::cout << key << " : " << val << std::endl;

    //     flags.insert(std::make_pair(key, val));
    // }

    // std::getline(in, line);
    // gameEnd = stoi(line);

    // std::getline(in, line);
    // currentRoom = line;

    return true;
}


/*
{
    flags: {},
    gameEnd: bool,
    currentRoom: string
}
*/
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
