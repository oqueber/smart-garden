const express = require('express');
const router = express.Router();
const User = require('../models/User');
const passport = require('passport');

router.get('/users', (req,res)=>{
    res.render('users/users');
});

router.get('/users/Login', (req,res)=>{
    res.render('users/login');
});


router.get('/users/signup', (req,res)=>{
    res.render('users/signup');
});


router.get('/users/logout', (req,res)=>{
    req.logout();
    res.redirect('/');
});

router.post('/users/login-user', passport.authenticate('local', {
    successRedirect: '/my-plants',
    failureRedirect: '/users/signup',
    failureFlash: true
}));


router.post('/Users/new-user', async(req,res) =>{
    const {name,password, email,emailRepeat, city,state,zip,MAC0,MAC1,MAC2,MAC3,MAC4,MAC5} = req.body;
    const errors = [];
    console.log(req.body);
    if(email != emailRepeat){errors.push({text: "Please both emails have the same"})}

    if(errors.length >= 1){
        res.render('users/signup', {errors, name, email,emailRepeat, city,state,zip,MAC0,MAC1,MAC2,MAC3,MAC4,MAC5});
     }else{
        const emailUser = await User.findOne({ email: email });
        if (emailUser) {
            errors.push({text: "The Email is already in use."})
            res.render('users/signup', {errors, name, email,emailRepeat, city,state,zip,MAC0,MAC1,MAC2,MAC3,MAC4,MAC5});
        }else{
            console.log("Save");
            const MAC = `${MAC0}:${MAC1}:${MAC2}:${MAC3}:${MAC4}:${MAC5}`;
            const newUser = new User({name, password, email,city,state,zip,MAC});
            newUser.password = await newUser.encryptPassword(password);
            await newUser.save();
            req.flash("success_msg","yoou are registered");
            res.redirect('/');
        }
         
     }
 });

module.exports= router;