#pragma once
#include <atomic>
#include <chrono>
#include <string>
#include <memory>
#include "tick_data.h"
#include "types.h"
#include "symbol.h"

namespace fx {

typedef unsigned long order_id_type;

enum class order_action { undefined, submitted, opened, closed, modified, deleted };

class base_order
{
public:
    struct custom_data
    {
        virtual ~custom_data() = default;
    };
    typedef std::shared_ptr <custom_data> custom_data_ptr;

protected:
    base_order(symbol sym, double volume, double order_price,
        double stop_loss, double take_profit, order_id_type id) :
        id_(id), symbol_(sym), volume_(volume), order_price_(order_price),
        open_price_(undefined_value<double>()), close_price_(undefined_value<double>()),
        stop_loss_(stop_loss), take_profit_(take_profit)
    {
        if (id_ == 0)
        {
            id_ = ++auto_order_id_;
        }

        //DEBUG_TRACE("base_order(): id=%ld", id_);
    }

public:
    order_id_type get_id() const
    {
        return id_;
    }

    void set_id(order_id_type id)
    {
        id_ = id;
    }

    symbol get_symbol() const
    {
        return symbol_;
    }

    double get_volume() const
    {
        return volume_;
    }

    bool has_stop_loss() const
    {
        return stop_loss_ != undefined_value<double>();
    }

    double get_stop_loss() const
    {
        return stop_loss_;
    }

    virtual bool set_stop_loss(double value)
    {
        if (value > 0.0)
        {
            stop_loss_ = value;
            return true;
        }

        return false;
    }

    void reset_stop_loss()
    {
        stop_loss_ = undefined_value<double>();
    }

    bool has_take_profit() const
    {
        return take_profit_ != undefined_value<double>();
    }

    double get_take_profit() const
    {
        return take_profit_;
    }

    virtual bool set_take_profit(double value)
    {
        if (value > 0.0)
        {
            take_profit_ = value;
            return true;
        }

        return false;
    }

    void reset_take_profit()
    {
        take_profit_ = undefined_value<double>();
    }

    double get_order_price() const
    {
        return order_price_;
    }

    double get_open_price() const
    {
        return open_price_;
    }

    void set_open_price(double open_price)
    {
        open_price_ = open_price;
    }

    double get_close_price() const
    {
        return close_price_;
    }

    void set_close_price(double close_price)
    {
        close_price_ = close_price;
    }

    const tick_data& get_open_tick() const
    {
        return open_tick_;
    }

    const tick_data& get_close_tick() const
    {
        return close_tick_;
    }

    timepoint_type get_open_time() const
    {
        return open_time_;
    }

    void set_open_time(timepoint_type t)
    {
        open_time_ = t;
    }

    timepoint_type get_close_time() const
    {
        return close_time_;
    }

    void set_close_time(timepoint_type t)
    {
        close_time_ = t;
    }

    const std::string& get_comment() const
    {
        return comment_;
    }

    void set_comment(const std::string& comment)
    {
        comment_ = comment;
    }

    custom_data_ptr get_custom_data() const
    {
        return custom_data_ptr_;
    }

    void set_custom_data(custom_data_ptr data_ptr)
    {
        custom_data_ptr_ = data_ptr;
    }

    bool is_opened() const
    {
        return open_price_ != undefined_value<double>();
    }

    bool is_closed() const
    {
        return close_price_ != undefined_value<double>();
    }

    // open the order
    virtual bool open(const tick_data& tick) = 0;

    // close the order
    virtual bool close(const tick_data& tick) = 0;

    // returns true if order is ready to open
    virtual bool check_open(const tick_data& tick) const = 0;

    // returns true if order is ready to close
    virtual bool check_close(const tick_data& tick) const = 0;

    // calculate the profit for closed order
    virtual double get_profit() const = 0;
    virtual double get_profit(const tick_data& tick) const = 0;

    virtual std::shared_ptr<base_order> clone() const = 0;

    std::string to_xml_message(order_action action = order_action::undefined) const;

protected:
    virtual std::string make_xml_header() const = 0;
    friend std::shared_ptr<base_order> from_xml_message(const std::string& xml);

protected:
    order_id_type id_; // ticket #
    const symbol symbol_;      // symbol for trading
    const double volume_;      // number of lots (0.01 lot = 1000 currency units)
    const double order_price_; // order price

