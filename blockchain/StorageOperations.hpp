#pragma once

#include <blockchain/Operations.hpp>
#include <blockchain/StorageTypes.hpp>

namespace thinkyoung {
    namespace blockchain {

        struct StorageDataChangeType
        {
            StorageDataType storage_before;
            StorageDataType storage_after;
        };

        struct StorageOperation
        {
            static const OperationTypeEnum type;

            StorageOperation(){}

            ContractIdType contract_id;
            std::map<std::string, StorageDataChangeType> contract_change_storages;

        };

    }
} // thinkyoung::blockchain


FC_REFLECT(thinkyoung::blockchain::StorageDataChangeType, (storage_before)(storage_after))
FC_REFLECT(thinkyoung::blockchain::StorageOperation, (contract_id)(contract_change_storages))
