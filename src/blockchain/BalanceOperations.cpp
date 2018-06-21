#include <blockchain/BalanceOperations.hpp>
#include <blockchain/Exceptions.hpp>
#include <sstream>


namespace thinkyoung {
    namespace blockchain {

        BalanceIdType DepositOperation::balance_id()const
        {
            return condition.get_address();
        }

        DepositOperation::DepositOperation(const Address& owner,
            const Asset& amnt,
            SlateIdType slate_id)
        {
            FC_ASSERT(amnt.amount > 0, "Amount should be bigger than 0");
            amount = amnt.amount;
            condition = WithdrawCondition(WithdrawWithSignature(owner),
                amnt.asset_id, slate_id);
        }

        BalanceIdType DepositContractOperation::balance_id()const
        {
            return condition.get_address();
        }


    }
} // thinkyoung::blockchain
