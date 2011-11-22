# CMakeLists.txt - root CMakeLists for the SuperTuxKart project
# TODO: make Ogg Vorbis optional (doesn't seem to work with autotools either!)

cmake_minimum_required(VERSION 2.8.1)

project(SuperTuxKart)

#set(CMAKE_VERBOSE_MAKEFILE TRUE)

set(STK_SOURCE_DIR "src")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin")
set(PROJECT_VERSION "0.7.3")

# Tweakable values
if(CMAKE_BUILD_TYPE MATCHES "Debug")
	set(DEBUG ON)
else()
	set(DEBUG OFF)
endif()

option(USE_FRIBIDI "Support for right-to-left languages" ON)
option(FONT_TOOL "Compile font tool" OFF)
set(IRRLICHT_DIR "/path/to/irrlicht" CACHE STRING "Path to Irrlicht")

# Build the Bullet physics library
add_subdirectory("${STK_SOURCE_DIR}/bullet")

# Build the ENet UDP network library
add_subdirectory("${STK_SOURCE_DIR}/enet")

# Set include paths
include_directories("${STK_SOURCE_DIR}")
include_directories("${STK_SOURCE_DIR}/enet/include")
include_directories("${STK_SOURCE_DIR}/bullet/src")

if(APPLE)
	# We could use smarter detection of the frameworks but supporting a single official setup makes things easy...
	include_directories("/Library/Frameworks/IrrFramework.framework/Versions/A/Headers/")
	include_directories("/Library/Frameworks/fribidi.framework/Headers")
	include_directories("/Library/Frameworks/Ogg.framework/Headers")
	include_directories("/Library/Frameworks/Vorbis.framework/Headers")
	include_directories("/usr/local/include")
	include_directories("/usr/include")
	find_library(Ogg /Library/Frameworks/Ogg.framework)
	find_library(Vorbis /Library/Frameworks/Vorbis.framework)
endif()

# Set lib paths
if(APPLE)
	# nothing to add
elseif(UNIX)
	link_directories("${IRRLICHT_DIR}/lib/Linux")
elseif(MSVC)
	link_directories("${IRRLICHT_DIR}/lib/Win32-visualstudio")
else()
	link_directories("${IRRLICHT_DIR}/lib/Win32-gcc")
endif()

link_directories("${STK_SOURCE_DIR}/bullet")

# OpenGL
if(APPLE)
    find_library(OpenGL /System/Library/Frameworks/OpenGL.framework /System/Library/Frameworks/AGL.framework)
else()
    find_package(OpenGL)
    
    if(OPENGL_FOUND)
	    message("-- OpenGL found (include dirs: ${OPENGL_INCLUDE_DIR})")
	    include_directories(${OPENGL_INCLUDE_DIR})
    else()
	    message(FATAL_ERROR "OpenGL not found.")
    endif()
    
    if(UNIX)
	    # X11, stk requires xf86vm
	    find_package(X11)
    
	    if(X11_FOUND)
		    message("-- X11 libs found")
	    else()
		    message(FATAL_ERROR "X11 not found.")
	    endif()
    endif()
endif()

# OpenAL
if(APPLE)
    find_library(OpenAL /Library/Frameworks/OpenAL.framework)
else()
    find_package(OpenAL)
    
    if(OPENAL_FOUND)
	    message("-- OpenAL found (include dirs: ${OPENAL_INCLUDE_DIR})")
	    include_directories(${OPENAL_INCLUDE_DIR})
    else()
	    message(FATAL_ERROR "OpenAL not found.")
    endif()
endif()

# Check if we finally got irrlicht
if(APPLE)
    find_library(Irrlicht /Library/Frameworks/IrrFramework.framework)
else()
    find_path(HAVE_IRRLICHT irrlicht.h PATHS ${IRRLICHT_DIR} ${IRRLICHT_DIR}/include /usr/include/irrlicht/ /usr/local/include/irrlicht/)
    
    if(HAVE_IRRLICHT)
	    message("-- Irrlicht found (in ${HAVE_IRRLICHT}/irrlicht.h)")
	    #include_directories(${IRRLICHT_DIR} ${IRRLICHT_DIR}/include /usr/include/irrlicht/ /usr/local/include/irrlicht/)
	    include_directories(${HAVE_IRRLICHT})
    else()
	    message(FATAL_ERROR "\n  --  Irrlicht not found (can't locate irrlicht.h)\n Use -DIRRLICHT_DIR=/path/to/irrlicht")
    endif()
