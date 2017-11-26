/*
 * Copyright (c) 2017, Ben Smith
 * All rights reserved.
 *
 */

#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include <functional>
#include <iostream>
#include <string>
#include <tuple>
#include <unordered_map>

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

  time_t now = time(nullptr);
  _collector.collect(now, pdu);

  if (now - _last_stats > _print_stats_interval) {
	print_stats();
	_last_stats = now;
  }
  if (now - _last_prune > _prune_interval) {
	_collector.prune(now - _prune_older_than);
	_last_prune = now;
  }

  return true;
}

string NetWhere::handle_request(const string& url) {
  size_t len = url.size();

  ReaderWriterExclusion::ReaderGuard guard(_rw_exclusion);

  if (len == 0) {
    return hosts();
  }

  size_t end = url.find_last_of('/');

  if (end == len-1 || end == string::npos)
    return hosts();
  else {
	size_t start;
	if (end == 0) {
	  end = url.find('#');
	  start = 1;
	  end = end > 0 ? end : len;
	} else {
	  start = url.find_last_of('/', end);
	}
	return host_flows(url.substr(start, end - start));
  }

  LOG(format("Invalid URL : %1%") % url);

  return "";
}

string NetWhere::hosts() {
  ostringstream out;

  out << "[";

  for (auto hflows = _collector.hosts().begin(); hflows != _collector.hosts().end(); ++hflows) {
    if (hflows != _collector.hosts().begin()) {
      out << ",";
    }

    out << format("[\"%1%\",\"%2%\",\"%3%\",\"%4%\",\"%5%\"]") % hflows->host().ip % hflows->host().hw
	  % get_hostname(hflows->host())
	  % hflows->bytes_in() % hflows->bytes_out();
  }

  out << "]" << endl;

  return out.str();
}

string NetWhere::host_flows(const string& host_key) {
  size_t dashpos = host_key.find('-');

  auto hflows = _collector.hosts().find(Host(host_key.substr(0, dashpos), host_key.substr(dashpos + 1)));

  if (hflows == _collector.hosts().end()) {
	throw runtime_error("Unknown host" + host_key);
  }

  ostringstream out;
  out << "[";

  auto flows = hflows->flows();

  for (auto flow_it = flows.begin(); flow_it != flows.end(); ++flow_it) {
	if (flow_it != flows.begin()) {
	  out << ", ";
	}

	const FlowCounter* summary = (*flow_it);

	out << format("[[[\"%1%\", \"%2%\"], [\"%3%\", \"%4%\"], %5%, %6%], %7%, %8% ]")
	  % summary->flow().src_hw % summary->flow().src_ip
	  % summary->flow().dst_hw % summary->flow().dst_ip % summary->flow().dst_port % (int)summary->flow().ip_protocol
	  % summary->bytes_to_src() % summary->bytes_to_dst();
  }

  out << "]" << endl;

  return out.str();
}

string NetWhere::get_hostname(const Host& host) {
	sockaddr_in address;
	char buf[260];
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = host.ip;
	int response = getnameinfo((sockaddr*)&address,
							sizeof(address),
							buf,
   						    sizeof(buf),
							nullptr,
							0,
							0);

	if (response == 0)
	  return buf;

	return "";
}
