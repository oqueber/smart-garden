const express = require('express');
const router = express.Router();
const Aromatics = require('../flowers/Aromaticas');
const vegetable = require('../flowers/Hortalizas');
const { isAuthenticated } = require("../helpers/auth")
const User = require('../models/User');

router.get('/my-plants',isAuthenticated, async(req,res)=>{
    
    const plants = {};
    const user = await User.findOne({email: req.user.email});
    
    user.plants.forEach( (element, index )=> {
        let info_copy;
        
        if(element.info.type == "vegetable"){
            info_copy = JSON.stringify(vegetable[ (element.info.index) ]);
        }else if(element.info.type == "aromatic"){
            info_copy = JSON.stringify(Aromatics[ (element.info.index) ]);
        }else{
            res.render('404');
        }
        let plant = JSON.parse(info_copy).info;
        plant['date'] = element.info.date;

        plants[index] = {info: plant};
    }); 
    res.render('plants/home', {plants});
});

router.get('/new-plant',isAuthenticated, (req,res)=>{
    res.render('plants/select', {Aromatics, vegetable});
});

router.get('/new-plant/:Type/:index',isAuthenticated, (req,res)=>{
    const index = parseInt(req.params.index , 10);
    const Type = req.params.Type;
    let plant;

    if (Type == "aromatic"){
        plant = Aromatics[index];
    }else if(Type == "vegetable") {
        plant = vegetable[index];
    }else{
        res.render('404');
    }

    res.render('plants/add', {plant,index,Type});
});

router.get('/edit-plant/:index/', async(req,res) =>{
    
    const index = parseInt(req.params.index,10);
    //const user = await User.findOne({email: req.user.email});
    const user = await User.findOne({email: "luis@gmail.com"});
    let plant = user.plants[index];
    let info_copy;
    
    if(plant.info.type == "vegetable"){
        info_copy = (vegetable[ plant.info.index]).info;
    }else if(plant.info.type == "aromatic"){
        info_copy = (Aromatics[ plant.info.index]).info;
    }else{
        res.render('404');
    }
    
    res.render('plants/edit',{plant, info:info_copy  ,index});
 });
 
router.post('/new-plant/:type/:index/save',isAuthenticated, async (req,res)=>{
    const _index = parseInt(req.params.index , 10);
    const _type = req.params.type;
    const { waterF, waterU, lightH, lightU} = req.body;
    
    let plant = {
        info:{
            type: _type,
            index: _index,
            date: Date.now()
        },
        sowing:{
            light:{
                hours: parseInt(lightH),
                color: "5,1,4"
            },
            water:{
                frequency: parseInt(waterF),
                limit: parseInt(waterU)
            },
            temperature:{
                min: parseInt('12')
            }
        },
        pinout:{
            ADC1: parseInt("1"),
            ADC2: parseInt("2"),
            ADC3: parseInt("3"),
            ADC4: parseInt("4")
        }
    };
    
    await User.findByIdAndUpdate(req.user.id, {'$push': {plants: plant } },{ new: true });
    req.flash("success_msg","Data update succefully");
    res.redirect('/my-plants');
});


router.post('/edit-plant/:index/save',isAuthenticated, async(req,res)=>{
    const { waterF, waterU, lightH, color} = req.body;
    const index = parseInt(req.params.index);
    await User.findOne( {_id:req.user.id }).then(doc => {

        let plant = doc.plants[index];
        plant.sowing.light.hours = parseInt(lightH,10);
        plant.sowing.light.color = color;
        plant.sowing.water.frequency = parseInt(waterF,10);
        plant.sowing.water.limit = parseInt(waterU,10);
        plant.sowing.temperature.min = parseInt('12');
        plant.pinout.ADC1 = parseInt('9');
        plant.pinout.ADC2 = parseInt('7');
        plant.pinout.ADC3 = parseInt('6');
        plant.pinout.ADC4 = parseInt('5');
        doc.save();
      
        //sent respnse to client
      }).catch(err => {
        console.log(err);
        console.log('Oh! Dark')
        res.send(err);
      });
    req.flash("success_msg","Data update succefully");
    res.redirect('/my-plants');
});



module.exports= router;



