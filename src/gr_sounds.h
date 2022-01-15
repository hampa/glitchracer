
#ifndef GR_SOUNDS_H
#define GR_SOUNDS_H

#define NUM_SOUNDS 2
struct sounds_t {
        int id;
        char *name;
};

enum { SND_PAIN1, SND_PAIN2 };

struct sounds_t sound_effects[]={
        { SND_PAIN1,	"pain1.wav" },
        { SND_PAIN2,   	"pain2.wav" }
};

#endif

