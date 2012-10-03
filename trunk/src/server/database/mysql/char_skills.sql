# MySQL-Front Dump 1.16 beta
#
# Host: localhost Database: planeshift
#--------------------------------------------------------
# Server version 3.23.52-max-nt
#
# Table structure for table 'character_skills'
#

DROP TABLE IF EXISTS `character_skills`;
CREATE TABLE character_skills (
  character_id int(10) unsigned NOT NULL DEFAULT '0' ,      # Key into characters table
  skill_id int(10) unsigned NOT NULL DEFAULT '0' ,          # Key into skills table
  skill_Z int(10) unsigned NOT NULL DEFAULT '0' ,           # Practice Level Of skill
  skill_Y int(10) unsigned NOT NULL DEFAULT '0' ,           # Knowledge Level of skills
  skill_Rank int(10) unsigned NOT NULL DEFAULT '0' ,        # Current Skill Rank
  PRIMARY KEY (character_id,skill_id)
);


# Acraig char
INSERT INTO character_skills VALUES("3","0","4","4","3");

# vengeance char
INSERT INTO character_skills VALUES("2","0","0","0","2");
INSERT INTO character_skills VALUES("2","11","0","0","3");
INSERT INTO character_skills VALUES("2","37","0","0","2");
INSERT INTO character_skills VALUES("2","15","0","0","10");
INSERT INTO character_skills VALUES("2","38","0","0","5");
INSERT INTO character_skills VALUES("2","39","0","0","5");
INSERT INTO character_skills VALUES("2","40","0","0","20");

# ??? this points to nothing
INSERT INTO character_skills VALUES("19","51","0","0","60");
INSERT INTO character_skills VALUES("19","50","0","0","60");
INSERT INTO character_skills VALUES("19","49","0","0","60");
INSERT INTO character_skills VALUES("19","48","0","0","60");
INSERT INTO character_skills VALUES("19","47","0","0","60");
INSERT INTO character_skills VALUES("19","46","0","0","60");

# Fighter1 skills on axe (Fighter2 inherits from Fighter1)
INSERT INTO character_skills VALUES("6","2","0","0","100");

# Tribe skills on mining
INSERT INTO character_skills VALUES("20","37","200","200","200");

# Strong fighter8 to test loot cost cap
INSERT INTO character_skills VALUES("87","46","0","0","400");
INSERT INTO character_skills VALUES("87","47","0","0","400");
INSERT INTO character_skills VALUES("87","48","0","0","400");
INSERT INTO character_skills VALUES("87","49","0","0","400");
INSERT INTO character_skills VALUES("87","50","0","0","400");
INSERT INTO character_skills VALUES("87","51","0","0","400");
