const Aromatics = [];

Aromatics.push({
    info:{
        name: "Albahaca",
        description: "The Albacaha is so delicius", 
        recomendation: "La Albahaca es una planta que no soporta para nada las heladas, por lo que se recomienda empezar la siembra en semilleros y luego transplantarlas",
        beginner: false,
        image: "albahaca.jpg",
        germination: 15, //Dias
        harvest: 50, // Dias
        space: "30x20", //cm
        web: "https://www.lahuertinadetoni.es/sembrar-germinar-la-albahaca-casa/"
    },
    sowing: {
        light:{
            min: 8, //Horas diarias de luz directa
            color: '12df44' //color que le gusta
        },
        water:{
            frequency: 3, //Dias
            limit: 1
        },
        temperature:{
            min: 15 //Grados
        }
    }
});

Aromatics.push({
    info: {
        name: "Perejil",
        description: "The Perejil is so delicius", 
        recomendation: "Poner la semilla en remojo 24 horas.",
        beginner: true,
        image:"perejil.jpg",
        germination: 15, //Dias
        harvest: 90, // Dias
        space: "30x20", //cm
        web: "https://www.lahuertinadetoni.es/plantar-perejil-huerto-maceta/",
    },
    sowing: {
        light:{
            min: 3, //Horas diarias de luz directa
            color: '12df44' //color que le gusta
        },
        water:{
            frequency: 2, //Dias
            limit: 3 // Esperar que se seque la tierra hata el humbral: Tipo 0 seco, Tipo 1 un poco humedo, tipo 2 humedo, tipo 3 enchargado.
        },
        Temperature:{
            Min: 15 //Grados
        }
    }
});

for (var i = 0; Aromatics.length > i; i++) {
    Object.freeze(Aromatics[i]);
}

module.exports= Object.freeze(Aromatics);