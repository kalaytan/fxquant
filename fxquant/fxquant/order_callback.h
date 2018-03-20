#pragma once
#include <memory>
#include "order.h"

namespace fx {

class fx_engine; // forward declaration

// order callback functions are called from the strategy object
// when order status is changed
struct order_callback
{
    virtual void on_order_submitted(fx_engine& eng, order_cptr order_ptr) = 0;
    virtual void on_order_opened(fx_engine& eng, order_cptr order_ptr) = 0;
    virtual void on_order_closed(fx_engine& eng, order_cptr order_ptr) = 0;
    virtual void on_order_modified(fx_engine& eng, order_cptr order_ptr) = 0;
    virtual void on_order_deleted(fx_engine& eng, order_cptr order_ptr) = 0;
    virtual ~order_callback() = default;
};

typedef std::shared_ptr<order_callback> order_callback_ptr;

} // namespace fx
