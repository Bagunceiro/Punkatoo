if (!!window.EventSource) {
    var source = new EventSource('/events');
    
    source.addEventListener('open', function(e) {
     console.log("Events Connected");
    }, false);
    source.addEventListener('error', function(e) {
     if (e.target.readyState != EventSource.OPEN) {
       console.log("Events Disconnected");
     }
    }, false);
    
    source.addEventListener('message', function(e) {
     console.log("message", e.data);
    }, false);
    
    source.addEventListener('nowtime', function(e) {
     console.log("nowtime", e.data);
     document.getElementById("nowtime").innerHTML = e.data;
    }, false);
    source.addEventListener('light', function(e) {
     console.log("light", e.data);
     document.getElementById("light").innerHTML = e.data;
    }, false);
     source.addEventListener('temperature', function(e) {
     console.log("temperature", e.data);
     document.getElementById("temperature").innerHTML = e.data;
    }, false);
    source.addEventListener('humidity', function(e) {
     console.log("humidity", e.data);
     document.getElementById("humidity").innerHTML = e.data;
    }, false);
    source.addEventListener('pressure', function(e) {
     console.log("pressure", e.data);
     document.getElementById("pressure").innerHTML = e.data;
    }, false);
     source.addEventListener('uptime', function(e) {
     console.log("uptime", e.data);
     document.getElementById("uptime").innerHTML = e.data;
    }, false);
   }