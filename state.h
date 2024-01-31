#pragma once

#include <unordered_map>
#include <map>
#include <string>

#include "predicate.h"

namespace ai
{
    namespace goap
    {
        class State
        {
        public:
            void set(const PredicateBind &token, bool value);
            bool get(const PredicateBind &token) const;
            auto range(const std::size_t index) const
            {
                return _tokenMap.equal_range(index);
            }
            bool meets(const State &goal) const;

            double operator-(const State &other) const;
            bool operator==(const State &other) const;

        private:
            friend class Planner;
            std::unordered_map<PredicateBind, bool> _stateMap;
            std::multimap<std::size_t, PredicateBind> _tokenMap;

        };
    }
}
