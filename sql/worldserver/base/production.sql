UPDATE `creature_template` SET `ScriptName` = 'npc_experience_chromie' WHERE `ScriptName` = 'npc_experience';

SET @CHROMIE_STRING_START = 90000;
DELETE FROM `acore_string` WHERE `entry` >= @CHROMIE_STRING_START;
INSERT INTO `acore_string` (`entry`, `content_default`) VALUES
(@CHROMIE_STRING_START+0, 'Your level is too low to become a beta tester.'),
(@CHROMIE_STRING_START+1, 'You are already a beta tester. Go test stuff and report bugs!'),
(@CHROMIE_STRING_START+2, 'Congratulations! You are now a ChromieCraft Beta Tester.\nYour mission is to help our project by testing the game contents and reporting bugs.'),
(@CHROMIE_STRING_START+3, 'Only beta testers are allowed to go further. You can become one by using .beta activate');
