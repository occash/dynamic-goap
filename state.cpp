#include "state.h"

#include <iostream>

namespace ai
{
    namespace goap
    {
        void State::set(const PredicateBind &token, bool value)
        {
            const auto it = _stateMap.find(token);

            if (it == _stateMap.end())
                _tokenMap.insert({ token.id, token });

            _stateMap.insert_or_assign(token, value);
        }

        bool State::get(const PredicateBind &token) const
        {
            const auto it = _stateMap.find(token);

            if (it == _stateMap.end())
                return false;

            return (*it).second;
        }

        bool State::meets(const State &goal) const
        {
            for (const auto &value : _stateMap) {
                const auto it = goal._stateMap.find(value.first);

                if (it == goal._stateMap.end() || (*it).second != value.second)
                    return false;
            }

            return true;
        }

        double State::operator-(const State &other) const
        {
            double result{ 0.0 };

            for (const auto &value : _stateMap) {
                const auto it = _stateMap.find(value.first);

                if (it == _stateMap.end() || (*it).second != value.second)
                    result += 1.0;
            }

            return result;
        }

        bool State::operator==(const State &other) const
        {
            return _stateMap == other._stateMap;
        }
    }
}
