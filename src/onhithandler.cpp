#pragma once
#include "onhithandler.hpp"

#include "logger.hpp"
// This requires namespace logger = SKSE::log to be set up before include!
// Currently this is done in the precompiled header and thus all compilation units in this directory will have that
// defined.
#include <PerkEntryPointExtenderAPI.h>

using skpepe_events::OnHitEventHandler;

constexpr auto kStaggerSpellId = "SKPEPE_StaggerSpell";
constexpr auto kKnockdownSpellId = "SKPEPE_KnockdownSpell";
constexpr RE::FormID kImmuneStrongUnrelentingForceAddr = 0x172AC;

OnHitEventHandler* OnHitEventHandler::GetSingleton() {
    static OnHitEventHandler singleton{};
    return std::addressof(singleton);
}

bool OnHitEventHandler::Register() {
    static bool registered{false};
    if (registered) {
        logger::warn("Stagger and knockdown hit event already registered.");
        return true;
    }
    auto handlerSingleton = OnHitEventHandler::GetSingleton();

    auto immuneKnockdownForm = RE::TESForm::LookupByID(kImmuneStrongUnrelentingForceAddr);
    if (!immuneKnockdownForm) {
        logger::error(
            "Failed to load in ImmuneStrongUnrelentingForce keyword spell from Skyrim.esm. This shouldn't happen.");
        return false;
    }
    handlerSingleton->immuneKnockdown = immuneKnockdownForm->As<RE::BGSKeyword>();
    if (!handlerSingleton->immuneKnockdown) {
        logger::error(
            "Found ImmuneStrongUnrelentingForce form is not the appropriate type: {}. This shouldn't be possible.",
            immuneKnockdownForm->GetFormType());
        return false;
    }

    auto staggerSpellForm = RE::TESForm::LookupByEditorID(kStaggerSpellId);
    if (!staggerSpellForm) {
        logger::error("Failed to load in stagger spell from ESP. Check that SKPEPE.esl is enabled.");
        return false;
    }
    handlerSingleton->staggerSpell = staggerSpellForm->As<RE::SpellItem>();
    if (!handlerSingleton->staggerSpell) {
        logger::error(
            "Found stagger spell not the appropriate type, got {}. Check that SKPEPE.esl doesn't have conflicts in "
            "xEdit.",
            staggerSpellForm->GetFormType());
        return false;
    }
    auto knockdownSpellForm = RE::TESForm::LookupByEditorID(kKnockdownSpellId);
    if (!knockdownSpellForm) {
        logger::error("Failed to load in knockdown spell from ESP. Check that SKPEPE.esl is enabled.");
        return false;
    }
    handlerSingleton->knockdownSpell = knockdownSpellForm->As<RE::SpellItem>();
    if (!handlerSingleton->knockdownSpell) {
        logger::error(
            "Found knockdown spell not the appropriate type, got {}. Check that SKPEPE.esl doesn't have conflicts in "
            "xEdit.",
            knockdownSpellForm->GetFormType());
        return false;
    }
    RE::ScriptEventSourceHolder* eventHolder = RE::ScriptEventSourceHolder::GetSingleton();
    eventHolder->AddEventSink(handlerSingleton);
    registered = true;
    logger::info("Stagger and knockdown hit event registered.");
    return true;
}

// This isn't currently used. Can be enabled by passing -DUSENATIVESTAGGER to cmake.
// Its code taken from chocolate poise, and is how to directly call the stagger function.
// Currently not using since I still need to use an ESL with a push effect since the native push function's offsets are
// unknown to me.
static void TryStagger(RE::Actor* a_target, float a_staggerMult, RE::Actor* a_aggressor) {
    REL::Relocation<decltype(&TryStagger)> func{REL::RelocationID(36700, 37710)};
    func(a_target, a_staggerMult, a_aggressor);
}

// Helper to print out trace messages when things are missing
inline bool checkNull(void* ptr, const std::string& name) {
    if (ptr == nullptr) {
        logger::trace("Missing needed Hit Event data: {}", name);
        return true;
    }
    return false;
}

