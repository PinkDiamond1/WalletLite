#include <blockchain/StorageTypes.hpp>


namespace thinkyoung {
    namespace blockchain {

        std::map <StorageValueTypes, std::string> storage_type_map = \
        {
            make_pair(storage_value_null, std::string("nil")),
                make_pair(storage_value_int, std::string("int")),
                make_pair(storage_value_number, std::string("number")),
                make_pair(storage_value_bool, std::string("bool")),
                make_pair(storage_value_string, std::string("string")),
                make_pair(storage_value_unknown_table, std::string("Map<unknown type>")),
                make_pair(storage_value_int_table, std::string("Map<int>")),
                make_pair(storage_value_number_table, std::string("Map<number>")),
                make_pair(storage_value_bool_table, std::string("Map<bool>")),
                make_pair(storage_value_string_table, std::string("Map<string>")),
                make_pair(storage_value_unknown_array, std::string("Array<unknown type>")),
                make_pair(storage_value_int_array, std::string("Array<int>")),
                make_pair(storage_value_number_array, std::string("Array<number>")),
                make_pair(storage_value_bool_array, std::string("Array<bool>")),
                make_pair(storage_value_string_array, std::string("Array<string>"))
        };

        const uint8_t StorageNullType::type = storage_value_null;
        const uint8_t StorageIntType::type = storage_value_int;
        const uint8_t StorageNumberType::type = storage_value_number;
        const uint8_t StorageBoolType::type = storage_value_bool;
        const uint8_t StorageStringType::type = storage_value_string;

        const uint8_t StorageIntTableType::type = storage_value_int_table;
        const uint8_t StorageNumberTableType::type = storage_value_number_table;
        const uint8_t StorageBoolTableType::type = storage_value_bool_table;
        const uint8_t StorageStringTableType::type = storage_value_string_table;

        const uint8_t StorageIntArrayType::type = storage_value_int_array;
        const uint8_t StorageNumberArrayType::type = storage_value_number_array;
        const uint8_t StorageBoolArrayType::type = storage_value_bool_array;
        const uint8_t StorageStringArrayType::type = storage_value_string_array;

    }
}
