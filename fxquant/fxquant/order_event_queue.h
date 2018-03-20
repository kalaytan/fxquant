#pragma once
#include "debug.h"
#include "order.h"
#include "gui_server.h"
#include "event_queue.h"
#include "order_callback.h"

namespace fx {

class fx_engine; // forward declaration

class base_order_event : public base_event
{
public:
    base_order_event(fx_engine& eng, order_cptr optr, order_callback_ptr cb_ptr) :
        engine_(eng), order_ptr_(optr), callback_ptr_(cb_ptr)
    {
    }

protected:
    fx_engine& engine_;
    order_cptr order_ptr_;
    order_callback_ptr callback_ptr_;
};

struct order_submitted_event : public base_order_event
{
    order_submitted_event(fx_engine& eng, order_cptr optr, order_callback_ptr cb_ptr) :
        base_order_event(eng, optr, cb_ptr)
    {
    }

    void operator ()() override
    {
        gui_server::instance().on_order(order_ptr_, order_action::submitted);

        if (callback_ptr_)
        {
            callback_ptr_->on_order_submitted(engine_, order_ptr_);
        }
    }
};

struct order_opened_event : public base_order_event
{
    order_opened_event(fx_engine& eng, order_cptr optr, order_callback_ptr cb_ptr) :
        base_order_event(eng, optr, cb_ptr)
    {
    }

    void operator ()() override
    {
        gui_server::instance().on_order(order_ptr_, order_action::opened);

        if (callback_ptr_)
        {
            callback_ptr_->on_order_opened(engine_, order_ptr_);
        }
    }
};

struct order_closed_event : public base_order_event
{
    order_closed_event(fx_engine& eng, order_cptr optr, order_callback_ptr cb_ptr) :
        base_order_event(eng, optr, cb_ptr)
    {
    }

    void operator ()() override
    {
        gui_server::instance().on_order(order_ptr_, order_action::closed);

        if (callback_ptr_)
        {
            callback_ptr_->on_order_closed(engine_, order_ptr_);
        }
    }
};

struct order_deleted_event : public base_order_event
{
    order_deleted_event(fx_engine& eng, order_cptr optr, order_callback_ptr cb_ptr) :
        base_order_event(eng, optr, cb_ptr)
    {
    }

    void operator ()() override
    {
        gui_server::instance().on_order(order_ptr_, order_action::deleted);

        if (callback_ptr_)
        {
            callback_ptr_->on_order_deleted(engine_, order_ptr_);
        }
    }
};

struct order_modified_event : public base_order_event
{
    order_modified_event(fx_engine& eng, order_cptr optr, order_callback_ptr cb_ptr) :
        base_order_event(eng, optr, cb_ptr)
    {
    }

    void operator ()() override
    {
        gui_server::instance().on_order(order_ptr_, order_action::modified);

        if (callback_ptr_)
        {
            callback_ptr_->on_order_modified(engine_, order_ptr_);
        }
    }
};

class order_event_queue : public event_queue
{
public:
    order_event_queue(fx_engine& eng, order_callback_ptr ocb_ptr, size_t nthreads = 1) :
        engine_(eng), event_queue(nthreads), ocb_ptr_(ocb_ptr)
    {
    }

    ~order_event_queue()
    {
        DEBUG_TRACE("~order_event_queue()");
    }

    bool push_order_submitted_event(order_cptr optr)
    {
        return push_event(std::make_shared<order_submitted_event>(engine_, optr->clone(), ocb_ptr_));
    }

    bool push_order_opened_event(order_cptr optr)
    {
        return push_event(std::make_shared<order_opened_event>(engine_, optr->clone(), ocb_ptr_));
    }

    bool push_order_closed_event(order_cptr optr)
    {
        return push_event(std::make_shared<order_closed_event>(engine_, optr->clone(), ocb_ptr_));
    }

    bool push_order_modified_event(order_cptr optr)
    {
        return push_event(std::make_shared<order_modified_event>(engine_, optr->clone(), ocb_ptr_));
    }

    bool push_order_deleted_event(order_cptr optr)
    {
        return push_event(std::make_shared<order_deleted_event>(engine_, optr->clone(), ocb_ptr_));
    }

private:
    fx_engine& engine_;
    order_callback_ptr ocb_ptr_;
};

} // namespace fx
