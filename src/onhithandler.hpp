#pragma once
#include "RE/Skyrim.h"

namespace skpepe_events {
    inline static bool Register();

    class OnHitEventHandler : public RE::BSTEventSink<RE::TESHitEvent> {
    public:
        static OnHitEventHandler* GetSingleton();

        RE::BSEventNotifyControl ProcessEvent(const RE::TESHitEvent* a_event,
                                              RE::BSTEventSource<RE::TESHitEvent>* a_eventSource) override;
        friend bool Register();

    private:
        OnHitEventHandler() = default;
        ~OnHitEventHandler() = default;
        static bool Register();
        RE::SpellItem* staggerSpell;
        RE::SpellItem* knockdownSpell;
        RE::BGSKeyword* immuneKnockdown;
    };

    inline static bool Register() {
        return OnHitEventHandler::Register();
    }
}