endif()

include_directories(${HAVE_IRRLICHT})

# CURL
find_package(CURL)
if(CURL_FOUND)
	include_directories(${CURL_INCLUDE_DIRS})
else()
	message(FATAL_ERROR "-- libcURL not found, please install it.")
endif()

# Fribidi
if(USE_FRIBIDI)
    FIND_PATH(FRIBIDI_INCLUDE_DIR fribidi/fribidi.h
      /usr/local/include
      /usr/include
      )

    SET(FRIBIDI_NAMES ${FRIBIDI_NAMES} fribidi libfribidi)
    FIND_LIBRARY(FRIBIDI_LIBRARY
      NAMES ${FRIBIDI_NAMES}
      PATHS /usr/lib /usr/local/lib
      )

    IF (FRIBIDI_LIBRARY AND FRIBIDI_INCLUDE_DIR)
        message("-- Fribidi found")
    else()
        message(FATAL_ERROR "Fribidi not found. Either install fribidi or disable bidi support with -DUSE_FRIBIDI=0 (if you don't use a right-to-left language then you don't need this).")
    endif()
endif()
   
# Set some compiler options
if(UNIX)
	add_definitions(-Wall)
endif()

add_definitions(-DHAVE_OGGVORBIS)	# TODO: remove this switch

if(DEBUG)
	add_definitions(-DDEBUG)
else()
	add_definitions(-DNDEBUG)
	add_definitions(-O2)
endif()

find_package(Freetype)

if(FONT_TOOL)
    if(FREETYPE_FOUND)
	    message("-- Freetype found")
	    find_package(X11)
	    include_directories(${FREETYPE_INCLUDE_DIRS})
	    add_executable(font_tool
		    tools/font_tool/CFontTool.cpp
		    tools/font_tool/CFontTool.h
		    tools/font_tool/CVectorFontTool.h
		    tools/font_tool/main.cpp
	    )
	    target_link_libraries(font_tool ${FREETYPE_LIBRARIES})
	    target_link_libraries(font_tool ${X11_Xft_LIB})
	    target_link_libraries(font_tool ${OPENGL_LIBRARIES})
	    target_link_libraries(font_tool Irrlicht)
    else()
	    message("-- Freetype was not found, the font tool won't be built (only useful for developers)")
    endif()
else()
    message("-- Font tool deactivated, the font tool won't be built (only useful for developers)")
endif()

