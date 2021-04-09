# tba: text-based adventures

TBA is a C++ library for making text-based adventure games. It is written using modern C++ and exported as a module. Aspects of generic programming and functional programming style are incorporated, and the library makes heavy use of templates and concepts.

This is a final project created for the COMS 4995 Design Using C++ course with Prof. Bjarne Stroustrup in Spring 2021.

Project members:  
Rounak Bera  
Justin Chen  
Manav Goel

## Tutorial
To illustrate library usage, we have created a tutorial game. We will construct this game here. A working expanded version of it can be found in `tutorial_game/`.

The `GameRunner` forms the central component of the game. We can start a basic game as follows:
```cpp
GameRunner<DefaultGameTalker, DefaultGameState> gameRunner {};
gameRunner.runGame();
```

Before we `runGame()`, however, we would probably like to define some game information.

The world of the game is represented using `Room`s.
```cpp
Room<DefaultGameState> mainHold {};
// define room information here...
gameRunner.addStartingRoom("main hold", mainHold);
```
`Room`s have `Event`s, which are run on entering a `Room`. We can insert a basic text `Event`, such as a description, using `Room.setDescription`.

```cpp
mainHold.setDescription("You are sitting in a passenger "
"chair in a dingy space freighter. As you look out the viewport, "
"you see the bright starlines of hyperspace flow around you.", "description");
```
The second argument sets a name for the `Event` which is unique to the `Room`. If a name is not specified, it is set to `"description"` by default.

`Event`s are actually wrappers for functions that take in the curren `Room` and `GameState` information, and a `bool` marking success and a text output. Here we peel back the abstraction layer to set a second text description more explicitly:
```cpp
EventFunc<DefaultGameState> descriptionEvent = [](auto& room, auto& state) {
    return std::make_pair(true, "You look around and see two other people. "
    "One is a man, and the other is a woman.");
};
mainHold.events.emplace("description2", Event{descriptionEvent});
```
This allows us to have more complex `Event`s which may modify `GameState` as well as `Room` information (such as available `Event`s, `Action`s, and connected `Room`s).

`Room`s also have `Action`s. These are run when the player inputs a command with the starting word as the `Action`'s name. `Action`s are specified like `Event`s, except they can also take arguments. They have a similar helper function for defining basic text-based actions:
```cpp
mainHold.setTextAction("Who would you like to greet?", "greet",
    {{"man", "The man looks up and smiles at you. \"Bored yet?\""},
    {"woman", "The woman makes eye contact with you but does not respond."}});
```

This is enough for a basic game. If we start the game, we can can see our two `Event` texts and interact with the two NPC `Action`s.
```
You are sitting in a passenger chair in a dingy space freighter. As you look out the viewport, you see the bright starlines of hyperspace flow around you.
You look around and see two other people. One is a man, and the other is a woman.

Currently available actions:
greet
go
save
load
quit

-----
> greet man

The man looks up and smiles at you. "Bored yet?"
...
-----
> greet woman

The woman makes eye contact with you but does not respond.
...
-----
> 
```
Now let's say we want to be able to poke the woman after her lack of response. To do this, we can write out the `Action` in its expanded form. This also allows us to create different cases for no arguments and invalid ones.
```cpp
ActionFunc<DefaultGameState> talkAction =
    [](auto& room, auto& state, vector<string> args) {
        if (args.empty()) {
            return make_pair(true, "Who do you want to greet?");
        }
        else if (args[0] == "man") {
            // ...
        }
        else if (args[0] == "woman") {
            // ...
        }
        return make_pair(true, "You can't greet this!");
    };
mainHold.actions.insert_or_assign("greet", Action{talkAction});
```
Then we can define the added `Action` as follows:
```cpp
else if (args[0] == "woman") {
    room.setTextAction("Who would you like to poke?", "poke",
        {{"woman", "The woman glares at you."}});
    return make_pair(true,
        "The woman makes eye contact with you but does not respond.");
}

```
Gives output:
```
-----
> greet woman

The woman makes eye contact with you but does not respond.

Currently available actions:
poke
nod
greet
go
save
load
quit

-----
> poke woman

The woman glares at you.
...
-----
> 
```
These added actions can also similarly modify the actions. For instance, we can add an action that removes itself.
```cpp
else if (args[0] == "man") {
    ActionFunc<DefaultGameState> nodAction =
        [](auto& room, auto& state, vector<string> args) {
            room.actions.erase("nod");
            return make_pair(true, "You nod. "
                "\"I knew it would happen eventually,\" he chuckles.");
        };
    room.actions.insert_or_assign("nod", Action{nodAction});
    return make_pair(true, "The man looks up and smiles at you. \"Bored yet?\"");
}
```
Gives output:
```
-----
> greet man

The man looks up and smiles at you. "Bored yet?"

Currently available actions:
poke
nod
greet
go
save
load
quit

-----
> nod

You nod. "I knew it would happen eventually," he chuckles.

Currently available actions:
poke
greet
go
save
load
quit

-----
> 
```
Note that these modifications are not currently persistent in the latest version of TBA. For now, information which cannot be lost on reloading the game should be stored in the `GameState`, which we will demonstrate later.

