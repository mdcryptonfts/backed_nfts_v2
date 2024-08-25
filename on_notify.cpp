#pragma once

void backednfts::listen_for_burn(name asset_owner, uint64_t asset_id, name collection_name,
            name schema_name, int32_t template_id, vector<asset> backed_tokens, atomicdata::ATTRIBUTE_MAP old_immutable_data,
            atomicdata::ATTRIBUTE_MAP old_mutable_data, name asset_ram_payer) 
{

    const name tkcontract = get_first_receiver();

    if(tkcontract != ATOMICASSETS_CONTRACT) return;

    auto it = nfts_t.find(asset_id);

    if(it == nfts_t.end()) return;

    nfts_t.modify(it, same_payer, [&](auto &_nft){
        _nft.is_claimable = 1;
        _nft.claimer = asset_owner;
    });
}

void backednfts::handle_nft_transfer(name _owner, name receiver, std::vector<uint64_t> &ids, std::string memo){
    //This is needed to so the catchall *::transfer doesn't cause errors when an NFT is transferred
    return;
}

void backednfts::receive_token_transfer(name from, name to, eosio::asset quantity, std::string memo){
    const name tkcontract = get_first_receiver();

    check(quantity.amount > 0, "Invalid quantity.");
    check(quantity.amount < MAX_ASSET_AMOUNT, "quantity too large");

    if(from == get_self() || to != get_self() || memo == "ignore"){
        return;
    }

    if(memo != "deposit"){
        check(false, "invalid memo");
    }

    auto it = balances_t.require_find(from.value, "must use announcedepo action first");

    std::vector<FUNGIBLE_TOKEN> existing = it->balances;

    bool alreadyHasBalance = false;

    for(FUNGIBLE_TOKEN& e : existing){
        if(e.quantity.symbol == quantity.symbol
            &&
            e.token_contract == tkcontract
        ){
            e.quantity.amount = safeAddInt64(e.quantity.amount, quantity.amount);       

            alreadyHasBalance = true;
            break;
        }
    }

    if(!alreadyHasBalance){
        check(false, "need to announcedepo for this token first");
    }

    balances_t.modify(it, same_payer, [&](auto &_b){
        _b.balances = existing;
    });
}