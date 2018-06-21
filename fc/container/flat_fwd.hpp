#pragma once 

#define ACHAINWALLETLIB

#ifndef ACHAINWALLETLIB

#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>

namespace fc {

   using boost::container::flat_map;
   using boost::container::flat_set;

   namespace raw {
       template<typename Stream, typename T>
       void pack( Stream& s, const flat_set<T>& value );
       template<typename Stream, typename T>
       void unpack( Stream& s, flat_set<T>& value );
       template<typename Stream, typename K, typename V>
       void pack( Stream& s, const flat_map<K,V>& value );
       template<typename Stream, typename K, typename V>
       void unpack( Stream& s, flat_map<K,V>& value ) ;
   } // namespace raw

} // fc
#else
#include<map>
#include<set>
namespace fc {
#define flat_set std::set
#define flat_map std::map
   namespace raw {
       template<typename Stream, typename T>
       void pack( Stream& s, const flat_set<T>& value );
       template<typename Stream, typename T>
       void unpack( Stream& s, flat_set<T>& value );
       template<typename Stream, typename K, typename V>
       void pack( Stream& s, const flat_map<K,V>& value );
       template<typename Stream, typename K, typename V>
       void unpack( Stream& s, flat_map<K,V>& value ) ;
   } // namespace raw

} // fc

#endif
