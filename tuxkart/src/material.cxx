
#include "tuxkart.h"


ssgSimpleState *grass_gst, *dirt_gst, *stone1_gst, *mnm_gst,
               *clouds_gst, *aarmco_gst, *roadway_gst, *adverts_gst,
               *concrete_gst, *penguin_gst, *fuzzy_gst, *herring_gst,
               *zipper_gst, *spark_gst, *missile_gst, *candystripe_gst,
               *explode_gst, *flamemissile_gst, *players_gst, *magnet_gst,
               *brick_gst, *grid_gst, *lava_gst, *stone_gst,
               *pebble_gst, *floor_gst, *railing_gst, *wood_gst,
               *tinytux_gst, *butterfly_gst, *sandstorm_gst, *sand_gst,
               *pyramidwall_gst, *egypt_gst, *flames_gst, *ruler_gst,
               *bzzt_gst, *herringbones_gst, *goldwall_gst, *rainbow_gst ;

ssgSimpleState *blank_gst ;

#define NOCLAMP 0
#define UCLAMP  1
#define VCLAMP  2
#define UVCLAMP 3

Material gs_setup [] =
{
  /*    gst          texture_map          clamp , trans,aref,light,frctn,0 */
  { &candystripe_gst,
               "images/candy_stripe.rgb", NOCLAMP,FALSE,0.0, TRUE ,1.0, MAT_CRASH },
  { &aarmco_gst  , "images/aarmco.rgb"  , VCLAMP ,TRUE ,0.5, TRUE ,1.0, 0 },
  { &railing_gst , "images/railing.rgb" , VCLAMP ,TRUE ,0.5, TRUE ,1.0, 0 },
  { &ruler_gst   , "images/ruler.rgb"   , NOCLAMP,TRUE ,0.2, TRUE ,1.0, 0 },
  { &goldwall_gst,
           "images/embossed_herring.rgb", NOCLAMP,TRUE ,0.5, TRUE ,1.0, 0 },

  { &adverts_gst , "images/adverts.rgb" , NOCLAMP,FALSE,0.0, TRUE ,1.0, MAT_CRASH },
  { &wood_gst    , "images/wood.rgb"    , NOCLAMP,FALSE,0.0, TRUE ,1.0, 0 },
  { &concrete_gst, "images/concrete.rgb", NOCLAMP,FALSE,0.0, TRUE ,1.0, MAT_CRASH },
  { &brick_gst   , "images/brick.rgb"   , NOCLAMP,FALSE,0.0, TRUE ,1.0, MAT_CRASH },
  { &stone_gst   , "images/stonewall.rgb",NOCLAMP,FALSE,0.0, TRUE ,1.0, 0 },
  { &pebble_gst  , "images/pebbles.rgb" , NOCLAMP,FALSE,0.0, TRUE ,1.0, 0 },
  { &floor_gst   , "images/floor.rgb"   , NOCLAMP,FALSE,0.0, TRUE ,1.0, 0 },
  { &sand_gst    , "images/sand.rgb"    , NOCLAMP,FALSE,0.0, TRUE ,1.0, 0 },
  { &egypt_gst   , "images/egypt.rgb"   , NOCLAMP,FALSE,0.0, TRUE ,1.0, MAT_CRASH },
  { &pyramidwall_gst,
                   "images/pyramidwall.rgb",
                                          NOCLAMP,FALSE,0.0, TRUE ,1.0, 0 },
  { &lava_gst    , "images/lava.rgb"    , NOCLAMP,FALSE,0.0, FALSE,1.0, MAT_RESET },
  { &grid_gst    , "images/metalgrid.rgb",NOCLAMP,TRUE ,0.3, TRUE ,1.0, 0 },
  { &sandstorm_gst,"images/fuzzy_sand.rgb",NOCLAMP,TRUE,0.0, FALSE,1.0, MAT_IGN },
  { &roadway_gst , "images/roadway.rgb" , UCLAMP ,TRUE ,0.0, TRUE ,1.0, 0 },
  { &rainbow_gst , "images/rainbow.rgb" , NOCLAMP,TRUE ,0.0, FALSE,1.0, 0 },
  { &tinytux_gst , "images/tinytux.rgb" , UVCLAMP,TRUE ,0.8, FALSE,1.0, MAT_IGN },
  { &butterfly_gst,"images/butterfly.rgb",UVCLAMP,TRUE ,0.8, FALSE,1.0, MAT_IGN },
  { &penguin_gst ,
               "images/Penguin_orig.rgb", NOCLAMP,FALSE,0.0, TRUE ,1.0, 0 },
  { &grass_gst   , "images/grass.rgb"   , NOCLAMP,FALSE,0.0, TRUE ,1.0, 0 },
  { &zipper_gst  , "images/zipper.rgb"  , NOCLAMP,FALSE,0.0, FALSE,1.0,MAT_ZIP},
  { &mnm_gst     , "images/mnm.rgb"     , UVCLAMP,TRUE ,0.5, FALSE,1.0,MAT_ZIP},
  { &fuzzy_gst   , "images/fuzzy.rgb"   , UVCLAMP,TRUE ,0.2, FALSE,0.0,MAT_IGN},
  { &spark_gst   , "images/spark.rgb"   , UVCLAMP,FALSE,0.2, FALSE,0.0,MAT_IGN},
  { &explode_gst , "images/explode.rgb" , UVCLAMP,TRUE ,0.2, FALSE,0.0,MAT_IGN},
  { &players_gst , "images/players.rgb" , UVCLAMP,TRUE ,0.9, FALSE,0.0,MAT_IGN},
  { &missile_gst , "images/missile.rgb" , UVCLAMP,TRUE ,0.9, FALSE,0.0,MAT_IGN},
  { &flames_gst  , "images/flames.rgb"  , NOCLAMP,FALSE,0.9, FALSE,0.0,MAT_IGN},
  { &magnet_gst  , "images/magnet.rgb"  , UVCLAMP,TRUE ,0.1, FALSE,0.0,MAT_IGN},
  { &bzzt_gst    , "images/bzzt.rgb"    , NOCLAMP,TRUE ,0.1, FALSE,0.0,MAT_IGN},
  { &flamemissile_gst,
               "images/flamemissile.rgb", UVCLAMP,TRUE ,0.9, FALSE,0.0,MAT_IGN},
  { &herring_gst , "images/herring.rgb" , UVCLAMP,TRUE ,0.9, FALSE,0.0,MAT_IGN},
  { &herringbones_gst,
               "images/herringbones.rgb", UVCLAMP,TRUE ,0.2, FALSE,0.0,MAT_IGN},
  { &blank_gst   , ""                   , NOCLAMP,FALSE,0.0, TRUE ,1.0,MAT_CRASH},
  { NULL, "", FALSE, FALSE, 0.0, FALSE, 1.0, 0 }
} ;


