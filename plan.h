#pragma once

#include "action.h"
#include "value.h"

#include <vector>

namespace ai
{
    namespace goap
    {
        struct Plan
        {
            std::vector<Value> values;
            std::vector<ActionBind> actions;
        };
    }
}