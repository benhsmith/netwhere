#include <time.h>
#include <unistd.h>

#include <functional>
#include <iostream>
#include <string>
#include <tuple>
#include <unordered_map>

#include <boost/format.hpp>

#include "netwhere.hpp"

using namespace std;
using namespace std::placeholders;
using namespace boost;
using namespace Tins;

void NetWhere::start() {
  auto functor = bind(&NetWhere::handle_request,this,_1);
  WebService<decltype(functor)> webservice(functor);
  webservice.start();
  
  Sniffer(interface).sniff_loop(bind(&NetWhere::handle_packet,this,_1));
}

bool NetWhere::handle_packet(const PDU &pdu) {
  const IP &ip = pdu.rfind_pdu<IP>();
  const TCP &tcp = pdu.rfind_pdu<TCP>();

  rw_exclusion.writer_acquire();

  if (internal_range.contains(ip.src_addr())) {
    Host& host = hosts[ip.src_addr()];
    host.add_out_flow(ip.dst_addr(), ip.tot_len() - ip.header_size());
  } else if (internal_range.contains(ip.dst_addr()))  {
    Host& host = hosts[ip.dst_addr()];
    host.add_in_flow(ip.src_addr(), ip.tot_len() - ip.header_size());
  }

  rw_exclusion.writer_release();

  return true;
}

string NetWhere::handle_request(const string& url) {
  size_t len = url.size();

  if (len == 0) {
    return full_stats();
  }
  
  size_t start = url.find_last_of('/');
  
  if (start == len-1)
    return full_stats();

  cout << "Invalid URL : " << url << endl;
}

string NetWhere::full_stats() {
  rw_exclusion.reader_acquire();

  ostringstream out;
  
  out << "{";

  for (auto it = hosts.begin(); it != hosts.end(); ++it) {
    if (it != hosts.begin()) {
      out << ",";
    }

    out << endl << format("\"%1%\": [") % (*it).ip_addr();
    
    auto peers = (*it).get_peers();
    for (auto peer_it = peers.begin(); peer_it != peers.end(); ++peer_it) {
      if (peer_it != peers.begin()) {
	out << ", ";
      }

      out << format("[\"%1%\", %2%, %3%]") % (*peer_it).first % get<0>((*peer_it).second) %get<1>((*peer_it).second);
    }

    out << "]";
  }
  
  out << "}" << endl;

  rw_exclusion.reader_release();

  return out.str();
}

string NetWhere::stats_since_counter(int counter) {
  return "";
}
