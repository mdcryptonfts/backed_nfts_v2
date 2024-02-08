#pragma once

//Tables on other contracts

struct [[eosio::table]] account {
  asset    balance;

  uint64_t primary_key()const { return balance.symbol.code().raw(); }
};
typedef eosio::multi_index< "accounts"_n, account > accounts;

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


struct [[eosio::table]] templates {
  int32_t template_id;
  name schema_name;
  bool transferable;
  bool burnable;
  uint32_t max_supply;
  uint32_t issued_supply;
  vector<uint8_t> immutable_serialized_data;
  int32_t primary_key() const { return template_id; }
};

typedef multi_index<"templates"_n, templates> atomic_temps;


//Tables on this contract

struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] backed_nfts {
  uint64_t                      asset_id;
  std::vector<FUNGIBLE_TOKEN>   backed_tokens;
  uint8_t                       is_claimable;
  eosio::name                   claimer;
  std::vector<AUTH_OBJECT>      received_auths;
  std::vector<AUTH_OBJECT>      required_auths;

  uint64_t primary_key() const { return asset_id; }
};
using nfts_table = eosio::multi_index<"backednfts"_n, backed_nfts
>;

struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] balances {
  eosio::name                   wallet;
  std::vector<FUNGIBLE_TOKEN>   balances;

  uint64_t primary_key() const { return wallet.value; }
};
using balances_tbl = eosio::multi_index<"balances"_n, balances
>;


struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] beta_testers {
  eosio::name   wallet;

  uint64_t primary_key() const { return wallet.value; }
};
using beta_table = eosio::multi_index<"betatesters"_n, beta_testers
>;

struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] blacklisted_symbols {
  eosio::symbol_code    symbol_code;
  uint64_t primary_key() const { return symbol_code.raw(); }
};
using black_syms = eosio::multi_index<"blacksymbols"_n, blacklisted_symbols
>;

struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] blacklisted_tokens {
  eosio::name   contract;

  uint64_t primary_key() const { return contract.value; }
};
using black_table = eosio::multi_index<"blacklist"_n, blacklisted_tokens
>;


struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] config {
	uint64_t  			          ID;
  uint64_t                  total_nfts_backed;    
  std::vector<eosio::name>  authorizers;
  uint8_t                   global_threshold;

	uint64_t primary_key() const { return ID; }
};
using config_table = eosio::multi_index<"config"_n, config
>;

struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] whitelisted_tokens {
  uint64_t        ID;
  eosio::symbol   token_symbol;
  eosio::name     contract;

  uint64_t primary_key() const { return ID; }
  uint128_t secondary_key() const{ return mix64to128(token_symbol.code().raw(), contract.value); }
};
using white_table = eosio::multi_index<"whitelist"_n, whitelisted_tokens,
eosio::indexed_by<"tokencombo"_n, eosio::const_mem_fun<whitelisted_tokens, uint128_t, &whitelisted_tokens::secondary_key>>
>;