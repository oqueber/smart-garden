const mongoose = require('mongoose');
mongoose.Promise = require('bluebird');

const url = 'mongodb://localhost/SG-db-app';
const connect= mongoose.connect(url ,{
    useNewUrlParser: true,
    useUnifiedTopology: true,
    useFindAndModify: false
});

module.exports = connect;
