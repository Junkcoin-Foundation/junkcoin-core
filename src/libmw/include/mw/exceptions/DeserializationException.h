#pragma once

#include <mw/exceptions/JKCException.h>

#define ThrowDeserialization(msg) throw DeserializationException(msg, __FUNCTION__)

class DeserializationException : public JKCException
{
public:
    DeserializationException(const std::string& message, const std::string& function)
        : JKCException("DeserializationException", message, function)
    {
    }
};
