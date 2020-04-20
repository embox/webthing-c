# webthing-c
POSIX-compatible implementation in C of a Web Thing server

To install Web Thing gateway install packages as described on https://github.com/mozilla-iot/gateway
Then  checkout gateway to a temporary folder
```
git clone https://github.com/mozilla-iot/gateway.git
```
and install it
```
cd gateway
./install.sh
```
gateway will be installed into `~/mozilla-iot/gateway` folder

Start gateway
```
cd ~/mozilla-iot/gateway
./run-app.sh
```

build webthings-c
install cjson (for archlinux) or libcjson (for ubuntu ubuntu) packet

