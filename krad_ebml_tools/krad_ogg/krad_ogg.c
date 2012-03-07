//#include "krad_io.h"
#include "krad_ogg.h"

int krad_ogg_track_count(krad_ogg_t *krad_ogg) {

	int t;
	int count;

	count = 0;

	for (t = 0; t < KRAD_OGG_MAX_TRACKS; t++) {
		if ((krad_ogg->tracks[t].serial != 0) && (krad_ogg->tracks[t].ready == 1)) {
			count++;
		}
	}

	return count;

}

krad_codec_t krad_ogg_track_codec (krad_ogg_t *krad_ogg, int track) {

	if ((krad_ogg->tracks[track].serial != 0) && (krad_ogg->tracks[track].ready == 1)) {
		return krad_ogg->tracks[track].codec;
	}

	return NOCODEC;

}


int krad_ogg_track_header_count(krad_ogg_t *krad_ogg, int track) {

	if ((krad_ogg->tracks[track].serial != 0) && (krad_ogg->tracks[track].ready == 1)) {
		return krad_ogg->tracks[track].header_count;
	}

	return 0;

}

int krad_ogg_track_header_size (krad_ogg_t *krad_ogg, int track, int header) {

	if ((krad_ogg->tracks[track].serial != 0) && (krad_ogg->tracks[track].ready == 1)) {
		return krad_ogg->tracks[track].header_len[header];
	}

	return 0;

}

int krad_ogg_read_track_header(krad_ogg_t *krad_ogg, unsigned char *buffer, int track, int header) {

	if ((krad_ogg->tracks[track].serial != 0) && (krad_ogg->tracks[track].ready == 1)) {
		memcpy(buffer, krad_ogg->tracks[track].header[header], krad_ogg->tracks[track].header_len[header]);
		return krad_ogg->tracks[track].header_len[header];
	}

	return 0;

}

int krad_ogg_track_active (krad_ogg_t *krad_ogg, int track) {

	if (krad_ogg->tracks[track].serial != 0) {
		return 1;
	}

	return 0;

}

int krad_ogg_track_changed (krad_ogg_t *krad_ogg, int track) {

	if (krad_ogg->tracks[track].serial != krad_ogg->tracks[track].last_serial) {
		krad_ogg->tracks[track].last_serial = krad_ogg->tracks[track].serial;
		return 1;
	}

	return 0;

}

float krad_ogg_vorbis_sample_rate(ogg_packet *packet) {

	float sample_rate;
	
	vorbis_info v_info;
	vorbis_comment v_comment;

	vorbis_info_init(&v_info);
	vorbis_comment_init(&v_comment);

	vorbis_synthesis_headerin(&v_info, &v_comment, packet);

	sample_rate = v_info.rate;
	printf("the sample rate is %f\n", sample_rate);

	vorbis_info_clear(&v_info);
	vorbis_comment_clear(&v_comment);

	return sample_rate;

}

int krad_ogg_theora_frame_rate (ogg_packet *packet) {

	int frame_rate;
	
    theora_comment t_comment;
    theora_info t_info;

    theora_info_init (&t_info);
	theora_comment_init (&t_comment);


	theora_decode_header (&t_info, &t_comment, packet);

	frame_rate = t_info.fps_numerator;
	printf("the frame rate is %d\n", frame_rate);

	theora_info_clear (&t_info);
	theora_comment_clear (&t_comment);

	return frame_rate;

}

int krad_ogg_theora_keyframe_shift (ogg_packet *packet) {

	int keyframe_shift;
	
    theora_comment t_comment;
    th_info t_info;

    th_info_init (&t_info);
	theora_comment_init (&t_comment);


	//theora_decode_header (&t_info, &t_comment, packet);

	keyframe_shift = t_info.keyframe_granule_shift;
	printf("the keyframe shift is %d\n", keyframe_shift);

	th_info_clear (&t_info);
	theora_comment_clear (&t_comment);

	return keyframe_shift;

}

krad_codec_t krad_ogg_get_codec (ogg_packet *packet) {

	krad_codec_t codec;
    theora_comment t_comment;
    theora_info t_info;
   
	codec = NOCODEC;
   

	if (memcmp(packet->packet, "fishead\0", 8) == 0) {
        printf("found skeleton\n");
		return NOCODEC;
	}
   
	if (memcmp (packet->packet + 1, "FLAC", 4) == 0) {
        printf("found flac\n");
		return FLAC;
	}
	
	if (memcmp (packet->packet, "Opus", 4) == 0) {
        printf("found opus\n");
		return OPUS;
	}

    if (vorbis_synthesis_idheader (packet) == 1) {
        printf("found vorbis\n");
        codec = VORBIS;
    }
    
    if (codec != NOCODEC) {
    	return codec;
    }
    
	theora_info_init (&t_info);
	theora_comment_init (&t_comment);

    if (theora_decode_header (&t_info, &t_comment, packet) == 0) {
        printf("found theora\n");
        codec = THEORA;
    }
    
	theora_info_clear (&t_info);
	theora_comment_clear (&t_comment);
    
    if (codec != NOCODEC) {
    	return codec;
    }
 
 	printf("sucky\n");
 
	return NOCODEC;
   
} 


