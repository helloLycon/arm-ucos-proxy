
var tim_var;

/* DRAW LEDS */
function draw_led(retry){
	function draw_all_led(led_str){
		function draw_single_led(id , on){
			var color = document.getElementById('panel_svg').getSVGDocument().getElementById(id).getAttribute('set');
			if(on)
				document.getElementById('panel_svg').getSVGDocument().getElementById(id).setAttribute('fill',color);
			else
				document.getElementById('panel_svg').getSVGDocument().getElementById(id).setAttribute('fill','white');
		}
		for(var i=0;i<16;i++){
			if('1' == led_str[i])
				draw_single_led('led'+i , true);
			else
				draw_single_led('led'+i , false);
		}	
	}
	$.post('panel.cgi',{end:'NULL'},function(data,status){
		if( 'success'!=status || data.match('fail') ){
			console.log('read panel failed...');
			draw_all_led('00000000000000000000000000000000000000000000000000000000');
			if(retry){
				setTimeout(function(){
					draw_led(false);
				} , 1000);
			}
			return;
		}
		draw_all_led(data.slice(data.indexOf('-->')+3));
	});
}

function panel_auto_refresh(sec){
	if(sec>0){
		tim_var = setInterval(function(){
			//console.log('refresh...\n');
			//$("#panel_svg").attr('data','assets/panel/gan111.svg');
			draw_led(false);
		} , sec*1000);
	}
	else
		clearInterval(tim_var);
}

function panel_stop_refresh(){
	clearInterval(tim_var);
}


$(function(){
	var cookie_period = getCookie('refresh');
	switch(cookie_period){
		case '0':
		case '5':
		case '10':
		case '30':
		case '60':
			//console.log('refresh cookie OK\n');
			r_period = parseInt(cookie_period);
			break;
		// default value
		default:
			//console.log('no valid cookie_period\n');
			r_period = 30;
	}
	panel_auto_refresh(r_period);
	switch(r_period){
		case 0:
		default:
			$("#panel_select").val('不刷新');
			break;
		case 5:
			$("#panel_select").val('5秒');
			break;
		case 10:
			$("#panel_select").val('10秒');
			break;
		case 30:
			$("#panel_select").val('30秒');
			break;
		case 60:
			$("#panel_select").val('1分钟');
			break;
	}
	$("#panel_select").change(function(){
		clearInterval(tim_var);
		var refresh_set;
		switch($("#panel_select").val()){
			case '不刷新':
				refresh_set = 0;
				break;
			case '5秒':
				refresh_set = 5;
				break;
			case '10秒':
				refresh_set = 10;
				break;
			case '30秒':
				refresh_set = 30;
				break;
			case '1分钟':
				refresh_set = 60;
				break;
		}
		panel_auto_refresh(refresh_set);
		document.cookie = "refresh="+refresh_set;
		//parent.myprompt('配置成功!');
		myprompt('配置成功!');
	});
	setTimeout(function(){
		draw_led(true);
	} , 1000);
});
