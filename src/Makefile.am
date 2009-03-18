# src/

SUBDIRS = ide

bindir=$(prefix)/games
bin_PROGRAMS = supertuxkart

AM_CPPFLAGS = -DSUPERTUXKART_DATADIR="\"$(datadir)/games/$(PACKAGE)/\""  \
 -I$(srcdir)/bullet/src/ -I$(srcdir)/enet/include/

supertuxkart_SOURCES = \
 main.cpp \
 actionmap.cpp \
 actionmap.hpp \
 material.cpp \
 material.hpp \
 network/network_manager.cpp \
 network/network_manager.hpp \
 network/network_kart.cpp \
 network/network_kart.hpp \
 network/message.cpp \
 network/message.hpp \
 network/race_info_message.hpp \
 network/race_info_message.cpp \
 network/race_result_ack_message.hpp \
 network/remote_kart_info.hpp \
 network/character_selected_message.hpp \
 network/race_start_message.hpp \
 network/character_confirm_message.hpp \
 network/connect_message.hpp \
 network/connect_message.cpp \
 network/num_players_message.hpp \
 network/world_loaded_message.hpp \
 network/connect_message.hpp \
 network/character_info_message.hpp \
 network/kart_update_message.hpp \
 network/kart_update_message.cpp \
 network/kart_control_message.hpp \
 network/kart_control_message.cpp \
 network/flyable_info.hpp \
 network/item_info.hpp \
 network/race_state.hpp \
 network/race_state.cpp \
 network/race_result_message.hpp \
 network/race_result_message.cpp \
 audio/music.hpp \
 audio/music_information.cpp \
 audio/music_information.hpp \
 audio/music_ogg.cpp \
 audio/music_ogg.hpp \
 audio/sfx_base.hpp \
 audio/sfx_manager.cpp \
 audio/sfx_manager.hpp \
 audio/sfx_openal.cpp \
 audio/sfx_openal.hpp \
 audio/sound_manager.cpp \
 audio/sound_manager.hpp \
 utils/constants.hpp \
 utils/coord.hpp \
 utils/random_generator.hpp \
 utils/random_generator.cpp \
 utils/ssg_help.cpp \
 utils/ssg_help.hpp \
 utils/string_utils.cpp \
 utils/string_utils.hpp \
 utils/translation.cpp \
 utils/translation.hpp \
 utils/vec3.cpp \
 utils/vec3.hpp \
 material_manager.cpp \
 material_manager.hpp \
 grand_prix_manager.cpp \
 grand_prix_manager.hpp \
 graphics/camera.cpp \
 graphics/camera.hpp \
 graphics/irr_driver.cpp \
 graphics/irr_driver.hpp \
 graphics/mesh_tools.hpp \
 graphics/mesh_tools.cpp \
 graphics/nitro.cpp \
 graphics/nitro.hpp \
 graphics/moving_texture.hpp \
 graphics/moving_texture.cpp \
 graphics/particle_system.cpp \
 graphics/particle_system.hpp \
 graphics/scene.hpp \
 graphics/scene.cpp \
 graphics/shadow.cpp \
 graphics/shadow.hpp \
 graphics/skid_marks.cpp \
 graphics/skid_marks.hpp \
 graphics/smoke.cpp \
 graphics/smoke.hpp \
 io/file_manager.cpp \
 io/file_manager.hpp \
 io/xml_node.cpp \
 io/xml_node.hpp \
 io/xml_reader.cpp \
 io/xml_reader.hpp\
 items/attachment.cpp \
 items/attachment.hpp \
 items/attachment_manager.cpp \
 items/attachment_manager.hpp \
 items/flyable.cpp \
 items/flyable.hpp \
 items/powerup.cpp \
 items/powerup.hpp \
 items/powerup_manager.cpp \
 items/powerup_manager.hpp \
 items/item.cpp \
 items/item.hpp \
 items/item_manager.cpp \
 items/item_manager.hpp \
 items/projectile_manager.cpp \
 items/projectile_manager.hpp \
 items/bubblegumitem.cpp \
 items/bubblegumitem.hpp \
 items/plunger.cpp \
 items/plunger.hpp \
 items/rubber_band.cpp \
 items/rubber_band.hpp \
 items/cake.cpp \
 items/cake.hpp \
 items/bowling.cpp \
 items/bowling.hpp \
 karts/auto_kart.hpp \
 karts/kart_control.hpp \
 karts/kart.cpp \
 karts/kart.hpp \
 karts/kart_model.cpp \
 karts/kart_model.hpp \
 karts/kart_properties.cpp \
 karts/kart_properties.hpp \
 karts/kart_properties_manager.cpp \
 karts/kart_properties_manager.hpp    \
 karts/moveable.cpp \
 karts/moveable.hpp \
 karts/player_kart.cpp \
 karts/player_kart.hpp \
 input.hpp \
 isect.cpp \
 isect.hpp \
 explosion.cpp \
 explosion.hpp \
 user_config.cpp \
 user_config.hpp \
 grand_prix_data.cpp \
 grand_prix_data.hpp \
 stk_config.cpp \
 stk_config.hpp \
 highscores.cpp \
 highscores.hpp \
 highscore_manager.cpp \
 highscore_manager.hpp \
 loader.cpp \
 loader.hpp \
 race_manager.cpp \
 race_manager.hpp \
 callback.hpp \
 callback_manager.cpp \
 callback_manager.hpp \
 main_loop.cpp \
 main_loop.hpp \
 sdldrv.cpp \
 sdldrv.hpp \
 user_pointer.hpp \
 history.cpp \
 history.hpp \
 no_copy.hpp \
 player.hpp \
 challenges/challenge.hpp \
 challenges/challenge.cpp \
 challenges/challenge_data.hpp \
 challenges/challenge_data.cpp \
 challenges/unlock_manager.cpp \
 challenges/unlock_manager.hpp \
 lisp/lisp.cpp \
 lisp/lisp.hpp \
 lisp/lexer.cpp \
 lisp/lexer.hpp \
 lisp/parser.cpp \
 lisp/parser.hpp \
 lisp/writer.cpp \
 lisp/writer.hpp \
 gui/widget.cpp \
 gui/widget.hpp \
 gui/race_gui.cpp \
 gui/race_gui.hpp \
 gui/font.hpp \
 gui/font.cpp \
 gui/engine.cpp \
 gui/engine.hpp \
 gui/my_button.cpp \
 gui/my_button.hpp \
 gui/screen.cpp \
 gui/screen.hpp \
 gui/screen_loader.cpp \
 gui/skin.cpp \
 gui/skin.hpp \
 gui/state_manager.cpp \
 gui/state_manager.hpp \
 modes/follow_the_leader.cpp \
 modes/follow_the_leader.hpp \
 modes/standard_race.cpp \
 modes/standard_race.hpp \
 modes/clock.cpp \
 modes/clock.hpp \
 modes/world.cpp \
 modes/world.hpp \
 modes/linear_world.cpp \
 modes/linear_world.hpp \
 modes/three_strikes_battle.cpp \
 modes/three_strikes_battle.hpp \
 physics/btKart.cpp \
 physics/btKart.hpp \
 physics/btUprightConstraint.cpp \
 physics/btUprightConstraint.hpp \
 physics/moving_physics.hpp \
 physics/moving_physics.cpp \
 physics/physics.cpp \
 physics/physics.hpp \
 physics/kart_motion_state.hpp \
 physics/triangle_mesh.cpp \
 physics/triangle_mesh.hpp \
 robots/default_robot.cpp \
 robots/default_robot.hpp \
 robots/track_info.cpp \
 robots/track_info.hpp \
 tracks/terrain_info.cpp \
 tracks/terrain_info.hpp \
 tracks/track.cpp \
 tracks/track.hpp \
 tracks/track_manager.cpp \
 tracks/track_manager.hpp \
 replay/replay_buffer_tpl.hpp \
 replay/replay_buffers.hpp \
 replay/replay_buffers.cpp \
 replay/replay_base.hpp \
 replay/replay_base.cpp \
 replay/replay_player.hpp \
 replay/replay_player.cpp \
 replay/replay_recorder.hpp \
 replay/replay_recorder.cpp

