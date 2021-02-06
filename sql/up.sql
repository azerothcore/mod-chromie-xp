UPDATE `creature_template` SET `ScriptName` = 'npc_experience_chromie' WHERE `ScriptName` = 'npc_experience';

CREATE TABLE IF NOT EXISTS `chromie_beta_testers` (
    `guid` int(10) unsigned NOT NULL DEFAULT 0,
    `isBetaTester` tinyint(1) unsigned NOT NULL DEFAULT 0,
    `comment` varchar(50) DEFAULT '',
    PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Chromie Beta Testers';


SET @CHROMIE_STRING_START = 90000;
DELETE FROM `acore_string` WHERE `entry` >= @CHROMIE_STRING_START;
INSERT INTO `acore_string` (`entry`, `content_default`) VALUES
(@CHROMIE_STRING_START+0, 'Your level is too low to become a beta tester.'),
(@CHROMIE_STRING_START+1, 'You are already a beta tester. Go test stuff and report bugs!'),
(@CHROMIE_STRING_START+2, 'Congratulations! You are now a ChromieCraft Beta Tester.\nYour mission is to help our project by testing the game contents and reporting bugs.');
