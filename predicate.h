#pragma once

#include "value.h"

#include <string>
#include <vector>

namespace ai
{
    namespace goap
    {
        class Agent;

        union PredicateBind
        {
            struct
            {
                std::uint8_t id;
                std::uint8_t slots[7];
            };
            std::uint64_t data;

            PredicateBind() :
                data{ std::size_t(-1) }
            {
            }

            PredicateBind(const std::uint8_t pid) :
                data{ std::size_t(-1) }
            {
                id = pid;
            }

            bool operator==(const PredicateBind &other) const
            {
                return data == other.data;
            }
        };

        using PredicateFunc = bool(*)(Agent *, const std::vector<Value> &, const PredicateBind &);

        struct Predicate
        {
            std::string name;
            std::vector<std::size_t> types;
            PredicateFunc func;

            bool operator()(Agent *a, const std::vector<Value> &v, const PredicateBind &p) const
            {
                return func(a, v, p);
            }
        };
    }
}

namespace std
{
    template<> struct hash<ai::goap::PredicateBind>
    {
        std::size_t operator()(ai::goap::PredicateBind const &token) const noexcept
        {
            return std::hash<std::uint64_t>{}(token.data);
        }
    };
}
