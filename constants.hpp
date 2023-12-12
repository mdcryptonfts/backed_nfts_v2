#pragma once

//Numeric Limits
const int64_t MAX_ASSET_AMOUNT = 4611686018427387903;

//Contract info
const eosio::name ATOMICASSETS_CONTRACT = "atomicassets"_n;

//Errors
static const char* ERR_CONFIG_NOT_FOUND = "could not locate config";