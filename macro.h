#ifndef __MACRO_H__
#define __MACRO_H__

#define COMMONASSET "ACT"

#define WALLET_WIDTH 960
#define WALLET_HEIGHT 580

#define AUTO_REFRESH_TIME 30000
#define PWD_LOCK_TIME 7200
#define ACHAIN_ACCOUNT_MAX 50

#define VERSION_CONFIG "version.ini"
#define UPDATE_TOOL_NAME "AchainUp.exe"

#ifdef WIN32
#define ACHAIN_WALLET_VERSION "2.0.4"
#else
#define ACHAIN_WALLET_VERSION "2.0.4"
#endif
#define ACHAIN_WALLET_VERSION_STR "v" ACHAIN_WALLET_VERSION

//#define TEST_CHAIN

#ifdef TEST_CHAIN
#define CHAIN_ID "5260ca3470af412ea1dc9fd647903901b9adb4d618effec8f4f9479eaa0c9c69"
#else
#define CHAIN_ID "6a1cb528f6e797e58913bff7a45cdd4709be75114ccd1ccb0e611b808f4d1b75"
#endif

#ifdef TEST_CHAIN
#define ACHAIN_BROWSER_URL "http://172.16.60.104/#/"
#define BROWSER_SERVER_DOMAIN "http://172.16.60.104/"
#define WALLET_SERVER_DOMAIN "http://wallet.achain.com/"
#else
#define ACHAIN_BROWSER_URL "https://browser.achain.com/#/"
#define BROWSER_SERVER_DOMAIN "https://lite.achain.com/"
#define WALLET_SERVER_DOMAIN "http://wallet.achain.com/"
#endif

#define REPORT_REQ BROWSER_SERVER_DOMAIN "wallets/report/"
#define BALANCE_REQ BROWSER_SERVER_DOMAIN "wallets/api/browser/act/contract/balance/query/"
#define TRANSACTION_REQ BROWSER_SERVER_DOMAIN "wallets/api/browser/act/contractTransactionAll?"
#define OTHER_TOKEN_REQ BROWSER_SERVER_DOMAIN "wallets/api/browser/act/contractTransactions?"
#define CON_CONFOG_URL BROWSER_SERVER_DOMAIN "wallets/api/browser/act/getAllContracts?status=2"
#define BROADCAST_URL BROWSER_SERVER_DOMAIN "wallets/api/wallet/act/network_broadcast_transaction"
#define TRACK_URL BROWSER_SERVER_DOMAIN "collect/api/browser/achain/collectLog"
#define QUOTATION_URL WALLET_SERVER_DOMAIN "api/v1/quotations/"
#define EXCHANGE_RATE_URL WALLET_SERVER_DOMAIN "api/v1/rates/"

#endif // __MACRO_H__
