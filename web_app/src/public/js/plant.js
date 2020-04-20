const umbralWater = ["Seco", "Poco Humedo", "Humedo", "Encharcado"];
const umbralLux = ["Sin luz", "poca luz", "en sombra", "Luz directa"];


window.onload = function() {
    
    const valueSliderFreq = document.getElementById("sliderFreq");
    const valueSpanFreq = document.getElementById('valueSpanFreq');
    valueSpanFreq.textContent = `cada ${valueSliderFreq.value} dias`;
    
    const valueSliderWater = document.getElementById("sliderWater");
    const valueSpanWater = document.getElementById('valueSpanWater');
    valueSpanWater.textContent = `${umbralWater[valueSliderWater.value]}`;
    
    const valueSliderLux = document.getElementById("sliderLux");
    const valueSpanLux = document.getElementById('valueSpanLux');
    valueSpanLux.textContent = `${valueSliderLux.value} horas diarias`;

    const valueSliderLuxUmbral = document.getElementById("sliderLuxUmbral");
    const valueSpanLuxUmbral = document.getElementById('valueSpanLuxUmbral');
    valueSpanLuxUmbral.textContent = `${valueSliderLuxUmbral.value}`;
    
    valueSliderLuxUmbral.addEventListener('change', (event) => {
        valueSpanLuxUmbral.textContent = `${umbralLux[event.target.value]}`;
    });

    valueSliderFreq.addEventListener('change', (event) => {
        valueSpanFreq.textContent = `cada ${event.target.value} dias`;
    });
    
    valueSliderWater.addEventListener('change', (event) => {
        valueSpanWater.textContent = `${umbralWater[event.target.value]}`;
    });
    
    valueSliderLux.addEventListener('change', (event) => {
        valueSpanLux.textContent = `${event.target.value} horas diarias`;
    });
}