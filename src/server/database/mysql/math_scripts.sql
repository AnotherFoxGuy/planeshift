# MySQL-Front Dump 1.16 beta
#
# Host: localhost Database: planeshift
#--------------------------------------------------------
# Server version 4.0.18-max-nt
#
# Table structure for table 'math_scripts'
#

CREATE TABLE math_scripts (
  name varchar(40) NOT NULL DEFAULT '' ,
  math_script text NOT NULL,
  PRIMARY KEY (name)
);


#
# Dumping data for table 'math_scripts'
#

INSERT INTO math_scripts VALUES( "Calculate Damage",
"
	AttackRoll = rnd(1);
        DefenseRoll = rnd(1);

        WeaponSkill = Attacker:getAverageSkillValue(AttackWeapon:Skill1,AttackWeapon:Skill2,AttackWeapon:Skill3);
        TargetWeaponSkill = Target:getAverageSkillValue(TargetAttackWeapon:Skill1,TargetAttackWeapon:Skill2,TargetAttackWeapon:Skill3);

        IAH = min(AttackRoll-.25,0.1);
        exit = if(0>IAH,1,0);
        AHR = min(AttackRoll-.5,.01);
        Blocked = AttackRoll - DefenseRoll;
        QOH = Blocked;

        RequiredInputVars = Target:AttackerTargeted+Attacker:getSkillValue(AttackWeapon:Skill1)+AttackLocationItem:Hardness;

        AttackerStance = Attacker:CombatStance;
        TargetStance = Target:CombatStance;

        AttackValue = WeaponSkill;
        TargetAttackValue = TargetWeaponSkill;
        DV = Attacker:Agility;
        TargetDV = 0;

        AVStance = if(AttackerStance=1, (AttackValue*2)+(DV*0.8),
               if(AttackerStance=2, (AttackValue*1.5)+(DV*0.5),
               if(AttackerStance=3, AttackValue,
               if(AttackerStance=4, (AttackValue*0.3),
               0))));

        TargetAVStance = if(TargetStance=1, (TargetAttackValue*2)+(TargetDV*0.8),
               if(TargetStance=2, (TargetAttackValue*1.5)+(TargetDV*0.5),
               if(TargetStance=3, TargetAttackValue,
               if(TargetStance=4, (TargetAttackValue*0.3),
               0))));


        FinalDamage = 10*(AVStance-TargetDV);
");

INSERT INTO math_scripts VALUES( "Lockpicking Time", "Time = ((LockQuality / 10) * 3)*1000;");

INSERT INTO math_scripts VALUES( "Calculate Fall Damage",
"
	exit = (rnd(1) > 0.5);
        Damage = if(FallHeight>1,pow(FallHeight, 1.8) * 0.8, 0);
");

INSERT INTO math_scripts VALUES( "CalculateManaCost", "ManaCost = Realm*4*(1+(KFactor*KFactor/10000));");

INSERT INTO math_scripts VALUES( "CalculateChanceOfCastSuccess", "ChanceOfSuccess = (50-KFactor) + WaySkill/20 + (RelatedStat)/20;");

INSERT INTO math_scripts VALUES( "CalculateChanceOfResearchSuccess", "ChanceOfSuccess = WaySkill/10 + ( 10 / Spell:Realm ) * 7;");

INSERT INTO math_scripts VALUES( "CalculatePowerLevel", "PowerLevel = 1.0 + (WaySkill/10)*(1+(1+(200-WaySkill)/100)*(KFactor*KFactor/10000));");

INSERT INTO math_scripts VALUES( "CalculateMaxCarryWeight", "MaxCarry =  ( Actor:getStatValue(0) );");

INSERT INTO math_scripts VALUES( "CalculateMaxCarryAmount", "MaxAmount =  750;");

INSERT INTO math_scripts VALUES( "MaxRealm", "MaxRealm = 1 + floor(WaySkill / 20);");

INSERT INTO math_scripts VALUES( "Summon Missile",
"
	Range = 40.0 * PowerLevel;
        CastingDuration = 5000;
        Duration = 0;
        AffectRange = 0;
        AffectAngle = 0;
        AffectTypes = 0;
        UseSaveThrow = 0;
");

INSERT INTO math_scripts VALUES( "Life Infusion",
"
Range = 0;
        Duration = 0;
        CastingDuration = 1000;
        AffectRange = 0;
        AffectAngle = 0;
        AffectTypes = 0;
        UseSaveThrow = 0;
");

INSERT INTO math_scripts VALUES( "Gust Of Wind",
"
        Range = 40.0 * PowerLevel;
        CastingDuration = 5000;
        AffectRange = 5.0 * PowerLevel + 5.0;
        AffectAngle = 45; 
        AffectTypes = 0;
        UseSaveThrow = 0;
");

INSERT INTO math_scripts VALUES( "Defensive Wind",
"
        Range = 40.0 * PowerLevel;
        Duration = 0;
        CastingDuration = 10000;
        AffectRange = 0;
        AffectAngle = 0;
        AffectTypes = 0;
        UseSaveThrow = 0;
");

INSERT INTO math_scripts VALUES( "Summon Creature",
"
        Range = 40 * PowerLevel;
        Duration = 0;
        CastingDuration = 5000;
        AffectRange = 0;
        AffectAngle = 0;
        AffectTypes = 0;
        UseSaveThrow = 0;
");

INSERT INTO math_scripts VALUES( "Summon Sword",
"
        Range = 40 * PowerLevel;
        Duration = 0;
        CastingDuration = 5000;
        AffectRange = 0;
        AffectAngle = 0;
        AffectTypes = 0;
        UseSaveThrow = 0;
");

INSERT INTO math_scripts VALUES( "Fire Warts",
"
        Range = 40.0 * PowerLevel;
        Duration = 20000;
        CastingDuration = 1000;
        Param0 = 5.0 * PowerLevel;
        AffectRange = 5.0 * PowerLevel + 5.0;
        AffectAngle = 90; 
        AffectTypes = 0;
        UseSaveThrow = 0;
");

INSERT INTO math_scripts VALUES( "Gem of Clarity",
"
        Range = 40.0 * PowerLevel;
        Duration = 6000;
        CastingDuration = 1000;
        Duration = Param1;
        AffectRange = 0;
        AffectAngle = 0;
        AffectTypes = 0;
        UseSaveThrow = 0;
");

INSERT INTO math_scripts VALUES( "CalculateSkillCosts",
"
        NextRankCost = BaseCost +(1*(SkillRank/200));
        ZCost = (PracticeFactor/100.00)*NextRankCost;
        YCost = ((100.00-PracticeFactor)/100.00)*NextRankCost;
");

INSERT INTO math_scripts VALUES( "CalculateStatCosts",
"
        YCost = SkillRank / 10 + 10;
        ZCost = 0;
");

INSERT INTO math_scripts VALUES( "StaminaMove", "Drain = Speed;");

INSERT INTO math_scripts VALUES( "StaminaCombat",
"
        PhyDrain = 4 + rnd(3);
        MntDrain = 2 + rnd(5);
");

INSERT INTO math_scripts VALUES( "StaminaBase",
"
        BasePhy = (STR + END + AGI) / 3;
        BaseMen = (INT + WILL + CHA) / 3;
");

INSERT INTO math_scripts VALUES( "CalculateMaxHP", "MaxHP = Actor:Will + Actor:Agility + Actor:Strength;");

INSERT INTO math_scripts VALUES( "CalculateMaxMana", "MaxMana = Actor:Will + Actor:Intelligence;");

INSERT INTO math_scripts VALUES( "LootModifierCostCap", "ModCap = MaxHP*10;");

INSERT INTO math_scripts VALUES( "Calculate Repair Time",
"
        Result = Object:SalePrice/100;
        Factor = Worker:getSkillValue(Object:RequiredRepairSkill) / (Object:SalePrice/20);
        Result = Result / Factor;
");

INSERT INTO math_scripts VALUES( "Calculate Repair Result",
"
        Factor = Worker:getSkillValue(Object:RequiredRepairSkill) / (Object:SalePrice/20);
        Result = ((Object:SalePrice/25) * Factor) * (rnd(1)+0.5);
");

INSERT INTO math_scripts VALUES( "CalculateFamiliarAffinity", "Affinity = Type + Lifecycle + AttackTool + AttackType;");

INSERT INTO math_scripts VALUES( "CalculateMaxPetTime", "MaxTime = 5 * 60 * 1000;");

INSERT INTO math_scripts VALUES( "Calc Player Sketch Limits",
"
	IconScore = Actor:getSkillValue(64) + 5;
	PrimCount = Actor:getSkillValue(64) + 20;
");

INSERT INTO math_scripts VALUES( "Calc Item Price", "FinalPrice = Price + Quality + MaxQuality;");

INSERT INTO math_scripts VALUES( "Calc Item Sell Price", "FinalPrice = Price * 0.8;");

INSERT INTO math_scripts VALUES( "Calc Guild Account Level", "accountLevel = 1 + log(log(totalTrias));");

INSERT INTO math_scripts VALUES( "Calc Char Account Level", "accountLevel = log(log(totalTrias));");

INSERT INTO math_scripts VALUES( "Calc Bank Fee", "bankFee = 5.25 - (accountLevel * 0.25);");

INSERT INTO math_scripts VALUES( "Calculate Mining Odds", "Total = Distance * Probability * Quality * Skill + 0.1");