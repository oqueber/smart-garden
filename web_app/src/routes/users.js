const express = require('express');
const router = express.Router();
const User = require('../models/User')

router.get('/users', (req,res)=>{
    res.render('users/users');
});

router.get('/users/signin', (req,res)=>{
    res.render('users/signin');
});


router.get('/users/signup', (req,res)=>{
    res.render('users/signup');
});



router.post('/Users/new-user', async(req,res) =>{
    const {Name,Password, Email,EmailRepeat, City,State,Zip,MAC0,MAC1,MAC2,MAC3,MAC4,MAC5} = req.body;
    const errors = [];
    console.log(req.body);
    if(Email != EmailRepeat){errors.push({text: "Please both emails have the same"})}

    if(errors.length >= 1){
        console.log(errors);
         res.render('users/signup', {errors, Name, Email,EmailRepeat, City,State,Zip,MAC0,MAC1,MAC2,MAC3,MAC4,MAC5});
     }else{
         console.log("Save");
         const MAC = `${MAC0}:${MAC1}:${MAC2}:${MAC3}:${MAC4}:${MAC5}`;
         const newUser = new User({Name, Password, Email,City,State,Zip,MAC});
         await newUser.save();
        res.redirect('/');
     }
 });

module.exports= router;