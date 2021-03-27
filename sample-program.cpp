/*
sample-program.cpp

This is how I imagine how someone would use our library to create a game. 
Obviously can be changed however, but I think if we have an idea of how
someone can write the game it might help.

The game map looks like this:

    #--#
    |
    #

The player starts in the top left. They go to the east room, and talk to 
a person to get a key. They go back to the start room and try to go south.
The key they acquired lets them enter the south room, and the game is complete.

*/

#include "tba"

int main()
{
    // We can define this to take a bunch of stuff, or have some setter methods
    GameRunner myGameRunner;
    myGameRunner.setHealth(10);
    myGameRunner.defineInventory([]);
    myGameRunner.defineActions(["pick up", "go", "talk", "attack"]);

    // Define the startRoom.
    // EntryEvents are the first thing called by the GameRunner. I think they might just be flavor text for now, but
    // user can probably define if anything actually happens.
    EntryEvent startingRoomEntryEvent {"You wake up in a dusty room. There's a locked door to the south, and a dark passage to the east."};
    Room startRoom {startingRoomEntryEvent};

    // Define the things for the east room
    Item myKey {"Key", "It glints under the faint light coming through the ceiling"};
    EntryEvent eastRoomEntryEvent {"I am an old man. Here's a key.", interactions=false};
    Room eastRoom {eastRoomEntryEvent, myKey};

    // Define the bottom room, and set it as the ending.
    EntryEvent southRoomEntryEvent {"You pass through the door. This room has a portal that will take you home. You step in."};
    Room southRoom {southRoomEntryRoom, endRoom=true};

    // We could add just generic events, which are triggered by actions. So a room has an unordered_map of 
    // actions to Event's. They can take function pointers to create custom interactions? Seems messy idk.
    // Also how will events take specific things or items?

    void takeDamage()
    {
        myGameRunner.takeDamage(5);
    }

    Event exampleEvent1 {"The minotaur charges at you", *takeDamage};
    Event exampleEvent1 {"You talk to the pretty barmaid."};

    auto exampleRoomEvents = {
        "attack": exampleEvent1, 
        "talk": exampleEvent2
    };

    // Let's link the rooms, maybe use an unordered_map to just link directions
    // and the user can define how they're referred to
    startRoom.setConnectingRoom("south", bottomRoom, bidirectional=false, requiresItem=myKey);
    startRoom.setConnectingRoom("east", rightRoom, bidirectional=true);

    // Drop the GameRunner in the correct place, and begin the game. runGame handles IO and game logic
    myGameRunner.setStartingRoom(startRoom);
    myGameRunner.runGame();
}
