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
  
  void collect(time_t current_time, const Tins::PDU &pdu) {
	const Tins::EthernetII &eth = pdu.rfind_pdu<Tins::EthernetII>();
    const Tins::IP &ip = pdu.rfind_pdu<Tins::IP>();

	const Tins::TCP* tcp = pdu.find_pdu<Tins::TCP>();
	const Tins::UDP* udp = pdu.find_pdu<Tins::UDP>();

	if (tcp) {
	  size_t data_size = tcp->size() - tcp->header_size();

	  if (tcp->flags() == Tins::TCP::SYN) {
		Flow flow(eth.src_addr(), ip.src_addr(), eth.dst_addr(), ip.dst_addr(), tcp->dport(), ip.protocol());
		
		HostFlows &sender = _hosts[Host(eth.src_addr(), ip.src_addr())];

		auto it = _tcp_flows.find(flow);
		
		if (it == _tcp_flows.end()) {
		  FlowSummary &summary = _tcp_flows[flow];
		  summary.incr_dst(current_time, data_size);
		  sender.add_flow(&summary);

		  if (_hosts_range.contains(ip.dst_addr())) {
			HostFlows &target = _hosts[Host(eth.dst_addr(), ip.dst_addr())];
			target.add_flow(&summary);
		  }
		} else  {
		  it->second->incr_dst(current_time, data_size);
		}
	  } else if (data_size > 0) {
		Flow flow_to(eth.src_addr(), ip.src_addr(), eth.dst_addr(), ip.dst_addr(), tcp->dport(), ip.protocol());
		Flow flow_from(eth.dst_addr(), ip.dst_addr(), eth.src_addr(), ip.src_addr(), tcp->sport(), ip.protocol());

		if (_tcp_flows.find(flow_to) != _tcp_flows.end()) {
		  FlowSummary& summary = _tcp_flows[flow_to];
		  HostFlows& host = _hosts.at(Host(eth.src_addr(), ip.src_addr()));
		  
		  _tcp_flows[flow_to].incr_dst(current_time, data_size);
		} else if (_tcp_flows.find(flow_from) != _tcp_flows.end()) {
		  FlowSummary& summary = _tcp_flows[flow_from];
		  HostFlows& host = _hosts.at(Host(eth.dst_addr(), ip.dst_addr()));

		  _tcp_flows[flow_from].incr_src(current_time, data_size);		  
		}
	  }
	}/* else if(udp) {
	  size_t data_size = udp->size() - udp->header_size();
	  
	  Flow flow(eth, ip, udp->dport());

	  if (!_udp_flows.has(flow)) {
		FlowSummary &summary = _udp_flows[flow];
		HostFlows &sender = _hosts[Host(eth.src_addr(), ip.src_addr())];
		HostFlows &target = _hosts[Host(eth.dst_addr(), ip.dst_addr())];
		
		sender.add_out_flow(&summary);
		target.add_in_flow(&summary);
	  }
	  
	  _udp_flows[flow].incr_to(current_time, data_size);

	} else {
	  std::cerr << "Packet was not TCP or UDP" << std::endl;
	  }*/
  }

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
