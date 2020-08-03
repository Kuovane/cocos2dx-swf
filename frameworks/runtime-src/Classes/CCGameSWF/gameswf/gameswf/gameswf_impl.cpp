// gameswf_impl.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Some implementation for SWF player.

// Useful links:
//
// http://sswf.sourceforge.net/SWFalexref.html
// http://www.openswf.org

//  Modify by chenee (chenee543216@gmail.com) on 13-05-20
//  @chenee: 1\change Objective-C to C++ ;2\change opengl es 1.1 to opengl es 2.0
//

#include"cocos2d.h"
#include "tu_file.h"
#include "utility.h"
#include "gameswf_action.h"
#include "gameswf_button.h"
#include "gameswf_impl.h"
#include "gameswf_font.h"
#include "gameswf_log.h"
#include "gameswf_morph2.h"
#include "gameswf_video_impl.h"
#include "gameswf_render.h"
#include "gameswf_shape.h"
#include "gameswf_stream.h"
#include "gameswf_styles.h"
#include "gameswf_dlist.h"
#include "gameswf_root.h"
#include "gameswf_movie_def.h"
#include "gameswf_sprite_def.h"
#include "gameswf_sprite.h"
#include "gameswf_function.h"
#include "gameswf_abc.h"
#include "gameswf_filters.h"
#include "image.h"
//#include "jpeg.h"
#include "zlib_adapter.h"
#include "tu_random.h"
#include <string.h>	// for memset
#include <typeinfo>
#include <float.h>
#include <map>
#include "image/stb_image_write.h"
#include "image/stb_image_resize.h"
#include "gameswf_tesselate.h"
//#include "utils/MyUtils.h"


#if TU_CONFIG_LINK_TO_ZLIB
#include <zlib.h>
#endif // TU_CONFIG_LINK_TO_ZLIB

namespace gameswf
{
	bool	s_verbose_action = false;
	bool	s_verbose_parse = false;
	bool	s_use_cached_movie_instance = false;

#ifndef NDEBUG
	bool	s_verbose_debug = true;
#else
	bool	s_verbose_debug = false;
#endif

	bool get_verbose_parse()
	{
		return s_verbose_parse;
	}

	bool get_verbose_debug()
	{
		return s_verbose_debug;
	}

	bool get_verbose_action()
	{
		return s_verbose_action;
	}

	void	set_verbose_action(bool verbose)
		// Enable/disable log messages re: actions.
	{
		s_verbose_action = verbose;
	}


	void	set_verbose_parse(bool verbose)
	// Enable/disable log messages re: parsing the movie.
	{
		s_verbose_parse = verbose;
	}

	//
	// some utility stuff
	//

	void	execute_actions(as_environment* env, const array<action_buffer*>& action_list)
	// Execute the actions in the action list, in the given
	// environment.
	{
		for (int i = 0; i < action_list.size(); i++)
		{
			action_list[i]->execute(env);
		}
	}

	character*	character_def::create_character_instance(character* parent, int id)
	// Default.  Make a generic_character.
	{
		character * ch = new generic_character(get_player(), this, parent, id);
		// instanciate_registered_class(ch);	//TODO: test it
		return ch;
	}

	void character_def::set_registered_class_constructor(const as_value& value)
	{
		m_registered_class_constructor = value.to_function();
	}

	void character_def::instanciate_registered_class (character* ch)
	{
		assert(ch);

		// flash9
		// try SymbolClass tag
		if (m_registered_class_constructor == NULL)
		{
			m_registered_class_constructor = instanciate_class(ch);
		}

		if (m_registered_class_constructor != NULL)
		{
			// as far as I remember
			// any function must have a prototype
			// action script class definition is a function and its
			// prototype contains set of function/methods of that class
			// any object must can have a proto
			// while creating new object as instance of class
			// all functions from prototype are copied to proto of object
			// so, proto of object contains all functions from all prototypes of
			// all super classes

			// we does not create prototype for func, it is exist already
			// But we must create proto for a object a call its constructor

			ch->m_this_ptr = ch;
			as_object* proto = ch->create_proto(m_registered_class_constructor.get_ptr());
			UNUSED(proto);

			// exec constructor
			ch->on_event(event_id::CONSTRUCT);	// set component args

			as_environment env(get_player());
			call_method(m_registered_class_constructor.get_ptr(), &env, ch, 0, 0);
		}
	}


	//
	// character
	//
    static bool	s_registered = false;
    void set_s_registered(bool flag)
    {
        s_registered = flag;
    }

	void	ensure_loaders_registered()
	{

		if (s_registered == false)
		{
			// Register the standard loaders.
			s_registered = true;
			register_tag_loader(0, end_loader);
			register_tag_loader(2, define_shape_loader);
			register_tag_loader(4, place_object_2_loader);
			register_tag_loader(5, remove_object_2_loader);
			register_tag_loader(6, define_bits_jpeg_loader);
			register_tag_loader(7, button_character_loader);
			register_tag_loader(8, jpeg_tables_loader);
			register_tag_loader(9, set_background_color_loader);
			register_tag_loader(10, define_font_loader);
			register_tag_loader(11, define_text_loader);
			register_tag_loader(12, do_action_loader);
			register_tag_loader(13, define_font_info_loader);
			register_tag_loader(14, define_sound_loader);
			register_tag_loader(15, start_sound_loader);
			register_tag_loader(17, button_sound_loader);
			register_tag_loader(18, sound_stream_head_loader);
			register_tag_loader(19, sound_stream_block_loader);
			register_tag_loader(20, define_bits_lossless_2_loader);
			register_tag_loader(21, define_bits_jpeg2_loader);
			register_tag_loader(22, define_shape_loader);
			register_tag_loader(24, null_loader);	// "protect" tag; we're not an authoring tool so we don't care.
			register_tag_loader(26, place_object_2_loader);
			register_tag_loader(28, remove_object_2_loader);
			register_tag_loader(32, define_shape_loader);
			register_tag_loader(33, define_text_loader);
			register_tag_loader(37, define_edit_text_loader);
			register_tag_loader(34, button_character_loader);
			register_tag_loader(35, define_bits_jpeg3_loader);
			register_tag_loader(36, define_bits_lossless_2_loader);
			register_tag_loader(39, sprite_loader);
			register_tag_loader(43, frame_label_loader);
			register_tag_loader(45, sound_stream_head_loader);
			register_tag_loader(46, define_shape_morph_loader);
			register_tag_loader(48, define_font_loader);
			register_tag_loader(56, export_loader);
			register_tag_loader(57, import_loader);
			register_tag_loader(58, define_enable_debugger_loader);
			register_tag_loader(59, do_init_action_loader);	  
			register_tag_loader(60, define_video_loader);
			register_tag_loader(61, video_loader);
			register_tag_loader(62, define_font_info_loader);

//			register_tag_loader(63, define_font_info_loader);
			register_tag_loader(64, define_enable_debugger_loader);
			register_tag_loader(66, define_tabindex_loader);
			register_tag_loader(69, define_file_attribute_loader);	// Flash 8
			register_tag_loader(70, place_object_2_loader);
			register_tag_loader(73, define_font_alignzones);	// DefineFontAlignZones - Flash 8
			register_tag_loader(74, define_csm_textsetting_loader); // CSMTextSetting - Flash 8
			register_tag_loader(75, define_font_loader);	// DefineFont3 - Flash 8
			register_tag_loader(76, symbol_class_loader);	// SymbolClass - Flash 9
			register_tag_loader(77, define_metadata_loader);	// MetaData - Flash 8
			register_tag_loader(82, define_abc_loader);	// doABC - Flash 9
			register_tag_loader(83, define_shape_loader);		// DefineShape4 - Flash 8
			register_tag_loader(84, define_shape_morph_loader); //DefineMorphShape2 - Flash 9
			register_tag_loader(86, define_scene_loader);		// DefineSceneAndFrameLabelData - Flash 9
			register_tag_loader(88, define_font_name);		// DefineFontName - Flash 9
		}
	}

	// FIXME
	/*
	void	get_movie_info(
		const char* filename,
		int* version,
		int* width,
		int* height,
		float* frames_per_second,
		int* frame_count,
		int* tag_count
		)
		// Attempt to read the header of the given .swf movie file.
		// Put extracted info in the given vars.
		// Sets *version to 0 if info can't be extracted.
	{
		if (s_opener_function == NULL)
		{
			log_error("error: get_movie_info(): no file opener function registered\n");
			if (version) *version = 0;
			return;
		}

		tu_file*	in = s_opener_function(filename);
		if (in == NULL || in->get_error() != TU_FILE_NO_ERROR)
		{
			log_error("error: get_movie_info(): can't open '%s'\n", filename);
			if (version) *version = 0;
			delete in;
			return;
		}

		Uint32	file_start_pos = in->get_position();
		Uint32	header = in->read_le32();
		Uint32	file_length = in->read_le32();
		Uint32	file_end_pos = file_start_pos + file_length;

		int	local_version = (header >> 24) & 255;
		if ((header & 0x0FFFFFF) != 0x00535746
			&& (header & 0x0FFFFFF) != 0x00535743)
		{
			// ERROR
			log_error("error: get_movie_info(): file '%s' does not start with a SWF header!\n", filename);
			if (version) *version = 0;
			delete in;
			return;
		}
		bool	compressed = (header & 255) == 'C';

		tu_file*	original_in = NULL;
		if (compressed)
		{
#if TU_CONFIG_LINK_TO_ZLIB == 0
			log_error("get_movie_info(): can't read zipped SWF data; TU_CONFIG_LINK_TO_ZLIB is 0!\n");
			return;
#endif
			original_in = in;

			// Uncompress the input as we read it.
			in = zlib_adapter::make_inflater(original_in);

			// Subtract the size of the 8-byte header, since
			// it's not included in the compressed
			// stream length.
			file_end_pos = file_length - 8;
		}

		stream	str(in);

		rect	frame_size;
		frame_size.read(&str);

		float	local_frame_rate = str.read_u16() / 256.0f;
		int	local_frame_count = str.read_u16();

		if (version) *version = local_version;
		if (width) *width = int(frame_size.width() / 20.0f + 0.5f);
		if (height) *height = int(frame_size.height() / 20.0f + 0.5f);
		if (frames_per_second) *frames_per_second = local_frame_rate;
		if (frame_count) *frame_count = local_frame_count;

		if (tag_count)
		{
			// Count tags.
			int local_tag_count = 0;
			while ((Uint32) str.get_position() < file_end_pos)
			{
				int tag_type = str.open_tag();
				UNUSED(tag_type);
				str.close_tag();
				local_tag_count++;
			}
			*tag_count = local_tag_count;
		}

		delete in;
		delete original_in;
	}
*/
	static bool	s_no_recurse_while_loading = false;	// @@ TODO get rid of this; make it the normal mode.

	/*
	movie_definition*	create_movie_no_recurse(
		tu_file* in,
		create_bitmaps_flag cbf,
		create_font_shapes_flag cfs)
	{
		ensure_loaders_registered();

		// @@ TODO make no_recurse the standard way to load.
		// In create_movie(), use the visit_ API to keep
		// visiting dependent movies, until everything is
		// loaded.  That way we only have one code path and
		// the resource_proxy stuff gets tested.
		s_no_recurse_while_loading = true;

		movie_def_impl*	m = new movie_def_impl(cbf, cfs);
		m->read(in);

		s_no_recurse_while_loading = false;

		m->add_ref();
		return m;
	}
*/

	//
	// global gameswf management
	//
	void clears_tag_loaders();
	void clear_shared_libs();

	void	precompute_cached_data(movie_definition* movie_def)
	// Fill in cached data in movie_def.
	// @@@@ NEEDS TESTING -- MIGHT BE BROKEN!!!
	{
		assert(movie_def != NULL);

		// Temporarily install null render and sound handlers,
		// so we don't get output during preprocessing.
		//
		// Use automatic struct var to make sure we restore
		// when exiting the function.
		struct save_stuff
		{
			render_handler*	m_original_rh;
			sound_handler*	m_original_sh;

			save_stuff()
			{
				// Save.
				m_original_rh = get_render_handler();
				m_original_sh = get_sound_handler();
				set_render_handler(NULL);
				set_sound_handler(NULL);
			}

			~save_stuff()
			{
				// Restore.
				set_render_handler(m_original_rh);
				set_sound_handler(m_original_sh);
			}
		} save_stuff_instance;

		// Need an instance.
		gc_ptr<gameswf::root>	m = movie_def->create_instance();
		if (m == NULL)
		{
			log_error("error: precompute_cached_data can't create instance of movie\n");
			return;
		}

		// Run through the movie's frames.
		//
		// @@ there might be cleaner ways to do this; e.g. do
		// execute_frame_tags(i, true) on movie and all child
		// sprites.
		int	kick_count = 0;
		for (;;)
		{
			// @@ do we also have to run through all sprite frames
			// as well?
			//
			// @@ also, ActionScript can rescale things
			// dynamically -- we can't really do much about that I
			// guess?
			//
			// @@ Maybe we should allow the user to specify some
			// safety margin on scaled shapes.

			int	last_frame = m->get_current_frame();
			m->advance(0.010f);
			m->display(NULL);

			if (m->get_current_frame() == movie_def->get_frame_count() - 1)
			{
				// Done.
				break;
			}

			if (m->get_play_state() == gameswf::character::STOP)
			{
				// Kick the movie.
				//printf("kicking movie, kick ct = %d\n", kick_count);
				m->goto_frame(last_frame + 1);
				m->set_play_state(gameswf::character::PLAY);
				kick_count++;

				if (kick_count > 10)
				{
					//printf("movie is stalled; giving up on playing it through.\n");
					break;
				}
			}
			else if (m->get_current_frame() < last_frame)
			{
				// Hm, apparently we looped back.  Skip ahead...
				log_error("loop back; jumping to frame %d\n", last_frame);
				m->goto_frame(last_frame + 1);
			}
			else
			{
				kick_count = 0;
			}
		}
	}


