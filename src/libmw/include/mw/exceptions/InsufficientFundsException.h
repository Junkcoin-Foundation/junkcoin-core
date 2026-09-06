#pragma once

#include <mw/exceptions/JKCException.h>

#define ThrowInsufficientFunds(msg) throw InsufficientFundsException(msg, __FUNCTION__)

class InsufficientFundsException : public JKCException
{
public:
    InsufficientFundsException(const std::string& message, const std::string& function)
        : JKCException("InsufficientFundsException", message, function)
    {
    }
};
