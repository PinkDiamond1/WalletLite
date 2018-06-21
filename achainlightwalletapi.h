#ifndef ACHAINLIGHTWALLETAPI_H
#define ACHAINLIGHTWALLETAPI_H

#include <blockchain/Address.hpp>
#include <blockchain/Transaction.hpp>
#include <string>
#include <map>

#include "misc.h"

#ifdef TEST_CHAIN
/*测试链ID*/
#define CHAIN_ID "5260ca3470af412ea1dc9fd647903901b9adb4d618effec8f4f9479eaa0c9c69"
#else
/*正式链ID*/
#define CHAIN_ID "6a1cb528f6e797e58913bff7a45cdd4709be75114ccd1ccb0e611b808f4d1b75"
#endif

namespace thinkyoung {
namespace blockchain {
    Address generate_address();
    void accountsplit(const string& original, string& to_account,
                      string& sub_account);

    QString signedtransaction_to_json(const SignedTransaction &trx);
    SignedTransaction wallet_transfer_to_address(
            const std::string& amount_to_transfer,
            int asset_id,
            const std::string& from_address,
            const std::string& to_address,
            const std::string& memo_message);

    SignedTransaction call_contract(const string& caller,
                                   const ContractIdType contract,
                                   const string& method,
                                   const string& arguments,
                                   const string& asset_symbol,
                                   double cost_limit);

    std::string key_to_wif_single_hash(const fc::ecc::private_key& key);
    std::string key_to_wif(const fc::ecc::private_key& key);
    fc::optional<fc::ecc::private_key> wif_to_key(const std::string& wif_key);
}
}

#endif // ACHAINLIGHTWALLETAPI_H