	//
	// Some tag implementations
	//


	void	null_loader(stream* in, int tag_type, movie_definition_sub* m)
		// Silently ignore the contents of this tag.
	{
	}

	void	frame_label_loader(stream* in, int tag_type, movie_definition_sub* m)
	// Label the current frame of m with the name from the stream.
	{
		tu_string n;
		in->read_string(&n);
		m->add_frame_name(n.c_str());
	}

	struct set_background_color : public execute_tag
	{
		rgba	m_color;

		void	execute(character* m)
		{
			float	current_alpha = m->get_background_alpha();
			m_color.m_a = frnd(current_alpha * 255.0f);
			m->set_background_color(m_color);
		}

		void	execute_state(character* m)
		{
			execute(m);
		}

		void	read(stream* in)
		{
			m_color.read_rgb(in);

			IF_VERBOSE_PARSE(log_msg("  set_background_color: (%d %d %d)\n",
				m_color.m_r, m_color.m_g, m_color.m_b));
		}
	};


	void	set_background_color_loader(stream* in, int tag_type, movie_definition_sub* m)
	{
		assert(tag_type == 9);
		assert(m);

		set_background_color*	t = new set_background_color;
		t->read(in);

		m->add_execute_tag(t);
	}

	void	jpeg_tables_loader(stream* in, int tag_type, movie_definition_sub* m)
	// Load JPEG compression tables that can be used to load
	// images further along in the stream.
	{
		assert(tag_type == 8);

    int curr_pos = in->get_position();
    int end_pos = in->get_tag_end_position();
    int header_size = end_pos - curr_pos;
    assert(header_size >= 0);

#if TU_CONFIG_LINK_TO_JPEGLIB

		jpeg::input*	j_in = NULL;
		if (header_size > 0)
		{
			j_in = jpeg::input::create_swf_jpeg2_header_only(in->get_underlying_stream());
		}
		m->set_jpeg_loader(j_in);

#endif // TU_CONFIG_LINK_TO_JPEGLIB
	}


	void	define_bits_jpeg_loader(stream* in, int tag_type, movie_definition_sub* m)
	// A JPEG image without included tables; those should be in an
	// existing jpeg::input object stored in the movie.
	{
		assert(tag_type == 6);

		Uint16	character_id = in->read_u16();

		//
		// Read the image data.
		//
		bitmap_info*	bi = NULL;

		if (m->get_create_bitmaps() == DO_LOAD_BITMAPS)
		{

#if TU_CONFIG_LINK_TO_JPEGLIB

			jpeg::input*	j_in = m->get_jpeg_loader();
			image::rgb*	im = NULL;
			if (j_in == NULL)
			{
				// there are no jpeg tables
				im = image::read_jpeg(in->get_underlying_stream());
			}
			else
			{
				j_in->discard_partial_buffer();
				im = image::read_swf_jpeg2_with_tables(j_in);
			}
			bi = render::create_bitmap_info_rgb(im);
			delete im;

#else
			log_error("gameswf is not linked to jpeglib -- can't load jpeg image data!\n");
			bi = render::create_bitmap_info_empty();
#endif
		}
		else
		{
			bi = render::create_bitmap_info_empty();
		}

		bitmap_character*	ch = new bitmap_character(m, bi);

		m->add_bitmap_character(character_id, ch);
	}

	void	symbol_class_loader(stream* in, int tag_type, movie_definition_sub* m)
	// The SymbolClass tag creates associations between symbols in the SWF file
	// and ActionScript 3.0 classes.
	{
		assert(tag_type == 76);

		int num_symbols = in->read_u16();
		for (int i = 0; i < num_symbols; i++)
		{
			Uint16	character_id = in->read_u16();
			tu_string class_name;
			in->read_string(&class_name);
			m->add_symbol_class(character_id, class_name);

			IF_VERBOSE_PARSE(log_msg("  symbol_class_loader: charid = %d class = '%s'\n",
				character_id, class_name.c_str()));
		}
	}

	void	define_bits_jpeg2_loader(stream* in, int tag_type, movie_definition_sub* m)
	{
		assert(tag_type == 21);

		Uint16	character_id = in->read_u16();

		IF_VERBOSE_PARSE(log_msg("  define_bits_jpeg2_loader: charid = %d pos = 0x%x\n", character_id, in->get_position()));

		//
		// Read the image data.
		//

		bitmap_info*	bi = NULL;

		if (m->get_create_bitmaps() == DO_LOAD_BITMAPS)
		{
#if TU_CONFIG_LINK_TO_JPEGLIB
			image::rgb* im = image::read_swf_jpeg2(in->get_underlying_stream());
			bi = render::create_bitmap_info_rgb(im);
			delete im;
#else
			log_error("gameswf is not linked to jpeglib -- can't load jpeg image data!\n");
			bi = render::create_bitmap_info_empty();
#endif
		}
		else
		{
			bi = render::create_bitmap_info_empty();
		}

		bitmap_character*	ch = new bitmap_character(m, bi);

		m->add_bitmap_character(character_id, ch);
	}


#if TU_CONFIG_LINK_TO_ZLIB
	void	inflate_wrapper(tu_file* in, void* buffer, int buffer_bytes)
		// Wrapper function -- uses Zlib to uncompress in_bytes worth
		// of data from the input file into buffer_bytes worth of data
		// into *buffer.
	{
		assert(in);
		assert(buffer);
		assert(buffer_bytes > 0);

		int err;
		z_stream d_stream; /* decompression stream */

		d_stream.zalloc = (alloc_func)0;
		d_stream.zfree = (free_func)0;
		d_stream.opaque = (voidpf)0;

		d_stream.next_in  = 0;
		d_stream.avail_in = 0;

		d_stream.next_out = (Byte*) buffer;
		d_stream.avail_out = (uInt) buffer_bytes;

		err = inflateInit(&d_stream);
		if (err != Z_OK) {
			log_error("error: inflate_wrapper() inflateInit() returned %d\n", err);
			return;
		}

		Uint8	buf[1];

		for (;;) {
			// Fill a one-byte (!) buffer.
			buf[0] = in->read_byte();
			d_stream.next_in = &buf[0];
			d_stream.avail_in = 1;

			err = inflate(&d_stream, Z_SYNC_FLUSH);
			if (err == Z_STREAM_END) break;
			if (err != Z_OK)
			{
				log_error("error: inflate_wrapper() inflate() returned %d\n", err);
			}
		}

		err = inflateEnd(&d_stream);
		if (err != Z_OK)
		{
			log_error("error: inflate_wrapper() inflateEnd() return %d\n", err);
		}
	}
#endif // TU_CONFIG_LINK_TO_ZLIB


	void	define_bits_jpeg3_loader(stream* in, int tag_type, movie_definition_sub* m)
		// loads a define_bits_jpeg3 tag. This is a jpeg file with an alpha
		// channel using zlib compression.
	{
		assert(tag_type == 35);

		Uint16	character_id = in->read_u16();

		IF_VERBOSE_PARSE(log_msg("  define_bits_jpeg3_loader: charid = %d pos = 0x%x\n", character_id, in->get_position()));

		Uint32	jpeg_size = in->read_u32();
		Uint32	alpha_position = in->get_position() + jpeg_size;

		bitmap_info*	bi = NULL;

		if (m->get_create_bitmaps() == DO_LOAD_BITMAPS)
		{
#if TU_CONFIG_LINK_TO_JPEGLIB == 0 || TU_CONFIG_LINK_TO_ZLIB == 0
			log_error("gameswf is not linked to jpeglib/zlib -- can't load jpeg/zipped image data!\n");
			bi = render::create_bitmap_info_empty();
#else
			//
			// Read the image data.
			//

			// Read rgb data.
			image::rgba*	im = image::read_swf_jpeg3(in->get_underlying_stream());

			// Read alpha channel.
			in->set_position(alpha_position);

			int	buffer_bytes = im->m_width * im->m_height;
			Uint8*	buffer = new Uint8[buffer_bytes];

			inflate_wrapper(in->get_underlying_stream(), buffer, buffer_bytes);

			for (int i = 0; i < buffer_bytes; i++)
			{
				im->m_data[4*i+3] = buffer[i];
			}

			delete [] buffer;

			bi = render::create_bitmap_info_rgba(im);

			delete im;
#endif

		}
		else
		{
			bi = render::create_bitmap_info_empty();
		}

		// Create bitmap character.
		bitmap_character*	ch = new bitmap_character(m, bi);

		m->add_bitmap_character(character_id, ch);
	}


