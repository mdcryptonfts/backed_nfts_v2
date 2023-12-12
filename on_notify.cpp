#pragma once

void backednfts::listen_for_burn(name asset_owner, uint64_t asset_id, name collection_name,
  			name schema_name, int32_t template_id, vector<asset> backed_tokens, atomicdata::ATTRIBUTE_MAP old_immutable_data,
  			atomicdata::ATTRIBUTE_MAP old_mutable_data, name asset_ram_payer) {

	const name tkcontract = get_first_receiver();

	if(tkcontract != ATOMICASSETS_CONTRACT){ return; }


}