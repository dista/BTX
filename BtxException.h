#ifndef BTX_EXCEPTION_H
#define BTX_EXCEPTION_H

#include <string>
#include <exception>

class BtxException: public std::exception
{
    private:
        std::string msg_;
    public:
        BtxException(const char* msg):
            msg_(msg)
        {
        }

        ~BtxException() throw() {}

        virtual const char* what() const throw()
        {
            return msg_.c_str();
        }
};


#endif
