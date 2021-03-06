/*
 * psattack.cpp              author: hartra344 [at] gmail [dot] com
 *
 * Copyright (C) 2001-2011 Atomic Blue (info@planeshift.it, http://www.atomicblue.org)
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation (version 2 of the License)
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */


#include <psconfig.h>
#include "psattack.h"
//=============================================================================
// Crystal Space Includes
//=============================================================================
#include <iutil/document.h>
#include <csutil/xmltiny.h>
#include <csgeom/math.h>
#include <csgeom/math3d.h>

//=============================================================================
// Project Includes
//=============================================================================
#include "util/log.h"
#include "util/mathscript.h"
#include "util/psdatabase.h"
#include "engine/psworld.h"

#include "psserver.h"
#include "gem.h"
#include "client.h"
#include "cachemanager.h"
#include "entitymanager.h"
#include "commandmanager.h"
#include "progressionmanager.h"
#include "combatmanager.h"
#include "npcmanager.h"
#include "psserverchar.h"
#include "../globals.h"
#include "scripting.h"

//=============================================================================
// Local Includes
//=============================================================================
#include "psquestprereqops.h"

/**********************************************************************************************************/

psAttack::psAttack() :
    attackDelay(0),
    attackRange(0),
    aoeRadius(0),
    aoeAngle(0)
{
}

psAttack::~psAttack()
{
    delete attackDelay;
    delete attackRange;
    delete aoeRadius;
    delete aoeAngle;
}

bool psAttack::Load(iResultRow& row)
{
    id = row.GetInt("id");
    name = row["name"];
    image = row["image_name"];
    animation = row["attack_anim"];
    description = row["attack_description"];

    MathScriptEngine* mse = psserver->GetMathScriptEngine();
    damage_script = mse->FindScript(row["damage"]);
    if(!damage_script)
        return false;

    unsigned id = atoi(row["attackType"]);
    type = psserver->GetCacheManager()->GetAttackTypeByID(id);

    attackDelay = MathExpression::Create(row["delay"], "attackDelay");
    attackRange = MathExpression::Create(row["range"], "attackRange");
    aoeRadius = MathExpression::Create(row["aoe_radius"], "aoeRadius");
    aoeAngle = MathExpression::Create(row["aoe_angle"], "aoeAngle");
    outcome = psserver->GetProgressionManager()->FindScript(row["outcome"]);

    const char* req = row["requirements"];
    if(req && req[0] && !LoadPrerequisiteXML(requirements, NULL, req))
    {
        return false;
    }

    if(type)
        TypeRequirements.AttachNew(new psPrereqOpAttackType(type));
    return true;
}

bool psAttack::CanAttack(Client* client)
{
    if(TypeRequirements)
    {
        if(!TypeRequirements->Check(client->GetCharacterData()))
            return false;
    }
    if(requirements)
    {
        return requirements->Check(client->GetCharacterData());
    }
    return true;
}

bool psAttack::Attack(gemActor* attacker, gemActor* target, INVENTORY_SLOT_NUMBER slot)
{
    psCharacter* character = attacker->GetCharacterData();
    psItem* weapon = character->Inventory().GetEffectiveWeaponInSlot(slot);

    if(!attacker->StartAttack())
        return false;

    float dist;
    {
        csVector3 attackerPos, targetPos;
        iSector* attackerSector, *targetSector;

        attacker->GetPosition(attackerPos, attackerSector);
        target->GetPosition(targetPos, targetSector);

        if((attacker->GetInstance() != target->GetInstance() &&
            attacker->GetInstance() != INSTANCE_ALL &&
            target->GetInstance() != INSTANCE_ALL) ||
           !(EntityManager::GetSingleton().GetWorld()->WarpSpace(targetSector, attackerSector, targetPos)))
        {
            return false;
        }
        dist = sqrtf(csSquaredDist::PointPoint(targetPos, attackerPos));
    }

    psCombatAttackGameEvent* event =
        new psCombatAttackGameEvent(0, this, attacker, target, slot, weapon);
    MathEnvironment& env(event->env);
    env.Define("AttackID",      id);
    env.Define("AttackTypeID",  type ? type->id : 0);
    env.Define("Attacker",      attacker);
    env.Define("Target",        target);
    env.Define("OrigTarget",    target);
    env.Define("AttackSlot",    slot);
    env.Define("AttackWeapon",  weapon);
    env.Define("RelatedStatID", type ? type->related_stat : 0);
    env.Define("Distance",      dist);
    csTicks delay = attackDelay ? (csTicks)round(attackDelay->Evaluate(&env)) : 0;
    event->triggerticks = csGetTicks() + delay;

    // Save the delay in the environment for other scripts to use.
    env.Define("Delay", delay);

    character->TagEquipmentObject(slot, event->id);
    psserver->GetEventManager()->Push(event);

    return true;
}