set( SRCS ${SRCS} src/main.cpp
	src/main_loop.cpp
	src/main_loop.hpp
	src/addons/addon.cpp
	src/addons/addon.hpp
	src/addons/addons_manager.cpp
	src/addons/addons_manager.hpp
	src/addons/network_http.cpp
	src/addons/network_http.hpp
	src/addons/news_manager.cpp
	src/addons/news_manager.hpp
	src/addons/request.cpp
	src/addons/request.hpp
	src/addons/zip.cpp
	src/addons/zip.hpp
	src/animations/animation_base.cpp
	src/animations/animation_base.hpp
	src/animations/billboard_animation.cpp
	src/animations/billboard_animation.hpp
	src/animations/ipo.cpp
	src/animations/ipo.hpp
	src/animations/three_d_animation.cpp
	src/animations/three_d_animation.hpp
	src/audio/music.hpp
	src/audio/music_information.cpp
	src/audio/music_information.hpp
	src/audio/music_manager.cpp
	src/audio/music_manager.hpp
	src/audio/music_ogg.cpp
	src/audio/music_ogg.hpp
	src/audio/sfx_base.hpp
	src/audio/sfx_buffer.cpp
	src/audio/sfx_buffer.hpp
	src/audio/sfx_manager.cpp
	src/audio/sfx_manager.hpp
	src/audio/sfx_openal.cpp
	src/audio/sfx_openal.hpp
	src/challenges/challenge.cpp
	src/challenges/challenge_data.cpp
	src/challenges/challenge_data.hpp
	src/challenges/challenge.hpp
	src/challenges/unlock_manager.cpp
	src/challenges/unlock_manager.hpp
	src/config/device_config.cpp
	src/config/device_config.hpp
	src/config/player.hpp
	src/config/stk_config.cpp
	src/config/stk_config.hpp
	src/config/user_config.cpp
	src/config/user_config.hpp
	src/enet/include/enet/callbacks.h
	src/enet/include/enet/enet.h
	src/enet/include/enet/list.h
	src/enet/include/enet/protocol.h
	src/enet/include/enet/time.h
	src/enet/include/enet/types.h
	src/enet/include/enet/unix.h
	src/enet/include/enet/utility.h
	src/enet/include/enet/win32.h
	src/graphics/camera.cpp
	src/graphics/camera.hpp
	src/graphics/CBatchingMesh.cpp
	src/graphics/CBatchingMesh.hpp
	src/graphics/explosion.cpp
	src/graphics/explosion.hpp
	src/graphics/hardware_skinning.cpp
	src/graphics/hardware_skinning.hpp
	src/graphics/hit_effect.hpp
	src/graphics/hit_sfx.cpp
	src/graphics/hit_sfx.hpp
	src/graphics/irr_driver.cpp
	src/graphics/irr_driver.hpp
	src/graphics/lod_node.cpp
	src/graphics/lod_node.hpp
	src/graphics/material.cpp
	src/graphics/material.hpp
	src/graphics/material_manager.cpp
	src/graphics/material_manager.hpp
	src/graphics/mesh_tools.cpp
	src/graphics/mesh_tools.hpp
	src/graphics/moving_texture.cpp
	src/graphics/moving_texture.hpp
	src/graphics/particle_emitter.cpp
	src/graphics/particle_emitter.hpp
	src/graphics/particle_kind.cpp
	src/graphics/particle_kind.hpp
	src/graphics/particle_kind_manager.cpp
	src/graphics/particle_kind_manager.hpp
	src/graphics/per_camera_node.cpp
	src/graphics/per_camera_node.hpp
	src/graphics/post_processing.cpp
	src/graphics/post_processing.hpp
	src/graphics/rain.cpp
	src/graphics/rain.hpp
        src/graphics/referee.cpp
        src/graphics/referee.hpp
	src/graphics/shadow.cpp
	src/graphics/shadow.hpp
	src/graphics/skid_marks.cpp
	src/graphics/skid_marks.hpp
	src/graphics/slip_stream.cpp
	src/graphics/slip_stream.hpp
	src/graphics/stars.cpp
	src/graphics/stars.hpp
	src/guiengine/abstract_state_manager.cpp
	src/guiengine/abstract_state_manager.hpp
	src/guiengine/abstract_top_level_container.cpp
	src/guiengine/abstract_top_level_container.hpp
	src/guiengine/CGUISpriteBank.cpp
	src/guiengine/CGUISpriteBank.h
	src/guiengine/engine.cpp
	src/guiengine/event_handler.cpp
	src/guiengine/event_handler.hpp
	src/guiengine/layout_manager.cpp
	src/guiengine/layout_manager.hpp
	src/guiengine/modaldialog.cpp
	src/guiengine/modaldialog.hpp
	src/guiengine/scalable_font.cpp
	src/guiengine/scalable_font.hpp
	src/guiengine/screen.cpp
	src/guiengine/screen.hpp
	src/guiengine/screen_loader.cpp
	src/guiengine/skin.cpp
	src/guiengine/skin.hpp
	src/guiengine/widget.cpp
	src/guiengine/widget.hpp
	src/guiengine/widgets/bubble_widget.cpp
	src/guiengine/widgets/bubble_widget.hpp
	src/guiengine/widgets/button_widget.cpp
	src/guiengine/widgets/button_widget.hpp
	src/guiengine/widgets/CGUIEditBox.cpp
	src/guiengine/widgets/CGUIEditBox.h
	src/guiengine/widgets/check_box_widget.cpp
	src/guiengine/widgets/dynamic_ribbon_widget.cpp
	src/guiengine/widgets/dynamic_ribbon_widget.hpp
	src/guiengine/widgets.hpp
	src/guiengine/widgets/icon_button_widget.cpp
	src/guiengine/widgets/icon_button_widget.hpp
	src/guiengine/widgets/label_widget.cpp
	src/guiengine/widgets/label_widget.hpp
	src/guiengine/widgets/list_widget.cpp
	src/guiengine/widgets/list_widget.hpp
	src/guiengine/widgets/model_view_widget.cpp
	src/guiengine/widgets/model_view_widget.hpp
	src/guiengine/widgets/progress_bar_widget.cpp
	src/guiengine/widgets/progress_bar_widget.hpp
	src/guiengine/widgets/ribbon_widget.cpp
	src/guiengine/widgets/ribbon_widget.hpp
	src/guiengine/widgets/spinner_widget.cpp
	src/guiengine/widgets/spinner_widget.hpp
	src/guiengine/widgets/text_box_widget.cpp
	src/guiengine/widgets/text_box_widget.hpp
	src/input/binding.cpp
	src/input/binding.hpp
	src/input/device_manager.cpp
	src/input/device_manager.hpp
	src/input/input_device.cpp
	src/input/input_device.hpp
	src/input/input.hpp
	src/input/input_manager.cpp
	src/input/input_manager.hpp
	src/io/file_manager.cpp
	src/io/file_manager.hpp
	src/io/xml_node.cpp
	src/io/xml_node.hpp
	src/io/xml_writer.cpp
	src/io/xml_writer.hpp
	src/items/attachment.cpp
	src/items/attachment.hpp
	src/items/attachment_manager.cpp
	src/items/attachment_manager.hpp
	src/items/attachment_plugin.hpp
	src/items/bowling.cpp
	src/items/bowling.hpp
	src/items/cake.cpp
	src/items/cake.hpp
	src/items/flyable.cpp
	src/items/flyable.hpp
	src/items/item.cpp
	src/items/item.hpp
	src/items/item_manager.cpp
	src/items/item_manager.hpp
	src/items/plunger.cpp
	src/items/plunger.hpp
	src/items/powerup.cpp
	src/items/powerup.hpp
	src/items/powerup_manager.cpp
	src/items/powerup_manager.hpp
	src/items/projectile_manager.cpp
	src/items/projectile_manager.hpp
	src/items/rubber_ball.cpp
	src/items/rubber_ball.hpp
	src/items/rubber_band.cpp
	src/items/rubber_band.hpp
	src/items/swatter.cpp
	src/items/swatter.hpp
	src/karts/controller/ai_base_controller.cpp
	src/karts/controller/ai_base_controller.hpp
	src/karts/controller/controller.cpp
	src/karts/controller/controller.hpp
	src/karts/controller/default_ai_controller.cpp
	src/karts/controller/default_ai_controller.hpp
	src/karts/controller/end_controller.cpp
	src/karts/controller/end_controller.hpp
	src/karts/controller/kart_control.hpp
	src/karts/controller/new_ai_controller.cpp
	src/karts/controller/new_ai_controller.hpp
	src/karts/controller/player_controller.cpp
	src/karts/controller/player_controller.hpp
	src/karts/emergency_animation.cpp
	src/karts/emergency_animation.hpp
	src/karts/kart.cpp
	src/karts/kart.hpp
	src/karts/kart_with_stats.cpp
	src/karts/kart_with_stats.hpp
	src/karts/kart_model.cpp
	src/karts/kart_model.hpp
	src/karts/kart_properties.cpp
	src/karts/kart_properties.hpp
	src/karts/kart_properties_manager.cpp
	src/karts/kart_properties_manager.hpp
	src/karts/max_speed.cpp
	src/karts/max_speed.hpp
	src/karts/moveable.cpp
	src/karts/moveable.hpp
	src/modes/follow_the_leader.cpp
	src/modes/follow_the_leader.hpp
	src/modes/game_tutorial.cpp
	src/modes/game_tutorial.hpp
	src/modes/linear_world.cpp
	src/modes/linear_world.hpp
	src/modes/profile_world.cpp
	src/modes/profile_world.hpp
	src/modes/standard_race.cpp
	src/modes/standard_race.hpp
	src/modes/three_strikes_battle.cpp
	src/modes/three_strikes_battle.hpp
	src/modes/tutorial_race.cpp
	src/modes/tutorial_race.hpp
	src/modes/world.cpp
	src/modes/world.hpp
	src/modes/world_status.cpp
	src/modes/world_status.hpp
	src/modes/world_with_rank.cpp
	src/modes/world_with_rank.hpp
	src/network/character_confirm_message.hpp
	src/network/character_info_message.hpp
	src/network/character_selected_message.hpp
	src/network/connect_message.cpp
	src/network/connect_message.hpp
	src/network/flyable_info.hpp
	src/network/item_info.hpp
	src/network/kart_control_message.cpp
	src/network/kart_control_message.hpp
	src/network/kart_update_message.cpp
	src/network/kart_update_message.hpp
	src/network/message.cpp
	src/network/message.hpp
	src/network/network_kart.cpp
	src/network/network_kart.hpp
	src/network/network_manager.cpp
	src/network/network_manager.hpp
	src/network/num_players_message.hpp
	src/network/race_info_message.cpp
	src/network/race_info_message.hpp
	src/network/race_result_ack_message.hpp
	src/network/race_result_message.cpp
	src/network/race_result_message.hpp
	src/network/race_start_message.hpp
	src/network/race_state.cpp
	src/network/race_state.hpp
	src/network/remote_kart_info.hpp
	src/network/world_loaded_message.hpp
	src/physics/btKart.cpp
	src/physics/btKart.hpp
	src/physics/btKartRaycast.cpp
	src/physics/btKartRaycast.hpp
	src/physics/btUprightConstraint.cpp
	src/physics/btUprightConstraint.hpp
	src/physics/irr_debug_drawer.cpp
	src/physics/irr_debug_drawer.hpp
	src/physics/kart_motion_state.hpp
	src/physics/physical_object.cpp
	src/physics/physical_object.hpp
	src/physics/physics.cpp
	src/physics/physics.hpp
	src/physics/triangle_mesh.cpp
	src/physics/triangle_mesh.hpp
	src/physics/user_pointer.hpp
	src/race/grand_prix_data.cpp
	src/race/grand_prix_data.hpp
	src/race/grand_prix_manager.cpp
	src/race/grand_prix_manager.hpp
	src/race/highscore_manager.cpp
	src/race/highscore_manager.hpp
	src/race/highscores.cpp
	src/race/highscores.hpp
	src/race/history.cpp
	src/race/history.hpp
	src/race/race_manager.cpp
	src/race/race_manager.hpp
	src/replay/replay_base.cpp
	src/replay/replay_base.hpp
	src/replay/replay_buffers.cpp
	src/replay/replay_buffers.hpp
	src/replay/replay_buffer_tpl.hpp
	src/replay/replay_player.cpp
	src/replay/replay_player.hpp
	src/replay/replay_recorder.cpp
	src/replay/replay_recorder.hpp
	src/states_screens/addons_screen.cpp
	src/states_screens/addons_screen.hpp
	src/states_screens/arenas_screen.cpp
	src/states_screens/arenas_screen.hpp
	src/states_screens/challenges.cpp
	src/states_screens/challenges.hpp
	src/states_screens/credits.cpp
	src/states_screens/credits.hpp
	src/states_screens/dialogs/add_device_dialog.cpp
	src/states_screens/dialogs/add_device_dialog.hpp
	src/states_screens/dialogs/addons_loading.cpp
	src/states_screens/dialogs/addons_loading.hpp
	src/states_screens/dialogs/confirm_resolution_dialog.cpp
	src/states_screens/dialogs/confirm_resolution_dialog.hpp
	src/states_screens/dialogs/enter_player_name_dialog.cpp
	src/states_screens/dialogs/enter_player_name_dialog.hpp
	src/states_screens/dialogs/gp_info_dialog.cpp
	src/states_screens/dialogs/gp_info_dialog.hpp
	src/states_screens/dialogs/message_dialog.cpp
	src/states_screens/dialogs/message_dialog.hpp
	src/states_screens/dialogs/player_info_dialog.cpp
	src/states_screens/dialogs/player_info_dialog.hpp
	src/states_screens/dialogs/press_a_key_dialog.cpp
	src/states_screens/dialogs/press_a_key_dialog.hpp
	src/states_screens/dialogs/race_over_dialog.cpp
	src/states_screens/dialogs/race_over_dialog.hpp
	src/states_screens/dialogs/race_paused_dialog.cpp
	src/states_screens/dialogs/race_paused_dialog.hpp
	src/states_screens/dialogs/track_info_dialog.cpp
	src/states_screens/dialogs/track_info_dialog.hpp
	src/states_screens/feature_unlocked.cpp
	src/states_screens/feature_unlocked.hpp
	src/states_screens/grand_prix_lose.cpp
	src/states_screens/grand_prix_lose.hpp
	src/states_screens/grand_prix_win.cpp
	src/states_screens/grand_prix_win.hpp
	src/states_screens/help_screen_1.cpp
	src/states_screens/help_screen_1.hpp
	src/states_screens/help_screen_2.cpp
	src/states_screens/help_screen_2.hpp
	src/states_screens/help_screen_3.cpp
	src/states_screens/help_screen_3.hpp
	src/states_screens/help_screen_4.cpp
	src/states_screens/help_screen_4.hpp
	src/states_screens/kart_selection.cpp
	src/states_screens/kart_selection.hpp
	src/states_screens/main_menu_screen.cpp
	src/states_screens/main_menu_screen.hpp
	src/states_screens/minimal_race_gui.cpp
	src/states_screens/minimal_race_gui.hpp
	src/states_screens/options_screen_audio.cpp
	src/states_screens/options_screen_audio.hpp
	src/states_screens/options_screen_input2.cpp
	src/states_screens/options_screen_input2.hpp
	src/states_screens/options_screen_input.cpp
	src/states_screens/options_screen_input.hpp
	src/states_screens/options_screen_players.cpp
	src/states_screens/options_screen_players.hpp
	src/states_screens/options_screen_ui.cpp
	src/states_screens/options_screen_video.cpp
	src/states_screens/options_screen_video.hpp
	src/states_screens/race_gui_base.cpp
	src/states_screens/race_gui_base.hpp
	src/states_screens/race_gui.cpp
	src/states_screens/race_gui.hpp
	src/states_screens/race_result_gui.cpp
	src/states_screens/race_result_gui.hpp
	src/states_screens/race_setup_screen.cpp
	src/states_screens/race_setup_screen.hpp
	src/states_screens/state_manager.cpp
	src/states_screens/state_manager.hpp
	src/states_screens/tracks_screen.cpp
	src/states_screens/tracks_screen.hpp
	src/states_screens/tutorial_screen.cpp
	src/states_screens/tutorial_screen.hpp
	src/tinygettext/dictionary.cpp
	src/tinygettext/dictionary.hpp
	src/tinygettext/dictionary_manager.cpp
	src/tinygettext/file_system.hpp
	src/tinygettext/iconv.cpp
	src/tinygettext/iconv.hpp
	src/tinygettext/language.cpp
	src/tinygettext/language.hpp
	src/tinygettext/log.cpp
	src/tinygettext/log.hpp
	src/tinygettext/log_stream.hpp
	src/tinygettext/plural_forms.cpp
	src/tinygettext/plural_forms.hpp
	src/tinygettext/po_parser.cpp
	src/tinygettext/po_parser.hpp
	src/tinygettext/stk_file_system.cpp
	src/tinygettext/stk_file_system.hpp
	src/tinygettext/tinygettext.cpp
	src/tinygettext/tinygettext.hpp
	src/tracks/ambient_light_sphere.cpp
	src/tracks/ambient_light_sphere.hpp
	src/tracks/bezier_curve.cpp
	src/tracks/bezier_curve.hpp
	src/tracks/check_lap.cpp
	src/tracks/check_lap.hpp
	src/tracks/check_line.cpp
	src/tracks/check_line.hpp
	src/tracks/check_manager.cpp
	src/tracks/check_manager.hpp
	src/tracks/check_sphere.cpp
	src/tracks/check_sphere.hpp
	src/tracks/check_structure.cpp
	src/tracks/check_structure.hpp
	src/tracks/graph_node.cpp
	src/tracks/graph_node.hpp
	src/tracks/quad.cpp
	src/tracks/quad_graph.cpp
	src/tracks/quad_graph.hpp
	src/tracks/quad.hpp
	src/tracks/quad_set.cpp
	src/tracks/quad_set.hpp
	src/tracks/terrain_info.cpp
	src/tracks/terrain_info.hpp
	src/tracks/track.cpp
	src/tracks/track_manager.cpp
	src/tracks/track_manager.hpp
	src/tracks/track_object.cpp
	src/tracks/track_object.hpp
	src/tracks/track_object_manager.cpp
	src/tracks/track_object_manager.hpp
 	src/tracks/track_sector.cpp
	src/tracks/track_sector.hpp
	src/tutorial/tutorial.cpp
	src/tutorial/tutorial_data.cpp
	src/tutorial/tutorial_data.hpp
	src/tutorial/tutorial.hpp
	src/tutorial/tutorial_manager.cpp
	src/tutorial/tutorial_manager.hpp
	src/utils/aligned_array.hpp
	src/utils/constants.cpp
	src/utils/constants.hpp
	src/utils/leak_check.cpp
	src/utils/leak_check.hpp
	src/utils/no_copy.hpp
	src/utils/profiler.cpp
	src/utils/profiler.hpp
	src/utils/ptr_vector.hpp
	src/utils/random_generator.cpp
	src/utils/random_generator.hpp
	src/utils/string_utils.cpp
	src/utils/string_utils.hpp
	src/utils/synchronised.hpp
	src/utils/time.hpp
	src/utils/translation.cpp
	src/utils/translation.hpp
	src/utils/utf8/core.h
	src/utils/utf8.h
	src/utils/utf8/unchecked.h
	src/utils/vec3.cpp
	src/utils/vec3.hpp)

