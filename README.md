# tba: text-based adventures

TBA is a C++ library for making text-based adventure games. It is written using modern C++ and exported as a module. Aspects of generic programming and functional programming style are incorporated, and the library makes heavy use of templates and concepts.

This is a final project created for the COMS 4995 Design Using C++ course with Prof. Bjarne Stroustrup in Spring 2021.

Project members:  
Rounak Bera  
Justin Chen  
Manav Goel

## Table of contents
- [tba: text-based adventures](#tba-text-based-adventures)
  - [Table of contents](#table-of-contents)
  - [Tutorial](#tutorial)
  - [Design](#design)
    - [Architectural overview](#architectural-overview)
    - [Performance measurements](#performance-measurements)
    - [Future work](#future-work)
  - [Manual](#manual)
    - [Concepts](#concepts)
    - [Types](#types)
    - [Classes](#classes)
  - [Build instructions](#build-instructions)

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

You can also define your own `GameTalker` concept. It must take and parse player input, and store an input history.

## Design

The text-based adventure game is a well-trodden genre. Our goal with this library is to make use of modern C++ features and techniques in order to make programming such a game easy and intuitive for the game developer, while also being extensible and flexible. To do this, we make use of generic programming and also take a functional approach to game events.

### Architectural overview

The `GameRunner` is a class which owns all the game information and has the primary functions for running the game. This RAII approach allows for all resources to be managed on the lifetime of the `GameRunner`.

To conform to this paradigm, `Room`s are stored as an `unordered_map` from `RoomName`s (which are `string`s) to `Room`s. Because `Room`s also contain a `unordered_map` of `Direction`s to `Room`s, this thus indirectly forms an adjacency list. Standard library functions make finding and accessing simple, and we provide helper functions to more easily create and assign new `Room`s and their connections.

The `GameRunner` is a templated class, taking `GameState` and `GameTalker` concepts. This allows developers to define their own `GameState` and `GameTalker` classes as needed. `GameTalker` does input handling, and `GameState` stores persistent game information (and provides serializing functionality); both are things which are desirable to customize.

For more quick and dirty setup, `DefaultGameState` and `DefaultGameTalker` are provided. `DefaultGameState` provides a basic `flags` container of type `unordered_map<string, variant<bool, int, string>>` to allow for flexible storage of state. This is effectively a bandaid solutions which pushes things onto the runtime, so a custom `GameState` is preferable for cleaner code on longer-term projects.

The `GameRunner` also contains functions for handling game events, moving between `Room`s, and saving/loading the game.

To allow for the greatest flexibility, we treat game events as `function`s which may be stored in `Room`s at will. These `function`s take the current `Room` and the `GameState` by reference so that they can easily modify both. We split these up into `Event`s, which are run on entering a `Room`; and `Action`s, which take string arguments and are run by the player. In each `Room`, `Action`s are stored in a `unordered_map`; `Event`s are stored in a custom ordered hash map so that they can be both easily accessed and iterated through in order.

The developer can therefore define their own game events as lambda functions and then store them in the relevant `Room`. Repeated events/actions can of course be copied to each relevant `Room`. The library provides functions to generate text-only `Event`s and `Action`s (the latter taking an `unordered_map` to deal with string arguments). The programmer can similarly define their own functions to generate often-used `Event`s and `Action`s. For example, in a combat-heavy game, the developer can create a function that generates a `function` which handles the intricacies of combat.

Finally, we provide functionality to save and load the game. The `DefaultGameState` provides two serializing formats: a simple text format, and a simple binary format. A user-provided `GameState` can also provide their own formats.

### Performance measurements


### Future work
Our current serialization functionality is limited to the `GameState`. This unfortunately means that the state of the `Room`s is not persistent. While this may be an acceptable environment to develop in, we believe that our library could become much more powerful if the `Room`s, their connections, and their associated `Event`s/`Action`s can be stored at will.

The primary limiting factor here is that `std::function` is not serializable. This is because certain information, such as the captured closure values, are stored in private class member fields. If we define our own `function` implementation, we should be able serialize the function pointer (though we may have to disable ASLR for this to work) and any closure values. This appears to have [been done before](https://stackoverflow.com/a/22772214). Though the code has not been made public, it should still be a feasible process. This change would allow the game programmer to fully express changes in `Event`/`Action` functions without having to rely on checking the `GameState` or dealing with issues of persistence.

Another issue we ran into in our implementation was Clang's limited support for C++20 features. In particular, we were hampered by the lack of module partitions, which led to our code and build chain being messier than they could have been. We also could have made use of the ranges library for string tokenization. Once these features are implemented in Clang/libc++, we will be able to make much nicer and cleaner code.

## Manual
The following is a simplified reference manual for using the TBA library. It explains important concepts, types, and classes for a game developer using the library. For more detailed information, please review the module file for definitions, and the tutorial for usage.

### Concepts
* `GameTalker`: handles and tokenizes input, and stores a history of input
* `GameState`: stores persistent game state, including whether the game has ended and the current room; must provide serializing/deserializing functions

### Types
* `EventFunc<GameState>`: an `std::function` which takes a `Room` and a `GameState` by reference, and returns a `bool` success flag and a `string` output (to be printed)
* `ActionFunc<GameState>`: an `std::function` which takes a `Room` and a `GameState` by reference, as well as a `vector<string>` of arguments, and returns a `bool` success flag and a `string` output
* `Direction`: a `string` name of a direction for a `Room` connection
* `RoomName`: a `string` name of a `Room`
* `Format`: a `string` name of a save format

### Classes
* `Event<GameState>`: wrapper for an `EventFunc`; intended to be run on entering a `Room`
  * `Event()`: constructor takes an `EventFunc`
* `Action<GameState>`: wrapper for an `ActionFunc`; intended to be run on player input
  * `Action()`: constructor takes an `ActionFunc`
* `EventMap<GameState>`: custom ordered hash map for `string`s to `Event`s
  * `map`: `unordered_map` of keys and values
  * `order`: `vector` of keys
  * `add()`: insert with replacement; returns true on new insertion
  * `emplace()`: insert without replacement; returns true on insertion
  * `erase()`: deletes entry specified by key if it exists
* `Room<GameState>`: stores room information
  * `connections`: `unordered_map` of `Direction`s to `RoomName`s
  * `events`: `EventMap` of event names to `Event`s
  * `actions`: `unordered_map` of action names to `Action`s
  * `setDescription()`: creates an `Event` which outputs specified description text
  * `setTextAction()`: creates an `Action` which outputs specified text on specified input arguments
* `GameRunner<GameTalker, GameState>`: owns all game information and provides game running functionality
  * `state`: a `GameState` which is used to store game state
  * `rooms`: an `unordered_map` of `RoomName`s to `Room`s, which stores all `Room`s in the game
  * `runGame()`: starts the game; handles the game loop
  * `getCurrentRoom()`: returns reference to the current `Room`
  * `addStartingRoom()`: stores the given `Room` and sets it as the current `Room`
  * `addConnectingRoom()`: stores the given `Room` and connects it to a specified `Room` via given `Direction`(s)
  * `goNextRoom()`: moves player to `Room` in given `Direction` as specified by the current `Room`'s `connections`
  * `setSaveState()`: sets the save `Format` and whether it is a binary or not
* `DefaultGameState`: a `GameState` class which is bundled with the library
  * `flags`: an `unordered_map` of `string` to `variant<bool, int, string>` which can be used to store custom game state
  * `gameEnd`: whether the game has ended or not
  * `currentRoom`: the name of the current `Room`

## Build instructions

Basic directory structure:
* `lib/`: library source code
* `tutorial_game/`: game using library
* `test_game/`: testing and data collection/analysis using library

The library and tutorial game are compiled separately.

To build, run `make` in each directory. Please make sure you have Clang 11 installed.

I recommend using the clangd extension for debugging and intellisense on VS Code. With the current setup, errors still show up on clangd, but it largely works once you do an initial compilation.

If you are using VS Code, I also recommend adding `""*.cppm": "cpp"` to your `.vscode/settings.json`.