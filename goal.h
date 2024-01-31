#pragma once

#include "action.h"
#include "value.h"

#include <vector>

namespace ai
{
    namespace goap
    {
        struct Goal
        {
            std::vector<Value> values;
            std::vector<Condition> conditions;
        };
    }
}