	void	define_bits_lossless_2_loader(stream* in, int tag_type, movie_definition_sub* m)
	{
		assert(tag_type == 20 || tag_type == 36);

		Uint16	character_id = in->read_u16();
		Uint8	bitmap_format = in->read_u8();	// 3 == 8 bit, 4 == 16 bit, 5 == 32 bit
		Uint16	width = in->read_u16();
		Uint16	height = in->read_u16();

		IF_VERBOSE_PARSE(log_msg("  defbitslossless2: tag_type = %d, id = %d, fmt = %d, w = %d, h = %d\n",
			tag_type,
			character_id,
			bitmap_format,
			width,
			height));

		bitmap_info*	bi = NULL;
		if (m->get_create_bitmaps() == DO_LOAD_BITMAPS)
		{
#if TU_CONFIG_LINK_TO_ZLIB == 0
			log_error("gameswf is not linked to zlib -- can't load zipped image data!\n");
			return;
#else
			if (tag_type == 20)
			{
				// RGB image data.
				image::rgb*	image = image::create_rgb(width, height);

				if (bitmap_format == 3)
				{
					// 8-bit data, preceded by a palette.

					const int	bytes_per_pixel = 1;
					int	color_table_size = in->read_u8();
					color_table_size += 1;	// !! SWF stores one less than the actual size

					int	pitch = (width * bytes_per_pixel + 3) & ~3;

					int	buffer_bytes = color_table_size * 3 + pitch * height;
					Uint8*	buffer = new Uint8[buffer_bytes];

					inflate_wrapper(in->get_underlying_stream(), buffer, buffer_bytes);
					assert(in->get_position() <= in->get_tag_end_position());

					Uint8*	color_table = buffer;

					for (int j = 0; j < height; j++)
					{
						Uint8*	image_in_row = buffer + color_table_size * 3 + j * pitch;
						Uint8*	image_out_row = image::scanline(image, j);
						for (int i = 0; i < width; i++)
						{
							Uint8	pixel = image_in_row[i * bytes_per_pixel];
							image_out_row[i * 3 + 0] = color_table[pixel * 3 + 0];
							image_out_row[i * 3 + 1] = color_table[pixel * 3 + 1];
							image_out_row[i * 3 + 2] = color_table[pixel * 3 + 2];
						}
					}

					delete [] buffer;
				}
				else if (bitmap_format == 4)
				{
					// 16 bits / pixel
					const int	bytes_per_pixel = 2;
					int	pitch = (width * bytes_per_pixel + 3) & ~3;

					int	buffer_bytes = pitch * height;
					Uint8*	buffer = new Uint8[buffer_bytes];

					inflate_wrapper(in->get_underlying_stream(), buffer, buffer_bytes);
					assert(in->get_position() <= in->get_tag_end_position());

					for (int j = 0; j < height; j++)
					{
						Uint8*	image_in_row = buffer + j * pitch;
						Uint8*	image_out_row = image::scanline(image, j);
						for (int i = 0; i < width; i++)
						{
							Uint16	pixel = image_in_row[i * 2] | (image_in_row[i * 2 + 1] << 8);

							// @@ How is the data packed???	 I'm just guessing here that it's 565!
							image_out_row[i * 3 + 0] = (pixel >> 8) & 0xF8;	// red
							image_out_row[i * 3 + 1] = (pixel >> 3) & 0xFC;	// green
							image_out_row[i * 3 + 2] = (pixel << 3) & 0xF8;	// blue
						}
					}

					delete [] buffer;
				}
				else if (bitmap_format == 5)
				{
					// 32 bits / pixel, input is ARGB format (???)
					const int	bytes_per_pixel = 4;
					int	pitch = width * bytes_per_pixel;

					int	buffer_bytes = pitch * height;
					Uint8*	buffer = new Uint8[buffer_bytes];

					inflate_wrapper(in->get_underlying_stream(), buffer, buffer_bytes);
					assert(in->get_position() <= in->get_tag_end_position());

					// Need to re-arrange ARGB into RGB.
					for (int j = 0; j < height; j++)
					{
						Uint8*	image_in_row = buffer + j * pitch;
						Uint8*	image_out_row = image::scanline(image, j);
						for (int i = 0; i < width; i++)
						{
							Uint8	a = image_in_row[i * 4 + 0];
							Uint8	r = image_in_row[i * 4 + 1];
							Uint8	g = image_in_row[i * 4 + 2];
							Uint8	b = image_in_row[i * 4 + 3];
							image_out_row[i * 3 + 0] = r;
							image_out_row[i * 3 + 1] = g;
							image_out_row[i * 3 + 2] = b;
							a = a;	// Inhibit warning.
						}
					}

					delete [] buffer;
				}

				//				bitmap_character*	ch = new bitmap_character(image);
				bi = render::create_bitmap_info_rgb(image);
				delete image;

				//				// add image to movie, under character id.
				//				m->add_bitmap_character(character_id, ch);
			}
			else
			{
				// RGBA image data.
				assert(tag_type == 36);

				image::rgba*	image = image::create_rgba(width, height);

				if (bitmap_format == 3)
				{
					// 8-bit data, preceded by a palette.

					const int	bytes_per_pixel = 1;
					int	color_table_size = in->read_u8();
					color_table_size += 1;	// !! SWF stores one less than the actual size

					int	pitch = (width * bytes_per_pixel + 3) & ~3;

					int	buffer_bytes = color_table_size * 4 + pitch * height;
					Uint8*	buffer = new Uint8[buffer_bytes];

					inflate_wrapper(in->get_underlying_stream(), buffer, buffer_bytes);
					assert(in->get_position() <= in->get_tag_end_position());

					Uint8*	color_table = buffer;

					for (int j = 0; j < height; j++)
					{
						Uint8*	image_in_row = buffer + color_table_size * 4 + j * pitch;
						Uint8*	image_out_row = image::scanline(image, j);
						for (int i = 0; i < width; i++)
						{
							Uint8	pixel = image_in_row[i * bytes_per_pixel];
							image_out_row[i * 4 + 0] = color_table[pixel * 4 + 0];
							image_out_row[i * 4 + 1] = color_table[pixel * 4 + 1];
							image_out_row[i * 4 + 2] = color_table[pixel * 4 + 2];
							image_out_row[i * 4 + 3] = color_table[pixel * 4 + 3];
						}
					}

					delete [] buffer;
				}
				else if (bitmap_format == 4)
				{
					// 16 bits / pixel
					const int	bytes_per_pixel = 2;
					int	pitch = (width * bytes_per_pixel + 3) & ~3;

					int	buffer_bytes = pitch * height;
					Uint8*	buffer = new Uint8[buffer_bytes];

					inflate_wrapper(in->get_underlying_stream(), buffer, buffer_bytes);
					assert(in->get_position() <= in->get_tag_end_position());

					for (int j = 0; j < height; j++)
					{
						Uint8*	image_in_row = buffer + j * pitch;
						Uint8*	image_out_row = image::scanline(image, j);
						for (int i = 0; i < width; i++)
						{
							Uint16	pixel = image_in_row[i * 2] | (image_in_row[i * 2 + 1] << 8);

							// @@ How is the data packed???	 I'm just guessing here that it's 565!
							image_out_row[i * 4 + 0] = 255;			// alpha
							image_out_row[i * 4 + 1] = (pixel >> 8) & 0xF8;	// red
							image_out_row[i * 4 + 2] = (pixel >> 3) & 0xFC;	// green
							image_out_row[i * 4 + 3] = (pixel << 3) & 0xF8;	// blue
						}
					}

					delete [] buffer;
				}
				else if (bitmap_format == 5)
				{
					// 32 bits / pixel, input is ARGB format

					inflate_wrapper(in->get_underlying_stream(), image->m_data, width * height * 4);
					assert(in->get_position() <= in->get_tag_end_position());

					// Need to re-arrange ARGB into RGBA.
					for (int j = 0; j < height; j++)
					{
						Uint8*	image_row = image::scanline(image, j);
						for (int i = 0; i < width;  i++)
						{
							auto index = i * 4;
							Uint8	a = image_row[index + 0];
							Uint8	r = image_row[index + 1];
							Uint8	g = image_row[index + 2];
							Uint8	b = image_row[index + 3];
							auto t = a / 255.0f;
							image_row[index + 0] = r/ t;
							image_row[index + 1] = g/ t;
							image_row[index + 2] = b/ t;
							image_row[index + 3] = a;
						}
					}
				}

				bi = render::create_bitmap_info_rgba(image);
				//				bitmap_character*	ch = new bitmap_character(image);
				delete image;

				//				// add image to movie, under character id.
				//				m->add_bitmap_character(character_id, ch);
			}
#endif // TU_CONFIG_LINK_TO_ZLIB
		}
		else
		{
			bi = render::create_bitmap_info_empty();
		}

		bitmap_character*	ch = new bitmap_character(m, bi);

		// add image to movie, under character id.
		m->add_bitmap_character(character_id, ch);
	}

	void	define_abc_loader(stream* in, int tag_type, movie_definition_sub* m)
	{
		assert(tag_type == 82);

		// if flags == 1 then parse only
		Uint32 flags = in->read_u32();
		UNUSED(flags);

		tu_string name;
		in->read_string(&name);

		IF_VERBOSE_PARSE(log_msg("\n doABC tag loader, abc_name = '%s'\n", name.c_str()));

		abc_def* abc = new abc_def(m->get_player());
		abc->read(in, m);
		m->add_abc(name, abc);
	}

	void	define_font_name(stream* in, int tag_type, movie_definition_sub* m)
	{
		assert(tag_type == 88);
		IF_VERBOSE_PARSE(log_msg("\n  define_font_name\n"));

		int font_id = in->read_u16();

		tu_string font_name;
		in->read_string(&font_name);

		tu_string font_copyright;	// unused
		in->read_string(&font_copyright);

		font*	f = m->get_font(font_id);
		if (f)
		{
			f->set_name(font_name);

			// font name contains Bold/Italic keyword
			// may be not correct !
			f->set_bold(false);
			f->set_italic(false);
		}

	}

	void	define_scene_loader(stream* in, int tag_type, movie_definition_sub* m)
	{
		assert(tag_type == 86);

		IF_VERBOSE_PARSE(log_msg("\n  define_scene_loader\n"));

		int scene_count = in->read_vs32();
		for (int i = 0; i < scene_count; i++)
		{
			int offset = in->read_vs32();
			tu_string scene_name;
			in->read_string(&scene_name);
			m->add_scene(offset, scene_name);

			IF_VERBOSE_PARSE(log_msg("  scene #%d, name='%s'\n", offset, scene_name.c_str()));
		}
		
		int frame_label_count = in->read_vs32();
		for (int i = 0; i < frame_label_count; i++)
		{
			int frame = in->read_vs32();
			tu_string frame_label;
			in->read_string(&frame_label);
			m->add_frame_label(frame, frame_label);

			IF_VERBOSE_PARSE(log_msg("  frame #%d, label='%s'\n", frame, frame_label.c_str()));
		}

	}

	void	define_shape_loader(stream* in, int tag_type, movie_definition_sub* m)
	{
		assert(tag_type == 2 || tag_type == 22 || tag_type == 32 || tag_type == 83);

		Uint16	character_id = in->read_u16();
		IF_VERBOSE_PARSE(log_msg("  shape_loader: id = %d\n", character_id));

		shape_character_def*	ch = new shape_character_def(m->get_player());
		ch->read(in, tag_type, true, m);

		IF_VERBOSE_PARSE(log_msg("  bound rect:"); ch->get_bound_local().print());

		m->add_character(character_id, ch);
	}

	void define_shape_morph_loader(stream* in, int tag_type, movie_definition_sub* m)
	{
		assert(tag_type == 46 || tag_type == 84);
		Uint16 character_id = in->read_u16();
		IF_VERBOSE_PARSE(log_msg("  shape_morph_loader: id = %d\n", character_id));
		morph2_character_def* morph = new morph2_character_def(m->get_player());
		morph->read(in, tag_type, true, m);
		m->add_character(character_id, morph);
	}

	//
	// font loaders
	//


	void	define_font_loader(stream* in, int tag_type, movie_definition_sub* m)
		// Load a DefineFont or DefineFont2 tag.
	{
		assert(tag_type == 10 || tag_type == 48 || tag_type == 75);

		Uint16	font_id = in->read_u16();

		font*	f = new font(m->get_player());
		f->read(in, tag_type, m);

		m->add_font(font_id, f);

		// Automatically keeping fonts in fontlib is
		// problematic.	 The app should be responsible for
		// optionally adding fonts to fontlib.
		// //fontlib::add_font(f);
	}


	void	define_font_info_loader(stream* in, int tag_type, movie_definition_sub* m)
		// Load a DefineFontInfo tag.  This adds information to an
		// existing font.
	{
		assert(tag_type == 13 || tag_type == 62);

		Uint16	font_id = in->read_u16();

		font*	f = m->get_font(font_id);
		if (f)
		{
			f->read_font_info(in, tag_type);
		}
		else
		{
			log_error("define_font_info_loader: can't find font w/ id %d\n", font_id);
		}
	}

	void	define_enable_debugger_loader(stream* in, int tag_type, movie_definition_sub* m)
	// this tag defines characteristics of the SWF file (Flash 8)
	{
		assert(tag_type == 58 || tag_type == 64);

		if (tag_type == 64)	// define_enable_debugger_loader
		{
			in->read_u16();	// reserved
		}

		// now attr is't used
		tu_string md5_password;
		in->read_string(&md5_password);
	}

	void	define_tabindex_loader(stream* in, int tag_type, movie_definition_sub* m)
	{
		assert(tag_type == 66);

		// for now this tag is not implemented
		Uint16 depth = in->read_u16();
		Uint16 tabindex = in->read_u16();
		UNUSED(depth);
		UNUSED(tabindex);
	}

	void	define_file_attribute_loader(stream* in, int tag_type, movie_definition_sub* m)
	// this tag defines characteristics of the SWF file (Flash 8)
	{
		assert(tag_type == 69);

		Uint8 attr = in->read_u8();
		in->read_uint(24);	// reserved

		m->m_has_metadata =  attr & 0x10 ? true : false;
		m->m_is_avm2 =  attr & 0x08 ? true : false;
		m->m_use_network =  attr & 0x010 ? true : false;
	}

	void	define_font_alignzones(stream* in, int tag_type, movie_definition_sub* m)
	// this tag defines characteristics of the SWF file (Flash 8)
	{
		assert(tag_type == 73);

		Uint16	font_id = in->read_u16();
		font*	f = m->get_font(font_id);
		if (f)
		{
			f->read_font_alignzones(in, tag_type);
		}
		else
		{
			log_error("define_font_alignzones: can't find font w/ id %d\n", font_id);
		}
	}

	void	define_csm_textsetting_loader(stream* in, int tag_type, movie_definition_sub* m)
	// this tag defines quality & options of text filed
	{
		assert(tag_type == 74);

		Uint16	text_id = in->read_u16();
		character_def*	ch = m->get_character_def(text_id);
		if (ch)
		{
			ch->csm_textsetting(in, tag_type);
		}
		else
		{
			log_error("define_font_alignzones: can't find font w/ id %d\n", text_id);
		}
	}

	//typedef std::function<void(float y0, float  y1, float  lx0, float  lx1, float  rx0, float  rx1)> DrawTrapezoidFunc;
	typedef   void(*DrawTrapezoidFunc)(float y0, float  y1, float  lx0, float  lx1, float  rx0, float  rx1) ;


