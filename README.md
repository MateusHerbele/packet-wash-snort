# ids-ic

## To build docker:
```
docker build -t snort3-docker .
```
## To run docker:
```
docker run --rm -it snort3-docker  
```
## Running snort:
```
snort -Q --daq afpacket -i eth0
