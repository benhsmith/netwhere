#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE netwhere_test
#include <boost/test/unit_test.hpp>

#include <tuple>

#include <tins/tins.h>

#include "host.hpp"
#include "netwhere.hpp"

using namespace std;
using namespace Tins;

BOOST_AUTO_TEST_CASE( handle_request__empty ) {
  string output = "[]\n";

  NetWhere netwhere("", Tins::IPv4Range("192.168.0.0", "255.255.255.0"));

  BOOST_CHECK_EQUAL(netwhere.handle_request(""), output);

  BOOST_CHECK_EQUAL(netwhere.handle_request("/"), output);

  BOOST_CHECK_EQUAL(netwhere.handle_request("#"), output);
}

BOOST_AUTO_TEST_CASE( handle_request ) {
  NetWhere netwhere("", Tins::IPv4Range("192.168.0.0", "255.255.255.0"));

  TCP tcp(443, 10000);
  tcp.set_flag(TCP::SYN, 1);
  IP ip("1.2.3.4", "192.168.0.123");
  ip.protocol(Constants::IP::PROTO_TCP);

  EthernetII eth = EthernetII("11:22:33:44:55:66", "AA:BB:CC:DD:EE:FF")
	/ ip
	/ tcp;

  netwhere.handle_packet(eth);

  BOOST_CHECK_EQUAL(netwhere.handle_request(""), "[" \
    "[\"192.168.0.123\",\"aa:bb:cc:dd:ee:ff\",\"192.168.0.123\",\"0\",\"0\"]" \
    "]\n"
  );

  BOOST_CHECK_EQUAL(netwhere.handle_request("/aa:bb:cc:dd:ee:ff-192.168.0.123"), "[" \
      "[[[\"aa:bb:cc:dd:ee:ff\", \"192.168.0.123\"], [\"11:22:33:44:55:66\", \"1.2.3.4\"], 443, 6], 0, 0 ]" \
    "]\n"
  );

  ip.dst_addr("2.3.4.5");

  eth = EthernetII("11:22:33:44:55:66", "AA:BB:CC:DD:EE:FF")
	/ ip
	/ tcp;

  netwhere.handle_packet(eth);

  BOOST_CHECK_EQUAL(netwhere.handle_request("/aa:bb:cc:dd:ee:ff-192.168.0.123#"), "[" \
      "[[[\"aa:bb:cc:dd:ee:ff\", \"192.168.0.123\"], [\"11:22:33:44:55:66\", \"1.2.3.4\"], 443, 6], 0, 0 ], " \
      "[[[\"aa:bb:cc:dd:ee:ff\", \"192.168.0.123\"], [\"11:22:33:44:55:66\", \"2.3.4.5\"], 443, 6], 0, 0 ]" \
	"]\n"
  );

  ip.src_addr("192.168.0.124");

  eth = EthernetII("11:22:33:44:55:66", "BA:BA:CC:DD:EE:FF")
	/ ip
	/ tcp;

  netwhere.handle_packet(eth);

  BOOST_CHECK_EQUAL(netwhere.handle_request(""), "[" \
     "[\"192.168.0.124\",\"ba:ba:cc:dd:ee:ff\",\"192.168.0.124\",\"0\",\"0\"]," \
     "[\"192.168.0.123\",\"aa:bb:cc:dd:ee:ff\",\"192.168.0.123\",\"0\",\"0\"]" \
	"]\n"
  );
}
