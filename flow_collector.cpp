#include "flow_collector.hpp"

void FlowsCollector::collect(time_t current_time, const Tins::PDU &pdu) {
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
	  Flow flow_to(eth.src_addr(), ip.src_addr(), eth.dst_addr(), ip.dst_addr(), udp->dport(), ip.protocol());
	  Flow flow_from(eth.dst_addr(), ip.dst_addr(), eth.src_addr(), ip.src_addr(), udp->sport(), ip.protocol());

	  if (!_udp_flows.has(flow)) {
	  FlowSummary &summary = _udp_flows[flow];
	  HostFlows &sender = _hosts[Host(eth.src_addr(), ip.src_addr())];
	  HostFlows &target = _hosts[Host(eth.dst_addr(), ip.dst_addr())];

	  sender.add_out_flow(&summary);
	  target.add_in_flow(&summary);
	  }

	  _udp_flows[flow].incr_to(current_time, data_size);
	  }*/
}
