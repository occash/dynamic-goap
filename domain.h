#pragma once

#include "type.h"
#include "predicate.h"
#include "action.h"
#include "goal.h"

#include <vector>
#include <string>
#include <unordered_map>

namespace ai
{
    namespace goap
    {
        // TODO: load from yaml file
        struct ArgDesc
        {
            std::string name;
            std::string type;
        };

        struct ConditionDesc
        {
            std::string name;
            std::vector<std::string> args;
            bool state;
        };

        struct ValueDesc
        {
            std::string name;
            std::string type;
            std::string value;
        };        

        class Domain
        {
        public:
            Domain();

            bool addType(const std::string &name);
            bool addPredicate(
                const std::string &name,
                std::initializer_list<std::string> types,
                PredicateFunc func
            );
            bool addAction(
                const std::string &name,
                double cost,
                std::initializer_list<ArgDesc> args,
                std::initializer_list<ConditionDesc> preconditions,
                std::initializer_list<ConditionDesc> effects,
                BindFunc bindFunc = nullptr
            );

            Goal goal(
                std::initializer_list<ValueDesc> values,
                std::initializer_list<ConditionDesc> conditions
            );

            const Predicate &predicate(const std::size_t index) const
            {
                return _predicates[index];
            }

            const std::vector<Action> &actions() const{ return _actions; }

            const Action &action(const std::size_t index) const
            {
                return _actions[index];
            }

        private:
            friend class Planner;
            std::string _error;
            std::vector<Type> _types;
            std::unordered_map<std::string, std::size_t> _typeMap;
            std::vector<Predicate> _predicates;
            std::unordered_map<std::string, std::size_t> _predicateMap;
            std::vector<Action> _actions;
            std::unordered_map<std::string, std::size_t> _actionMap;

        };
    }
}