#pragma once

void backednfts::check_token_exists(const symbol& token_symbol, const name& contract){
	const uint64_t raw_token_symbol = token_symbol.code().raw();
	const uint128_t symbol_contract_combo = mix64to128(raw_token_symbol, contract.value);
	stat_table stat(contract, token_symbol.code().raw());
	auto stat_itr = stat.find(token_symbol.code().raw());
	check(stat_itr != stat.end(), "That token does not exist on that contract");
	check(stat_itr->supply.symbol == token_symbol, "Symbol mismatch. You probably put the wrong amount of decimals in the precision field");
	return;
}

void backednfts::transfer_tokens(const name& user, const asset& amount_to_send, const name& contract, const std::string& memo){
    action(permission_level{get_self(), "active"_n}, contract,"transfer"_n,std::tuple{ get_self(), user, amount_to_send, memo}).send();
    return;
}
