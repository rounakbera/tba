/*
    TBA library module definition.
    Because Clang does not yet support module partitions, all classes must be
    defined within this file, though they may be implemented elsewhere. This
    also means that any template classes must be implemented fully within this
    file.
*/

export module tba;

import <string>;
import <utility>;
import <chrono>;
import <iostream>;
import <vector>;
import <concepts>;
import <unordered_map>;
import <functional>;
import <variant>;
import <cstdlib>;

export namespace tba {
    // concept definitions
    template<typename T>
    concept GameTalker = requires(T t) {
        { t.history } -> std::same_as<std::vector<std::string>>;
        { t.getInput() } -> std::same_as<std::vector<std::string>>;
    };

    template<typename S>
    concept GameState = requires(S s, std::string d) {
        { s.gameEnd } -> std::same_as<bool>;
    };

    // class definitions
    enum class Format {json};

    template <GameState S> class Event;
    template <GameState S> class Action;
    template <GameState S> class Room;

    template <GameState S>
    class Event {
    public:
        std::function<std::pair<bool, std::string>(Room<S>&, S&)> run;
    };

    template <GameState S>
    class Action {
    public:
        std::function<std::pair<bool, std::string>(Room<S>&, S&, std::vector<std::string>)> run;
    };

    template <GameState S>
    class Room {
    public:
        std::unordered_map<std::string, Room<S>> connections;
        std::unordered_map<std::string, Event<S>> events;
        std::unordered_map<std::string, Action<S>> actions;
    };

    template <GameTalker T, GameState S>
    class GameRunner {
    public:
        T talker;
        S state;
        Room<S> currentRoom;
        Format saveFormat;

        void runGame();
        std::string tryAction(std::vector<std::string> args);
        void checkEvents();
        void setStartingRoom(Room<S> room);
        void goNextRoom(std::string direction);
        std::pair<bool, std::chrono::microseconds> saveGame(Format format);
        std::pair<bool, std::chrono::microseconds> loadGame(Format format);
    };

    class DefaultGameTalker {
    public:
        std::vector<std::string> history;

        std::vector<std::string> getInput();
    };
    
    class DefaultGameState {
    public:
        std::unordered_map<std::string, std::variant<bool, int, std::string>> flags;
        bool gameEnd;
    };

    // GameRunner method definitions
    template <GameTalker T, GameState S>
    void GameRunner<T, S>::runGame()
    {
        std::cout << "Hello world!\n";
        while (true) {
            std::vector<std::string> args = talker.getInput();
            std::string output = tryAction(args);
            std::cout << output << "\n";
        }
    }

    template <GameTalker T, GameState S>
    std::string GameRunner<T, S>::tryAction(std::vector<std::string> args)
    {
        std::string actionName = args[0];
        args.erase(args.begin()); // remove action from args

        // check for quit game
        if (actionName == "quit" && args.size() == 0) {
            exit(0);
        }

        // check for and validate go action
        if (actionName == "go" && (args.size() != 1 || !currentRoom.connections.contains(args[0]))) {
            return "Room not found!";
        }

        // search and run room actions
        auto actionIt = currentRoom.actions.find(actionName);
        std::string actionOutput = "";
        if (actionIt != currentRoom.actions.end()) {
            auto [success, output] = actionIt->second.run(currentRoom, state, args);
            if (success) {
                actionOutput = output;
                
            }
            else {
                actionOutput = "Action failed: " + output;
            }
        }
        else if (actionName != "go") {
            // action is not a room action or a go action
            actionOutput = "Invalid action!";
        }
        else {
            // run a valid go action
            // TODO: uncomment below when goNextRoom is implemented
            // goNextRoom(args[1]);
            actionOutput = "Moved " + args[1] + ": " + actionOutput;
        }
        return actionOutput;
    }
}