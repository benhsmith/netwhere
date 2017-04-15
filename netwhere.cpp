#include <time.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <tuple>
#include <unordered_map>

#include <tins/tins.h>

#include "flow.h"
#include "host.h"
#include "webservice.h"

using namespace Tins;
using namespace std;

FlowMap flows;
HostMap hosts;

void print_status() {
  cout << "----------------------------------------" << endl;

  IPv4Range local_range = IPv4Address("192.168.0.0") / 24;

  cout << "var nodes = new vis.DataSet([" << endl;
  for (auto it = hosts.begin(); it != hosts.end(); ++it) {
    if (it != hosts.begin()) {
      cout << ", ";
    }
  
    cout << "{id: " << static_cast<uint32_t>((*it).ip_addr()) << ", label: '" << (*it).ip_addr() << "'}";
  }
  
  cout << "]);" << endl;
  cout << "var edges = new vis.DataSet([" << endl;

  for (auto it = flows.begin(); it != flows.end(); ++it) {
    if (it != flows.begin()) {
      cout << ", ";
    }

    cout << "{from: " << static_cast<uint32_t>((*it).src_addr())
	 << ", to: " <<  static_cast<uint32_t>((*it).dst_addr())
	 << ", label:'" << (*it).packets_seen() << "'}";
  }
  
  cout << "]);" << endl;
}

timespec start;

bool callback(const PDU &pdu) {
    const IP &ip = pdu.rfind_pdu<IP>(); // Find the IP layer
    const TCP &tcp = pdu.rfind_pdu<TCP>(); // Find the TCP layer
    
    Flow& flow = flows[make_tuple(ip.src_addr(), ip.dst_addr())];

    Host& src = hosts[ip.src_addr()];
    Host& dst = hosts[ip.dst_addr()];

    src.add_out_flow(flow);
    dst.add_in_flow(flow);

    flow += 1;

    timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    if (now.tv_sec - start.tv_sec >= 10) {
      print_status();
      start = now;
    }

    return true;
}

int main() {
  clock_gettime(CLOCK_MONOTONIC, &start);

  WebService webservice;
  
  Sniffer("enp0s31f6").sniff_loop(callback);
}
