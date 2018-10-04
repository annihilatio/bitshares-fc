#include <fc/exception/exception.hpp>
#include <fc/io/iostream.hpp>
#include <fc/io/fstream.hpp>
#include <fc/io/json.hpp>
#include <fc/io/sstream.hpp>
#include <fc/log/logger.hpp>
#include <iostream>
#include <ostream>
#include <fstream>
#include <sstream>

#include <boost/filesystem/fstream.hpp>

#include <fc/io/config_file_output.hpp>

namespace fc {

template<typename T> T& config_ini_to_stream( T& os, const variant& o );

ostream& config_ini_output::to_stream(ostream &out, const variant &v)
{
  FC_ASSERT(v.is_array(), "Invalid type of data for config file appending");
  const auto& va = v.get_array();
  out << "# private keys from create_examlpe_genesis function #\n";
  out << "# ================================================= #\n";
  config_ini_output::to_stream(out, va);
  return out;
}

ostream& fc::config_ini_output::to_stream(ostream &out, const variants &va) {
  std::for_each(va.begin(), va.end(), [&out](const auto &v) {
    FC_ASSERT(v.is_object(), "Invalid type of key record for config file appending");
    config_ini_to_stream(out, v);
  });
  out << "# ================================================= #\n\n";
  return out;
}

template<typename T>
T& config_ini_to_stream( T& os, const variant& o )
{
  auto record = o.get_object();
  FC_ASSERT(record.size() == 2, "Key record object must have only 2 keys");
  os << "# keyname = " << record["key_name"].as_string() << "\n";
  os << "private-key = ";
  json::to_stream(os, record["key_pair"], json::stringify_large_ints_and_doubles);
  os << "\n";
  return os;
}

}//fc