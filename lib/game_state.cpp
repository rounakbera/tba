module tba;

import <vector>;
import <string>;
import <iostream>;
import <unordered_map>;
import <variant>;

void tba::DefaultGameState::serializeJson(std::ostream& out)
{
    out << "{ flags: { ";

    for (auto const&p : flags) 
    {
        out << p.first << ": " << std::get<0>(p.second) << ", ";
    }

    out << " }, gameEnd: " << gameEnd << " ,";
    out << "currentRoom: " << currentRoom << " }";
}

void tba::DefaultGameState::serialize(std::ostream& mystream, std::string format) 
{
    if (format == "json") {
        serializeJson(mystream);
    }
    // } else if (format == "binary") {
    //     saveSerial();
    // }
    else return;
}

// void tba::DefaultGameState::deserialize(std::string format)
// {
//     if (format == "json") {
//         loadJson();
//     } else if (format == "binary") {
//         loadSerial();
//     }
//     else return;
// }



// bool tba::DefaultGameState::saveSerial()
// {
    
// }