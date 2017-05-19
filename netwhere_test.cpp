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
  string output = "{}\n";

  NetWhere netwhere("", Tins::IPv4Range("192.168.0.0", "255.255.255.0"));

  BOOST_CHECK_EQUAL(netwhere.handle_request(""), output);

  BOOST_CHECK_EQUAL(netwhere.handle_request("/"), output);
}

BOOST_AUTO_TEST_CASE( handle_request ) {
  string output = "{}\n";

  NetWhere netwhere("", Tins::IPv4Range("192.168.0.0", "255.255.255.0"));

  TCP tcp(443, 10000);
  tcp.set_flag(TCP::SYN, 1);

  EthernetII eth = EthernetII("11:22:33:44:55:66", "AA:BB:CC:DD:EE:FF")
	/ IP("1.2.3.4", "192.168.0.123")
	/ tcp;
  
  netwhere.handle_packet(eth);
  
  BOOST_CHECK_EQUAL(netwhere.handle_request(""), "{\n" \
    "\"192.168.0.123|aa:bb:cc:dd:ee:ff\": [" \
      "[ [\"aa:bb:cc:dd:ee:ff\", \"192.168.0.123\"], [\"11:22:33:44:55:66\", \"1.2.3.4\", 443], 0, 0 ]" \
	"]" \
  "}\n"
  );

  eth = EthernetII("11:22:33:44:55:66", "AA:BB:CC:DD:EE:FF")
	/ IP("2.3.4.5", "192.168.0.123")
	/ tcp;

  netwhere.handle_packet(eth);

  BOOST_CHECK_EQUAL(netwhere.handle_request(""), "{\n" \
    "\"192.168.0.123|aa:bb:cc:dd:ee:ff\": [" \
      "[ [\"aa:bb:cc:dd:ee:ff\", \"192.168.0.123\"], [\"11:22:33:44:55:66\", \"1.2.3.4\", 443], 0, 0 ], " \
      "[ [\"aa:bb:cc:dd:ee:ff\", \"192.168.0.123\"], [\"11:22:33:44:55:66\", \"2.3.4.5\", 443], 0, 0 ]" \
	"]" \
  "}\n"
  );

  eth = EthernetII("11:22:33:44:55:66", "BA:BA:CC:DD:EE:FF")
	/ IP("2.3.4.5", "192.168.0.124")
	/ tcp;

  netwhere.handle_packet(eth);

  BOOST_CHECK_EQUAL(netwhere.handle_request(""), "{\n" \
    "\"192.168.0.124|ba:ba:cc:dd:ee:ff\": [" \
      "[ [\"ba:ba:cc:dd:ee:ff\", \"192.168.0.124\"], [\"11:22:33:44:55:66\", \"2.3.4.5\", 443], 0, 0 ]" \
	"],\n" \
    "\"192.168.0.123|aa:bb:cc:dd:ee:ff\": [" \
      "[ [\"aa:bb:cc:dd:ee:ff\", \"192.168.0.123\"], [\"11:22:33:44:55:66\", \"1.2.3.4\", 443], 0, 0 ], " \
      "[ [\"aa:bb:cc:dd:ee:ff\", \"192.168.0.123\"], [\"11:22:33:44:55:66\", \"2.3.4.5\", 443], 0, 0 ]" \
	"]" \
  "}\n"
  );

}
