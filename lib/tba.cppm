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
        std::function<std::pair<bool, std::string>(Room<S>, S&)> run;
    };

    template <GameState S>
    class Action {
    public:
        std::function<std::pair<bool, std::string>(Room<S>, S&, std::vector<std::string>)> run;
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
    }
}