	struct draw_into_bitmap : tesselate::trapezoid_accepter
		// A trapezoid accepter that does B&W rendering into our
		// software buffer.
	{
		// Overrides from trapezoid_accepter
		virtual void	accept_trapezoid(int style, const tesselate::trapezoid& tr)
		{
			// Transform the coords.
			//float	x_scale = s_render_matrix.m_[0][0];
			//float	y_scale = s_render_matrix.m_[1][1];
			//float	x_offset = s_render_matrix.m_[0][2];
			//float	y_offset = s_render_matrix.m_[1][2];
			float x_scale =1 / 20.0f;
			float y_scale = 1 / 20.0f;

			float	y0 = tr.m_y0 * y_scale ;
			float	y1 = tr.m_y1 * y_scale ;
			float	lx0 = tr.m_lx0 * x_scale ;
			float	lx1 = tr.m_lx1 * x_scale ;
			float	rx0 = tr.m_rx0 * x_scale ;
			float	rx1 = tr.m_rx1 * x_scale ;

			// Draw into the software buffer.
			//software_trapezoid(y0, y1, lx0, lx1, rx0, rx1);
			if (m_bitmapInfo)
			{

				drawOnBitmap(y0, y1, lx0, lx1, rx0, rx1);

				//m_drawTrapezoidFunc(y0, y1, lx0, lx1, rx0, rx1);
			}

		}

		void drawOnBitmap(float	y0, float	y1, float	lx0, float	lx1, float	rx0, float	rx1)
		{
			int w = m_bitmapInfo->get_width();
			int h = m_bitmapInfo->get_height();
			auto pData = m_bitmapInfo->get_data();
			float  ox = w * m_sAnchorPoint.m_x;
			float  oy = h * (1.0f - m_sAnchorPoint.m_y);

			y0 = y0 + oy;
			y1 = y1 + oy;
			lx0 = lx0 + ox;
			lx1 = lx1 + ox;
			rx0 = rx0 + ox;
			rx1 = rx1 + ox;



			int	iy0 = (int)ceilf(y0);
			int	iy1 = (int)ceilf(y1);
			float	dy = y1 - y0;

			for (int y = iy0; y < iy1; y++)
			{
				if (y < 0)
				{
					continue;
				}
				if (y >= h)
				{
					return;
				}

				float	f = (y - y0) / dy;
				int	xl = (int)ceilf(flerp(lx0, lx1, f));
				int	xr = (int)ceilf(flerp(rx0, rx1, f));

				xl = iclamp(xl, 0, w - 1);
				xr = iclamp(xr, 0, w - 1);

				if (xr > xl)
				{
					//memset(pData + (y * w + xl)*4, 255, (xr - xl)*4);
					for (int i = xl; i < xr; i++)
					{
						auto p = &pData[(y * w + i) * 4];
						doBlend(p, false);

					}
					{
						int i = xl - 1;
						if (i > 0)
						{
							auto p = &pData[(y * w + i) * 4];
							//doBlend(p, true);
						}

					}

					{
						int i = xr;
						if (i < w)
						{
							auto p = &pData[(y * w + i) * 4];
							//doBlend(p,true);

						}

					}


				}


			}
		}

		virtual void	accept_line_strip(int style, const point coords[], int coord_count)
		{
			//assert(0);	// Shape glyphs should not contain lines.
		}

		virtual void end_shape() {}

		void setDrawFunc(DrawTrapezoidFunc func) {
		//	m_bitmapInfo = info;
			m_drawTrapezoidFunc = func;
		}

		void doBlend(unsigned char* p,bool isBoundary) {

			if (m_blendMode == 2)
			{
				auto sa = p[3] / 255.0f;
				if (isBoundary)
				{
					p[0] = m_sColor.m_r;
					p[1] = m_sColor.m_g;
					p[2] = m_sColor.m_b;
					p[3] = m_sColor.m_a / 2;
				}
				else
				{
					p[0] = m_sColor.m_r;
					p[1] = m_sColor.m_g;
					p[2] = m_sColor.m_b;
					p[3] = m_sColor.m_a;
				}
			}
			else
			{
				if (isBoundary)
				{
					p[0] = m_sColor.m_r;
					p[1] = m_sColor.m_g;
					p[2] = m_sColor.m_b;
					p[3] = m_sColor.m_a / 2;
				}
				else
				{
					p[0] = m_sColor.m_r;
					p[1] = m_sColor.m_g;
					p[2] = m_sColor.m_b;
					p[3] = m_sColor.m_a;
				}
			}
		}

		void setBitmapInfo(gameswf::bitmap_info* info,point & sAnchorPoint,  rgba& color) {
			m_bitmapInfo = info;
			m_sAnchorPoint = sAnchorPoint;
			m_sColor = color;
		}
		;
		void setBlendMode(int blendMode)
		{
			m_blendMode = blendMode;
			if (m_blendMode == 2)
			{
				auto sa = m_sColor.m_a / 255.0f;
				m_sColorEx.m_r = m_sColor.m_r*sa;
				m_sColorEx.m_g = m_sColor.m_g*sa;
				m_sColorEx.m_b = m_sColor.m_b*sa;
				m_sColorEx.m_a = m_sColor.m_a*sa;
			}
		}
		;
		DrawTrapezoidFunc m_drawTrapezoidFunc = nullptr;
		gameswf::bitmap_info* m_bitmapInfo = nullptr;
		point m_sAnchorPoint;
		rgba m_sColor;
		rgba m_sColorEx;
		int  m_blendMode = 1;
	};


	
	struct MyBitmap {
		
		void setData(GLubyte*data, unsigned int w, unsigned int  h, unsigned int channel) {
			m_data = data;
			m_w = w;
			m_h = h;
			m_channel = channel;
		}
		GLubyte* m_data;
		unsigned int m_w;
		unsigned int m_h;
		unsigned int m_channel;
	};
	
	//
	// swf_event
	//
	// For embedding event handlers in place_object_2

	//
	// place_object_2
	//

	struct place_object_2 : public execute_tag
	{
		int	m_tag_type;
		tu_string	m_character_name;
		float	m_ratio;
		cxform	m_color_transform;
		matrix	m_matrix;
		bool	m_has_matrix;
		bool	m_has_cxform;
		int 	m_depth;
		Uint16	m_character_id;
		Uint16	m_clip_depth;
		Uint8   m_blend_mode;
		enum place_type {
			PLACE,
			MOVE,
			REPLACE,
		} m_place_type;
		array<swf_event*>	m_event_handlers;

		place_object_2() :
			m_tag_type(0), m_ratio(0), m_has_matrix(false), m_has_cxform(false), m_depth(0),
			m_character_id(0), m_clip_depth(0), m_blend_mode(0), m_place_type(PLACE)
		{
		}

		~place_object_2()
		{
			for (int i = 0, n = m_event_handlers.size(); i < n; i++)
			{
				delete m_event_handlers[i];
			}
			m_event_handlers.resize(0);
		}

		void	read(player* player, stream* in, int tag_type, int movie_version)
		{
			assert(tag_type == 4 || tag_type == 26 || tag_type == 70);

			m_tag_type = tag_type;

			if (tag_type == 4)
			{
				// Original place_object tag; very simple.
				m_character_id = in->read_u16();
				m_depth = in->read_u16();
				m_matrix.read(in);

				IF_VERBOSE_PARSE(
					log_msg("  char_id = %d\n"
					"  depth = %d\n"
					"  mat = \n",
					m_character_id,
					m_depth);
				m_matrix.print());

				if (in->get_position() < in->get_tag_end_position())
				{
					m_color_transform.read_rgb(in);
					IF_VERBOSE_PARSE(log_msg("  cxform:\n"); m_color_transform.print());
				}
			}
			else
			if (tag_type == 26 || tag_type == 70)
			{
				in->align();

				bool	has_actions = in->read_uint(1) ? true : false;
				bool	has_clip_bracket = in->read_uint(1) ? true : false;
				bool	has_name = in->read_uint(1) ? true : false;
				bool	has_ratio = in->read_uint(1) ? true : false;
				bool	has_cxform = in->read_uint(1) ? true : false;
				bool	has_matrix = in->read_uint(1) ? true : false;
				bool	has_char = in->read_uint(1) ? true : false;
				bool	flag_move = in->read_uint(1) ? true : false;

				bool	has_image = false;
				bool	has_class_name = false;
				bool	has_cache_asbitmap = false;
				bool	has_blend_mode = false;
				bool	has_filter_list = false;

				if (tag_type == 70)
				{
					in->read_uint(3); // unused

					has_image = in->read_uint(1) ? true : false;
					has_class_name = in->read_uint(1) ? true : false;
					has_cache_asbitmap = in->read_uint(1) ? true : false;
					has_blend_mode = in->read_uint(1) ? true : false;
					has_filter_list = in->read_uint(1) ? true : false;
				}

				m_depth = in->read_u16();
				IF_VERBOSE_PARSE(log_msg("  depth = %d\n", m_depth));

				if (has_char)
				{
					m_character_id = in->read_u16();
					IF_VERBOSE_PARSE(log_msg("  char id = %d\n", m_character_id));
				}

				if (has_matrix)
				{
					m_has_matrix = true;
					m_matrix.read(in);
					IF_VERBOSE_PARSE(log_msg("  mat:\n"); m_matrix.print());
				}
				if (has_cxform)
				{
					m_has_cxform = true;
					m_color_transform.read_rgba(in);
					IF_VERBOSE_PARSE(log_msg("  cxform:\n"); m_color_transform.print());
				}

				if (has_ratio)
				{
					m_ratio = (float)in->read_u16() / (float)65535;
					IF_VERBOSE_PARSE(log_msg("  ratio: %f\n", m_ratio));
				}

				if (has_name)
				{
					in->read_string(&m_character_name);
					IF_VERBOSE_PARSE(log_msg("  name = %s\n", m_character_name.c_str()));
				}
				if (has_clip_bracket)
				{
					m_clip_depth = in->read_u16(); 
					IF_VERBOSE_PARSE(log_msg("  clip_depth = %d\n", m_clip_depth));
				}

				if (has_filter_list)
				{
					read_filter_list(in);
				}

				if (has_blend_mode)
				{
					// TODO, implement blend_mode
					m_blend_mode = in->read_u8();
				}

				if (has_actions)
				{
					Uint16	reserved = in->read_u16();
					UNUSED(reserved);

					// The logical 'or' of all the following handlers.
					// I don't think we care about this...
					Uint32	all_flags = 0;
					if (movie_version >= 6)
					{
						all_flags = in->read_u32();
					}
					else
					{
						all_flags = in->read_u16();
					}
					UNUSED(all_flags);

					IF_VERBOSE_PARSE(log_msg("  actions: flags = 0x%X\n", all_flags));

					// Read swf_events.
					for (;;)
					{
						// Read event.
						in->align();

						Uint32 flags = (movie_version >= 6) ? in->read_u32() : in->read_u16();
						if (flags == 0)
						{
							// Done with events.
							break;
						}

						Uint32 event_length = in->read_u32();
						Uint8 ch = key::INVALID;

						if (flags & (1 << 17))	// has keypress event
						{
							ch = in->read_u8();
							event_length--;
						}

						// Read the actions for event(s)
						action_buffer action;
						action.read(in);

						if (action.get_length() != static_cast<int>(event_length))
						{
							log_error("swf_event::read(), event_length = %d, but read %d\n",
								event_length, action.get_length());
							break;
						}

						// 13 bits reserved, 19 bits used
						static const event_id s_code_bits[19] =
						{
							event_id::LOAD,
							event_id::ENTER_FRAME,
							event_id::UNLOAD,
							event_id::MOUSE_MOVE,
							event_id::MOUSE_DOWN,
							event_id::MOUSE_UP,
							event_id::KEY_DOWN,
							event_id::KEY_UP,
							event_id::DATA,
							event_id::INITIALIZE,
							event_id::PRESS,
							event_id::RELEASE,
							event_id::RELEASE_OUTSIDE,
							event_id::ROLL_OVER,
							event_id::ROLL_OUT,
							event_id::DRAG_OVER,
							event_id::DRAG_OUT,
							event_id(event_id::KEY_PRESS, key::CONTROL),
							event_id::CONSTRUCT
						};

						// Let's see if the event flag we received is for an event that we know of
						if (1 << (TU_ARRAYSIZE(s_code_bits)) < flags)
						{
							log_error("swf_event::read() -- unknown event type received, flags = 0x%x\n", flags);
						}

						for (int i = 0, mask = 1; i < int(sizeof(s_code_bits)/sizeof(s_code_bits[0])); i++, mask <<= 1)
						{
							if (flags & mask)
							{
								swf_event* ev = new swf_event;
								ev->m_event = s_code_bits[i];

								if (i == 17)	// has keypress event ?
								{
									ev->m_event.m_key_code = ch;
								}

								// Create a function to execute the actions.
								array<with_stack_entry>	empty_with_stack;
								as_s_function*	func = new as_s_function(player, &action, 0, empty_with_stack);
								func->set_length(action.get_length());

								ev->m_method.set_as_object(func);

								m_event_handlers.push_back(ev);
							}
						}
					}
				}

				if (has_char == true && flag_move == true)
				{
					// Remove whatever's at m_depth, and put m_character there.
					m_place_type = REPLACE;
				}
				else if (has_char == false && flag_move == true)
				{
					// Moves the object at m_depth to the new location.
					m_place_type = MOVE;
				}
				else if (has_char == true && flag_move == false)
				{
					// Put m_character at m_depth.
					m_place_type = PLACE;
				}

				//log_msg("place object at depth %i\n", m_depth);
			}
		}


