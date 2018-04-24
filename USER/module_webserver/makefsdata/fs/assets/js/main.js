
function cent_rmt_init(){
	function cent_rmt_read(){
		$.post('cent_rmt.cgi',{end:'NULL'},function(data,status){
			//console.log('data = '+data+'\n');
			if('success' == status && data.match('remote')){
				$("#cent_rmt").val('远端');
			}
			else{
				$("#cent_rmt").val('局端');
			}
		});
	}
	cent_rmt_read();
	$("#cent_rmt").change(function(){
		$.post('cent_rmt.cgi',
		{
			cent_rmt:$("#cent_rmt").val(),
			end:'NULL'
		},
		function(data,status){
			if('success' != status)
				myprompt('配置失败!' , 'red');
			else{
				var src;
				src = $("#myiframe").attr('src');
				$("#myiframe").attr('src' , src);
//				src = $("#panel_iframe").attr('src');
//				$("#panel_iframe").attr('src' , src);
				myprompt('配置成功!');
				draw_led();  // refresh panel right now
			}
		});
	});
}


function reboot_init(){
	$("#reboot_btn").mouseenter(function(){
		this.className = "btn btn-danger";
	}).mouseleave(function(){
		this.className = "btn btn-inverse";
	});
	$("#reboot_btn").click(function(){
		mycheck('确定重启?' , function(){
			$.post('reboot.cgi',
			{
				reboot:'reboot',
				end:'NULL'
			},
			function(data,status){
				if(status == 'success'){
					document.onmousedown = function(){return false};
					document.onmouseup = function(){return false};
					document.onclick = function(){return false};
					document.oncontextmenu = function(){return false};
					document.onkeydown = function(){return false};
					document.onkeyup = function(){return false};
					document.onkeypress = function(){return false};
					//document.getElementById('panel_iframe').contentWindow.panel_stop_refresh();
					panel_stop_refresh();
					$("#reboot_modal").modal({
						keyboard: false,
						backdrop: 'static'
					});
					// start countdown
					cnter = 20;
					$("#reboot_text").html('倒计时：'+(cnter--));
					setInterval(function(){
						$("#reboot_text").html('倒计时：'+(cnter--));
						if(0 == cnter){
							delCookie('passwd');
							top.location = 'index.html';
						}
					} , 1000);
				}
				else
					myprompt('重启失败!' , 'red');
			});			
		});
	});
}


function nav_init(){
	function nav_active(selector){
		//console.log('nav_active called');
		$(".accordion-menu,.accordion-submenu").removeAttr('active').removeAttr('style');
		$(".text-menu,.text-submenu").removeAttr('style');
		selector.attr('active' , 'true').attr('style' , "background-color: #0480BE;");
		selector.find('.text-menu,.text-submenu').attr('style' , "color: white;");
		$("#myiframe").attr('src',selector.attr('href'));
	}
	/* expand/collapse */
	$("#collapseTwo").on('shown' , function(){
		$("#nav_sys_icon").attr('class' , 'icon-chevron-up');
	});
	$("#collapseTwo").on('hidden' , function(){
		$("#nav_sys_icon").attr('class' , 'icon-chevron-down');
	});
	$("#collapseFour").on('shown' , function(){
		$("#nav_port_icon").attr('class' , 'icon-chevron-up');
	});
	$("#collapseFour").on('hidden' , function(){
		$("#nav_port_icon").attr('class' , 'icon-chevron-down');
	});
	/* mouseenter/mouseleave */
	$(".accordion-submenu,.accordion-menu").mouseenter(function(){
		if( $(this).attr('active') == 'true' )
			return;
		$(this).attr('style' , "background-color: #EDEDED;");
		$(this).find('.text-submenu,.text-menu').attr('style' , "color: #005580");
	});
	$(".accordion-submenu,.accordion-menu").mouseleave(function(){
		if( $(this).attr('active') == 'true' )
			return;
		$(this).removeAttr('style');
		$(this).find('.text-submenu,.text-menu').removeAttr('style');
	});
	/* click and activate */
	$(".accordion-menu,.accordion-submenu").click(function(){
		nav_active($(this));
	});
	/* INITIALIZATION */
	nav_active($("#nav_account"));
}

$(function(){
	$.post('login.cgi' ,{end:'NULL'}, function(data,status){
		var regex;
		var cook_user,cook_passwd,username,passwd;
/*		regex = /username=(.*);/;
		if(regex.test(data)){
			username = RegExp.$1;
		}
		regex = /passwd=(.*);/;
		if(regex.test(data)){
			passwd = RegExp.$1;
		}*/
		username = getValue(data, 'username');
		passwd   = getValue(data ,'passwd');
		cook_user = getCookie('username');
		cook_passwd = getCookie('passwd');
		if( cook_user!=username || cook_passwd!=passwd ){
			myprompt('请先登录!' , 'red',function(){
				top.location = 'index.html';
			});
		}
		else{
			$("#account_name").html(username);
		}
	});
	cent_rmt_init();
	reboot_init();
	nav_init();
	var ifm = document.getElementById("myiframe");
	ifm.height = document.documentElement.clientHeight-215;
//	document.getElementById('myiframe').height = 700;
/*	$(document.getElementById('myiframe').contentDocument).ready(function(){
		var pTar = document.getElementById('myiframe');
		if(pTar.contentDocument && pTar.contentWindow.body.offsetHeight)
			pTar.height = pTar.contentWindow.document.body.offsetHeight;
		else
			pTar.height = pTar.contentWindow.document.body.scrollHeight;		
	});*/
/*	$("#myiframe").load(function(){
		var mainheight = $(this).contents().find('body').height()+30;
		$(this).height(mainheight);
	});*/
});

