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
  void prune(int older_than);

  const FlowSummaries& flows() const {
	return _flows;
  }

  const HostSummaries& hosts() const {
	return _hosts;
  }

private:
  FlowSummaries _flows;
  HostSummaries _hosts;
  const Tins::IPv4Range _hosts_range;
};

#endif
