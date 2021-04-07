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
import <algorithm>;

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

    // class forward declarations
    template <GameState S> class Event;
    template <GameState S> class Action;
    template <GameState S> class Room;

    // type definitions
    template<GameState S>
    using EventFunc = std::function<std::pair<bool, std::string>(Room<S>&, S&)>;

    template<GameState S>
    using ActionFunc = std::function<std::pair<bool, std::string>(Room<S>&, S&, std::vector<std::string>)>;

    // class definitions
    enum class Format {json};

    template <GameState S>
    class Event {
    public:
        EventFunc<S> run;

        Event(EventFunc<S> eventFunc);
    };

    template <GameState S>
    class Action {
    public:
        ActionFunc<S> run;

        Action(ActionFunc<S> actionFunc);
    };

    // basic implementation of an ordered hash table
    template <GameState S>
    class EventMap {
    public:
        std::unordered_map<std::string, Event<S>> map;
        std::vector<std::string> order;

        bool add(std::string key, Event<S> event);
        bool emplace(std::string key, Event<S> event);
        bool erase(std::string key);
    };

    template <GameState S>
    class Room {
    public:
        std::unordered_map<std::string, Room<S>> connections;
        EventMap<S> events;
        std::unordered_map<std::string, Action<S>> actions;

        bool setDescription(std::string description, std::string name="description", bool replace=true);
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

    // Implementation begins here:

    // Event member function definitions
    template <GameState S>
    Event<S>::Event(EventFunc<S> eventFunc)
    {
        run = eventFunc;
    }

    // Action member function definitions
    template <GameState S>
    Action<S>::Action(ActionFunc<S> actionFunc)
    {
        run = actionFunc;
    }

    // EventMap member function defintions
    template <GameState S>
    bool EventMap<S>::add(std::string key, Event<S> event)
    {
        bool isNew = map.insert_or_assign(key, event).second;
        if (isNew) {
            order.emplace_back(key);
        }
        return isNew;
    }

    template <GameState S>
    bool EventMap<S>::emplace(std::string key, Event<S> event)
    {
        bool success = map.emplace(key, event).second;
        if (success) {
            order.emplace_back(key);
        }
        return success;
    }

    template <GameState S>
    bool EventMap<S>::erase(std::string key)
    {
        if (map.erase(key) == 0) {
            return false;
        }
        order.erase(std::remove(order.begin(), order.end(), key), order.end());
        return true;
    }

    // Room member function definitions
    // setDescription creates an Event which prints the specified description string
    // returns true if successfully add Event to events
    template<GameState S>
    bool Room<S>::setDescription(std::string description, std::string name, bool replace)
    {
        EventFunc<S> descriptionEvent = [=](Room<S>&, S&) {
            return std::make_pair(true, description);
        };
        if (replace) {
            events.add("description", Event{descriptionEvent});
            return true;
        }
        else {
            return events.emplace("description", Event{descriptionEvent});
        }
    }

    // GameRunner member function definitions
    template <GameTalker T, GameState S>
    void GameRunner<T, S>::runGame()
    {
        std::cout << "Starting game...\n\n";
        checkEvents();
        while (true) {
            std::cout << "\nCurrently available actions:\n";
            for (const auto& [key, action] : currentRoom.actions) {
                std::cout << key << "\n";
            }
            std::cout << "go\nsave\nquit\n\n";

            std::vector<std::string> args = talker.getInput();
            std::string output = tryAction(args);
            std::cout << "\n-----\n" << output << "\n\n";

            if (output == "Quitting now...") {
                return;
            }
        }
    }

    template <GameTalker T, GameState S>
    std::string GameRunner<T, S>::tryAction(std::vector<std::string> args)
    {
        std::string actionName = args[0];
        args.erase(args.begin()); // remove action from args

        // check for quit game
        if (actionName == "quit" && args.size() == 0) {
            return "Quitting now...";
        }

        // check for and validate go action
        if (actionName == "go" && (args.size() != 1 || !currentRoom.connections.contains(args[0]))) {
            std::cout << "\nCurrently available directions:\n";
            for (const auto& [key, action] : currentRoom.connections) {
                std::cout << key << "\n";
            }
            return "Location not found!";
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
            checkEvents();
        }
        return actionOutput;
    }

    template <GameTalker T, GameState S>
    void GameRunner<T, S>::checkEvents()
    {
        for (const auto& key : currentRoom.events.order) {
            auto eventIt = currentRoom.events.map.find(key);
            if (eventIt != currentRoom.events.map.end()) {
                auto [success, output] = eventIt->second.run(currentRoom, state);
                if (success) {
                    std::cout << output << "\n";
                }
            }
            else {
                std::cout << "internal error: could not find event\n";
            }
        }
    }
}