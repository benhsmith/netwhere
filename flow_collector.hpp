#ifndef __FLOW_COLLECTOR_H__
#define __FLOW_COLLECTOR_H__

#include "flow.hpp"
#include "host.hpp"
#include "auto_dynamic_map.hpp"

class FlowsCollector {
public:
  typedef AutoDynamicMap<Flow, FlowSummary> FlowSummaries;
  typedef AutoDynamicMap<Host, HostFlows> HostSummaries;

  FlowsCollector(const Tins::IPv4Range hosts_range) : _hosts_range(hosts_range) {}

  void collect(time_t current_time, const Tins::PDU &pdu);

  const FlowSummaries& tcp_flows() const {
	return _tcp_flows;
  }

  const FlowSummaries& udp_flows() const {
	return _udp_flows;
  }

  const HostSummaries& hosts() const {
	return _hosts;
  }
private:
  FlowSummaries _tcp_flows;
  FlowSummaries _udp_flows;
  HostSummaries _hosts;
  const Tins::IPv4Range _hosts_range;
};

#endif
