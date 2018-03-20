#include "debug.h"
#include "fix_client.h"
#include "quickfix/SocketInitiator.h"
#include "quickfix/ThreadedSSLSocketInitiator.h"

namespace fx {

fix_client::fix_client(const fs::path& fix_config, bool use_ssl) :
    settings_(fix_config.string()), file_store_(settings_)
{
    if (use_ssl)
    {
        initiator_ptr_ = std::make_unique<FIX::ThreadedSSLSocketInitiator>(*this, file_store_, settings_);
    }
    else
    {
        initiator_ptr_ = std::make_unique<FIX::SocketInitiator>(*this, file_store_, settings_);
    }

    DEBUG_ENSURE(!!initiator_ptr_);
}

fix_client::~fix_client()
{
    stop();
    initiator_ptr_.reset();
}

bool fix_client::start()
{
    initiator_ptr_->start();
    return true;
}

bool fix_client::stop()
{
    initiator_ptr_->stop();
    return true;
}

// FIX::Application

void fix_client::onCreate(const FIX::SessionID&)
{
}

void fix_client::onLogon(const FIX::SessionID&)
{
}

void fix_client::onLogout(const FIX::SessionID&)
{
}

void fix_client::toAdmin(FIX::Message& msg, const FIX::SessionID& sid)
{
    if (msg.getHeader().getField(FIX::FIELD::MsgType) == FIX::MsgType_Logon)
    {
        auto user = settings_.get(sid).getString("Username");
        auto pass = settings_.get(sid).getString("Password");
        auto sender_sub_id = settings_.get(sid).getString("SenderSubID");
        auto target_sub_id = settings_.get(sid).getString("TargetSubID");

        msg.setField(FIX::Username(user));
        msg.setField(FIX::Password(pass));
        msg.setField(FIX::SenderSubID(sender_sub_id));
        msg.setField(FIX::TargetSubID(target_sub_id));
    }
}

void fix_client::toApp(FIX::Message&, const FIX::SessionID&)
{
}

void fix_client::fromAdmin(const FIX::Message&, const FIX::SessionID&)
{
}

void fix_client::fromApp(const FIX::Message& msg, const FIX::SessionID& sid)
{
    message_cracker_.crack(msg, sid);
}

} // namespace fx
