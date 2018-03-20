#pragma once
#include <map>
#include <cstdlib>
#include "types.h"
#include "utils.h"
#include "debug.h"
#include "data_callback.h"
#include "order_callback.h"

using namespace std::chrono_literals;

namespace fx {

class dummy_order_callback : public order_callback
{
    void on_order_submitted(fx_engine& eng, order_cptr order_ptr) override
    {
        DEBUG_TRACE("on_order_submitted(%lu)", order_ptr->get_id());
    }

    void on_order_opened(fx_engine& eng, order_cptr order_ptr) override
    {
        //DEBUG_TRACE("on_order_opened(%lu)", order_ptr->get_id());
    }

    void on_order_closed(fx_engine& eng, order_cptr order_ptr) override
    {
        //DEBUG_TRACE("on_order_closed(%lu). ", order_ptr->get_id());
        //DEBUG_TRACE("on_order_closed(%s). ", time_to_string(order_ptr->get_open_tick().get_time()).c_str());
        //eng.delete_order(order_ptr->get_id());
    }

    void on_order_modified(fx_engine& eng, order_cptr order_ptr) override
    {
        DEBUG_TRACE("on_order_modified(%lu)", order_ptr->get_id());
    }

    void on_order_deleted(fx_engine& eng, order_cptr order_ptr) override
    {
        DEBUG_TRACE("on_order_deleted(%lu)", order_ptr->get_id());
    }
};

class dummy_data_callback : public data_callback
{
public:
    dummy_data_callback::dummy_data_callback() :
        div_(1000.0), bar_count_(0)
    {
    }

    void print_stats() const
    {
        for (auto& bs : bar_stats_)
        {
            //double d = bs.first / div_;
            //DEBUG_TRACE(">=%.3f: %.2f%%", d, double(bs.second * 100) / bar_count_);
            ////DEBUG_TRACE("%.5f: %.2f%%", bs.first / 100000.0, double(bs.second * 100) / bar_count_);
            DEBUG_TRACE("%u: %.2f%%", bs.first, double(bs.second * 100) / bar_count_);
        }
    }

private:
    void on_tick(const tick_data& tick) override
    {
        //DEBUG_TRACE("on_tick(%lu)", tick.get_time().time_since_epoch().count());
    }

    void on_bar(timeframe_type tf, const bar_data& bar) override
    {
        //DEBUG_TRACE("on_bar(tf:%u o:%.5f c:%.5f t:%lu)", tf.count(),bar.o, bar.c, bar.t);

        if ((tf != 1h) || (bar.c <= 0.0))
        {
            return;
        }

        if (++bar_count_ > 1)
        {
            //double delta = bar.c - prev_bar_.c;
            //double pct = (delta * 100.0) / prev_bar_.c;
            //unsigned int r = static_cast<unsigned int>(abs(pct) * 1000.0);
            ////unsigned int r = fabs((prev_bar_.c - bar.o) * 100000);

            // flags:
            // 1: body
            // 2: upper shadow
            // 4: lower shadow

            unsigned int r = 0;

            if (bar.o != bar.c)
            {
                r |= 1; // has body

                if (bar.o > bar.c)
                {
                    if (bar.h > bar.o)
                    {
                        r |= 2; // has upper shadow
                    }

                    if (bar.l < bar.c)
                    {
                        r |= 4; // has lower shadow
                    }
                }
                else // bar.o < bar.c
                {
                    if (bar.h > bar.c)
                    {
                        r |= 2; // has upper shadow
                    }

                    if (bar.l < bar.o)
                    {
                        r |= 4; // has lower shadow
                    }
                }
            }
            else // bar.o == bar.c
            {
                if (bar.h > bar.o)
                {
                    r |= 2; // has upper shadow
                }

                if (bar.l < bar.c)
                {
                    r |= 4; // has lower shadow
                }
            }

            auto it = bar_stats_.find(r);

            if (it == bar_stats_.end())
            {
                bar_stats_.insert({ r, 1 });
            }
            else
            {
                it->second++;
            }
        }

        prev_bar_ = bar;
    }

private:
    const double div_;
    size_t bar_count_;
    bar_data prev_bar_;
    std::map<unsigned int, size_t> bar_stats_;
};

} // namespace fx
