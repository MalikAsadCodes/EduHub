-- 002_user_profiles.sql
-- A display name for every account (shown in the header and lists).
-- Teacher and student names also live on their profiles (later migrations);
-- for administrators this is the only place a name is stored.

ALTER TABLE users ADD COLUMN display_name TEXT NOT NULL DEFAULT '';

UPDATE users SET display_name = username WHERE display_name = '';

CREATE INDEX idx_users_role ON users (role);
