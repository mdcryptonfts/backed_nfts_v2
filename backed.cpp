#include "backed.hpp"
#include "functions.cpp"
#include "on_notify.cpp"
#include "safe.cpp"

ACTION backednfts::addnewsigner(const eosio::name& signer_name){
	require_auth(get_self());

	if(!is_account(signer_name)){
		check(false, "signer is not a valid account");
	}

	auto it = config_t.require_find(0, ERR_CONFIG_NOT_FOUND);

	std::vector<eosio::name> existing = it->authorizers;

	if(std::find(existing.begin(), existing.end(), signer_name) != existing.end()){
		check(false, "signer already exists");
	}	

	existing.push_back(signer_name);

	config_t.modify(it, get_self(), [&](auto &_config){
		_config.authorizers = existing;
	});
}

ACTION backednfts::addblacklist(const std::vector<eosio::name>& contracts_to_blacklist){
	require_auth(get_self());

	for(name c : contracts_to_blacklist){
		auto it = black_t.find(c.value);

		if(it == black_t.end()){
			black_t.emplace(get_self(), [&](auto &_row){
				_row.contract = c;
			});
		}
	}
}

ACTION backednfts::addwhitelist(const eosio::symbol& token_symbol, const eosio::name& contract){
	require_auth(get_self());

	const uint64_t raw_token_symbol = token_symbol.code().raw();
	const uint128_t symbol_contract_combo = mix64to128(raw_token_symbol, contract.value);
	auto tokens_secondary = white_t.get_index<"tokencombo"_n>();

	auto it = tokens_secondary.find(symbol_contract_combo);

	if(it == tokens_secondary.end()){
		white_t.emplace(get_self(), [&](auto &_row){
			_row.ID = white_t.available_primary_key();
			_row.token_symbol = token_symbol;
			_row.contract = contract;
		});
	}

	else{
		check(false, "token is already whitelisted");
	}
}

ACTION backednfts::announcedepo(const eosio::name& user, const std::vector<FUNGIBLE_TOKEN>& tokens)
{
	require_auth(user);
	check_for_duplicates(tokens);

	for(FUNGIBLE_TOKEN t : tokens){
		if(contract_is_blacklisted(t.token_contract)){
			check(false, (t.token_contract.to_string() + " is a blacklisted contract").c_str());
		}
		check_token_exists(t.quantity.symbol, t.token_contract);
	    check(t.quantity.amount > 0, "Invalid quantity.");
	    check(t.quantity.amount < MAX_ASSET_AMOUNT, "quantity too large");		
	}

	auto it = balances_t.find(user.value);

	if(it == balances_t.end()){

		std::vector<FUNGIBLE_TOKEN> zeroed_tokens = tokens;

		for(FUNGIBLE_TOKEN& z : zeroed_tokens){
			z.quantity.amount = 0;
		}

		balances_t.emplace(user, [&](auto &_b){
			_b.wallet = user;
			_b.balances = zeroed_tokens;
		});
	}

	else {
		std::vector<FUNGIBLE_TOKEN> existing = it->balances;

		for(FUNGIBLE_TOKEN t : tokens){
			bool alreadyHasBalance = false;

			for(FUNGIBLE_TOKEN& e : existing){
				if(e.quantity.symbol == t.quantity.symbol
					&&
					e.token_contract == t.token_contract
				){
					alreadyHasBalance = true;
					break;
				}
			}

			if(!alreadyHasBalance){
				existing.push_back({asset(0, t.quantity.symbol), t.token_contract});
			}
		}

		balances_t.modify(it, user, [&](auto &_b){
			_b.balances = existing;
		});
	}

}


