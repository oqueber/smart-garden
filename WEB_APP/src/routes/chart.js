const express = require('express');
const router = express.Router();

router.get('/chart',  (req,res)=>{
    res.render('charts/main_chart');
});


router.get('/notes', async (req,res)=>{
   const notes = await Note.find().sort({date: 'desc'});
   console.log(notes);
   res.render('notes/all-notes',{ notes });
});

module.exports= router;