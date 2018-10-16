#pragma once
#include <fc/utility.hpp>
#include <string.h>
#include <stdint.h>
#include <vector>

namespace fc {

namespace detail 
{
  NO_RETURN void throw_datastream_range_error( const char* file, size_t len, int64_t over );
}

/**
 *  The purpose of this datastream is to provide a fast, effecient, means
 *  of calculating the amount of data "about to be written" and then
 *  writing it.  This means having two modes of operation, "test run" where
 *  you call the entire pack sequence calculating the size, and then 
 *  actually packing it after doing a single allocation.
 */
template<typename T>
class datastream {
   public:
      datastream( T start, size_t s )
      :_start(start),_pos(start),_end(start+s){};
      
      
      inline void skip( size_t s ){ _pos += s; }
      inline bool read( char* d, size_t s ) {
        if( size_t(_end - _pos) >= (size_t)s ) {
          memcpy( d, _pos, s );
          _pos += s;
          return true;
        }
        detail::throw_datastream_range_error( "read", _end-_start, int64_t(-((_end-_pos) - 1)));
      }
      
      inline bool write( const char* d, size_t s ) {
        if( _end - _pos >= (int32_t)s ) {
          memcpy( _pos, d, s );
          _pos += s;
          return true;
        }
        detail::throw_datastream_range_error( "write", _end-_start, int64_t(-((_end-_pos) - 1)));
      }
      
      inline bool   put(char c) { 
        if( _pos < _end ) {
          *_pos = c; 
          ++_pos; 
          return true;
        }
        detail::throw_datastream_range_error( "put", _end-_start, int64_t(-((_end-_pos) - 1)));
      }
      
      inline bool   get( unsigned char& c ) { return get( *(char*)&c ); }
      inline bool   get( char& c ) 
      {
        if( _pos < _end ) {
          c = *_pos;
          ++_pos; 
          return true;
        }
        detail::throw_datastream_range_error( "get", _end-_start, int64_t(-((_end-_pos) - 1)));
      }
      
      T               pos()const        { return _pos;                               }
      inline bool     valid()const      { return _pos <= _end && _pos >= _start;  }
      inline bool     seekp(size_t p) { _pos = _start + p; return _pos <= _end; }
      inline size_t   tellp()const      { return _pos - _start;                     }
      inline size_t   remaining()const  { return _end - _pos;                       }
    private:
      T _start;
      T _pos;
      T _end;
};

template<>
template <typename T>
class datastream<std::vector<T>> {
public:
    datastream(std::vector<T> & vec, size_t t): _vec(vec)
    {
      static_assert(
              std::is_same<T, char>::value ||
              std::is_same<T, unsigned char>::value ||
              std::is_same<T, signed char>::value,
              "invalid type of vector datastream");
      _vec.reserve(t);
    };

    inline bool write( const char * c, size_t t)
    {
      if (_vec.capacity() >= _vec.size()+t)
      {
        auto distance = std::distance(_vec.begin(), _vec.end());
        _vec.resize(_vec.size() + t);
        memcpy(&(*(_vec.begin() + distance)), c, t);

        return true;
      }
      detail::throw_datastream_range_error( "write", _vec.capacity(), int64_t(t-(_vec.capacity()-_vec.size()) ));
    }

    inline size_t  tellp()const  { return _vec.size();}
    inline bool    valid()const  { return true;}
    inline size_t  remaining()const  { return _vec.capacity()-_vec.size();                       }

private:
    std::vector<T>& _vec;
};

template<>
class datastream<size_t> {
   public:
     datastream( size_t init_size = 0):_size(init_size){};
     inline bool     skip( size_t s )                 { _size += s; return true;  }
     inline bool     write( const char* ,size_t s )  { _size += s; return true;  }
     inline bool     put(char )                      { ++_size; return  true;    }
     inline bool     valid()const                     { return true;              }
     inline bool     seekp(size_t p)                  { _size = p;  return true;  }
     inline size_t   tellp()const                     { return _size;             }
     inline size_t   remaining()const                 { return 0;                 }
  private:
     size_t _size;
};

template<typename ST>
inline datastream<ST>& operator<<(datastream<ST>& ds, const int32_t& d) {
  ds.write( (const char*)&d, sizeof(d) );
  return ds;
}
template<typename ST, typename DATA>
inline datastream<ST>& operator>>(datastream<ST>& ds, int32_t& d) {
  ds.read((char*)&d, sizeof(d) );
  return ds;
}
template<typename ST>
inline datastream<ST>& operator<<(datastream<ST>& ds, const uint32_t& d) {
  ds.write( (const char*)&d, sizeof(d) );
  return ds;
}

template<typename ST, typename DATA>
inline datastream<ST>& operator>>(datastream<ST>& ds, uint32_t& d) {
  ds.read((char*)&d, sizeof(d) );
  return ds;
}
template<typename ST>
inline datastream<ST>& operator<<(datastream<ST>& ds, const int64_t& d) {
  ds.write( (const char*)&d, sizeof(d) );
  return ds;
}

template<typename ST, typename DATA>
inline datastream<ST>& operator>>(datastream<ST>& ds, int64_t& d) {
  ds.read((char*)&d, sizeof(d) );
  return ds;
}
template<typename ST>
inline datastream<ST>& operator<<(datastream<ST>& ds, const uint64_t& d) {
  ds.write( (const char*)&d, sizeof(d) );
  return ds;
}

template<typename ST, typename DATA>
inline datastream<ST>& operator>>(datastream<ST>& ds, uint64_t& d) {
  ds.read((char*)&d, sizeof(d) );
  return ds;
}
template<typename ST>
inline datastream<ST>& operator<<(datastream<ST>& ds, const int16_t& d) {
  ds.write( (const char*)&d, sizeof(d) );
  return ds;
}

template<typename ST, typename DATA>
inline datastream<ST>& operator>>(datastream<ST>& ds, int16_t& d) {
  ds.read((char*)&d, sizeof(d) );
  return ds;
}
template<typename ST>
inline datastream<ST>& operator<<(datastream<ST>& ds, const uint16_t& d) {
  ds.write( (const char*)&d, sizeof(d) );
  return ds;
}

template<typename ST, typename DATA>
inline datastream<ST>& operator>>(datastream<ST>& ds, uint16_t& d) {
  ds.read((char*)&d, sizeof(d) );
  return ds;
}
template<typename ST>
inline datastream<ST>& operator<<(datastream<ST>& ds, const int8_t& d) {
  ds.write( (const char*)&d, sizeof(d) );
  return ds;
}

template<typename ST, typename DATA>
inline datastream<ST>& operator>>(datastream<ST>& ds, int8_t& d) {
  ds.read((char*)&d, sizeof(d) );
  return ds;
}
template<typename ST>
inline datastream<ST>& operator<<(datastream<ST>& ds, const uint8_t& d) {
  ds.write( (const char*)&d, sizeof(d) );
  return ds;
}

template<typename ST, typename DATA>
inline datastream<ST>& operator>>(datastream<ST>& ds, uint8_t& d) {
  ds.read((char*)&d, sizeof(d) );
  return ds;
}


} // namespace fc

