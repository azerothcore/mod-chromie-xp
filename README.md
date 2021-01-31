# mode-chromie-xp


### SQL Utils

```sql
SET @LOCK_FLAG = 33554432;

-- Check which players have the XP locked
SELECT * FROM `characters` WHERE playerFlags & @LOCK_FLAG = @LOCK_FLAG;

-- Lock character XP - NOTE: the character MUST be offline.
UPDATE`characters` SET playerFlags = playerFlags | @LOCK_FLAG WHERE name = 'Craft';

-- Unlock character XP - NOTE: the character MUST be offline.
UPDATE`characters` SET playerFlags = playerFlags & ~@LOCK_FLAG WHERE name = 'Craft';

-- Check which players have the XP locked
SELECT * FROM `characters` WHERE playerFlags & @LOCK_FLAG = @LOCK_FLAG;

```