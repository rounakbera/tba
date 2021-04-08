module tba;

import <vector>;
import <string>;
import <sstream>;
import <iostream>;

void tba::DefaultGameState::serializeJson(std::ostream& out)
{
    out << "test";
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