/* for update */
function update_modal(show , proc){
	if(show){
		$("#update_proc").attr('style' , "width: "+proc+"%;");
		$("#update_modal").modal({
			keyboard: false,
			backdrop: 'static',
		});
	}
	else{
		$("#update_modal").modal('hide');
	}
}
function update_reboot(){
	document.onmousedown = function(){return false};
	document.onmouseup = function(){return false};
	document.onclick = function(){return false};
	document.oncontextmenu = function(){return false};
	document.onkeydown = function(){return false};
	document.onkeyup = function(){return false};
	document.onkeypress = function(){return false};
	//document.getElementById('panel_iframe').contentWindow.panel_stop_refresh();
	panel_stop_refresh();
	$("#reboot_modal").modal({
		keyboard: false,
		backdrop: 'static'
	});
	// start countdown
	cnter = 60;
	$("#reboot_title").html('升级成功/等待重启');
	$("#reboot_text").html('倒计时：'+(cnter--));
	setInterval(function(){
		$("#reboot_text").html('倒计时：'+(cnter--));
		if(0 == cnter){
			delCookie('passwd');
			top.location = 'index.html';
		}
	} , 1000);
}
/* for opt_config */
function opt_modal(brd,prt,title,sw,trap,loop){
	$("#opt_title").html(title);
	$("#opt_sw").val(sw);
	$("#opt_trap").val(trap);
	$("#opt_loop").val(loop);
	$("#opt_yes").unbind('click').click(function(){
		$.post('opt.cgi',
		{
			board: brd,
			portno : prt,
			opt_switch: $("#opt_sw").val(),
			opt_trap_mask:$("#opt_trap").val(),
			opt_loop: $("#opt_loop").val(),
			end:'NULL'
		},
		function(data,status){
			//console.log(data);
			if('success'==status && data.match('success'))
				myprompt('配置成功!');
			else
				myprompt('配置失败!','red');
			document.getElementById('myiframe').contentWindow.opt_read_data();
		});
	});
	$("#opt_modal").modal({
		keyboard: false,
		backdrop: 'static',
	});
}
/* for eth_config */
function eth_modal(ge,brd,prt,title,sw,mode,speed,duplex,fctrl,trap){
	if(ge == 'G'){
		if($("#eth_speed").html().match('1000M')){
			//console.log('1000M exist');
		}
		else{
			//console.log('append 1000M');
			$("#eth_speed").append("<option>1000M</option>");
		}
	}
	else{
		if($("#eth_speed").html().match('1000M')){
			//console.log('remove 1000M');
			$("#eth_speed option:last").remove();
		}
	}
	$("#eth_title").html(title);
	$("#eth_sw").val(sw);
	$("#eth_mode").val(mode);
	$("#eth_speed").val(speed);
	$("#eth_duplex").val(duplex);
	$("#eth_fctrl").val(fctrl);
	$("#eth_trap").val(trap);
	$("#eth_yes").unbind('click').click(function(){
		$.post('eth.cgi',
		{
			ge: ge,
			board: brd,
			portno : prt,
			eth_switch: $("#eth_sw").val(),
			eth_auto_nego: $("#eth_mode").val(),
			eth_bandwidth: $("#eth_speed").val(),
			eth_duplex: $("#eth_duplex").val(),
			eth_flow_ctrl: $("#eth_fctrl").val(),
			eth_trap_mask:$("#eth_trap").val(),
			end:'NULL'
		},
		function(data,status){
			if('success'==status && data.match('success'))
				myprompt('配置成功!');
			else
				myprompt('配置失败!','red');
			document.getElementById('myiframe').contentWindow.eth_read_data();
		});
	});
	$("#eth_modal").modal({
		keyboard: false,
		backdrop: 'static',
	});
}
/* for e1_config */
function e1_modal(brd,prt,title,sw,trap,loop){
	$("#e1_title").html(title);
	$("#e1_sw").val(sw);
	$("#e1_trap").val(trap);
	$("#e1_loop").val(loop);
	$("#e1_yes").unbind('click').click(function(){
		$.post('e1.cgi',
		{
			board: brd,
			portno : prt,
			e1_switch: $("#e1_sw").val(),
			e1_trap_mask:$("#e1_trap").val(),
			e1_loop: $("#e1_loop").val(),
			end:'NULL'
		},
		function(data,status){
			//console.log(data);
			if('success'==status && data.match('success'))
				myprompt('配置成功!');
			else
				myprompt('配置失败!','red');
			document.getElementById('myiframe').contentWindow.e1_read_data();
		});
	});
	$("#e1_modal").modal({
		keyboard: false,
		backdrop: 'static',
	});
}
