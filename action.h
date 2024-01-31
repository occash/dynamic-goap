#pragma once

#include <string>
#include <vector>

namespace ai
{
    namespace goap
    {
        struct Condition
        {
            std::size_t index;
            std::vector<std::size_t> slots;
            bool state;

            /*bool operator==(const Condition &other) const
            {
                if (index == other.index)
                    return false;

                if (slots.size() != other.slots.size())
                    return false;

                auto it = slots.begin();
                auto ot = other.slots.begin();

                for (; it != slots.end(); ++it, ++ot) {
                    if (*it != *ot)
                        return false;
                }

                if (state != other.state)
                    return false;

                return true;
            }*/
        };

        struct Action;
        union ActionBind;
        class State;

        using BindFunc = bool(*)(
            const Action &,
            const std::vector<PredicateBind> &,
            const std::vector<std::size_t> &,
            std::vector<Value> &,
            ActionBind &,
            State &
        );

        struct Action
        {
            std::string name;
            double cost;
            std::size_t args;
            std::vector<Condition> preconditions;
            std::vector<Condition> effects;
            BindFunc bindFunc;
        };

        union ActionBind
        {
            struct
            {
                std::uint8_t id;
                std::uint8_t slots[7];
            };
            std::uint64_t data;

            ActionBind() :
                data{ std::size_t(-1) }
            {
            }

            ActionBind(const std::uint8_t aid) :
                data{ std::size_t(-1) }
            {
                id = aid;
            }
        };
    }
}