    double open_price_;  // open price
    double close_price_; // close price

    double stop_loss_;   // stop loss level
    double take_profit_; // take profit level

    timepoint_type open_time_;  // open time
    timepoint_type close_time_; // close time

    std::string comment_;  // comment

    tick_data open_tick_;
    tick_data close_tick_;

    custom_data_ptr custom_data_ptr_;

    static order_id_type auto_order_id_;
};

class base_buy_order : public base_order
{
public:
    bool set_stop_loss(double value) override
    {
        if ((value == undefined_value<double>()) || (value < order_price_))
        {
            return base_order::set_stop_loss(value);
        }

        return false;
    }

    bool set_take_profit(double value) override
    {
        if ((value == undefined_value<double>()) || (value > order_price_))
        {
            return base_order::set_take_profit(value);
        }

        return false;
    }

    bool check_close(const tick_data& tick) const override final
    {
        if (is_opened() && !is_closed())
        {
            if ((has_stop_loss() && (tick.get_bid() <= stop_loss_)) ||
                (has_take_profit() && (tick.get_bid() >= take_profit_)))
            {
                return true;
            }
        }

        return false;
    }

    bool open(const tick_data& tick) override final
    {
        if (check_open(tick))
        {
            open_price_ = tick.get_ask();
            open_time_ = tick.get_time();//std::chrono::system_clock::now();
            open_tick_ = tick;
            return true;
        }

        return false;
    }

    bool close(const tick_data& tick) override final
    {
        if (is_opened() && !is_closed())
        {
            close_price_ = tick.get_bid();
            close_time_ = tick.get_time(); //std::chrono::system_clock::now();
            close_tick_ = tick;
            return true;
        }

        return false;
    }

    double get_profit() const override final;
    double get_profit(const tick_data& tick) const override final;

protected:
    base_buy_order(symbol sym, double volume, double order_price,
        double stop_loss, double take_profit, order_id_type id) :
        base_order(sym, volume, order_price, stop_loss, take_profit, id)
    {}

    base_buy_order(symbol sym, double volume, double order_price,
        int stop_loss, int take_profit, order_id_type id) :
        base_order(sym, volume, order_price
            , order_price - stop_loss * symbol_pip(sym)
            , order_price + take_profit * symbol_pip(sym), id) {}

    bool is_valid() const;
};

class base_sell_order : public base_order
{
public:
    bool set_stop_loss(double value) override
    {
        if ((value == undefined_value<double>()) || (value > order_price_))
        {
            return base_order::set_stop_loss(value);
        }

        return false;
    }

    bool set_take_profit(double value) override
    {
        if ((value == undefined_value<double>()) || (value < order_price_))
        {
            return base_order::set_take_profit(value);
        }

        return false;
    }

    bool check_close(const tick_data& tick) const override final
    {
        if (is_opened() && !is_closed())
        {
            if ((has_stop_loss() && (tick.get_ask() >= stop_loss_)) ||
                (has_take_profit() && (tick.get_ask() <= take_profit_)))
            {
                return true;
            }
        }

        return false;
    }

    bool open(const tick_data& tick) override final
    {
        if (check_open(tick))
        {
            open_price_ = tick.get_bid();
            open_time_ = tick.get_time();//std::chrono::system_clock::now();
            open_tick_ = tick;
            return true;
        }

        return false;
    }

    bool close(const tick_data& tick) override final
    {
        if (is_opened() && !is_closed())
        {
            close_price_ = tick.get_ask();
            close_time_ = tick.get_time(); //std::chrono::system_clock::now();
            close_tick_ = tick;
            return true;
        }

        return false;
    }

    double get_profit() const override final;
    double get_profit(const tick_data& tick) const override final;

protected:
    base_sell_order(symbol sym, double volume, double order_price,
        double stop_loss, double take_profit, order_id_type id) :
        base_order(sym, volume, order_price, stop_loss, take_profit, id)
    {}
    base_sell_order(symbol sym, double volume, double order_price,
        int stop_loss, int take_profit, order_id_type id) :
        base_order(sym, volume, order_price
            , order_price + stop_loss * symbol_pip(sym)
            , order_price - take_profit * symbol_pip(sym), id)
    {}

