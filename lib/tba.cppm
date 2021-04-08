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
import <algorithm>;
import <stdexcept>;

export namespace tba {
    // concept definitions
    enum class Format {json};

    template<typename T>
    concept GameTalker = requires(T t) {
        { t.history } -> std::same_as<std::vector<std::string>>;
        { t.getInput() } -> std::same_as<std::vector<std::string>>;
    };

    template<typename S>
    concept GameState = requires(S s, tba::Format f) {
        { s.gameEnd } -> std::same_as<bool>;
        { s.currentRoom } -> std::same_as<std::string>;
        { s.save(f) } -> std::same_as<std::pair<bool, std::chrono::microseconds>>;
        { s.load(f) } -> std::same_as<std::pair<bool, std::chrono::microseconds>>;
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

    using Direction = std::string;
    using RoomName = std::string;

    // class definitions

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
        std::unordered_map<Direction, RoomName> connections;
        EventMap<S> events;
        std::unordered_map<std::string, Action<S>> actions;

        bool setDescription(std::string description, std::string name="description", bool replace=true);
        bool setTextAction(std::string defaultText, std::string name,
            std::unordered_map<std::string, std::string> argsMap={}, bool replace=true);
    };

    template <GameTalker T, GameState S>
    class GameRunner {
    public:
        T talker;
        S state;
        std::unordered_map<RoomName, Room<S>> rooms;
        Format saveFormat;

        void runGame();
        std::string tryAction(std::vector<std::string> args);
        void checkEvents();

        Room<S>& getCurrentRoom();
        void addStartingRoom(RoomName roomName, Room<S> room={});
        bool addConnectingRoom(Direction direction, RoomName newRoom, Room<S> room, Direction reverseDirection="", RoomName oldRoom="");
        void goNextRoom(Direction direction);

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
        RoomName currentRoom;
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
        EventFunc<S> descriptionEvent = [=](Room<S>& r, S& s) {
            return std::make_pair(true, description);
        };
        if (replace) {
            events.add(name, Event{descriptionEvent});
            return true;
        }
        else {
            return events.emplace(name, Event{descriptionEvent});
        }
    }

    // setTextAction creates an Action which prints the specified text string
    // returns true if successfully add Event to events
    template<GameState S>
    bool Room<S>::setTextAction(std::string defaultText, std::string name,
        std::unordered_map<std::string, std::string> argsMap, bool replace)
    {
        ActionFunc<S> textAction =
            [=](Room<S>& r, S& s, std::vector<std::string> args) {
                if (!args.empty()) {
                    if (argsMap.contains(args[0])) {
                        return std::make_pair(true, argsMap.at(args[0]));
                    }
                }
                return std::make_pair(true, defaultText);
            };
        if (replace) {
            actions.insert_or_assign(name, Action{textAction});
            return true;
        }
        else {
            return actions.emplace(name, Action{textAction}).second;
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
            for (const auto& [key, action] : getCurrentRoom().actions) {
                if (key != "go" && key != "save" && key != "quit") {
                    std::cout << key << "\n";
                }
            }
            std::cout << "go\nsave\nquit\n\n-----\n";

            std::vector<std::string> args = talker.getInput();
            std::string output = tryAction(args);
            std::cout << "\n" << output << "\n";

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
        if (actionName == "quit" && args.empty()) {
            return "Quitting now...";
        }

        // check for and validate go action
        if (actionName == "go" && (args.size() != 1 || !getCurrentRoom().connections.contains(args[0]))) {
            std::cout << "\nCurrently available directions:\n";
            for (const auto& [key, action] : getCurrentRoom().connections) {
                std::cout << key << "\n";
            }
            if (args.empty()) {
                return "Where would you like to go?";
            }
            else {
                return "Location not found!";
            }
        }

        // search and run room actions
        std::string actionOutput = "";
        bool actionSuccess = true;
        if (getCurrentRoom().actions.contains(actionName)) {
            auto [success, output] = getCurrentRoom().actions.at(actionName).run(getCurrentRoom(), state, args);
            if (success) {
                actionOutput = output;
            }
            else {
                actionOutput = "Action failed: " + output;
                actionSuccess = false;
            }
        }
        else if (actionName != "go") {
            // action is not a room action or a go action
            actionOutput = "Invalid action!";
        }

        if (actionName == "go" && actionSuccess) {
            // run a valid go action
            goNextRoom(args[0]);
            std::cout << "\n" << actionOutput << "\n";
            actionOutput = "Moved " + args[0] + ".";
            checkEvents();
        }
        return actionOutput;
    }

    template <GameTalker T, GameState S>
    void GameRunner<T, S>::checkEvents()
    {
        for (const auto& key : getCurrentRoom().events.order) {
            auto [success, output] = getCurrentRoom().events.map.at(key).run(getCurrentRoom(), state);
            if (success) {
                std::cout << output << "\n";
            }
        }
    }
    
    template <GameTalker T, GameState S>
    Room<S>& GameRunner<T, S>::getCurrentRoom()
    {
        if (!rooms.contains(state.currentRoom)) {
            std::cerr << "internal error: missing current room\n";
        }
        return rooms.at(state.currentRoom);
    }

    template <GameTalker T, GameState S>
    void GameRunner<T, S>::addStartingRoom(RoomName roomName, Room<S> room)
    {
        rooms.insert_or_assign(roomName, room);
        state.currentRoom = roomName;
    }

    template <GameTalker T, GameState S>
    bool GameRunner<T, S>::addConnectingRoom(Direction direction, RoomName newRoom, Room<S> room,
        Direction reverseDirection, RoomName oldRoom)
    {
        if (oldRoom.empty()) {
            oldRoom = state.currentRoom;
        }
        
        if (!reverseDirection.empty()) {
            room.connections.insert_or_assign(reverseDirection, oldRoom);
        }

        rooms.insert_or_assign(newRoom, room);

        if (!rooms.contains(oldRoom)) {
            return false;
        }
        rooms.at(oldRoom).connections.insert_or_assign(direction, newRoom);

        return true;
    }

    template <GameTalker T, GameState S>
    void GameRunner<T, S>::goNextRoom(Direction direction)
    {
        if (getCurrentRoom().connections.contains(direction)) {
            state.currentRoom = getCurrentRoom().connections.at(direction);
        }
        else {
            std::cout << "This place does not exist!\n";
        }
    }
}