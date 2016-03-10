$(document).ready(function(){	
	$("#getspecific").click(function(){ 
		var data = {};
		data.input = $("#Searchname").val(); 
		jobj = JSON.stringify(data); 
		//$("#json").text(jobj); 	
		var url = "searchcomments"; 
		$.ajax({ 
			url:url, type: "POST", 
			data: jobj, contentType: "application/json; charset=utf-8",
			success: function(data,textStatus) { 
				//console.log(data);
				if(!data)
					$("#specificcomments").html('No comments found.');
				else{
				var everything = "<ul>"; 
				everything += "<li>Comment: " + data.Comment + "</li>"; 
				everything += "</ul>"; 
				$("#specificcomments").html(everything);
				}
			} 
		})	
		
	
		});
	});
 
