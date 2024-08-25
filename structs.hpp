#pragma once

struct ASSET_UPDATE {
    eosio::name     claimer;
    uint64_t        asset_id;
};

struct AUTH_OBJECT {
    eosio::name     authorizer_name;
    eosio::name     claimer;
};

/**
 * Redefinition of eosio::extended_asset
 * 
 * I would remove this and use extended_asset in its place
 * if the contract was not already deployed, however at 
 * this point it would risk breaking existing logic,
 * so it's being left as-is.
 */

struct FUNGIBLE_TOKEN {
    eosio::asset    quantity;
    eosio::name     token_contract;
};