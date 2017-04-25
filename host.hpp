#ifndef __HOST_H__
#define __HOST_H__

#include <unordered_map>
#include <tuple>
#include <tins/tins.h>

#include "pointer_iterator.hpp"

class Host {
public:
  typedef std::unordered_map<Tins::IPv4Address, std::tuple<size_t, size_t>> PeerMap;

  Host(const Tins::IPv4Address ip) : ip(ip) {}

  bool operator< (const Host &rhs) {
    return ip < rhs.ip;
  }

  void add_in_flow(const Tins::IPv4Address peer_ip, size_t len) {
    auto it = peers.find(peer_ip);
    if (it == peers.end())
      peers.insert({peer_ip, std::make_tuple(0,0)});

    std::get<0>(peers[peer_ip]) += len;
  }

  void add_out_flow(const Tins::IPv4Address peer_ip, size_t len) {
    auto it = peers.find(peer_ip);
    if (it == peers.end())
      peers.insert({peer_ip, std::make_tuple(0,0)});

    std::get<1>(peers[peer_ip]) += len;
  }

  const Tins::IPv4Address& ip_addr() const { return ip; }

  const PeerMap& get_peers() const { return peers; }

 private:
  const Tins::IPv4Address ip;
  PeerMap peers;
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
