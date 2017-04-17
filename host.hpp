#ifndef __HOST_H__
#define __HOST_H__

#include <unordered_map>
#include <tins/tins.h>

#include "pointer_iterator.hpp"

class Host {
public:
  typedef std::unordered_map<Tins::IPv4Address, const Flow*> IpFlowMap;
  typedef PointerIteratorWrapper<IpFlowMap::const_iterator, const Flow> FlowIterator;

  struct FlowRange {
  public:
    FlowRange(const IpFlowMap& map) : map(map) {}
				 
    FlowIterator begin() const { return FlowIterator(map.begin()); }
    FlowIterator end() const { return FlowIterator(map.end()); }

  private:
    const IpFlowMap &map;
  };

  Host(const Tins::IPv4Address ip) : ip(ip) {}

  bool operator< (const Host &rhs) {
    return ip < rhs.ip;
  }

  void add_in_flow(const Flow &flow) {
    auto item = in_flows.find(flow.src_addr());
    if (item == in_flows.end())
      in_flows.insert({flow.src_addr(), &flow});
  }

  void add_out_flow(const Flow &flow) {
    auto item = out_flows.find(flow.dst_addr());
    if (item == out_flows.end())
      out_flows.insert({flow.dst_addr(), &flow});
  }

  const Tins::IPv4Address& ip_addr() const { return ip; }

  const FlowRange get_in_flows() { return FlowRange(in_flows); }
  const FlowRange get_out_flows() { return FlowRange(out_flows); }

 private:
  const Tins::IPv4Address ip;
  IpFlowMap in_flows;
  IpFlowMap out_flows;
};

class HostMap {
public:
  typedef PointerIteratorWrapper<std::unordered_map<Tins::IPv4Address, Host*>::const_iterator, const Host> Iterator;
    
  Host& operator [] (const Tins::IPv4Address& key) {
    auto item = hosts_by_ip.find(key);
    if (item == hosts_by_ip.end())
      hosts_by_ip.insert({key, new Host(key)});
    return *(hosts_by_ip[key]);
  }

  ~HostMap() {
    for (auto&& kv : hosts_by_ip) {
      delete kv.second;
    }
  }

  const Iterator begin() const {
    return Iterator(hosts_by_ip.begin());
  }

  const Iterator end() const {
    return Iterator(hosts_by_ip.end());
  }

private:
  std::unordered_map<Tins::IPv4Address, Host*> hosts_by_ip;
};

#endif
