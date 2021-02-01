UPDATE `creature_template` SET `ScriptName` = 'npc_experience_chromie' WHERE `ScriptName` = 'npc_experience';

CREATE TABLE IF NOT EXISTS `chromie_beta_testers` (
    `guid` int(10) unsigned NOT NULL DEFAULT 0,
    `isBetaTester` tinyint(1) unsigned NOT NULL DEFAULT 0,
    `comment` varchar(50) DEFAULT '',
    PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Chromie Beta Testers';
