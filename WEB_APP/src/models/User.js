const mongoose = require('mongoose');
const { Schema }= mongoose;

const UserSchema = new Schema({
    Name: { type: String, required:true },
    Email: { type: String, required:true },
    Password: { type: String, required:true },
    Zip: { type: Number},
    State: { type: String},
    City: { type: String},
    MAC: { type: String, required:true },
    Data: {},
    Date:{ type: Date, default: Date.now }
})

module.exports = mongoose.model('User', UserSchema)