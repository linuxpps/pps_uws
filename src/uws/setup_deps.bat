@ECHO "rapidjson"
git clone https://github.com/Tencent/rapidjson.git

@ECHO "uWebSockets"
git clone https://github.com/uNetworking/uWebSockets.git uWebSockets
cd uWebSockets
git submodule init
git submodule update


