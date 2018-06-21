#include <blockchain/ExtendedAddress.hpp>
#include <blockchain/WithdrawTypes.hpp>

#include <fc/reflect/variant.hpp>


namespace thinkyoung {
    namespace blockchain {

        const uint8_t WithdrawWithSignature::type = withdraw_signature_type;
        const uint8_t WithdrawWithMultisig::type = withdraw_multisig_type;
        const uint8_t withdraw_with_escrow::type = withdraw_escrow_type;

        MemoStatus::MemoStatus(const ExtendedMemoData& memo, bool valid_signature,
            const fc::ecc::private_key& opk)
            :ExtendedMemoData(memo), has_valid_signature(valid_signature), owner_private_key(opk)
        {
        }

        void MemoData::set_message(const std::string& message_str)
        {
            if (message_str.empty()) return;
            FC_ASSERT(message_str.size() <= sizeof(message));
            memcpy(message.data, message_str.c_str(), message_str.size());
        }

        void ExtendedMemoData::set_message(const std::string& message_str)
        {
            if (message_str.empty()) return;
            FC_ASSERT(message_str.size() <= sizeof(message) + sizeof(extra_memo));
            if (message_str.size() <= sizeof(message))
            {
                memcpy(message.data, message_str.c_str(), message_str.size());
            }
            else
            {
                memcpy(message.data, message_str.c_str(), sizeof(message));
                memcpy(extra_memo.data, message_str.c_str() + sizeof(message), message_str.size() - sizeof(message));
            }
        }

        std::string MemoData::get_message()const
        {
            // add .c_str() to auto-truncate at null byte
            return std::string((const char*)&message, sizeof(message)).c_str();
        }

        std::string ExtendedMemoData::get_message()const
        {
            // add .c_str() to auto-truncate at null byte
            return (std::string((const char*)&message, sizeof(message))
                + std::string((const char*)&extra_memo, sizeof(extra_memo))).c_str();
        }

        BalanceIdType WithdrawCondition::get_address()const
        {
            return Address(*this);
        }

        set<Address> WithdrawCondition::owners()const
        {
            switch (WithdrawConditionTypes(type))
            {
            case withdraw_signature_type:
                return set < Address > { this->as<WithdrawWithSignature>().owner };
            case withdraw_multisig_type:
                return this->as<WithdrawWithMultisig>().owners;
            case withdraw_escrow_type:
            {
                const auto escrow = this->as<withdraw_with_escrow>();
                return set < Address > { escrow.sender, escrow.receiver, escrow.escrow };
            }
            default:
                return set<Address>();
            }
        }

        optional<Address> WithdrawCondition::owner()const
        {
            switch (WithdrawConditionTypes(type))
            {
            case withdraw_signature_type:
                return this->as<WithdrawWithSignature>().owner;
            default:
                return optional<Address>();
            }
        }

        string WithdrawCondition::type_label()const
        {
            string label = string(this->type);
            label = label.substr(9);
            label = label.substr(0, label.find("_"));
            QString str = QString::fromStdString(label);
            str.toUpper();
            return str.toStdString();
        }
    }
} // thinkyoung::blockchain

namespace fc {
    void to_variant(const thinkyoung::blockchain::WithdrawCondition& var, variant& vo)
    {
        using namespace thinkyoung::blockchain;
        fc::mutable_variant_object obj;
        obj["asset_id"] = var.asset_id;
        obj["slate_id"] = var.slate_id;
        obj["type"] = var.type;
		obj["balance_type"] = var.balance_type;
        switch ((WithdrawConditionTypes)var.type)
        {
        case withdraw_null_type:
            obj["data"] = fc::variant();
            break;
        case withdraw_signature_type:
            obj["data"] = fc::raw::unpack<WithdrawWithSignature>(var.data);
            break;
        case withdraw_multisig_type:
            obj["data"] = fc::raw::unpack<WithdrawWithMultisig>(var.data);
            break;
        case withdraw_escrow_type:
            obj["data"] = fc::raw::unpack<withdraw_with_escrow>(var.data);
            break;
            // No default to force compiler warning
        }
        vo = std::move(obj);
    }

    void from_variant(const variant& var, thinkyoung::blockchain::WithdrawCondition& vo)
    {
        using namespace thinkyoung::blockchain;
        auto obj = var.get_object();
        from_variant(obj["asset_id"], vo.asset_id);
        try
        {
            from_variant(obj["slate_id"], vo.slate_id);
        }
        catch (const fc::key_not_found_exception&)
        {
            from_variant(obj["delegate_slate_id"], vo.slate_id);
        }
        from_variant(obj["type"], vo.type);
		from_variant(obj["balance_type"],vo.balance_type);
        switch ((WithdrawConditionTypes)vo.type)
        {
        case withdraw_null_type:
            return;
        case withdraw_signature_type:
            vo.data = fc::raw::pack(obj["data"].as<WithdrawWithSignature>());
            return;
        case withdraw_multisig_type:
            vo.data = fc::raw::pack(obj["data"].as<WithdrawWithMultisig>());
            return;
        case withdraw_escrow_type:
            vo.data = fc::raw::pack(obj["data"].as<withdraw_with_escrow>());
            return;
            // No default to force compiler warning
        }
        FC_ASSERT(false, "Invalid withdraw condition!");
    }

    void to_variant(const thinkyoung::blockchain::MemoData& var, variant& vo)
    {
        mutable_variant_object obj("from", var.from);
        obj("from_signature", var.from_signature)
            ("message", var.get_message())
            ("memo_flags", var.memo_flags);
        vo = std::move(obj);
    }

    void from_variant(const variant& var, thinkyoung::blockchain::MemoData& vo)
    {
        try {
            const variant_object& obj = var.get_object();
            if (obj.contains("from"))
                vo.from = obj["from"].as<thinkyoung::blockchain::PublicKeyType>();
            if (obj.contains("from_signature"))
                vo.from_signature = obj["from_signature"].as_int64();
            if (obj.contains("message"))
                vo.set_message(obj["message"].as_string());
            if (obj.contains("memo_flags"))
                vo.memo_flags = obj["memo_flags"].as<thinkyoung::blockchain::MemoFlagsEnum>();
        } FC_RETHROW_EXCEPTIONS(warn, "unable to convert variant to memo_data", ("variant", var))
    }

} // fc
