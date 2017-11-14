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
  HostFlows(const Host& host) : _host(host), _bytes_in(0), _bytes_out(0) {}

  void add_flow(const FlowSummary* summary) {
	_flows.push_back(summary);
  }

  void remove_flow(const Flow& flow) {
	for (auto flow_summary = _flows.begin(); flow_summary != _flows.end(); flow_summary++) {
	  if ((*flow_summary)->flow() == flow) {
		_flows.erase(flow_summary);
		break;
	  }
	}
  }

  void update_host_stats(const FlowSummary& summary) {
	if (summary.flow().src_ip == _host.ip && summary.flow().src_hw == _host.hw)
	  _bytes_in += summary.bytes_to_src();
	else
	  _bytes_out += summary.bytes_to_dst();
  }

  const Host& host() const { return _host; }
  const std::vector<const FlowSummary*>& flows() const { return _flows; }
  size_t bytes_in() const { return _bytes_in; }
  size_t bytes_out() const { return _bytes_out; }

private:
  const Host _host;
  std::vector<const FlowSummary*> _flows;

  size_t _bytes_in;
  size_t _bytes_out;
};

#endif