    bool is_valid() const;
};

class buy_order : public base_buy_order
{
public:
    buy_order(symbol sym, double volume, double order_price,
        double stop_loss = undefined_value<double>(),
        double take_profit = undefined_value<double>(),
        order_id_type id = 0);

    buy_order(symbol sym, double volume, double order_price,
        int stop_loss, int take_profit, order_id_type id = 0);

    bool check_open(const tick_data& tick) const override final
    {
        return !is_opened();
    }

    std::shared_ptr<base_order> clone() const override final
    {
        return std::make_shared<buy_order>(*this);
    }

private:
    std::string make_xml_header() const override;
};

class buy_limit_order : public base_buy_order
{
public:
    buy_limit_order(symbol sym, double volume, double order_price,
        double stop_loss = undefined_value<double>(),
        double take_profit = undefined_value<double>(),
        order_id_type id = 0);

    buy_limit_order(symbol sym, double volume, double order_price,
        int stop_loss, int take_profit, order_id_type id = 0);

    bool check_open(const tick_data& tick) const override final
    {
        return !is_opened() && (tick.get_ask() <= order_price_);
    }

    std::shared_ptr<base_order> clone() const override final
    {
        return std::make_shared<buy_limit_order>(*this);
    }

private:
    std::string make_xml_header() const override;
};

class buy_stop_order : public base_buy_order
{
public:
    buy_stop_order(symbol sym, double volume, double order_price,
        double stop_loss = undefined_value<double>(),
        double take_profit = undefined_value<double>(),
        order_id_type id = 0);

    buy_stop_order(symbol sym, double volume, double order_price,
        int stop_loss, int take_profit, order_id_type id = 0);

    bool check_open(const tick_data& tick) const override final
    {
        return !is_opened() && (tick.get_ask() >= order_price_);
    }

    std::shared_ptr<base_order> clone() const override final
    {
        return std::make_shared<buy_stop_order>(*this);
    }

private:
    std::string make_xml_header() const override;
};

class sell_order : public base_sell_order
{
public:
    sell_order(symbol sym, double volume, double order_price,
        double stop_loss = undefined_value<double>(),
        double take_profit = undefined_value<double>(),
        order_id_type id = 0);

    sell_order(symbol sym, double volume, double order_price,
        int stop_loss, int take_profit, order_id_type id = 0);

    bool check_open(const tick_data& tick) const override final
    {
        return !is_opened();
    }

    std::shared_ptr<base_order> clone() const override final
    {
        return std::make_shared<sell_order>(*this);
    }

private:
    std::string make_xml_header() const override;
};

class sell_limit_order : public base_sell_order
{
public:
    sell_limit_order(symbol sym, double volume, double order_price,
        double stop_loss = undefined_value<double>(),
        double take_profit = undefined_value<double>(),
        order_id_type id = 0);

    sell_limit_order(symbol sym, double volume, double order_price,
        int stop_loss, int take_profit, order_id_type id = 0);

    bool check_open(const tick_data& tick) const override final
    {
        return !is_opened() && (tick.get_bid() >= order_price_);
    }

    std::shared_ptr<base_order> clone() const override final
    {
        return std::make_shared<sell_limit_order>(*this);
    }

private:
    std::string make_xml_header() const override;
};

class sell_stop_order : public base_sell_order
{
public:
    sell_stop_order(symbol sym, double volume, double order_price,
        double stop_loss = undefined_value<double>(),
        double take_profit = undefined_value<double>(),
        order_id_type id = 0);

    sell_stop_order(symbol sym, double volume, double order_price,
        int stop_loss, int take_profit, order_id_type id = 0);

    bool check_open(const tick_data& tick) const override final
    {
        return !is_opened() && (tick.get_bid() <= order_price_);
    }

    std::shared_ptr<base_order> clone() const override final
    {
        return std::make_shared<sell_stop_order>(*this);
    }

private:
    std::string make_xml_header() const override;
};

typedef std::shared_ptr<base_order> order_ptr;
typedef std::shared_ptr<const base_order> order_cptr;

order_ptr from_xml_message(const std::string& xml, order_action& action);

} // namespace fx
