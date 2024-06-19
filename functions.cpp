#pragma once

void backednfts::check_for_duplicates(const std::vector<FUNGIBLE_TOKEN>& tokens_to_back){
    std::set<std::pair<eosio::symbol, eosio::name>> unique_tokens;

    for (const auto& t : tokens_to_back) {
        auto pair = std::make_pair(t.quantity.symbol, t.token_contract);

        if(unique_tokens.find(pair) != unique_tokens.end()) {
            eosio::check(false, "Duplicate reward found");
        } else {
            unique_tokens.insert(pair);
        }
    }
}

void backednfts::check_token_exists(const symbol& token_symbol, const name& contract){
	stat_table stat_t = stat_table( contract, token_symbol.code().raw() );
	auto itr = stat_t.require_find( token_symbol.code().raw(), "symbol does not exist on that contract" );
	check( itr->max_supply.symbol.precision() == token_symbol.precision(), "precision mismatch" );
}

bool backednfts::contract_is_blacklisted(const name& contract){
	auto it = black_t.find(contract.value);

	if(it != black_t.end()){
		return true;
	}

	return false;
}

std::vector<std::string> backednfts::get_words(std::string memo){
	std::string delim = "|";
	std::vector<std::string> words{};
	size_t pos = 0;
	while ((pos = memo.find(delim)) != string::npos) {
		words.push_back(memo.substr(0, pos));
		memo.erase(0, pos + delim.length());
	}
	return words;
}

bool backednfts::has_balance_object(const eosio::name& user, const eosio::name& contract, const eosio::symbol& token_symbol){
    accounts account_t = accounts(contract, user.value);
    const uint64_t raw_token_symbol = token_symbol.code().raw();
    auto it = account_t.find(raw_token_symbol);
    if(it == account_t.end()) return false;
    return true;
}

bool backednfts::is_an_authorizer(const eosio::name& wallet, const std::vector<eosio::name>& authorizers){
	if(std::find(authorizers.begin(), authorizers.end(), wallet) != authorizers.end()) return true;	
	return false;
}

bool backednfts::is_beta_tester(const eosio::name& user){
	auto it = beta_t.find(user.value);
	if(it == beta_t.end()){
		return false;
	}
	return true;
}

bool backednfts::is_ignored(const eosio::name& contract, const std::vector<eosio::name>& ignore_list){
	if(std::find(ignore_list.begin(), ignore_list.end(), contract) != ignore_list.end()) return true;	
	return false;
}

void backednfts::log_back_asset(const eosio::name& backer, const eosio::name& asset_owner, 
	const uint64_t& asset_id, const std::vector<FUNGIBLE_TOKEN>& tokens_to_back, 
	const eosio::name& collection_name, const eosio::name& schema_name,
	const int32_t& template_id)
{
    action(permission_level{get_self(), "active"_n}, get_self(),"logbackasset"_n,std::tuple{ backer, asset_owner, asset_id, 
    	tokens_to_back, collection_name, schema_name, template_id }).send();
    return;
}

void backednfts::log_remaining(const uint64_t& asset_id, const std::vector<FUNGIBLE_TOKEN>& backed_tokens){
    action(permission_level{get_self(), "active"_n}, get_self(),"logremaining"_n,std::tuple{ asset_id, backed_tokens }).send();
    return;
}

std::vector<FUNGIBLE_TOKEN> backednfts::remove_zero_balances(const std::vector<FUNGIBLE_TOKEN>& balances){
    std::vector<FUNGIBLE_TOKEN> cleanedBalances = balances;
    
    cleanedBalances.erase(
        std::remove_if(
            cleanedBalances.begin(), 
            cleanedBalances.end(),
            [](const FUNGIBLE_TOKEN& token) {
                return token.quantity.amount == 0;
            }
        ),
        cleanedBalances.end()
    );
    
    return cleanedBalances;
}

bool backednfts::symbol_is_blacklisted(const eosio::symbol_code& symbol){
	auto it = blacklisted_symbols_t.find(symbol.raw());
	if(it == blacklisted_symbols_t.end()){
		return false;
	}
	return true;
}

bool backednfts::token_is_whitelisted(const symbol& token_symbol, const name& contract){
	const uint64_t raw_token_symbol = token_symbol.code().raw();
	const uint128_t symbol_contract_combo = mix64to128(raw_token_symbol, contract.value);
	auto tokens_secondary = white_t.get_index<"tokencombo"_n>();

	auto it = tokens_secondary.find(symbol_contract_combo);
	if(it != tokens_secondary.end()) return true;

	return false;
}

void backednfts::transfer_tokens(const name& user, const asset& amount_to_send, const name& contract, const std::string& memo){
    action(permission_level{get_self(), "active"_n}, contract,"transfer"_n,std::tuple{ get_self(), user, amount_to_send, memo}).send();
    return;
}

void backednfts::upsert_claimable_tokens(const name& user, vector<FUNGIBLE_TOKEN>& tokens){
	auto itr = balances_t.find( user.value );

	if( itr == balances_t.end() ){

		balances_t.emplace(_self, [&](auto &row){
			row.wallet = user;
			row.balances = tokens;
		});
		
	} else {

		vector<FUNGIBLE_TOKEN> existing_tokens = itr->balances;

		for(FUNGIBLE_TOKEN& t : tokens){

			auto bal_itr = std::find_if( existing_tokens.begin(), existing_tokens.end(), 
			[&](const auto& b) { return b.quantity.symbol == t.quantity.symbol && b.token_contract == t.token_contract; });
		
			if( bal_itr == existing_tokens.end() ){
				existing_tokens.push_back( t );
			} else {
				bal_itr->quantity += t.quantity;
			}

		}	

		balances_t.modify(itr, _self, [&](auto &row){
			row.balances = existing_tokens;
		});
	}
}