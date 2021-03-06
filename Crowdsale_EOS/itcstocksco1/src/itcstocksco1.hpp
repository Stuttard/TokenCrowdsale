#pragma once
#include <eosio/eosio.hpp>
#include <eosio/name.hpp>
#include <eosio/singleton.hpp>
#include <eosio/time.hpp>
#include <eosio/system.hpp>

#include <eosio/datastream.hpp>
#include <eosio/asset.hpp>

const eosio::symbol sy_sys = eosio::symbol("WAX", 8);
const eosio::symbol sy_itc = eosio::symbol("ITC", 8);
const eosio::asset zero_itc = eosio::asset( 0, eosio::symbol("ITC", 8 )); 


using namespace eosio;

CONTRACT itcstocksco1 : public eosio::contract
{
  public:
    // constructor
    itcstocksco1(name self, name code, datastream<const char *> ds);

    // destructor
    ~itcstocksco1();

    ACTION init(eosio::name recipient, eosio::time_point_sec start, eosio::time_point_sec finish); // initialize the crowdsale
    
    ACTION issue(eosio::name to, eosio::asset quantity, uint64_t _class, std::string memo);

    // for trade
    ACTION transfer(eosio::name from, eosio::name to, eosio::asset quantity, std::string memo); // redirect to handle_investment

    // to itcstocksco1
    ACTION buyitc(eosio::name from, eosio::name to, eosio::asset quantity, std::string memo); // redirect to handle_investment

    ACTION withdraw(); // transfer tokens from the contract account to the recipient

    ACTION pause(); // for pause/ unpause contract

    ACTION rate(); // for the rate of 1 QUI 
    
    ACTION checkgoal(); // to check if goal is achieved or not
    

  private:
    // type for defining state
    struct state_t
    {
        eosio::name recipient;
        eosio::asset total_eoses;
        eosio::asset total_tokens;
        eosio::time_point_sec start;
        eosio::time_point_sec finish;
        bool pause;

        // utility method for converting this object to string
        std::string toString()
        {
            std::string str = " RECIPIENT " + this->recipient.to_string() +
                              " PAUSED " + std::to_string(this->pause) +
                              " EOSES " + std::to_string(this->total_eoses.amount) +
                              " TOKENS " + std::to_string(this->total_tokens.amount) +
                              " START " + std::to_string(this->start.utc_seconds) +
                              " FINISH " + std::to_string(this->finish.utc_seconds);

            return str;
        }
    };

    // type for issuing the reserved tokens
    struct reserved_t
    {
        eosio::asset class1;
        eosio::asset class2;
        eosio::asset class3;
        eosio::asset class4;
        eosio::asset class5;
        eosio::asset class6;

        // utility method for converting this object to string
        std::string toString()
        {
            std::string str = " Investors/Core-Team "  + std::to_string(this->class1.amount)  +
                              " Advisors " +  std::to_string(this->class2.amount)  +
                              " Zeptagram Reserve " + std::to_string(this->class3.amount) +
                              " AirDrop " +  std::to_string(this->class4.amount)  +
                              " Exchanges " + std::to_string(this->class5.amount) +
                              " Development/Promotional " + std::to_string(this->class6.amount);

            return str;
        }
    };

    // table for holding investors information
    TABLE deposit_t
    {
        eosio::name account;
     //   eosio::asset eoses;
        eosio::asset tokens;
        uint64_t primary_key() const { return account.value; }
    };

    // persists the state of the aplication in a singleton. Only one instance will be strored in the RAM for this application
    eosio::singleton<"state"_n, state_t> state_singleton;

    // persists the state of reserved tokens 
    eosio::singleton<"reserved"_n, reserved_t> reserved_singleton;

    // store investors and balances with contributions in the RAM
    eosio::multi_index<"deposit"_n, deposit_t> deposits;

    // hold present state of the application
    state_t state;

    // holds reserved tokens state for all classes
    reserved_t reserved;

    // handle investments on token transfers
    void handle_investment(eosio::name investor, uint64_t tokens_to_give);

    // private function to call issue action from inside the contract
    void inline_issue(eosio::name to, eosio::asset quantity, std::string memo) const
    {
        // define the type for storing issue information
        struct issue
        {
            eosio::name to;
            eosio::asset quantity;
            std::string memo;
        };

        // create an instance of the action sender and call send function on it
        eosio::action issue_action = eosio::action(
            eosio::permission_level(this->_self, "active"_n),
            eosio::name("eosio.token"), // name of the account contract deployed
            eosio::name("issue"),
            issue{to, quantity, memo});
            issue_action.send();
    }

    // private function to handle token transfers
    void inline_transfer(eosio::name from, eosio::name to, eosio::asset quantity, std::string memo) const
    {
        struct transfer
        {
            eosio::name from;
            eosio::name to;
            eosio::asset quantity;
            std::string memo;
        };

        eosio::action transfer_action = eosio::action(
            eosio::permission_level(_self, eosio::name("active")),
            eosio::name("eosio.token"), // name of the contract
            eosio::name("transfer"),
            transfer{from, to, quantity, memo});
            transfer_action.send();
    }

    // a utility function to return default parameters for the state of the crowdsale
    state_t default_parameters() const
    {
        state_t ret;
        ret.total_eoses.symbol = sy_sys;
        ret.total_eoses.amount = 0;
        ret.total_tokens.symbol = sy_itc;        
        ret.total_tokens.amount = 0;
        ret.pause = false;
        ret.start = eosio::time_point_sec(0);
        ret.finish = eosio::time_point_sec(0);
        return ret;
    }

    // a utility function to return default parameters for the state of the crowdsale
    reserved_t default_parameters1() const
    {
        reserved_t res;
        res.class1 = zero_itc;
        res.class2 = zero_itc;
        res.class3 = zero_itc;
        res.class4 = zero_itc;
        res.class5 = zero_itc;
        res.class6 = zero_itc;
        return res;
    }
};