#include "contiki.h"
#include "net/netstack.h"
#include "cc2420.h"
#include <stdio.h>
#include <limits.h>

// see datasheet for cc2420, page 48, chapter 23
#define RSSI_OFFSET_DBM -45

// we want to force 64-bit arithmetic, because of the risk of overflows
#define RSSI_SAMPLE_LENGTH_MICROSECONDS 128LL

// half a second
#define RSSI_MEASUREMENT_TIME_MICROSECONDS 500000LL

#define RSSI_NUM_SAMPLES (RSSI_MEASUREMENT_TIME_MICROSECONDS / RSSI_SAMPLE_LENGTH_MICROSECONDS)

#define NEW_CHANNEL_DELAY_SECONDS 10

// convert from raw RSSI reading to calibrated RSSI reading
long long int raw2rssi(int r) {
	return (long long int) r + RSSI_OFFSET_DBM;
}

PROCESS(interference_measurement, "Interference Measurement Process");
AUTOSTART_PROCESSES(&interference_measurement);

PROCESS_THREAD(interference_measurement, ev, data)
{
	static struct etimer timer;

	PROCESS_BEGIN();

	// init  and turn on telosB radio driver
	cc2420_init();
	NETSTACK_RADIO.on();

	// find a new best channel every 10 seconds
	while(1) 
	{
		//init values for best channel and rssi
		int best_channel = -1;
		long long int best_rssi = INT_MAX;
		for (int channel = 11; channel < 27; channel++)
		{
			// set radio channel
			cc2420_set_channel(channel);
			// average enough RSSI samples to cover the chosen measurement time
			long long int rssi = 0;
			for (int i = 0; i < RSSI_NUM_SAMPLES; i++) {
				rssi += raw2rssi(cc2420_rssi());
			}
			rssi /= RSSI_NUM_SAMPLES;
			printf("Channel %d - Signal strength: %lld dBm\n", channel, rssi);
			// Check if better than previous best channel, if so, update
			if(rssi < best_rssi) {
				best_rssi = rssi;
				best_channel = channel;
			}
		}
		printf("Best channel is %d - Signal strength: %lld dBm\n", best_channel, best_rssi);

		// wait 10 seconds before switching to a new best channel
		etimer_set(&timer, CLOCK_SECOND * NEW_CHANNEL_DELAY_SECONDS);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
	}
	
	// turn off radio receiver
	NETSTACK_RADIO.off();

	PROCESS_END();
}
