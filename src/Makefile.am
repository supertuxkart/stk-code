# src/

bin_PROGRAMS = supertuxkart

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

noinst_LIBRARIES         =  libstatic_ssg.a
libstatic_ssg_a_SOURCES  = static_ssg.cpp static_ssg.hpp
libstatic_ssg_a_CXXFLAGS = @NOREGMOVE@

AM_CPPFLAGS=-DSUPERTUXKART_DATADIR="\"$(datadir)/games/@PACKAGE@/\""

supertuxkart_SOURCES =             main.cpp  			\
	actionmap.cpp		   actionmap.hpp		\
	material.cpp		   material.hpp           	\
	material_manager.cpp	   material_manager.hpp    	\
	attachment.cpp		   attachment.hpp	        \
	attachment_manager.cpp     attachment_manager.hpp       \
	collectable.cpp		   collectable.hpp	        \
	collectable_manager.cpp	   collectable_manager.hpp   	\
	sound_manager.cpp	   sound_manager.hpp           	\
	sound_plib.cpp             sound_plib.hpp              	\
	music_ogg.cpp	   	   music_ogg.hpp            	\
	music_information.cpp  	   music_information.hpp       	\
	sfx_openal.cpp             sfx_openal.hpp              	\
	smoke.cpp		   smoke.hpp			\
	input.hpp 						\
	isect.cpp		   isect.hpp                	\
	track.cpp		   track.hpp                	\
	herring.cpp 		   herring.hpp              	\
	herring_manager.cpp	   herring_manager.hpp       	\
	explosion.cpp		   explosion.hpp            	\
	user_config.cpp		   user_config.hpp             	\
	cup_data.cpp   		   cup_data.hpp              	\
	kart_properties_manager.cpp kart_properties_manager.hpp \
	projectile_manager.cpp	   projectile_manager.hpp    	\
	kart_properties.cpp	   kart_properties.hpp       	\
	stk_config.cpp		   stk_config.hpp    		\
	highscores.cpp		   highscores.hpp		\
	highscore_manager.cpp	   highscore_manager.hpp	\
	unlock_manager.cpp	   unlock_manager.hpp		\
	file_manager.cpp           file_manager.hpp           	\
	loader.cpp		   loader.hpp			\
	race_manager.cpp	   race_manager.hpp          	\
	string_utils.cpp	   string_utils.hpp          	\
	track_manager.cpp	   track_manager.hpp         	\
	world.cpp		   world.hpp                	\
	callback.hpp		   music.hpp			\
	sfx.hpp							\
	moving_physics.hpp	   moving_physics.cpp		\
	moving_texture.hpp	   moving_texture.cpp		\
	callback_manager.cpp       callback_manager.hpp         \
	physics.cpp                physics.hpp                  \
	skid_mark.cpp		   skid_mark.hpp	        \
	shadow.cpp		   shadow.hpp	        	\
	particle_system.cpp	   particle_system.hpp       	\
	game_manager.cpp	   game_manager.hpp        	\
	widget_manager.cpp	   widget_manager.hpp	        \
	widget.cpp		   widget.hpp	        	\
	camera.cpp		   camera.hpp	        	\
	sdldrv.cpp		   sdldrv.hpp	        	\
	moveable.cpp 		   moveable.hpp             	\
	ssg_help.cpp               ssg_help.hpp                 \
	kart.cpp		   kart.hpp		        \
	            		   auto_kart.hpp             	\
	player_kart.cpp		   player_kart.hpp	        \
	terrain_info.cpp	   terrain_info.hpp		\
        triangle_mesh.cpp	   triangle_mesh.hpp		\
				   user_pointer.hpp		\
	flyable.cpp		   flyable.hpp			\
	missile.cpp		   missile.hpp			\
	homing.cpp		   homing.hpp			\
	spark.cpp		   spark.hpp			\
	history.cpp		   history.hpp	        	\
	scene.hpp		   scene.cpp			\
	no_copy.hpp	           constants.hpp 		\
	translation.cpp		   translation.hpp 		\
	traffic.cpp 			  	        	\
	player.hpp		   				\
	challenges/challenge.hpp   challenges/challenge.cpp	\
	challenges/all_tracks.cpp  challenges/all_tracks.hpp	\
	challenges/energy_math_class.cpp  challenges/energy_math_class.hpp\
	challenges/penguin_playground_gp.cpp challenges/penguin_playground_gp.hpp \
	challenges/race_track_time.cpp challenges/race_track_time.hpp\
	challenges/tollway_time.cpp challenges/tollway_time.hpp \
	challenges/jungle_follow.cpp challenges/jungle_follow.hpp \
	challenges/energy_shifting_sands.cpp challenges/energy_shifting_sands.hpp \
	challenges/moon_and_back_gp.cpp challenges/moon_and_back_gp.hpp \
	challenges/city_time.cpp  challenges/city_time.hpp  \
	challenges/island_follow.cpp  challenges/island_follow.hpp  \
	challenges/worlds_end_gp.cpp challenges/worlds_end_gp.hpp  \
	lisp/lisp.cpp		   lisp/lisp.hpp            	\
	lisp/lexer.cpp		   lisp/lexer.hpp           	\
	lisp/parser.cpp		   lisp/parser.hpp          	\
	lisp/writer.cpp		   lisp/writer.hpp	        \
	gui/menu_manager.cpp       gui/menu_manager.hpp     	\
	gui/base_gui.cpp	   gui/base_gui.hpp	        \
	gui/race_gui.cpp	   gui/race_gui.hpp	        \
	gui/race_results_gui.cpp   gui/race_results_gui.hpp 	\
	gui/grand_prix_ending.cpp  gui/grand_prix_ending.hpp 	\
	gui/race_menu.cpp	   gui/race_menu.hpp	        \
	gui/num_players.cpp	   gui/num_players.hpp       	\
	gui/track_sel.cpp	   gui/track_sel.hpp	        \
	gui/player_controls.cpp	   gui/player_controls.hpp   	\
	gui/config_display.cpp	   gui/config_display.hpp    	\
	gui/display_res_confirm.cpp	gui/display_res_confirm.hpp	\
	gui/config_sound.cpp	   gui/config_sound.hpp    	\
	gui/config_controls.cpp	   gui/config_controls.hpp   	\
	gui/options.cpp		   gui/options.hpp	        \
	gui/game_mode.cpp	   gui/game_mode.hpp	        \
	gui/race_options.cpp   gui/race_options.hpp       	\
	gui/char_sel.cpp	   gui/char_sel.hpp	        \
	gui/leader_result.cpp	   gui/leader_result.hpp	\
	gui/start_race_feedback.cpp	   gui/start_race_feedback.hpp	\
	gui/main_menu.cpp	   gui/main_menu.hpp         	\
	gui/help_page_one.cpp  gui/help_page_one.hpp 	        \
	gui/help_page_two.cpp  gui/help_page_two.hpp 	        \
	gui/credits_menu.cpp	   gui/credits_menu.hpp		\
	gui/grand_prix_select.cpp  gui/grand_prix_select.hpp    \
	gui/challenges_menu.cpp	   gui/challenges_menu.hpp	\
	gui/feature_unlocked.cpp   gui/feature_unlocked.hpp	\
	gui/font.hpp		   gui/font.cpp                 \
	robots/default_robot.cpp   robots/default_robot.hpp	\
	replay_buffer_tpl.hpp 					\
	replay_buffers.hpp	   replay_buffers.cpp		\
	replay_base.hpp		   replay_base.cpp		\
	replay_player.hpp          replay_player.cpp		\
	replay_recorder.hpp	   replay_recorder.cpp       	\
	ide/vc8/supertuxkart.sln   ide/vc8/supertuxkart.vcproj	\
	ide/vc8/bullet_lib.vcproj  ide/vc8/README


# Link in the specific gcc 4.1 bug work around
supertuxkart_LDADD = -L. -lstatic_ssg \
	-lplibjs -lplibsl -lplibssg -lplibpu -lplibfnt -lplibsg \
	-lplibul -lplibssgaux $(bullet_LIBS) $(opengl_LIBS)  $(sdl_LIBS) $(openal_LIBS)

.PHONY: pot
pot:
	xgettext -o supertuxkart.pot -k_ --c++ *.?pp */*.?pp

SUBDIRS = robots

