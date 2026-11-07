#pragma once

#include <QString>
#include <QStringList>

namespace edu::SqlScript {

// Splits a .sql file into single statements, because Qt's SQLite driver runs
// only one statement per exec().
//
// Handles: 'string' and "identifier" quotes (with doubled-quote escapes),
// [bracket] and `backtick` identifiers, -- line and /* block */ comments
// (removed from the output), and CREATE TRIGGER bodies, whose inner
// semicolons do not end the statement until the closing END.
QStringList split(const QString& script);

} // namespace edu::SqlScript