		void	execute(character* m)
			// Place/move/whatever our object in the given movie.
		{
			switch (m_place_type)
			{
				default:
					break;

				case PLACE:
					if ((m_blend_mode == 0) && (m->m_blend_mode != 0))
					{
						m->add_display_object( m_character_id, m_character_name.c_str(), m_event_handlers, m_depth, m_tag_type != 4, m_color_transform, m_matrix, m_ratio, m_clip_depth, m->m_blend_mode);
					}
					else
					{
						m->add_display_object( m_character_id, m_character_name.c_str(), m_event_handlers, m_depth, m_tag_type != 4, m_color_transform, m_matrix, m_ratio, m_clip_depth, m_blend_mode);
					}
					break;

				case MOVE:
					if ((m_blend_mode == 0) && (m->m_blend_mode != 0))
					{
						m->move_display_object( m_depth, m_has_cxform, m_color_transform, m_has_matrix, m_matrix, m_ratio, m_clip_depth, m->m_blend_mode);
					}
					else
					{
						m->move_display_object( m_depth, m_has_cxform, m_color_transform, m_has_matrix, m_matrix, m_ratio, m_clip_depth, m_blend_mode);
					}
					break;

				case REPLACE:
					m->replace_display_object( m_character_id, m_character_name.c_str(), m_depth, m_has_cxform, m_color_transform, m_has_matrix, m_matrix, m_ratio, m_clip_depth, m_blend_mode);
					break;
				}
		}

		void	execute_state(character* m)
		{
			execute(m);
		}

		void	execute_state_reverse(character* m, int frame)
		{
			switch (m_place_type)
			{
				default:
					break;

				case PLACE:
					// reverse of add is remove
					// NOTE:  this line used to read m->remove_display_object(m_depth, m_tag_type == 4 ? m_character : -1);
					// we were having problems with in certain cases remove_display_object removing the incorrect object,
					// the line was returning -1 frequently and inside remove_display_object it would pick an incorrect object
					// at the same depth
					m->remove_display_object(m_depth, m_character_id);
					break;

				case MOVE:
					// reverse of move is move
					m->move_display_object( m_depth, m_has_cxform, m_color_transform, m_has_matrix, m_matrix, m_ratio, m_clip_depth, m->m_blend_mode);
					break;

				case REPLACE:
					{
						// reverse of replace is to re-add the previous object.
						execute_tag*	last_add = m->find_previous_replace_or_add_tag(frame, m_depth, -1);
						if (last_add)
						{
							last_add->execute_state(m);
						}
						else
						{
							log_error("reverse REPLACE can't find previous replace or add tag(%d, %d)\n", frame, m_depth);
						}
						break;
					}
			}
		}

		virtual uint32	get_depth_id_of_replace_or_add_tag() const
			// "depth_id" is the 16-bit depth & id packed into one 32-bit int.
		{
			if (m_place_type == PLACE || m_place_type == REPLACE)
			{
				int	id = -1;
				if (m_tag_type == 4)
				{
					// Old-style PlaceObject; the corresponding Remove
					// is specific to the character_id.
					id = m_character_id;
				}
				return ((m_depth & 0x0FFFF) << 16) | (id & 0x0FFFF);
			}
			else
			{
				return (uint32) -1;
			}
		}

		void getSizeByPath(float &w, float &h, array<path>&	paths)
		{
			
		}

		void drawPathToBitmap(const array<path>& sPath, gameswf::bitmap_info* bitmap, point sAnchorPoint, rgba& color)
		{
			int w = bitmap->get_width();
			int h = bitmap->get_height();
			auto pData = bitmap->get_data();
			float  x = w*20* sAnchorPoint.m_x;
			float y = h*20* (1.0f- sAnchorPoint.m_y);
			memset(pData, 0, w*h * 4);
			for (int i = 0; i < w ; i ++)
			{
				for (int j = 0; j < h ;j ++)
				{
					for (int k = 0; k < sPath.size(); k++)
					{
						auto it = sPath[k];// .point_test(i, j);
						float ix = (i* 20 - x);
						float jy = (j* 20 - y);
						if (it.point_test(ix, jy))
						{
							pData[(j*w + i) * 4] = color.m_r;
							pData[(j*w + i) * 4+1] = color.m_g;
							pData[(j*w + i) * 4+2] = color.m_b;
							pData[(j*w + i) * 4+3] = color.m_a;
							break;
						}
					}

				}

			}

		}


		void drawBitmapToBitmap(gameswf::bitmap_info* dst, gameswf::bitmap_info* src, int offx=0,int offy=0,int w=0,int h=0)
		{
			int dw = dst->get_width();
			int dh = dst->get_height();
			unsigned char* pDstData = dst->get_data();


			int sw = src->get_width();
			int sh = src->get_height();
			unsigned char* pSrcData = src->get_data();

			w = w == 0 ? sw : w;
			h = h == 0 ? sh : h;
			int startx = 0;
			int starty = 0;
			
			if (offx < 0)
			{
				w = w + offx;
				offx = 0;
				startx = -offx;
			}

			if (offy < 0)
			{
				h = h + offy;
				offy = 0;
				starty = -offy;
			}

			if (offx + w > dw)
			{
				w = dw - offx;
			}

			if (offy + h > dh)
			{
				h = dh - offy;
			}


			if (w>0&&h>0)
			{
				for (int i = 0; i < h; i++)
				{
					for (int j = 0; j < w; j++)
					{
						auto pDst = &pDstData [((offy + i)*dw + offx+ j) * 4] ;
						auto pSrc = &pSrcData[((starty + i)*sw + startx+j) * 4];

						float a = pSrc[3] / 255.0f;
						pDst[0] = pDst[0] * (1.0 - a) + pSrc[0]* a;
						pDst[1] = pDst[1] * (1.0 - a) + pSrc[1]* a;
						pDst[2] = pDst[2] * (1.0 - a) + pSrc[2]* a;

					}
				}
			}
			

		}

		void maskBitmapByPath(const array<path>& sPath, gameswf::bitmap_info* bitmap, point sAnchorPoint, rgba& color)
		{

	
			int w = bitmap->get_width();
			int h = bitmap->get_height();
			auto pData = bitmap->get_data();
			float  x = w * 20 * sAnchorPoint.m_x;
			float y = h * 20 * (1.0f - sAnchorPoint.m_y);
		//	memset(pData, 0, w*h * 4);
			for (int i = 0; i < w; i++)
			{
				for (int j = 0; j < h; j++)
				{

				
					bool bFind = false;

					for (int k = 0; k < sPath.size(); k++)
					{
						auto it = sPath[k];// .point_test(i, j);
						float ix = (i * 20 - x);
						float jy = (j * 20 - y);
						if (it.point_test(ix, jy))
						{
							bFind = true;
							break;
						}
					}
					if (!bFind)
					{
						 //pData[(j*w + i) * 4];
						 //pData[(j*w + i) * 4 + 1];
						 //pData[(j*w + i) * 4 + 2];
						pData[(j*w + i) * 4 + 3] = 0;
					}

				}

			}

		}

		void copyDataToPackData(int x, int y, int w, int h,unsigned char* pData, unsigned char*pTargetData, int targetWidth)
		{
			for (int i = 0; i < h; ++i)
			{
				memcpy(pTargetData + ((i + y) * targetWidth + x) * 4, pData + i * w * 4, w * 4);
			}
		}

		Uint8*	software_resample(
			int bytes_per_pixel,
			int src_width,
			int src_height,
			int src_pitch,
			uint8* src_data,
			int dst_width,
			int dst_height)
			// Code from Alex Streit
			//
			// Creates an OpenGL texture of the specified dst dimensions, from a
			// resampled version of the given src image.  Does a bilinear
			// resampling to create the dst image.
		{
			//	printf("original bitmap %dx%d, resampled bitmap %dx%d\n",
			//		src_width, src_height, dst_width, dst_height);

			assert(bytes_per_pixel == 3 || bytes_per_pixel == 4);

			//	assert(dst_width >= src_width);
			//	assert(dst_height >= src_height);

			//	unsigned int	internal_format = bytes_per_pixel == 3 ? GL_RGB : GL_RGBA;
			unsigned int	input_format = bytes_per_pixel == 3 ? GL_RGB : GL_RGBA;

			// FAST bi-linear filtering
			// the code here is designed to be fast, not readable
			Uint8* rescaled = new Uint8[dst_width * dst_height * bytes_per_pixel];
			float Uf, Vf;		// fractional parts
			float Ui, Vi;		// integral parts
			float w1, w2, w3, w4;	// weighting
			Uint8* psrc;
			Uint8* pdst = rescaled;

			// i1,i2,i3,i4 are the offsets of the surrounding 4 pixels
			const int i1 = 0;
			const int i2 = bytes_per_pixel;
			int i3 = src_pitch;
			int i4 = src_pitch + bytes_per_pixel;

			// change in source u and v
			float dv = (float)(src_height - 2) / dst_height;
			float du = (float)(src_width - 2) / dst_width;

			// source u and source v
			float U;
			float V = 0;

#define BYTE_SAMPLE(offset)	\
(Uint8) (w1 * psrc[i1 + (offset)] + w2 * psrc[i2 + (offset)] + w3 * psrc[i3 + (offset)] + w4 * psrc[i4 + (offset)])

			if (bytes_per_pixel == 3)
			{
				for (int v = 0; v < dst_height; ++v)
				{
					Vf = modff(V, &Vi);
					V += dv;
					U = 0;

					for (int u = 0; u < dst_width; ++u)
					{
						Uf = modff(U, &Ui);
						U += du;

						w1 = (1 - Uf) * (1 - Vf);
						w2 = Uf * (1 - Vf);
						w3 = (1 - Uf) * Vf;
						w4 = Uf * Vf;
						psrc = &src_data[(int)(Vi * src_pitch) + (int)(Ui * bytes_per_pixel)];

						*pdst++ = BYTE_SAMPLE(0);	// red
						*pdst++ = BYTE_SAMPLE(1);	// green
						*pdst++ = BYTE_SAMPLE(2);	// blue

						psrc += 3;
					}
				}

#ifdef DEBUG_WRITE_TEXTURES_TO_PPM
				static int s_image_sequence = 0;
				char temp[256];
				sprintf(temp, "image%d.ppm", s_image_sequence++);
				FILE* f = fopen(temp, "wb");
				if (f)
				{
					fprintf(f, "P6\n# test code\n%d %d\n255\n", dst_width, dst_height);
					fwrite(rescaled, dst_width * dst_height * 3, 1, f);
					fclose(f);
				}
#endif
			}
			else
			{
				assert(bytes_per_pixel == 4);

				for (int v = 0; v < dst_height; ++v)
				{
					Vf = modff(V, &Vi);
					V += dv;
					U = 0;

					for (int u = 0; u < dst_width; ++u)
					{
						Uf = modff(U, &Ui);
						U += du;

						w1 = (1 - Uf) * (1 - Vf);
						w2 = Uf * (1 - Vf);
						w3 = (1 - Uf) * Vf;
						w4 = Uf * Vf;
						psrc = &src_data[(int)(Vi * src_pitch) + (int)(Ui * bytes_per_pixel)];

						*pdst++ = BYTE_SAMPLE(0);	// red
						*pdst++ = BYTE_SAMPLE(1);	// green
						*pdst++ = BYTE_SAMPLE(2);	// blue
						*pdst++ = BYTE_SAMPLE(3);	// alpha

						psrc += 4;
					}
				}
			}

			
			//create_texture(input_format, dst_width, dst_height, rescaled, 0);


			//delete[] rescaled;
		}

		rgba getBlendColor(rgba & color,cxform& cx)
		{
			rgba sColor;
			auto& m = cx.m_;
			float c00 = m_color_transform.m_[0][0], c01 = m_color_transform.m_[0][1], c10 = m_color_transform.m_[1][0], c11 = m_color_transform.m_[1][1];
			float c20 = m_color_transform.m_[2][0], c21 = m_color_transform.m_[2][1], c30 = m_color_transform.m_[3][0], c31 = m_color_transform.m_[3][1];
			sColor.m_r = color.m_r*m[0][0] + m[0][1];
			sColor.m_g = color.m_g*m[1][0] + m[1][1];
			sColor.m_b = color.m_b*m[2][0] + m[2][1];
			sColor.m_a = color.m_a*m[3][0] + m[3][1];
			return sColor;
		}
	
		void myDraw(movie_def_impl* pMovie)
		{

		}
		unsigned int s_textureColorbuffer = 0;
		unsigned int s_framebuffer;
		unsigned int s_fbwidth, s_fbheight;


