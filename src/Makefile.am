# src/

bin_PROGRAMS = supertuxkart
noinst_LIBRARIES = libMain.a

libMain_a_SOURCES = \
	material.cpp		 material.hpp           \
	material_manager.cpp	 material_manager.hpp    \
	attachment.cpp		 attachment.hpp	        \
	collectable.cpp		 collectable.hpp	        \
	collectable_manager.cpp	 collectable_manager.hpp   \
	sound.cpp		 sound.hpp                \
	utils.cpp		 utils.hpp                \
	isect.cpp		 isect.hpp                \
	track.cpp		 track.hpp                \
	herring.cpp 		 herring.hpp              \
	herring_manager.cpp	 herring_manager.hpp       \
	explosion.cpp		 explosion.hpp            \
	config.cpp 		 config.hpp               \
	cup_data.cpp   		 cup_data.hpp              \
	kart_manager.cpp		 kart_manager.hpp          \
	projectile_manager.cpp	 projectile_manager.hpp    \
	kart_properties.cpp	 kart_properties.hpp       \
	physics_parameters.cpp	 physics_parameters.hpp    \
	loader.cpp          	 loader.hpp               \
	race_manager.cpp		 race_manager.hpp          \
	string_utils.cpp		 string_utils.hpp          \
	track_manager.cpp	 track_manager.hpp         \
	world.cpp		 world.hpp                \
	preprocessor.cpp	 preprocessor.hpp	        \
	hook_manager.cpp		 hook_manager.hpp	        \
	skid_mark.cpp		 skid_mark.hpp	        \
	shadow.cpp		 shadow.hpp	        \
	start_screen.cpp		 start_screen.h	        \
	world_screen.cpp		 world_screen.h	        \
	particle_system.cpp	 particle_system.hpp       \
	screen_manager.cpp	 screen_manager.hpp        \
	widget_set.cpp		 widget_set.hpp	        \
	camera.cpp		 camera.hpp	        \
	plibdrv.cpp		 plibdrv.hpp	        \
	moveable.cpp 		 moveable.hpp             \
	kart.cpp		 kart.hpp		        \
	auto_kart.cpp		 auto_kart.hpp             \
	player_kart.cpp		 player_kart.hpp	        \
	projectile.cpp 		 projectile.hpp	        \
	static_ssg.cpp 		 statig_ssg.hpp	        \
	history.cpp		 history.hpp	        \
	race_setup.h	   	 no_copy.hpp	        \
	kart_control.h		 constants.hpp            \
	traffic.cpp 				        \
	lisp/lisp.cpp		 lisp/lisp.hpp            \
	lisp/lexer.cpp		 lisp/lexer.hpp           \
	lisp/parser.cpp		 lisp/parser.hpp          \
	lisp/writer.cpp		 lisp/writer.hpp	        \
	gui/menu_manager.cpp  gui/menu_manager.hpp     \
	gui/base_gui.cpp		 gui/base_gui.hpp	        \
	gui/race_gui.cpp		 gui/race_gui.hpp	        \
	gui/race_results_gui.cpp gui/race_results_gui.hpp \
	gui/num_laps.cpp		 gui/num_laps.hpp	        \
	gui/race_menu.cpp	 gui/race_menu.hpp	        \
	gui/num_players.cpp	 gui/num_players.hpp       \
	gui/track_sel.cpp	 gui/track_sel.hpp	        \
	gui/player_controls.cpp	 gui/player_controls.hpp   \
	gui/config_display.cpp	 gui/config_display.hpp    \
	gui/config_sound.cpp	     gui/config_sound.hpp    \
	gui/config_controls.cpp	 gui/config_controls.hpp   \
	gui/options.cpp		 gui/options.hpp	        \
	gui/game_mode.cpp	 gui/gameMode.hpp	        \
	gui/difficulty.cpp	 gui/difficulty.hpp       \
	gui/char_sel.cpp		 gui/char_sel.hpp	        \
	gui/main_menu.cpp	 gui/main_menu.hpp         \
	gui/single_window_menu.cpp gui/single_window_menu.hpp

supertuxkart_SOURCES = \
  start_tuxkart.cpp

supertuxkart_LDADD = \
	$(top_builddir)/src/libMain.a \
	-lplibjs -lplibsl -lplibssg -lplibpu -lplibfnt -lplibsg -lplibpw -lplibul \
	$(opengl_LIBS)
