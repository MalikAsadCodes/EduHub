#include "core/SqlScript.h"

namespace edu::SqlScript {

namespace {

enum class State { Normal, SingleQuote, DoubleQuote, Bracket, Backtick, LineComment, BlockComment };

bool isWordChar(QChar c)
{
    return c.isLetterOrNumber() || c == QLatin1Char('_');
}

// Tracks one statement while it is being collected.
struct Statement {
    QString text;
    QStringList leadingWords; // first few keywords, upper-case
    bool isTrigger = false;
    bool seenBegin = false;
    int depth = 0; // BEGIN/CASE ... END nesting inside a trigger

    void addWord(const QString& word)
    {
        const QString upper = word.toUpper();
        if (leadingWords.size() < 3) {
            leadingWords.append(upper);
            const bool create = leadingWords.constFirst() == QLatin1String("CREATE");
            if (create && leadingWords.size() == 2 && upper == QLatin1String("TRIGGER"))
                isTrigger = true;
            if (create && leadingWords.size() == 3 && upper == QLatin1String("TRIGGER") &&
                (leadingWords.at(1) == QLatin1String("TEMP") ||
                 leadingWords.at(1) == QLatin1String("TEMPORARY")))
                isTrigger = true;
        }
        if (!isTrigger)
            return;
        if (upper == QLatin1String("BEGIN")) {
            seenBegin = true;
            ++depth;
        } else if (upper == QLatin1String("CASE")) {
            ++depth;
        } else if (upper == QLatin1String("END") && depth > 0) {
            --depth;
        }
    }

    // A semicolon ends the statement unless we are inside a trigger body.
    bool semicolonEnds() const { return !isTrigger || (seenBegin && depth == 0); }
};

} // namespace

QStringList split(const QString& script)
{
    QStringList statements;
    Statement current;
    QString word;
    State state = State::Normal;

    auto flushWord = [&]() {
        if (!word.isEmpty()) {
            current.addWord(word);
            word.clear();
        }
    };
    auto flushStatement = [&]() {
        const QString trimmed = current.text.trimmed();
        if (!trimmed.isEmpty())
            statements.append(trimmed);
        current = Statement();
    };

    const qsizetype length = script.size();
    for (qsizetype i = 0; i < length; ++i) {
        const QChar c = script.at(i);
        const QChar next = (i + 1 < length) ? script.at(i + 1) : QChar();

        switch (state) {
        case State::Normal:
            if (c == QLatin1Char('-') && next == QLatin1Char('-')) {
                flushWord();
                state = State::LineComment;
                current.text += QLatin1Char(' ');
                ++i;
                break;
            }
            if (c == QLatin1Char('/') && next == QLatin1Char('*')) {
                flushWord();
                state = State::BlockComment;
                current.text += QLatin1Char(' ');
                ++i;
                break;
            }
            if (isWordChar(c)) {
                word += c;
                current.text += c;
                break;
            }
            flushWord();
            if (c == QLatin1Char(';') && current.semicolonEnds()) {
                flushStatement();
                break;
            }
            if (c == QLatin1Char('\''))
                state = State::SingleQuote;
            else if (c == QLatin1Char('"'))
                state = State::DoubleQuote;
            else if (c == QLatin1Char('['))
                state = State::Bracket;
            else if (c == QLatin1Char('`'))
                state = State::Backtick;
            current.text += c;
            break;

        case State::SingleQuote:
        case State::DoubleQuote:
        case State::Backtick: {
            const QChar quote = state == State::SingleQuote   ? QLatin1Char('\'')
                                : state == State::DoubleQuote ? QLatin1Char('"')
                                                              : QLatin1Char('`');
            current.text += c;
            if (c == quote) {
                if (next == quote) { // doubled quote is an escape
                    current.text += next;
                    ++i;
                } else {
                    state = State::Normal;
                }
            }
            break;
        }

        case State::Bracket:
            current.text += c;
            if (c == QLatin1Char(']'))
                state = State::Normal;
            break;

        case State::LineComment:
            if (c == QLatin1Char('\n')) {
                state = State::Normal;
                current.text += c;
            }
            break;

        case State::BlockComment:
            if (c == QLatin1Char('*') && next == QLatin1Char('/')) {
                state = State::Normal;
                ++i;
            }
            break;
        }
    }

    flushWord();
    flushStatement();
    return statements;
}

} // namespace edu::SqlScript
