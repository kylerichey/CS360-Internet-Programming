var express = require('express');
var path = require('path');
var favicon = require('serve-favicon');
var logger = require('morgan');
var cookieParser = require('cookie-parser');
var bodyParser = require('body-parser');

var routes = require('./routes/index');
var users = require('./routes/users');

var app = express();

// view engine setup
app.set('views', path.join(__dirname, 'views'));
app.set('view engine', 'jade');

// uncomment after placing your favicon in /public
//app.use(favicon(path.join(__dirname, 'public', 'favicon.ico')));
app.use(logger('dev'));
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: false }));
app.use(cookieParser());
app.use(express.static(path.join(__dirname, 'public')));



 /* Set up mongoose in order to connect to mongo database */ 
 var mongoose = require('mongoose');  //Adds mongoose as a usable dependency 
 mongoose.connect('mongodb://localhost/commentDB'); //Connects to a mongo database called "commentDB" 
 var commentSchema = mongoose.Schema({ //Defines the Schema for this database 
	Name: String, 
	Comment: String 
}); 
var Comment = mongoose.model('Comment', commentSchema); //Makes an object from that schema as a model 
var connection = mongoose.connection; //Saves the connection as a variable to use 
connection.on('error', console.error.bind(console, 'connection error:')); //Checks for connection errors 
connection.once('open', function() { //Lets us know when we're connected 
	console.log('Connected'); 
});	
var db = {
	Comment: Comment
}

app.use(function(req,res,next){
	req.db = db;
	next()
})

app.use('/', routes);
app.use('/users', users);

// catch 404 and forward to error handler
app.use(function(req, res, next) {
  var err = new Error('Not Found');
  err.status = 404;
  next(err);
});

// error handlers

// development error handler
// will print stacktrace
if (app.get('env') === 'development') {
  app.use(function(err, req, res, next) {
    res.status(err.status || 500);
    res.render('error', {
      message: err.message,
      error: err
    });
  });
}

// production error handler
// no stacktraces leaked to user
app.use(function(err, req, res, next) {
  res.status(err.status || 500);
  res.render('error', {
    message: err.message,
    error: {}
  });
});


module.exports = app;
