/*
 * Copyright (c) 2017, Ben Smith
 * All rights reserved.
 *
 */

#include <vector>

#include "flow_collector.hpp"

using namespace std;
using namespace Tins;

void FlowsCollector::collect(time_t current_time, const PDU &pdu) {
  const EthernetII& eth = pdu.rfind_pdu<EthernetII>();
  const IP& ip = pdu.rfind_pdu<IP>();
  const TCP* tcp = pdu.find_pdu<TCP>();
  const UDP* udp = pdu.find_pdu<UDP>();

  if (!_hosts_range.contains(ip.src_addr()) && !_hosts_range.contains(ip.dst_addr())) {
	cout << "This shouldn't happen, neither source nor destination is in the specified host range:  " << ip.src_addr() << " -> " << ip.dst_addr() << ":" << tcp->dport() << endl;
	return;
  }

  if (tcp) {
	collect_tcp(current_time, eth, ip, tcp);
  } else if(udp) {
	collect_udp(current_time, eth, ip, udp);
  }
}

void FlowsCollector::collect_tcp(time_t current_time, const EthernetII& eth, const IP& ip, const TCP* tcp) {
  size_t datasize = tcp->size() - tcp->header_size();

  /* SYN indicates a new connection, so we make a Flow (if it doesn't already exist) whenever we see a packet with SYN set.
	 We want to establish Flows with meaningful destination ports, when possible. */
  if (tcp->flags() == TCP::SYN) {
	collect_tcp_connect(
	  Flow(eth.src_addr(), ip.src_addr(), eth.dst_addr(), ip.dst_addr(), tcp->dport(), ip.protocol()),
	  current_time,
	  datasize
	);
  }	else if (datasize > 0) {
	collect_ongoing(
	  Flow(eth.src_addr(), ip.src_addr(), eth.dst_addr(), ip.dst_addr(), tcp->dport(), ip.protocol()),
	  Flow(eth.dst_addr(), ip.dst_addr(), eth.src_addr(), ip.src_addr(), tcp->sport(), ip.protocol()),
	  current_time,
	  datasize,
	  false
	);
  }
}

void FlowsCollector::collect_tcp_connect(const Flow& flow, time_t current_time, int datasize) {
  auto flow_found = _flows.insert(flow);

  FlowCounter* flow_counter = *(flow_found.first);
  bool is_new_flow = flow_found.second;

  flow_counter->incr_dst(current_time, datasize);

  update_hosts(flow_counter, is_new_flow, current_time, datasize);
}

void FlowsCollector::collect_udp(time_t current_time, const Tins::EthernetII& eth, const Tins::IP& ip, const UDP* udp) {
  size_t datasize = udp->size() - udp->header_size();

  collect_ongoing(
	Flow(eth.src_addr(), ip.src_addr(), eth.dst_addr(), ip.dst_addr(), udp->dport(), ip.protocol()),
	Flow(eth.dst_addr(), ip.dst_addr(), eth.src_addr(), ip.src_addr(), udp->sport(), ip.protocol()),
	current_time,
	datasize,
	true
  );
}

void FlowsCollector::collect_ongoing(const Flow& flow_to, const Flow& flow_from, time_t current_time, int datasize, bool insert_if_not_found) {
  /* Search for an existing flow in either direction. */

  bool is_new_flow = false;

  auto found = _flows.find(flow_to);
  if (found != _flows.end()) {
	found->incr_dst(current_time, datasize);
  } else {
	found = _flows.find(flow_from);
	if (found != _flows.end()) {
	  found->incr_src(current_time, datasize);
	} else if (insert_if_not_found) {
		is_new_flow = true;
		auto inserted = _flows.insert(flow_to);
		found = inserted.first;
		found->incr_dst(current_time, datasize);
	} else {
	  return;
	}
  }

  update_hosts(*found, is_new_flow, current_time, datasize);
}

void FlowsCollector::update_hosts(const FlowCounter* flow_counter, bool is_new_flow, time_t current_time, int datasize) {
  if (_hosts_range.contains(flow_counter->flow().src_ip)) {
	auto host_found = _hosts.insert(Host(flow_counter->flow().src_hw, flow_counter->flow().src_ip));

	if (is_new_flow) {
	  host_found.first->add_flow(flow_counter);
	}

	host_found.first->update_host_stats(flow_counter->flow(), datasize);
  }

  if (_hosts_range.contains(flow_counter->flow().dst_ip)) {
	auto host_found = _hosts.insert(Host(flow_counter->flow().dst_hw, flow_counter->flow().dst_ip));

	if (is_new_flow) {
	  host_found.first->add_flow(flow_counter);
	}

	host_found.first->update_host_stats(flow_counter->flow(), datasize);
  }
}

void FlowsCollector::prune(int older_than) {
  vector<Flow> to_remove;
  for (auto && summary : _flows) {
	if (summary->modified_at() < older_than) {
	  to_remove.push_back(summary->flow());
	}
  }

  cout << "Pruning " << to_remove.size() << " flows" << endl;

  for (auto && flow : to_remove) {

	auto host_flow = _hosts.find(Host(flow.src_hw, flow.src_ip));
	if (host_flow != _hosts.end()) {
	  host_flow->remove_flow(flow);
	}

	host_flow = _hosts.find(Host(flow.dst_hw, flow.dst_ip));
	if (host_flow != _hosts.end()) {
	  host_flow->remove_flow(flow);
	}

	_flows.erase(flow);
  }
}

