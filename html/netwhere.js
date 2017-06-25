var collector_ip;
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

	if (row[0][0][1] == host.ip) {
		ip = row[0][1][1];
		data_in = row[1];
		data_out = row[2];
	} else {
		ip = row[0][0][1];
		data_in = row[2];
		data_out = row[1];
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
							 org_handle,
							 data_in,
							 data_out
						   ]).draw();
    });
}

function fill_tables(elem, host_key) {
    $('.current_selection').removeClass("current_selection");
    $(elem).addClass("current_selection");
	hosts_table.clear();
	networks_table.clear();
	orgs = {};

	var host = hosts_info[host_key]
	var mac_prefix = host.mac.slice(0,8);

	var label = host.ip;
	if (host.hostname != host.ip) {
		label += ' - ' + host.hostname;
	}
	label += ' [' + mac_to_vendor[mac_prefix] + ']';
	host_label.html(label);

	$.ajax({
	    url: 'http://' + collector_ip + '/' + host_key,
	    dataType: 'json',
	}).done(function( json ) {
		json.forEach( function(row) {
			populate_host(host, hosts_table, networks_table, row);
		});
	});
}

function get_flows(ip, hosts_table_id, networks_table_id, host_label_id) {
	collector_ip = ip;
	hosts_table = $(hosts_table_id).DataTable({ searching: true });
	networks_table = $(networks_table_id).DataTable({ searching: false });
	host_label = $(host_label_id);

	$.ajax({
	    url: 'http://' + collector_ip,
	    dataType: 'json',
	}).done(function( json ) {
	    hosts = json;
	    for (i=0;i < hosts.length;i++) {
			var mac_prefix = hosts[i][1].slice(0,8);

			if (mac_prefix != 'ff:ff:ff' && ! (mac_prefix in mac_to_vendor) ) {
				mac_to_vendor[mac_prefix] = null;
			}

			hosts_info[hosts[i][1] + '-' + hosts[i][0]] = {
				mac : hosts[i][1],
				ip : hosts[i][0],
				hostname : hosts[i][2]
			}
		}

		for (key in hosts_info) {
			var html = hosts_info[key].ip;
			if (hosts_info[key].hostname != hosts_info[key].ip) {
				html += ' - ' + hosts_info[key].hostname;
			}

			hosts_info[key].href = $( "<a/>", {
				href: "#",
				html: html,
				onclick: 'fill_tables(this, "' + key + '")'
			});

			$('#hosts_list').append(
				hosts_info[key].href
			);
		}

		for (mac_prefix in mac_to_vendor) {
			$.ajax({
				url: 'http://api.macvendors.com/' + mac_prefix,
				context: { mac_prefix : mac_prefix }
			}).done(function( vendor ) {
				mac_to_vendor[this.mac_prefix] = vendor;
				for (key in hosts_info) {
					host = hosts_info[key];
					if (host.mac.slice(0,8) == this.mac_prefix)
						host.href.html(host.href.html() + '<div style="text-align:right">[' + vendor + ']</div>');
				}
			}).fail(function( xhr, status, errorThrown ) {
				console.log(status)
			});
		}
	});
}
