# ids-ic

## To build docker:
```
docker build -t snort3-docker .
```
## To run docker:
```
docker run --rm -it snort3-docker  
```
## Running snort - using a single rule file:
### Inline:
```
snort -Q --daq afpacket -R <rules file> -i <interface> -A cmg
```
### Passive:
```
snort --daq afpacket -R <rules file> -i <interface> -A cmg
```
## Running snort - using a rule path
 

### Inline:
```
snort -Q --daq afpacket --rule-path <rules path> -i <interface> -A cmg
```
### Passive:
```
snort --daq afpacket --rule-path <rules path> -i <interface> -A cmg