ACTION backednfts::backnft(const eosio::name& user, const eosio::name& asset_owner, const uint64_t& asset_id,
	const std::vector<FUNGIBLE_TOKEN>& tokens_to_back)
{
	require_auth(user);

	auto atomic_it = atomics_t(ATOMICASSETS_CONTRACT, asset_owner.value).require_find(asset_id, ("asset " + to_string(asset_id) + " could not be located").c_str());

    if(atomic_it->template_id != -1){
        auto template_itr = atomic_temps(ATOMICASSETS_CONTRACT, atomic_it->collection_name.value).require_find(atomic_it->template_id, "template could not be found");
        check(template_itr->burnable, ("Asset " + to_string(asset_id) + " is not burnable, so it can't be backed").c_str());
    }	

	check_for_duplicates(tokens_to_back);

	auto balance_it = balances_t.require_find(user.value, "must deposit tokens first");

	std::vector<FUNGIBLE_TOKEN> existing = balance_it->balances;

	for(FUNGIBLE_TOKEN t : tokens_to_back){
		if(symbol_is_blacklisted(t.quantity.symbol.code())
			&&
			!token_is_whitelisted(t.quantity.symbol, t.token_contract)
			)
		{
			check(false, t.quantity.symbol.code().to_string() + " is a blacklisted symbol");
		}

		check(t.quantity.amount > 0, "must back with positive amount");

		bool hasThisToken = false;

		for(FUNGIBLE_TOKEN& e : existing){
			if(e.quantity.symbol == t.quantity.symbol
				&&
				e.token_contract == t.token_contract
			){
				if(e.quantity.amount < t.quantity.amount){
					check(false, "insufficient balance");
				}

				e.quantity.amount -= t.quantity.amount;

				hasThisToken = true;
				break;
			}

		}

		if(!hasThisToken){
			check(false, "no balance found for this token");
		}		
	}

	existing = remove_zero_balances(existing);

	balances_t.modify(balance_it, user, [&](auto &_b){
		_b.balances = existing;
	});

	auto asset_it = nfts_t.find(asset_id);

	if(asset_it == nfts_t.end()){
		nfts_t.emplace(user, [&](auto &_nft){
			_nft.asset_id = asset_id;
			_nft.backed_tokens = tokens_to_back;
			_nft.is_claimable = 0;
			_nft.claimer = get_self();
			_nft.required_auths = std::vector<AUTH_OBJECT>(5, DEFAULT_AUTH_OBJECT);
		});

		auto c_it = config_t.require_find(0, ERR_CONFIG_NOT_FOUND);

		config_t.modify(c_it, get_self(), [&](auto &_config){
			_config.total_nfts_backed += 1;
		});	

		log_back_asset(user, asset_owner, asset_id, tokens_to_back, atomic_it->collection_name, 
			atomic_it->schema_name, atomic_it->template_id);			
	}

	else{
		std::vector<FUNGIBLE_TOKEN> existingBackings = asset_it->backed_tokens;

		for(FUNGIBLE_TOKEN t : tokens_to_back){
			bool alreadyBackedWithThisToken = false;

			for(FUNGIBLE_TOKEN& e : existingBackings){
				if(e.quantity.symbol == t.quantity.symbol
					&&
					e.token_contract == t.token_contract
				){
					
					e.quantity.amount = safeAddInt64(e.quantity.amount, t.quantity.amount);

					alreadyBackedWithThisToken = true;
					break;
				}

			}

			if(!alreadyBackedWithThisToken){
				existingBackings.push_back(t);
			}			
		}		

		nfts_t.modify(asset_it, user, [&](auto &_nft){
			_nft.backed_tokens = existingBackings;
		});

		log_back_asset(user, asset_owner, asset_id, existingBackings, atomic_it->collection_name, 
			atomic_it->schema_name, atomic_it->template_id);		
	}

}


ACTION backednfts::blacklistsym(const std::vector<eosio::symbol_code>& symbols_to_blacklist){
	require_auth(get_self());

	for(symbol_code s : symbols_to_blacklist){
		auto it = blacklisted_symbols_t.find(s.raw());

		if(it == blacklisted_symbols_t.end()){
			blacklisted_symbols_t.emplace(get_self(), [&](auto &_row){
				_row.symbol_code = s;
			});
		}
	}
}


ACTION backednfts::initconfig(){
	require_auth(get_self());

	auto it = config_t.find(0);

	if(it != config_t.end()){
		check(false, "config has already been initialized");
	}

	config_t.emplace(get_self(), [&](auto &_config){
		_config.ID = 0;
		_config.total_nfts_backed = 0;
		_config.authorizers = {"waxdaobacker"_n};
		_config.global_threshold = 3;
	});
}


ACTION backednfts::logbackasset(const eosio::name& backer, const eosio::name& asset_owner, 
	const uint64_t& asset_id, const std::vector<FUNGIBLE_TOKEN>& tokens_to_back, 
	const eosio::name& collection_name, const eosio::name& schema_name,
	const int32_t& template_id)
{
	require_auth(get_self());
}

ACTION backednfts::logremaining(const uint64_t& asset_id, const std::vector<FUNGIBLE_TOKEN>& backed_tokens){
	require_auth(get_self());
}

ACTION backednfts::removesigner(const eosio::name& signer_name){
	require_auth(get_self());

	auto it = config_t.require_find(0, ERR_CONFIG_NOT_FOUND);

	std::vector<eosio::name> existing = it->authorizers;

    existing.erase(
        std::remove_if(
            existing.begin(), 
            existing.end(),
            [&](const eosio::name& signer) {
                return signer == signer_name;
            }
        ),
        existing.end()
    );

	config_t.modify(it, get_self(), [&](auto &_config){
		_config.authorizers = existing;
	});
}

