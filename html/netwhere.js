var hosts;
var orgs = {};
var hosts_table;
var networks_table;
var host_label;
var mac_to_vendor = {};
var hosts_info = {};

function populate_host(host, hosts_table, networks_table, row) {
	var ip;
	var data_in;
	var data_out;
	
	if (row[0][1] == host.ip) {
		ip = row[1][1];
		data_in = row[2];
		data_out = row[3];
	} else {
		ip = row[0][1];
		data_in = row[3];
		data_out = row[2];
	}

	$.ajax({
        url: 'http://whois.arin.net/rest/ip/' + ip,
	    dataType: 'json',
	}).done(function( json ) {
	    var org_name = "";

	    if (json['net']['orgRef']) {
			org_name = json['net']['orgRef']["@name"];
			var org_handle = json['net']['orgRef']["@handle"];

			if (!(org_handle in orgs)) {
				new_row = networks_table.row.add([org_name + ' (' + org_handle + ')',0,0]);
				orgs[org_handle] = new_row;
			}

			new_row = orgs[org_handle];
			data = new_row.data();
			data[1] += data_in;
			data[2] += data_out;
			new_row.data(data).draw();
	    }
		
	    hosts_table.row.add([ip,
							 json['net']['name']['$'],
							 org_name,
							 data_in,
							 data_out
						   ]).draw();
    });
}

function fill_tables(elem, host) {
    $('.current_selection').removeClass("current_selection"); 
    $(elem).addClass("current_selection");
	hosts_table.clear();
	networks_table.clear();
	orgs = {};

	var parts = host.split("|");
	var mac_prefix = parts[1].slice(0,8);
	
	host_label.html(parts[0] + ' - ' + parts[1] +  ' [' + mac_to_vendor[mac_prefix] + ']');
		
	hosts[host].forEach( function(row) {
		populate_host(hosts_info[host], hosts_table, networks_table, row);
	});
}

function get_flows(collector_ip, hosts_table_id, networks_table_id, host_label_id) {
	hosts_table = $(hosts_table_id).DataTable({ searching: false });
	networks_table = $(networks_table_id).DataTable({ searching: false });
	host_label = $(host_label_id);
	
	$.ajax({
	    url: 'http://' + collector_ip,
	    dataType: 'json',
	}).done(function( json ) {
	    hosts = json;
	    for (host in hosts) {
			if (!host.startsWith("192.168.0")) {
				continue;
			}

			var parts = host.split("|");
			var mac = parts[1].slice(0,8);
			if (mac != 'ff:ff:ff' && ! (mac in mac_to_vendor) ) {
				mac_to_vendor[mac] = null;
			}

			hosts_info[host] = {
				ip : parts[0],
				mac : parts[1]
			}
		}

		for (mac in mac_to_vendor) {
			$.ajax({
				url: 'http://api.macvendors.com/' + mac,
				context: { mac : mac }
			}).done(function( vendor ) {
				mac_to_vendor[this.mac] = vendor;
				for (key in hosts_info) {
					host = hosts_info[key];
					if (host.mac.slice(0,8) == this.mac)
						host.href.html(host.href.html() + ' [' + vendor + ']');
				}
			}).fail(function( xhr, status, errorThrown ) {
				console.log(status)
			});
		}

		for (key in hosts_info) {
			hosts_info[key].href = $( "<a/>", {
				href: "#",
				html: hosts_info[key].ip,
				onclick: 'fill_tables(this, "' + key + '")'
			});
				
			$('#hosts_list').append(
				hosts_info[key].href
			);
		}
	});
}
