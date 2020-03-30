const { app, express, server} = require('./utils/socketio.js');
const connectdb = require('./database');
const Mqtt = require('./mqtt/server');
const Socket = require('./socketio/server');

const path = require('path');
const exphbs = require('express-handlebars');
const methodOverride= require('method-override');
const session = require('express-session');


//Initializations
process.title = 'myApp';
connectdb.then( db => {
    console.log(`[Smart-Garden-dB]: Connected`);
})

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