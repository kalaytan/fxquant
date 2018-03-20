#pragma once
#include <string>
#include "pugixml.hpp"
#include "types.h"
#include "symbol.h"
#include "bar_data.h"
#include "tick_data.h"

namespace fx {
namespace network {

struct message
{
    virtual std::string to_xml() const = 0;
    virtual bool from_xml(const pugi::xml_node& message_node) = 0;
};

struct ping_message : public message
{
    virtual std::string to_xml() const override;
    virtual bool from_xml(const pugi::xml_node& message_node) override;
};

struct pong_message : public message
{
    virtual std::string to_xml() const override;
    virtual bool from_xml(const pugi::xml_node& message_node) override;
};

struct bar_message : public message
{
    virtual std::string to_xml() const override;
    virtual bool from_xml(const pugi::xml_node& message_node) override;

    symbol symbol_;
    timeframe_type time_frame_;
    bar_data bar_;
};

struct tick_message : public message
{
    virtual std::string to_xml() const override;
    virtual bool from_xml(const pugi::xml_node& message_node) override;

    symbol symbol_;
    tick_data tick_;
};

struct bar_array_message : public message
{
    virtual std::string to_xml() const override;
    virtual bool from_xml(const pugi::xml_node& message_node) override;

    symbol symbol_;
    timeframe_type time_frame_;
    size_t count_;
};

struct status_message : public message
{
    virtual std::string to_xml() const override;
    virtual bool from_xml(const pugi::xml_node& message_node) override;

    status status_;
};

struct options_message : public message
{
    virtual std::string to_xml() const override;
    virtual bool from_xml(const pugi::xml_node& message_node) override;

    symbol symbol_;
    unsigned int speed_;
};

bool get_message_id(pugi::xml_node& message_node, std::string& id);

bool read_value(pugi::xml_node parent_node, const std::string& child_name, double& value);
bool read_value(pugi::xml_node parent_node, const std::string& child_name, unsigned int& value);
bool read_value(pugi::xml_node parent_node, const std::string& child_name, unsigned long long& value);
bool read_value(pugi::xml_node parent_node, const std::string& child_name, time_t& value);

} // namespace network
} // namespace fx