		unsigned int s_textureColorBufferMultiSampled;
		unsigned int s_intermediateFBO;
		unsigned int s_screenTexture;
		unsigned int s_rbo = 0;
		std::map<int, MyBitmap > m_sMyBitMaps;
		void beginRenderToFramebuffer(int width, int height)
		{
			width = 800;
			height = 640;
			static float m[16] = {
			0, 0,  0, width,
			0, 0,  0, height,
			0, 0,  0, 0,
			0, 0,  0, 1

			};
			rgba s;
			m[3] = width;
			m[7] = height;
			render::begin_display(m, s, 0, 0, 0, 0, 0, 0, 0, 0);
			render::setGenImageMode(true);

			
			glBlendFunc(GL_ONE, GL_ZERO);
			glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			if (m_clip_depth > 0)
			{
				render::begin_submit_mask();
				glDisable(GL_ALPHA_TEST);
				glDisable(GL_STENCIL_TEST);
			}
			s_myBitmapId = 0;
			
		}

		void saveFramebuffetToFile(std::string &fileName)
		{
			GLubyte* pixels = (GLubyte*)malloc(s_fbwidth * s_fbheight * sizeof(GLubyte) * 4);
			glReadPixels(0, 0, s_fbwidth, s_fbheight, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
			//glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA,GL_UNSIGNED_BYTE,outBuffer);
			stbi_flip_vertically_on_write(1);

			stbi_write_png(fileName.c_str(), s_fbwidth, s_fbheight, 4, pixels, 0);
			free(pixels);

		
		}


		void endRenderToFreameBuffer(std::string &fileName)
		{
#if 0 
			saveFramebuffetToFile(fileName);
#else 
			mergeMyBitMaps(fileName);
#endif

			render::setGenImageMode(false);
			if (m_clip_depth > 0)
			{
				render::end_submit_mask();
				render::disable_mask();
			}
		}
		

		void saveFrameBufferToMem(int id)
		{
			int w = s_fbwidth;
			int h = s_fbheight;
			//int w = 800;
			//int h = 640;
			//glfwSwapBuffers(MyUtils::getCreateWindow());
			GLubyte* pixels = (GLubyte*)malloc(w * h * sizeof(GLubyte) * 4);
			glReadPixels(0, 640 - h, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
			MyBitmap t;
			t.m_data = pixels;
			t.m_w = s_fbwidth;
			t.m_h = s_fbheight;
			t.m_channel = 4;
			m_sMyBitMaps[id] = t;

			/*static char f[20] = { 0 };
			static int i = 0;
			i++;
			sprintf(f, "test%d.png", i);
			stbi_flip_vertically_on_write(1);
			stbi_write_png(f, w, h, 4, pixels, 0);*/
		}
		static place_object_2* s_self;
		static int s_myBitmapId;
		static void callback() {

			s_self->saveFrameBufferToMem(s_myBitmapId);
			s_myBitmapId++;
		}
		void drawMyBitmapToMem(MyBitmap& sMyBitmap,unsigned char * pTargetData)
		{
			int w = sMyBitmap.m_w;
			int h = sMyBitmap.m_h;
			int channel = sMyBitmap.m_channel;
			unsigned char * pSrcData = sMyBitmap.m_data;

			for (int j = 0; j < h; ++j)
			{
				for (int i = 0; i < w; ++i)
				{
					unsigned char * ps = &pSrcData[(j*w+i)*channel];
					unsigned char * pd = &pTargetData[(j*w + i)*channel];

					if (pd[3] == 0)
					{
						pd[0] = ps[0];
						pd[1] = ps[1];
						pd[2] = ps[2];
						pd[3] = ps[3];
					}
					else
					{
						float sa = ps[3] / 255.0f;
						float sd = 1.0 - sa;

						pd[0] = ps[0]* sa + sd* pd[0];
						pd[1] = ps[1] * sa + sd * pd[1];
						pd[2] = ps[2] * sa + sd * pd[2];
						pd[3] = ps[3] * sa + sd * pd[3];
					}
				}
			}
		}

		void mergeMyBitMaps(std::string &fileName)
		{
			int lenghth = s_fbwidth * s_fbheight * 4;
			GLubyte* pixels = (GLubyte*)malloc(lenghth);
			memset(pixels, 0, lenghth);
			for (auto it = m_sMyBitMaps.begin(); it != m_sMyBitMaps.end(); it++)
			{
				drawMyBitmapToMem(it->second,pixels);
			}

			stbi_flip_vertically_on_write(1);

			stbi_write_png(fileName.c_str(), s_fbwidth, s_fbheight, 4, pixels, 0);
			free(pixels);
			clearMyBitmaps();
		}

		void clearMyBitmaps()
		{
			for (auto it = m_sMyBitMaps.begin(); it != m_sMyBitMaps.end(); it++)
			{
				free(it->second.m_data);
			}
			m_sMyBitMaps.clear();
		}

	


		void genImage(movie_def_impl* pMovie, shape_character_def*pCharactor, point& sAnchorPoint, float w ,float h )
		{
			
			static char pBuf[512] = { 0 };
			std::map<int, gameswf::bitmap_info*> &s_sBitmapInfos = pMovie->getBitmapInfos();
			std::map<int, gameswf::bitmap_info*> &s_sChBitmapInfos = pMovie->getCHBitmapInfos();

			std::map<int, rect>& s_sRects = pMovie->getRects();
			int &bitmapId = pMovie->getBitmapId();


			auto pBitmapInfo = s_sBitmapInfos[m_depth];
			auto pChBitmapInfo = s_sChBitmapInfos[m_character_id];


			auto fillStyles = pCharactor->get_fill_styles();
			bool bNeedGenBitmap = false;
			for (int i = 0; i < fillStyles.size(); i++)
			{
				auto style = fillStyles[i];
				auto bc = style.getBitmapCharacter();
				if (bc)
				{
					pBitmapInfo = bc->get_bitmap_info();
					auto imageW = pBitmapInfo->get_width();
					auto imageH = pBitmapInfo->get_height();

					if (pChBitmapInfo != pBitmapInfo)

					{
						memset(pBuf, 0, sizeof(pBuf));
						sprintf(pBuf, "{%d,%d,%d,%g,%g},", 1, m_character_id, bitmapId, w, h);
						pMovie->writeStrToFile(pBuf);
						//pMovie->writeBitmapData(pBitmapInfo, bitmapId, w, h);
						bitmapId++;
						s_sBitmapInfos[m_depth] = pBitmapInfo;
						s_sChBitmapInfos[m_character_id] = pBitmapInfo;
						bNeedGenBitmap = false;
						break;
					}
					else
					{
						s_sBitmapInfos[m_depth] = pBitmapInfo;
					}

				}
				else
				{
					pChBitmapInfo = s_sChBitmapInfos[m_character_id];
					{
						auto nType = style.get_type();
						if (nType == 0)
						{
							bNeedGenBitmap = true;
						}
						else if (0x12 == nType || 0x10 == nType)
						{
							bNeedGenBitmap = true;

						}
					}

				}

			}


			if (bNeedGenBitmap && (!pChBitmapInfo))
			{
				float scale = 1.0;
				s_fbwidth = ceil(w)*scale;
				s_fbheight = ceil(h)*scale;
				auto maxScale = m_matrix.get_max_scale();

				beginRenderToFramebuffer(s_fbwidth, s_fbheight);

				mesh_set*	m = new mesh_set(pCharactor, 5);
				s_self = this;
				m->setObject(callback);
				matrix  tmatrix;
				tmatrix.m_[0][0] = scale;
				tmatrix.m_[1][1] = scale;
				tmatrix.m_[0][2] = scale *w * sAnchorPoint.m_x*20.0f;
				tmatrix.m_[1][2] = scale *h * (1 - sAnchorPoint.m_y)*20.0f;
				cxform cx;
				m->display(tmatrix, cx, pCharactor->get_fill_styles(), pCharactor->get_line_styles(), render_handler::bitmap_blend_mode::BLEND_NORMAL);
				//pMovie->getFileName(bitmapId);
				std::string sName = "test";
				endRenderToFreameBuffer(sName);


				{
					int tw = ceil(w);// w + 0.5f;
					int th = ceil(h);// + 0.5f;
					auto pImage = image::create_rgba(tw, th);

					gameswf::bitmap_info* pInfo = render::create_bitmap_info_rgba(pImage);

					memset(pBuf, 0, sizeof(pBuf));
					sprintf(pBuf, "{%d,%d,%d,%g,%g},", 1, m_character_id, bitmapId, w, h);
					pMovie->writeStrToFile(pBuf);
					bitmapId++;
					s_sBitmapInfos[m_depth] = pInfo;
					s_sChBitmapInfos[m_character_id] = pInfo;
				}


			}
		}


		void printMatrixAndCXForm(movie_def_impl* pMovie,point &sAnchorPoint,int w,int h)
		{
			static char pBuf[512] = { 0 };
			float a = m_matrix.m_[0][0];
			float b = m_matrix.m_[0][1];
			float d = m_matrix.m_[1][0];
			float e = m_matrix.m_[1][1];

			auto sx = m_matrix.get_x_scale();
			auto sy = m_matrix.get_y_scale();
			auto angle = m_matrix.get_rotation();


			float sinAngle = std::sin(angle);
			float cosAngle = std::cos(angle);





			auto x = -w * sx *sAnchorPoint.m_x;
			auto y = -h * sy *(1.0 - sAnchorPoint.m_y);


			float tempX = x * cosAngle - y * sinAngle;
			y = y * cosAngle + x * sinAngle;
			x = tempX;



			float c = m_matrix.m_[0][2] / 20.0f  + x;

			float f = m_matrix.m_[1][2] / 20.0f  + y;

			float c00 = m_color_transform.m_[0][0], c01 = m_color_transform.m_[0][1], c10 = m_color_transform.m_[1][0], c11 = m_color_transform.m_[1][1];
			float c20 = m_color_transform.m_[2][0], c21 = m_color_transform.m_[2][1], c30 = m_color_transform.m_[3][0], c31 = m_color_transform.m_[3][1];

			float count = 1000.0f;
			a = (int)(count * a + 0.5) / count;
			b = (int)(count * b + 0.5) / count;
			c = (int)(count * c + 0.5) / count;
			d = (int)(count * d + 0.5) / count;
			e = (int)(count * e + 0.5) / count;
			f = (int)(count * f + 0.5) / count;

			c00 = (int)(count * c00 + 0.5) / count;
			c01 = (int)(count * c01 + 0.5) / count;
			c10 = (int)(count * c10 + 0.5) / count;
			c11 = (int)(count * c11 + 0.5) / count;
			c20 = (int)(count * c20 + 0.5) / count;
			c21 = (int)(count * c21 + 0.5) / count;
			c30 = (int)(count * c30 + 0.5) / count;
			c31 = (int)(count * c31 + 0.5) / count;


			//float tx = a * c + d * f;
			//float ty = b * c + e * f;
			angle = 57.295780*angle;

			if (m_has_cxform)
			{
				if (m_has_matrix)
				{
					memset(pBuf, 0, sizeof(pBuf));
					sprintf(pBuf, "{%d,%d,%d,{%g,%g,%g,%g,%g}", 2, m_depth, m_character_id, sx, sy, angle,c, f);
					pMovie->writeStrToFile(pBuf);
					memset(pBuf, 0, sizeof(pBuf));
					sprintf(pBuf, ",{%g,%g,%g,%g,%g,%g,%g,%g},%d,%d},", c00, c10, c20, c30, c01, c11, c21, c31, m_clip_depth, 0);
					pMovie->writeStrToFile(pBuf);
				}
				else
				{
					memset(pBuf, 0, sizeof(pBuf));
					sprintf(pBuf, "{%d,%d,%d,0", 2, m_depth, m_character_id);
					pMovie->writeStrToFile(pBuf);
					memset(pBuf, 0, sizeof(pBuf));
					sprintf(pBuf, ",{%g,%g,%g,%g,%g,%g,%g,%g},%d,%d},", c00, c10, c20, c30, c01, c11, c21, c31, m_clip_depth, 0);
					pMovie->writeStrToFile(pBuf);
				}

			}
			else
			{
				if (m_has_matrix)
				{
					memset(pBuf, 0, sizeof(pBuf));
					sprintf(pBuf, "{%d,%d,%d,{%g,%g,%g,%g,%g},%d,%d,%d},", 2, m_depth, m_character_id, sx, sy, angle,c, f, 0, m_clip_depth, 0);
					pMovie->writeStrToFile(pBuf);
				}
				else
				{
					memset(pBuf, 0, sizeof(pBuf));
					sprintf(pBuf, "{%d,%d,%d,%d,%d,%d,%d},", 2, m_depth, m_character_id, 0, 0, m_clip_depth, 0);
					pMovie->writeStrToFile(pBuf);
				}


			}

		
		}

		void print(movie_def_impl* pMovie)
		{
			static char pBuf[512] = { 0 };
			auto & s_movieClips = pMovie->getMovieClips();
			auto pSprite = dynamic_cast<sprite_definition*>(pMovie->get_character_def(m_character_id));
			if (pSprite)
			{
				memset(pBuf, 0, sizeof(pBuf));
				sprintf(pBuf, "{%d,%d,%d,{", 1, m_character_id, -1);
				pMovie->writeStrToFile(pBuf);

				auto size = pSprite->m_playlist.size();
				for (int i = 0; i < size; ++i)
				{
					memset(pBuf, 0, sizeof(pBuf));
					sprintf(pBuf, "{");
					pMovie->writeStrToFile(pBuf);

					auto frames = pSprite->m_playlist[i];
					auto count = frames.size();
					//writeStrToFile("    {");
					for (int j = 0; j < count; ++j)
					{
						frames[j]->print(pMovie);
					}

					memset(pBuf, 0, sizeof(pBuf));
					sprintf(pBuf, "},");
					pMovie->writeStrToFile(pBuf);
					//writeStrToFile("},\n");
				}

				memset(pBuf, 0, sizeof(pBuf));
				sprintf(pBuf, "}},");
				pMovie->writeStrToFile(pBuf);
				s_movieClips[m_depth] = m_character_id;
				//return;
			}
			auto pCharactor = dynamic_cast<shape_character_def*>(pMovie->get_character_def(m_character_id));
			if(!pCharactor)
			{

				auto it = s_movieClips.find(m_depth);
				if (it != s_movieClips.end())
				{
					float a = m_matrix.m_[0][0];
					float b = m_matrix.m_[0][1];
					float d = m_matrix.m_[1][0];
					float e = m_matrix.m_[1][1];
					float c = m_matrix.m_[0][2] / 20.0f;
					float f = m_matrix.m_[1][2] / 20.0f;

					float c00 = m_color_transform.m_[0][0], c01 = m_color_transform.m_[0][1], c10 = m_color_transform.m_[1][0], c11 = m_color_transform.m_[1][1];
					float c20 = m_color_transform.m_[2][0], c21 = m_color_transform.m_[2][1], c30 = m_color_transform.m_[3][0], c31 = m_color_transform.m_[3][1];
					if (m_has_cxform)
					{
						if (m_has_matrix)
						{
							memset(pBuf, 0, sizeof(pBuf));
							sprintf(pBuf, "{%d,%d,%d,{%g,%g,%g,%g,%g,%g}", 2, m_depth, m_character_id, a, d, b, e, c, f);
							pMovie->writeStrToFile(pBuf);
							memset(pBuf, 0, sizeof(pBuf));
							sprintf(pBuf, ",{%g,%g,%g,%g,%g,%g,%g,%g},%d,%d},", c00, c10, c20, c30, c01, c11, c21, c31, m_clip_depth, 0);
							pMovie->writeStrToFile(pBuf);
						}
						else
						{
							memset(pBuf, 0, sizeof(pBuf));
							sprintf(pBuf, "{%d,%d,%d,0", 2, m_depth, m_character_id);
							pMovie->writeStrToFile(pBuf);
							memset(pBuf, 0, sizeof(pBuf));
							sprintf(pBuf, ",{%g,%g,%g,%g,%g,%g,%g,%g},%d,%d},", c00, c10, c20, c30, c01, c11, c21, c31, m_clip_depth, 0);
							pMovie->writeStrToFile(pBuf);
						}

					}
					else
					{
						if (m_has_matrix)
						{
							memset(pBuf, 0, sizeof(pBuf));
							sprintf(pBuf, "{%d,%d,%d,{%g,%g,%g,%g,%g,%g},%d,%d,%d},", 2, m_depth, m_character_id, a, d, b, e, c, f, 0, m_clip_depth, 0);
							pMovie->writeStrToFile(pBuf);
						}
						else
						{
							memset(pBuf, 0, sizeof(pBuf));
							sprintf(pBuf, "{%d,%d,%d,%d,%d,%d,%d},", 2, m_depth, m_character_id, 0, 0, m_clip_depth, 0);
							pMovie->writeStrToFile(pBuf);
						}


					}
					return;
				}
			}







			std::map<int, rect>& s_sRects = pMovie->getRects();

			

			float w = 0, h = 0;
			point sAnchorPoint(0.5, 0.5);

			std::map<int, gameswf::bitmap_info*> &s_sBitmapInfos = pMovie->getBitmapInfos();
			std::map<int, gameswf::bitmap_info*> &s_sChBitmapInfos = pMovie->getCHBitmapInfos();
			auto pBitmapInfo = s_sBitmapInfos[m_depth];
			auto pChBitmapInfo = s_sChBitmapInfos[m_character_id];
			auto ite = s_sRects.find(m_depth);
			if (pCharactor)
			{
				rect srect;
				pCharactor->get_bound(&srect);
				w = TWIPS_TO_PIXELS(srect.m_x_max - srect.m_x_min);
				h = TWIPS_TO_PIXELS(srect.m_y_max - srect.m_y_min);
				sAnchorPoint = srect.getAnchorPoint();
				s_sRects[m_depth] = srect;


#if 1

				genImage(pMovie, pCharactor, sAnchorPoint, w, h);

#else
				auto fillStyles = pCharactor->get_fill_styles();
				bool bFillVertor = false;
				for (int i = 0; i < fillStyles.size(); i++)
				{
					static int bitmapId = 1;
					auto style = fillStyles[i];
					auto bc = style.getBitmapCharacter();
					if (bc)
					{
						pBitmapInfo = bc->get_bitmap_info();
						auto imageW = pBitmapInfo->get_width();
						auto imageH = pBitmapInfo->get_height();

						if (pChBitmapInfo != pBitmapInfo)

						{
							memset(pBuf, 0, sizeof(pBuf));
							sprintf(pBuf, "{%d,%d,%d,%g,%g},", 1, m_character_id, bitmapId, w, h);
							pMovie->writeStrToFile(pBuf);
							pMovie->writeBitmapData(pBitmapInfo, bitmapId, w, h);
							bitmapId++;
							s_sBitmapInfos[m_depth] = pBitmapInfo;
							s_sChBitmapInfos[m_character_id] = pBitmapInfo;
							break;
						}
						else
						{
							s_sBitmapInfos[m_depth] = pBitmapInfo;
						}

					}
					else
					{
						pChBitmapInfo = s_sChBitmapInfos[m_character_id];
						{
							auto nType = style.get_type();
							if (nType == 0)
							{
								int tw = ceil(w);// w + 0.5f;
								int th = ceil(h);// + 0.5f;
								auto pImage = image::create_rgba(tw, th);

								gameswf::bitmap_info* pInfo = render::create_bitmap_info_rgba(pImage);

								auto maxScale = m_matrix.get_max_scale();
								draw_into_bitmap	accepter;
								memset(pInfo->get_data(), 0, pInfo->get_width()*pInfo->get_height() * 4);
								accepter.setBitmapInfo(pInfo, sAnchorPoint, getBlendColor(style.get_color(), m_color_transform));
								pCharactor->tesselate(20.0f / maxScale * 0.75, &accepter);
								//drawPathToBitmap(pCharactor->get_paths(), pInfo, sAnchorPoint, style.get_color());

								if (maxScale > 1.0f)
								{
									int sw = tw / maxScale;
									int sh = th / maxScale;
									unsigned char* output_data = (unsigned char*)malloc(w * h * 4);
									stbir_resize_uint8(pInfo->get_data(), tw, th, 0, output_data, sw, sh, 0, 4);
									//stbi_write_png(fileName.c_str(), w, h, 4, output_data, 0);
									memset(pInfo->get_data(), 0, tw*th * 4);
									copyDataToPackData((tw - sw) / 2, (th - sh) / 2, sw, sh, output_data, pInfo->get_data(), tw);
									free(output_data);
								}




								if (!pChBitmapInfo)
								{
									memset(pBuf, 0, sizeof(pBuf));
									sprintf(pBuf, "{%d,%d,%d,%g,%g},", 1, m_character_id, bitmapId, w, h);
									pMovie->writeStrToFile(pBuf);
									//maskBitmapByPath(pCharactor->get_paths(), pInfo, sAnchorPoint, style.get_color());
									pMovie->writeBitmapData(pInfo, bitmapId, tw, th);
									bitmapId++;
									s_sBitmapInfos[m_depth] = pInfo;
									s_sChBitmapInfos[m_character_id] = pInfo;
									bFillVertor = true;
								}

							}
							else if (0x12 == nType || 0x10 == nType)
							{
								//auto pImage = image::create_rgba(w, h);
								int tw = ceil(w);
								int th = ceil(h);
								float maxScale = m_matrix.get_max_scale();
								gameswf::bitmap_info* pInfo = style.create_gradient_bitmap(tw, th);

								if (maxScale > 1.0f)
								{
									int sw = tw / maxScale;
									int sh = th / maxScale;
									gameswf::bitmap_info* pSInfo = style.create_gradient_bitmap(sw, sh);
									memset(pInfo->get_data(), 0, tw*th * 4);
									copyDataToPackData((tw - sw) / 2, (th - sh) / 2, sw, sh, pSInfo->get_data(), pInfo->get_data(), tw);

								}




								if (!pChBitmapInfo)
								{
									memset(pBuf, 0, sizeof(pBuf));
									sprintf(pBuf, "{%d,%d,%d,%g,%g},", 1, m_character_id, bitmapId, w, h);
									pMovie->writeStrToFile(pBuf);
									//maskBitmapByPath(pCharactor->get_paths(), pInfo, sAnchorPoint, style.get_color());
									pMovie->writeBitmapData(pInfo, bitmapId, tw, th);
									bitmapId++;
									s_sBitmapInfos[m_depth] = pInfo;
									s_sChBitmapInfos[m_character_id] = pInfo;
									bFillVertor = true;
								}
							}
						}

					}

				}

#endif
			}
			else if (ite != s_sRects.end())
			{
				rect srect = ite->second;
				w = TWIPS_TO_PIXELS(srect.m_x_max - srect.m_x_min);
				h = TWIPS_TO_PIXELS(srect.m_y_max - srect.m_y_min);
				sAnchorPoint = srect.getAnchorPoint();
			}

			else if (pChBitmapInfo)
			{
				w = pChBitmapInfo->get_width();
				h = pChBitmapInfo->get_height();
			}


			printMatrixAndCXForm(pMovie,sAnchorPoint,w,h);
		}
	

	};



	void	place_object_2_loader(stream* in, int tag_type, movie_definition_sub* m)
	{
		assert(tag_type == 4 || tag_type == 26 || tag_type == 70);

		IF_VERBOSE_PARSE(log_msg("  place_object_2\n"));

		place_object_2*	ch = new place_object_2;
		ch->read(m->get_player(), in, tag_type, m->get_version());

		m->add_execute_tag(ch);
	}

	// Create a (mutable) instance of our definition.  The
	// instance is created to live (temporarily) on some level on
	// the parent movie's display list.
	character*	sprite_definition::create_character_instance(character* parent, int id)
	{
		sprite_instance*	si = new sprite_instance(get_player(), this, parent->get_root(), parent, id);
		return si;
	}

	root*	movie_def_impl::create_instance()
	// Create a playable movie instance from a def.
	{
		root*	root = create_root();
		
		// create dlist
		root->get_root_movie()->execute_frame_tags(0);		

		return root;
	}

	root*	movie_def_impl::create_root()
	{
		// Is the movie instance already in the library?
		if (s_use_cached_movie_instance)
		{
			if (m_instance != NULL)
			{
				// Return cached movie instance.
				return m_instance.get_ptr();
			}
		}

		root*	m = new root(get_player(), this);
		assert(m);

		if (s_use_cached_movie_instance)
		{
			m_instance = m;
		}

		sprite_instance*	root_movie = new sprite_instance(get_player(), this, m, NULL, -1);
		assert(root_movie);

		// By default _root has no name
		//		root_movie->set_name("_root");
		root_movie->set_member("$version", get_gameswf_version());	// Flash 8
		m->set_root_movie(root_movie);
		return m;
	}


	// Create and initialize a sprite, and add it to the movie.
	void	sprite_loader(stream* in, int tag_type, movie_definition_sub* m)
	{
		assert(tag_type == 39);

		int	character_id = in->read_u16();

		IF_VERBOSE_PARSE(log_msg("  sprite\n  char id = %d\n", character_id));

		sprite_definition*	ch = new sprite_definition(m->get_player(), m);	// @@ combine sprite_definition with movie_def_impl
		ch->read(in);

		m->add_character(character_id, ch);
	}

	//
	// end_tag
	//

	// end_tag doesn't actually need to exist.

	void	end_loader(stream* in, int tag_type, movie_definition_sub* m)
	{
		assert(tag_type == 0);
		assert(in->get_position() == in->get_tag_end_position());
	}


	//
	// remove_object_2
	//


	struct remove_object_2 : public execute_tag
	{
		int	m_depth, m_id;

		remove_object_2() : m_depth(-1), m_id(-1) {}

		void read(stream* in, int tag_type)
		{
			assert(tag_type == 5 || tag_type == 28);

			if (tag_type == 5)
			{
				// Older SWF's allow multiple objects at the same depth;
				// this m_id disambiguates.  Later SWF's just use one
				// object per depth.
				m_id = in->read_u16();
			}
			m_depth = in->read_u16();
		}

		virtual void execute(character* m)
		{
			m->remove_display_object(m_depth, m_id);
		}

		virtual void execute_state(character* m)
		{
			execute(m);
		}

		virtual void execute_state_reverse(character* m, int frame)
		{
			// reverse of remove is to re-add the previous object.
			execute_tag* last_add = m->find_previous_replace_or_add_tag(frame, m_depth, m_id);
			if (last_add)
			{
				last_add->execute_state(m);
			}
			else
			{
				log_error("reverse REMOVE can't find previous replace or add tag(%d, %d)\n", frame, m_depth);
			}
		}

		virtual bool is_remove_tag() const { return true; }
		virtual void myDraw(movie_def_impl* pMovie)
		{
			pMovie->removeDepth(m_depth, m_id);
		}
		virtual void print(movie_def_impl* pMovie)
		{
			char pBuf[56] = {0};
			memset(pBuf, 0, sizeof(pBuf));
			sprintf(pBuf, "{%d,%d},", 3, m_depth);
			pMovie->writeStrToFile(pBuf);
			pMovie->removeDepth(m_depth, m_id);
		}
	};


	void	remove_object_2_loader(stream* in, int tag_type, movie_definition_sub* m)
	{
		assert(tag_type == 5 || tag_type == 28);

		remove_object_2*	t = new remove_object_2;
		t->read(in, tag_type);

		IF_VERBOSE_PARSE(log_msg("  remove_object_2(%d)\n", t->m_depth));

		m->add_execute_tag(t);
	}


	void	button_sound_loader(stream* in, int tag_type, movie_definition_sub* m)
	{
		assert(tag_type == 17);

		int	button_character_id = in->read_u16();
		button_character_definition* ch = (button_character_definition*) m->get_character_def(button_character_id);
		assert(ch != NULL);

		ch->read(in, tag_type, m);
	}


	void	button_character_loader(stream* in, int tag_type, movie_definition_sub* m)
	{
		assert(tag_type == 7 || tag_type == 34);

		int	character_id = in->read_u16();

		IF_VERBOSE_PARSE(log_msg("  button character loader: char_id = %d\n", character_id));

		button_character_definition*	ch = new button_character_definition(m->get_player());
		ch->read(in, tag_type, m);

		m->add_character(character_id, ch);
	}


	//
	// export
	//


	void	export_loader(stream* in, int tag_type, movie_definition_sub* m) // Load an export tag (for exposing internal resources of m)
	{
		assert(tag_type == 56);

		int	count = in->read_u16();

		IF_VERBOSE_PARSE(log_msg("  export: count = %d\n", count));

		// Read the exports.
		for (int i = 0; i < count; i++)
		{
			Uint16	id = in->read_u16();
			tu_string resource_name;
			in->read_string(&resource_name);
			IF_VERBOSE_PARSE(log_msg("  export: id = %d, name = %s\n", id, resource_name.c_str()));

			if (font* f = m->get_font(id))
			{
				// Expose this font for export.
				m->export_resource(resource_name, f);
			}
			else if (character_def* ch = m->get_character_def(id))
			{
				// Expose this movie/button/whatever for export.
				m->export_resource(resource_name, ch);
			}
			else if (sound_sample* ch = m->get_sound_sample(id))
			{
				m->export_resource(resource_name, ch);
			}
			else
			{
				log_error("export error: don't know how to export resource '%s'\n", resource_name.c_str());
			}
		}
	}


	//
	// import
	//


	void	import_loader(stream* in, int tag_type, movie_definition_sub* m) // Load an import tag (for pulling in external resources)
	{
		assert(tag_type == 57);

		tu_string source_url;
		in->read_string(&source_url);
		int	count = in->read_u16();

		IF_VERBOSE_PARSE(log_msg("  import: source_url = %s, count = %d\n", source_url.c_str(), count));

		// Try to load the source movie into the movie library.
		movie_definition*	source_movie = NULL;

		if (s_no_recurse_while_loading == false)
		{
			source_movie = m->get_player()->create_movie(source_url.c_str());
			if (source_movie == NULL)
			{
				// If workdir is set, try again with
				// the path relative to workdir.
				tu_string relative_url = m->get_player()->get_workdir();
				if (relative_url.length())
				{
					relative_url += source_url;
					source_movie = m->get_player()->create_movie(relative_url);
				}

				if (source_movie == NULL)
				{
					// Give up on imports.
					log_error("can't import movie from url %s\n", source_url.c_str());
					return;
				}
			}
		}

		// Get the imports.
		for (int i = 0; i < count; i++)
		{
			Uint16	id = in->read_u16();
			tu_string symbol_name;
			in->read_string(&symbol_name);
			IF_VERBOSE_PARSE(log_msg("  import: id = %d, name = %s\n", id, symbol_name.c_str()));

			if (s_no_recurse_while_loading)
			{
				m->add_import(source_url, id, symbol_name);
			}
			else
			{
				// @@ TODO get rid of this, always use
				// s_no_recurse_while_loading, change
				// create_movie_sub().

				assert(cast_to<movie_def_impl>(source_movie));
				character_def* res = cast_to<movie_def_impl>(source_movie)->get_exported_resource(symbol_name);
				if (res == NULL)
				{
					IF_VERBOSE_ACTION(log_msg("import error: resource '%s' is not exported from movie '%s'\n", symbol_name.c_str(), source_url.c_str()));
				}
				else if (font* f = cast_to<font>(res))
				{
					// Add this shared font to the currently-loading movie.
					m->add_font(id, f);
				}
				else if (character_def* ch = cast_to<character_def>(res))
				{
					// Add this character to the loading movie.
					m->add_character(id, ch);
				}
				else
				{
					IF_VERBOSE_ACTION(log_msg("import error: resource '%s' from movie '%s' has unknown type\n", symbol_name.c_str(), source_url.c_str()));
				}
			}
		}
	}

	void define_video_loader(stream* in, int tag, movie_definition_sub* m)
	{
		assert(tag == 60); // 60
		Uint16 character_id = in->read_u16();

		video_stream_definition* ch = new video_stream_definition(m->get_player());
		ch->read(in, tag, m);

		m->add_character(character_id, ch);
	}

	void video_loader(stream* in, int tag, movie_definition_sub* m)
	{
		assert(tag == 61); // 61

		Uint16 character_id = in->read_u16();
		character_def* chdef = m->get_character_def(character_id);

		video_stream_definition* ch = cast_to<video_stream_definition>(chdef);
		assert(ch != NULL);

		ch->read(in, tag, m);
	}

	void sound_stream_head_loader(stream* in, int tag, movie_definition_sub* m)
	{
		assert(tag == 18 || tag == 45);

		in->read_u8();	// player may ignore this byte
		sound_handler::format_type format = (sound_handler::format_type) in->read_uint(4);
		int	sample_rate = in->read_uint(2);	// multiples of 5512.5
		bool sample_16bit = in->read_uint(1) ? true : false;
		bool stereo = in->read_uint(1) ? true : false;
		int	sample_count = in->read_u16();
		int  latency_seek = 0;
		if (format == sound_handler::FORMAT_MP3)
		{
			latency_seek = in->read_s16();
		}

		IF_VERBOSE_PARSE(log_msg("define stream sound: format=%d, rate=%d, 16=%d, stereo=%d, ct=%d\n", int(format), sample_rate, int(sample_16bit), int(stereo), sample_count));


		sound_handler* sound = get_sound_handler();
//	where will be deleted sound ?
		if (sound)
		{
			int	handler_id = sound->create_sound(NULL, 0, sample_count, format, get_sample_rate(sample_rate), stereo);

			m->m_ss_id = handler_id;
			m->m_ss_format = format;
		}
	}

	void sound_stream_block_loader(stream* in, int tag, movie_definition_sub* m)
	{
		assert(tag == 19);

		// no sound
		if (m->m_ss_id < 0)
		{
			return;
		}

		if (m->m_ss_start == -1)
		{
			m->m_ss_start = m->get_loading_frame();
		}

		int sample_count = 0;
		int seek_samples = 0;
		if (m->m_ss_format == sound_handler::FORMAT_MP3)	//MP3
		{
			sample_count = in->read_u16();
			seek_samples = in->read_s16();
		}

		int data_size = in->get_tag_end_position() - in->get_position();
		if (data_size > 0)
		{
//			printf("frame=%d, samples=%d, data_size=%d, seek_samples=%d\n",
//				m->get_loading_frame(), sample_count, data_size, seek_samples);

			Uint8* data = new Uint8[data_size];

			for (int i = 0; i < data_size; i++)
			{
				data[i] = in->read_u8();
			}

			sound_handler* sound = get_sound_handler();
			if (sound)
			{
				sound->append_sound(m->m_ss_id, data, data_size);
			}

			delete [] data;
		}
	}

	void define_metadata_loader(stream* in, int tag, movie_definition_sub* m)
	{
		// Flash help says: Flash Player always ignores the Metadata tag.
		assert(tag == 77);
		tu_string str;
		in->read_string(&str);
	}

	// Bitmap character implementation

	bitmap_character::bitmap_character(movie_definition* rdef, bitmap_info* bi) :
		bitmap_character_def(rdef->get_player()),
		m_bitmap_info(bi)
	{
		if (get_player()->get_log_bitmap_info() == true)
		{
			static int s_file_no = 1;
			static int s_total = 0;

			int w = bi->get_width();
			int h = bi->get_height();

			// log bitmap info
			s_total += (p2(w) * p2(h)) >> 10;		// KB
			log_msg("bitmap #%04d: w=%d, h=%d, p2w=%d, p2h=%d, mem=%dK, total=%dK\n",
				s_file_no, w, h, p2(w), p2(h), (p2(w) * p2(h)) >> 10, s_total);

#if TU_CONFIG_LINK_TO_LIBPNG

			// bitmap grubber

			if (bi->get_data())
			{
				tu_string url = get_player()->get_root_filename(rdef);
				if (url.size() > 4)
				{
					url = url.utf8_substring(0, url.size() - 4);	// exclude '.swf'
				}

				char bitmap_file[256];
				snprintf(bitmap_file, 256, "%s-%04d-%dx%d.png", url.c_str(), s_file_no++, w, h);

				FILE* out = fopen(bitmap_file, "wb");
				if (out)
				{
					png_helper::write_rgba(out, bi->get_data(), w, h, bi->get_bpp());
					fclose(out);
				}
				else
				{
					log_error("can't create '%s' file\n", bitmap_file);
				}
			}
#endif

		}
	}

	bool	bitmap_character::point_test_local(float x, float y)
	// Return true if the specified point is on the interior of our shape.
	// Incoming coords are local coords.
	{
		rect coords;
		coords.m_x_min = 0.0f;
		coords.m_x_max = PIXELS_TO_TWIPS(m_bitmap_info->get_width());
		coords.m_y_min = 0.0f;
		coords.m_y_max = PIXELS_TO_TWIPS(m_bitmap_info->get_height());
		if (coords.point_test(x, y))
		{
			return true;
		}
		return false;
	}

	void bitmap_character::get_bound(rect* bound)
	{
		bound->m_x_min = 0.0f;
		bound->m_x_max = PIXELS_TO_TWIPS(m_bitmap_info->get_width());
		bound->m_y_min = 0.0f;
		bound->m_y_max = PIXELS_TO_TWIPS(m_bitmap_info->get_height());
	}

	void	bitmap_character::display(character* ch)
	{
		rect coords;
		coords.m_x_min = 0.0f;
		coords.m_x_max = PIXELS_TO_TWIPS(m_bitmap_info->get_width());
		coords.m_y_min = 0.0f;
		coords.m_y_max = PIXELS_TO_TWIPS(m_bitmap_info->get_height());

		// show whole picture
		rect uv_coords;
		uv_coords.m_x_min = 0.0f;
		uv_coords.m_x_max = 1.0f;
		uv_coords.m_y_min = 0.0f;
		uv_coords.m_y_max = 1.0f;

		cxform cx = ch->get_world_cxform();
		rgba color = cx.transform(gameswf::rgba());
		matrix m = ch->get_world_matrix();
		render::draw_bitmap(m, m_bitmap_info.get_ptr(), coords,	uv_coords, color);
	}

	gameswf::bitmap_info*	bitmap_character::get_bitmap_info()
	{
		return m_bitmap_info.get_ptr();
	}

}

gameswf::place_object_2* gameswf::place_object_2::s_self = nullptr;
int gameswf::place_object_2::s_myBitmapId = 0;
// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
