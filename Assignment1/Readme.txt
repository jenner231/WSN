# Data compression

Scans Wi-Fi channels 11 to 26, for each channel it calculates the average RSSI. Picks the best channel, i.e. the one with the lowest noise, then waits 5 seconds, then repeats on a loop.
First compresses a signal x, or this exercise i just chose a basic sine wave as my signal. Then we decompress the signal again. Currently I just printed the values when ran in contiki,
but im not entirely sure how this works on the TelosB mote, as when run on there, i commented out write to file part.
This code should be pretty easily extended to work such that you can compress on 1 mote, and then decompress on another mote.
When changing M and L, we are changing the number of coefficients from the horizontal and vertical frequencies. I'm no expert in Signal processing, so this might be a limited view on what actually happens in the DCT transform.
So, increasing L without increasing M, we're essentially increasing the vertical resolution of our signal. This also made somewhat sense with the pictures we got when doing the testing as a group.


## How to run
If running on TelosB, I think it will be a good idea to comment out the section writing to a file.

### Run in contiki
Place the files in `contiki-ng/assignments/datacompression/`
Create file logs.txt
```bash
make TARGET=native decompress.native
./decompress.native
```
###To run on the TelosB mote

```bash
make TARGET=sky MOTES=/dev/ttyUSB0 decompress.upload
make TARGET=sky MOTES=/dev/ttyUSB0 login 
```
