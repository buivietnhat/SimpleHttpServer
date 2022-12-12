## Simple Http Server

### 1. Build and Run
* Compilier: GNU gcc version 9.4.0
* cmake: version 3.19.6
#### 1.1. How to build
```angular2html
    mkdir build && cd build
    cmake ..
```
  * main 
```angular2html
    make main 
```
  * test
```angular2html
    make test
```
  * or just 
```angular2html
    make 
```
for building all

#### 1.2. Run:
 * main program:
```angular2html
    ./main
```
  * test:
```angular2html
    ./test/test_module
    for example: ./test/message_parser_test
```
  * or 
```angular2html
    make test
```
will execute all the tests

### 2. Current supported Http operations:
For now the system only supports simple http requests with request target is an url as an absolute path, and also only supports HTTP/1.1 version, for example:
```angular2html
GET / HTTP/1.1
POST / HTTP/1.1
GET /index.html HTTP/1.1
HEAD /index.html HTTP/1.1
```

### 3. Performance report
I use the [mkr](https://github.com/wg/wrk) benchmark tool for testing the performance and run both client and server on the same device

#### 3.1. The testing device:
* CPU
```angular2html
  vendor_id	: GenuineIntel
  model name	: 11th Gen Intel(R) Core(TM) i7-1165G7 @ 2.80GHz
  CPU(s): 8
  NUMA node0 CPU(s): 0-7

```
* Networking
```angular2html
	DeviceName: Onboard - Ethernet
	Subsystem: Intel Corporation Device [8086:0074]
	Kernel driver in use: iwlwifi
--
    Ethernet controller [0200]: Intel Corporation Device [8086:15f3] (rev 03)
	Subsystem: Intel Corporation Device [8086:3004]
	Kernel driver in use: igc

```
#### 3.2. Performance report:
* 1 minute test, 1000 active connections:
```angular2html
./wrk -t5 -c1000 -d60s http://0.0.0.0:8080/
Running 1m test @ http://0.0.0.0:8080/
5 threads and 1000 connections
Thread Stats   Avg      Stdev     Max   +/- Stdev
Latency     4.58ms    1.50ms  44.16ms   90.15%
Req/Sec    43.69k     5.70k   88.57k    70.70%
13026954 requests in 1.00m, 1.14GB read
Requests/sec: 216840.47
Transfer/sec:     19.44MB

```

* 1 minute test, 10000 active connections:
```angular2html
./wrk -t5 -c10000 -d60s http://0.0.0.0:8080/
Running 1m test @ http://0.0.0.0:8080/
5 threads and 10000 connections
Thread Stats   Avg      Stdev     Max   +/- Stdev
Latency    57.44ms    7.92ms 129.80ms   67.41%
Req/Sec    34.88k     3.24k   86.75k    73.57%
10382562 requests in 1.00m, 0.91GB read
Requests/sec: 172820.65
Transfer/sec:     15.49MB

```

* Conclusion: The system can handle 10000 connections with 172820.65 reqs/s as requirement 