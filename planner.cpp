#include "planner.h"
#include "goal.h"

#include <algorithm>
#include <array>
#include <set>
#include <iostream>

namespace ai
{
    namespace goap
    {
        namespace
        {
            PredicateBind from(const Condition &c)
            {
                PredicateBind bind{ static_cast<std::uint8_t>(c.index) };
                
                for (std::size_t i = 0; i < c.slots.size(); ++i)
                    bind.slots[i] = static_cast<std::uint8_t>(c.slots[i]);

                return bind;
            }
        }

        Planner::Planner(const Domain &domain) :
            _domain{ domain }
        {
        }

        Plan Planner::plan(const Goal &g)
        {
            State initial;
            State goal;

            // First create goal state and
            // calculate initial state
            for (const auto &c : g.conditions) {
                const PredicateBind bind = from(c);
                goal.set(bind, c.state);
                const Predicate &predicate = _domain.predicate(c.index);
                const bool state = predicate(nullptr, g.values, bind);
                initial.set(bind, state);
            }

            // If we already meet our goal return
            if (initial == goal)
                return{};

            _nodes.clear();
            _open.clear();
            _closed.clear();
            _values.clear();

            _values = g.values;

            // Create first node
            _open.push_back(_nodes.size());
            _nodes.push_back({ goal, 0.0, initial - goal });

            while (_open.size() > 0) {
                _closed.push_back(_open.front());
                _open.erase(_open.begin());
                const std::size_t currentId = _closed.back();
                const Node *current = &_nodes[currentId];

#if defined(_DEBUG)
                dump(current);
#endif

                // Check if current state meets goal state
                if (current->state.meets(initial)) {
                    std::vector<ActionBind> result;
                    const Node *node = current;

                    do {
                        result.push_back(node->action);
                        node = &_nodes[node->parent];
                    } while (node->parent != std::size_t(-1));

                    return{ _values, std::move(result) };
                }

                const auto &actions = _domain.actions();

                // Iterate through all actions
                for (std::size_t i = 0; i < actions.size(); ++i) {
                    const Action &action = actions[i];
                    std::size_t count{ 0 };

                    _binds.clear();
                    _ranges.clear();
                    _indices.clear();

                    // First let's see if we can connect to current state with this action
                    // This means that at least one effect can lead to this state
                    for (const auto &effect : action.effects) {
                        const auto binds = current->state.range(effect.index);
                        const std::size_t start{ _binds.size() };
                        Range range{ start, start };

                        if (binds.first == current->state._tokenMap.end()) {
                            _binds.push_back({ static_cast<std::uint8_t>(effect.index) });
                            ++range.max;
                        } else {
                            for (auto it = binds.first; it != binds.second; ++it) {
                                if (current->state.get((*it).second) == effect.state) {
                                    _binds.push_back((*it).second);
                                    ++range.max;
                                    ++count;
                                }
                            }
                        }

                        _ranges.push_back(range);
                        _indices.push_back(start);
                    }

                    if (count == 0)
                        continue;

                    do {
                        State outcome{ current->state };
                        ActionBind actionBind{ static_cast<std::uint8_t>(i) };

                        // Bind slots for action and fill state
                        if (!bindSlots(action, actionBind, outcome))
                            continue;

                        updateState(outcome, initial);

                        auto nodeEqual = [this, &outcome](std::size_t i) {
                            return _nodes[i].state == outcome;
                        };

                        auto nodeLess = [this](std::size_t l, std::size_t r) {
                            return _nodes[l] < _nodes[r];
                        };

                        auto it = std::find_if(_closed.begin(), _closed.end(), nodeEqual);

                        if (it != _closed.end())
                            continue;

                        it = std::find_if(_open.begin(), _open.end(), nodeEqual);

                        if (it == _open.end()) {
                            const std::size_t index = _nodes.size();
                            _nodes.push_back({
                                outcome,
                                current->g + action.cost,
                                initial - outcome, 
                                actionBind,
                                currentId
                            });
                            const auto it = std::lower_bound(_open.begin(), _open.end(), index, nodeLess);
                            _open.emplace(it, index);
                            current = &_nodes[currentId];
                        } else {
                            Node &node = _nodes[*it];
                            node.g = current->g + action.cost;
                            node.h = initial - outcome;
                            node.action = actionBind;
                            node.parent = currentId;
                            std::sort(_open.begin(), _open.end(), nodeLess);
                            current = &_nodes[currentId];
                        }
                    } while (next());
                }
            }

            return{};
        }

        void Planner::dump(const Node *current)
        {
            std::cout << "Partial plan" << std::endl;

            const Node *node = current;

            while (node->parent != std::size_t(-1)) {
                const Action &action = _domain.action(node->action.id);
                std::cout << action.name << "(";

                for (std::size_t i = 0; i < action.args; ++i) {
                    const std::size_t index = node->action.slots[i];
                    std::cout << _values[index].value << ", ";
                }

                std::cout << ") -> ";
                node = &_nodes[node->parent];
            };

            std::cout << std::endl;
        }

        bool Planner::next()
        {
            std::size_t index{ 0 };

            while (index < _ranges.size()) {
                const Range &range = _ranges[index];
                std::size_t &current = _indices[index];

                if (++current == range.max) {
                    current = range.min;
                    ++index;
                } else
                    break;
            }

            return index < _ranges.size();
        }

        bool Planner::bindSlots(const Action &action, ActionBind &actionBind, State &state)
        {
            // Check if action has specialized map function
            if (action.bindFunc != nullptr) {
                return action.bindFunc(action, _binds, _indices, _values, actionBind, state);
            } else {
                // TODO: revise
                // maybe add runtime checks? debug mode
                for (std::size_t i = 0; i < action.effects.size(); ++i) {
                    const std::size_t index = _indices[i];
                    const PredicateBind pred = _binds[index];
                    const Condition &effect = action.effects[i];

                    for (std::size_t i = 0; i < effect.slots.size(); ++i) {
                        const std::size_t ai = effect.slots[i];
                        actionBind.slots[ai] = pred.slots[i];
                    }

                    state.set(pred, !effect.state);
                }

                for (std::size_t i = 0; i < action.preconditions.size(); ++i) {
                    const Condition &precondition = action.preconditions[i];
                    PredicateBind pred{ static_cast<std::uint8_t>(precondition.index) };

                    for (std::size_t i = 0; i < precondition.slots.size(); ++i) {
                        const std::size_t ai = precondition.slots[i];
                        pred.slots[i] = actionBind.slots[ai];
                    }

                    state.set(pred, precondition.state);
                }

                return true;
            }

            return false;
        }

        void Planner::updateState(const State &current, State &state)
        {
            for (const auto &s : current._stateMap) {
                auto it = state._stateMap.find(s.first);

                if (it == state._stateMap.end()) {
                    const Predicate &predicate = _domain.predicate(s.first.id);
                    const bool value = predicate(nullptr, _values, s.first);
                    state.set(s.first, value);
                }
            }
        }
    }
}