if (APPLE)		
    # icon files to copy in the bundle
    set( OSX_ICON_FILES ${PROJECT_SOURCE_DIR}/src/ide/Xcode/stk.icns)
    set_source_files_properties( ${OSX_ICON_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set( SRCS ${SRCS} ${OSX_ICON_FILES} )
    
    # build the executable and create an app bundle
    add_executable(supertuxkart MACOSX_BUNDLE ${SRCS} )
    
    target_link_libraries(supertuxkart
	    bulletdynamics
	    bulletcollision
	    bulletmath
	    enet
	    ${CURL_LIBRARIES}
	    # We could use smarter detection of the frameworks but supporting a single official setup makes things easy...
	    /Library/Frameworks/IrrFramework.framework
	    /Library/Frameworks/Ogg.framework
	    /Library/Frameworks/OpenAL.framework
	    /Library/Frameworks/Vorbis.framework
	    # TODO: allow linking against frameworks from a SDK.
	    /System/Library/Frameworks/AGL.framework
	    /System/Library/Frameworks/IOKit.framework
	    /System/Library/Frameworks/QuickTime.framework
	    /System/Library/Frameworks/Carbon.framework
	    /System/Library/Frameworks/AudioUnit.framework
	    /System/Library/Frameworks/Cocoa.framework
	    /System/Library/Frameworks/OpenGL.framework)
	    
    # configure CMake to use a custom Info.plist
    set_target_properties(supertuxkart PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/src/ide/Xcode/SuperTuxKart-Info.plist )
  
    add_definitions(`ln -sf ${PROJECT_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/bin/supertuxkart.app/Contents/Resources`)

else()

    add_definitions(-DSUPERTUXKART_DATADIR=\"${CMAKE_INSTALL_PREFIX}/share/games/supertuxkart\")

    # Build the final executable
    add_executable(supertuxkart ${SRCS})

    target_link_libraries(supertuxkart
	    Irrlicht
	    bulletdynamics
	    bulletcollision
	    bulletmath
	    enet
	    pthread
	    ${CURL_LIBRARIES}
	    ${OPENGL_LIBRARIES}
	    ${OPENAL_LIBRARY}
	    vorbisfile)
endif()

if(USE_FRIBIDI)
    if(APPLE)
	    target_link_libraries(supertuxkart /Library/Frameworks/fribidi.framework)
    else()
        target_link_libraries(supertuxkart fribidi)
    endif()
    add_definitions(-DENABLE_BIDI=1)
endif()

if(UNIX AND NOT APPLE)
	target_link_libraries(supertuxkart Xxf86vm)
elseif(APPLE)
	# We could use smarter detection of the frameworks but supporting a single official setup makes things easy...
	target_link_libraries(supertuxkart /Library/Frameworks/IrrFramework.framework)
	target_link_libraries(supertuxkart /Library/Frameworks/Ogg.framework)
	target_link_libraries(supertuxkart /Library/Frameworks/OpenAL.framework)
	target_link_libraries(supertuxkart /Library/Frameworks/Vorbis.framework)

	target_link_libraries(supertuxkart /System/Library/Frameworks/AGL.framework)
	target_link_libraries(supertuxkart /System/Library/Frameworks/IOKit.framework)
	target_link_libraries(supertuxkart /System/Library/Frameworks/QuickTime.framework)
	target_link_libraries(supertuxkart /System/Library/Frameworks/Carbon.framework)
	target_link_libraries(supertuxkart /System/Library/Frameworks/AudioUnit.framework)
	target_link_libraries(supertuxkart /System/Library/Frameworks/Cocoa.framework)
	target_link_libraries(supertuxkart /System/Library/Frameworks/OpenGL.framework)
endif()

# ==== Make dist target ====
add_custom_target(dist
    COMMAND rm -rf ${CMAKE_BINARY_DIR}/SuperTuxKart-${PROJECT_VERSION} && rm -f ${CMAKE_BINARY_DIR}/SuperTuxKart-${PROJECT_VERSION}.tar.bz2
            && echo "Exporting..."
            && svn export ${PROJECT_SOURCE_DIR} ${CMAKE_BINARY_DIR}/SuperTuxKart-${PROJECT_VERSION}
            && echo "Compressing..."
            && cd ${CMAKE_BINARY_DIR}
            && tar -cjf ${CMAKE_BINARY_DIR}/SuperTuxKart-${PROJECT_VERSION}.tar.bz2 ./SuperTuxKart-${PROJECT_VERSION}
            && echo "Done, cleaning up"
            && rm -rf ${CMAKE_BINARY_DIR}/SuperTuxKart-${PROJECT_VERSION}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# ==== Install target ====
MACRO(installfilemacro filepath)
    set(source_regex "(.*)/data/(.*)")
    set(svn_regex "(.*).svn(.*)")
    
    string(REGEX MATCH "${svn_regex}" is_svn_dir "${curr}")
    if(is_svn_dir)
        # don't install SVN files
    else()
        string(REGEX MATCH "${source_regex}" source_path "${curr}")
        if(source_path)
            string(REGEX REPLACE "${source_regex}" "\\2" after_source "${source_path}")
            get_filename_component(install_location ${after_source} PATH)
            install(FILES ${filepath} DESTINATION "share/games/supertuxkart/data/${install_location}")
        endif(source_path)
    endif()
ENDMACRO(installfilemacro)

FILE(GLOB_RECURSE datafiles "${CMAKE_CURRENT_SOURCE_DIR}/data/*")
foreach(curr ${datafiles})
    installfilemacro(${curr})
endforeach()


install(TARGETS supertuxkart RUNTIME DESTINATION games BUNDLE DESTINATION .)
install(FILES data/supertuxkart.desktop DESTINATION share/applications)
install(FILES data/supertuxkart_32.xpm data/supertuxkart_64.xpm DESTINATION share/pixmaps)

add_custom_target(GenerateDesktopFile
                  COMMAND sed 's\#PREFIX\#${CMAKE_INSTALL_PREFIX}\#' ${CMAKE_CURRENT_SOURCE_DIR}/data/supertuxkart_desktop.template | sed 's\#VERSION\#${PROJECT_VERSION}\#' > ${CMAKE_CURRENT_SOURCE_DIR}/data/supertuxkart.desktop
                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
ADD_DEPENDENCIES(supertuxkart GenerateDesktopFile)


