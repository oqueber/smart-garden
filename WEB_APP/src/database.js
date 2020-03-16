const mongoose = require('mongoose');
mongoose.connect('mongodb://34.106.214.250/SG-db-app',{
    useCreateIndex: true,
    useNewUrlParser: true,
    useFindAndModify: false,
    useUnifiedTopology: true
}).then(db => console.log('DB is connected'))
  .catch( err => console.log('ERROR', err));