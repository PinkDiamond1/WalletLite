#include <blockchain/AccountOperations.hpp>
#include <blockchain/Exceptions.hpp>
#include <fc/time.hpp>


namespace thinkyoung {
    namespace blockchain {

        bool RegisterAccountOperation::is_delegate()const
        {
            return delegate_pay_rate <= 100;
        }


        bool UpdateAccountOperation::is_retracted()const
        {
            return this->active_key.valid() && *this->active_key == PublicKeyType();
        }

        bool UpdateAccountOperation::is_delegate()const
        {
            return delegate_pay_rate <= 100;
        }
    }
} // thinkyoung::blockchain
