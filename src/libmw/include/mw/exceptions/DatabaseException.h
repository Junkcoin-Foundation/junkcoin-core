#pragma once

#include <mw/exceptions/JKCException.h>

#define ThrowDatabase(msg) throw DatabaseException(msg, __FUNCTION__)

class DatabaseException : public JKCException
{
public:
    DatabaseException(const std::string& message, const std::string& function)
        : JKCException("DatabaseException", message, function)
    {
    }
};
