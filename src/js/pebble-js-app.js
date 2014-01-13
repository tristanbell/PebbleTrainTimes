var api_key = '55c9e53cb6ab119eb1b02218671c2bda';
var app_id = 'f933bbb6';
var station_distance = 0;
var my_station;
var my_station_full;

var departures_found = false;

var messageQueue = [];
var appMessageTimeout = 50;
var maxAppMessageTries = 3;
var appMessageRetryTimeout = 500;
var num_departures = 25; // This must be kept synchronized with quick_journey.c

/* Left with everything in order. Still to do:
 *		- Give option to choose station
 *		- Give option to pre-configure destination
 *		- Store recent stations
 *		- Finish icons
 */
					
function sendAppMessages() {
        if (messageQueue.length > 0) {
                currentMessage = messageQueue[0];
                currentMessage.numTries = currentMessage.numTries || 0;
                if (currentMessage.numTries < maxAppMessageTries) {
                        console.log('Sending: ' + JSON.stringify(currentMessage));
                        currentMessage.transactionId = Pebble.sendAppMessage(
                                currentMessage,
                                function(e) {
									messageQueue.shift();
                                    setTimeout(function() {
										sendAppMessages();
                                    }, appMessageTimeout);
                                }, function(e) {
                                    console.log('Failed sending AppMessage with transactionId:' + e.data.transactionId + '. Error: ' + e.data.error.message);
                                    messageQueue[0].numTries++;
                                    setTimeout(function() {
                                        sendAppMessages();
                                    }, appMessageRetryTimeout);
                                }
                        );
                } else {
					console.log("Sending app message failed too many times. Aborting: " + currentMessage.transactionId);
                }
        } else if (!departures_found) {
			setTimeout(function() { sendAppMessages(); }, appMessageRetryTimeout);
		} else {
			console.warn("No messages to send.");
		}
}

function httpRequest() {
	if (my_station == "") { 
		return;
	}
	
	console.log("Getting departures...");
	
	var request = new XMLHttpRequest();
	var url = 'http://transportapi.com/v3/uk/train/station/' + my_station + '/live.json';
	url = url + "?api_key=" + api_key + "&app_id=" + app_id + "&limit=" + num_departures;
	console.log("URL: " + url);
	request.open('GET', url, true);
	
	request.onload = function(e) {
		if (request.readyState == 4 && request.status == 200) {
			var response = JSON.parse(request.responseText);
			
			if (response && response.departures && response.departures.all.length > 0) {
				var result = response.departures.all;
				for (var i = 0; i < result.length; i++) {
					var station_name = result[i].destination_name;
					var platform = result[i].platform;
					var departure_time = result[i].expected_departure_time;
					console.log(station_name + ', ' + platform + ', ' + departure_time);

					if (platform) {
						messageQueue.push({"from_station": my_station_full, "destination": station_name, "expected_departure": departure_time, "platform": platform });
 					} else {
						messageQueue.push({"from_station": my_station_full, "destination": station_name, "expected_departure": departure_time });
 					}
			    }
				
				departures_found = true;
			} else {
				console.warn("No departures");
				Pebble.showSimpleNotificationOnPebble("No departures", "No scheduled departures for " + my_station);
			}
		} else {
			console.warn("Connection to server failed: ready state " + request.readyState + ", status " + request.status);
			Pebble.showSimpleNotificationOnPebble("Error", "Connection to server failed.");
		}
	}
	
	sendAppMessages();
	request.send(null);
}

function findNearbyStations(lat, lon) {
	console.log("Finding nearby stations...");
	var request = new XMLHttpRequest();
	var url = 'http://transportapi.com/v3/uk/train/stations/near.json';
	var rpp = 10;
	request.open('GET', url + "?lat=" + lat + "&lon=" + lon + "&page=1" + "&rpp=" + rpp + "&api_key=" + api_key + "&app_id=" + app_id, true);
	
	request.onload = function(e) {
		if (request.readyState == 4 && request.status == 200) {
			var response = JSON.parse(request.responseText);
			
			if (response && response.stations && response.stations.length > 0) {
				var result = response.stations;
				my_station = result[station_distance].station_code;
				my_station_full = result[station_distance].name;
				console.log("Nearest station: " + my_station);
				httpRequest();
			} else {
				console.warn("ERROR: no stations found");
			}
		}
	}
	
	request.send(null);
}

var locationOptions = { "timeout": 10000, "maximumAge": 60000 };

function locationSuccess(pos) {
	console.log("Location found.");
	var coordinates = pos.coords;
	findNearbyStations(coordinates.latitude, coordinates.longitude);
}

function locationError(err) {
	console.warn('location error (' + err.code + '): ' + err.message);
	Pebble.showSimpleNotificationOnPebble("Error", "Location cannot be found.");
}

Pebble.addEventListener("ready",
  	function(e) {
    	console.log("JavaScript app ready and running!");
  	}
);

ebble.addEventListener("showConfiguration",
  	function(e) {
    	console.log("Configuring...");
          Pebble.openURL("");
  	}
);


Pebble.addEventListener("appmessage",
	function(e) {
		console.log("Received message: " + e.payload);
                   //Pebble.sendAppMessage({"from_station":"Test", "destination": "Test", "expected_departure":"12:00", "platform": 2 });
 		window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
	});