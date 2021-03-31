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

export namespace tba {
    enum class Format {json};

    template<typename T>
    concept GameTalker = requires(T t) {
        { t.history } -> std::same_as<std::vector<std::string>>;
        { t.getInput() } -> std::same_as<std::vector<std::string>>;
    };

    template <GameTalker T, typename S>
    class GameRunner {
    public:
        T talker;
        S state;
        Format saveFormat;

        void runGame();
        std::string tryAction(std::vector<std::string> args);
        void checkEvents();
        std::pair<bool, std::chrono::microseconds> saveGame(Format format);
        std::pair<bool, std::chrono::microseconds> loadGame(Format format);
    };

    class DefaultGameTalker {
    public:
        std::vector<std::string> history;

        std::vector<std::string> getInput();
    };
    
    class DefaultGameState {
        
    };

    class Room {

    };

    class Event {

    };

    class Action {

    };

    template <GameTalker T, typename S>
    void tba::GameRunner<T, S>::runGame()
    {
        std::cout << "Hello world!\n";
    }
}