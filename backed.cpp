#pragma once

#include "backed.hpp"
#include "functions.cpp"
#include "on_notify.cpp"


ACTION backednfts::initconfig(){
	require_auth(get_self());

	auto it = config_t.find(0);

	if(it != config_t.end()){
		check(false, "config has already been initialized");
	}

	config_t.emplace(get_self(), [&](auto &_config){
		_config.ID = 0;
	});
}

