#ifndef __FLOW_H__
#define __FLOW_H__

#include <unordered_map>
#include <boost/functional/hash.hpp>

#include <tins/tins.h>

#include "pointer_iterator.hpp"

class Flow {
public:
  Flow(const Tins::IPv4Address src, const Tins::IPv4Address dst)
    : src(src), dst(dst), num_packets(0) {}

  int operator += (int incr) { return num_packets += incr; }

  const Tins::IPv4Address& src_addr() const { return src; }
  const Tins::IPv4Address& dst_addr() const { return dst; }

  int packets_seen() const { return num_packets; }
  
private:
  const Tins::IPv4Address src;
  const Tins::IPv4Address dst;
  int num_packets;
};

typedef std::tuple<Tins::IPv4Address, Tins::IPv4Address> ip_pair;

namespace std {
  template <>
  struct hash<ip_pair>
  {
    size_t operator()(const tuple<Tins::IPv4Address, Tins::IPv4Address>& ips) const {
      size_t seed = 0;
      boost::hash_combine<uint32_t>(seed, get<0>(ips));
      boost::hash_combine<uint32_t>(seed, get<1>(ips));

      return seed;
    }
  };
}

class FlowMap {
public:
  typedef std::unordered_map<ip_pair, Flow*> IpsFlowMap;
  typedef PointerIteratorWrapper<IpsFlowMap::const_iterator, const Flow> Iterator;
  
  Flow& operator [] (const ip_pair &key) {
    auto item = flows_by_ips.find(key);
    if (item == flows_by_ips.end())
      flows_by_ips.insert({key, new Flow(std::get<0>(key), std::get<1>(key))});
    return *(flows_by_ips[key]);
  }

  ~FlowMap() {
    for (auto&& kv : flows_by_ips) {
      delete kv.second;
    }
  }

  Iterator begin() const { return Iterator(flows_by_ips.begin()); }
  Iterator end() const { return Iterator(flows_by_ips.end()); }

private:
  IpsFlowMap flows_by_ips;
};

#endif
