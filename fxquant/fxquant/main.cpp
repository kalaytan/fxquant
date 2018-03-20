#include <set>
#include <string>
#include <sstream>
#include <iostream>

#include "csv.h"
#include "logger.h"
#include "loader.h"
#include "config.h"
#include "mongodb.h"
#include "fx_engine.h"
#include "ctrl_handler.h"
#include "dummy_feeder.h"
#include "controlled_feeder.h"
#include "dummy_callbacks.h"
#include "strategies/ladder/ladder_strategy.h"
#include "strategies/ladder/ladder_strategy_generator.h"
#include "engine_registry.h"
#include "optimizer.h"
#include "gui_server.h"
#include "fix_client.h"

using namespace fx;
using namespace std::placeholders;

// return errors:
// 0 - no error
// 1 - can not parse command line parameters
// 2 - config file could not be read.
// 3 - can not connect to MongoDB
// 4 - cought thrown error

namespace {
std::set<strategy_ptr> optim_set;

void on_start(strategy_ptr strategy, dummy_feeder_ptr feeder_ptr)
{
    feeder_ptr->wait_for_stop();
}

void on_stop(strategy_ptr strategy)
{
    optim_set.insert(strategy);
}
}// end of namespace

int main(int argc, char* argv[])
{
    //fix_client fix("D:/Projects/kalaytan/fxquant/fxquant/fix_client.cfg");
    //fix.start();

    ctrl_handler handler;
    int err = loader(argc, argv);
    if (err > 0) { return err; }

    gui_server::instance();

#if 1
    try
    {
        //dummy_feeder_ptr feeder_ptr = std::make_shared<dummy_feeder>();
        controlled_feeder_ptr feeder_ptr = std::make_shared<controlled_feeder>();
        auto dcb_ptr = std::make_shared<dummy_data_callback>();

        auto strategy_ptr = std::make_shared<ladder_strategy>();

        engine_ptr eptr = std::make_shared<fx_engine>(
            feeder_ptr, strategy_ptr, dcb_ptr,
            std::make_shared<dummy_order_callback>());

        engine_registry::instance().add_engine(eptr);

        feeder_ptr->start();
        feeder_ptr->wait_for_stop();
        feeder_ptr->stop();

        eptr->calc_closed_trades_stats();
        eptr->calc_open_trades_stats();

        strategy_ptr->print_simple_report();

        strategy_ptr->print_spread();
    }
    catch (const std::exception& x)
    {
        std::cout << std::string(x.what());
    }
#elif 0
    try
    {
        dummy_feeder_ptr feeder_ptr = std::make_shared<dummy_feeder>();
        logger::instance().info("Step 1.");
        optimizer opt(feeder_ptr, std::make_shared<ladder_strategy_generator>(),
            std::bind(on_start, _1, feeder_ptr), on_stop);
        logger::instance().info("Step 2.");
        opt.run();
        logger::instance().info("Step 3.");
        int n = 0;

        csv c("ladder_strategy");
        c.add_line((*optim_set.begin())->get_csv_header());

        for (auto st : optim_set)
        {
            c.add_line(st->get_csv_line());

            auto& stats = st->get_stats();
            //logger::instance().info("printing report. profit: " + std::to_string(stats.profit));
            st->print_params();
            st->print_simple_report();

            logger::instance().info(st->get_csv_line());

            if (++n >= 10)
            {
                break;
            }
        }
    }
    catch (const std::exception& x)
    {
        std::cout << "ERROR. Cought error: " << std::string(x.what());
        return 4;
    }
#elif 0
    //
    //    dummy_feeder_ptr feeder_ptr = std::make_shared<dummy_feeder>();
    //    optimizer opt(feeder_ptr, std::make_shared<default_strategy_generator>(),
    //        std::bind(on_start, _1, feeder_ptr), on_stop);
    //    opt.run();
    //#else
    //    auto dcb_ptr = std::make_shared<dummy_data_callback>();
    //    std::vector<symbol> symbols = { /*symbol::eurgbp,*/ symbol::eurusd, /*symbol::gbpusd, symbol::usdchf, symbol::usdcad, symbol::nzdusd */ };
    //
    //    for (auto sym : symbols)
    //    {
    //        dummy_feeder_ptr feeder_ptr = std::make_shared<dummy_feeder>(sym, 1, 1, 2017, 365);
    //        feeder_ptr->add_callback(dcb_ptr);
    //
    //        feeder_ptr->start();
    //        feeder_ptr->wait_for_stop();
    //        feeder_ptr->stop();
    //    }
    //
    //    dcb_ptr->print_stats();
#endif

    logger::instance().info("waiting for termination signal...");
    handler.wait();

    logger::instance().info("application terminated");
    return 0;
}// end of main
