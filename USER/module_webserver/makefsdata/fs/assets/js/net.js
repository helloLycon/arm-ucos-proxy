
function read_net_data(refresh){
	$.post('net.cgi' ,{end:'NULL'}, function(data,status){
//		console.log("data = "+data+"\nstatus = "+status+'\n');
		if( status != 'success' )
			parent.myprompt('提取数据失败!','red');
		else{
			// clear wrong status
			$("#dev_ip_group").removeClass('error');
			$("#dev_ip_help").html("");
			$("#dev_mask_group").removeClass('error');
			$("#dev_mask_help").html("");
			$("#dev_gateway_group").removeClass('error');
			$("#dev_gateway_help").html("");
			$("#mng_ip_group").removeClass('error');
			$("#mng_ip_help").html("");
			$('#net_submit').addClass('disabled');
			$('#net_submit').attr({disabled:'disabled'});
			// handle data
			$("#dev_ip_input").val(getValue(data,'dev_ip'));
			$("#dev_port_input").html(getValue(data,'dev_port'));
			$("#dev_mask_input").val(getValue(data,'dev_mask'));
			$("#dev_gateway_input").val(getValue(data,'dev_gateway'));
			$("#dev_mac_input").html(getValue(data,'dev_mac'));
			$("#mng_ip_input").val(getValue(data,'mng_ip'));
			$("#mng_port_input").html(getValue(data,'mng_port'));
			$("#dev_id_input").html(getValue(data,'dev_id'));
			$("#boot_ver_input").html(getValue(data,'boot_ver'));
			$("#app_ver_input").html(getValue(data,'app_ver'));
			if(refresh){
				parent.myprompt("提取数据成功!");
			}
		}
	});
}

$(function(){
	// read data from server first
	read_net_data(false);
	// refresh button
	$("#refresh_btn").click(function(){
		read_net_data(true);
	});
	// submit modification
	$("#net_submit").click(function(){
		$.post('net.cgi' , 
		{
			dev_ip:$("#dev_ip_input").val(),
			dev_mask:$("#dev_mask_input").val(),
			dev_gateway:$("#dev_gateway_input").val(),
			mng_ip:$("#mng_ip_input").val(),
			end:'NULL'
		},
		function(data,status){
//			console.log("data = "+data+'\n');
//			console.log('status = '+status+'\n');
			if('success'== status && data.match('success')){
				parent.myprompt('配置成功!');
			}
			else{
				parent.myprompt('配置失败!','red');
			}
			read_net_data(false);
		});
	});
	// form validation
	$("input").bind("input" , function(){
		function check_ip(str){
			if(str == "")
				return false;
			var regex = /^(\d+)\.(\d+)\.(\d+)\.(\d+)$/g;
			if(regex.test(str)){
				if(RegExp.$1>255 ||RegExp.$2>255 || RegExp.$3>255 || RegExp.$4>255)
					return false;
				if(RegExp.$1.length>3 ||RegExp.$2.length>3 ||RegExp.$3.length>3 ||RegExp.$4.length>3)
					return false;
				return true;
			}
			return false;
		}
		function check_host_ip(str){
			if(str == "")
				return false;
			var regex = /^(\d+)\.(\d+)\.(\d+)\.(\d+)$/g;
			if(regex.test(str)){
				//console.log(RegExp.$1.length);
				// 主机ip第一个字节在1~223
				if(RegExp.$1 == 0 || RegExp.$1 > 223)
					return false;
				if(RegExp.$2>255 || RegExp.$3>255 || RegExp.$4>255)
					return false;
				if(RegExp.$1.length>3 ||RegExp.$2.length>3 ||RegExp.$3.length>3 ||RegExp.$4.length>3)
					return false;
				return true;
				/*
				if((RegExp.$1>0 && RegExp.$1<224)&& RegExp.$2<256 && RegExp.$3<256 && RegExp.$4<256 )
					return true;
				*/
			}
			return false;
		}
		function generic_ip_checker(check_method,input_id,group_id,help_id,help_text){
			if(check_method( $("#"+input_id).val() )){
				$("#"+group_id).removeClass('error');
				$("#"+help_id).html("");
				return true;
			}
			else{
				$("#"+group_id).addClass('error');
				//$("#"+help_id).html(help_text);
				return false;
			}
		}
		// start check all input
		console.log("check net input...\n");
		var r1 = generic_ip_checker(check_host_ip,
		                            "dev_ip_input",
		                            "dev_ip_group",
		                            "dev_ip_help",
		                            "&#26080;&#25928;&#30340;&#35774;&#22791;IP&#22320;&#22336;");
		var r2 = generic_ip_checker(check_ip,
		                            "dev_mask_input",
		                            "dev_mask_group",
		                            "dev_mask_help",
		                            "&#26080;&#25928;&#30340;&#35774;&#22791;&#23376;&#32593;&#25513;&#30721;");
		var r3 = generic_ip_checker(check_ip,
		                            "dev_gateway_input",
		                            "dev_gateway_group",
		                            "dev_gateway_help",
		                            "&#26080;&#25928;&#30340;&#35774;&#22791;&#32593;&#20851;IP&#22320;&#22336;");
		var r4 = generic_ip_checker(check_ip,
		                            "mng_ip_input",
		                            "mng_ip_group",
		                            "mng_ip_help",
		                            "&#26080;&#25928;&#30340;&#32593;&#31649;&#20027;&#26426;IP&#22320;&#22336;");
		// 检查所有输入是否正确
		if(r1 && r2 && r3 && r4){
			console.log("correct...\n");   // debug
			$('#net_submit').removeClass('disabled');
			$('#net_submit').removeAttr('disabled');
		}
		else{
			console.log("WRONG...\n");  // debug
			$('#net_submit').addClass('disabled');
			$('#net_submit').attr({disabled:'disabled'});
		}
	});
});
