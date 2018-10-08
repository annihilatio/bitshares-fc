#pragma once
#include <fc/variant.hpp>
#include <fc/filesystem.hpp>

namespace fc
{
class ostream;

namespace config_ini_output
{

ostream& to_stream( ostream& out, const variant& v);
ostream& to_stream( ostream& out, const variants& v);

}
} // fc
