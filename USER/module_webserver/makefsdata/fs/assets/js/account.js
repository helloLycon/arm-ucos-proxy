
$(function() {
	// 表单验证
	$("input").bind("input" , function (){
		// check username/passwd
		function check_username_passwd(str){
			// check single character
			function check_single(c){
				if(c>='a' && c<='z')
					return true;
				if(c>='A' && c<='Z')
					return true;
				if(!isNaN(c))
					return true;
				/*if( '_' == c )
					return true;*/
				return false;
			}
			var len = str.length;
			if(len < 4)
				return false;
			for(var i=0;i<len;++i){
				if( false == check_single(str.charAt(i)) )
					return false;
			}
			return true;
		}
		var r1 = ( $("#old_user").val() != "" );
		var r2 = ( $("#old_passwd").val() != "" );
		var r3 = check_username_passwd( $("#new_user").val() );
		var r4 = check_username_passwd( $("#new_passwd").val() );
		var r5 = ( $("#new_passwd").val() == $("#new_passwd_cfm").val() );
		if( r1 && r2 && r3 && r4 && r5 ){
			console.log("OK\n");
			//$("#account_btn").attr('type' , 'submit');
			$("#account_btn").removeClass('disabled').removeAttr('disabled');
		}
		else{
			//console.log("No\n");
			//$("#account_btn").attr('type' , 'button');
			$("#account_btn").addClass('disabled').attr({disabled:'disabled'});
		}
	});
	// 提交确认
	$("#account_btn").click(function(){
		//var cfm = confirm('Sure to Modify Username/Password?');
		parent.mycheck('确认修改用户名/密码?' , function(){
			$.post(
				'account.cgi',
				{
					cur_user:$("#old_user").val(),
					cur_passwd:$("#old_passwd").val(),
					new_user:$("#new_user").val(),
					new_passwd:$("#new_passwd").val(),
					new_passwd_cfm:$("#new_passwd_cfm").val(),
					end:'NULL'
				},
				function(data,status){
//					console.log("data = "+data+"\nstatus = "+status+'\n');
					if( status != 'success' ){
						parent.myprompt('配置失败!' , 'red');
					}
					else{
						if( data.match('success') ){
							parent.myprompt('配置成功!' , 'black',function(){
								delCookie('passwd');
								top.location = 'index.html';								
							});
						}
						else if(data.match('incorrect'))
							parent.myprompt('请输入正确的用户名/密码!' ,'red');
						else
							parent.myprompt('配置失败!' , 'red');
					}
				}
			);			
		});
	});
});

