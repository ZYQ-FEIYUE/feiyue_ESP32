set(COMPONENT_SRCS "app_main.c"
                    "play_mp3.c"
					"asr.c")
set(COMPONENT_ADD_INCLUDEDIRS .)
set(COMPONENT_EMBED_TXTFILES dingdong.mp3)
register_component()