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
    res.render('plants/plants', {plants});
});

router.get('/new-plant',isAuthenticated, (req,res)=>{
    res.render('plants/newPlant', {Aromatics, vegetable});
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

    res.render('plants/newPlant-edit', {plant,index,Type});
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
                color: "#23321"
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
    const { waterF, waterU, lightH, lightU} = req.body;
    const index = parseInt(req.params.index);

    await User.findOne( {_id:req.user.id }).then(doc => {

        console.log(`Element index in plants ${index}`);
        let plant = doc.plants[index];
        console.log(plant);
        plant.sowing.light.hours = parseInt(lightH);
        plant.sowing.light.color = "#1345";
        plant.sowing.water.frequency = parseInt(waterF);
        plant.sowing.water.limit = parseInt(waterU);
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

    res.redirect('/my-plants');
});

router.get('/edit-plant/:index/',isAuthenticated, async(req,res) =>{
    const index = req.params.index;
    const user = await User.findOne({email: req.user.email});
    const plant = user.plants[index];

    res.render('plants/edit',{plant,index});
 });

module.exports= router;