int krad_ogg_read_packet (krad_ogg_t *krad_ogg, int *track, uint64_t *timecode, unsigned char *buffer) {

	int t;
	int ret;
	ogg_packet packet;
	
	while (1) {
	
		for (t = 0; t < KRAD_OGG_MAX_TRACKS; t++) {
			if ((krad_ogg->tracks[t].serial != 0) && (krad_ogg->tracks[t].ready == 0)) {
				while (krad_ogg->tracks[t].ready == 0) {
					ret = ogg_stream_packetout(&krad_ogg->tracks[t].stream_state, &packet);
					if (ret == 1) {
					
						krad_ogg->tracks[t].header_len[krad_ogg->tracks[t].header_count] = packet.bytes;
					
						krad_ogg->tracks[t].header[krad_ogg->tracks[t].header_count] = malloc(packet.bytes);
						memcpy(krad_ogg->tracks[t].header[krad_ogg->tracks[t].header_count], packet.packet, packet.bytes);
					
						if (krad_ogg->tracks[t].header_count == 0) {
					
							krad_ogg->tracks[t].codec = krad_ogg_get_codec(&packet);
							if (krad_ogg->tracks[t].codec == VORBIS) {
								krad_ogg->tracks[t].sample_rate = krad_ogg_vorbis_sample_rate(&packet);
							}
							
							if (krad_ogg->tracks[t].codec == THEORA) {
								krad_ogg->tracks[t].frame_rate = krad_ogg_theora_frame_rate(&packet);
								krad_ogg->tracks[t].keyframe_shift = krad_ogg_theora_keyframe_shift (&packet);
							}
							
						}
					
					
						krad_ogg->tracks[t].header_count++;
						if ((krad_ogg->tracks[t].header_count == 3) && ((krad_ogg->tracks[t].codec == VORBIS) || (krad_ogg->tracks[t].codec == THEORA))) {					
							krad_ogg->tracks[t].ready = 1;
						}
						if ((krad_ogg->tracks[t].header_count == 2) && ((krad_ogg->tracks[t].codec == OPUS) || (krad_ogg->tracks[t].codec == FLAC))) {					
							krad_ogg->tracks[t].header_count = 1;
							
							if (krad_ogg->tracks[t].codec == FLAC) {
								// remove oggflac extra stuff
								memmove(krad_ogg->tracks[t].header[0], krad_ogg->tracks[t].header[0] + 9, krad_ogg->tracks[t].header_len[0] - 9);
								krad_ogg->tracks[t].header_len[0] -= 9;
								if (krad_ogg->tracks[t].header_len[0] != 42) {
									printf("ruh oh! our oggflac expectations where not met, problem likely!\n");
								}
							}
							
							
							krad_ogg->tracks[t].ready = 1;
						}
					} else {
						break;
					}
				}
			}
		}
	
		for (t = 0; t < KRAD_OGG_MAX_TRACKS; t++) {
			if ((krad_ogg->tracks[t].serial != 0) && (krad_ogg->tracks[t].ready == 1)) {
				ret = ogg_stream_packetout(&krad_ogg->tracks[t].stream_state, &packet);
				if (ret == 1) {
					memcpy(buffer, packet.packet, packet.bytes);
					*track = t;
				
					if (packet.e_o_s) {
						ogg_stream_clear(&krad_ogg->tracks[t].stream_state);
						krad_ogg->tracks[t].serial = 0;
						krad_ogg->tracks[t].ready = 0;
						krad_ogg->tracks[t].header_count = 0;
						krad_ogg->tracks[t].codec = NOCODEC;
						while (krad_ogg->tracks[t].header_count) {
							free (krad_ogg->tracks[t].header[krad_ogg->tracks[t].header_count - 1]);
							krad_ogg->tracks[t].header[krad_ogg->tracks[t].header_count - 1] = NULL;
							krad_ogg->tracks[t].header_count--;
						}
					}
					if (packet.granulepos != -1) {
						if (krad_ogg->tracks[t].codec == VORBIS) {
							krad_ogg->tracks[t].last_granulepos = (packet.granulepos / krad_ogg->tracks[t].sample_rate) * 1000.0;
						}
						
						if (krad_ogg->tracks[t].codec == THEORA) {
							ogg_int64_t iframe;
							ogg_int64_t pframe;
							iframe = packet.granulepos >> krad_ogg->tracks[t].keyframe_shift;
							pframe = packet.granulepos - (iframe << krad_ogg->tracks[t].keyframe_shift);
							/* kludged, we use the default shift of 6 and assume a 3.2.1+ bitstream */
							krad_ogg->tracks[t].last_granulepos = ((iframe + pframe - 1) / krad_ogg->tracks[t].frame_rate) * 1000.0;
						}
					}
					*timecode = krad_ogg->tracks[t].last_granulepos;
					return packet.bytes;
				}
			}
		}
	
	
		ret = krad_io_read(krad_ogg->krad_io, krad_ogg->input_buffer, 4096);
		if (ret > 0) {
			krad_ogg_write (krad_ogg, krad_ogg->input_buffer, ret);
		} else {
			break;
		}

	}

	return 0;

}


