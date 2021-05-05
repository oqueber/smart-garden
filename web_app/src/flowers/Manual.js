const ManualPlant = [];

ManualPlant.push({
    info:{
        name: "",
        description: "", 
        image: "Manual_Plant.jpg"
    },
    sowing: {
        light:{
            hours: 8, //Horas diarias de luz directa
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


for (var i = 0; ManualPlant.length > i; i++) {
    Object.freeze(ManualPlant[i]);
}

module.exports= Object.freeze(ManualPlant);