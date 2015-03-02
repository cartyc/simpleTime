var xhrRequest = function(url, type, callback){
  var xhr = new XMLHttpRequest();
  xhr.onload = function(){
    callback(this.responseText);
  };
  xhr.open(type,url);
  xhr.send();
};

function locationSuccess(pos){
  
  //We will request the weather here
  var url = "http://api.openweathermap.org/data/2.5/weather?lat=45.41&lon=-75.7";
  console.log("Before request");
  //send request
  xhrRequest(url, 'GET', 
            function(responseText){
              var json = JSON.parse(responseText);
              
              //Convert Kelvins
              var temperature = Math.round(json.main.temp - 273.15);
              console.log("Temperature is " + temperature);
              
              //conditions
              var conditions = json.weather[0].main;
              console.log("Conditions are " + conditions);
              
              var dictionary = {
                'KEY_TEMPERATURE': temperature,
                'KEY_CONDITIONS': conditions
              };
              //Send to pebble
              Pebble.sendAppMessage(dictionary,
                     function(e){
                       console.log("Weather info sent to Pebble Successfully");
                     },
                     function(e){
                       console.log("Error sending weather info to Pebble!");
                     });
            });
}

function locationError(err){
  console.log("Error requesting location!");
}

function getWeather(){
  console.log("getting weather");
  locationSuccess();
}




//Listen for when the watchface is opened
Pebble.addEventListener('ready',
                        function(e){
                          console.log("PebbleKit JS ready!");
                        
                         getWeather();
                       }
                       );

// List for when an AppMessage is received

Pebble.addEventListener('appmessage',
                       function(e){
                         console.log("AppMessage received!");
                         getWeather();
                       });