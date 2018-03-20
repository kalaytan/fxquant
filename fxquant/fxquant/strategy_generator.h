#pragma once
#include <memory>
#include "strategy.h"
#include "default_strategy.h"

namespace fx {

class strategy_generator
{
public:
    virtual void reset() = 0;
    virtual strategy_ptr create() = 0;

private:
    default_strategy::param_type param_;
};

typedef std::shared_ptr<strategy_generator> strategy_generator_ptr;

} // namespace fx
