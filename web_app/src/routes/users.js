const express = require('express');
const router = express.Router();
const User = require('../models/User');
const passport = require('passport');
const { isAuthenticated } = require("../helpers/auth");

router.get('/users/Edit',isAuthenticated, async (req,res)=>{
    //console.log(req.user);
    const name  =  req.user.name;
    const email =  req.user.email;
    const state = req.user.state;
    const zip = req.user.zip;
    const city = req.user.city;
    const MAC =  (req.user.MAC).split(":");
    const MAC0 =  MAC[0];
    const MAC1 =  MAC[1];
    const MAC2 =  MAC[2];
    const MAC3 =  MAC[3];
    const MAC4 =  MAC[4];
    const MAC5 =  MAC[5];
    let TCS34725 ,BME280 ,Si7021,CCS811,indoor;
    if((req.user.devices)[0] == '1'){ TCS34725 = 'checked' };
    if((req.user.devices)[1] == '1'){ BME280 = 'checked' };
    if((req.user.devices)[2] == '1'){ Si7021 = 'checked' };
    if((req.user.devices)[3] == '1'){ CCS811 = 'checked' };
    if(req.user.indoor == '1'){ indoor = 'checked' };
    res.render('users/edit', {name, email, indoor, state,zip,city, MAC0,MAC1,MAC2,MAC3,MAC4,MAC5,BME280, CCS811, Si7021, TCS34725});
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
router.post('/Users/update-user', async(req,res) =>{
    let {name,password, email,emailRepeat, city,state,zip,MAC0,MAC1,MAC2,MAC3,MAC4,MAC5,BME280, CCS811, Si7021, TCS34725, indoor} = req.body;
    const errors = [];
    //console.log(req.body);
    if(email != emailRepeat){errors.push({text: "Please both emails have the same"})}
    if(password.length <=2 ){errors.push({text: "Please introduce a password"})}

    if(errors.length >= 1){
        if(TCS34725 == 'on'){ TCS34725 = 'checked' };
        if(BME280 == 'on'){ BME280 = 'checked' };
        if(Si7021 == 'on'){ Si7021 = 'checked' };
        if(CCS811 == 'on'){ CCS811 = 'checked' };
        if(indoor == 'on'){ indoor = 'checked' };

        res.render('users/edit', {errors, name, email,emailRepeat, city,state,zip,MAC0,MAC1,MAC2,MAC3,MAC4,MAC5,BME280, CCS811, Si7021, TCS34725, indoor});
     }else{
        let devices = "";   
        const MAC = `${MAC0}:${MAC1}:${MAC2}:${MAC3}:${MAC4}:${MAC5}`;
        
        if(TCS34725 == 'on'){ devices += '1'}else{ devices += '0'};
        if(BME280 == 'on'){ devices += '1'}else{ devices += '0'};
        if(CCS811 == 'on'){ devices += '1'}else{ devices += '0'};
        if(Si7021 == 'on'){ devices += '1'}else{ devices += '0'};
        if(indoor == 'on'){ indoor = true }else{ indoor = false};

        //console.log("Updated...:"+ devices);
        password = await req.user.encryptPassword(password);
        await User.findByIdAndUpdate(req.user.id, { name, email,MAC,city,state, zip,password, devices:devices,indoor});
        req.flash("success_msg","Data update succefully");
        res.redirect('/');
        
         
     }
});

router.post('/Users/new-user', async(req,res) =>{
    const {name,password, email,emailRepeat, city,state,zip,MAC0,MAC1,MAC2,MAC3,MAC4,MAC5, BME280, CCS811, Si7021, TCS34725,indoor} = req.body;
    const errors = [];
    //console.log(req.body);
    if(email != emailRepeat){errors.push({text: "Please both emails have the same"})}
    if(email == '' || emailRepeat == ''){errors.push({text: "Please enter a email"})}
    if(TCS34725 == 'on'){ TCS34725 = 'checked' };
    if(BME280 == 'on'){ BME280 = 'checked' };
    if(Si7021 == 'on'){ Si7021 = 'checked' };
    if(CCS811 == 'on'){ CCS811 = 'checked' };
    if(indoor == 'on'){ indoor = 'checked' };


    if(errors.length >= 1){
        res.render('users/signup', {errors, name, email,emailRepeat, city,state,zip,MAC0,MAC1,MAC2,MAC3,MAC4,MAC5,TCS34725,BME280,Si7021, CCS811, indoor});
     }else{
        const emailUser = await User.findOne({ email: email });
        if (emailUser) {
            errors.push({text: "The Email is already in use."})
            res.render('users/signup', {errors, name, email,emailRepeat, city,state,zip,MAC0,MAC1,MAC2,MAC3,MAC4,MAC5,TCS34725,BME280,Si7021, CCS811, indoor});
        }else{
            let devices = "";   
            //console.log("Saving...");
            if(TCS34725 == 'on'){ devices += '1'}else{ devices += '0'};
            if(BME280 == 'on'){ devices += '1'}else{ devices += '0'};
            if(CCS811 == 'on'){ devices += '1'}else{ devices += '0'};
            if(Si7021 == 'on'){ devices += '1'}else{ devices += '0'};
            if(indoor == 'on'){ indoor = true }else{ indoor = false};

            const MAC = `${MAC0}:${MAC1}:${MAC2}:${MAC3}:${MAC4}:${MAC5}`;
            const newUser = new User({name, password, email,city,state,zip,MAC,devices:devices,indoor});
            newUser.password = await newUser.encryptPassword(password);
            await newUser.save();
            //console.log(newUser);
            req.flash("success_msg","yoou are registered");
            res.redirect('/');
        }
         
     }
});

module.exports= router;