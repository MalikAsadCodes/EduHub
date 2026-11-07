-- 001_init.sql
-- Edu Hub base schema: app settings and user accounts.
--
-- Never edit this file once it has been applied to a database.
-- Schema changes always go in a new file (002_..., 003_...).
-- Dates are ISO-8601 text in UTC, e.g. 2026-10-24T09:30:00Z.

CREATE TABLE settings (
    key   TEXT PRIMARY KEY,
    value TEXT NOT NULL
) WITHOUT ROWID;

INSERT INTO settings (key, value) VALUES
    ('institution_name',          'Edu Hub University'),
    ('institution_logo',          ''),
    ('attendance_warn_percent',   '80'),
    ('attendance_min_percent',    '75'),
    ('max_credit_hours_per_term', '21');

-- Login accounts. Admin, teacher and student profiles link to this in later
-- migrations. Passwords are stored only as salted PBKDF2 hashes (M1).
CREATE TABLE users (
    id                   INTEGER PRIMARY KEY,
    username             TEXT    NOT NULL UNIQUE COLLATE NOCASE,
    password_hash        TEXT    NOT NULL,
    salt                 TEXT    NOT NULL,
    role                 TEXT    NOT NULL CHECK (role IN ('admin', 'teacher', 'student')),
    is_active            INTEGER NOT NULL DEFAULT 1 CHECK (is_active IN (0, 1)),
    must_change_password INTEGER NOT NULL DEFAULT 0 CHECK (must_change_password IN (0, 1)),
    failed_login_count   INTEGER NOT NULL DEFAULT 0 CHECK (failed_login_count >= 0),
    locked_until         TEXT,
    created_at           TEXT    NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%SZ', 'now')),
    last_login_at        TEXT,
    CHECK (length(trim(username)) >= 3)
);
