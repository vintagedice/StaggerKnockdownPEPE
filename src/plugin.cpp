#include "logger.hpp"
#include "onhithandler.hpp"

namespace {
    static void SKSEMessageHandler(SKSE::MessagingInterface::Message* message) {
        if (message->type == SKSE::MessagingInterface::kDataLoaded) {
            if (skpepe_events::Register()) {
                logger::info("Stagger and Knockdown Perk Entry Point Extensions loaded.");
            } else {
                logger::info("Failed to register Stagger and Knockdown Perk Entry Point Extensions.");
            }
        }
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    skpepe_logger::SetupLog();
    auto* plugin = SKSE::PluginDeclaration::GetSingleton();
    logger::info("Registering {}, Version {}, for load.", plugin->GetName(), plugin->GetVersion());
    SKSE::GetMessagingInterface()->RegisterListener("SKSE", SKSEMessageHandler);
    return true;
}
