#include "achainlightwalletapi.h"
#include <fc/crypto/elliptic.hpp>
#include <fc/crypto/base58.hpp>
#include <fc/crypto/sha256.hpp>

#include <blockchain/Exceptions.hpp>
#include <blockchain/ContractOperations.hpp>
#include <blockchain/BalanceOperations.hpp>
#include <blockchain/ImessageOperations.hpp>
#include <blockchain/AssetOperations.hpp>
#include <blockchain/WithdrawTypes.hpp>
#include <blockchain/WithdrawTypes.hpp>

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

#include <QJsonDocument>
#include <QDebug>

#include <database.h>
#include <datamgr.h>

#define INVALIDE_SUB_ADDRESS ("ffffffffffffffffffffffffffffffff")
namespace thinkyoung {
namespace blockchain {
Address generate_address()
{
    fc::ecc::private_key key = fc::ecc::private_key::generate();
    Address addr = thinkyoung::blockchain::Address(key.get_public_key());
    return addr;
}
std::string key_to_wif(const fc::ecc::private_key& key)
{
    fc::sha256 secret = key.get_secret();
    const size_t size_of_data_to_hash = sizeof(secret) + 1;
    const size_t size_of_hash_bytes = 4;
    char data[size_of_data_to_hash + size_of_hash_bytes];
    data[0] = (char)0x80;
    memcpy(&data[1], (char*)&secret, sizeof(secret));
    fc::sha256 digest = fc::sha256::hash(data, size_of_data_to_hash);
    digest = fc::sha256::hash(digest);
    memcpy(data + size_of_data_to_hash, (char*)&digest, size_of_hash_bytes);
    return fc::to_base58(data, sizeof(data));
}

/**
      * @deprecated - this is for backward compatibility of keys generated
      */
std::string key_to_wif_single_hash(const fc::ecc::private_key& key)
{
    fc::sha256 secret = key.get_secret();
    const size_t size_of_data_to_hash = sizeof(secret) + 1;
    const size_t size_of_hash_bytes = 4;
    char data[size_of_data_to_hash + size_of_hash_bytes];
    data[0] = (char)0x80;
    memcpy(&data[1], (char*)&secret, sizeof(secret));
    fc::sha256 digest = fc::sha256::hash(data, size_of_data_to_hash);
    digest = fc::sha256::hash(digest);
    memcpy(data + size_of_data_to_hash, (char*)&digest, size_of_hash_bytes);
    return fc::to_base58(data, sizeof(data));
}

fc::optional<fc::ecc::private_key> wif_to_key(const std::string& wif_key)
{
    std::vector<char> wif_bytes = fc::from_base58(wif_key);
    if (wif_bytes.size() < 5)
        return fc::optional<fc::ecc::private_key>();
    std::vector<char> key_bytes(wif_bytes.begin() + 1, wif_bytes.end() - 4);
    fc::ecc::private_key key = fc::variant(key_bytes).as<fc::ecc::private_key>();
    fc::sha256 check = fc::sha256::hash(wif_bytes.data(), wif_bytes.size() - 4);
    fc::sha256 check2 = fc::sha256::hash(check);

    if (memcmp((char*)&check, wif_bytes.data() + wif_bytes.size() - 4, 4) == 0 ||
            memcmp((char*)&check2, wif_bytes.data() + wif_bytes.size() - 4, 4) == 0)
        return key;

    return fc::optional<fc::ecc::private_key>();
}

SignedTransaction wallet_transfer_to_address(
		const std::string& real_amount_to_transfer,
		int asset_id,
        const std::string& from_address,
        const std::string& to_address,
        const std::string& memo_message)
{
        string strToAccount;
        string strSubAccount;
        accountsplit(to_address, strToAccount, strSubAccount);
        Address effective_address;
        if (Address::is_valid(strToAccount))
            effective_address = Address(strToAccount);
        else
            effective_address = Address(PublicKeyType(strToAccount));

        const int64_t precision = 100000;

        //FC_ASSERT(QString::isNumber(real_amount_to_transfer), "inputed amount is not a number");
        auto ipos = real_amount_to_transfer.find(".");
        if (ipos != string::npos)
        {
            string str = real_amount_to_transfer.substr(ipos + 1);
            int64_t precision_input = static_cast<int64_t>(pow(10, str.size()));
            FC_ASSERT((precision_input <= precision), "Precision is not correct");
        }
        double dAmountToTransfer = std::stod(real_amount_to_transfer);
        ShareType amount_to_transfer = static_cast<ShareType>(floor(dAmountToTransfer * precision + 0.5));
        Asset asset_to_transfer(amount_to_transfer, asset_id);
        Address from_addr(from_address);
        SignedTransaction trx;
        if (strSubAccount != "") {
            //trx.from_account = from_address;
            trx.alp_account = strSubAccount;
            trx.alp_inport_asset = asset_to_transfer;
        }
        const auto required_fees = Asset(1000, 0);
        const auto required_imessage_fee = Asset(0, 0);
        if (required_fees.asset_id == asset_to_transfer.asset_id) {
            WithdrawCondition condition = WithdrawCondition(WithdrawWithSignature(from_addr),
                                           0, 0);
            trx.withdraw(condition.get_address(),
                         (required_fees + asset_to_transfer + required_imessage_fee).amount);
        } else {
            WithdrawCondition condition1 = WithdrawCondition(WithdrawWithSignature(from_addr),
                                          asset_id, 0);
            trx.withdraw(condition1.get_address(),
                         asset_to_transfer.amount);
            WithdrawCondition condition2 = WithdrawCondition(WithdrawWithSignature(from_addr),
                                          0, 0);
            trx.withdraw(condition2.get_address(),
                         (required_fees + required_imessage_fee).amount);
        }
        trx.deposit(effective_address, asset_to_transfer);
        trx.expiration = fc::time_point_sec(fc::time_point::now() + fc::hours(1));
        if (memo_message != "") {
            trx.AddtionImessage(memo_message);
        }
        return trx;
}

SignedTransaction call_contract(const string& caller,
                               const ContractIdType contract,
                               const string& method,
                               const string& arguments,
                               const string& asset_symbol,
                               double cost_limit)

{
        if (arguments.length() > CONTRACT_PARAM_MAX_LEN)
            FC_CAPTURE_AND_THROW(contract_parameter_length_over_limit, ("the parameter length of contract function is over limit"));

        FC_ASSERT(cost_limit > 0, "cost_limit should greater than 0");
        //FC_ASSERT(data_ptr->is_valid_symbol(asset_symbol), "Invalid asset symbol");
        FC_ASSERT(asset_symbol == ALP_BLOCKCHAIN_SYMBOL, "asset_symbol must be ACT");

        ShareType amount_limit = cost_limit * 100000;
        Asset asset_limit(amount_limit, 0);
        SignedTransaction     trx;

        Asset fee = Asset(1000, 0);

        //WithdrawWithSignature()
        map<BalanceIdType, ShareType> balances;
        WithdrawCondition condition = WithdrawCondition(WithdrawWithSignature(Address(caller)), 0, 0);
        balances[condition.get_address()] =  (asset_limit+fee).amount;
        //TODO: get caller public_key
        PublicKeyType  caller_public_key = PublicKeyType(
        DataMgr::getInstance()->getAccountPublicKeyFromAddr(QString::fromStdString(caller)).toStdString());
        trx.call_contract(contract, method, arguments, caller_public_key, asset_limit,fee, balances);//�����Լ����op
        FC_ASSERT(fee.asset_id == 0, "register fee must be ACT");
        trx.expiration = fc::time_point_sec(fc::time_point::now() + fc::hours(1));
        return trx;
}

void accountsplit(const std::string &original, std::string &to_account,
                  std::string &sub_account)
{
    if (original.size() < 66)
    {
        to_account = original;
        return;
    }
    to_account = original.substr(0, original.size() - 32);
    sub_account = original.substr(original.size() - 32);
    if (INVALIDE_SUB_ADDRESS == sub_account)
    {
        sub_account = "";
    }
    else
    {
        sub_account = original;
    }
}

QJsonValue asset_to_json(const Asset& asset)
{
    QJsonObject json;
    json.insert("amount", QString::number(asset.amount));
    json.insert("asset_id", int32_t(asset.asset_id));
    return QJsonValue(json);
}

QJsonValue call_contract_to_json(const CallContractOperation &ops){

    QJsonObject json;
    json.insert("args", QString::fromStdString(ops.args));
    json.insert("caller", QString::fromStdString(std::string(ops.caller)));
    QJsonArray banlance;
    for (const auto& ban: ops.balances){
        QJsonValue first(QString::fromStdString(ban.first.AddressToString()));
        QJsonValue second(ban.second);
        QJsonArray maps;
        maps.append(first);
        maps.append(second);
        banlance.append(maps);
    }
    json.insert("balances", banlance);
    json.insert("method", QString::fromStdString(ops.method));
    json.insert("costlimit", asset_to_json(ops.costlimit));
    json.insert("contract", QString::fromStdString(ops.contract.AddressToString()));
    json.insert("transaction_fee", asset_to_json(ops.transaction_fee));
    return QJsonValue(json);
}

QJsonValue withdraw_to_json(const WithdrawOperation &ops){
    QJsonObject json;
    json.insert("claim_input_data","");
    json.insert("balance_id", QString::fromStdString(ops.balance_id.AddressToString()));
    json.insert("amount", QString::number(ops.amount));
    return QJsonValue(json);
}
QJsonValue imessage_to_json(const ImessageMemoOperation& message){
    QJsonObject json;
    json.insert("imessage", QString::fromStdString(message.imessage));
    return QJsonValue(json);
}
QJsonValue deposit_to_json(const DepositOperation& ops){

    QJsonObject condition;
    QJsonObject data;

    data.insert("owner",
            QString::fromStdString(ops.condition.as<WithdrawWithSignature>()\
                           .owner.AddressToString()));

    condition.insert("slate_id", int32_t(ops.condition.slate_id));
    condition.insert("data", data);
    condition.insert("balance_type",
                     QString::fromStdString(ops.condition.balance_type));
    condition.insert("asset_id", int32_t(ops.condition.asset_id));
    condition.insert("type", QString::fromStdString(ops.condition.type));

    QJsonObject json;
	json.insert("amount", QString::number(ops.amount));
    json.insert("condition",condition);
    return QJsonValue(json);
}

QJsonValue operations_to_json(const vector<Operation> &ops){
    QJsonArray array;
    for (const Operation& op : ops) {
        QJsonObject json;
        switch (op.type) {
        case OperationTypeEnum::call_contract_op_type:
            json.insert("data", call_contract_to_json(op.as<CallContractOperation>()));
            break;
        case OperationTypeEnum::deposit_op_type:
            json.insert("data", deposit_to_json(op.as<DepositOperation>()));
            break;
        case OperationTypeEnum::withdraw_op_type:
            json.insert("data", withdraw_to_json(op.as<WithdrawOperation>()));
            break;
        case OperationTypeEnum::imessage_memo_op_type:
            json.insert("data", imessage_to_json(op.as<ImessageMemoOperation>()));
            break;
        default:
            FC_ASSERT("error in operations");
            break;
        }
        json.insert("type", QString::fromStdString(op.type));
        array.append(json);
    }
    return QJsonValue(array);
}

QJsonValue signatures_to_json(const vector<fc::ecc::compact_signature>& signs){
    QJsonArray array;
    for(const auto& sign : signs){
        QByteArray byte;
        for (auto& c : sign){
            byte.append(c);
        }
        qDebug()<<"sign hex:"<<byte.toHex();
        QJsonValue val(QString(byte.toHex()));
        array.append(val);
    }
    return QJsonValue(array);
}

//only (withdraw/despi/call_contract operations)
//fc::reflect
QString signedtransaction_to_json(const SignedTransaction &trx) {

    QJsonObject transaction;
    transaction.insert("operations", operations_to_json(trx.operations));
    transaction.insert("alp_account", QString::fromStdString(trx.alp_account));
    transaction.insert("expiration",
                       QString::fromStdString(trx.expiration.to_iso_string()));

    transaction.insert("signatures", signatures_to_json(trx.signatures));
    transaction.insert("alp_inport_asset", asset_to_json(trx.alp_inport_asset));
    QJsonDocument qdoc;
    qdoc.setObject(transaction);
    return QString(qdoc.toJson(QJsonDocument::Compact));
}

}
}
