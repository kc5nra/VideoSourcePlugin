#pragma once

#define VLC21

#ifdef VLC21
    
    #pragma include_alias (<vlc/vlc.h>                      , <vlc21/vlc.h>                        )
    #pragma include_alias (<vlc/libvlc_structures.h>        , <vlc21/libvlc_structures.h>          )
    #pragma include_alias (<vlc/libvlc.h>                   , <vlc21/libvlc.h>                     )
    #pragma include_alias (<vlc/libvlc_media.h>             , <vlc21/libvlc_media.h>               )
    #pragma include_alias (<vlc/libvlc_media_player.h>      , <vlc21/libvlc_media_player.h>        )
    #pragma include_alias (<vlc/libvlc_media_list.h>        , <vlc21/libvlc_media_list.h>          )
    #pragma include_alias (<vlc/libvlc_media_list_player.h> , <vlc21/libvlc_media_list_player.h>   )
    #pragma include_alias (<vlc/libvlc_media_library.h>     , <vlc21/libvlc_media_library.h>       )
    #pragma include_alias (<vlc/libvlc_media_discoverer.h>  , <vlc21/libvlc_media_discoverer.h>    )
    #pragma include_alias (<vlc/libvlc_events.h>            , <vlc21/libvlc_events.h>              )
    #pragma include_alias (<vlc/libvlc_vlm.h>               , <vlc21/libvlc_vlm.h>                 )
    #pragma include_alias (<vlc/deprecated.h>               , <vlc21/deprecated.h>                 )

    #include "vlc21\vlc.h"

    #ifdef _WIN64
        #pragma comment(lib, "libvlc21-x64.lib")
    #else
        #pragma comment(lib, "libvlc21-x86.lib")
    #endif

#else //VLC20

    #include "vlc\vlc.h"

    #ifdef _WIN64
        #pragma comment(lib, "libvlc20-x64.lib")
    #else
        #pragma comment(lib, "libvlc20-x86.lib")
    #endif
    
#endif