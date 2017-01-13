var http = require('http');
var mongodb = require('mongodb');

var mongoClient = mongodb.MongoClient;
var headers_keys = ['latitude', 'longitude', 'altitude', 'satelites_count', 'localization_time', 'battery_rate'];

function filter_request(request){
    var result = true;
    for (var header in headers_keys){
        result = result && request.headers[headers_keys[header]] !== undefined;
    }
    return result;
}

var server = http.createServer(function(request, response){
    response.writeHead(200, {"Content-Type": "text/plain"});
    console.log(request.headers);
    if(filter_request(request) === false){
        console.log("The request does not contain required headers\n");
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
                    //console.log("Result", result)
                });
        }
    );
    response.end("\n");
});

server.listen(80);
console.log("Server is running\n");
