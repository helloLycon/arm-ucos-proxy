

$(function(){
	// validate form
	$("#update_input").change(function(){
		function file_get_size(){
			var file = document.getElementById("update_input").files;
			return file[0].size;
		}
		function filename_crop(str){
			var src = 0;
			//console.log("fullpath = \""+str+"\"\n");
			for(var i=(str.length-1);i>=0;--i){
				if(str.charAt(i) == '\\'){
					src = i+1;
					break;
				}
			}
			console.log("\""+str+"\" => \""+str.slice(src)+"\"\n");
			return str.slice(src);
		}
		function filename_chk(str){
			if(str.length == 0){
				console.log("empty\n");
				return 1;
			}
			if(str.length > 60){
				console.log("filename length too long\n");
				return 2;
			}
			if(str.slice(str.length-4) != ".bin" ){
				console.log("must be \".bin\" file\n");
				return 3;
			}
			for(var i=0;i<(str.length-4);++i){
				if(str.charAt(i)>='a' && str.charAt(i)<='z')
					continue;
				if(str.charAt(i)>='A' && str.charAt(i)<='Z')
					continue;
				if(!isNaN(str.charAt(i)))
					continue;
				if('_' == str.charAt(i) || '-' == str.charAt(i))
					continue;
				console.log("filename wrong\n");
				return 4;
			}
			//filename = str;
			console.log("correct\n");
			return 0;
		}
		var short_name = filename_crop( $(this).val() );
		var res = filename_chk(short_name);
		if( 0 == res ){
			size_filename = file_get_size() + "\\" + short_name;
			console.log("submit-value = \""+size_filename+"\"\n");
			parent.mycheck('确认升级?' , function(){
				//$("#update_modal",parent.document).modal();
				parent.update_modal(true , 0);
				$.post('update.cgi' , 
				{
					filename: size_filename,
					end: 'NULL'
				},
				function(data,status){
					if('success' != status){
						parent.update_modal(false);
						parent.myprompt('服务器无响应!','red');
						return;
					}
					console.log('lowlevel start updating...\n');
					timer = setInterval(function(){
						$.post('update.cgi' ,{end:'NULL'}, function(data,status){
							console.log('data = '+data+'\n');
							if('success' != status){
								clearInterval(timer);
								parent.update_modal(false);
								parent.myprompt('服务器错误!','red');
								return;
							}
							if(data.match('success')){
								clearInterval(timer);
								parent.update_modal(false);
								//parent.myprompt('升级成功!');
								parent.update_reboot();
							}
							else if(data.match('fail')){
								clearInterval(timer);
								parent.update_modal(false);
								parent.myprompt('升级失败!','red');
							}
							else{
								parent.update_modal(true , getValue(data,'proc'));
							}
						});
					} , 2000);
				});				
			});
		}
		else{
			switch(res){
				case 1:break;  // empty
				case 2:
					parent.myprompt('文件名过长!','red');
					break;
				case 3:
					parent.myprompt('必须为bin文件!','red');
					break;
				case 4:
					parent.myprompt('文件名不能包含特殊字符!','red');
					break;
				default:
					parent.myprompt('未知错误','red');
			}
		}
	});
});
