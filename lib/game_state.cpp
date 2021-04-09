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
    for (auto const& p : flags) {
        if (std::holds_alternative<bool>(p.second)) {
            auto val = std::get<bool>(p.second);
            if (val) {
                out << p.first << " : true\n";
            }
            else {
                out << p.first << " : false\n";
            }
        } 
        else if (std::holds_alternative<int>(p.second)) {
            auto val = std::get<int>(p.second);
            out << p.first << " : " << val << "\n";
        } 
        else if (std::holds_alternative<std::string>(p.second)) {
            auto val = std::get<std::string>(p.second);
            out << p.first << " : '" << val << "'\n";
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
    for (int i=0; i<std::stoi(line); i++) {
        std::string flagEntry;
        std::getline(in, flagEntry);

        std::string delimiter = " : ";
        auto key = flagEntry.substr(0, flagEntry.find(delimiter));

        flagEntry.erase(0, flagEntry.find(delimiter) + delimiter.length());
        switch (flagEntry.at(0)) {
        case '\'': {
            auto valStr = flagEntry.substr(1, flagEntry.size() - 2);
            flags.insert(std::make_pair(key, valStr));
            break;
        }
        
        case 't': {
            flags.insert(std::make_pair(key, true));
            break;
        }

        case 'f': {
            flags.insert(std::make_pair(key, false));
            break;
        }

        default: {
            auto valInt = std::stoi(flagEntry);
            flags.insert(std::make_pair(key, valInt));
        }
        }

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
    uint32_t num = flags.size();
    out.write((char *) (&num), sizeof(num));

    for (auto const& p : flags) {
        writeString(out, p.first);
        writeVariant(out, p.second);
    }

    if (gameEnd) {
        writeString(out, "true");
    }
    else {
        writeString(out, "false");
    }
    writeString(out, currentRoom);
    return true;
}

void tba::DefaultGameState::writeString(std::ostream& out, std::string str)
{
    uint32_t strSize = (uint32_t) str.size();
    out.write((char *) (&strSize), sizeof(uint32_t));
    out.write(str.c_str(), strSize);
}

void tba::DefaultGameState::writeVariant(std::ostream& out, std::variant<bool, int, std::string> vals)
{
    if (std::holds_alternative<bool>(vals)) {
        auto val = std::get<bool>(vals);
        std::string bool_val;
        if (val) bool_val = "true"; else bool_val = "false";
        writeString(out, bool_val);
    } 
    else if (std::holds_alternative<int>(vals)) {
        auto val = std::get<int>(vals);
        writeString(out, std::to_string(val));
    } 
    else if (std::holds_alternative<std::string>(vals)) {
        auto val = std::get<std::string>(vals);
        val = "'" + val + "'";
        writeString(out, val);
    }
}

bool tba::DefaultGameState::deserializeBinary(std::istream& infile)
{
    flags.clear();
    auto numPairs = readNum(infile);

    for (int i=0; i < numPairs; i++) {
        auto key = readNextString(infile);
        auto val = readNextString(infile);

        switch (val.at(0)) {
        case '\'': {
            auto valStr = val.substr(1, val.size() - 2);
            flags.insert(std::make_pair(key, valStr));
            break;
        }
        
        case 't': {
            flags.insert(std::make_pair(key, true));
            break;
        }

        case 'f': {
            flags.insert(std::make_pair(key, false));
            break;
        }

        default: {
            auto valInt = std::stoi(val);
            flags.insert(std::make_pair(key, valInt));
        }
        }
    }

    //read bool
    auto gamebool = readNextString(infile);
    switch (gamebool.at(0)) {
    case 't': {
        gameEnd = true;
        break;
    }

    case 'f': {
        gameEnd = false;
        break;
    }
    }

    //read currentRoom
    currentRoom = readNextString(infile);

    return true;
}

int tba::DefaultGameState::readNum(std::istream& infile)
{
    uint32_t num;
    infile.read(reinterpret_cast<char *>(&num), sizeof(num));
    return num;
}

std::string tba::DefaultGameState::readNextString(std::istream& infile)
{
    auto strLength = readNum(infile);
    std::string str;
    str.resize(strLength);
    infile.read(&str[0], strLength);
    return str;
}

bool tba::DefaultGameState::serialize(std::ostream& out, std::string format) 
{
    if (format == "simple") {
        return serializeSimple(out);
    }
    else if (format == "binary") {
        return serializeBinary(out);
    }
    else return false;
}

bool tba::DefaultGameState::deserialize(std::istream& in, std::string format)
{
    if (format == "simple") {
        return deserializeSimple(in);
    }
    else if (format == "binary") {
        return deserializeBinary(in);
    }
    else return false;
}
