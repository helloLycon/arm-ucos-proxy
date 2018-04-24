
$(function(){
//	function mygoto(url){
//		$.get(url , function(data,status){
//			document.write(data);
//		});
//	}
	delCookie('username');
	delCookie('passwd');
	$("#login_btn").mouseenter(function(){
		$(this).addClass('btn-primary');
		$("#login_icon").addClass("icon-white");
		//mygoto('main.html');
	}).mouseleave(function(){
		$(this).removeClass('btn-primary');
		$("#login_icon").removeClass("icon-white");
	});
	$("#login_btn").click(function(){
		if($("#username").val()=="" || $("#passwd").val()==""){
			myprompt('请填写用户名和密码!','red');
			return;
		}
		$.post('login.cgi' ,{end:'NULL'}, function(data,status){
			//console.log('data = '+ data+'\n');
			if( 'success' != status){
				myprompt('服务器错误!','red');
			}
			else{
				var regex,username,passwd;
/*				regex = /username=(.*);/;
				if(regex.test(data)){
					username = RegExp.$1;
					//console.log('username = '+username+'\n');
				}
				regex = /passwd=(.*);/;
				if(regex.test(data)){
					passwd = RegExp.$1;
					//console.log('passwd = '+passwd+'\n');
				}*/
				username = getValue(data,'username');
				passwd   = getValue(data,'passwd');
				if( $("#username").val()==username && $("#passwd").val()==passwd ){
					document.cookie = "username="+username;
					document.cookie = "passwd="+passwd;
					top.location = 'main.html';
				}
				else{
					myprompt('用户名或密码错误!','red');
				}
			}
		});
	});
});
