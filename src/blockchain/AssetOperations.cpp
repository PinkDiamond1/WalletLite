#include <blockchain/AssetOperations.hpp>
#include <blockchain/Exceptions.hpp>


namespace thinkyoung {
    namespace blockchain {

        bool is_power_of_ten(uint64_t n)
        {
            switch (n)
            {
            case 1ll:
            case 10ll:
            case 100ll:
            case 1000ll:
            case 10000ll:
            case 100000ll:
            case 1000000ll:
            case 10000000ll:
            case 100000000ll:
            case 1000000000ll:
            case 10000000000ll:
            case 100000000000ll:
            case 1000000000000ll:
            case 10000000000000ll:
            case 100000000000000ll:
            case 1000000000000000ll:
                return true;
            default:
                return false;
            }
        }
    }
} // thinkyoung::blockchain
