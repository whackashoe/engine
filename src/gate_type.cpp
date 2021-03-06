#include "gate_type.hpp"

std::string to_string(const gate_type m)
{
    switch(m) {
        case gate_type::off:  return "off";  break;
        case gate_type::on:   return "on";   break;
        case gate_type::blue: return "blue"; break;
        case gate_type::red:  return "red"; break;
        default:
            std::cerr << "error: gate_type not enumerated in to_string" << std::endl;
            return "";
    }
}

gate_type gate_type_from_string(const std::string & m)
{
    if(m == "off")  { return gate_type::off; }
    if(m == "on")   { return gate_type::on; }
    if(m == "blue") { return gate_type::blue; }
    if(m == "red")  { return gate_type::red; }

    std::cerr << "error: gate_type " << m << " not enumerated in from_string" << std::endl;
    return gate_type::off; // for warning suppression mainly
}
