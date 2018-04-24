function alphaton(alpha){
	if(!isNaN(alpha))
		return alpha-'0';
	if( alpha>='a' && alpha<='z' )
		return alpha-'a'+10;
	if(alpha>='A' && alpha<='Z')
		return alpha-'A'+36;
	return 0;
}

function opt_name(i,msg){
	return 'OPT_'+(i+1);
}
function opt_pos(i,msg){
	return (alphaton(msg[0])+1)+'_OPT_'+(alphaton(msg[1])+1);
}
function opt_switch(i,msg){
	var cfg = parseInt(msg.slice(2,4),16);
	//console.log(cfg);
	if(cfg & (1<<7))
		return '启用';
	else
		return '关断';
}
function opt_loop(i,msg){
	var cfg = parseInt(msg.slice(2,4),16);
	if((cfg&(1<<1))&&(cfg&(1<<2)))
		return '双向环回';
	if(cfg &(1<<1))
		return '线路环回';
	if(cfg &(1<<2))
		return '设备环回';
	return '正常工作';
}
function opt_trap(i,msg){
	var cfg = parseInt(msg.slice(2,4),16);
	if(cfg & (1<<0))
		return '抑制告警上报';
	else
		return '允许告警上报';
}
function opt_LOF(i,msg){
	var st = parseInt(msg.slice(4,6),16);
	if(st&(1<<0))
		return '告警';
	else
		return '正常';
}
function opt_NOP(i,msg){
	var st = parseInt(msg.slice(4,6),16);
	if(st&(1<<1))
		return '告警';
	else
		return '正常';
}
function opt_LOOP(i,msg){
	var st = parseInt(msg.slice(4,6),16);
	if(st&(1<<2))
		return '告警';
	else
		return '正常';
}
function opt_btn(i,msg){
	return "<button id=\"btn"+i+"\" type=\"button\" class=\"btn btn-inverse btn-mini\">配置</button>";
}
opt_col_arr = new Array(
	opt_name,
	opt_pos,
	opt_switch,
	opt_loop,
	opt_trap,
	opt_LOF,
	opt_NOP,
	opt_LOOP,
	opt_btn
);

function opt_read_data(refresh){
	function opt_parse_one_port(line , msg){
		function fill_td(str){
			return '<td>'+str+'</td>';
		}
		$("#tbody").append("<tr id=\'tr"+line+"\'></tr>");
		for(var c=0;c<opt_col_arr.length;c++){
			$("#tr"+line).append(fill_td(opt_col_arr[c](line,msg)));
		}
	}
	$("#tbody").empty();
	$.post('opt.cgi' ,{end:'NULL'}, function(data,status){
		if('success' != status || data.match('fail')){
			parent.myprompt('提取数据失败!' , 'red');
			return;
		}
		//test data...
		//data = "\n<!--#opt_read_data-->00010301ff03\n";
		save_data = data;
		port_len = 6;  //一个端口的帧长度
		data_start = data.indexOf('-->')+3;
		port_no = parseInt((data.length-data_start)/port_len);
		for(var line=0;line<port_no;line++){
			opt_parse_one_port(line,data.slice(data_start+line*port_len,data_start+(line+1)*port_len));
		}
		// OPT CONFIG RELATIVE
		for(var btn=0;btn<port_no;btn++){
			$("#btn"+btn).attr('onclick','local_opt_modal('+btn+')');
		}
		if(refresh)
			parent.myprompt('提取数据成功!');
	});
}

$(function(){
	$("#opt_refresh").click(function(){
		opt_read_data(true);
	});
	opt_read_data();
});

function local_opt_modal(line){
	var msg = save_data.slice(data_start+line*port_len,data_start+(line+1)*port_len);
	parent.opt_modal(
		alphaton(msg[0]),
		alphaton(msg[1]),
		opt_name(line,msg),
		opt_switch(line,msg),
		opt_trap(line,msg),
		opt_loop(line,msg)
	);
}
