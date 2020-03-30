
const Hortalizas;

Hortalizas["Albahaca"] ={
    Siembra: {
        Germinacion: 15, //Dias
        Transplante: 45, //Dias
        Inicio: 4,       // (Mayo) Numero del mes empezando desde cero
        Fin: 5,          // (Junio) Numero del mes empezando desde cero
        Riego:{
            Tierra_Seca: false, //No permitir que se seque el sustrato
            Frecuencia: [3,4,5] //Dias
        },
        Temperatura:{
            Min: 15 //Grados
        }
    },
    Transplante:{
        Inicio: 5,          // (Junio) Numero del mes empezando desde cero
        Fin: 7,             // (Agosto) Numero del mes empezando desde cero
        Riego:{
            Tierra_Seca: true,
            Frecuencia: [4,5] //Dias
        }
    },
    Floracion:{
        Inicio: 7,  // (Agoto) Numero del mes empezando desde cero
        Fin:    9   // (Octubre) Numero del mes empezando desde cero
    },
    info: {
        web: "https://www.lahuertinadetoni.es/sembrar-germinar-la-albahaca-casa/",
        Video: "https://www.youtube.com/watch?v=a1MOu_vHOsQ",
    },
    Comentario: "La Albahaca es una planta que no soporta para nada las heladas, por lo que se recomienda empezar la siembra en semilleros y luego transplantarlas"
}

module.exports= Hortalizas;