void psAttack::Affect(psCombatAttackGameEvent* event)
{
    gemActor* attacker = event->GetAttacker();
    gemActor* target = event->GetTarget();
    if(!attacker || !target)
    {
        Debug2(LOG_COMBAT,event->GetAttackerID(),"Attacker ID: %d. Combat stopped as one participant logged off.",event->GetAttackerID());
        if(attacker)
        {
            attacker->EndAttack();
            psserver->GetCombatManager()->StopAttack(attacker);
        }
        return;
    }

    attacker->EndAttack();

    // If the attacker is no longer in attack mode, abort.
    if(attacker->GetMode() != PSCHARACTER_MODE_COMBAT)
    {
        Debug2(LOG_COMBAT,event->GetAttackerID(),"Combat stopped as attacker %d left combat mode.",event->GetAttackerID());
        return;
    }

    // If target is dead, abort.
    if(!target->IsAlive())
    {
        Debug2(LOG_COMBAT,event->GetAttackerID(),"Combat stopped as target %d is dead.",event->GetTargetID());
        psserver->GetCombatManager()->StopAttack(attacker);
        return;
    }

    // If the slot is no longer attackable, abort
    psCharacter* attacker_data = attacker->GetCharacterData();
    INVENTORY_SLOT_NUMBER slot = event->GetWeaponSlot();
    if(!attacker_data->Inventory().CanItemAttack(slot))
    {
        Debug2(LOG_COMBAT,event->GetAttackerID(),"Combat stopped as attacker no longer has an attacking item equipped. Attacker ID: %d",event->GetAttackerID());
        psserver->GetCombatManager()->StopAttack(attacker);
        return;
    }

    psItem* weapon = attacker_data->Inventory().GetEffectiveWeaponInSlot(slot);

    // weapon became unwieldable
    csString response;
    if(weapon && !weapon->CheckRequirements(attacker_data, response))
    {
        Debug2(LOG_COMBAT, event->GetAttackerID(), "%s has lost use of weapon", attacker->GetName());
        psserver->GetCombatManager()->StopAttack(attacker);
        psserver->SendSystemError(event->GetAttackerID(), "You can't use your %s any more.", weapon->GetName());
        return;
    }

    Client* attacker_client = attacker->GetClient();
    if(attacker_client)
    {
        // Input the stamina data
        MathEnvironment env;
        env.Define("Actor", event->GetAttacker());
        env.Define("Weapon", weapon);

        (void) psserver->GetCacheManager()->GetStaminaCombat()->Evaluate(&env);

        MathVar* PhyDrain = env.Lookup("PhyDrain");
        MathVar* MntDrain = env.Lookup("MntDrain");

        // stop the attack if the attacker has no stamina left
        if((attacker_data->GetStamina(true) < PhyDrain->GetValue()) ||
           (attacker_data->GetStamina(false) < MntDrain->GetValue()))
        {
            psserver->GetCombatManager()->StopAttack(attacker);
            psserver->SendSystemError(event->GetAttackerID(), "You are too tired to attack.");
            return;
        }

        // If the target has become impervious, abort and give up attacking
        csString msg;
        if(!attacker->IsAllowedToAttack(target, msg))
        {
            psserver->GetCombatManager()->StopAttack(attacker);
            return;
        }

        //I will be seeing how this affects things and make changes accordingly
        // If the target has changed, abort (assume another combat event has started since we are still in attack mode)
        if(target != attacker_client->GetTargetObject())
        {
            Debug2(LOG_COMBAT,event->GetAttackerID(),"Skipping attack, Target changed for attacker ID: %d.",event->GetAttackerID());
            return;
        }
    }
    else
    {
        // Check if the npc's target has changed.
        // If it has, then assume another combat event has started.
        gemNPC* npcAttacker = attacker->GetNPCPtr();
        if(npcAttacker && npcAttacker->GetTarget() != target)
        {
            Debug2(LOG_COMBAT,event->GetAttackerID(),"Skipping attack, Target changed for attacker ID: %d.",event->GetAttackerID());
            return;
        }
    }

    // If the weapon in the slot has been changed,
    // skip a turn (latency for this slot may also have changed).
    if(event->GetWeapon() != weapon)
    {
        Debug2(LOG_COMBAT, attacker->GetClientID(), "Skipping attack because %s has changed weapons mid battle", attacker->GetName());
    }
    else if(attacker->IsSpellCasting())
    {
        psserver->SendSystemInfo(event->GetAttackerID(), "You can't attack while casting spells.");
    }
    else
    {
        int attack_result;

        if(weapon->GetIsRangeWeapon() && weapon->GetUsesAmmo())
        {
            INVENTORY_SLOT_NUMBER otherHand =
                event->GetWeaponSlot() == PSCHARACTER_SLOT_RIGHTHAND ?
                    PSCHARACTER_SLOT_LEFTHAND: PSCHARACTER_SLOT_RIGHTHAND;
            psItem* otherItem = attacker_data->Inventory().GetInventoryItem(otherHand);
            uint32_t item_id = 0;
            if(weapon->GetAmmoType() == PSITEMSTATS_AMMOTYPE_ROCKS)
            {
                // Rocks are their own ammo.
                attack_result = ATTACK_NOTCALCULATED;
                item_id = weapon->GetUID();
            }
            else if(otherItem == NULL)
            {
                attack_result = ATTACK_OUTOFAMMO;
            }
            else if(otherItem->GetIsContainer())  // Is it a quiver?
            {
                // Pick the first ammo we can shoot from the container
                // And set it as the active ammo
                attack_result = ATTACK_OUTOFAMMO;
                for(size_t i=1; i<attacker_data->Inventory().GetInventoryIndexCount(); i++)
                {
                    psItem* currItem = attacker_data->Inventory().GetInventoryIndexItem(i);
                    if(currItem &&
                       currItem->GetContainerID() == item_id &&
                       weapon->UsesAmmoType(currItem->GetBaseStats()->GetUID()))
                    {
                        otherItem = currItem;
                        attack_result = ATTACK_NOTCALCULATED;
                        item_id = otherItem->GetUID();
                        break;
                    }
                }
            }
            else if(!weapon->UsesAmmoType(otherItem->GetBaseStats()->GetUID()))
            {
                attack_result = ATTACK_OUTOFAMMO;
            }
            else
            {
                attack_result = ATTACK_NOTCALCULATED;
                item_id = otherItem->GetUID();
            }

            if(attack_result != ATTACK_OUTOFAMMO)
            {
                psItem* usedAmmo = attacker_data->Inventory().RemoveItemID(item_id, 1);
                if(usedAmmo)
                {
                    attack_result = CalculateAttack(event, usedAmmo);
                    usedAmmo->Destroy();
                    psserver->GetCharManager()->UpdateItemViews(attacker_client->GetClientNum());
                }
                else
                    attack_result = CalculateAttack(event);
            }
        }
        else
        {
            attack_result = CalculateAttack(event);
        }

        int affectedCount = 1;
        AffectTarget(target, event, attack_result);

        // Handle AOE (Area of Effect) if attack caused some damage.
        float radius = aoeRadius ? aoeRadius->Evaluate(&event->env) : 0.0;
        if(radius >= 0.01f && attack_result == ATTACK_DAMAGE)
        {
            csVector3 attackerPos;
            csVector3 targetPos;
            iSector* attackerSector, *targetSector;
            InstanceID targetInstance = target->GetInstance();

            attacker->GetPosition(attackerPos, attackerSector);
            target->GetPosition(targetPos, targetSector);

            // directional vector for a line from attacker to original target
            csVector3 attackerToTarget;
            attackerToTarget = targetPos - attackerPos;

            float angle = aoeAngle ? aoeAngle->Evaluate(&event->env) : 0.0;
            if(angle <= SMALL_EPSILON || angle > 360)
                angle = 360;
            angle = (angle/2)*(PI/180); // convert degrees to radians

            MathEnvironment& env(event->env);
            env.Define("AOE_Radius", radius);
            env.Define("AOE_Angle", angle);

            csArray<gemObject*> nearby = psserver->entitymanager->GetGEM()->FindNearbyEntities(targetSector, targetPos, targetInstance, radius);
            for(size_t i = 0; i < nearby.GetSize(); i++)
            {
                gemActor* nearby_target = nearby[i]->GetActorPtr();
                if(nearby_target == attacker || nearby_target == target)
                    continue;

                csString msg;
                if(!attacker->IsAllowedToAttack(nearby_target, msg))
                    continue;

                if(angle < PI)
                {
                    csVector3 attackerToAffected = nearby_target->GetPosition();

                    // obtain a directional line for the vector from attacker to affacted target
                    // note that this line does not originate at the original target because the
                    // cone that shall include the hittable area shall originate at the attacker
                    attackerToAffected -= attackerPos;

                    // Angle between the line original target->attacker and original target->affected target
                    float cosATAngle = (attackerToAffected * attackerToTarget) /
                        (attackerToAffected.Norm() * attackerToTarget.Norm());

                    // cap the value to meaningful ones to account for rounding issues
                    if(cosATAngle > 1.0f || CS::IsNaN(cosATAngle))
                        cosATAngle = 1.0f;
                    if(cosATAngle < -1.0f)
                        cosATAngle = -1.0f;

                    // use the absolute value of the angle here to account
                    // for both sides equally - see above
                    if(fabs(acosf(cosATAngle)) >= angle)
                        continue;
                }

                // XXX recompute attack_result?
                AffectTarget(nearby_target, event, attack_result);
                affectedCount++;
            }
            psserver->SendSystemInfo(attacker->GetClientID(),
                    "%s affected %d %s.",
                    name.GetData(), affectedCount,
                    (affectedCount == 1) ? "target" : "targets");
        }

        // Get the next special attack to execute.
        psAttack* attack = attacker_data->GetAttackQueue()->First();
        if(!attack || !attack->CanAttack(attacker_client))
        {
            // Use the default attack type.
            attack = psserver->GetCacheManager()->GetAttackByID(attacker->GetDefaultAttackID());
        }

        // Schedule the next attack.
        attack->Attack(attacker, target, slot);
    }
}