void krad_ogg_process (krad_ogg_t *krad_ogg) {

	ogg_page page;
	int serial;
	int t;
	
	while (ogg_sync_pageout(&krad_ogg->sync_state, &page) == 1) {
		
		serial = ogg_page_serialno(&page);
		
		if (ogg_page_bos(&page)) {
		
			for (t = 0; t < KRAD_OGG_MAX_TRACKS; t++) {
				if (krad_ogg->tracks[t].serial == 0) {
					break;
				}
			}

			krad_ogg->tracks[t].serial = serial;
			ogg_stream_init(&krad_ogg->tracks[t].stream_state, serial);
			ogg_stream_pagein(&krad_ogg->tracks[t].stream_state, &page);
			
			continue;
		}
		
		for (t = 0; t < KRAD_OGG_MAX_TRACKS; t++) {
			if (krad_ogg->tracks[t].serial == serial) {
				break;
			}
		}

		if ( t != KRAD_OGG_MAX_TRACKS) {
			ogg_stream_pagein(&krad_ogg->tracks[t].stream_state, &page);
		}
	}

}


int krad_ogg_write (krad_ogg_t *krad_ogg, unsigned char *buffer, int length) {

    char* input_buffer;
    int ret;
    
	input_buffer = ogg_sync_buffer(&krad_ogg->sync_state, length);
    memcpy (input_buffer, buffer, length);
    ret = ogg_sync_wrote(&krad_ogg->sync_state, length);

	if (ret != 0) {
		printf("uh oh! ogg_sync_wrote problem\n");
		return -1;
	}


	krad_ogg_process (krad_ogg);

	return length;

}



void krad_ogg_destroy(krad_ogg_t *krad_ogg) {
	
	int t;
	
	ogg_sync_clear(&krad_ogg->sync_state);

	for (t = 0; t < krad_ogg->track_count; t++) {
		while (krad_ogg->tracks[t].header_count) {
			free (krad_ogg->tracks[t].header[krad_ogg->tracks[t].header_count - 1]);
			krad_ogg->tracks[t].header[krad_ogg->tracks[t].header_count - 1] = NULL;
			krad_ogg->tracks[t].header_count--;
		}
	}
	
	free(krad_ogg->tracks);
	
	free(krad_ogg->input_buffer);
	
	if (krad_ogg->krad_io) {
		krad_io_destroy(krad_ogg->krad_io);
	}
	
	free (krad_ogg);

}

krad_ogg_t *krad_ogg_create() {

	krad_ogg_t *krad_ogg = calloc(1, sizeof(krad_ogg_t));

	krad_ogg->tracks = calloc(KRAD_OGG_MAX_TRACKS, sizeof(krad_ogg_track_t));

	krad_ogg->input_buffer = calloc(1, 4096);

	ogg_sync_init(&krad_ogg->sync_state);

	return krad_ogg;

}


krad_ogg_t *krad_ogg_open_file(char *filename, krad_io_mode_t mode) {

	krad_ogg_t *krad_ogg;
	
	krad_ogg = krad_ogg_create();
	
	krad_ogg->krad_io = krad_io_open_file(filename, mode);
	
	return krad_ogg;

}

krad_ogg_t *krad_ogg_open_stream(char *host, int port, char *mount, char *password) {

	krad_ogg_t *krad_ogg;

	krad_ogg = krad_ogg_create();
	
	krad_ogg->krad_io = krad_io_open_stream(host, port, mount, password);
	
	return krad_ogg;
}