#include <vector>

#include "flow_collector.hpp"

using namespace std;
using namespace Tins;

void FlowsCollector::collect(time_t current_time, const PDU &pdu) {
  const EthernetII &eth = pdu.rfind_pdu<EthernetII>();
  const IP &ip = pdu.rfind_pdu<IP>();

  const TCP* tcp = pdu.find_pdu<TCP>();
  const UDP* udp = pdu.find_pdu<UDP>();

  if (tcp) {
	size_t data_size = tcp->size() - tcp->header_size();

	/* SYN indicates a new connection, so we make a Flow (if it doesn't already exist) whenever we see a packet with SYN set.
	   We want to establish Flows with meaningful destination ports, when possible. */
	if (tcp->flags() == TCP::SYN) {
	  Flow flow(eth.src_addr(), ip.src_addr(), eth.dst_addr(), ip.dst_addr(), tcp->dport(), ip.protocol());
	  auto it = _flows.find(flow);
	  if (it == _flows.end()) {
		if (!_hosts_range.contains(ip.src_addr()) && !_hosts_range.contains(ip.dst_addr())) {
		  cout << "This is weird:  " << ip.src_addr() << " -> " << ip.dst_addr() << ":" << tcp->dport() << endl;
		  return;
		}

		FlowSummary &summary = _flows[flow];
		summary.incr_dst(current_time, data_size);

		if (_hosts_range.contains(ip.src_addr())) {
		  HostFlows& host = _hosts[Host(eth.src_addr(), ip.src_addr())];
		  host.add_flow(&summary);
		}

		if (_hosts_range.contains(ip.dst_addr())) {
		  HostFlows &target = _hosts[Host(eth.dst_addr(), ip.dst_addr())];
		  target.add_flow(&summary);
		}
	  } else {
		it->second->incr_dst(current_time, data_size);
	  }
	} else if (data_size > 0) {
	  Flow flow_to(eth.src_addr(), ip.src_addr(), eth.dst_addr(), ip.dst_addr(), tcp->dport(), ip.protocol());

	  auto it = _flows.find(flow_to);
	  if (it != _flows.end()) {
		it->second->incr_dst(current_time, data_size);
	  } else {
		Flow flow_from(eth.dst_addr(), ip.dst_addr(), eth.src_addr(), ip.src_addr(), tcp->sport(), ip.protocol());

		it = _flows.find(flow_from);
		if (it != _flows.end()) {
		  it->second->incr_src(current_time, data_size);
		}
	  }
	}
  } else if(udp) {
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
	}
  }
}

void FlowsCollector::prune(int older_than) {
  vector<Flow> to_remove;
  for (auto && summary : _flows) {
	if (summary.second->modified_at() < older_than) {
	  to_remove.push_back(summary.first);
	}
  }

  for (auto && flow : to_remove) {
	_flows.erase(flow);
  }
}
