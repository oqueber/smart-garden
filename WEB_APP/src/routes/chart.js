const express = require('express');
const router = express.Router();

router.get('/chart',  (req,res)=>{
    res.render('charts/main_chart');
});

router.post('/note/new-note', async(req,res) =>{
   const {title,description} = req.body;

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
       res.redirect('/notes');
    }
});

router.get('/notes', async (req,res)=>{
   const notes = await Note.find().sort({date: 'desc'});
   console.log(notes);
   res.render('notes/all-notes',{ notes });
});

module.exports= router;