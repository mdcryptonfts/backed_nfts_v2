#pragma once

int64_t backednfts::safeAddInt64(const int64_t& a, const int64_t& b){
    const int64_t combinedValue = a + b;

    if(MAX_ASSET_AMOUNT - a < b){
        /** if the remainder is less than what we're adding, it means there 
         *  will be overflow
         */

        check(false, "overflow error");
    }   

    return combinedValue;
}