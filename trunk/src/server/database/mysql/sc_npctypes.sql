DROP TABLE IF EXISTS `sc_npctypes`;
CREATE TABLE  `sc_npctypes` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT 'An unique id for this row',
  `name` varchar(200) NOT NULL COMMENT 'The name of this npctype',
  `parents` varchar(200) NOT NULL COMMENT 'The parents of this npctype for inheritance',
  `ang_vel` float DEFAULT '999' COMMENT 'Angular speed of the npctype',
  `vel` varchar(200) DEFAULT '999' COMMENT 'Speed of the npctype',
  `collision` varchar(200) DEFAULT NULL COMMENT 'Perception when colliding',
  `out_of_bounds` varchar(200) DEFAULT NULL COMMENT 'Perception when out of bounds',
  `in_bounds` varchar(200) DEFAULT NULL COMMENT 'Perception when in bounds',
  `falling` varchar(200) DEFAULT NULL COMMENT 'Perception when falling',
  `script` text NOT NULL COMMENT 'The script of this npctype',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Stores the list of npc types';

#
# Dumping data for table 'sc_npctypes'
#
INSERT INTO sc_npctypes VALUES("1","DoNothing","","","","","","","",
'<behavior name="do nothing" decay="0" growth="0" initial="50">
   <wait duration="1" anim="stand" />
</behavior>');

INSERT INTO sc_npctypes VALUES("2","GoHomeOnTeleport","","","","","","","",
'<behavior name="Teleported" completion_decay="-1" growth="0" initial="0">
   <locate obj="perception" />
   <teleport />                              <!-- Teleport to located position --> 
   <locate obj="waypoint"  static="no" />    <!-- Locate nearest waypoint -->
   <navigate anim="walk" />                  <!-- Local navigation -->
   <locate obj="home:$name" />
   <wander anim="walk" />                    <!-- Navigate using waypoints -->
   <navigate anim="walk" />                  <!-- Local navigation -->
</behavior>

<react event="teleported" behavior="Teleported" />');

INSERT INTO sc_npctypes VALUES("3","InRegion","","","","","out of bounds","","",
'<behavior name="GoToRegion" completion_decay="-1" growth="0" initial="1000">
   <locate obj="waypoint"  static="no" />    <!-- Locate nearest waypoint -->
   <navigate anim="walk" />                  <!-- Local navigation -->
   <locate obj="region" />
   <wander anim="walk" />        <!-- Navigate using waypoints -->
   <navigate anim="walk" />      <!-- Local navigation -->
</behavior>

<behavior name="MoveBackInRegion" completion_decay="-1" growth="0" initial="0" >
   <!-- Move back into the region -->
   <locate obj="region" />
   <navigate anim="walk" />      <!-- Local navigation -->
</behavior>

<react event="out of bounds"       behavior="MoveBackInRegion" do_not_interrupt="GoToRegion" />');

INSERT INTO sc_npctypes VALUES("4","Fight","","","","","","","",
'<behavior name="Fight"   initial="0" growth="0" decay="0" completion_decay="-1" >
   <locate obj="target"  range="20" />
   <rotate type="locatedest" anim="walk" ang_vel="120" />
   <melee seek_range="20" melee_range="3" />
</behavior>
<behavior name="Chase" initial="0" growth="0" decay="1" completion_decay="-1" >
   <chase type="target" chase_range="20" anim="run" vel="5" />
</behavior>

<react event="attack"              behavior="Fight" />
<react event="damage"              behavior="Fight" delta="20" weight="1" />
<react event="target out of range" behavior="Chase" />
<react event="target out of chase" behavior="Chase" absolute="0" only_interrupt="chase"/>
<react event="death"               behavior="Fight" absolute="0" />');

INSERT INTO sc_npctypes VALUES("5","TurnToSensedPlayer","","","","","","","",
'<behavior name="TurnToFacePlayer" completion_decay="-1" growth="0" initial="0">
   <locate obj="perception" />
   <rotate type="locatedest" anim="walk" ang_vel="60" />
</behavior>

<react event="player sensed"       behavior="TurnToFacePlayer" delta="100" inactive_only="yes" />');


INSERT INTO sc_npctypes VALUES("100","Smith","GoHomeOnTeleport","","","","","","",
'<behavior name="do nothing" decay="0" growth="0" initial="10">
   <wait duration="1" anim="stand" />
</behavior>
<behavior name="go_climbing1" initial="0" completion_decay="20" loop="no"> 
   <moveto x="-53.6003" y="0.0" z="-155.041" anim="walk" />
   <moveto x="-8.89576" y="0.0" z="-162.498" anim="walk" />
   <moveto x="18.4303" y="21.9941" z="-163.082" anim="walk" />
   <moveto x="-9.01569" y="0.0" z="-159.477" anim="walk" />
   <moveto x="-19.7409" y="0.0" z="-153.539" anim="walk" />
   <moveto x="-42.8985" y="0.0" z="-146.811" anim="walk" />
   <moveto x="-53.6242" y="0.0" z="-155.023" anim="walk" />
   <wait duration="60" anim="stand" />
   <moveto x="-53.6242" y="0.0" z="-155.023" anim="walk" />
   <moveto x="-42.8985" y="0.0" z="-146.811" anim="walk" />
   <moveto x="-19.7409" y="0.0" z="-153.539" anim="walk" />
   <moveto x="-9.01569" y="0.0" z="-159.477" anim="walk" />
   <moveto x="18.4303" y="21.9941" z="-163.082" anim="walk" />
   <moveto x="-8.89576" y="0.0" z="-162.498" anim="walk" />
   <moveto x="-53.6003" y="0.0" z="-155.041" anim="walk" />
</behavior> 
<behavior name="go_obstacles1" initial="0" completion_decay="20" loop="no"> 
   <moveto x="-53.5112" y="0.0" z="-155.135" anim="walk" />
   <moveto x="-44.2481" y="0.0" z="-153.507" anim="walk" />
   <moveto x="-27.3333" y="0.0" z="-163.914" anim="walk" />
   <moveto x="-32.8587" y="0.0" z="-199.55" anim="walk" />
   <moveto x="-45.5557" y="0.0" z="-213.535" anim="walk" />
   <moveto x="-45.5928" y="0.0" z="-225.046" anim="walk" />
   <moveto x="-42.5066" y="0.0" z="-247.016" anim="walk" />
   <moveto x="-35.3197" y="0.0" z="-246.967" anim="walk" />
   <moveto x="-35.1166" y="0.0" z="-237.451" anim="walk" />
   <moveto x="-26.9788" y="0.0" z="-237.333" anim="walk" />
   <moveto x="-26.5143" y="0.0" z="-248.163" anim="walk" />
   <moveto x="-19.3833" y="0.0" z="-247.596" anim="walk" />
   <moveto x="-19.4912" y="0.0" z="-236.566" anim="walk" />
   <moveto x="-10.7207" y="0.0" z="-236.432" anim="walk" />
   <moveto x="-11.0175" y="0.0" z="-249.279" anim="walk" />
   <moveto x="-4.43381" y="0.0" z="-248.536" anim="walk" />
   <moveto x="-4.49285" y="0.0" z="-236.315" anim="walk" />
   <moveto x="1.82961" y="0.0" z="-235.959" anim="walk" />
   <moveto x="1.64485" y="0.0" z="-249.233" anim="walk" />
   <moveto x="8.12992" y="0.0" z="-249.23" anim="walk" />
   <moveto x="8.19972" y="0.0" z="-230.12" anim="walk" />
   <moveto x="-24.0757" y="0.0" z="-222.12" anim="walk" />
   <moveto x="-24.8802" y="0.0" z="-205.212" anim="walk" />
   <moveto x="-42.7254" y="0.0" z="-166.754" anim="walk" />
   <moveto x="-46.2027" y="0.0" z="-152.995" anim="walk" />
   <moveto x="-53.5608" y="0.0" z="-155.176" anim="walk" />
   <wait duration="30" anim="stand" />
   <moveto x="-53.5608" y="0.0" z="-155.176" anim="walk" />
   <moveto x="-46.2027" y="0.0" z="-152.995" anim="walk" />
   <moveto x="-42.7254" y="0.0" z="-166.754" anim="walk" />
   <moveto x="-24.8802" y="0.0" z="-205.212" anim="walk" />
   <moveto x="-24.0757" y="0.0" z="-222.12" anim="walk" />
   <moveto x="8.19972" y="0.0" z="-230.12" anim="walk" />
   <moveto x="8.12992" y="0.0" z="-249.23" anim="walk" />
   <moveto x="1.64485" y="0.0" z="-249.233" anim="walk" />
   <moveto x="1.82961" y="0.0" z="-235.959" anim="walk" />
   <moveto x="-4.49285" y="0.0" z="-236.315" anim="walk" />
   <moveto x="-4.43381" y="0.0" z="-248.536" anim="walk" />
   <moveto x="-11.0175" y="0.0" z="-249.279" anim="walk" />
   <moveto x="-10.7207" y="0.0" z="-236.432" anim="walk" />
   <moveto x="-19.4912" y="0.0" z="-236.566" anim="walk" />
   <moveto x="-19.3833" y="0.0" z="-247.596" anim="walk" />
   <moveto x="-26.5143" y="0.0" z="-248.163" anim="walk" />
   <moveto x="-26.9788" y="0.0" z="-237.333" anim="walk" />
   <moveto x="-35.1166" y="0.0" z="-237.451" anim="walk" />
   <moveto x="-35.3197" y="0.0" z="-246.967" anim="walk" />
   <moveto x="-42.5066" y="0.0" z="-247.016" anim="walk" />
   <moveto x="-45.5928" y="0.0" z="-225.046" anim="walk" />
   <moveto x="-45.5557" y="0.0" z="-213.535" anim="walk" />
   <moveto x="-32.8587" y="0.0" z="-199.55" anim="walk" />
   <moveto x="-27.3333" y="0.0" z="-163.914" anim="walk" />
   <moveto x="-44.2481" y="0.0" z="-153.507" anim="walk" />
   <moveto x="-53.5112" y="0.0" z="-155.135" anim="walk" />
</behavior> 
<behavior name="go_other_sector1" initial="0" completion_decay="20" loop="no"> 
   <moveto x="-53.546" y="0.0" z="-155.243" anim="walk" />
   <moveto x="-42.1452" y="0.0" z="-140.879" anim="walk" />
   <moveto x="-55.2024" y="0.0" z="-126.302" anim="walk" />
   <moveto x="-57.6445" y="0.0" z="-113.753" anim="walk" />
   <wait duration="30" anim="stand" />
   <moveto x="-57.6445" y="0.0" z="-113.753" anim="walk" />
   <moveto x="-55.2024" y="0.0" z="-126.302" anim="walk" />
   <moveto x="-42.1452" y="0.0" z="-140.879" anim="walk" />
   <moveto x="-53.546" y="0.0" z="-155.243" anim="walk" />
</behavior> 
<behavior name="go_sleep1" initial="0" completion_decay="20" loop="no"> 
   <moveto x="-53.6185" y="0.0" z="-155.243" anim="walk" />
   <moveto x="-51.4396" y="0.0" z="-149.481" anim="walk" />
   <moveto x="-54.8432" y="0.0" z="-146.474" anim="walk" />
   <moveto x="-55.6514" y="0.0" z="-147.652" anim="walk" />
   <invisible/>
   <wait duration="30" anim="stand" />
   <visible/>
   <moveto x="-55.6514" y="0.0" z="-147.652" anim="walk" />
   <moveto x="-54.8432" y="0.0" z="-146.474" anim="walk" />
   <moveto x="-51.4396" y="0.0" z="-149.481" anim="walk" />
   <moveto x="-53.6185" y="0.0" z="-155.243" anim="walk" />
</behavior> 

<react event="time" value="8,0,,,"  behavior="go_other_sector1" delta="20" /> 
<react event="time" value="10,0,,,"  behavior="go_sleep1" delta="20" /> 
<react event="time" value="12,0,,,"  behavior="go_obstacles1" delta="20" /> 
<react event="time" value="14,0,,,"  behavior="go_climbing1" delta="20" />');

INSERT INTO sc_npctypes VALUES("101","Fighter","DoNothing,InRegion,Fight","","","","","","",
'<empty/>');

INSERT INTO sc_npctypes VALUES("102","Wanderer","Fighter","","","","","","",
'<behavior name="Walk" decay="0" growth="0" initial="50">
   <loop>
      <locate obj="region" static="no" />                    <!-- Locate random point within region -->
      <rotate type="locatedest" anim="walk" ang_vel="10" />  <!-- Rotate to fase last located destination -->
      <navigate anim="walk" />                               <!-- Local navigation -->
      <wait duration="5" anim="stand" />
   </loop>
</behavior>');


INSERT INTO sc_npctypes VALUES("103","On Sight Fighter","TurnToSensedPlayer,Fighter","","","","","","",
'<react event="player nearby"       behavior="Fight" delta="100" inactive_only="yes" faction_diff="-200" oper=">"  /> <!-- Nearby(<10) -->
<react event="player adjacent"     behavior="Fight" delta="100" inactive_only="yes" faction_diff="-200" oper=">"  />');

INSERT INTO sc_npctypes VALUES("104","Answerer","DoNothing","","","","","","",
'<behavior name="turn to face" completion_decay="-1" growth="0" initial="0" >
   <locate obj="perception" />
   <rotate type="locatedest" anim="walk" ang_vel="20" />
   <emote cmd="/greet" />
   <wait duration="10" anim="stand" />
</behavior>

<react event="talk" inactive_only="yes" faction_diff="-100" oper=">" behavior="turn to face" delta="100"  when_invisible="yes" when_invincible="yes" />');

INSERT INTO sc_npctypes VALUES("105","Sit","DoNothing","","","","","","",
'<behavior name="sit" completion_decay="-1" growth="0" initial="0" >
   <sit />
   <wait duration="10" anim="sit_idle" />
   <standup />
</behavior>
<react event="player adjacent" inactive_only="yes" behavior="sit" delta="100"  when_invisible="yes" when_invincible="yes" />');

INSERT INTO sc_npctypes VALUES("106","ChaseTest1","DoNothing","","","","","","",
'<behavior name="init" initial="1000" completion_decay="-1" >
   <!--debug level="0" /-->
</behavior>
<behavior name="chase" decay="1" completion_decay="-1" growth="0" initial="0">
   <talk text="Start chaseing actor" target="false" />
   <chase type="nearest_actor" offset="3" offset_angle="90" range="11" chase_range="20" anim="walk" />
   <talk text="Stop chaseing actor" target="false" />
</behavior>

<react event="player nearby" behavior="chase" delta="100" /> <!-- Nearby(<10) -->
<react event="nearest_actor out of chase" behavior="chase" absolute="0" />');

INSERT INTO sc_npctypes VALUES("107","ChaseTest2","DoNothing","","","","","","",
'<behavior name="init" initial="1000" completion_decay="-1" >
   <!--debug level="0" /-->
</behavior>
<behavior name="chase" initial="100" >
   <locate obj="entity:pid:22" />  <!-- Locate MoveMaster2 -->
   <chase type="target" side_offset="2" offset_relative_heading="true" anim="walk" />
   <wait anim="stand" duration="1" />  <!-- Could be a watch operation!! -->
</behavior>');

INSERT INTO sc_npctypes VALUES("108","Minion","DoNothing","","","","","","",
'<!-- Follow, start on /pet follow and stop on /pet stay -->
<behavior name="follow" completion_decay="100" decay="0" growth="0" initial="0" loop="yes">
   <watch type="owner" range="3.0" />
</behavior>
<behavior name="follow_chase" completion_decay="100" growth="0" initial="0">
   <chase type="owner" anim="walk" collision="follow_collided" offset="2.0" />
</behavior>
<behavior name="follow_turn" completion_decay="100" growth="0" initial="0">
   <rotate type="random" min="90" max="270" anim="walk" ang_vel="30" />
   <move vel="2" anim="walk" duration="1.0"/>
</behavior>

<!-- Attack, start on /pet attack and stop on /pet stopattack -->
<behavior name="attack" decay="0" growth="0" initial="0">
   <locate obj="target"  range="50"/>
   <rotate type="locatedest" anim="walk" ang_vel="120" />
   <melee seek_range="50" melee_range="3" />
</behavior>
<behavior name="attack_chase" completion_decay="100" growth="0" initial="0">
   <chase type="target" anim="run" vel="5" />
</behavior>
<behavior name="attack_turn" completion_decay="100" growth="0" initial="0">
   <rotate type="random" min="90" max="270" anim="walk" ang_vel="30" />
   <move vel="2" anim="walk" duration="1.0"/>
</behavior>
	
<react event="ownercmd:stay"        behavior="follow,follow_chase,follow_turn"       absolute="0"   only_interrupt="follow,follow_chase,follow_turn" />
<react event="owner out of chase"   behavior="follow,follow_chase,follow_turn"       absolute="0"   only_interrupt="follow,follow_chase,follow_turn" />
<react event="ownercmd:follow"      behavior="follow"       delta=  "100"  inactive_only="true" />
<react event="follow_collided"      behavior="follow_turn"  delta=  "100" />
<react event="owner out of range"   behavior="follow_chase"  />

<react event="ownercmd:stopattack"  behavior="attack,attack_turn,attack_chase"       absolute= "0" />
<react event="ownercmd:attack"      behavior="attack"       delta=  "200"  inactive_only="true" weight="10" />
<react event="collision"            behavior="attack_turn"  delta=  "100" />
<react event="damage"               behavior="attack"       delta=  "200"  inactive_only="true" weight="1" />
<react event="target out of range"  behavior="attack_chase"  />');

INSERT INTO sc_npctypes VALUES("109","AbstractTribesman","DoNothing","","","","","","",
'<!-- Abstract base npc type for tribes -->

<behavior name="peace_meet" completion_decay="500">
   <locate obj="perception" />
   <rotate type="locatedest" anim="walk" ang_vel="120" />
   <emote cmd="/greet" />
</behavior>

<behavior name="aggressive_meet" decay="10" completion_decay="150">
   <locate obj="perception" />
   <rotate type="locatedest" anim="walk" ang_vel="120" />
   <melee seek_range="20" melee_range="3" />
</behavior>

<behavior name="coward_attacked" decay="10">
   <emote cmd="/flee" />
   <locate obj="waypoint" static="no" />
   <navigate anim="walk" />
   <locate obj="waypoint" static="no" random="yes" range="10" />
   <wander anim="walk" />
</behavior>

<behavior name="normal_attacked" decay="10">
   <locate obj="target" range="20" />
   <rotate type="locatedest" anim="walk" ang_vel="120" />
   <melee seek_range="20" melee_range="3" />
</behavior>

<behavior name="united_attacked" decay="10">
   <locate obj="enemy" />
   <percept event="tribesman attacked" target="tribe" />
   <melee seek_range="20" melee_range="3" />
</behavior>

<behavior name="Chase" initial="0" growth="0" decay="1" complection_decay="-1" >
   <chase type="target" chase_range="20" anim="run" vel="5" />
</behavior>

<behavior name="Explore" completion_decay="100" resume="yes" auto_memorize="all">
   <locate obj="waypoint" static="no" />
   <navigate anim="walk" />
   <locate obj="waypoint" static="no" random="yes" range="80" />
   <wander anim="walk" />
   <wait anim="stand" duration="3" />
   <locate obj="tribe:home" static="no" />
   <wander anim="walk" />
   <navigate anim="walk" />
   <share_memories />
</behavior>

<behavior name="GatherResource" completion_decay="100" resume="yes">
   <locate obj="ownbuffer" />
   <navigate anim="walk" />
   <equip item="Rock Pick" slot="righthand" />
   <dig resource="tribe:wealth" />
   <locate obj="tribe:home"/>
   <navigate anim="walk" />
   <transfer item="tribe:wealth" target="tribe" />
   <share_memories />
</behavior>

<behavior name="MineResource" completion_decay="100" resume="yes">
   <locate obj="waypoint" static="no" />
   <navigate anim="walk" />
   <locate obj="ownbuffer" static="no" />
   <wander anim="walk" />
   <equip item="Rock Pick" slot="righthand" />
   <loop iterations="3">
      <dig resource="tribe:buffer" />
      <wait anim="stand" duration="10" />
   </loop>
   <dequip slot="righthand" />
   <locate obj="waypoint" static="no" />
   <navigate anim="walk" />
   <locate obj="tribe:home" static="no" />
   <wander anim="walk" />
   <transfer item="ownbuffer" target="tribe" />
</behavior>

<behavior name="GoToSleep" resume="yes">
   <locate obj="waypoint" static="no" />
   <navigate anim="walk" />
   <locate obj="tribe:home" static="no" />
   <wander anim="walk" />
   <wait anim="sit" duration="180" />
</behavior>

<behavior name="Breed" resume="yes" completion_decay="100">
   <locate obj="waypoint" static="no" />
   <navigate anim="walk" />
   <locate obj="tribe:home" static="no" />
   <wander anim="walk" />
   <navigate anim="walk" />
   <emote cmd="/divide" />
   <wait duration="30" anim="stand" />
   <locate obj="self" static="no" />
   <reproduce />
</behavior>

<behavior name="Resurrect" when_dead="yes" completion_decay="200">
   <resurrect />
</behavior>

<behavior name="GoToWork" completion_decay="100">
   <locate obj="waypoint" static="no" />
   <navigate anim="walk" />
   <locate obj="tribe:memory:work" static="no" />
   <wander anim="walk" />
   <locate obj="waypoint" static="no" />
   <navigate anim="walk" />
   <emote cmd="/work" />
   <wait duration="buffer" anim="stand" />
   <locate obj="tribe:home" static="no" />
   <wander anim="walk" />
</behavior>

<behavior name="Guard" resume="yes">
   <locate obj="ownbuffer" />
   <navigate anim="walk" />
   <circle anim="walk" radius="2" />
</behavior>

<behavior name="Turn" completion_decay="500">
   <rotate type="random" min="0" max="360" anim="walk" />
   <move anim="walk" duration="3" />
</behavior>

<react event="collision" behavior="Turn" delta="500" />
<react event="tribe:work" behavior="GoToWork" delta="100" />
<react event="tribe:resurrect" behavior="Resurrect" delta="200" when_dead="yes" />
<react event="tribe:mine" behavior="MineResource" delta="100" />
<react event="tribe:gather" behavior="GatherResource" delta="100" />
<react event="tribe:breed" behavior="Breed" delta="100" />
<react event="tribe:explore" behavior="Explore" delta="100" />
<react event="target out of range" behavior="Chase" />
<react event="target out of chase" behavior="Chase" absolute="0" only_interrupt="chase" />');

INSERT INTO sc_npctypes VALUES("111","MineingTribe","AbstractTribesman","","","","","","",
'<empty/>');

INSERT INTO sc_npctypes VALUES("112","HuntingTribe","AbstractTribesman","","","","","","",
'<empty/>');

INSERT INTO sc_npctypes VALUES("113","MoveTest1","","","$walk","","","","",
'<!-- Testing the wander operation. -->

<!-- Locate nearest waypoint -->
<behavior name="Locate waypoint" completion_decay="-1" initial="100">
   <locate obj="waypoint"  static="no" />
   <navigate anim="walk" />        <!-- Local navigation -->
</behavior>

<behavior name="Wander waypoints" decay="0" growth="0" initial="50" loop="yes">
   <!--debug level="0" /-->
   <wander anim="walk" random="yes" />
</behavior>');

INSERT INTO sc_npctypes VALUES("114","MoveTest2","DoNothing","","$run","","","","",
'<!-- Initial behavior to locate the starting point -->

<behavior name="Initialize" completion_decay="-1" initial="1000">
   <!--debug level="15" /-->
   <locate obj="waypoint"  static="no" />    <!-- Locate nearest waypoint -->
   <navigate anim="walk" />                  <!-- Local navigation -->
</behavior>

<!-- Advanced movement using waypoints -->
<behavior name="Move1" decay="0" growth="0" initial="80" loop="yes">
   <!--debug level="0" /-->
   <locate obj="waypoint:name:p1"  static="no" />
   <wander anim="walk" />        <!-- Navigate using waypoints -->
   <wait anim="stand" duration="5" />
   <rotate type="relative" value="180" ang_vel="45"/>
   <locate obj="waypoint:group:tower_watch"  random="yes" static="no" />
   <wander anim="walk" />        <!-- Navigate using waypoints -->
   <wait anim="stand" duration="5" />
   <locate obj="waypoint:name:tower_2"  static="no" />
   <wander anim="walk" />        <!-- Navigate using waypoints -->
   <movepath path="Test Path 1" direction="REVERSE" />
   <wait anim="stand" duration="5" />
</behavior>');

INSERT INTO sc_npctypes VALUES("115","MoveTest3","DoNothing","","","","","","",
'<!-- Testing that advanced movement using waypoints -->
<!-- work fine when interruped. If working ok this NPC -->
<!-- should go the same way as MoveTest2 -->

<!-- Advanced movement using waypoints -->
<behavior name="Move1" decay="0" growth="0" initial="80" loop="yes" resume="yes">
    <!--debug level="0" /-->
    <locate obj="waypoint"  static="no" />    <!-- Locate nearest waypoint -->
    <navigate anim="walk" />                  <!-- Local navigation -->
    <locate obj="waypoint:name:p1"  static="no" />
    <wander anim="walk" />        <!-- Navigate using waypoints -->
    <wait anim="stand" duration="5" />
    <rotate type="relative" value="180" ang_vel="45"/>
    <locate obj="waypoint:name:tower_2"  static="no" />
    <wander anim="walk" />        <!-- Navigate using waypoints -->
    <wait anim="stand" duration="5" />
    <rotate type="relative" value="180" ang_vel="45"/>
</behavior>

<behavior name="Interrupt" completion_decay="0" growth="0" initial="0">
   <!-- Test that a Move1 behaviour is interrupted without failing to move -->
   <!-- Use this as a null operation. -->
   <memorize obj="perception" />
</behavior>

<react event="location sensed"    behavior="Interrupt" />');

INSERT INTO sc_npctypes VALUES("116","MoveTest4","DoNothing","","","","","","",
'<!-- This test moving between sectors -->

<!-- Advanced movement using waypoints -->
<behavior name="Move1" decay="0" growth="0" initial="80" loop="yes">
   <locate obj="waypoint"  static="no" />    <!-- Locate nearest waypoint -->
   <navigate anim="walk" />                  <!-- Local navigation -->

   <locate obj="waypoint:name:p13"  static="no" />
   <wander anim="walk" vel="$walk" />        <!-- Navigate using waypoints -->
   <wait anim="stand" duration="5" />
   <locate obj="waypoint:name:p3"  static="no" />
   <wander anim="walk" vel="$run" />        <!-- Navigate using waypoints -->
   <wait anim="stand" duration="5" />

   <!-- Test the teleport flag on paths -->
   <locate obj="waypoint:name:npcr1_014"  static="no" />
   <wander anim="walk" vel="$run" />        <!-- Navigate using waypoints -->
   <wait anim="stand" duration="5" />
   <talk text="Going for the teleport" target="false" />
   <locate obj="waypoint:name:npcr1_017"  static="no" />
   <talk text="Going to test teleport path" target="false" />
   <wander anim="walk" vel="$run" />        <!-- Navigate using waypoints -->
   <talk text="Did I make it?" target="false" />
   <wait anim="stand" duration="5" />

</behavior>');

INSERT INTO sc_npctypes VALUES("117","MoveTest5","Answerer","","$walk","","","","",
'<!-- Example Citizen behaviour -->

<behavior name="GoHome" decay="0" growth="0" initial="0">
   <locate obj="waypoint"  static="no" />    <!-- Locate nearest waypoint -->
   <navigate anim="walk" />                  <!-- Local navigation -->

   <locate obj="waypoint:name:$name_home"  static="no" />
   <wander anim="walk" />        <!-- Navigate using waypoints -->
   <!-- Simulate going inside by setting invisbile -->
   <wait duration="10" anim="stand" />
   <invisible/>
   <wait duration="30" anim="stand" />
   <visible/>
</behavior>

<behavior name="GoWork" decay="0" growth="0" initial="0">
   <locate obj="waypoint"  static="no" />    <!-- Locate nearest waypoint -->
   <navigate anim="walk" />                  <!-- Local navigation -->

   <locate obj="waypoint:name:$name_work"  static="no" />
   <wander anim="walk" />        <!-- Navigate using waypoints -->
</behavior>

<react event="time" value="0,0,,," random=",5,,," behavior="GoWork" /> 
<react event="time" value="1,0,,," random=",5,,,"  behavior="GoHome" /> 
<react event="time" value="2,0,,," random=",5,,,"  behavior="GoWork" /> 
<react event="time" value="3,0,,," random=",5,,,"  behavior="GoHome" /> 
<react event="time" value="4,0,,," random=",5,,,"  behavior="GoWork" /> 
<react event="time" value="5,0,,," random=",5,,,"  behavior="GoHome" /> 
<react event="time" value="6,0,,," random=",5,,,"  behavior="GoWork" /> 
<react event="time" value="7,0,,," random=",5,,,"  behavior="GoHome" /> 
<react event="time" value="8,0,,," random=",5,,,"  behavior="GoWork" /> 
<react event="time" value="9,0,,," random=",5,,,"  behavior="GoHome" /> 
<react event="time" value="10,0,,," random=",5,,,"  behavior="GoWork" /> 
<react event="time" value="11,0,,," random=",5,,,"  behavior="GoHome" /> 
<react event="time" value="12,0,,," random=",5,,,"  behavior="GoWork" /> 
<react event="time" value="13,0,,," random=",5,,,"  behavior="GoHome" /> 
<react event="time" value="14,0,,," random=",5,,,"  behavior="GoWork" /> 
<react event="time" value="15,0,,," random=",5,,,"  behavior="GoHome" /> 
<react event="time" value="16,0,,," random=",5,,,"  behavior="GoWork" /> 
<react event="time" value="17,0,,," random=",5,,,"  behavior="GoHome" /> 
<react event="time" value="18,0,,," random=",5,,,"  behavior="GoWork" /> 
<react event="time" value="19,0,,," random=",5,,,"  behavior="GoHome" /> 
<react event="time" value="20,0,,," random=",5,,,"  behavior="GoWork" /> 
<react event="time" value="21,0,,," random=",5,,,"  behavior="GoHome" /> 
<react event="time" value="22,0,,," random=",5,,,"  behavior="GoWork" /> 
<react event="time" value="23,0,,," random=",5,,,"  behavior="GoHome" />');

INSERT INTO sc_npctypes VALUES("118","MoveTest6","DoNothing","","$run","","","","",
'<!-- Initial behavior to locate the starting point -->
<behavior name="Initialize" completion_decay="-1" initial="1000">
   <!--debug level="0" /-->
   <locate obj="waypoint:name:npcr1_014"  static="no" />    <!-- Locate nearest waypoint -->
   <teleport />                  <!-- Teleport to waypoint -->
</behavior>

<!-- Advanced movement using waypoints -->
<behavior name="Move1" decay="0" growth="0" initial="80" loop="yes" failure="Error" >
   <!--debug level="0" /-->
   <talk text="Going to other waypoint not using private" target="false" />
   <locate obj="waypoint:name:npcr1_021"  static="no" />
   <wander anim="walk" private="false" />        <!-- Navigate using waypoints -->
   <rotate type="relative" value="180" ang_vel="45"/>
   <wait anim="stand" duration="5" />

   <talk text="Going to other waypoint" target="false" />
   <locate obj="waypoint:name:npcr1_014"  random="yes" static="no" />
   <wander anim="walk" />        <!-- Navigate using waypoints -->
   <rotate type="relative" value="180" ang_vel="45"/>
   <wait anim="stand" duration="5" />
</behavior>

<react event="Error" behavior="Initialize" />');

INSERT INTO sc_npctypes VALUES("119","MoveUnderground","DoNothing","","$walk","","","","",
'<!-- Testing the wander operation. -->

<!-- Locate nearest waypoint -->
<behavior name="Initialize" completion_decay="-1" initial="100">
   <!-- debug level="15" /-->
   <locate obj="waypoint"  static="no" />
   <navigate anim="walk" />        <!-- Local navigation -->
   <locate obj="waypoint:name:ug_1" static="no" />
   <wander anim="walk" />          <!-- Navigate using waypoints -->
</behavior>

<behavior name="Wander waypoints" decay="0" growth="0" initial="50" loop="yes">
   <wander anim="walk" random="yes" underground="true" />
</behavior>');

INSERT INTO sc_npctypes VALUES("120","WinchMover","DoNothing","","","","","","",
'<!-- Initial behavior to locate the starting point -->
<behavior name="Initialize" completion_decay="-1" initial="1000">
   <!--debug level="0" /-->
   <locate obj="waypoint"  static="no" />    <!-- Locate nearest waypoint -->
   <navigate anim="walk" />                  <!-- Local navigation -->
   <locate obj="winch:Mover pos 1" static="no" />
   <wander anim="walk" />        <!-- Navigate using waypoints -->
   <navigate anim="walk" />      <!-- Local navigation -->
   <rotate type="absolute" value="180" ang_vel="20"/>
</behavior>

<behavior name="do nothing" decay="0" growth="0" initial="1">
   <wait duration="1" anim="stand" />
</behavior>

<behavior name="turn to face" completion_decay="-1" growth="0" initial="0" >
   <locate obj="perception" />
   <rotate type="locatedest" anim="walk" ang_vel="20" />
   <wait duration="10" anim="stand" />
</behavior>

<react event="talk" inactive_only="yes" faction_diff="-100" oper=">" behavior="turn to face" delta="100"  when_invisible="yes" when_invincible="yes" />');

INSERT INTO sc_npctypes VALUES("121","WinchBeast","DoNothing","","","","","","",
'<!-- Test interaction with sequences in the map -->
<!-- and communication from other NPCs using NPCCMD -->

<!-- Initial behavior to locate the starting point -->
<behavior name="Initialize" completion_decay="-1" initial="1000">
   <!--debug level="0" /-->
   <locate obj="waypoint"  static="no" />    <!-- Locate nearest waypoint -->
   <navigate anim="walk" />                  <!-- Local navigation -->
   <locate obj="winch:Beast pos 1" static="no" />
   <wander anim="walk" />        <!-- Navigate using waypoints -->
   <navigate anim="walk" />      <!-- Local navigation -->
   <rotate type="absolute" value="90" ang_vel="20"/>
</behavior>

<behavior name="do nothing" decay="0" growth="0" initial="1">
   <wait duration="1" anim="stand" />
</behavior>

<!-- Running the wheel -->
<behavior name="winch_up" completion_decay="100">
   <rotate type="absolute" value="180" ang_vel="20"/>
   <sequence name="winch_up" cmd="start" />
   <circle radius="2.5" angle="360" anim="walk" vel="2" />
   <rotate type="relative" value="-90" ang_vel="20"/>
</behavior> 

<behavior name="winch_down" completion_decay="100">
   <rotate type="absolute" value="0" ang_vel="20"/>
   <sequence name="winch_down" cmd="start" />
   <circle radius="2.5" angle="-360" anim="walk" vel="2" />
   <rotate type="relative" value="90" ang_vel="20"/>
</behavior> 

<react event="npccmd:global:winch_up" behavior="winch_up" delta="100" inactive_only="yes"/>
<react event="npccmd:global:winch_down" behavior="winch_down" delta="100" inactive_only="yes"/>');

INSERT INTO sc_npctypes VALUES("122","LocateTest1","DoNothing","","$run","","","","",
'<!-- Test some locate fetures -->

<behavior name="LocateNearestWaypointTest" decay="0" completion_decay="-1" growth="0.1" initial="80" >
   <!--debug level="0" /-->
   <talk text="Try to locating nearest waypoint" target="false" />
   <locate obj="waypoint"  static="no" />
   <navigate anim="walk" />                  <!-- Local navigation -->
   <talk text="Found nearest waypoint" target="false" />
   <wait anim="stand" duration="5" />
</behavior>

<behavior name="LocateFailureTest" decay="0" completion_decay="-1" growth="0.1" initial="0" >
   <!--debug level="0" /-->
   <talk text="Try to locate something that will fail" target="false" />
   <locate obj="SomethingThatDoNotExists" failure="LocateFailed" static="no" />
   <talk text="Error, Locate should have failed and operation stopped." target="false" />
   <wait anim="stand" duration="5" />
</behavior>
<behavior name="LocateFailureFailed" >
   <!--debug level="0" /-->
   <talk text="Locate failed, what do I do?" target="false" />
   <wait anim="stand" duration="5" />
</behavior>

<react event="LocateFailed" behavior="LocateFailureTest" only_interrupt="LocateFailureTest" absolute="0" />
<react event="LocateFailed" behavior="LocateFailureFailed" />');

