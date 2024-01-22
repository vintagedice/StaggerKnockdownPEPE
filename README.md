# PEPE - Stagger and Knockdown

## General

This is a commonlibsse-ng plugin that adds perk entry points to cause a stagger or knockdown, using the Perk Entry Point framework.
With PEPE users can create perks that cause adjustable amounts of stagger an knockdown force using the **`Mod Attack Damage`** perk entry.

This new perk entry effect uses the PEPE group **`Knockback`** and has categories **1** and **2**.

This plugin listens for hit events and will apply the stagger or knockback effect according if the perk entry point results in a value greater than or equal to 1.

## Stagger

Stagger uses **`GROUP__Knockback`** and category 1 (Perk Rank 2). The result of the perk entry is result is divided by 100 and then used as the magnitude of the stagger effect.

The stagger system from testing seems to work on a gradiant system where playback of the stagger animations is effected by the magnitude of the stagger.Additionally different animations will change according to certain value thresholds. I've not seen anyone experiment with this and post what the stagger thresholds are, so for reference from my testing here are the animation breakpoints per provided stagger (These values are divided by 100 before being passed to the stagger magic effect):

1. `(0,12.5)`
    - H2H_StaggerBackSmallest
    - H2H_StaggerBackSmall
    - 1HM_StaggerForwardSmall
    - 1HM_StaggerForwardMedium
1. `[12.5,12.5]`
    - H2H_StaggerBackSmall
    - 1HM_StaggerForwardMedium
1. `(12.5,25)`
    - H2H_StaggerBackSmall
    - H2H_StaggerBackMedium
    - 1HM_StaggerForwardMedium
    - 1HM_StaggerForwardLarge
1. `[25,25]`
    - H2H_StaggerBackMedium
    - 1HM_StaggerForwardMedium
    - 1HM_StaggerForwardLarge
1. `(25,37.5)`
    - H2H_StaggerBackMedium
    - H2H_StaggerBackLarge
    - 1HM_StaggerForwardMedium
    - 1HM_StaggerForwardLarge
1. `[37.5,37.5]`
    - H2H_StaggerBackLarge
    - 1HM_StaggerForwardLarge
1. `(37.5,50)`
    - H2H_StaggerBackLarge
    - H2H_StaggerBackLargest
    - 1HM_StaggerForwardLarge
    - 1HM_StaggerForwardLargest
1. `[50,+inf)`
    - H2H_StaggerBackLargest
    - 1HM_StaggerForwardLargest


Testing was done by creating a perk that always staggers and having it scale off of the Variable09 AV. tdetect, removeallitems, and tcai was run on an actor and hit with fists. Using OAR animation log I tracked when new animations would play for stagger. Stagger Direction Fix was installed.

## Knockdown

Knockdown uses **`GROUP__Knockback`** and category 2 (Perk Rank 3). The result of the entry point is divided by 10 and used as the force the victim is sent. At 100 perk entry result, aka 10 force, it will send them close to the distance of a werewolf knockdown. Simply adding 1 to the value in the perk entry value is enough to just make them fall in place.

If the target has the `ImmuneStrongUnrelentingForce` keyword, they are instead staggered at 100 aka hard stagger.