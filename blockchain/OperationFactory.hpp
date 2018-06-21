#pragma once

#include <blockchain/Exceptions.hpp>
#include <blockchain/Operations.hpp>

namespace thinkyoung {
    namespace blockchain {

        /**
         * @class operation_factory
         *
         *  Enables polymorphic creation and serialization of operation objects in
         *  an manner that can be extended by derived chains.
         */
        class OperationFactory
        {
        public:
            static OperationFactory& instance();
            class OperationConverterBase
            {
            public:
                virtual ~OperationConverterBase(){};
                virtual void to_variant(const thinkyoung::blockchain::Operation& in, fc::variant& out) = 0;
                virtual void from_variant(const fc::variant& in, thinkyoung::blockchain::Operation& out) = 0;
            };

            template<typename OperationType>
            class operation_converter : public OperationConverterBase
            {
            public:
                virtual void to_variant(const thinkyoung::blockchain::Operation& in, fc::variant& output)
                {
                    try {
                        FC_ASSERT(in.type == OperationType::type);
                        fc::mutable_variant_object obj("type", in.type);

                        obj["data"] = fc::raw::unpack<OperationType>(in.data);

                        output = std::move(obj);
                    } FC_RETHROW_EXCEPTIONS(warn, "")
                }

                virtual void from_variant(const fc::variant& in, thinkyoung::blockchain::Operation& output)
                {
                    try {
                        auto obj = in.get_object();

                        FC_ASSERT(output.type == OperationType::type);
                        output.data = fc::raw::pack(obj["data"].as<OperationType>());
                    } FC_RETHROW_EXCEPTIONS(warn, "type: ${type}", ("type", fc::get_typename<OperationType>::name()))
                }

            };

            template<typename OperationType>
            void   register_operation()
            {
                FC_ASSERT(_converters.find(OperationType::type) == _converters.end(),
                    "Operation ID already Registered ${id}", ("id", OperationType::type));
                _converters[OperationType::type] = std::make_shared< operation_converter<OperationType> >();
            }

            /// defined in operations.cpp
            void to_variant(const thinkyoung::blockchain::Operation& in, fc::variant& output);
            /// defined in operations.cpp
            void from_variant(const fc::variant& in, thinkyoung::blockchain::Operation& output);

        private:
            std::unordered_map<int, std::shared_ptr<OperationConverterBase> > _converters;
        };

    }
} // thinkyoung::blockchain 
