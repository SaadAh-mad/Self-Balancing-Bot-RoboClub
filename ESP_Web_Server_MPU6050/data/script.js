var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

window.addEventListener('load', onload);

function onload(event) {
    initWebSocket();
}

function getValues() {
    websocket.send("getValues");
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Connection opened');
    getValues();
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

// Update PID parameter sliders
function updatePIDSlider(element) {
    var sliderId = element.id;
    var sliderValue = document.getElementById(element.id).value;
    
    // Update display value
    document.getElementById(sliderId + "Value").innerHTML = sliderValue;
    
    console.log(sliderId + ": " + sliderValue);
    
    // Send to ESP32 via WebSocket
    websocket.send(sliderId + sliderValue.toString());
}

function onMessage(event) {
    console.log(event.data);
    var myObj = JSON.parse(event.data);
    var keys = Object.keys(myObj);

    for (var i = 0; i < keys.length; i++) {
        var key = keys[i];
        
        // Update display values
        if (document.getElementById(key + "Value")) {
            document.getElementById(key + "Value").innerHTML = myObj[key];
        }
        
        // Update slider positions
        if (document.getElementById(key)) {
            document.getElementById(key).value = myObj[key];
        }
    }
}

// Optional: Reset button functionality
function resetPID() {
    document.getElementById("Kp").value = 20;
    document.getElementById("Ki").value = 0;
    document.getElementById("Kd").value = 0;
    document.getElementById("target").value = -2.5;
    
    websocket.send("Kp20");
    websocket.send("Ki0");
    websocket.send("Kd0");
    websocket.send("target-2.5");
}

// Optional: Emergency stop
function emergencyStop() {
    websocket.send("Kp0");
    websocket.send("Ki0");
    websocket.send("Kd0");
    console.log("Emergency stop activated!");
}