#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <jack/jack.h>

// don't forget to include the matlab coefficients ..
#include "fdacoefs.h"
// You need to implement a cascade soluition
// Here is a sample data structure to store the
// coefficients and taps of a single transpose-form
// cascade section. Refer also to the examples
// discussed in class on how you would use
// this structure

#define  NUM_SECTIONS ((MWSPT_NSEC-1)/2)

int initialized = 0;

typedef struct cascadestate {
  float s[2];   // state
  float b[3];   // nominator coeff  b0 b1 b2
  float a[2];   // denominator coeff   a1 a2
} cascadestate_t;

float random_half_range() {
  return ((float)rand() / RAND_MAX)*20.0f - 10.0f;
}

float transposefilter(float in) {

  //  in = random_half_range(); Uncomment to switch to internally produced noise

  static cascadestate_t state[NUM_SECTIONS];

  //Check if initialized and initialize

  if(!initialized) {
	for (int i = 0; i < NUM_SECTIONS; i++) {
		state[i].b[0] = (float)(NUM[2*i][0] * NUM[2*i+1][0]);
                state[i].b[1] = (float)(NUM[2*i][0] * NUM[2*i+1][1]);
                state[i].b[2] = (float)(NUM[2*i][0] * NUM[2*i+1][2]);

		state[i].a[0] = (float)(DEN[2*i][0] * DEN[2*i+1][1]);
                state[i].a[1] = (float)(DEN[2*i][0] * DEN[2*i+1][2]);

		state[i].s[0] = 0; state[i].s[1] = 0;
		}

  initialized = 1;
  }

  float x = in;
  float y;

  for (int i = 0; i < NUM_SECTIONS; i++) {
	y = x * state[i].b[0] + state[i].s[0];
	state[i].s[0] = x * state[i].b[1] - state[i].a[0] * y + state[i].s[1];
	state[i].s[1] = x * state[i].b[2] - state[i].a[1] * y;
	x = y;
  }

   x *= (NUM[2*NUM_SECTIONS][0] / DEN[2*NUM_SECTIONS][0]);

  return x;
}

jack_port_t *input_port_left;
jack_port_t *output_port_left;
jack_port_t *input_port_right;
jack_port_t *output_port_right;
jack_client_t *client;

int processSample (jack_nframes_t nframes, void *arg) {
  jack_default_audio_sample_t *in, *out;
  int i;
  static int index = 0;

  // the left channel goes through the IIR filter
  in = jack_port_get_buffer (input_port_left, nframes);
  out = jack_port_get_buffer (output_port_left, nframes);
  for (i=0; i<nframes; i++) {
    out[i] = transposefilter(in[i]);
  } 

  // the right channel gets passed from input to output
  in = jack_port_get_buffer (input_port_right, nframes);
  out = jack_port_get_buffer (output_port_right, nframes);
  memcpy (out, in, sizeof (jack_default_audio_sample_t) * nframes);  

  return 0;
}

void jack_shutdown (void *arg) {
  exit (1);
}

int main (int argc, char *argv[]) {
  const char **ports;
  const char *client_name = "simple";
  const char *server_name = NULL;
  jack_options_t options = JackNullOption;
  jack_status_t status;

  client = jack_client_open (client_name, options, &status, server_name);
  if (client == NULL) {
    fprintf (stderr, "jack_client_open() failed, status = 0x%2.0x\n", status);
    if (status & JackServerFailed) {
      fprintf (stderr, "Unable to connect to JACK server\n");
    }
    exit (1);
  }
  if (status & JackServerStarted) {
    fprintf (stderr, "JACK server started\n");
  }
  if (status & JackNameNotUnique) {
    client_name = jack_get_client_name(client);
    fprintf (stderr, "unique name `%s' assigned\n", client_name);
  }

  jack_set_process_callback (client, processSample, 0);
  jack_on_shutdown (client, jack_shutdown, 0);

  printf ("engine sample rate: %" PRIu32 "\n", jack_get_sample_rate (client));

  input_port_left = jack_port_register (client, "input_left",
                                   JACK_DEFAULT_AUDIO_TYPE,
                                   JackPortIsInput, 0);
  output_port_left = jack_port_register (client, "output_left",
                                    JACK_DEFAULT_AUDIO_TYPE,
                                    JackPortIsOutput, 0);
  input_port_right = jack_port_register (client, "input_right",
                                   JACK_DEFAULT_AUDIO_TYPE,
                                   JackPortIsInput, 0);
  output_port_right = jack_port_register (client, "output_right",
                                    JACK_DEFAULT_AUDIO_TYPE,
                                    JackPortIsOutput, 0);

  if ((input_port_left == NULL) || (output_port_left == NULL) ||
      (input_port_right == NULL) || (output_port_right == NULL)) {
    fprintf(stderr, "no more JACK ports available\n");
    exit (1);
  }

  if (jack_activate (client)) {
    fprintf (stderr, "cannot activate client");
    exit (1);
  }

  while (1)
    sleep(10);

  jack_client_close (client);
  
  exit (0);
}