int psAttack::CalculateAttack(psCombatAttackGameEvent* event, psItem* subWeapon)
{
    INVENTORY_SLOT_NUMBER otherHand =
        event->GetWeaponSlot() == PSCHARACTER_SLOT_LEFTHAND ?
            PSCHARACTER_SLOT_RIGHTHAND : PSCHARACTER_SLOT_LEFTHAND;
    event->AttackLocation = (INVENTORY_SLOT_NUMBER) psserver->GetCombatManager()->targetLocations[psserver->rng->Get((int) psserver->GetCombatManager()->targetLocations.GetSize())];

    gemActor* attacker = event->GetAttacker();
    gemActor* target = event->GetTarget();
    psCharacter* target_data = target->GetCharacterData();

    // calculate difference between target and attacker location - to be used for angle validation
    csVector3 diff;
    {
        csVector3 attackerPos, targetPos;
        iSector* attackerSector, *targetSector;

        attacker->GetPosition(attackerPos, attackerSector);
        target->GetPosition(targetPos, targetSector);

        if((attacker->GetInstance() != target->GetInstance() &&
            attacker->GetInstance() != INSTANCE_ALL &&
            target->GetInstance() != INSTANCE_ALL) ||
           !(EntityManager::GetSingleton().GetWorld()->WarpSpace(targetSector, attackerSector, targetPos)))
        {
            return ATTACK_OUTOFRANGE;
        }
        diff = targetPos - attackerPos;
    }

    MathEnvironment& env(event->env);
    env.Define("AttackWeaponSecondary", subWeapon);
    env.Define("TargetWeapon",          target_data->Inventory().GetEffectiveWeaponInSlot(event->GetWeaponSlot(), true));
    env.Define("TargetWeaponSecondary", target_data->Inventory().GetEffectiveWeaponInSlot(otherHand, true));
    env.Define("AttackLocationItem",    target_data->Inventory().GetEffectiveArmorInSlot(event->AttackLocation));
    env.Define("DiffX",                 diff.x ? diff.x : 0.00001F); // force minimal value
    env.Define("DiffY",                 diff.y ? diff.y : 0.00001F); // force minimal value
    env.Define("DiffZ",                 diff.z ? diff.z : 0.00001F); // force minimal value

    float range = attackRange ? attackRange->Evaluate(&env) : 0.0;
    env.Define("Range", range);

    (void) damage_script->Evaluate(&env);

    if(DoLogDebug2(LOG_COMBAT, event->GetAttackerID()))
    {
        CPrintf(CON_DEBUG, "Variables for Calculate Damage (%s vs. %s):\n",
                attacker->GetName(), target->GetName());
        env.DumpAllVars();
    }

    MathVar* badrange = env.Lookup("BadRange");    // BadRange = Target is too far away
    MathVar* badangle = env.Lookup("BadAngle");    // BadAngle = Attacker doesn't aim at enemy
    MathVar* missed   = env.Lookup("Missed");      // Missed   = Attack missed the enemy
    MathVar* dodged   = env.Lookup("Dodged");      // Dodged   = Attack dodged  by enemy
    MathVar* blocked  = env.Lookup("Blocked");     // Blocked  = Attack blocked by enemy
    MathVar* damage   = env.Lookup("FinalDamage"); // Actual damage done, if any

    if(badrange && badrange->GetValue() < 0.0)
        return ATTACK_OUTOFRANGE;
    else if(badangle && badangle->GetValue() < 0.0)
        return ATTACK_BADANGLE;
    else if(missed && missed->GetValue() < 0.0)
        return ATTACK_MISSED;
    else if(dodged && dodged->GetValue() < 0.0)
        return ATTACK_DODGED;
    else if(blocked && blocked->GetValue() < 0.0)
        return ATTACK_BLOCKED;

    event->FinalDamage = damage->GetValue();

    return ATTACK_DAMAGE;
}

