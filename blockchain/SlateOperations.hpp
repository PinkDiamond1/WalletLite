#pragma once

#include <blockchain/Operations.hpp>

namespace thinkyoung {
    namespace blockchain {

        struct DefineSlateOperation
        {
            static const OperationTypeEnum type;

            std::vector<fc::signed_int> slate;

        };

    }
} // thinkyoung::blockchain

FC_REFLECT(thinkyoung::blockchain::DefineSlateOperation, (slate))
