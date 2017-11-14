#include <vector>

#include "flow_collector.hpp"

using namespace std;
using namespace Tins;

void FlowsCollector::collect(time_t current_time, const PDU &pdu) {
  const EthernetII& eth = pdu.rfind_pdu<EthernetII>();
  const IP& ip = pdu.rfind_pdu<IP>();
  const TCP* tcp = pdu.find_pdu<TCP>();
  const UDP* udp = pdu.find_pdu<UDP>();

  if (tcp) {
	collect_tcp(current_time, eth, ip, tcp);
  } else if(udp) {
	collect_udp(current_time, eth, ip, udp);
  }
}

void FlowsCollector::collect_tcp(time_t current_time, const Tins::EthernetII& eth, const Tins::IP& ip, const TCP* tcp) {
  size_t data_size = tcp->size() - tcp->header_size();

  /* SYN indicates a new connection, so we make a Flow (if it doesn't already exist) whenever we see a packet with SYN set.
	 We want to establish Flows with meaningful destination ports, when possible. */
  if (tcp->flags() == TCP::SYN) {
	Flow flow(eth.src_addr(), ip.src_addr(), eth.dst_addr(), ip.dst_addr(), tcp->dport(), ip.protocol());
	bool is_new_flow = _flows.has(flow);

	FlowSummary& summary = _flows[flow];
	summary.incr_dst(current_time, data_size);

	if (!_hosts_range.contains(ip.src_addr()) && !_hosts_range.contains(ip.dst_addr())) {
	  cout << "Neither source nor destination is in the specified host range:  " << ip.src_addr() << " -> " << ip.dst_addr() << ":" << tcp->dport() << endl;
	  return;
	}

	if (_hosts_range.contains(ip.src_addr())) {
	  HostFlows& sender = _hosts[Host(eth.src_addr(), ip.src_addr())];
	  if (is_new_flow) {
		sender.add_flow(&summary);
	  }

	  sender.update_host_stats(summary);
	}

	if (_hosts_range.contains(ip.dst_addr())) {
	  HostFlows& target = _hosts[Host(eth.dst_addr(), ip.dst_addr())];
	  if (is_new_flow) {
		target.add_flow(&summary);
	  }

	  target.update_host_stats(summary);
	}
  } else if (data_size > 0) {
	/* Search for an existing flow in either direction. */
	Flow flow_to(eth.src_addr(), ip.src_addr(), eth.dst_addr(), ip.dst_addr(), tcp->dport(), ip.protocol());

	auto flow_it = _flows.find(flow_to);
	if (flow_it != _flows.end()) {
	  flow_it->second->incr_dst(current_time, data_size);
	} else {
	  Flow flow_from(eth.dst_addr(), ip.dst_addr(), eth.src_addr(), ip.src_addr(), tcp->sport(), ip.protocol());
	  auto flow_it = _flows.find(flow_to);
	  if (flow_it != _flows.end()) {
		flow_it->second->incr_dst(current_time, data_size);
	  }
	}

	if (_hosts_range.contains(ip.src_addr())) {
	  HostFlows& sender = _hosts[Host(eth.src_addr(), ip.src_addr())];
	  sender.update_host_stats(flow_it->second);
	}

	if (_hosts_range.contains(ip.dst_addr())) {
	  HostFlows& target = _hosts[Host(eth.dst_addr(), ip.dst_addr())];
	  target.update_host_stats(flow_it->second);
	}
  }
}

void FlowsCollector::collect_udp(time_t current_time, const Tins::EthernetII& eth, const Tins::IP& ip, const UDP* udp) {
  size_t data_size = udp->size() - udp->header_size();

  if (_hosts_range.contains(ip.src_addr())) {
	Flow flow(eth.src_addr(), ip.src_addr(), eth.dst_addr(), ip.dst_addr(), udp->dport(), ip.protocol());
	HostFlows& sender = _hosts[Host(eth.src_addr(), ip.src_addr())];

	auto it = _flows.find(flow);
	if (it == _flows.end()) {
	  FlowSummary& summary = _flows[flow];
	  summary.incr_dst(current_time, data_size);
	  sender.add_flow(&summary);
	} else {
	  it->second->incr_dst(current_time, data_size);
	}

	sender.update_host_stats(summary);
  }

  if (_hosts_range.contains(ip.dst_addr())) {
	Flow flow(eth.dst_addr(), ip.dst_addr(), eth.src_addr(), ip.src_addr(), udp->sport(), ip.protocol());
	HostFlows& target = _hosts[Host(eth.dst_addr(), ip.dst_addr())];

	auto it = _flows.find(flow);
	if (it == _flows.end()) {
	  FlowSummary& summary = _flows[flow];
	  summary.incr_src(current_time, data_size);
	  target.add_flow(&summary);
	} else {
	  it->second->incr_dst(current_time, data_size);
	}

	target.update_host_stats(summary);
  }
}

void FlowsCollector::prune(int older_than) {
  vector<Flow> to_remove;
  for (auto && summary : _flows) {
	if (summary.second->modified_at() < older_than) {
	  to_remove.push_back(summary.first);
	}
  }

  cout << "Pruning " << to_remove.size() << " flows" << endl;

  for (auto && flow : to_remove) {

	auto host_flow = _hosts.find(Host(flow.src_hw, flow.src_ip));
	if (host_flow != _hosts.end()) {
	  host_flow->second->remove_flow(flow);
	}

	host_flow = _hosts.find(Host(flow.dst_hw, flow.dst_ip));
	if (host_flow != _hosts.end()) {
	  host_flow->second->remove_flow(flow);
	}

	_flows.erase(flow);
  }
}
