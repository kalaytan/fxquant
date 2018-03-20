#pragma once
#include <memory>
#include <experimental/filesystem>
#include "quickfix/FileStore.h"
#include "quickfix/Initiator.h"
#include "quickfix/Application.h"
#include "quickfix/MessageCracker.h"
#include "quickfix/SessionSettings.h"

namespace fs = std::experimental::filesystem;

namespace fx {

class fix_client : public FIX::Application
{
public:
    explicit fix_client(const fs::path& fix_config, bool use_ssl = true);
    ~fix_client();

    bool start();
    bool stop();

private:
    // FIX::Application
    virtual void onCreate(const FIX::SessionID&) override;
    virtual void onLogon(const FIX::SessionID&) override;
    virtual void onLogout(const FIX::SessionID&) override;
    virtual void toAdmin(FIX::Message&, const FIX::SessionID&) override;
    virtual void toApp(FIX::Message&, const FIX::SessionID&) override;
    virtual void fromAdmin(const FIX::Message&, const FIX::SessionID&) override;
    virtual void fromApp(const FIX::Message&, const FIX::SessionID&) override;

    class message_cracker : public FIX::MessageCracker
    {
    private:
        virtual void onMessage(const FIX44::ExecutionReport&, const FIX::SessionID&) override {}
        virtual void onMessage(const FIX44::OrderCancelReject&, const FIX::SessionID&) override {}
        virtual void onMessage(const FIX44::MarketDataRequest&, const FIX::SessionID&) override {}
        virtual void onMessage(const FIX44::MarketDataSnapshotFullRefresh&, const FIX::SessionID&) override {}
        virtual void onMessage(const FIX44::MarketDataIncrementalRefresh&, const FIX::SessionID&) override {}
    };
    
private:
    FIX::SessionSettings settings_;
    FIX::FileStoreFactory file_store_;
    message_cracker message_cracker_;
    std::unique_ptr<FIX::Initiator> initiator_ptr_;
};

} // namespace fx
