# open62541 Performance Test Scripts


## Read Speed

This test reads a variable as often as possible while running and
measures the average time it takes between reading two values.

Usage:
```
./read-speed-server
./read-speed-client opc.tcp://server-ip:4840
```

