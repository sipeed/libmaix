
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "libmaix_mfcc.h"

#include <list>

#define WAVE_FORMAT 0x001 // PCM
#define WAVE_CHANNELS 1
#define WAVE_SAMPLE_RATE 8000
#define WAVE_BITS_PER_SAMPLE 16
#define FRAMES_IN_BLOCK 8 * 30
#define BLOCKSIZE (FRAMES_IN_BLOCK * (WAVE_BITS_PER_SAMPLE / 8))
#define WAVE_FRAME_SIZE ((WAVE_BITS_PER_SAMPLE + 7) >> 3 * WAVE_CHANNELS) //(bits/sample + 7) / 8 * channels
#define WAVE_BYTES_PER_SECOND (WAVE_SAMPLE_RATE * WAVE_FRAME_SIZE)		  // framesize * samplerate

extern "C"
{

	typedef struct
	{
		double real;
		double imag;
	} comp;

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

		// Zaehle die benoetigten Bits zur binaeren Darstellung von n
		for (i = n; i > 1; i /= 2)
			bits++;

		// Veraendere die Indizierung fuer das Aufteilen in die Haelften
		for (i = 0; i < n; i++)
		{
			int reversed = 0;
			for (j = 0; j < bits; j++)
				reversed |= (((i >> j) % 2) << (bits - j - 1));
			freq[reversed] = time[i];
		}

		// Fuer alle Laengen, die die Haelften haben
		for (i = bits; i > 0; i--)
		{
			// Laenge der Haelften
			unsigned int length_groups = (n >> (i - 1));
			// Anzahl der Untergruppen
			unsigned int num_groups = n / length_groups;
			// Fuer alle Untergruppen
			for (j = 0; j < num_groups; j++)
			{
				// Fasse zwei Untergruppen zusammen
				for (k = 0; k < length_groups / 2; k++)
				{
					// Zahl in der ersten Haelfte
					unsigned int off1 = length_groups * j + k;
					// Zahl in der zweiten Haelfte
					unsigned int off2 = off1 + length_groups / 2;
					comp off1_val, off2_val;
					double cosine = cos((-2 * pi * k) / length_groups);
					double sine = sin((-2 * pi * k) / length_groups);
					// Multipliziere die komplexen Zahlen
					off1_val.real = freq[off1].real + (freq[off2].real * cosine - freq[off2].imag * sine);
					off1_val.imag = freq[off1].imag + (freq[off2].imag * cosine + freq[off2].real * sine);
					off2_val.real = freq[off1].real - (freq[off2].real * cosine - freq[off2].imag * sine);
					off2_val.imag = freq[off1].imag - (freq[off2].imag * cosine + freq[off2].real * sine);
					freq[off1] = off1_val;
					freq[off2] = off2_val;
				}
			}
		}
		// Normalisiere das Ergebnis
		for (i = 0; i < n; i++)
		{
			freq[i].real /= n;
			freq[i].imag /= n;
			freq[i].imag *= -1;
		}
	}

	// Asymmetrische Dreiecksfilter
	// Liste von Mittelfrequenzen; Bandbreite von links benachbarter bis rechts benachbarter Mittelfrequenz
	static int mel_filters[N_FILTER] = {150, 200, 250, 300, 350, 400, 450,	 // Linear
										490, 560, 640, 730, 840, 960, 1100,	 // 500-1000Hz Logarithmisch
										1210, 1340, 1480, 1640, 1810, 2000,	 // 1000-2000Hz Logarithmisch
										2250, 2520, 2840, 3190, 3580, 4020}; // 2000-4000Hz Logarithmisch

	unsigned int _make_frames_hamming(int16_t *buffer, unsigned int n, frame **frames)
	{
		// Die Rahmen ueberlappen sich zum Teil
		unsigned int frame_number = (n / (N - OVERLAP)) - 1;
		comp *result = (comp *)malloc(sizeof(comp) * frame_number * N);
		comp *data = (comp *)malloc(sizeof(comp) * frame_number * N);
		unsigned int pos = 0, i = 0, j = 0;
		double pi = 4 * atan(1.0);

		*frames = (frame *)malloc(sizeof(frame) * frame_number);
		// Erzeuge die Rahmen
		for (i = 0; i < frame_number; i++)
		{
			pos = (i + 1) * (N - 64);
			for (j = 0; j < N; j++)
			{
				// Filtere die Werte, sodass sie am Rand duerfen weniger Gewicht haben.
				// Die diskrete Fourier-Transformation setzt eine periodische Funktion voraus, somit wuerden die Werte am Rand des Rahmens diese verfaelschen.
				data[i * N + j].real = buffer[pos + j] * (0.54 + 0.46 * cos(2 * pi * (j - N / 2) / N));
				data[i * N + j].imag = 0;
			}
		}

		// Transformiere die einzelnen Rahmen
		for (i = 0; i < frame_number; i++)
			_fft(data + i * N, result + i * N, N);
		// Berechne die Intensitaeten und ignoriere die Phasenverschiebung
		for (i = 0; i < frame_number; i++)
			for (j = 0; j < N; j++)
				(*frames)[i].magnitudes[j] = sqrt(pow(result[i * N + j].real, 2) + pow(result[i * N + j].imag, 2));
		// Normalisiere Intensitaeten
		for (i = 0; i < frame_number; i++)
		{
			double mean = 0;
			for (j = 0; j < N; j++)
				mean += (*frames)[i].magnitudes[j];
			mean /= N;
			for (j = 0; j < N; j++)
				(*frames)[i].magnitudes[j] /= mean;
		}
		free(data);
		free(result);
		return frame_number;
	}

	// need stack > 1m
	void _mfcc_features(frame *frames, unsigned int n, mfcc_frame *mfcc_frames)
	{
		unsigned int i = 0, j = 0, k = 0;
		double filterOutput[N_FILTER];
		double filterSpectrum[N_FILTER][N] = {{0}};
		double c0 = sqrt(1.0 / N_FILTER);
		double cn = sqrt(2.0 / N_FILTER);
		double pi = 4 * atan(1.0);

		// Erzeuge fuer jeden Filter sein Spektrum.
		for (i = 0; i < N_FILTER; i++)
		{
			double maxFreq = 0, minFreq = 0, centerFreq = 0;

			// Der erste Mel-Filter besitzt unten keinen Nachbarfilter, also wird er symmetrisch gemacht.
			if (i == 0)
				minFreq = mel_filters[0] - (mel_filters[1] - mel_filters[0]);
			else
				minFreq = mel_filters[i - 1];
			centerFreq = mel_filters[i];

			// Der letzte Mel-Filter besitzt oben keinen Nachbarfilter, also wird er symmetrisch gemacht.
			if (i == N_FILTER - 1)
				maxFreq = mel_filters[N_FILTER - 1] + (mel_filters[N_FILTER - 1] - mel_filters[N_FILTER - 2]);
			else
				maxFreq = mel_filters[i + 1];

			// Berechne den Koeffizienten des Filters fuer jede Frequenz.
			for (j = 0; j < N; j++)
			{
				double freq = 1.0 * j * WAVE_SAMPLE_RATE / N;
				// Ist die Frequenz innerhalb des Filterbereiches?
				if (freq > minFreq && freq < maxFreq)
					// Bei der aufsteigenden oder absteigenden Flanke?
					if (freq < centerFreq)
						filterSpectrum[i][j] = 1.0 * (freq - minFreq) / (centerFreq - minFreq);
					else
						filterSpectrum[i][j] = 1 - 1.0 * (freq - centerFreq) / (maxFreq - centerFreq);
				else
					filterSpectrum[i][j] = 0;
			}
		}

		// Berechne die MFCC-Merkmale fuer jeden Rahmen.
		for (i = 0; i < n; i++)
		{
			// Setze jeden Filter ein und errechne die Summe.
			for (j = 0; j < N_FILTER; j++)
			{
				filterOutput[j] = 0;
				for (k = 0; k < N; k++)
					filterOutput[j] += frames[i].magnitudes[k] * filterSpectrum[j][k];
			}
			// Berechne die MFCC-Merkmale mithilfe der Summen und der diskreten Kosinus-Transformation.
			for (j = 0; j < N_MFCC; j++)
			{
				mfcc_frames[i].features[j] = 0;

				for (k = 0; k < N_FILTER; k++)
					mfcc_frames[i].features[j] += log(fabs(filterOutput[k]) + 1e-10) * cos((pi * (2 * k + 1) * j) / (2 * N_FILTER));

				if (j)
					mfcc_frames[i].features[j] *= cn;
				else
					mfcc_frames[i].features[j] *= c0;
			}
		}
	}

	void new_word(mfcc_frame *frames, unsigned int n, int id)
	{
		system("mkdir words -p");
		FILE *f = NULL;
		unsigned int i = 0, j = 0;
		chdir("words");
		char name[16] = {0};
		sprintf(name, "%d", id);
		f = fopen(name, "w");
		fprintf(f, "%u\n", n);
		// In der Datei weden einfach alle Merkmale von Leerzeichen getrennt ausgegeben
		for (i = 0; i < n; i++)
			for (j = 0; j < N_MFCC; j++)
				fprintf(f, "%lf ", frames[i].features[j]);
		fclose(f);
		chdir("..");
		// system("sync");
	}

	#include <dirent.h>

	int get_list(word *head)
	{
		system("mkdir words -p");
		DIR *d = opendir("words");
		FILE *f = NULL;
		struct dirent *dentry = readdir(d);
		int i = 0, j = 0;
		int count = 0;

		// Holen der Woerter
		chdir("words");
		while (dentry != NULL)
		{
			// Ignoriere die Standardeintraege
			if (!memcmp(dentry->d_name, ".", 1))
			{
				dentry = readdir(d);
				continue;
			}

			// Eintrag mit Daten fuellen
			f = fopen(dentry->d_name, "r");
			sscanf(dentry->d_name, "%d", &head->id);
			// printf("head->id %d\r\n", head->id);
			fscanf(f, "%u", &(head->n));
			head->frames = (mfcc_frame *)malloc(sizeof(mfcc_frame) * head->n);
			// Auslesen
			for (i = 0; i < head->n; i++)
				for (j = 0; j < N_MFCC; j++)
					fscanf(f, "%lf", &(head->frames[i].features[j]));
			fclose(f);

			dentry = readdir(d);
			count++;

			// Erzeuge neuen Eintrag, wenn benoetigt und ignoriere die Standardeintraege
			head->next = NULL;
			if (dentry != NULL && memcmp(dentry->d_name, ".", 2) && memcmp(dentry->d_name, "..", 3))
			{
				head->next = (word *)malloc(sizeof(word));
				head = head->next;
			}
		}
		chdir("..");

		return count;
	}

	double compare(const mfcc_frame *mfcc_frames1, unsigned int n1, const mfcc_frame *mfcc_frames2, unsigned int n2)
	{
		double distances[n1 + 1][n2 + 1];
		unsigned int i = 0, j = 0, k = 0;

		// Berechne die lokalen Distanzen aller Paare von MFCC-Merkmalen
		for (i = 0; i < n1; i++)
		{
			for (j = 0; j < n2; j++)
			{
				distances[i + 1][j + 1] = 0;
				for (k = 0; k < N_MFCC; k++)
					distances[i + 1][j + 1] += pow(mfcc_frames1[i].features[k] - mfcc_frames2[j].features[k], 2);
				distances[i + 1][j + 1] = sqrt(distances[i + 1][j + 1]);
			}
		}

		// Fuelle die Raender mit Unendlich
		for (i = 0; i <= n1; i++)
			distances[i][0] = atof("Inf");
		for (i = 0; i <= n2; i++)
			distances[0][i] = atof("Inf");
		// Der einzig gueltige Startwert am Rand
		distances[0][0] = 0;

		// Berechne den guenstigsten Weg von einem zum anderen Ende der Matrix
		for (i = 1; i <= n1; i++)
			for (j = 1; j <= n2; j++)
			{
				// Bestimme den guenstigsten Vorgaenger
				double prev_min = distances[i - 1][j];
				if (distances[i - 1][j - 1] < prev_min)
					prev_min = distances[i - 1][j - 1];
				if (distances[i][j - 1] < prev_min)
					prev_min = distances[i][j - 1];
				// Fuehre den Schritt aus
				distances[i][j] += prev_min;
			}

		// Normalisiere die Distanz
		return distances[n1][n2] / sqrt(pow(n1, 2) + pow(n2, 2));
	}

	static char header[45] = "RIFF1234WAVEfmt 12341212123412341212data1234";

	void prepare_header(const unsigned int size)
	{
		unsigned int values[9] = {size - 8, 16, WAVE_FORMAT, WAVE_CHANNELS, WAVE_SAMPLE_RATE, WAVE_BYTES_PER_SECOND, WAVE_FRAME_SIZE, WAVE_BITS_PER_SAMPLE, size - 44};

		memcpy(header + 4, values, 4);
		memcpy(header + 16, values + 1, 4);
		memcpy(header + 20, values + 2, 2);
		memcpy(header + 22, values + 3, 2);
		memcpy(header + 24, values + 4, 4);
		memcpy(header + 28, values + 5, 4);
		memcpy(header + 32, values + 6, 2);
		memcpy(header + 34, values + 7, 2);
		memcpy(header + 40, values + 8, 4);
	}

	void dls_mfcc_reset(dls_mfcc *self)
	{
		self->state = dls_state_init;
	}

	void dls_mfcc_save(dls_mfcc *self, int id)
	{
		self->frame_id = id;
		self->state = dls_state_save;
	}

	void dls_mfcc_free(dls_mfcc *self)
	{
		if (self->words)
		{
			for (word *tmp, *words = self->words; words != NULL;)
			{
				tmp = words, words = words->next, free(tmp);
			}
			self->words = NULL;
		}
	}

	void dls_mfcc_load(dls_mfcc *self)
	{
		dls_mfcc_free(self);
		self->words = (word *)malloc(sizeof(word));
		if (self->words)
		{
			self->words_size = get_list(self->words);
			if (0 == self->words_size) free(self->words), self->words = NULL;
		}
	}

	void dls_mfcc_event(dls_mfcc *self)
	{
		if (self->state == dls_state_destroy) return ;

		struct _dls_mfcc_event_frame
		{
			uint8_t frame[BLOCKSIZE];
		};

		static std::list<_dls_mfcc_event_frame> _mfcc_frames;

		static bool is_save = false;

		switch (self->state)
		{
			case dls_state_save:
				is_save = true;
				self->state = dls_state_init;
				break;
			case dls_state_init:
			{
				_mfcc_frames.clear();
				self->state = dls_state_idle;
			}
			case dls_state_idle:
			{
				int vadres, prev = -1;
				long frames[2] = {0, 0};
				long segments[2] = {0, 0};
				uint8_t voice[BLOCKSIZE] = {0};

				for (int i = 0; i < 20; i++)
				{ // 20 * 30 ms = 600ms check noise
					int r = snd_pcm_readi(self->handle, voice, FRAMES_IN_BLOCK);
					if (r < 0)
					{
						// fprintf(stderr, "WARNUNG: snd_pcm_readi %d\n", r);
					}
					else
					{
						vadres = fvad_process(self->vad, (const int16_t *)(voice), FRAMES_IN_BLOCK);
						if (vadres < 0)
						{
							// fprintf(stderr, "VAD processing failed\n");
						}
						vadres = !!vadres; // make sure it is 0 or 1
						frames[vadres]++;
						if (prev != vadres)
							segments[vadres]++;
						prev = vadres;
					}
				}

				if (frames[1] < 5)
				{
					self->state = dls_state_ready;
					_mfcc_frames.clear();
					// fprintf(stderr, "dls_state_ready\n");
				}

				// printf("1 voice detected in %ld of %ld frames (%.2f%%)\n",
				// 	frames[1], frames[0] + frames[1],
				// 	frames[0] + frames[1] ?
				// 		100.0 * ((double)frames[1] / (frames[0] + frames[1])) : 0.0);

				break;
			}
			case dls_state_ready:
			{
				int vadres, prev = -1;
				long frames[2] = {0, 0};
				long segments[2] = {0, 0};
				for (int i = 0; i < 20; i++)
				{ // 20 * 30 ms = 600ms check voice
					_dls_mfcc_event_frame tmp;
					int r = snd_pcm_readi(self->handle, tmp.frame, FRAMES_IN_BLOCK);
					if (r < 0)
					{
						// fprintf(stderr, "WARNUNG: snd_pcm_readi %d\n", r);
					}
					else
					{
						vadres = fvad_process(self->vad, (const int16_t *)(tmp.frame), FRAMES_IN_BLOCK);
						if (vadres < 0)
						{
							// fprintf(stderr, "VAD processing failed\n");
						}
						vadres = !!vadres; // make sure it is 0 or 1
						frames[vadres]++;
						if (prev != vadres)
							segments[vadres]++;
						prev = vadres;
						_mfcc_frames.push_back(tmp);
					}
				}

				if (frames[1] > 10)
				{ // have voice
					self->state = dls_state_work;
					// fprintf(stderr, "dls_state_work\n");
				}
				else
				{
					if (_mfcc_frames.size() >= 20)
					{ // 600ms no-voice to clear
						_mfcc_frames.clear();
						self->state = dls_state_ready;
					}
				}

				// printf("2 voice detected in %ld of %ld frames (%.2f%%)\n",
				// 	frames[1], frames[0] + frames[1],
				// 	frames[0] + frames[1] ?
				// 		100.0 * ((double)frames[1] / (frames[0] + frames[1])) : 0.0);

				break;
			}
			case dls_state_work:
			{
				int vadres, prev = -1;
				long frames[2] = {0, 0};
				long segments[2] = {0, 0};
				for (int i = 0; i < 30; i++)
				{ // 30 * 30 ms = 600ms check voice to stop
					_dls_mfcc_event_frame tmp;
					int r = snd_pcm_readi(self->handle, tmp.frame, FRAMES_IN_BLOCK);
					if (r < 0)
					{
						// fprintf(stderr, "WARNUNG: snd_pcm_readi %d\n", r);
					}
					else
					{
						vadres = fvad_process(self->vad, (const int16_t *)(tmp.frame), FRAMES_IN_BLOCK);
						if (vadres < 0)
						{
							// fprintf(stderr, "VAD processing failed\n");
						}
						vadres = !!vadres; // make sure it is 0 or 1
						frames[vadres]++;
						if (prev != vadres)
							segments[vadres]++;
						prev = vadres;
						_mfcc_frames.push_back(tmp);
					}
				}
				if (frames[1] < 10)
				{ // no voice
					self->state = dls_state_finish;
					// fprintf(stderr, "dls_state_finish\n");
				}
				else
				{
					if (_mfcc_frames.size() > 150)
					{ // mfcc voice max 5 * 600ms 3000ms
						self->state = dls_state_init;
					}
				}

				// printf("3 voice detected in %ld of %ld frames (%.2f%%)\n",
				//     frames[1], frames[0] + frames[1],
				//     frames[0] + frames[1] ?
				//         100.0 * ((double)frames[1] / (frames[0] + frames[1])) : 0.0);

				break;
			}
			case dls_state_finish:
			{
				const unsigned int size = BLOCKSIZE * _mfcc_frames.size();
				int16_t *buffer = (int16_t *)malloc(size);
				uint32_t buflen = ((int)(((size) / N)) * N) / 2; // uint8_t-> int16_t
				// printf("size %d buflen %d\r\n", size, buflen);
				if (buffer)
				{
					uint8_t *tmp = (uint8_t *)buffer;
					for (auto it = _mfcc_frames.begin(); it != _mfcc_frames.end(); ++it)
					{
						memcpy(tmp, it->frame, BLOCKSIZE), tmp += BLOCKSIZE;
					}
					self->frames = NULL;
					self->mfcc_frames = NULL;
					self->frame_n = _make_frames_hamming(buffer, buflen, &self->frames);
					self->mfcc_frames = (mfcc_frame *)malloc(sizeof(mfcc_frame) * self->frame_n);
					if (self->mfcc_frames)
					{
						_mfcc_features(self->frames, self->frame_n, self->mfcc_frames);
						// printf("frames %p mfcc_frames %p frame_n %d words %p words_size %d\r\n", self->frames, self->mfcc_frames, self->frame_n, self->words, self->words_size);
						if (is_save)
						{
							{
								char *path = "/tmp/replay.wav";
								FILE *file = NULL;
								//Dateideskriptor zuweisen
								if (path != NULL) {
									file = fopen(path, "w");
									if (file != NULL) {
										const unsigned int size = BLOCKSIZE * _mfcc_frames.size();
										//Header vorbereiten
										prepare_header(size + 44);
										//Schreiben
										fwrite(header, 44, 1, file);
										for (auto it = _mfcc_frames.begin(); it != _mfcc_frames.end(); ++it) {
											fwrite(it->frame, BLOCKSIZE, 1, file);
										}
										fclose(file);
										system("aplay /tmp/replay.wav");
									}
								}
							}
							is_save = false;
							new_word(self->mfcc_frames, self->frame_n, self->frame_id);
							dls_mfcc_load(self);
						}
						else
						{
							if (self->words && self->words_size != 0)
							{
								double best = self->frame_min;
								int id = -1;
								for (word *words = self->words; words != NULL; words = words->next)
								{
									double now = compare(self->mfcc_frames, self->frame_n, words->frames, words->n);
									printf("words %p id %d now %f best %f (%f, %f)\r\n", words, words->id, now, best, self->frame_min, self->frame_max), fflush(NULL);
									if (now < best)
										best = now, id = words->id;
								}
								if (best < self->frame_max)
								{
									self->frame_id = id;
									printf("maybe is %d %f \r\n", id, best), fflush(NULL);
								}
							}
						}
						self->state = dls_state_result;
						free(self->mfcc_frames);
					}
					free(self->frames);
					free(buffer);
				}
				break;
			}
			case dls_state_result:
			case dls_state_error:
			case dls_state_destroy:
				break;
		}
	}

	void _dls_mfcc_event(dls_mfcc *self)
	{
		while (self->state != dls_state_destroy)
		{
			usleep(10000); // 10ms
			dls_mfcc_event(self);
		}
	}

	dls_mfcc *dls_mfcc_init(const char *device, float min, float max)
	{
		dls_mfcc *self = (dls_mfcc *)malloc(sizeof(dls_mfcc));
		if (!self) return NULL;

		memset(self, 0, sizeof(*self));

		self->frame_min = min, self->frame_max = max;

		unsigned int sample_rate = WAVE_SAMPLE_RATE;
		snd_pcm_hw_params_t *hw_params = NULL;
		self->frame_id = dls_state_error;
		self->state = dls_state_error;
		// Oeffnen des Handle zum Aufnehmen
		if (snd_pcm_open(&self->handle, !device ? "default" : device, SND_PCM_STREAM_CAPTURE, 0) < 0)
		{
			fprintf(stderr, "Konnte Geraet %s nicht oeffnen\n", !device ? "default" : device);
			return NULL;
		}
		// Speicher fuer Hardware-Parameter reservieren
		if (snd_pcm_hw_params_malloc(&hw_params) < 0)
		{
			fprintf(stderr, "Fehler beim Reservieren von Speicher fuer die Hardware-Parameter\n");
			return NULL;
		}
		// Standard Hardware-Parameter erfragen
		if (snd_pcm_hw_params_any(self->handle, hw_params) < 0)
		{
			fprintf(stderr, "Fehler beim Holen der Standard-Hardware-Parameter\n");
			return NULL;
		}
		// Setzen der Hardware-Parameter innerhalb der Struktur
		if (snd_pcm_hw_params_set_access(self->handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0)
		{
			fprintf(stderr, "Fehler beim Setzen des Lese-\\Schreibe-Zugriffs\n");
			return NULL;
		}
		if (snd_pcm_hw_params_set_format(self->handle, hw_params, SND_PCM_FORMAT_S16_LE) < 0)
		{
			fprintf(stderr, "Fehler beim Setzen des PCM-Formates\n");
			return NULL;
		}
		if (snd_pcm_hw_params_set_rate_near(self->handle, hw_params, &sample_rate, 0) < 0)
		{
			fprintf(stderr, "Fehler beim Setzen der Frequenz\n");
			return NULL;
		}
		if (snd_pcm_hw_params_set_channels(self->handle, hw_params, 1) < 0)
		{
			fprintf(stderr, "Fehler beim Setzen der Anzahl der Kanaele\n");
			return NULL;
		}
		// Setzen der Hardware-Parameter beim Handle
		if (snd_pcm_hw_params(self->handle, hw_params) < 0)
		{
			fprintf(stderr, "Fehler beim Setzen der Hardwareparameter\n");
			return NULL;
		}
		// Speicher freigeben
		snd_pcm_hw_params_free(hw_params);
		// Hardware vorbereiten
		if (snd_pcm_prepare(self->handle) < 0)
		{
			fprintf(stderr, "Fehler beim Vorbereiten des Handles\n");
			return NULL;
		}
		self->vad = fvad_new();
		// Set aggressiveness mode
		fvad_set_mode(self->vad, 3); // Mode 3, Very aggressive. // alsamixer set 70 ~ 90
		if (fvad_set_sample_rate(self->vad, sample_rate) < 0)
		{
			fprintf(stderr, "invalid sample rate: %d Hz\n", sample_rate);
		}


		// pthread_attr_init(&self->tattr);
		// self->stack = malloc(64 * 1024); // 64k for _dls_mfcc_event
		// pthread_attr_setstack(&self->tattr, self->stack, 64 * 1024);

		// // Starten des Thread
		// if (pthread_create(&self->_thread, &self->tattr, (void *(*)(void *))_dls_mfcc_event, self) < 0)
		// {
		// 	return dls_state_error;
		// }

		dls_mfcc_load(self);
		return self;
	}

	void dls_mfcc_exit(dls_mfcc **self)
	{
		(*self)->state = dls_state_destroy;
		// pthread_join(self->_thread, NULL);
		// //
		// pthread_attr_destroy(&self->tattr);
		// free(self->stack);
		// Handle schliessen
		snd_pcm_close((*self)->handle);
		if ((*self)->vad)
			fvad_free((*self)->vad);
		//
		dls_mfcc_free(*self);
		//
		if (*self) free(*self), *self = NULL;
	}

	int unit_test_mfcc()
	{
		printf("dls_mfcc_init\r\n");
		dls_mfcc * self = dls_mfcc_init(NULL, 5.0, 3.0);

		char ans[16];
		printf("\r\n0 Enter identifier number (0, 1, 2) (x to skip): \r\n"), fflush(NULL);
		scanf("%s", (char *)ans);
		if (!(ans[0] == 'x' && ans[1] == '\0'))
		{
			int id = atoi(ans);
			printf("0 identifier: %d \r\n", id);

			dls_mfcc_save(self, id);
			while (1)
			{
				dls_mfcc_event(self);
				if (self->state == dls_state_idle)
				{
					printf("0 keep quiet...\r\n");
				}
				if (self->state == dls_state_ready)
				{
					printf("0 speak, please...\r\n");
				}
				if (self->state == dls_state_result)
				{
					printf("0 save id: %d \r\n", id), fflush(NULL);
					break;
				}
				usleep(10 * 1000); // 10ms
			}
		}

		printf("\r\n1 Enter recognition 3 times \r\n"), fflush(NULL);
		for (int i = 0; i < 1; i++)
		{
			printf("1 recognition... \r\n");
			dls_mfcc_reset(self);
			while (1)
			{
				dls_mfcc_event(self);
				if (self->state == dls_state_idle)
				{
					printf("1 keep quiet...\r\n");
				}
				if (self->state == dls_state_ready)
				{
					printf("1 speak, please...\r\n");
				}
				if (self->state == dls_state_result)
				{
					printf("1 reco id: %d \r\n", self->frame_id);
					break;
				}
				usleep(10 * 1000); // 10ms
			}
		}

		dls_mfcc_exit(&self);
		printf("dls_mfcc_exit\r\n");

		return 0;
	}
}
