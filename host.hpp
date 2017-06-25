#ifndef __HOST_H__
#define __HOST_H__

#include <unordered_map>
#include <tuple>
#include <tins/tins.h>

#include "flow.hpp"

struct Host {
  Host(const Tins::ARP::hwaddress_type hw, const Tins::IPv4Address ip)
	: hw(hw), ip(ip) {
  }

  bool operator == (const Host& other) const {
	return hw == other.hw
	  && ip == other.ip;
  }

  const Tins::ARP::hwaddress_type hw;
  const Tins::IPv4Address ip;
};

namespace std {
  template <>
  struct hash<Host>
  {
	size_t operator()(const Host& host) const {
	  size_t seed = 0;
	  boost::hash_combine<std::string>(seed, host.hw.to_string());
	  boost::hash_combine<uint32_t>(seed, host.ip);

	  return seed;
	}
  };
}

class HostFlows {
public:
  HostFlows(const Host& host) : _host(host) {}

  void add_flow(const FlowSummary* summary) {
	_flows.push_back(summary);
  }

  const Host& host() const { return _host; }
  const std::vector<const FlowSummary*>& flows() const { return _flows; }

private:
  const Host _host;
  std::vector<const FlowSummary*> _flows;
};

#endif
