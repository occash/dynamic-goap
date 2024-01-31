#include <iostream>
#include <chrono>

#include "type.h"
#include "predicate.h"
#include "action.h"
#include "domain.h"
#include "goal.h"
#include "planner.h"

namespace ai
{
    namespace goap
    {
        namespace predicates
        {
            bool exists(Agent *, const std::vector<Value> &values, const PredicateBind &bind)
            {
                const std::size_t index{ bind.slots[0] };

                if (values[index].value == "tree" || values[index].value == "pile")
                    return true;

                return false;
            }

            bool near(Agent *, const std::vector<Value> &, const PredicateBind &)
            {
                return false;
            }

            bool has(Agent *, const std::vector<Value> &, const PredicateBind &)
            {
                return false;
            }

            bool inside(Agent *, const std::vector<Value> &, const PredicateBind &)
            {
                return false;
            }
        }

        namespace actions
        {
            bool gather(
                const Action &action,
                const std::vector<PredicateBind> &binds,
                const std::vector<std::size_t> &indices,
                std::vector<Value> &values,
                ActionBind &actionBind,
                State &state
            )
            {
                static const std::map<std::string, std::string> resourceMap
                {
                    { "wood", "tree" }, { "stone", "quarry" }
                };

                // Get first and only effect
                std::size_t index = indices[0];
                const PredicateBind &bind = binds[index];

                if (bind.slots[0] == std::uint8_t(-1))
                    return false;

                index = bind.slots[0];
                auto it = resourceMap.find(values[index].value);

                if (it == resourceMap.end())
                    return false;

                actionBind.slots[0] = static_cast<std::uint8_t>(values.size());
                actionBind.slots[1] = static_cast<std::uint8_t>(index);
                // TODO: add state to action binder and
                // acquire all required types and cache values
                values.push_back({ 0, (*it).second });

                // Set resource exists false in initial state
                state.set(bind, false);

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
            };
        }
    }
}

int main()
{
    using namespace ai::goap;

    Domain domain;

    domain.addType("object");

    domain.addPredicate("exists", { "object" }, predicates::exists);
    domain.addPredicate("near", { "object" }, predicates::near);
    domain.addPredicate("has", { "object" }, predicates::has);
    domain.addPredicate("inside", { "object", "object" }, predicates::inside);

    /*domain.addAction(
        "goto",
        1.0,
        {
            { "where", "object" }
        },
        {
            { "exists", { "where" }, true }
        },
        {
            { "near", { "where" }, true }
        }
    );*/

    domain.addAction(
        "pickup",
        1.0,
        {
            { "what", "object" }
        },
        {
            { "exists", { "what" }, true }
        },
        {
            { "has", { "what" }, true }
        }
    );

    domain.addAction(
        "gather",
        1.0,
        {
            { "source", "object" },
            { "resource", "object" }
        },
        {
            { "exists", { "source" }, true }
        },
        {
            { "exists", { "resource" }, true }
        },
        actions::gather
    );

    domain.addAction(
        "place",
        1.0,
        {
            { "what", "object" },
            { "where", "object" },
        },
        {
            { "has", { "what" }, true },
            { "exists", { "where" }, true },
        },
        {
            { "inside", { "what", "where" }, true }
        }
    );

    Goal goal = domain.goal(
        {
            { "what", "object", "wood" },
            { "where", "object", "pile" }
        },
        {
            { "inside", { "what", "where" }, true }
        }
    );

    /*
    State start;
    start.set({ "inside", { "wood", "pile" } }, false);
    start.set({ "has", { "wood" } }, false);
    start.set({ "near", { "wood" } }, false);
    start.set({ "near", { "pile" } }, false);
    start.set({ "near", { "tree" } }, false);
    start.set({ "exist", { "wood" } }, false);
    start.set({ "exist", { "pile" } }, true);
    start.set({ "exist", { "tree" } }, true);
    */

    Planner planner{ domain };
    planner.plan(goal);

    /*std::cout << "Plan: ";

    for (const auto &action : results)
        std::cout << action.name << " -> ";

    std::cout << "end" << std::endl;*/

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    for (int i = 0; i < 10000; ++i)
        planner.plan(goal);

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
}
