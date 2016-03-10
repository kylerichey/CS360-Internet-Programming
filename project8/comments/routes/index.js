var express = require('express');
var router = express.Router();

router.post('/comment', function(req, res, next) { 
	//console.log("POST comment route"); 
	//console.log(req.body);
	//res.sendStatus(200);
	
	
	 var newcomment = new req.db.Comment(req.body); 
	 console.log(newcomment); 
	 newcomment.save(function(err, post) {  
		if (err) return console.error(err); 
		console.log(post); 
		res.sendStatus(200); 
	});
 });
 
 
 /* GET comments from database */
 router.get('/comments', function(req, res, next) {
	 console.log("In the GET route"); 
	 req.db.Comment.find(function(err,commentList) { 
	 //Calls the find() method on your database 
		if (err) return next(err); //If there's an error, print it out 
		else { 
			console.log(commentList); //Otherwise console log the comments you found 
			} 
		res.json(commentList); //Then send them
	}) 
});


router.post('/searchcomments', function(req, res, next) { 
	console.log("search post comment route"); 
	var input = req.body.input;
	 req.db.Comment.findOne({'Name': input},'Comment',function(err,commentList) { 
	 //Calls the find() method on your database 
		if (err) console.log(err); //If there's an error, print it out 
		else { 
			console.log(commentList); //Otherwise console log the comments you found 
			} 
		res.json(commentList); //Then send them
	})
	

	
	
	
 });

 
 

module.exports = router;
