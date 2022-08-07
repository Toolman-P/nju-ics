#include <common.h>
#include <device/map.h>
#include <SDL2/SDL.h>
#define min(a,b) ((a)<(b)?(a):(b))
enum {
  reg_freq,
  reg_channels,
  reg_samples,
  reg_sbuf_size,
  reg_init,
  reg_count,
  nr_reg
};

static uint8_t *sbuf = NULL;
static uint8_t *queue = NULL;
static uint32_t *audio_base = NULL;
static uint32_t front = 0,rear = 0;

static inline void enqueue(uint8_t *buf){
  if(front==rear) queue[front]=*buf;
  else{
    rear = (rear+1)%(CONFIG_SB_SIZE+1);
    queue[rear]=*buf;
  }
  audio_base[reg_count]++;
}

static inline uint8_t dequeue(){
  assert(audio_base[reg_count]>0);
  uint8_t ret = queue[front];
  front=(front+1)%(CONFIG_SB_SIZE+1);
  audio_base[reg_count]--;
  return ret;
}

static void sdl_audio_callback(void *userdata,uint8_t *stream,int len){
  uint32_t count = min(audio_base[reg_count],len);
  uint32_t i;
  for(i=0;i<count;i++)
    stream[i]=dequeue();
  for(;i<len;i++)
    stream[i]=0;
}

static int init_audio_output(){

  audio_base[reg_count]=0;
  
  SDL_AudioSpec want;
  want.freq = audio_base[reg_freq];
  want.channels = audio_base[reg_channels];
  want.samples = audio_base[reg_samples];
  want.size = 2048;
  want.format = AUDIO_S16SYS;
  want.userdata = NULL;
  want.callback = sdl_audio_callback;
  SDL_InitSubSystem(SDL_INIT_AUDIO);
  SDL_OpenAudio(&want,NULL);
  SDL_PauseAudio(0);
  return 0;
}

static void audio_io_handler(uint32_t offset, int len, bool is_write) {
  assert(len==4);
  switch(offset){
    case reg_freq:
    case reg_channels:
    case reg_samples:
      assert(is_write);
      break;
    case reg_sbuf_size:
    case reg_count:
      assert(!is_write);
      break;
    case reg_init:
      assert(is_write);
      init_audio_output();
      break;
    default:
      return;
  }
}

static void audio_sbuf_write_handler(uint32_t offset, int len, bool is_write){
  assert(len==1);
  assert(is_write);
  enqueue(sbuf+offset);
}

void init_audio() {
  uint32_t space_size = sizeof(uint32_t) * nr_reg;
  audio_base = (uint32_t *)new_space(space_size);
  
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size, audio_io_handler);
#else
  add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size, audio_io_handler);
#endif

  sbuf = (uint8_t *)new_space(CONFIG_SB_SIZE);
  queue = (uint8_t *)new_space(CONFIG_SB_SIZE+1);
  audio_base[reg_sbuf_size]=CONFIG_SB_SIZE;
  add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, audio_sbuf_write_handler);
}
