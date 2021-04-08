module tba;

import <vector>;
import <string>;
import <sstream>;

void tba::DefaultGameState::serializeJson(std::ostringstream& out)
{
    out << "test\n\n\n";
}

void tba::DefaultGameState::serialize(std::ostringstream& mystream, std::string format) 
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