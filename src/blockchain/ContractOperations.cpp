#include "blockchain/ContractOperations.hpp"
#include "blockchain/Exceptions.hpp"
#include "fc/crypto/ripemd160.hpp"
#include "fc/crypto/sha512.hpp"
#include "blockchain/BalanceOperations.hpp"
#include <sstream>

#define  CLOSE_REGISTER_CONTRACT 0
namespace thinkyoung
{
    namespace blockchain
    {
        bool is_contract_has_method(const string& method_name, const std::set<std::string>& abi_set)
        {
            class finder
            {
            public:
                finder(const std::string& _cmp_str) : cmp_str(_cmp_str){}
                bool operator()(const std::string& str){
                    return str == cmp_str;
                }
            private:
                std::string cmp_str;
            };

            auto iter = abi_set.begin();
            if ((iter = std::find_if(abi_set.begin(), abi_set.end(), finder(method_name))) != abi_set.end())
                return true;

            return false;
        }

        ShareType get_amount_sum(ShareType amount_l, ShareType amount_r)
        {
            ShareType amount_sum = amount_l + amount_r;
            if (amount_sum < amount_l || amount_sum < amount_r)
                FC_CAPTURE_AND_THROW(addition_overflow, ("Addition overflow"));
            return amount_sum;
        }

        void withdraw_enough_balances(const std::map<BalanceIdType, ShareType>& balances_from, ShareType amount_required, std::map<BalanceIdType, ShareType>& balances_to)
        {
            balances_to.clear();
            std::map<BalanceIdType, ShareType>::const_iterator it = balances_from.begin();
            while (it != balances_from.end())
            {
                if (amount_required <= it->second)
                {
                    balances_to.insert(std::pair<BalanceIdType, ShareType>(it->first, amount_required));
                    break;
                }
                else
                {
                    balances_to.insert(std::pair<BalanceIdType, ShareType>(it->first, it->second));
                    amount_required = amount_required - it->second;
                }
                ++it;
            }

            if (it == balances_from.end())
                FC_CAPTURE_AND_THROW(insufficient_funds, (amount_required));
        }

        bool CallContractOperation::is_function_not_allow_call(const string& method)
        {
            return method == CON_INIT_INTERFACE || method == CON_ON_DEPOSIT_INTERFACE
                || method == CON_ON_UPGRADE_INTERFACE || method == CON_ON_DESTROY_INTERFACE;
        }
        ContractIdType RegisterContractOperation::get_contract_id() const
        {
            ContractIdType id;
            fc::sha512::encoder enc;
            fc::raw::pack(enc, *this);
            id.addr = fc::ripemd160::hash(enc.result());
            return id;
        }
    }
}

