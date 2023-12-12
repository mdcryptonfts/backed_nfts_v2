#pragma once

//Tables on other contracts

struct [[eosio::table]] assets_s {
  uint64_t asset_id;
  name collection_name;
  name schema_name;
  int32_t template_id;
  name ram_payer;
  std::vector<asset> backed_tokens;
  std::vector<uint8_t> immutable_serialized_data;
  std::vector<uint8_t> mutable_serialized_data;
  uint64_t primary_key() const { return asset_id; }
};
typedef multi_index<"assets"_n, assets_s> atomics_t;

struct [[eosio::table]] stat {
  asset    	supply;
  asset 	max_supply;
  name 		issuer;

  uint64_t primary_key()const { return supply.symbol.code().raw(); }
};
typedef eosio::multi_index< "stat"_n, stat > stat_table;


//Tables on this contract

struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] config {
	uint64_t  								ID;

	uint64_t primary_key() const { return ID; }
};
using config_table = eosio::multi_index<"config"_n, config
>;
