#pragma once

//Numeric Limits
const int64_t MAX_ASSET_AMOUNT = 4611686018427387903;
const uint64_t MAX_ASSET_AMOUNT_64 = 4611686018427387903;

//Contract info
const eosio::name ATOMICASSETS_CONTRACT = "atomicassets"_n;

//Default objects
const AUTH_OBJECT DEFAULT_AUTH_OBJECT = {"placeholder"_n, "placeholder"_n};

//Errors
static const char* ERR_CONFIG_NOT_FOUND = "could not locate config";
static const char* TOKEN_TRANSFER_MEMO(const uint64_t& asset_id){
	return ("your backed tokens from asset " + to_string(asset_id)).c_str();
}