# This is a VERY ugly work-around for a compiler bug: if static_ssg s compiled with -O2
# (more particularly: with -fregmove, which is part of -O2), it will causes the kart
# to fall through the tracks very easily. Therefore, static_ssg needs special compiler
# flags to work correctly.
# It might (didn't check the details) be that this is the problem reported in 
# http://gcc.gnu.org/bugzilla/show_bug.cgi?id=28146
# (which states that the problem does not happen with i686 and x86_64, which
# is what I have seen as well, it only happened on i586, not on x86_64).
#
# Since it is not possible to set compiler flags for a single .o file (see
# http://sources.redhat.com/ml/automake/2004-12/msg00063.html), I used the
# workaround provided there.
# Adding -fno-regmove works, even though it is put before the actual compiler flags,
# i.e.: -fno-regmove -g -O2, still works by disabling -fregmove.
# The configure script checks if the compiler accepts this flag, and if so, it
# will be used here.
noinst_LIBRARIES         = libstatic_ssg.a
libstatic_ssg_a_SOURCES  = static_ssg.cpp static_ssg.hpp
libstatic_ssg_a_CXXFLAGS = $(NOREGMOVE)

# Link in the specific gcc 4.1 bug work around
supertuxkart_LDADD = libstatic_ssg.a \
        -lplibjs -lplibsl -lplibssg -lplibpu -lplibfnt -lplibsg \
        -lplibul -lplibssgaux $(irrlicht_LIBS) $(bullet_LIBS) $(enet_LIBS) $(glut_LIBS) \
        $(opengl_LIBS) $(sdl_LIBS) $(openal_LIBS) $(oggvorbis_LIBS) \
        $(INTLLIBS)

EXTRA_DIST = robots/empty_robot.cpp robots/empty_robot.hpp

