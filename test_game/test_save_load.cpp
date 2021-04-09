import tba;
import <iostream>;
import <vector>;
import <string>;
import <utility>;
import <unordered_map>;
import <string>;
import <sstream>;
import <fstream>;
import <variant>;
import <typeinfo>;
import <random>;
import <unordered_set>;
// clang modules are a bit buggy; may need to import some extra standard library modules

#include "../rapidxml/rapidxml.hpp"
#include "../rapidxml/rapidxml_print.hpp"
#include "../rapidjson/rapidjson.h"
#include "../rapidjson/document.h"
#include "../rapidjson/allocators.h"
#include "../rapidjson/ostreamwrapper.h"
#include "../rapidjson/istreamwrapper.h"
#include "../rapidjson/writer.h"

class TestGameState {
public:
    std::unordered_map<std::string, std::variant<bool, int, std::string>> flags;
    bool gameEnd;
    tba::RoomName currentRoom;

    bool serialize(std::ostream& myOStream, std::string format);
    bool deserialize(std::istream& myIStream, std::string format);

    bool serializeSimple(std::ostream& out);
    bool deserializeSimple(std::istream& in);
    bool serializeBinary(std::ostream& out);
    bool deserializeBinary(std::istream& in);
    bool serializeJson(std::ostream& out);
    bool deserializeJson(std::istream& in);
    bool serializeXml(std::ostream& out);
    bool deserializeXml(std::istream& in);

private:
    void writeString(std::ostream&, std::string);
    void writeVariant(std::ostream&, std::variant<bool, int, std::string>);
    int readNum(std::istream& infile);
    std::string readNextString(std::istream& infile);
};

/*
format is:
# of Entries in flags
every kv pair in the map
gameEnd
currentRoom
*/
bool TestGameState::serializeSimple(std::ostream& out)
{
    out << flags.size() << "\n";
    for (auto const& p : flags) 
    {
        if (std::holds_alternative<bool>(p.second)) 
        {
            auto val = std::get<bool>(p.second);
            if (val) 
            {
                out << p.first << " : true\n";
            }
            else
            {
                out << p.first << " : false\n";
            }
        } 
        else if (std::holds_alternative<int>(p.second)) 
        {
            auto val = std::get<int>(p.second);
            out << p.first << " : " << val << "\n";
        } 
        else if (std::holds_alternative<std::string>(p.second)) 
        {
            auto val = std::get<std::string>(p.second);
            out << p.first << " : '" << val << "'\n";
        }
    }
    out << gameEnd << "\n";
    out << currentRoom;
    return true;
}