void psAttack::AffectTarget(gemActor* target, psCombatAttackGameEvent* event, int attack_result)
{
    // Redefine the target
    event->env.Define("Target", target);

    gemActor* attacker = event->GetAttacker();
    psCharacter* attacker_data = attacker->GetCharacterData();
    psCharacter* target_data = target->GetCharacterData();

    MathVar* weaponDecay = NULL;
    MathVar* blockDecay = NULL;
    MathVar* armorDecay = NULL;

    psItem* weapon         = event->GetWeapon();
    psItem* blockingWeapon = target_data->Inventory().GetEffectiveWeaponInSlot(event->GetWeaponSlot(), true);
    psItem* struckArmor    = target_data->Inventory().GetEffectiveArmorInSlot(event->AttackLocation);

    // there may only be a decay if you actually hit your target by some means
    if(attack_result == ATTACK_DAMAGE || attack_result == ATTACK_BLOCKED)
    {
        // We are guaranteed some armor is present:
        // real one, race one or base one.
        CS_ASSERT(struckArmor);
        float ArmorVsWeapon = weapon->GetArmorVSWeaponResistance(struckArmor->GetBaseStats());

        // clamp value between 0 and 1
        ArmorVsWeapon = ArmorVsWeapon > 1.0F ? 1.0F : ArmorVsWeapon < 0.0F ? 0.0F : ArmorVsWeapon;

        MathEnvironment& env(event->env);
        env.Define("Weapon", weapon);                             // weapon used in the attack
        env.Define("BlockingWeapon", blockingWeapon);             // weapon that blocked the attack
        env.Define("Armor", struckArmor);                         // armor hit
        env.Define("ArmorVsWeapon", ArmorVsWeapon);               // armor vs weapon effectiveness
        env.Define("Blocked", (attack_result == ATTACK_BLOCKED)); // identifies whether this attack was blocked

        (void) psserver->GetCacheManager()->GetCalcDecay()->Evaluate(&env);

        weaponDecay = env.Lookup("WeaponDecay");
        blockDecay  = env.Lookup("BlockingDecay");
        armorDecay  = env.Lookup("ArmorDecay");
    }

    switch(attack_result)
    {
        case ATTACK_DAMAGE:
        {
            bool isNearlyDead = false;
            if(target_data->GetMaxHP().Current() > 0.0 &&
               target_data->GetHP()/target_data->GetMaxHP().Current() > 0.2)
            {
                if((target_data->GetHP() - event->FinalDamage) / target_data->GetMaxHP().Current() <= 0.2)
                    isNearlyDead = true;
            }

            psCombatEventMessage ev(event->GetAttackerID(),
                                    isNearlyDead ? psCombatEventMessage::COMBAT_DAMAGE_NEARLY_DEAD : psCombatEventMessage::COMBAT_DAMAGE,
                                    attacker->GetEID(),
                                    target->GetEID(),
                                    event->AttackLocation,
                                    event->FinalDamage,
                                    weapon->GetAttackAnimID(attacker_data),
                                    target->FindAnimIndex(animation));

            ev.Multicast(target->GetMulticastClients(), 0, MAX_COMBAT_EVENT_RANGE);

            // Apply final damage
            if(outcome)
                outcome->Run(&event->env);
            else
                target->DoDamage(attacker, event->FinalDamage);

            attacker->InvokeAttackScripts(target, weapon);
            target->InvokeDefenseScripts(attacker, weapon);
            if(isNearlyDead)
            {
                target->InvokeNearlyDeadScripts(attacker, weapon);
            }

            // change target only if there is not target set yet
            if(!target->GetTargetObject()) {
                // If the target wasn't in combat, it is now...
                // Note that other modes shouldn't be interrupted automatically
                if(target->GetMode() == PSCHARACTER_MODE_PEACE ||
                   target->GetMode() == PSCHARACTER_MODE_WORK)
                {
                    if(target->GetClient())   // Set reciprocal target
                    {
                        target->GetClient()->SetTargetObject(attacker, true);
                        target->SendTargetStatDR(target->GetClient());
                    }

                    // The default stance is 'Fully Defensive'.
                    const Stance& initialStance(psserver->GetCombatManager()->GetStance(psserver->GetCacheManager(), "FullyDefensive"));
                    psserver->GetCombatManager()->AttackSomeone(target, attacker, initialStance);
                }
            }

            weapon->AddDecay(weaponDecay->GetValue());
            struckArmor->AddDecay(armorDecay->GetValue());

            psserver->GetCombatManager()->NotifyTarget(attacker, target);

            break;
        }
        case ATTACK_DODGED:
        {
            psCombatEventMessage ev(event->GetAttackerID(),
                                    psCombatEventMessage::COMBAT_DODGE,
                                    attacker->GetEID(),
                                    target->GetEID(),
                                    event->AttackLocation,
                                    0, // no dmg on a dodge
                                    weapon->GetAttackAnimID(attacker_data),
                                    (unsigned int)-1); // no defense anims yet

            ev.Multicast(target->GetMulticastClients(), 0, MAX_COMBAT_EVENT_RANGE);
            psserver->GetCombatManager()->NotifyTarget(attacker, target);
            break;
        }
        case ATTACK_BLOCKED:
        {
            psCombatEventMessage ev(event->GetAttackerID(),
                                    psCombatEventMessage::COMBAT_BLOCK,
                                    attacker->GetEID(),
                                    target->GetEID(),
                                    event->AttackLocation,
                                    0, // no dmg on a block
                                    weapon->GetAttackAnimID(attacker_data),
                                    (unsigned int)-1); // no defense anims yet

            ev.Multicast(target->GetMulticastClients(),0,MAX_COMBAT_EVENT_RANGE);

            if(weapon)
            {
                weapon->AddDecay(weaponDecay->GetValue());
            }
            //TODO: for now we disable decaying for bows (see PS#5181)
            if(blockingWeapon && !blockingWeapon->GetIsRangeWeapon())
            {
                blockingWeapon->AddDecay(blockDecay->GetValue());
            }

            psserver->GetCombatManager()->NotifyTarget(attacker,target);

            break;
        }
        case ATTACK_MISSED:
        {
            psCombatEventMessage ev(event->GetAttackerID(),
                                    psCombatEventMessage::COMBAT_MISS,
                                    attacker->GetEID(),
                                    target->GetEID(),
                                    event->AttackLocation,
                                    0, // no dmg on a miss
                                    weapon->GetAttackAnimID(attacker_data),
                                    (unsigned int)-1); // no defense anims yet

            ev.Multicast(target->GetMulticastClients(), 0, MAX_COMBAT_EVENT_RANGE);
            psserver->GetCombatManager()->NotifyTarget(attacker, target);
            break;
        }
        case ATTACK_OUTOFRANGE:
        {
            psserver->SendSystemError(event->GetAttackerID(),"You are too far away to attack!");
            break;
        }
        case ATTACK_BADANGLE:
        {
            psserver->SendSystemError(event->GetAttackerID(),"You must face the enemy to attack!");
            break;
        }
        case ATTACK_OUTOFAMMO:
        {
            psserver->SendSystemError(event->GetAttackerID(), "You are out of ammo!");

            // If you run out of ammo, you exit attack mode.
            psserver->GetCombatManager()->StopAttack(attacker);
            break;
        }
    }
}

psCombatAttackGameEvent::psCombatAttackGameEvent(csTicks delayticks,
        psAttack* attack,
        gemActor* attacker,
        gemActor* target,
        INVENTORY_SLOT_NUMBER weaponslot,
        psItem* weapon) :
    psGameEvent(0, delayticks, "psCombatAttackGameEvent")
{
    this->attack = attack;
    this->attacker = attacker;
    this->target = target;
    this->weaponSlot = weaponslot;
    this->weapon = weapon;
    this->AttackerCID = attacker->GetClientID();
    this->TargetCID = target->GetClientID();

    AttackLocation = PSCHARACTER_SLOT_NONE;
    FinalDamage = -1;
}

psCombatAttackGameEvent::~psCombatAttackGameEvent()
{
    if(attacker)
    {
        if(attack && attack->GetID() != 1)
        {
            psCharacter* attacker_data = attacker->GetCharacterData();
            attacker_data->GetAttackQueue()->Pop();
        }
    }
}

void psCombatAttackGameEvent::Trigger()
{
    if(attack->GetID() != 1 && attacker)
    {
        psCharacter* attacker_data = attacker->GetCharacterData();
        attacker_data->GetAttackQueue()->Pop();
    }
    attack->Affect(this);
    attack = NULL;
}
