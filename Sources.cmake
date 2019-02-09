#
#  Sources.cmake
#  RtmpKit
#
#  Created by Diego Stamigni on 11/08/2016.
#  Copyright © 2016 Diego Stamigni. All rights reserved.
#

set (ANDROID_BRIDGES
)

set (JAVA_BRIDGES
        private/jni/utils.h
        src/jni/bridge_utils.cc

        src/jni/bridge_watcher_session.cc
        
        # android-specific stuff
        ${ANDROID_BRIDGES}
)

set (APPLE_BRIDGES
        include/apple/KBSession.h
        include/apple/KBWatcherSessionDelegate.h
        include/apple/KBBroadcasterSessionDelegate.h

        include/apple/KBLibConfig.h
        src/apple/bridge_lib_config.mm

        include/apple/KBNetworkConfiguration.h
        src/apple/bridge_network_configuration.mm

        include/apple/KBWatcherSessionView.h
        src/apple/bridge_watcher_session_view.mm

        include/apple/KBRtmpWatcherSessionView.h
        src/apple/bridge_rtmp_watcher_session_view.mm

        include/apple/KBBroadcasterSessionView.h
        src/apple/bridge_broadcaster_session_view.mm
        
        include/apple/KBRtmpBroadcasterSessionView.h
        src/apple/bridge_rtmp_broadcaster_session_view.mm
        
        include/apple/KBRtmpSessionSettings.h
        src/apple/bridge_rtmp_session_settings.mm

        private/apple/KBVideoCompressorSessionDelegate.h
        private/apple/KBAudioCaptureSessionDelegate.h
        private/apple/KBVideoDecompressorSessionDelegate.h
        private/apple/KBAudioRendererSessionDelegate.h

        private/apple/KBOpenGLBasedPreviewView.h
        src/apple/bridge_opengl_preview_view.mm

        private/apple/KBAVAudioSessionManager.h
        src/apple/bridge_avaudiosession_manager.mm

        private/apple/KBAudioRendererSession.h
        src/apple/bridge_audio_renderer_session.mm

        private/apple/KBVideoDecompressorSession.h
        src/apple/bridge_video_decompressor_session.mm

        private/apple/KBFLVReader.h
        src/apple/bridge_flv_reader.mm
        
        private/apple/KBFLVMediaTags.h
        src/apple/bridge_flv_media_tags.mm
        
        private/apple/KBUtility.h
        src/apple/bridge_utility.mm
        
        private/apple/KBVideoCaptureSessionView.h
        src/apple/bridge_video_capture_session_view.mm
        
        private/apple/KBVideoCaptureSession.h
        src/apple/bridge_video_capture_session.mm
        
        private/apple/KBVideoCompressorSession.h
        src/apple/bridge_video_compressor_session.mm
        
        private/apple/KBAudioCaptureSession.h
        src/apple/bridge_audio_capture_session.mm
        
        3rdparty/SAMContentMode/SAMContentMode/SAMContentMode.h
        3rdparty/SAMContentMode/SAMContentMode/SAMContentMode.mm)

set (AMF_SOURCES
        private/amf/amf.h
        private/amf/types.h

        src/amf/0/amf0_serializer.cc
        src/amf/0/amf0_deserializer.cc)

