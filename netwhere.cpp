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
  
  Sniffer(_interface).sniff_loop(bind(&NetWhere::handle_packet,this,_1));
}

bool NetWhere::handle_packet(const PDU &pdu) {
  ReaderWriterExclusion::WriterGuard guard(_rw_exclusion);

  _collector.collect(time(nullptr), pdu);

  return true;
}

string NetWhere::handle_request(const string& url) {
  size_t len = url.size();

  if (len == 0) {
    return hosts();
  }
  
  size_t start = url.find_last_of('/');
  
  if (start == len-1)
    return hosts();

  cout << "Invalid URL : " << url << endl;
}

string NetWhere::hosts() {
  ReaderWriterExclusion::ReaderGuard guard(_rw_exclusion);

  ostringstream out;
  
  out << "{";

  for (auto it = _collector.hosts().begin(); it != _collector.hosts().end(); ++it) {
    if (it != _collector.hosts().begin()) {
      out << ",";
    }

	const HostFlows& flows = (*it).second;
    out << endl << format("\"%1%|%2%\": [") % flows.host().ip % flows.host().hw;
	
    for (auto flow_it = flows.flows().begin(); flow_it != flows.flows().end(); ++flow_it) {
      if (flow_it != flows.flows().begin()) {
		out << ", ";
      }

	  const FlowSummary* summary = (*flow_it);

      out << format("[ [\"%1%\", \"%2%\"], [\"%3%\", \"%4%\", %5%], %6%, %7% ]")
		% summary->flow().src_hw % summary->flow().src_ip
		% summary->flow().dst_hw % summary->flow().dst_ip % summary->flow().dst_port
		% summary->bytes_src() % summary->bytes_dst();
    }

	out << "]";
  }
  
  out << "}" << endl;

  _rw_exclusion.reader_release();

  return out.str();
}