bool TestGameState::deserializeSimple(std::istream& in)
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
        switch (flagEntry.at(0))
        {
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
bool TestGameState::serializeBinary(std::ostream& out)
{
    uint32_t num = flags.size();
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

void TestGameState::writeString(std::ostream& out, std::string str)
{
    uint32_t strSize = (uint32_t) str.size();
    out.write((char *) (&strSize), sizeof(uint32_t));
    out.write(str.c_str(), strSize);
}

void TestGameState::writeVariant(std::ostream& out, std::variant<bool, int, std::string> vals)
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

bool TestGameState::deserializeBinary(std::istream& infile)
{
    flags.clear();
    auto numPairs = readNum(infile);

    for (int i=0; i < numPairs; i++)
    {
        auto key = readNextString(infile);
        auto val = readNextString(infile);

        switch (val.at(0))
        {
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
    switch (gamebool.at(0))
    {
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

int TestGameState::readNum(std::istream& infile)
{
    uint32_t num;
    infile.read(reinterpret_cast<char *>(&num), sizeof(num));
    return num;
}

std::string TestGameState::readNextString(std::istream& infile)
{
    auto strLength = readNum(infile);
    std::string str;
    str.resize(strLength);
    infile.read(&str[0], strLength);
    return str;
}

bool TestGameState::serialize(std::ostream& out, std::string format) 
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
    else if (format == "xml") 
    {
        return serializeXml(out);
    }
    else return false;
}

bool TestGameState::deserialize(std::istream& in, std::string format)
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
    else if (format == "xml") 
    {
        return deserializeXml(in);
    }
    else return false;
}

bool TestGameState::serializeXml(std::ostream& out)
{
    rapidxml::xml_document<> doc;
    rapidxml::xml_node<>* flagsNode = doc.allocate_node(rapidxml::node_element, "flags");

    for (auto const& p : flags) {
        std::string val;
        std::string type;

        if (std::holds_alternative<bool>(p.second)) {
            val = (std::get<bool>(p.second)) ? "true" : "false";
            type = "bool";
        } 
        else if (std::holds_alternative<int>(p.second)) {
            val = std::to_string(std::get<int>(p.second));
            type = "int";
        } 
        else if (std::holds_alternative<std::string>(p.second)) {
            val = std::get<std::string>(p.second);
            type = "string";
        }

        rapidxml::xml_node<>* keyNode = doc.allocate_node(rapidxml::node_element, "key", doc.allocate_string(p.first.c_str()));
        rapidxml::xml_node<>* valNode = doc.allocate_node(rapidxml::node_element, "val", doc.allocate_string(val.c_str()));
        rapidxml::xml_attribute<> *typeAttr = doc.allocate_attribute("type", doc.allocate_string(type.c_str()));
        valNode->append_attribute(typeAttr);

        rapidxml::xml_node<>* flagNode = doc.allocate_node(rapidxml::node_element, "flag");
        flagNode->append_node(keyNode);
        flagNode->append_node(valNode);

        flagsNode->append_node(flagNode);
    }
    doc.append_node(flagsNode);

    std::string gameEndStr = (gameEnd) ? "true" : "false";

    rapidxml::xml_node<>* gameEndNode = doc.allocate_node(rapidxml::node_element, "gameEnd", gameEndStr.c_str());
    rapidxml::xml_node<>* currRoomNode = doc.allocate_node(rapidxml::node_element, "currentRoom", currentRoom.c_str());
    doc.append_node(gameEndNode);
    doc.append_node(currRoomNode);

    out << doc;

    return true;
}

bool TestGameState::deserializeXml(std::istream& in)
{
    std::stringstream buffer;
    buffer << in.rdbuf();
    std::string str = buffer.str();

    rapidxml::xml_document<> doc;
    doc.parse<0>(str.data());

    flags.clear();
    rapidxml::xml_node<>* flagsNode = doc.first_node("flags");
    for (rapidxml::xml_node<> *flagNode = flagsNode->first_node(); flagNode; flagNode = flagNode->next_sibling()) {
        std::string key = flagNode->first_node("key")->value();
        rapidxml::xml_node<> *valNode = flagNode->first_node("val");
        std::string val = valNode->value();
        std::string type = valNode->first_attribute("type")->value();

        if (type == "bool") {
            if (val == "true") {
                flags.insert(std::make_pair(key, true));
            }
            else {
                flags.insert(std::make_pair(key, false));
            }
        }
        else if (type == "int") {
            flags.insert(std::make_pair(key, std::stoi(val)));
        }
        else if (type == "string") {
            flags.insert(std::make_pair(key, val));
        }
    }
    
    currentRoom = doc.first_node("currentRoom")->value();
    std::string gameEndStr = doc.first_node("gameEnd")->value();
    gameEnd = (gameEndStr == "true");
    return true;
}

bool TestGameState::serializeJson(std::ostream& out)
{
    rapidjson::Document d;
    rapidjson::Document::AllocatorType& allocator = d.GetAllocator(); 
    rapidjson::Value root(rapidjson::kObjectType); 
    rapidjson::Value flag(rapidjson::kObjectType); 
    rapidjson::Value key(rapidjson::kStringType); 
    rapidjson::Value value(rapidjson::kStringType); 

    std::string val;

    for (auto const& p : flags) 
    {
        key.SetString(p.first.c_str(), allocator);
        if (std::holds_alternative<bool>(p.second)) 
        {
            auto boolVal = std::get<bool>(p.second);
            if (boolVal)
            {
                val = "true";
            }
            else
            {
                val = "false";
            }
        } 
        else if (std::holds_alternative<int>(p.second)) 
        {
            auto intVal = std::get<int>(p.second);
            val = std::to_string(intVal);
        } 
        else if (std::holds_alternative<std::string>(p.second)) 
        {
            auto stringVal = std::get<std::string>(p.second);
            val = "'" + stringVal + "'";
        }
        value.SetString(val.c_str(), allocator);
        flag.AddMember(key, value, allocator);
    }
    root.AddMember("flags", flag, allocator);
    root.AddMember("gameEnd", gameEnd, allocator);
    value.SetString(currentRoom.c_str(), allocator);
    root.AddMember("currentRoom", value, allocator);

    rapidjson::OStreamWrapper osw(out);
    rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);

    root.Accept(writer);

    return true;
}

bool TestGameState::deserializeJson(std::istream& in)
{
    rapidjson::IStreamWrapper isw(in);
    // rapidjson::Reader<rapidjson::IStreamWrapper> reader(isw);
    rapidjson::Document d;
    d.ParseStream(isw);
    if (d.HasParseError())
    {
        return false;
    }

    flags.clear();
    
    for (rapidjson::Value::ConstMemberIterator it = d["flags"].MemberBegin(); it != d["flags"].MemberEnd(); ++it){
        auto val = it;
        if(val->value.IsBool())
        {
            flags[val->name.GetString()] = val->value.GetBool();
        }
        else if (val->value.IsInt())
        {
            flags[val->name.GetString()] = val->value.GetInt();
        }
        else if (val->value.IsString())
        {
            flags[val->name.GetString()] = val->value.GetString();
        }
    }

    currentRoom = d["currentRoom"].GetString();
    gameEnd = d["gameEnd"].GetBool();

    return true;
}

std::default_random_engine::result_type seed = 0;
std::default_random_engine::result_type nextSeed()
{
    return seed++;
}

// Rand_int class taken from A Tour of C++ (2nd Edition) by Bjarne Stroustrup
class Rand_int {
public:
    Rand_int(int low, int high) :dist{low,high} { }
    int operator()() { return dist(re); }
    void seed(int s) { re.seed(s); }
private:
    std::default_random_engine re;
    std::uniform_int_distribution<> dist;
};

void compareFormats(tba::GameRunner<tba::DefaultGameTalker, TestGameState>& gameRunner, std::size_t size, std::ofstream& output)
{
    Rand_int rnd {std::numeric_limits<int>::min(), std::numeric_limits<int>::max()};
    rnd.seed(nextSeed());

    std::unordered_map<std::string, std::variant<bool, int, std::string>> flags;
    for (std::size_t i = 0; i < size; ++i) {
        std::string iString = std::to_string(i);
        flags.insert(std::make_pair("testInt" + iString, rnd()));
        flags.insert(std::make_pair("testBool" + iString, i % 2 == 0));
        flags.insert(std::make_pair("testString" + iString, iString));
    }

    gameRunner.state.flags = flags;
    gameRunner.setSaveState("simple", false);
    auto [simpleSaveSuccess, simpleSaveDuration] = gameRunner.saveGame();
    auto [simpleLoadSuccess, simpleLoadDuration] = gameRunner.loadGame();
    auto simpleDuration = simpleSaveDuration + simpleLoadDuration;

    gameRunner.state.flags = flags;
    gameRunner.setSaveState("binary", true);
    auto [binSaveSuccess, binSaveDuration] = gameRunner.saveGame();
    auto [binLoadSuccess, binLoadDuration] = gameRunner.loadGame();
    auto binDuration = binSaveDuration + binLoadDuration;

    gameRunner.state.flags = flags;
    gameRunner.setSaveState("json", false);
    auto [jsonSaveSuccess, jsonSaveDuration] = gameRunner.saveGame();
    auto [jsonLoadSuccess, jsonLoadDuration] = gameRunner.loadGame();
    auto jsonDuration = jsonSaveDuration + jsonLoadDuration;

    gameRunner.state.flags = flags;
    gameRunner.setSaveState("xml", false);
    auto [xmlSaveSuccess, xmlSaveDuration] = gameRunner.saveGame();
    auto [xmlLoadSuccess, xmlLoadDuration] = gameRunner.loadGame();
    auto xmlDuration = xmlSaveDuration + xmlLoadDuration;

    output << size << ","
        << simpleDuration.count() << "," << binDuration.count() << "," << jsonDuration.count() << "," << xmlDuration.count() << ","
        << simpleSaveDuration.count() << "," << binSaveDuration.count() << ","
            << jsonSaveDuration.count() << "," << xmlSaveDuration.count() << ","
        << simpleLoadDuration.count() << "," << binLoadDuration.count() << ","
            << jsonLoadDuration.count() << "," << xmlLoadDuration.count()
        << std::endl; //intentional flushing
}

int main()
{
    std::ios_base::sync_with_stdio(false); // iostream optimization

    tba::GameRunner<tba::DefaultGameTalker, TestGameState> gameRunner {};
    tba::Room<TestGameState> startingRoom {};
    gameRunner.addStartingRoom("start", startingRoom);

    std::ofstream output("experimental_output.csv");
    if (!output) {
        std::cerr << "couldn't open 'experimental_output.csv' for writing\n";
        return 1;
    }

    output << "size,simple,binary,json,xml,simple_save,binary_save,json_save,xml_save,simple_load,binary_load,json_load,xml_load\n";

    for (std::size_t i = 2; i < 10'000'000; i = i * 2) {
        for (int j = 0; j < 3; ++j) {
            compareFormats(gameRunner, i, output);
        }
    }
}