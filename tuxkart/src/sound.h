
#define SOUND_UGH		0
#define SOUND_BOING		1
#define SOUND_BONK		2
#define SOUND_BURP		3
#define SOUND_LASER		4
#define SOUND_OW		5
#define SOUND_WEE		6
#define SOUND_EXPLOSION         7
#define SOUND_BZZT              8
#define SOUND_BEEP              9
#define SOUND_SHOOMF           10
 
class SoundSystem
{
  char current_track [ 256 ] ;
  slScheduler *sched ;

public:
  SoundSystem () ;

  void update () ;
  void playSfx ( int sound ) ;

  void setSafetyMargin ( float t = 0.25 )
  {
    sched -> setSafetyMargin ( t ) ;
  }

  void  change_track ( char *fname ) ;
  void disable_music () ;
  void  enable_music () ;

  void disable_sfx   () ;
  void  enable_sfx   () ;
} ;

