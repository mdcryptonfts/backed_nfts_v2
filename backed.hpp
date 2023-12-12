#pragma once

#define CONTRACT_NAME "backednfts"
#define mix64to128(a, b) (uint128_t(a) << 64 | uint128_t(b))

#include <eosio/eosio.hpp>
#include <eosio/print.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/symbol.hpp>
#include <string>
#include <eosio/crypto.hpp>
#include <eosio/transaction.hpp>
#include "include/atomic.hpp"
#include "include/atomicdata.hpp"
#include<map>
#include "structs.hpp"
#include "constants.hpp"
#include "tables.hpp"

using namespace eosio;


CONTRACT backednfts : public contract {
	public:
		using contract::contract;

		backednfts(name receiver, name code, datastream<const char *> ds):contract(receiver, code, ds){}

		//Main Actions
		ACTION initconfig(const eosio::name& filter_type);

		//Notifications
		[[eosio::on_notify("atomicassets::logburnasset")]] void listen_for_burn(name asset_owner, uint64_t asset_id, name collection_name,
  			name schema_name, int32_t template_id, vector<asset> backed_tokens, atomicdata::ATTRIBUTE_MAP old_immutable_data,
  			atomicdata::ATTRIBUTE_MAP old_mutable_data, name asset_ram_payer);

	private:

		//Tables
		config_table config_t = config_table(get_self(), get_self().value);

		//Functions
		void check_token_exists(const symbol& token_symbol, const name& contract);
		void transfer_tokens(const name& user, const asset& amount_to_send, const name& contract, const std::string& memo);
};
