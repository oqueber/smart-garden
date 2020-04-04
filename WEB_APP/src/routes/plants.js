const express = require('express');
const router = express.Router();
const aromaticas = require('../flowers/Aromaticas');
const hortalizas = require('../flowers/Hortalizas');

router.get('/my-plants', (req,res)=>{
    res.render('plants/plants');
});

router.get('/new-plant', (req,res)=>{
    res.render('plants/newPlant', {aromaticas, hortalizas});

});

router.get('/new-plant/:Type/:index', (req,res)=>{
    var index = req.params.index
    var Type = req.params.Type

    if (Type == "aromatica"){
        res.render('plants/newPlant-edit', aromaticas[index]);
    }else if(Type == "hortaliza") {
        res.render('plants/newPlant-edit', hortalizas[index]);
    }else{
        res.render('404');
    }
});


router.post('/my-plants/new-plant', async(req,res) =>{
    /*const {title,description} = req.body;
 
    const errors = [];
 
    if(!title){errors.push({text: "Please write a title"})}
    if(!description){errors.push({text: "Please write a description"})}
 
     if(errors.length >0){
         res.render('notes/new-note',{
             errors,
             title,
             description
         });
     }else{
        const newNote = new Note({title,title2:"hola",description});
        await newNote.save();
    }*/
    res.redirect('/my-plants');
 });

module.exports= router;



