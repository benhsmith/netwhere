#ifndef __FLOW_H__
#define __FLOW_H__

#include <unordered_map>
#include <tuple>
#include <tins/tins.h>
#include <boost/functional/hash.hpp>

#include "pointer_iterator.hpp"

/*
 * A Flow represents communication between a source Host and a destination Host and port.
 * We retain the destination port since it often indicates the service being requested but
 * ignore the source port. Ignoring the source port makes it easier to aggregate the same
 * type of communication into a single Flow.
 */
struct Flow {
  Flow(const Tins::EthernetII::address_type& src_hw,
	   const Tins::IPv4Address& src_ip,
	   const Tins::EthernetII::address_type& dst_hw,
	   const Tins::IPv4Address& dst_ip,
	   int dst_port,
	   uint8_t ip_protocol)
	: src_hw(src_hw), src_ip(src_ip),
	  dst_hw(dst_hw), dst_ip(dst_ip), dst_port(dst_port), ip_protocol(ip_protocol) {}

  bool operator == (const Flow& other) const {
	return src_hw == other.src_hw
	  && src_ip   == other.src_ip
	  && dst_hw   == other.dst_hw
	  && dst_ip   == other.dst_ip
	  && dst_port == other.dst_port;
  }

  const Tins::EthernetII::address_type src_hw;
  const Tins::IPv4Address src_ip;
  const Tins::EthernetII::address_type dst_hw;
  const Tins::IPv4Address dst_ip;
  const int dst_port;
  const uint8_t ip_protocol;
};


class FlowSummary {
public:
  FlowSummary(const Flow& flow) : _flow(flow), _bytes_to_src(0), _bytes_to_dst(0), _modified_at(0) {}
  FlowSummary(const Flow& flow, time_t current_time, size_t bytes) : _flow(flow),  _bytes_to_src(0), _bytes_to_dst(0), _modified_at(current_time) {}

  void incr_src(time_t current_time, size_t bytes) {
	_bytes_to_src += bytes;
	_modified_at = current_time;
  }

  void incr_dst(time_t current_time, size_t bytes) {
	_bytes_to_dst += bytes;
	_modified_at = current_time;
  }

  const Flow& flow() const { return _flow; }
  size_t bytes_to_src() const { return _bytes_to_src; }
  size_t bytes_to_dst() const { return _bytes_to_dst; }
  time_t modified_at() const { return _modified_at; }

private:
  const Flow _flow;
  size_t _bytes_to_src;
  size_t _bytes_to_dst;
  time_t _modified_at;
};

namespace std {
  template <>
  struct hash<Flow>
  {
	size_t operator()(const Flow& flow) const {
	  size_t seed = 0;
	  for(auto && value : flow.src_hw) {
		boost::hash_combine<uint8_t>(seed, value);
	  }
	  boost::hash_combine<uint32_t>(seed, flow.src_ip);
	  for(auto && value : flow.dst_hw) {
		boost::hash_combine<uint8_t>(seed, value);
	  }
	  boost::hash_combine<uint32_t>(seed, flow.dst_ip);
	  boost::hash_combine<uint32_t>(seed, flow.dst_port);
	  boost::hash_combine<uint8_t>(seed, flow.ip_protocol);

	  return seed;
	}
  };
}

#endif
