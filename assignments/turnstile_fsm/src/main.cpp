#include <iostream>
#include <map>
#include <string>
#include <functional>

enum class State { Locked, Unlocked };
enum class Event { Coin, Push };

using Transition = std::pair<State, std::function<void()>>;
using Key = std::pair<State, Event>;

std::map<Key, Transition> transitions =
{
    {{State::Locked, Event::Coin}, {State::Unlocked, [](){ std::cout << "Unlocking\n"; }}},
    {{State::Locked, Event::Push}, {State::Locked, [](){ std::cout << "Still locked\n"; }}},

    {{State::Unlocked, Event::Push}, {State::Locked, [](){ std::cout << "Locking\n"; }}},
    {{State::Unlocked, Event::Coin}, {State::Unlocked, [](){ std::cout << "Already unlocked\n"; }}}
};

Event parse_event(const std::string& input)
{
    if(input == "coin") return Event::Coin;
    if(input == "push") return Event::Push;
    throw std::runtime_error("Invalid event");
}

std::string state_to_string(State s)
{
    return (s == State::Locked) ? "Locked" : "Unlocked";
}

int main()
{
    State state = State::Locked;

    while(true)
    {
        std::cout << "\nCurrent state: " << state_to_string(state) << "\n";
        std::cout << "Enter event (coin/push/exit): ";

        std::string input;
        std::cin >> input;

        if(input == "exit") break;

        try
        {
            Event evt = parse_event(input);

            auto it = transitions.find({state, evt});
            if(it != transitions.end())
            {
                auto [next_state, action] = it->second;
                action();
                state = next_state;
                std::cout << "New state: " << state_to_string(state) << "\n";
            }
            else
            {
                std::cout << "No transition defined\n";
            }
        }
        catch(const std::exception& e)
        {
            std::cout << e.what() << "\n";
        }
    }
}
