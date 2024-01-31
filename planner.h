#pragma once

#include "domain.h"
#include "state.h"
#include "goal.h"
#include "plan.h"

#include <unordered_set>
#include <array>

/*namespace std
{
    template<> struct hash<ai::goap::Condition>
    {
        std::hash<std::size_t> hasher;
        std::hash<bool> hasher2;

        std::size_t operator()(ai::goap::Condition const &condition) const noexcept
        {
            std::size_t result{ hasher(condition.index) };

            for (const auto &slot : condition.slots)
                result = result * 31 + hasher(slot);

            result = result * 31 + hasher2(condition.state);
            return result;
        }
    };
}*/

namespace ai
{
    namespace goap
    {
        class Domain;
        struct Goal;

        class Planner
        {
        public:
            Planner(const Domain &domain);

            Plan plan(const Goal &goal);

        private:
            struct Node
            {
                State state;
                double g;
                double h;
                ActionBind action;
                std::size_t parent = std::size_t(-1);

                double f() const { return g + h; }

                bool operator<(const Node &other)
                {
                    return f() < other.f();
                }
            };

            struct Range
            {
                std::size_t min;
                std::size_t max;
            };

        private:
            void dump(const Node *node);
            bool next();
            bool bindSlots(const Action &action, ActionBind &actionBind, State &state);
            void updateState(const State &current, State &state);

        private:
            const Domain &_domain;
            std::vector<Node> _nodes;
            std::vector<std::size_t> _open;
            std::vector<std::size_t> _closed;
            std::vector<Value> _values;
            std::vector<PredicateBind> _binds;
            std::vector<Range> _ranges;
            std::vector<std::size_t> _indices;

        };
    }
}