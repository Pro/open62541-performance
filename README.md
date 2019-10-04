# open62541 Performance Test Scripts


## Read Speed

This test reads a variable as often as possible while running and
measures the average time it takes between reading two values.

Usage:
```
./read-speed-server
./read-speed-client opc.tcp://server-ip:4840
```

Output when running both scripts on the same host:

```text
------ Starting reading loop. ------
readPerSec     readTimeAvg    readTimeMin      readTimeMax
74835.25     13362ns        12126ns            92148ns
77235.46     12947ns        12071ns           110574ns
77618.16     12883ns        12144ns            48023ns
77870.80     12842ns        12089ns           111430ns
77191.22     12955ns        12124ns            52855ns
75973.59     13162ns        12150ns            44507ns
75688.90     13212ns        12125ns           106780ns
74671.96     13392ns        12164ns           109631ns
75868.60     13181ns        12128ns            65457ns
78238.87     12781ns        12123ns            48529ns
```

Output when running on two different hosts connected through gigabit ethernet:

```text
------ Starting reading loop. ------
readPerSec     readTimeAvg    readTimeMin      readTimeMax
7694.28    129965ns       104597ns           556284ns
7709.44    129710ns       104277ns           273465ns
7687.28    130084ns       105299ns           499083ns
7681.97    130174ns       105424ns           253498ns
7687.33    130083ns       104274ns           254800ns
8092.11    123576ns        95484ns           387728ns
8904.85    112297ns        89530ns           269674ns
9418.94    106168ns        94253ns           204267ns
9417.27    106187ns        94450ns           820189ns
9414.08    106223ns        94372ns           202409ns
```
