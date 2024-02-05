#pragma once

struct ASSET_UPDATE {
	eosio::name  	claimer;
	uint64_t  		asset_id;
};

struct AUTH_OBJECT {
	eosio::name 	authorizer_name;
	eosio::name  	claimer;
};

struct FUNGIBLE_TOKEN {
	eosio::asset  	quantity;
	eosio::name  	token_contract;
};