ACTION backednfts::rmvblacklist(const std::vector<eosio::name>& contracts_to_remove){
	require_auth(get_self());

	for(name c : contracts_to_remove){
		auto it = black_t.find(c.value);

		if(it != black_t.end()){
			it = black_t.erase(it);
		}
	}
}

ACTION backednfts::rmvblacksym(const std::vector<eosio::symbol_code>& symbols_to_remove){
	require_auth(get_self());

	for(symbol_code s : symbols_to_remove){
		auto it = blacklisted_symbols_t.find(s.raw());

		if(it != blacklisted_symbols_t.end()){
			it = blacklisted_symbols_t.erase(it);
		}
	}
}

ACTION backednfts::rmvwhitelist(const eosio::symbol& token_symbol, const eosio::name& contract){
	require_auth(get_self());

	const uint64_t raw_token_symbol = token_symbol.code().raw();
	const uint128_t symbol_contract_combo = mix64to128(raw_token_symbol, contract.value);
	auto tokens_secondary = white_t.get_index<"tokencombo"_n>();

	auto it = tokens_secondary.require_find(symbol_contract_combo, "token is not whitelisted");

	it = tokens_secondary.erase(it);
}


ACTION backednfts::setclaimable(const eosio::name& authorizer, const std::vector<ASSET_UPDATE>& assets_to_update){
	require_auth(authorizer);

	auto config = config_t.require_find(0, ERR_CONFIG_NOT_FOUND);

	if(!is_an_authorizer(authorizer, config->authorizers)){
		check(false, "you don't have authorizer permissions");
	}

	for(ASSET_UPDATE nft : assets_to_update){

		if(!is_account(nft.claimer)) continue;

		auto it = nfts_t.find(nft.asset_id);
		if(it == nfts_t.end()) continue;

		//check if this authorizer already chose
		std::vector<AUTH_OBJECT> existing_auths = it->received_auths;

		bool alreadyVoted = false;
		for(AUTH_OBJECT& e : existing_auths){
			if(e.authorizer_name == authorizer){
				e.claimer = nft.claimer;
				alreadyVoted = true;
				break;
			}
		}

		std::vector<AUTH_OBJECT> placeholders = it->required_auths;
		if(!alreadyVoted){
			existing_auths.push_back({authorizer, nft.claimer});
			placeholders.pop_back();
		}

		bool is_claimable = 0;
		const uint8_t threshold = config->global_threshold;
		uint8_t vote_count = 0;

		for(AUTH_OBJECT e : existing_auths){
			if(e.claimer == nft.claimer){
				vote_count ++;
			}
			if(vote_count >= threshold){
				is_claimable = 1;
				vector<FUNGIBLE_TOKEN> backed_tokens = it->backed_tokens;
				upsert_claimable_tokens(nft.claimer, backed_tokens);
				nfts_t.erase(it);
				break;
			} 
		}

		if(is_claimable != 1){
			nfts_t.modify(it, same_payer, [&](auto &_nft){
				_nft.required_auths = placeholders;
				_nft.received_auths = existing_auths; 
			});
		}
	}
}

ACTION backednfts::setthreshold(const uint8_t& new_threshold){
	require_auth(get_self());

	auto it = config_t.require_find(0, ERR_CONFIG_NOT_FOUND);

	config_t.modify(it, get_self(), [&](auto &_config){
		_config.global_threshold = new_threshold;
	});
}

ACTION backednfts::withdraw(const name& user, const vector<name>& contract_ignore_list)
{
	require_auth(user);

	auto itr = balances_t.require_find( user.value, "user not found" );
	vector<FUNGIBLE_TOKEN> existing = itr->balances;

	for( FUNGIBLE_TOKEN& e : existing ){
		if(!contract_is_blacklisted(e.token_contract) 
			&& e.quantity.amount > 0
			&& !is_ignored(e.token_contract, contract_ignore_list)
		){
			if(!has_balance_object(user, e.token_contract, e.quantity.symbol)){
				check(false, ("you need to use the 'open' action on " + e.token_contract.to_string() + " contract for " + e.quantity.symbol.code().to_string() + " token").c_str());
			}
			transfer_tokens(user, e.quantity, e.token_contract, "your withdrawal from WaxDAO NFT Backer");
			e.quantity.amount = 0;
		}
	}

	existing = remove_zero_balances( existing );
	if( existing.size() == 0 ){
		balances_t.erase( itr );
	} else {
		balances_t.modify(itr, same_payer, [&](auto &_bal){
			_bal.balances = existing;
		});
	}
}