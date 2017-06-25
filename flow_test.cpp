#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE flow_test
#include <boost/test/unit_test.hpp>

#include "flow_collector.hpp"

using namespace std;
using namespace Tins;

BOOST_AUTO_TEST_CASE( tcp_flow ) {
  FlowsCollector collector(IPv4Range::from_mask("192.168.0.0", "255.255.255.0"));

  {
	TCP tcp(80, 10000);
	tcp.set_flag(TCP::SYN, 1);

	IP ip("192.168.0.20", "192.168.0.10");
	ip.protocol(Constants::IP::PROTO_TCP);
	EthernetII eth = EthernetII("11:22:33:44:55:66", "AA:BB:CC:DD:EE:FF")
	  / ip
	  / tcp
	  / RawPDU("hello");

	collector.collect(1234, eth);
  }

  {
	IP ip("192.168.0.10", "192.168.0.20");
	ip.protocol(Constants::IP::PROTO_TCP);
	EthernetII eth = EthernetII("AA:BB:CC:DD:EE:FF", "11:22:33:44:55:66")
	  / ip
	  / TCP(10000, 80)
	  / RawPDU("hi");

	collector.collect(1234, eth);
  }

  {
	TCP tcp(22, 10000);
	tcp.set_flag(TCP::SYN, 1);

	IP ip("192.168.0.10", "1.2.3.4");
	ip.protocol(Constants::IP::PROTO_TCP);
	EthernetII eth = EthernetII("AA:BB:CC:DD:EE:FF", "A1:B2:C3:D4:E5:F6")
	  / ip
	  / tcp
	  / RawPDU("hi");

	collector.collect(1234, eth);
  }

  BOOST_CHECK_EQUAL(collector.flows().size(), 2);
  BOOST_CHECK_EQUAL(collector.hosts().size(), 2);

  Flow http_flow("AA:BB:CC:DD:EE:FF", "192.168.0.10", "11:22:33:44:55:66", "192.168.0.20", 80, Constants::IP::PROTO_TCP);

  auto summary_it = value_iterator(collector.flows().find(http_flow));
  BOOST_REQUIRE(summary_it != collector.flows().end());
  BOOST_CHECK_EQUAL(http_flow.src_hw, (*summary_it)->flow().src_hw);
  BOOST_CHECK_EQUAL((*summary_it)->bytes_dst(), 5);
  BOOST_CHECK_EQUAL((*summary_it)->bytes_src(), 2);

  Flow ssh_flow("A1:B2:C3:D4:E5:F6", "1.2.3.4", "AA:BB:CC:DD:EE:FF", "192.168.0.10", 22, Constants::IP::PROTO_TCP);

  summary_it = value_iterator(collector.flows().find(ssh_flow));
  BOOST_REQUIRE(summary_it != collector.flows().end());
  BOOST_CHECK_EQUAL(ssh_flow.src_hw, (*summary_it)->flow().src_hw);
  BOOST_CHECK_EQUAL(ssh_flow.dst_hw, (*summary_it)->flow().dst_hw);
  BOOST_CHECK_EQUAL((*summary_it)->bytes_dst(), 2);

  auto host_it = value_iterator(collector.hosts().find(Host("AA:BB:CC:DD:EE:FF", "192.168.0.10")));
  BOOST_REQUIRE(host_it != collector.hosts().end());
  BOOST_CHECK_EQUAL((*host_it)->host().hw, "AA:BB:CC:DD:EE:FF");
  BOOST_CHECK_EQUAL((*host_it)->host().ip, "192.168.0.10");

  BOOST_REQUIRE_EQUAL((*host_it)->flows().size(),  2);
  BOOST_CHECK_EQUAL((*host_it)->flows()[0]->flow().src_ip, "192.168.0.10");
  BOOST_CHECK_EQUAL((*host_it)->flows()[0]->flow().dst_port, 80);
  BOOST_CHECK_EQUAL((*host_it)->flows()[1]->flow().src_ip, "1.2.3.4");
  BOOST_CHECK_EQUAL((*host_it)->flows()[1]->flow().dst_port, 22);

  host_it = value_iterator(collector.hosts().find(Host("11:22:33:44:55:66", "192.168.0.20")));
  BOOST_REQUIRE(host_it != collector.hosts().end());
  BOOST_CHECK_EQUAL((*host_it)->host().hw, "11:22:33:44:55:66");
  BOOST_CHECK_EQUAL((*host_it)->host().ip, "192.168.0.20");
}

BOOST_AUTO_TEST_CASE( udp_flow ) {
  FlowsCollector collector(IPv4Range::from_mask("192.168.0.0", "255.255.255.0"));

  EthernetII eth = EthernetII("AA:BB:CC:DD:EE:FF", "11:22:33:44:55:66")
	/ IP("1.2.3.4", "192.168.0.10")
	/ UDP(80, 10000)
	/ RawPDU("Hello UDP");

  collector.collect(1234, eth);

  BOOST_CHECK_EQUAL(collector.flows().size(), 1);
  auto it = value_iterator(collector.flows().begin());

  BOOST_CHECK_EQUAL((*it)->flow().src_ip, "192.168.0.10");
  BOOST_CHECK_EQUAL((*it)->flow().dst_ip, "1.2.3.4");
  BOOST_CHECK_EQUAL((*it)->bytes_dst(), 9);
}

BOOST_AUTO_TEST_CASE( prune ) {
  FlowsCollector collector(IPv4Range::from_mask("192.168.0.0", "255.255.255.0"));

  {
	TCP tcp(80, 10000);
	tcp.set_flag(TCP::SYN, 1);
	IP ip("1.2.3.4", "192.168.0.10");
	ip.protocol(Constants::IP::PROTO_TCP);
	EthernetII eth = EthernetII("11:22:33:44:55:66", "AA:BB:CC:DD:EE:FF")
	  / ip
	  / tcp;

	collector.collect(1000, eth);
  }

  {
	TCP tcp(80, 10000);
	tcp.set_flag(TCP::SYN, 1);
	IP ip("1.2.3.5", "192.168.0.10");
	ip.protocol(Constants::IP::PROTO_TCP);
	EthernetII eth = EthernetII("11:22:33:44:55:66", "AA:BB:CC:DD:EE:FF")
	  / ip
	  / tcp;

	collector.collect(2000, eth);
  }

  BOOST_CHECK_EQUAL(collector.flows().size(), 2);

  collector.prune(500);

  BOOST_CHECK_EQUAL(collector.flows().size(), 2);

  collector.prune(1500);

  BOOST_CHECK_EQUAL(collector.flows().size(), 1);

  auto summary_it = value_iterator(collector.flows().begin());
  BOOST_REQUIRE(summary_it != collector.flows().end());
  BOOST_CHECK_EQUAL((*summary_it)->flow().dst_ip, "1.2.3.5");
}

