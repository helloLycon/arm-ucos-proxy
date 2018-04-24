function alphaton(alpha){
	if(!isNaN(alpha))
		return alpha-'0';
	if( alpha>='a' && alpha<='z' )
		return alpha-'a'+10;
	if(alpha>='A' && alpha<='Z')
		return alpha-'A'+36;
	return 0;
}

function eth_board_name(ge,i,msg){
	return "";
}
function eth_name(ge,i,msg){
	if('G'==ge)
		return 'GTH_'+(i+1);
	else
		return 'ETH_'+(i+1);
}
function eth_pos(ge,i,msg){
	if('G'==ge)
		return (alphaton(msg[0])+1)+'_GTH_'+(alphaton(msg[1])+1);
	else
		return (alphaton(msg[0])+1)+'_ETH_'+(alphaton(msg[1])+1);
}
function eth_type(ge,i,msg){
	return "";
}
function eth_switch(ge,i,msg){
	var cfg = parseInt(msg.slice(2,6),16);
	if(cfg & (1<<15))
		return '启用';
	else
		return '关断';
}
function eth_link(ge,i,msg){
	var cfg = parseInt(msg.slice(2,6),16);
	if(cfg & (1<<7))
		return '已连接';
	else
		return '未连接';
}
function eth_work_mode(ge,i,msg){
	var cfg = parseInt(msg.slice(2,6),16);
	if(cfg & (1<<3))
		return '自动协商';
	else
		return '强制';
}
function eth_speed(ge,i,msg){
	var cfg = parseInt(msg.slice(2,6),16);
	switch((cfg&(3<<5))>>5){
		case 0:
			return '10M';
		case 1:
			return '100M';
		case 2:
			return '1000M';
		default:
			return "";
	}
}
function eth_duplex(ge,i,msg){
	var cfg = parseInt(msg.slice(2,6),16);
	if(cfg & (1<<4))
		return '全双工';
	else
		return '半双工';
}
function eth_fctrl(ge,i,msg){
	var cfg = parseInt(msg.slice(2,6),16);
	if(cfg & (1<<14))
		return '开启';
	else
		return '关闭';
}
function eth_trap(ge,i,msg){
	var cfg = parseInt(msg.slice(2,6),16);
	if(cfg & (1<<13))
		return '抑制告警上报';
	else
		return '允许告警上报';
}
function eth_btn(ge,i,msg){
	return "<button id=\"btn"+ge+i+"\" type=\"button\" class=\"btn btn-inverse btn-mini\">配置</button>";
}
eth_col_arr = new Array(
	eth_board_name,
	eth_name,
	eth_pos,
	eth_type,
	eth_switch,
	eth_link,
	eth_work_mode,
	eth_speed,
	eth_duplex,
	eth_fctrl,
	eth_trap,
	eth_btn
);

function eth_read_data(refresh){
	function eth_parse_one_port(ge,line , msg){
		function fill_td(str){
			return '<td>'+str+'</td>';
		}
		$("#tbody").append("<tr id=\'tr"+ge+line+"\'></tr>");
		for(var c=0;c<eth_col_arr.length;c++){
			$("#tr"+ge+line).append(fill_td(eth_col_arr[c](ge,line,msg)));
		}
	}
	$("#tbody").empty();
	$.post('eth.cgi' ,{end:'NULL'}, function(data,status){
		if('success' != status || data.match('fail')){
			parent.myprompt('提取数据失败!' , 'red');
			return;
		}
		//test data...
		//data = "\n<!--#eth_read_data-->00010301ff03\n";
		save_data = data;
		port_len = 6;  //一个端口的帧长度
		data_start = data.indexOf('-->')+3;
		fe_start = data.indexOf('_FE_')+4;
		ge_port_no = parseInt((data.indexOf('_FE_')-data.indexOf('-->')-3)/port_len);
		fe_port_no = parseInt((data.length-data.indexOf('_FE_')-4)/port_len);
//		console.log(data);
//		console.log('ge_start = '+data_start);
//		console.log('fe_start = '+fe_start);
//		console.log('ge_port_no = '+ge_port_no);
//		console.log('fe_port_no = '+fe_port_no);
		for(var line=0;line<ge_port_no;line++){
			eth_parse_one_port('G',line,data.slice(data_start+line*port_len,data_start+(line+1)*port_len));
		}
		for(var line=0;line<fe_port_no;line++){
			eth_parse_one_port('F',line,data.slice(fe_start+line*port_len,fe_start+(line+1)*port_len));
		}
		// eth CONFIG RELATIVE
		for(var btn=0;btn<ge_port_no;btn++){
			$("#btnG"+btn).attr('onclick',"local_eth_modal(\'G\',"+btn+')');
		}
		for(var btn=0;btn<fe_port_no;btn++){
			$("#btnF"+btn).attr('onclick',"local_eth_modal(\'F\',"+btn+')');
		}
		if(refresh)
			parent.myprompt('提取数据成功!');
	});
}

$(function(){
	$("#eth_refresh").click(function(){
		eth_read_data(true);
	});
	eth_read_data();
});

function local_eth_modal(ge,line){
	var msg;
	if(ge == 'G')
		msg = save_data.slice(data_start+line*port_len,data_start+(line+1)*port_len);
	else
		msg = save_data.slice(fe_start+line*port_len,fe_start+(line+1)*port_len);	
	parent.eth_modal(
		ge,
		alphaton(msg[0]),
		alphaton(msg[1]),
		eth_name(ge,line,msg),
		eth_switch(ge,line,msg),
		eth_work_mode(ge,line,msg),
		eth_speed(ge,line,msg),
		eth_duplex(ge,line,msg),
		eth_fctrl(ge,line,msg),
		eth_trap(ge,line,msg)
	);
}
