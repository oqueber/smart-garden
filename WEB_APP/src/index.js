const express = require('express');
const path = require('path');
const exphbs = require('express-handlebars');
const methodOverride= require('method-override');
const session = require('express-session');
const socketio = require('socket.io');
const http = require('http');

//Initializations
process.title = 'myApp';

const app= express();
const server = http.Server(app);
const io = new socketio(server);
require('./database');
require('./mqtt/server');
const Data = require('./models/DatadB');

//-----------------------------------------
//------------ Settings  ------------------
//-----------------------------------------
const port =process.env.PORT || 3000;
app.set('views',path.join(__dirname,'views'));

// Diseño de las plantillas HTML
app.engine('.hbs', exphbs({
    defaultLayout: 'main', // Pagina principal
    layoutsDir: path.join(app.get('views'),'layouts'),  //Plantilla principal
    partialsDir: path.join(app.get('views'),'partials'),//Pequeñar partes de HTML que se pueden reutilizar
    extname: '.hbs' //Que extrensiones tendran nuestros archivos
}))
app.set('view engine', '.hbs'); // Configuramos el mortor de las plantillas

io.on('connection', (socket)=>{
    console.log('[Index:Socket] connected'+ socket.id)

    Data.find({User: "Usuario1"}).sort({date: 'desc'}).then( dataUser =>{
        socket.emit('chart/getData',
            dataUser
        );

    })
    
});




//-----------------------------------------
//------------ Middlewares  ---------------
//-----------------------------------------
app.use(express.urlencoded({extended: false}))
app.use(methodOverride('_method'));
app.use(session({
    secret: 'mysecreapp',
    resave: true,
    saveUninitialized: true
}));

// Global variables


//-----------------------------------------
//------------ Routes  --------------------
//-----------------------------------------
app.use(require('./routes/index'));
app.use(require('./routes/chart'));
app.use(require('./routes/users'));


// Static Files
app.use(express.static(path.join(__dirname, 'public')));

//-----------------------------------------
//------------ Server is listenning -------
//-----------------------------------------
server.listen(port, () =>{
    console.log(`[Smart-Garden-Server]: Server on port ${port}`);
});

