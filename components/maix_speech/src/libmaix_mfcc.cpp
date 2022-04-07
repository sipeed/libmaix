#include <iostream>
#include <vector>

#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include "time.h"

#include "fvad.h"
#include <alsa/asoundlib.h>

extern "C" {

	static unsigned int WAVE_SAMPLE_RATE = 8000; //44100 48000

	#define DEBUG_MEM
	#ifdef DEBUG_MEM
	static void *_warp_alloc;
	#define malloc(size)            \
		_warp_alloc = malloc(size); \
		printf("[%s-%d]:%p:%ld\r\n", __FUNCTION__, __LINE__, _warp_alloc, (size));
	#define free(p) \
		free(p);    \
		printf("[%s-%d]:%p:free\r\n", __FUNCTION__, __LINE__, (p))
	#endif

	typedef enum
	{
		dls_state_error = -1,
		dls_state_idle,
		dls_state_ready,
		dls_state_study,
		dls_state_work,
		dls_state_finish,
		dls_state_destroy,
	} dls_state;

	typedef struct _dls_mfcc
	{
		int id;
		snd_pcm_t *handle;
		dls_state state;
		pthread_t _thread;
		Fvad *vad;
	} dls_mfcc;

	void _dls_mfcc_event(dls_mfcc *self) {
		while (self->state != dls_state_destroy) {
			switch (self->state) {
				case dls_state_idle:
					self->state = dls_state_ready;
					/* code */
					break;
				case dls_state_ready:
					self->state = dls_state_work;
					break;
				case dls_state_work:
					self->state = dls_state_finish;
					/* code */
					break;
				case dls_state_finish:
					/* code */
					break;
				case dls_state_destroy:
					break;
			}
			usleep(30*1000); // 30ms
		}
	}

	void dls_mfcc_reset(dls_mfcc *self) {
		self->state = dls_state_ready;
	}

	int dls_mfcc_load(dls_mfcc *self) {
		// load words
	}

	int dls_mfcc_save(dls_mfcc *self, int id) {
		if (dls_state_finish == self->state) {
			// save words
			dls_mfcc_load(self);
			return id;
		}
		return dls_state_error;
	}

	int dls_mfcc_init(dls_mfcc *self, const char *device) {
		int freq_adjust_direction = 1;
		snd_pcm_hw_params_t *hw_params = NULL;
		self->id = dls_state_error;
		self->state = dls_state_idle;
		//Oeffnen des Handle zum Aufnehmen
		if (snd_pcm_open(&self->handle, !device ? "default" : device, SND_PCM_STREAM_CAPTURE, 0) < 0)
		{
			fprintf(stderr, "Konnte Geraet %s nicht oeffnen\n", !device ? "default" : device);
			return -1;
		}
		//Speicher fuer Hardware-Parameter reservieren
		if (snd_pcm_hw_params_malloc(&hw_params) < 0)
		{
			fprintf(stderr, "Fehler beim Reservieren von Speicher fuer die Hardware-Parameter\n");
			return -2;
		}
		//Standard Hardware-Parameter erfragen
		if (snd_pcm_hw_params_any(self->handle, hw_params) < 0)
		{
			fprintf(stderr, "Fehler beim Holen der Standard-Hardware-Parameter\n");
			return -3;
		}
		//Setzen der Hardware-Parameter innerhalb der Struktur
		if (snd_pcm_hw_params_set_access(self->handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0)
		{
			fprintf(stderr, "Fehler beim Setzen des Lese-\\Schreibe-Zugriffs\n");
			return -4;
		}
		if (snd_pcm_hw_params_set_format(self->handle, hw_params, SND_PCM_FORMAT_S16_LE) < 0)
		{
			fprintf(stderr, "Fehler beim Setzen des PCM-Formates\n");
			return -5;
		}
		if (snd_pcm_hw_params_set_rate_near(self->handle, hw_params, &WAVE_SAMPLE_RATE, 0) < 0)
		{
			fprintf(stderr, "Fehler beim Setzen der Frequenz\n");
			return -6;
		}
		if (snd_pcm_hw_params_set_channels(self->handle, hw_params, 1) < 0)
		{
			fprintf(stderr, "Fehler beim Setzen der Anzahl der Kanaele\n");
			return -7;
		}
		//Setzen der Hardware-Parameter beim Handle
		if (snd_pcm_hw_params(self->handle, hw_params) < 0)
		{
			fprintf(stderr, "Fehler beim Setzen der Hardwareparameter\n");
			return -8;
		}
		//Speicher freigeben
		snd_pcm_hw_params_free(hw_params);
		//Hardware vorbereiten
		if (snd_pcm_prepare(self->handle) < 0)
		{
			fprintf(stderr, "Fehler beim Vorbereiten des Handles\n");
			return -9;
		}
		self->vad = fvad_new();
		// Set aggressiveness mode
		fvad_set_mode(self->vad, 1); // Mode 3, Very aggressive.
		if (fvad_set_sample_rate(self->vad, WAVE_SAMPLE_RATE) < 0) {
				fprintf(stderr, "invalid sample rate: %d Hz\n", WAVE_SAMPLE_RATE);
		}

		//Starten des Thread
		if (pthread_create(&self->_thread, NULL, (void *(*) (void *))_dls_mfcc_event, self) < 0)
		{
			return dls_state_error;
		}

		dls_mfcc_load(self);
	}

	void dls_mfcc_exit(dls_mfcc *self) {
		self->state = dls_state_destroy;
		pthread_join(self->_thread, NULL);
		if (self->vad) fvad_free(self->vad);
		//Handle schliessen
		snd_pcm_close(self->handle);
	}

	int test_vector() {

		// Create a vector containing integers
		std::vector<int> v = { 7, 5, 16, 8 };

		// Add two more integers to vector
		v.push_back(25);
		v.push_back(13);

		// Print out the vector
		std::cout << "v = { ";
		for (int n : v) {
			std::cout << n << ", ";
		}
		std::cout << "}; \n";

	}

	int unit_test_mfcc() {

		test_vector();

		dls_mfcc mfcc, *self = &mfcc;
		dls_mfcc_init(self, NULL);

		char ans[16];
		printf("0 Enter identifier number (0, 1, 2) (x to skip): "), fflush(NULL);
		scanf("%s", (char *)ans);
		if (!(ans[0] == 'x' && ans[1] == '\0'))
		{
			int id = atoi(ans);
			printf("1 identifier: %d \r\n", id);

			dls_mfcc_reset(self);
			while (1) {
				if (self->state == dls_state_ready) {
					puts("2 keep quiet...");
				}
				if (self->state == dls_state_finish) {
					dls_mfcc_save(self, id);
					printf("2 save id: %d \r\n", id), fflush(NULL);
					break;
				}
				usleep(30*1000); // 100ms
			}
		}

		printf("0 Enter recognition 3 times \r\n"), fflush(NULL);
		for (int i = 0; i < 3; i++) {
			printf("1 recognition... \r\n");
			dls_mfcc_reset(self);
			while (1) {
				if (self->state == dls_state_ready) {
					puts("2 keep quiet...");
				}
				if (self->state == dls_state_finish) {
					printf("2 reco id: %d \r\n", self->id);
					break;
				}
				usleep(30*1000); // 100ms
			}
		}

		dls_mfcc_exit(self);

		return 0;
	}

}
