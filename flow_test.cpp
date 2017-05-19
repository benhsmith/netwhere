#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE netwhere_test
#include <boost/test/unit_test.hpp>

#include "flow_collector.hpp"

using namespace std;
using namespace Tins;

BOOST_AUTO_TEST_CASE( tcp_flow ) {
  FlowsCollector collector(IPv4Range::from_mask("192.168.0.0", "255.255.255.0"));

  {
	IP ip("192.168.0.10", "192.168.0.20");
	ip.protocol(Constants::IP::PROTO_TCP);
	ARP arp = ARP("192.168.0.10", "192.168.0.20", "00:11:22:33:44:55", "AA:BB:CC:DD:EE:FF")
	  / ip
	  / TCP(80, 10000)
	  / RawPDU("hello");
  
	collector.collect(1234, arp);
  }

  {
	IP ip("192.168.0.20", "192.168.0.10");
	ip.protocol(Constants::IP::PROTO_TCP);
	ARP arp = ARP("192.168.0.20", "192.168.0.10", "AA:BB:CC:DD:EE:FF", "00:11:22:33:44:55")
	  / ip
	  / TCP(10000, 80)
	  / RawPDU("hi");
  
	collector.collect(1234, arp);
  }

  BOOST_CHECK_EQUAL(collector.tcp_flows().size(), 2);
  BOOST_CHECK_EQUAL(collector.hosts().size(), 2);

  Flow flow( "AA:BB:CC:DD:EE:FF", "192.168.0.20", "00:11:22:33:44:55", "192.168.0.10", 80, Constants::IP::PROTO_TCP);
  auto summary_it = value_iterator(collector.tcp_flows().find(flow));
  BOOST_REQUIRE(summary_it != collector.tcp_flows().end()); 
  BOOST_CHECK_EQUAL(flow.src_hw, (*summary_it)->flow().src_hw);
  BOOST_CHECK_EQUAL((*summary_it)->bytes_dst(), 5);
  BOOST_CHECK_EQUAL((*summary_it)->bytes_src(), 2);

  auto host_it = value_iterator(collector.hosts().find(Host("AA:BB:CC:DD:EE:FF", "192.168.0.20")));
  BOOST_REQUIRE(host_it != collector.hosts().end()); 
  BOOST_CHECK_EQUAL((*host_it)->host().hw, "AA:BB:CC:DD:EE:FF");
  BOOST_CHECK_EQUAL((*host_it)->host().ip, "192.168.0.20");

  auto flows = (*host_it)->flows();
  BOOST_CHECK_EQUAL(flows[0]->flow().src_hw,  "00:11:22:33:44:55");
  BOOST_CHECK_EQUAL(flows[0]->bytes_dst(), 2);

  //BOOST_CHECK_EQUAL(flows[1]->flow().dst_hw,  "00:11:22:33:44:55");
  BOOST_CHECK_EQUAL(flows[0]->bytes_src(), 5);	 
}

BOOST_AUTO_TEST_CASE( udp_flow ) {
  FlowsCollector collector(IPv4Range::from_mask("192.168.0.", "255.255.255.0"));

  ARP arp = ARP("192.168.0.10", "192.168.0.20", "00:11:22:33:44:55", "AA:BB:CC:DD:EE:FF")
	/ IP("192.168.0.10", "192.168.0.20")
	/ UDP(10000, 80)
	/ RawPDU("Hello UDP");
 
  collector.collect(1234, arp);
 
  BOOST_CHECK_EQUAL(collector.udp_flows().size(), 1);
  auto it = value_iterator(collector.udp_flows().begin());
  
  BOOST_CHECK_EQUAL((*it)->flow().src_ip, "192.168.0.20");
  BOOST_CHECK_EQUAL((*it)->flow().dst_ip, "192.168.0.10");
  BOOST_CHECK_EQUAL((*it)->bytes_dst(), 9);
}

