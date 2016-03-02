var express = require('express');
var router = express.Router();
var fs = require('fs');

/* GET home page. */
router.get('/', function(req, res, next) {
  //res.render('index', { title: 'Express' });
	 res.sendFile('weather.html', { root: 'public' 
	 });
	 });
 
router.get('/getcity',function(req,res,next) { 
var myRe = new RegExp("^" + req.query.q);
fs.readFile(__dirname + '/cities.dat.txt',function(err,data) { 
if(err) 
   throw err;
var cities = data.toString().split("\n"); 
var jsonresult = [];
for(var i = 0; i < cities.length; i++) {
	var result = cities[i].search(myRe);
	if(result != -1) { 
	//console.log(cities[i]); 
	jsonresult.push({city:cities[i]}); } 
	} 
//	console.log(jsonresult); 
	res.status(200).json(jsonresult);
});

 // console.log("In getcity route"); 
 
  //console.log(myRe);
}); 
	  

	  
router.get('/getcat',function(req,res,next) { 
var randNum = Math.floor(Math.random() * (4 - 1) + 1);
console.log(randNum);
var fileStream = fs.createReadStream(__dirname + '/../cats/cat' + randNum + '.jpg');
	fileStream.pipe(res);

});

router.get('/getcat/:id',function(req,res,next) { 	
	fs.createReadStream(__dirname + '/../cats/cat' + req.params.id + '.jpg').pipe(res);
});


module.exports = router;
