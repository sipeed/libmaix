
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "libmaix_mfcc.h"

#include <list>

extern "C" {

	typedef
		struct
		{
			double real;
			double imag;
		}
	comp;

	void _fft(comp *time, comp *freq, unsigned int n)
	{
		double pi = 4 * atan(1.0);
		unsigned int i = 0, j = 0, k = 0;
		unsigned int bits = 0;

		if (n == 1)
		{
			freq[0] = time[0];
			return;
		}

		//Zaehle die benoetigten Bits zur binaeren Darstellung von n
		for (i = n; i > 1; i /= 2)
			bits++;

		//Veraendere die Indizierung fuer das Aufteilen in die Haelften
		for (i = 0; i < n; i++)
		{
			int reversed = 0;
			for (j = 0; j < bits; j++)
				reversed |= (((i >> j) % 2) << (bits - j - 1));
			freq[reversed] = time[i];
		}

		//Fuer alle Laengen, die die Haelften haben
		for (i = bits; i > 0; i--)
		{
			//Laenge der Haelften
			unsigned int length_groups = (n >> (i - 1));
			//Anzahl der Untergruppen
			unsigned int num_groups = n / length_groups;
			//Fuer alle Untergruppen
			for (j = 0; j < num_groups; j++)
			{
				//Fasse zwei Untergruppen zusammen
				for (k = 0; k < length_groups / 2; k++)
				{
					//Zahl in der ersten Haelfte
					unsigned int off1 = length_groups * j + k;
					//Zahl in der zweiten Haelfte
					unsigned int off2 = off1 + length_groups / 2;
					comp off1_val, off2_val;
					double cosine = cos((-2 * pi * k) / length_groups);
					double sine = sin((-2 * pi * k) / length_groups);
					//Multipliziere die komplexen Zahlen
					off1_val.real = freq[off1].real + (freq[off2].real * cosine - freq[off2].imag * sine);
					off1_val.imag = freq[off1].imag + (freq[off2].imag * cosine + freq[off2].real * sine);
					off2_val.real = freq[off1].real - (freq[off2].real * cosine - freq[off2].imag * sine);
					off2_val.imag = freq[off1].imag - (freq[off2].imag * cosine + freq[off2].real * sine);
					freq[off1] = off1_val;
					freq[off2] = off2_val;
				}
			}
		}
		//Normalisiere das Ergebnis
		for (i = 0; i < n; i++)
		{
			freq[i].real /= n;
			freq[i].imag /= n;
			freq[i].imag *= -1;
		}
	}

	#define WAVE_FORMAT 0x001 //PCM
	#define WAVE_CHANNELS 1
	#define WAVE_SAMPLE_RATE 8000
	#define WAVE_BITS_PER_SAMPLE 16
	#define FRAMES_IN_BLOCK 8*30
	#define BLOCKSIZE (FRAMES_IN_BLOCK * (WAVE_BITS_PER_SAMPLE / 8))
	#define WAVE_FRAME_SIZE ((WAVE_BITS_PER_SAMPLE + 7) >> 3 * WAVE_CHANNELS) //(bits/sample + 7) / 8 * channels
	#define WAVE_BYTES_PER_SECOND (WAVE_SAMPLE_RATE * WAVE_FRAME_SIZE) //framesize * samplerate

	struct _dls_mfcc_event_frame
	{
		uint8_t frame[BLOCKSIZE];
	};

	std::list<_dls_mfcc_event_frame> voice_frames;

	// static char header[45] = "RIFF1234WAVEfmt 12341212123412341212data1234";

	// void prepare_header(const unsigned int size)
	// {
	// 	int values[9] = {size - 8, 16, WAVE_FORMAT, WAVE_CHANNELS, WAVE_SAMPLE_RATE, WAVE_BYTES_PER_SECOND, WAVE_FRAME_SIZE, WAVE_BITS_PER_SAMPLE, size - 44};

	// 	memcpy(header + 4, values, 4);
	// 	memcpy(header + 16, values + 1, 4);
	// 	memcpy(header + 20, values + 2, 2);
	// 	memcpy(header + 22, values + 3, 2);
	// 	memcpy(header + 24, values + 4, 4);
	// 	memcpy(header + 28, values + 5, 4);
	// 	memcpy(header + 32, values + 6, 2);
	// 	memcpy(header + 34, values + 7, 2);
	// 	memcpy(header + 40, values + 8, 4);
	// }

	// int write_pcm(const char *path)
	// {
	// 	FILE *file = NULL;

	// 	//Dateideskriptor zuweisen
	// 	if (path != NULL)
	// 		file = fopen(path, "w");

	// 	if (file == NULL)
	// 		return -1;

	// 	const unsigned int size = BLOCKSIZE * voice_frames.size();

	// 	//Header vorbereiten
	// 	prepare_header(size + 44);

	// 	//Schreiben
	// 	fwrite(header, 44, 1, file);
	// 	for (auto it = voice_frames.begin(); it != voice_frames.end(); ++it) {
	// 		fwrite(it->frame, BLOCKSIZE, 1, file);
	// 	}

	// 	//Eventuell schliessen
	// 	if (path != NULL)
	// 		fclose(file);
	// 	return 0;
	// }

	void _dls_mfcc_event(dls_mfcc *self) {

		while (self->state != dls_state_destroy) {
			usleep(10*1000); // 10ms
			switch (self->state) {
				case dls_state_idle:
				{
					int vadres, prev = -1;
					long frames[2] = {0, 0};
					long segments[2] = {0, 0};
					uint8_t voice[BLOCKSIZE] = {0};

					for (int i = 0; i < 20; i++) { // 20 * 30 ms = 600ms check noise
						int r = snd_pcm_readi(self->handle, voice, FRAMES_IN_BLOCK);
						if (r < 0) {
							fprintf(stderr, "WARNUNG: snd_pcm_readi %d\n", r);
						} else {
							vadres = fvad_process(self->vad, (const int16_t *)(voice), FRAMES_IN_BLOCK);
							if (vadres < 0) {
								fprintf(stderr, "VAD processing failed\n");
							}
							vadres = !!vadres; // make sure it is 0 or 1
							frames[vadres]++;
							if (prev != vadres) segments[vadres]++;
							prev = vadres;
						}
					}

					if (frames[1] < 5) {
						self->state = dls_state_ready;
						voice_frames.clear();
						fprintf(stderr, "dls_state_ready\n");
					}
					printf("1 voice detected in %ld of %ld frames (%.2f%%)\n",
						frames[1], frames[0] + frames[1],
						frames[0] + frames[1] ?
							100.0 * ((double)frames[1] / (frames[0] + frames[1])) : 0.0);

					break;
				}
				case dls_state_ready:
				{
					int vadres, prev = -1;
					long frames[2] = {0, 0};
					long segments[2] = {0, 0};
					for (int i = 0; i < 40; i++) { // 40 * 30 ms = 1200ms check voice
						_dls_mfcc_event_frame tmp;
						int r = snd_pcm_readi(self->handle, tmp.frame, FRAMES_IN_BLOCK);
						if (r < 0) {
							fprintf(stderr, "WARNUNG: snd_pcm_readi %d\n", r);
						} else {
							vadres = fvad_process(self->vad, (const int16_t *)(tmp.frame), FRAMES_IN_BLOCK);
							if (vadres < 0) {
								fprintf(stderr, "VAD processing failed\n");
							}
							vadres = !!vadres; // make sure it is 0 or 1
							frames[vadres]++;
							if (prev != vadres) segments[vadres]++;
							prev = vadres;
							voice_frames.push_back(tmp);
						}
					}

					if (frames[1] > 10) { // have voice
						self->state = dls_state_work;
					} else {
						if (voice_frames.size() >= 80) { // 1200ms no-voice to clear
							voice_frames.clear();
							self->state = dls_state_ready;
						}
					}

					printf("2 voice detected in %ld of %ld frames (%.2f%%)\n",
						frames[1], frames[0] + frames[1],
						frames[0] + frames[1] ?
							100.0 * ((double)frames[1] / (frames[0] + frames[1])) : 0.0);

					break;
				}
				case dls_state_work:
					{
						int vadres, prev = -1;
						long frames[2] = {0, 0};
						long segments[2] = {0, 0};
						for (int i = 0; i < 20; i++) { // 20 * 30 ms = 600ms check voice to stop
							_dls_mfcc_event_frame tmp;
							int r = snd_pcm_readi(self->handle, tmp.frame, FRAMES_IN_BLOCK);
							if (r < 0) {
								fprintf(stderr, "WARNUNG: snd_pcm_readi %d\n", r);
							} else {
								vadres = fvad_process(self->vad, (const int16_t *)(tmp.frame), FRAMES_IN_BLOCK);
								if (vadres < 0) {
									fprintf(stderr, "VAD processing failed\n");
								}
								vadres = !!vadres; // make sure it is 0 or 1
								frames[vadres]++;
								if (prev != vadres) segments[vadres]++;
								prev = vadres;
								voice_frames.push_back(tmp);
							}
						}
						if (frames[1] < 5) { // no voice
							self->state = dls_state_finish;
						} else {
							if (voice_frames.size() >= 100) { // mfcc voice max 3000ms
								voice_frames.clear();
								self->state = dls_state_idle;
							}
						}

						printf("3 voice detected in %ld of %ld frames (%.2f%%)\n",
						    frames[1], frames[0] + frames[1],
						    frames[0] + frames[1] ?
						        100.0 * ((double)frames[1] / (frames[0] + frames[1])) : 0.0);

						break;
					}
					break;
				case dls_state_finish:
				{
					fprintf(stderr, "dls_state_finish\n");
					// char *path = "test.wav";
					// write_pcm(path);
					// system("aplay test.wav");
					voice_frames.clear();
					self->state = dls_state_idle;
					break;
				}
				case dls_state_destroy:
					break;
			}
		}
	}

	void dls_mfcc_reset(dls_mfcc *self) {
		voice_frames.clear();
		self->state = dls_state_idle;
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
		unsigned int sample_rate = WAVE_SAMPLE_RATE;
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
		if (snd_pcm_hw_params_set_rate_near(self->handle, hw_params, &sample_rate, 0) < 0)
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
		fvad_set_mode(self->vad, 3); // Mode 3, Very aggressive. // alsamixer set 70 ~ 90
		if (fvad_set_sample_rate(self->vad, sample_rate) < 0) {
				fprintf(stderr, "invalid sample rate: %d Hz\n", sample_rate);
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

	int unit_test_mfcc() {

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
					// puts("2 keep quiet...");
				}
				if (self->state == dls_state_finish) {
					dls_mfcc_save(self, id);
					printf("2 save id: %d \r\n", id), fflush(NULL);
					break;
				}
				usleep(30*1000); // 100ms
			}
		}

		// printf("0 Enter recognition 3 times \r\n"), fflush(NULL);
		// for (int i = 0; i < 3; i++) {
		// 	printf("1 recognition... \r\n");
		// 	dls_mfcc_reset(self);
		// 	while (1) {
		// 		if (self->state == dls_state_ready) {
		// 			puts("2 keep quiet...");
		// 		}
		// 		if (self->state == dls_state_finish) {
		// 			printf("2 reco id: %d \r\n", self->id);
		// 			break;
		// 		}
		// 		usleep(30*1000); // 100ms
		// 	}
		// }

		dls_mfcc_exit(self);

		return 0;
	}

}
