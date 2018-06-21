#pragma once

#include <blockchain/Transaction.hpp>
#include <blockchain/Operations.hpp>

namespace thinkyoung {
    namespace blockchain {

        struct TransactionOperation
        {
            static const OperationTypeEnum type;

            TransactionOperation(){}

            TransactionOperation(const SignedTransaction& signed_trx)
                :trx(signed_trx){}

            TransactionOperation(const TransactionOperation& trx_op)
                :trx(trx_op.trx){}

            SignedTransaction trx;

        };

    }
} // thinkyoung::blockchain


FC_REFLECT(thinkyoung::blockchain::TransactionOperation, (trx))
