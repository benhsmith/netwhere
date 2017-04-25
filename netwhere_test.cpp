#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE netwhere_test
#include <boost/test/unit_test.hpp>

#include <tuple>

#include <tins/tins.h>

#include "host.hpp"
#include "netwhere.hpp"

using namespace std;
using namespace Tins;

BOOST_AUTO_TEST_CASE( hash_map_create ) {
  HostMap hosts;

  IPv4Address src = IPv4Address("192.168.0.123");

  Host& host1 = hosts[src];
  Host& host2 = hosts[src];

  BOOST_CHECK_EQUAL(&host1, &host2);
}

BOOST_AUTO_TEST_CASE( handle_request ) {
  string output = "{}\n";

  NetWhere netwhere("", Tins::IPv4Range("192.168.0.0", "255.255.255.0"));

  BOOST_CHECK_EQUAL(netwhere.handle_request(""), output);

  BOOST_CHECK_EQUAL(netwhere.handle_request("/"), output);
}
