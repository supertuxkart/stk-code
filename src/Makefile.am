# src/
bindir=@prefix@/games
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

supertuxkart_SOURCES =     main.cpp  			\
	vec3.cpp		       vec3.hpp			\
	coord.hpp						\
	actionmap.cpp		   actionmap.hpp		\
	material.cpp		   material.hpp           	\
	network/network_manager.cpp      network/network_manager.hpp 		\
	network/network_kart.cpp         network/network_kart.hpp		\
	network/message.cpp              network/message.hpp			\
	network/race_info_message.hpp    network/race_info_message.cpp		\
	network/remote_kart_info.hpp     network/character_selected_message.hpp	\
	network/race_start_message.hpp   network/character_confirm_message.hpp	\
	network/connect_message.hpp	 network/connect_message.cpp	\
	network/num_players_message.hpp  network/world_loaded_message.hpp	\
	network/connect_message.hpp      network/character_info_message.hpp	\
	network/kart_update_message.hpp  network/kart_update_message.cpp	\
	network/kart_control_message.hpp network/kart_control_message.cpp	\
	network/flyable_info.hpp	 network/item_info.hpp                  \
	network/race_state.hpp		 network/race_state.cpp			\
	network/race_result_message.hpp	 network/race_result_message.cpp	\
	network/race_result_ack_message						\
	audio/music.hpp								\
	audio/music_information.cpp audio/music_information.hpp 		\
	audio/music_ogg.cpp	   	    audio/music_ogg.hpp          	\
 	audio/sfx_base.hpp              					\
	audio/sfx_manager.cpp	   	audio/sfx_manager.hpp      	        \
	audio/sfx_openal.cpp	   	audio/sfx_openal.hpp		        \
	audio/sound_manager.cpp	   	audio/sound_manager.hpp      	        \
	utils/random_generator.hpp	utils/random_generator.cpp	        \
	utils/ssg_help.cpp          utils/ssg_help.hpp                          \
	material_manager.cpp	    material_manager.hpp    	                \
	grand_prix_manager.cpp	    grand_prix_manager.hpp	                \
	items/attachment.cpp	       items/attachment.hpp	                \
	items/attachment_manager.cpp   items/attachment_manager.hpp         \
	items/powerup.cpp	           items/powerup.hpp	                \
	items/powerup_manager.cpp      items/powerup_manager.hpp            \
	items/item.cpp 		           items/item.hpp                       \
	items/item_manager.cpp         items/item_manager.hpp               \
	items/projectile_manager.cpp   items/projectile_manager.hpp         \
	items/bubblegumitem.cpp	       items/bubblegumitem.hpp		        \
	items/plunger.cpp	           items/plunger.hpp		            \
	items/cake.cpp		           items/cake.hpp		                \
	items/bowling.cpp		       items/bowling.hpp	                \
    karts/auto_kart.hpp            karts/kart_control.hpp               \
	karts/kart.cpp		           karts/kart.hpp		                \
	karts/kart_model.cpp           karts/kart_model.hpp                 \
	karts/kart_properties.cpp	   karts/kart_properties.hpp            \
	karts/kart_properties_manager.cpp  karts/kart_properties_manager.hpp    \
	karts/moveable.cpp 		       karts/moveable.hpp                   \
	karts/player_kart.cpp		   karts/player_kart.hpp	            \
	smoke.cpp		       smoke.hpp			        \
	input.hpp 		              \
	isect.cpp		       isect.hpp                	\
	track.cpp		       track.hpp                	\
	explosion.cpp		   explosion.hpp            	\
	user_config.cpp		   user_config.hpp             	\
	grand_prix_data.cpp    grand_prix_data.hpp        	\
	stk_config.cpp		   stk_config.hpp    		    \
	highscores.cpp		   highscores.hpp		        \
	highscore_manager.cpp  highscore_manager.hpp	    \
	unlock_manager.cpp	   unlock_manager.hpp		    \
	file_manager.cpp       file_manager.hpp           	\
	loader.cpp		       loader.hpp			        \
	race_manager.cpp	   race_manager.hpp          	\
	string_utils.cpp	   string_utils.hpp          	\
	track_manager.cpp	   track_manager.hpp         	\
	callback.hpp 						                \
	moving_physics.hpp	   moving_physics.cpp		    \
	moving_texture.hpp	   moving_texture.cpp		    \
	callback_manager.cpp   callback_manager.hpp         \
	skid_mark.cpp		   skid_mark.hpp	            \
	shadow.cpp		       shadow.hpp	        	    \
	particle_system.cpp	   particle_system.hpp       	\
	main_loop.cpp	 	   main_loop.hpp        	    \
	camera.cpp		       camera.hpp	        	    \
	sdldrv.cpp		       sdldrv.hpp	        	    \
	user_pointer.hpp	                	\
	terrain_info.cpp	   terrain_info.hpp		        \
    triangle_mesh.cpp	   triangle_mesh.hpp		    \
	flyable.cpp		       flyable.hpp			        \
	history.cpp		       history.hpp	        	    \
	scene.hpp		       scene.cpp			        \
	no_copy.hpp	           constants.hpp 		        \
	translation.cpp		   translation.hpp 		        \
	player.hpp		   				\
	challenges/challenge.hpp      challenges/challenge.cpp	\
	challenges/challenge_data.hpp challenges/challenge_data.cpp	\
	lisp/lisp.cpp		       lisp/lisp.hpp            	\
	lisp/lexer.cpp		       lisp/lexer.hpp           	\
	lisp/parser.cpp		       lisp/parser.hpp          	\
	lisp/writer.cpp		       lisp/writer.hpp	        \
	gui/widget_manager.cpp     gui/widget_manager.hpp       \
	gui/widget.cpp		       gui/widget.hpp	        \
	gui/menu_manager.cpp       gui/menu_manager.hpp     	\
	gui/base_gui.cpp	       gui/base_gui.hpp	        \
	gui/race_gui.cpp	       gui/race_gui.hpp	        \
	gui/race_results_gui.cpp   gui/race_results_gui.hpp 	\
	gui/grand_prix_ending.cpp  gui/grand_prix_ending.hpp 	\
	gui/race_menu.cpp	       gui/race_menu.hpp	        \
	gui/num_players.cpp	       gui/num_players.hpp       	\
	gui/track_sel.cpp	       gui/track_sel.hpp	        \
	gui/player_controls.cpp	   gui/player_controls.hpp   	\
	gui/config_display.cpp	   gui/config_display.hpp    	\
	gui/display_res_confirm.cpp gui/display_res_confirm.hpp	\
	gui/config_sound.cpp	   gui/config_sound.hpp    	\
	gui/config_controls.cpp	   gui/config_controls.hpp   	\
	gui/options.cpp		       gui/options.hpp	        \
	gui/game_mode.cpp	       gui/game_mode.hpp	        \
	gui/race_options.cpp       gui/race_options.hpp       	\
	gui/char_sel.cpp	       gui/char_sel.hpp	        \
	gui/start_race_feedback.cpp gui/start_race_feedback.hpp	\
	gui/network_gui.cpp	       gui/network_gui.hpp		\
	gui/main_menu.cpp	       gui/main_menu.hpp         	\
	gui/help_page_one.cpp	   gui/help_page_one.hpp 	\
	gui/help_page_two.cpp  	   gui/help_page_two.hpp	\
	gui/help_page_three.cpp    gui/help_page_three.hpp	\
	gui/credits_menu.cpp	   gui/credits_menu.hpp		\
	gui/grand_prix_select.cpp  gui/grand_prix_select.hpp    \
	gui/challenges_menu.cpp	   gui/challenges_menu.hpp	\
	gui/feature_unlocked.cpp   gui/feature_unlocked.hpp	\
	gui/font.hpp		   gui/font.cpp                 \
	physics/physics.cpp        physics/physics.hpp          \
        physics/kart_motion_state.hpp                           \
	robots/default_robot.cpp   robots/default_robot.hpp	\
	modes/follow_the_leader.cpp modes/follow_the_leader.hpp \
	modes/standard_race.cpp    modes/standard_race.hpp      \
	modes/clock.cpp            modes/clock.hpp              \
	modes/world.cpp		   modes/world.hpp           	\
	modes/linear_world.cpp     modes/linear_world.hpp       \
	modes/three_strikes_battle.cpp modes/three_strikes_battle.hpp \
	replay_buffer_tpl.hpp 					\
	replay_buffers.hpp	   replay_buffers.cpp		\
	replay_base.hpp            replay_base.cpp		\
	replay_player.hpp          replay_player.cpp		\
	replay_recorder.hpp	   replay_recorder.cpp       	\
	ide/vc8/supertuxkart.sln   ide/vc8/supertuxkart.vcproj	\
	ide/vc8/bullet_lib.vcproj  ide/vc8/README \
	ide/vc9/supertuxkart.sln   ide/vc9/supertuxkart.vcproj	\
	ide/vc9/bullet_lib.vcproj  ide/vc9/enet.vcproj		\
	ide/vc9/README						\
	Xcode/STK_XCode.xcodeproj/project/pbxproj


# Link in the specific gcc 4.1 bug work around
supertuxkart_LDADD = -L. -lstatic_ssg \
	-lplibjs -lplibsl -lplibssg -lplibpu -lplibfnt -lplibsg \
	-lplibul -lplibssgaux $(bullet_LIBS) $(enet_LIBS) $(opengl_LIBS)  $(sdl_LIBS) $(openal_LIBS)

SUBDIRS = robots

