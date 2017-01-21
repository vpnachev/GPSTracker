var mongodb = require('mongodb');
var express = require('express');

var mongoClient = mongodb.MongoClient;
var headers_keys = ['latitude', 'longitude', 'altitude', 'satelites_count', 'localization_time', 'battery_rate'];

function filter_request(request){
    var result = true;
    for (var header in headers_keys){
        result = result && request.headers[headers_keys[header]] !== undefined;
    }
    return result;
}

var app = express();
app.use(express.static('public'));

app.get('/', function(request, response){
    response.writeHead(200, {"Content-Type": "text/plain"});
    console.log(request.headers);
    if(filter_request(request) === false){
        console.log("The request does not contain required headers\n");
        response.end("\n");
        return;
    }
    mongoClient.connect("mongodb://localhost/GPSTracker",
        function(err, database){
            if(err){
                console.error("Error occured:", error);
                return;
            }
            var tracker = database.collection("tracker");
            var location = {};
            for (var key in headers_keys) {
                location[headers_keys[key]] = request.headers[headers_keys[key]];
            }
            location["submit_date"] = new Date();
            tracker.insert(location,
                function(err, result){
                    if (err) {
                        console.error(err);
                        return
                    }
                });
        }
    );
    response.end("\n");
});

app.get('/positions', function(req, res){
    mongoClient.connect("mongodb://localhost/GPSTracker",
        function(err, database){
            if(err){
                console.error("Error occured:", error);
                return;
            }
            var tracker = database.collection("tracker");
            tracker.find({}, {"_id": 0, "longitude": 1, "latitude": 1, "battery_rate": 1})
                .sort({"submit_date": -1})
                .limit(20)
                .toArray(function(err, positions){
                    if(err){
                        console.error("Cannot get positions:", err);
                        return res.status(500).send();
                    }
                    res.json(positions);
                });
        });
});

app.listen(80, function(){
    console.log("Server is running\n");
});
