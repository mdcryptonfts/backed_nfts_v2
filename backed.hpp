#pragma once

#define CONTRACT_NAME "backednfts"
#define mix64to128(a, b) (uint128_t(a) << 64 | uint128_t(b))

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/symbol.hpp>
#include <eosio/crypto.hpp>
#include <eosio/transaction.hpp>
#include "include/atomic.hpp"
#include "include/atomicdata.hpp"
#include "structs.hpp"
#include "constants.hpp"
#include "tables.hpp"

using namespace eosio;
using namespace std;

CONTRACT backednfts : public contract {
	public:
		using contract::contract;

		backednfts(name receiver, name code, datastream<const char *> ds):contract(receiver, code, ds){}

		//Main Actions
		ACTION clearassets();
		ACTION clearconfig();
		ACTION clearwlist();
		ACTION addblacklist(const std::vector<eosio::name>& contracts_to_blacklist);
		ACTION addnewsigner(const eosio::name& signer_name);
		ACTION addwhitelist(const eosio::symbol& token_symbol, const eosio::name& contract);
		ACTION announcedepo(const eosio::name& user, const std::vector<FUNGIBLE_TOKEN>& tokens);
		ACTION backnft(const eosio::name& user, const eosio::name& asset_owner, const uint64_t& asset_id, const std::vector<FUNGIBLE_TOKEN>& tokens_to_back);
		ACTION blacklistsym(const std::vector<eosio::symbol_code>& symbols_to_blacklist);
		ACTION claimall(const name& claimer, const std::vector<uint64_t>& asset_ids,
			const std::vector<eosio::name>& contract_ignore_list, const uint8_t& limit);
		ACTION claimtokens(const name& claimer, const uint64_t& asset_id,
			const std::vector<eosio::name>& contract_ignore_list, const uint8_t& limit);
		ACTION initconfig();
		ACTION logbackasset(const eosio::name& backer, const eosio::name& asset_owner, 
			const uint64_t& asset_id, const std::vector<FUNGIBLE_TOKEN>& tokens_to_back, 
			const eosio::name& collection_name, const eosio::name& schema_name,
			const int32_t& template_id);
		ACTION logremaining(const uint64_t& asset_id, const std::vector<FUNGIBLE_TOKEN>& backed_tokens);
		ACTION removesigner(const eosio::name& signer_name);
		ACTION rmvblacklist(const std::vector<eosio::name>& contracts_to_remove);
		ACTION rmvblacksym(const std::vector<eosio::symbol_code>& symbols_to_remove);
		ACTION rmvwhitelist(const eosio::symbol& token_symbol, const eosio::name& contract);
		ACTION setclaimable(const eosio::name& authorizer, const std::vector<ASSET_UPDATE>& assets_to_update);
		ACTION setthreshold(const uint8_t& new_threshold);
		ACTION withdraw(const name& user, const vector<name>& contract_ignore_list);

		//Notifications
		[[eosio::on_notify("atomicassets::transfer")]] void handle_nft_transfer(name _owner, name receiver, 
				std::vector<uint64_t> &ids, std::string memo);
		[[eosio::on_notify("atomicassets::logburnasset")]] void listen_for_burn(name asset_owner, uint64_t asset_id, name collection_name,
  			name schema_name, int32_t template_id, vector<asset> backed_tokens, atomicdata::ATTRIBUTE_MAP old_immutable_data,
  			atomicdata::ATTRIBUTE_MAP old_mutable_data, name asset_ram_payer);
		[[eosio::on_notify("*::transfer")]] void receive_token_transfer(name from, name to, eosio::asset quantity, std::string memo);

	private:

		//Tables
		balances_tbl balances_t = balances_tbl(get_self(), get_self().value);
		beta_table beta_t = beta_table(get_self(), get_self().value);
		black_syms blacklisted_symbols_t = black_syms(get_self(), get_self().value);
		black_table black_t = black_table(get_self(), get_self().value);
		config_table config_t = config_table(get_self(), get_self().value);
		nfts_table nfts_t = nfts_table(get_self(), get_self().value);
		white_table white_t = white_table(get_self(), get_self().value);

		//Functions
		void check_for_duplicates(const std::vector<FUNGIBLE_TOKEN>& tokens_to_back);
		void check_token_exists(const symbol& token_symbol, const name& contract);
		bool contract_is_blacklisted(const name& contract);
		std::vector<std::string> get_words(std::string memo);
		bool has_balance_object(const eosio::name& user, const eosio::name& contract, const eosio::symbol& token_symbol);
		bool is_an_authorizer(const eosio::name& wallet, const std::vector<eosio::name>& authorizers);
		bool is_beta_tester(const eosio::name& user);
		bool is_ignored(const eosio::name& contract, const std::vector<eosio::name>& ignore_list);
		void log_back_asset(const eosio::name& backer, const eosio::name& asset_owner, 
			const uint64_t& asset_id, const std::vector<FUNGIBLE_TOKEN>& tokens_to_back, 
			const eosio::name& collection_name, const eosio::name& schema_name,
			const int32_t& template_id);
		void log_remaining(const uint64_t& asset_id, const std::vector<FUNGIBLE_TOKEN>& backed_tokens);
		std::vector<FUNGIBLE_TOKEN> remove_zero_balances(const std::vector<FUNGIBLE_TOKEN>& balances);
		bool symbol_is_blacklisted(const eosio::symbol_code& symbol);
		bool token_is_whitelisted(const symbol& token_symbol, const name& contract);
		void transfer_tokens(const name& user, const asset& amount_to_send, const name& contract, const std::string& memo);
		void upsert_claimable_tokens(const name& user, vector<FUNGIBLE_TOKEN>& tokens);

		//Safemath
		int64_t safeAddInt64(const int64_t& a, const int64_t& b);
};
