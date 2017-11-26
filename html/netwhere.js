var collector_ip;
var hosts;
var orgs = {};
var hosts_table;
var parts_table;
var networks_table;
var host_label;
var mac_to_vendor = {};
var hosts_info = {};

function format_bytes(a,b) {
	if (0==a) return"0";
	var c=1024,d=b||2,e=["Bytes","KB","MB","GB","TB","PB","EB","ZB","YB"],f=Math.floor(Math.log(a)/Math.log(c));
	return parseFloat((a/Math.pow(c,f)).toFixed(d))+" "+e[f]
}

function render_size(data, type, row, meta) {
	if (type == "display")
		return format_bytes(data);
	else
		return data;
}

function populate_host(host, hosts_table, networks_table, row) {
	var ip;
	var data_in;
	var data_out;
	var protocol;
	var port;

	if (row[0][0][1] == host.ip) {
		ip = row[0][1][1];
		data_in = row[1];
		data_out = row[2];
	} else {
		ip = row[0][0][1];
		data_in = row[2];
		data_out = row[1];
	}

	port = row[0][2];

	if (row[0][3] == 6)
		protocol = 'TCP';
	else if (row[0][3] == 17)
		protocol = 'UDP';
	else
		protocol = row[0][3];

	$.ajax({
        url: 'http://whois.arin.net/rest/ip/' + ip,
	    dataType: 'json',
	}).done(function( json ) {
		var org_handle = "";
	    if (json['net']['orgRef']) {
			org_name = json['net']['orgRef']["@name"];
			org_handle = json['net']['orgRef']["@handle"];

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
							 json['net']['name']['$'] || "",
							 org_handle,
							 port + '/' + protocol,
							 data_in,
							 data_out,
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
	hosts_table = $(hosts_table_id).DataTable(
		{ searching: true, columnDefs: [
		  {
			  targets: [4, 5],
			  render: render_size,
		  }
		]}
	);
	networks_table = $(networks_table_id).DataTable(
		{ searching: false,  columnDefs: [
		  {
			  targets: [1, 2],
			  render: render_size,
		  }
		]
		}
	);
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
				hostname : hosts[i][2],
				bytes_in : format_bytes(hosts[i][3]),
				bytes_out : format_bytes(hosts[i][4]),
			}
		}

		for (key in hosts_info) {
			html = "<div class='row'><div class='col-md'>"
			html += hosts_info[key].ip;
			if (hosts_info[key].hostname != hosts_info[key].ip) {
				html += ' - ' + hosts_info[key].hostname;
			}
			html += "</div></div>";
			html += "<div class='row'><div class='col-md'>";
			html +=	hosts_info[key].bytes_in + ', ' + hosts_info[key].bytes_out;
			html += '</div></div>';
			//html += "</div>";

			href = $( "<button/>", {
				class: "host",
				html: html,
				onclick: 'fill_tables(this, "' + key + '")'
			});
			hosts_info[key].html = href;

			hl = $('#hosts_list');
			hl.append(
				hosts_info[key].html
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
					if (host.mac.slice(0,8) == this.mac_prefix) {
						row = host.html.children()[1];
						$(row).append("<div class='col-md'>[" + vendor + "]</div>");
					}
				}
			}).fail(function( xhr, status, errorThrown ) {
				console.log(status)
			});
		}
	});
}
