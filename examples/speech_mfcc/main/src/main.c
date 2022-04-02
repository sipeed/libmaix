
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include "time.h"

// .h

typedef enum
{
	dls_mfcc_idle,
	dls_mfcc_ready,
	dls_mfcc_work,
	dls_mfcc_finish,
	dls_mfcc_reco,
	dls_mfcc_save,
	dls_mfcc_destroy,
} dls_mfcc_state;

struct dls_mfcc_data
{
	dls_mfcc_state state, event;
	pthread_t _thread;
} _dls_mfcc_data;

void dls_mfcc_set(dls_mfcc_state state);

dls_mfcc_state dls_mfcc_get(); // dls_mfcc_ready \ dls_mfcc_work \ dls_mfcc_finish

dls_mfcc_state dls_mfcc_event(); // dls_mfcc_reco \ dls_mfcc_save \ dls_mfcc_idle

/*

dls_mfcc_init();

if (dls_mfcc_ready == dls_mfcc_get()) {
  // no-noise try work voice

  dls_mfcc_set(dls_mfcc_reco); // reco currert voice

  dls_mfcc_set(dls_mfcc_save); // save currert voice

}

if (dls_mfcc_work == dls_mfcc_get()) {

  if (dls_mfcc_reco == dls_mfcc_event()) {
	// recoing
  }

  if (dls_mfcc_save == dls_mfcc_event()) {
	// studying
  }

}

if (dls_mfcc_finish == dls_mfcc_get()) {

  if (dls_mfcc_reco == dls_mfcc_event()) {
	// recoed get result
  }

  if (dls_mfcc_save == dls_mfcc_event()) {
	// saved get result
  }

}

dls_mfcc_exit();

*/

// .c

void _dls_mfcc_load()
{
}

void _dls_mfcc_save()
{
	_dls_mfcc_load();
}

void _dls_mfcc_free()
{
}

void _dls_mfcc_reco()
{
	while (1)
	{
		switch (_dls_mfcc_data.state)
		{
		case dls_mfcc_idle:
			/* code */
			break;
		case dls_mfcc_ready:
			/* code */
			break;
		case dls_mfcc_work:
			/* code */
			break;
		case dls_mfcc_finish:
			/* code */
			break;
		case dls_mfcc_destroy:
			/* code */
		default:
			return;
		}
		usleep(30000);
	}
}

void dls_mfcc_set(dls_mfcc_state state)
{
	_dls_mfcc_data.state = state;
}

dls_mfcc_state dls_mfcc_get()
{
	return _dls_mfcc_data.state;
}

dls_mfcc_state dls_mfcc_event()
{
	return _dls_mfcc_data.event;
}

void dls_mfcc_config()
{
	; //
}

void *dls_mfcc_result()
{
	if (_dls_mfcc_data.state == dls_mfcc_finish)
		_dls_mfcc_data.event = dls_mfcc_idle;
	// return _dls_mfcc_data.result;
	return NULL;
}

int dls_mfcc_init()
{
	_dls_mfcc_data.state = dls_mfcc_idle;
	if (pthread_create(&_dls_mfcc_data._thread, NULL, (void *)_dls_mfcc_reco, NULL) < 0)
	{
		return -10;
	}
}

void dls_mfcc_exit()
{
	_dls_mfcc_data.state = dls_mfcc_destroy;
	pthread_join(_dls_mfcc_data._thread, NULL);
	_dls_mfcc_free();
}

#include "split.h"
#include "frame.h"
#include "list.h"
#include "compare.h"
#include "wave.h"
#include "audio.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

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

void words_capture(int how);

int main(void)
{
	dls_mfcc_init();

	char ans[50];
	do
	{
		printf("(n)ew words, (l)ist words, (d)ictate words, (e)xit: "), fflush(NULL);
		scanf("%s", (char *)ans);
		switch (ans[0])
		{
		case 'n':
			words_capture(0);
			break;
		case 'l':
			system("ls words");
			break;
		case 'd':
			words_capture(1);
			break;
		}
	} while (ans[0] != 'e');

	dls_mfcc_exit();

	return 0;
}

void words_capture(int how)
{
	system("mkdir waves words -p");
	void *buf = NULL;
	unsigned int size = 0, n = 0, i = 0;
	char ans[500];
	voice_signal *signals = NULL;
	printf("Please speak now (end with return)\n"), fflush(NULL);

	// capture_start(NULL);
	// getchar();
	// getchar();
	// capture_stop(&size, &buf);

	vad_capture_start(NULL);
	vad_capture_stop(&size, &buf);

	n = split(buf, size / 2, &signals);
	printf("split size %d n = %d\n", size / 2, n), fflush(NULL);
	for (i = 0; i < n; i++)
	{
		frame *frames = NULL;
		mfcc_frame *mfcc_frames = NULL;
		int frame_n = make_frames_hamming(signals[i].buffer, signals[i].number, &frames);
		mfcc_frames = malloc(sizeof(mfcc_frame) * frame_n);
		mfcc_features(frames, frame_n, mfcc_frames);
		if (!how)
		{
			// play(NULL, signals[i].buffer, signals[i].number * 2);
			printf("Enter identifier (x to skip): "), fflush(NULL);
			scanf("%s", (char *)ans);
			if (!(ans[0] == 'x' && ans[1] == '\0'))
			{
				new_word(mfcc_frames, frame_n, (char *)ans);
				chdir("waves");
				char *path = malloc(strlen(ans) + 5);
				char *ext = ".wav";
				memcpy(path, ans, strlen(ans));
				memcpy(path + strlen(ans), ext, 5);
				write_pcm(signals[i].buffer, signals[i].number * 2, path);
				char tmp[64] = {0};
				sprintf(tmp, "aplay %s", path);
				system(tmp);
				free(path);
				chdir("..");
			}
		}
		else
		{
			word *words = malloc(sizeof(word));
			double best = 1e10;
			char *name = NULL;
			void **word_adresses;
			unsigned int n = 0, i = 0, count = 0;
			if ((n = get_list(words)))
			{
				word_adresses = malloc(n * sizeof(void *));
				while (words != NULL)
				{
					double now = compare(mfcc_frames, frame_n, words->frames, words->n);
					word_adresses[count++] = words;
					if (now < best)
					{
						best = now;
						name = words->name;
					}
					words = words->next;
				}
				for (i = 0; i < count; i++)
					free(word_adresses[i]);
				free(word_adresses);
			}
			if (best < 3.5)
			{
				printf("maybe is %s %f \r\n", name, best), fflush(NULL);
			}
		}
		free(mfcc_frames);
		free(frames);
	}
	if (how)
		printf("\n"), fflush(NULL);
	free(buf);
}