set (RTMP
        private/rtmp/rtmp_time.h
        private/rtmp/rtmp_utils.h
        private/rtmp/rtmp_socket.h
        private/rtmp/session/rtmp_session.h
        private/rtmp/session/rtmp_client_session.h
        private/rtmp/rtmp_packet_generator.h
        private/rtmp/rtmp_packet.h
        private/rtmp/message/rtmp_message.h
        private/rtmp/message/rtmp_media_message.h
        private/rtmp/message/rtmp_video_message.h
        private/rtmp/message/rtmp_audio_message.h
        private/rtmp/message/rtmp_metadata_message.h
        private/rtmp/session/rtmp_session_settings.h
        private/rtmp/session/rtmp_broadcaster_session.h
        private/rtmp/session/rtmp_watcher_session.h
        private/rtmp/handshake/rtmp_handshake_packet_generator.h
        private/rtmp/handshake/rtmp_handshake_packet.h
        private/rtmp/handshake/rtmp_handshake_packet_c0.h
        private/rtmp/handshake/rtmp_handshake_packet_c1.h
        private/rtmp/handshake/rtmp_handshake_packet_c2.h
        private/rtmp/handshake/rtmp_handshake_packet_s0.h
        private/rtmp/handshake/rtmp_handshake_packet_s1.h
        private/rtmp/handshake/rtmp_handshake_packet_s2.h
        private/rtmp/message/control/rtmp_window_ack_size_message.h
        private/rtmp/message/control/rtmp_set_peer_bandwidth_message.h
        private/rtmp/message/control/rtmp_set_chunk_size_message.h
        private/rtmp/message/control/rtmp_stream_begin_control_message.h
        private/rtmp/message/command/rtmp_call_command_message.h
        private/rtmp/message/command/rtmp_close_command_message.h
        private/rtmp/message/command/rtmp_command_message.h
        private/rtmp/message/command/rtmp_command_response_message.h
        private/rtmp/message/command/rtmp_connect_command_message.h
        private/rtmp/message/command/rtmp_connect_command_response_message.h
        private/rtmp/message/command/rtmp_create_stream_command_message.h
        private/rtmp/message/command/rtmp_create_stream_command_response_message.h
        private/rtmp/message/command/rtmp_publish_command_message.h
        private/rtmp/message/command/rtmp_onstatus_command_response_message.h
        private/rtmp/message/command/rtmp_close_stream_command_message.h
        private/rtmp/message/command/rtmp_release_stream_command_message.h
        private/rtmp/message/command/rtmp_fcpublish_command_message.h
        private/rtmp/message/command/rtmp_onfcpublish_command_response_message.h
        private/rtmp/message/command/rtmp_play_command_message.h
        private/rtmp/message/command/rtmp_sample_access_message.h

        src/rtmp/handshake/rtmp_handshake_packet_generator.cc
        src/rtmp/handshake/rtmp_handshake_packet.cc
        src/rtmp/handshake/rtmp_handshake_packet_c0.cc
        src/rtmp/handshake/rtmp_handshake_packet_c1.cc
        src/rtmp/handshake/rtmp_handshake_packet_c2.cc
        src/rtmp/handshake/rtmp_handshake_packet_s0.cc
        src/rtmp/handshake/rtmp_handshake_packet_s1.cc
        src/rtmp/handshake/rtmp_handshake_packet_s2.cc
        src/rtmp/rtmp_socket.cc
        src/rtmp/session/rtmp_session.cc
        src/rtmp/session/rtmp_client_session.cc
        src/rtmp/message/rtmp_message.cc
        src/rtmp/message/rtmp_media_message.cc
        src/rtmp/message/rtmp_video_message.cc
        src/rtmp/message/rtmp_audio_message.cc
        src/rtmp/message/rtmp_metadata_message.cc
        src/rtmp/session/rtmp_session_settings.cc
        src/rtmp/session/rtmp_watcher_session.cc
        src/rtmp/session/rtmp_broadcaster_session.cc
        src/rtmp/message/command/rtmp_call_command_message.cc
        src/rtmp/message/command/rtmp_close_command_message.cc
        src/rtmp/message/command/rtmp_command_message.cc
        src/rtmp/message/command/rtmp_connect_command_message.cc
        src/rtmp/message/command/rtmp_connect_command_response_message.cc
        src/rtmp/message/command/rtmp_create_stream_command_message.cc
        src/rtmp/message/command/rtmp_create_stream_command_response_message.cc
        src/rtmp/message/command/rtmp_publish_command_message.cc
        src/rtmp/message/command/rtmp_onstatus_command_response_message.cc
        src/rtmp/message/control/rtmp_set_chunk_size_message.cc
        src/rtmp/message/control/rtmp_set_peer_bandwidth_message.cc
        src/rtmp/message/control/rtmp_window_ack_size_message.cc
        src/rtmp/message/control/rtmp_stream_begin_control_message.cc
        src/rtmp/message/command/rtmp_close_stream_command_message.cc
        src/rtmp/message/command/rtmp_release_stream_command_message.cc
        src/rtmp/message/command/rtmp_fcpublish_command_message.cc
        src/rtmp/message/command/rtmp_onfcpublish_command_response_message.cc
        src/rtmp/message/command/rtmp_play_command_message.cc
        src/rtmp/message/command/rtmp_sample_access_message.cc)

set (SOCKET
        private/socket/socket_utils.h
        private/socket/base_socket.h
        src/socket/base_socket.cc)

set (READER_SOURCES
        private/reader.h
        src/reader.cc)

set (FLV_SOURCES
        private/flv/flv_types.h
        private/flv/flv_player.h
        private/flv/flv_mediaitem.h
        private/flv/flv_mediaheader.h
        private/flv/flv_mediatag.h
        private/flv/flv_amf_mediatag.h
        private/flv/flv_audio_mediatag.h
        private/flv/flv_video_mediatag.h

        src/flv/flv_player.cc
        src/flv/flv_mediaheader.cc
        src/flv/flv_mediaitem.cc
        src/flv/flv_mediatag.cc
        src/flv/flv_amf_mediatag.cc
        src/flv/flv_audio_mediatag.cc
        src/flv/flv_video_mediatag.cc)

set(SOURCES
        include/filetype.h
        include/version.h
        include/network_types.h

        private/logger.h
        private/network_performance_metrics.h
        private/queue.h
        private/typified.h
        private/constants.h
        private/targets.h
        private/mediaheader.h
        private/mediaitem.h
        private/mediatag.h
        private/parsable.h
        private/player.h
        private/utils.h
        private/multicast_delegate.h
        private/semaphore.h
        private/typed_tag.h
        private/bufferable.h
        private/audio_render_item.h
        private/buffer_queue.h
        private/network_speed_indicator.h
        private/audiodescriptor.h
        private/videodescriptor.h
        private/mediautils.h
        
        src/targets.cc
        src/mediatag.cc
        src/semaphore.cc
        src/typed_tag.cc
        src/network_performance_metrics.cc
        src/audio_render_item.cc
        src/network_speed_indicator.cc
        src/audiodescriptor.cc
        src/videodescriptor.cc
        src/mediautils.cc

        ${READER_SOURCES}
        ${AMF_SOURCES}
        ${FLV_SOURCES}
        ${SOCKET}
        ${RTMP})
