module tba;

import <vector>;
import <string>;

void tba::DefaultGameState::serialize(std::ostream& out, std::string format) 
{
    if (format == "json") {
        saveJson();
    } else if (format == "binary") {
        saveSerial();
    }
    else return;
}

void tba::DefaultGameState::deserialize(std::string format)
{
    if (format == "json") {
        loadJson();
    } else if (format == "binary") {
        loadSerial();
    }
    else return;
}

bool tba::DefaultGameState::saveJson()
{
    return false
}

bool tba::DefaultGameState::saveBinary()
{
    
}