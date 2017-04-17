#include <time.h>
#include <unistd.h>

#include <functional>
#include <iostream>
#include <string>
#include <tuple>
#include <unordered_map>

#include "netwhere.hpp"

using namespace Tins;
using namespace std;
using namespace std::placeholders;


void NetWhere::start() {
  auto functor = bind(&NetWhere::handle_request,this,_1);
  WebService<decltype(functor)> webservice(functor);
  webservice.start();
  
  Sniffer("enp0s31f6").sniff_loop(bind(&NetWhere::handle_packet,this,_1));
}

bool NetWhere::handle_packet(const PDU &pdu) {
    const IP &ip = pdu.rfind_pdu<IP>(); // Find the IP layer
    const TCP &tcp = pdu.rfind_pdu<TCP>(); // Find the TCP layer

    rw_exclusion.writer_acquire();

    Flow& flow = flows[make_tuple(ip.src_addr(), ip.dst_addr())];

    Host& src = hosts[ip.src_addr()];
    Host& dst = hosts[ip.dst_addr()];

    src.add_out_flow(flow);
    dst.add_in_flow(flow);

    flow += 1;

    rw_exclusion.writer_release();

    return true;
}

string NetWhere::handle_request(const string& url) {
  cout << "Request received : " << url << endl;
  size_t len = url.size();

  if (len == 0) {
    return full_stats();
  }
  
  size_t start = url.find_last_of('/');
  
  if (start == len-1)
    return full_stats();

  int counter;

  try {
    int counter = stoi(url.substr(start + 1, len - (start + 1)));
  } catch (invalid_argument) {
    cout << "Invalid URL : " << url << endl;
    return "";
  }

  return stats_since_counter(counter);
}

string NetWhere::full_stats() {
  ostringstream out;
  
  IPv4Range local_range = IPv4Address("192.168.0.0") / 24;

  out << "{ \"nodes\": [" << endl;
  for (auto it = hosts.begin(); it != hosts.end(); ++it) {
    if (it != hosts.begin()) {
      out << ", ";
    }
  
    out << "{ \"id\": " << static_cast<uint32_t>((*it).ip_addr()) << ", \"label\": \"" << (*it).ip_addr() << "\"}";
  }
  
  out << "]," << endl;
  out << "\"edges\" : [" << endl;

  for (auto it = flows.begin(); it != flows.end(); ++it) {
    if (it != flows.begin()) {
      out << ", ";
    }

    out << "{\"from\": " << static_cast<uint32_t>((*it).src_addr())
	 << ", \"to\": " <<  static_cast<uint32_t>((*it).dst_addr())
	 << ", \"label\":" << (*it).packets_seen() << "}";
  }
  
  out << "]}" << endl;

  return out.str();
}

string NetWhere::stats_since_counter(int counter) {
  return "";
}
