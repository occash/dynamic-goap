#include "domain.h"

#include <sstream>

namespace ai
{
    namespace goap
    {
        Domain::Domain()
        {
        }

        bool Domain::addType(const std::string &name)
        {
            const auto it = _typeMap.find(name);

            if (it != _typeMap.end()) {
                std::stringstream message;
                message << "Domain already contains " << name << " type";
                _error = message.str();
                return false;
            }

            _typeMap.insert({ name, _types.size() });
            _types.push_back({ name });

            return true;
        }

        bool Domain::addPredicate(
            const std::string &name,
            std::initializer_list<std::string> typeNames,
            PredicateFunc func
        )
        {
            const auto it = _predicateMap.find(name);

            if (it != _predicateMap.end()) {
                std::stringstream message;
                message << "Domain already contains " << name << " predicate";
                _error = message.str();
                return false;
            }

            std::vector<std::size_t> types;
            types.reserve(typeNames.size());

            for (const auto &typeName : typeNames) {
                const auto it = _typeMap.find(typeName);

                if (it == _typeMap.end()) {
                    std::stringstream message;
                    message << "Domain doesn't contains " << typeName << " type";
                    _error = message.str();
                    return false;
                }

                types.push_back((*it).second);
            }

            _predicateMap.insert({ name, _predicates.size() });
            _predicates.push_back({ name, std::move(types), func });

            return true;
        }

        bool Domain::addAction(
            const std::string &name,
            double cost,
            std::initializer_list<ArgDesc> args,
            std::initializer_list<ConditionDesc> preconditions,
            std::initializer_list<ConditionDesc> effects,
            BindFunc bindFunc
        )
        {
            const auto it = _actionMap.find(name);

            if (it != _actionMap.end()) {
                std::stringstream message;
                message << "Domain already contains " << name << " action";
                _error = message.str();
                return false;
            }

            std::unordered_map<std::string, std::size_t> argMap;
            std::size_t index{ 0 };

            for (const auto &arg : args) {
                const auto it = _typeMap.find(arg.type);

                if (it == _typeMap.end()) {
                    std::stringstream message;
                    message << "Domain doesn't contains " << arg.type << " type";
                    _error = message.str();
                    return false;
                }

                argMap.insert({ arg.name, index++ });
            }

            std::vector<Condition> pre;

            for (const auto &condition : preconditions) {
                const auto it = _predicateMap.find(condition.name);

                if (it == _predicateMap.end()) {
                    std::stringstream message;
                    message << "Domain doesn't contains " << condition.name << " predicate";
                    _error = message.str();
                    return false;
                }

                std::vector<std::size_t> args;

                for (const auto &arg : condition.args) {
                    const auto it = argMap.find(arg);

                    if (it == argMap.end()) {
                        std::stringstream message;
                        message << "Argument " << arg << " not found in " << name << " action";
                        _error = message.str();
                        return false;
                    }

                    args.push_back((*it).second);
                }

                pre.push_back({ (*it).second, args, condition.state });
            }

            std::vector<Condition> post;

            for (const auto &condition : effects) {
                const auto it = _predicateMap.find(condition.name);

                if (it == _predicateMap.end()) {
                    std::stringstream message;
                    message << "Domain doesn't contains " << condition.name << " predicate";
                    _error = message.str();
                    return false;
                }

                std::vector<std::size_t> args;

                for (const auto &arg : condition.args) {
                    const auto it = argMap.find(arg);

                    if (it == argMap.end()) {
                        std::stringstream message;
                        message << "Argument " << arg << " not found in " << name << " action";
                        _error = message.str();
                        return false;
                    }

                    args.push_back((*it).second);
                }

                post.push_back({ (*it).second, args, condition.state });
            }

            _actionMap.insert({ name, _actions.size() });
            _actions.push_back({ name, cost, args.size(), std::move(pre), std::move(post), bindFunc });

            return true;
        }

        Goal Domain::goal(
            std::initializer_list<ValueDesc> values,
            std::initializer_list<ConditionDesc> conditions
        )
        {
            std::unordered_map<std::string, std::size_t> valueMap;
            std::vector<Value> out;
            out.reserve(values.size());

            for (const auto &value : values) {
                const auto it = _typeMap.find(value.type);

                if (it == _typeMap.end()) {
                    std::stringstream message;
                    message << "Domain doesn't contains " << value.type << " type";
                    _error = message.str();
                    return{};
                }

                valueMap.insert({ value.name, out.size() });
                out.push_back({ (*it).second, value.value });
            }

            std::vector<Condition> cond;

            for (const auto &condition : conditions) {
                const auto it = _predicateMap.find(condition.name);

                if (it == _predicateMap.end()) {
                    std::stringstream message;
                    message << "Domain doesn't contains " << condition.name << " predicate";
                    _error = message.str();
                    return{};
                }

                std::vector<std::size_t> args;

                for (const auto &arg : condition.args) {
                    const auto it = valueMap.find(arg);

                    if (it == valueMap.end()) {
                        std::stringstream message;
                        message << "Argument " << arg << " not found in goal values";
                        _error = message.str();
                        return{};
                    }

                    args.push_back((*it).second);
                }

                cond.push_back({ (*it).second, args, condition.state });
            }

            return{ out, cond };
        }
    }
}