void Material::install ( int index )
{
  *gst = new ssgSimpleState ;

  (*gst) -> ref () ;
  (*gst) -> setExternalPropertyIndex ( index ) ;

  if ( texture_map [ 0 ] != '\0' )
  {
    (*gst) -> setTexture ( texture_map, !(clamp_tex & UCLAMP),
                                        !(clamp_tex & VCLAMP) ) ;
    (*gst) -> enable ( GL_TEXTURE_2D ) ;
  }
  else
    (*gst) -> disable ( GL_TEXTURE_2D ) ;

  if ( lighting )
    (*gst) -> enable ( GL_LIGHTING ) ;
  else
    (*gst) -> disable ( GL_LIGHTING ) ;

  (*gst) -> setShadeModel ( GL_SMOOTH ) ;
  (*gst) -> enable ( GL_COLOR_MATERIAL ) ;
  (*gst) -> enable ( GL_CULL_FACE      ) ;
  (*gst) -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;
  (*gst) -> setMaterial ( GL_EMISSION, 0, 0, 0, 1 ) ;
  (*gst) -> setMaterial ( GL_SPECULAR, 0, 0, 0, 1 ) ;
  (*gst) -> setShininess ( 0 ) ;

  if ( transparency )
  {
    (*gst) -> setTranslucent () ;
    (*gst) -> enable ( GL_ALPHA_TEST ) ;
    (*gst) -> setAlphaClamp ( alpha_ref ) ;
    (*gst) -> enable ( GL_BLEND ) ;
  }
  else
  {
    (*gst) -> setOpaque () ;
    (*gst) -> disable ( GL_BLEND ) ;
  }
}



ssgState *getAppState ( char *fname )
{
  if ( fname == NULL || fname[0] == '\0' )
{
printf("No texture.\n");
    return gs_setup [ 0 ] . getState() ;
}

  char *fn ;

  /* Remove all leading path information. */

  for ( fn = & fname [ strlen ( fname ) - 1 ] ; fn != fname &&
                                               *fn != '/' ; fn-- )
    /* Search back for a '/' */ ;

  if ( *fn == '/' )
    fn++ ;

  char basename [ 1024 ] ;

  strcpy ( basename, fn ) ;

  /* Remove last trailing extension. */

  for ( fn = & basename [ strlen ( basename ) - 1 ] ; fn != basename &&
                                                     *fn != '.' ; fn-- )
    /* Search back for a '.' */ ;

  if ( *fn == '.' )
    *fn = '\0' ;

  for ( int i = 0 ; ! gs_setup [ i ] . isNull () ; i++ )
  {
    char *fname2 = gs_setup [ i ] . getTexFname () ;

    if ( fname2 != NULL && fname2[0] != '\0' )
    {
      char *fn2 ;

      /* Remove all leading path information. */

      for ( fn2 = & fname2 [ strlen ( fname2 ) -1 ] ; fn2 != fname2 &&
                                                     *fn2 != '/' ; fn2-- )
        /* Search back for a '/' */ ;

      if ( *fn2 == '/' )
        fn2++ ;

      char basename2 [ 1024 ] ;

      strcpy ( basename2, fn2 ) ;

      /* Remove last trailing extension. */

      for ( fn2 = & basename2 [ strlen ( basename2 ) - 1 ] ; fn2 != basename2 &&
                                                         *fn2 != '.' ; fn2-- )
        /* Search back for a '.' */ ;

      if ( *fn2 == '.' )
        *fn2 = '\0' ;

      if ( strcmp ( basename, basename2 ) == 0 )
        return gs_setup [ i ] . getState() ;
    }
  }

  strcpy ( fname, basename  ) ;
  strcat ( fname, ".rgb"    ) ;
  return NULL ;
}



void initMaterials ()
{
  for ( int i = 0 ; ! gs_setup [ i ] . isNull () ; i++ )
    gs_setup [ i ] . install ( i ) ;

  ssgSetAppStateCallback ( getAppState ) ;
}


Material *getMaterial ( ssgState *s )
{
  return & ( gs_setup [ s -> getExternalPropertyIndex () ] ) ;
}


Material *getMaterial ( ssgLeaf *l )
{
  return getMaterial ( l -> getState () ) ;
}