Finally, `Room`s can connect to other `Room`s. We can easily add a new `Room` and connect it to an existing `Room`s.
```cpp
Room<DefaultGameState> cockpit {};
Room<DefaultGameState> engineRoom {};
Room<DefaultGameState> cargoHold {};
cockpit.setDescription("You enter the cockpit. "
"The pilot is leaning back at her chair.");
// define rooms information here...

gameRunner.addConnectingRoom("up", "cockpit", cockpit, "down");
gameRunner.addConnectingRoom("left", "engine room", engineRoom, "right");
gameRunner.addConnectingRoom("left", "cargo hold", cargoHold, "back", "engine room");
```
The first argument specifies the direction to the new `Room`, the second argument is the name of the new `Room`, and the third argument is the `Room` object itself. If we want the connection to be bidirectional, we can optionally specify the reverse direction. The final argument is the old `Room` which is being connected to; it is the current `Room` by default.

You can also create connections between existing `Room`s by modifying the `GameRunner.rooms` hash table. Here we define a one-way tunnel from the cargo hold to the cockpit:
```cpp
cargoHold.connections.insert_or_assign("forward", "cockpit");
```

Note that `Room`s are passed by value. It is best to define all `Room` information before adding it if possible; if you desire to modify a `Room` after, you can look it up using its name in `GameRunner.rooms`.

The `go <roomName>` action moves the player from one `Room` to another. We can add on additional functionality to `go` by defining our own `Action` with the same name.
```cpp
mainHold.setTextAction("", "go",
    {{"up", "You climb up the ladder to the cockpit."}});
```
Gives output:
```
-----
> go up

You climb up the ladder to the cockpit.
You enter the cockpit. The pilot is leaning back at her chair.

Moved up.

Currently available actions:
...
-----
>
```
We can also use this to block the player from entering a `Room`, perhaps until some precondition is met. For instance, let's say that the tunnel from the cargo hold to the cockpit is blocked by a stowaway droid. Trying to go this way leads you to encounter the droid, and you cannot pass until you have neutralized the droid. We will store this information in the `GameState`.
```cpp
gameRunner.state.flags.insert(make_pair("is stowaway alive", true));
ActionFunc<DefaultGameState> cargoGoAction =
[](auto& room, auto& state, vector<string> args) {
    if (!args.empty() && args[0] == "forward") {
        // triggers only on going forward, and when stowaway is not dead
        bool isStowawayAlive = get<bool>(state.flags.at("is stowaway alive"));
        if (isStowawayAlive) {
            ActionFunc<DefaultGameState> attackAction =
                [](auto& room, auto& state, vector<string> args) {
                    room.actions.erase("attack");
                    state.flags.insert_or_assign("is stowaway alive", false);
                    return make_pair(true, "You blast the droid in its face."
                        "It falls over and dies.");
                };
            room.actions.insert_or_assign("attack", Action{attackAction});
            return make_pair(false, "You try to crawl forward, "
                "but you see a stowaway droid blocking the path!");
        }
        else {
            return std::make_pair(true, "You step past the burnt remains of "
                "the droid and make your way up.");
        }
    }
    return std::make_pair(true, "You exit the cargo hold.");
};
cargoHold.actions.insert_or_assign("go", Action{cargoGoAction});
```
Gives output:
```
-----
> go forward

Action failed: You try to climb forward, but you see a stowaway droid blocking the path!

Currently available actions:
befriend
attack
go
save
load
quit

-----
> attack

You blast the droid in its face. It falls over and dies.

Currently available actions:
go
save
load
quit

-----
> go forward

You step past the burnt remains of the droid and make your way forward.
You enter the cockpit. The pilot is leaning back at her chair.

Moved forward.

Currently available actions:
greet
go
save
load
quit

-----
> 
```
As can be seen above, `DefaultGameState.flags` provides an `unordered_map` of `string` to `variant<bool, int, string>`, which you allows you to easily define your own game state without providing your own `GameState` class.

For greater flexibility, you can also define your own `GameState` with additional member variables. You must provide `currentRoom` and `gameEnd` variables, as well as serializing and deserializing functions.

## Design
The `GameRunner` is a class which owns all the game information and has the primary functions for running the game.

## Manual

## Build instructions

Basic directory structure:
* `lib/`: library source code
* `tutorial_game/`: game using library

The library and tutorial game are compiled separately.

To build, run `make` in each directory. Please make sure you have Clang 11 installed.

I recommend using the clangd extension for debugging and intellisense on VS Code. With the current setup, errors still show up on clangd, but it largely works once you do an initial compilation.

If you are using VS Code, I also recommend adding `""*.cppm": "cpp"` to your `.vscode/settings.json`.