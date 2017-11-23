/*
 * Copyright (c) 2017, Ben Smith
 * All rights reserved.
 *
 */

#ifndef __FLOW_COLLECTOR_H__
#define __FLOW_COLLECTOR_H__

#include "flow.hpp"
#include "host.hpp"

class FlowsCollector {
public:
  FlowsCollector(const Tins::IPv4Range hosts_range) : _hosts_range(hosts_range) {}

  void collect(time_t current_time, const Tins::PDU &pdu);
  void collect_tcp(time_t current_time, const Tins::EthernetII& eth, const Tins::IP& ip, const Tins::TCP* udp);
  void collect_udp(time_t current_time, const Tins::EthernetII& eth, const Tins::IP& ip, const Tins::UDP* udp);
  void collect_tcp_connect(const Flow& flow, time_t current_time, int datasize);
  void collect_ongoing(const Flow& flow_to, const Flow& flow_from, time_t current_time, int datasize, bool insert_if_not_found);

  void update_hosts(const FlowCounter* flow_counter, bool is_new_flow, time_t current_time, int datasize);

  void prune(int older_than);

  const FlowCounters& flows() const {
	return _flows;
  }

  const HostSummaries& hosts() const {
	return _hosts;
  }

private:
  FlowCounters _flows;
  HostSummaries _hosts;
  const Tins::IPv4Range _hosts_range;
};

#endif
