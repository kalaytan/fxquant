#include "optimizer.h"
#include "fx_engine.h"

namespace fx {

optimizer::optimizer(data_feeder_ptr df_ptr, strategy_generator_ptr sg_ptr,
    callback_func on_start, callback_func on_stop) :
    df_ptr_(df_ptr), sg_ptr_(sg_ptr), on_start_(on_start), on_stop_(on_stop)
{
}

bool optimizer::run()
{
    sg_ptr_->reset();

    for ( ; ; )
    {
        strategy_ptr sptr = sg_ptr_->create();

        if (!sptr)
        {
            break; // done
        }

        engine_ptr eptr = std::make_shared<fx_engine>(df_ptr_, sptr);

        df_ptr_->start();

        if (on_start_)
        {
            on_start_(sptr);
        }

        df_ptr_->stop();
        eptr->calc_closed_trades_stats();

        if (on_stop_)
        {
            on_stop_(sptr);
        }
    }

    return true;
}

} // namespace fx