RE::BSEventNotifyControl OnHitEventHandler::ProcessEvent(const RE::TESHitEvent* event,
                                                         RE::BSTEventSource<RE::TESHitEvent>*) {
    if (!event) {
        logger::error("Hit Event Source Not Found!");
        return RE::BSEventNotifyControl::kContinue;
    }
    if (!event->cause) {
        logger::error("Hit Event Attacker Not Found!");
        return RE::BSEventNotifyControl::kContinue;
    }
    if (!event->target) {
        logger::error("Hit Event Target Not Found!");
        return RE::BSEventNotifyControl::kContinue;
    }

    logger::trace("Hit Event recieved: attacker {}, target {}", event->cause->GetDisplayFullName(),
                  event->target->GetDisplayFullName());

    // Gather necessary objects to apply spell and check if appropriate
    auto perkOwner = event->cause->As<RE::Actor>();
    if (checkNull(perkOwner, "Perk Owner Actor")) return RE::BSEventNotifyControl::kContinue;
    auto defender = event->target->As<RE::Actor>();
    if (checkNull(defender, "Attack Victom Actor")) return RE::BSEventNotifyControl::kContinue;
    if (!defender->Get3D() ||
        // Must be either alive or reanimated
        (defender->AsActorState()->GetLifeState() != RE::ACTOR_LIFE_STATE::kAlive &&
         defender->AsActorState()->GetLifeState() != RE::ACTOR_LIFE_STATE::kReanimate) ||
        // If not upright ignore
        defender->IsInRagdollState()) {
        logger::trace("Invalid target to apply Hit Event.");
        return RE::BSEventNotifyControl::kContinue;
    }
    auto attackingWeapon = RE::TESForm::LookupByID<RE::TESObjectWEAP>(event->source);
    if (checkNull(attackingWeapon, "Attacker Weapon")) return RE::BSEventNotifyControl::kContinue;
    auto caster = perkOwner->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant);
    if (!caster) {
        logger::error("Failed to product magic caster from {}", perkOwner->GetDisplayFullName());
        return RE::BSEventNotifyControl::kContinue;
    }

    // Now we actually try to apply the appropriate spell. First knockdown, and then stagger.
    float knockdownVal = 0.0f;
    float staggerVal = 0.0f;
    auto knockdownPerkResult = RE::HandleEntryPoint(RE::PerkEntryPoint::kModAttackDamage, perkOwner, &knockdownVal,
                                                    "Knockback", 2, {attackingWeapon, defender});
    if (knockdownPerkResult != PEPE::RequestResult::Success) {
        logger::error("Perk Entry Failed with result: {}", static_cast<int>(knockdownPerkResult));
        return RE::BSEventNotifyControl::kContinue;
    }
    logger::trace("knockdown val: {}", knockdownVal);
    if (defender->HasKeyword(immuneKnockdown)) {
        logger::trace("Actor immune to knockdown, setting stagger instead.");
        staggerVal = 100.0f;
    } else if (knockdownVal > 0.0f) {
        logger::trace("Applying perk knockdown effect");
        caster->CastSpellImmediate(knockdownSpell, false, defender, 1.0, false, knockdownVal / 10, perkOwner);
        //  Don't stagger if we're already knocking down the actor.
        return RE::BSEventNotifyControl::kContinue;
    }

    // No Knockdown, check if stagger, or if knockdown target was on immune we skip since it set it to 100 for hard
    // stagger replacement of knockdown
    if (staggerVal == 0.0f) {
        RE::HandleEntryPoint(RE::PerkEntryPoint::kModAttackDamage, perkOwner, &staggerVal, "Knockback", 1,
                             {attackingWeapon, defender});
        auto staggerPerkResult = RE::HandleEntryPoint(RE::PerkEntryPoint::kModAttackDamage, perkOwner, &staggerVal,
                                                      "Knockback", 1, {attackingWeapon, defender});
        if (staggerPerkResult != PEPE::RequestResult::Success) {
            logger::error("Perk Entry Failed with result: {}", static_cast<int>(staggerPerkResult));
            return RE::BSEventNotifyControl::kContinue;
        }
        logger::trace("stagger val: {}", staggerVal);
    }
    if (staggerVal > 0.0f) {
        logger::trace("Applying perk stagger effect");
#ifdef USENATIVESTAGGER
        TryStagger(defender, staggerVal / 100, perkOwner);
#else
        caster->CastSpellImmediate(staggerSpell, false, defender, 1.0, false, staggerVal / 100, perkOwner);
#endif
        return RE::BSEventNotifyControl::kContinue;
    }
    return RE::BSEventNotifyControl::kContinue;
}
