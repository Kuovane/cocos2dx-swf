// gameswf_movie_def.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Some implementation for SWF player.

// Useful links:
//
// http://sswf.sourceforge.net/SWFalexref.html
// http://www.openswf.org

// @@ Need to break this file into pieces

#include "cocos2d.h"
#include "tu_file.h"
#include "gameswf_font.h"
#include "gameswf_sound.h"
#include "gameswf_stream.h"
#include "gameswf_fontlib.h"
//#include "jpeg.h"
#include "zlib_adapter.h"
//#include "XGMacros.h"

#include "image/stb_image_write.h"
#include "image/stb_image_resize.h"
#include <unordered_map>


#if TU_CONFIG_LINK_TO_ZLIB
#include <zlib.h>
#endif // TU_CONFIG_LINK_TO_ZLIB
#include "VxConvert.h"
#include "VxFile.h"

namespace gameswf
{

	// it's running in loader thread
	void movie_def_loader(void* arg)
	{
		movie_def_impl* m = (movie_def_impl*) arg;
		m->read_tags();
	}

	// Keep a table of loader functions for the different tag types.

	static std::unordered_map <int, loader_function>	s_tag_loaders;

	void clears_tag_loaders()
	{
		s_tag_loaders.clear();
	}

	bool get_tag_loader(int tag_type, loader_function* lf)
	{
		*lf = s_tag_loaders[tag_type];//() lf);
		return true;
	}

	void	register_tag_loader(int tag_type, loader_function lf)
	// Associate the specified tag type with the given tag loader
	// function.
	{
		//assert(s_tag_loaders.get(tag_type, NULL) == false);
		//assert(lf != NULL);

		s_tag_loaders[tag_type] = lf;
	}

	static void	dump_tag_bytes(stream* in)
	// Log the contents of the current tag, in hex.
	{
		static const int	ROW_BYTES = 16;
		char	row_buf[ROW_BYTES];
		int	row_count = 0;

		while(in->get_position() < in->get_tag_end_position())
		{
			int	c = in->read_u8();
			log_msg("%02X", c);

			if (c < 32) c = '.';
			if (c > 127) c = '.';
			row_buf[row_count] = c;

			row_count++;
			if (row_count >= ROW_BYTES)
			{
				log_msg("    ");
				for (int i = 0; i < ROW_BYTES; i++)
				{
					log_msg("%c", row_buf[i]);
				}

				log_msg("\n");
				row_count = 0;
			}
			else
			{
				log_msg(" ");
			}
		}

		if (row_count > 0)
		{
			log_msg("\n");
		}
	}

	movie_def_impl::movie_def_impl(player* player, 
		create_bitmaps_flag cbf, create_font_shapes_flag cfs)	:
		movie_definition_sub(player),
		m_create_bitmaps(cbf),
		m_create_font_shapes(cfs),
		m_frame_rate(30.0f),
		m_version(0),
		m_loaded_length(0),
		m_jpeg_in(0),
		m_str(NULL),
		m_file_end_pos(0),
		m_zlib_in(NULL),
		m_origin_in(NULL)
	{
	}

	movie_def_impl::~movie_def_impl()
	{
		break_loading();
		if (m_thread != NULL)
		{
			m_thread->wait();
		}

		// Release our playlist data.
		{for (int i = 0, n = m_playlist.size(); i < n; i++)
		{
			for (int j = 0, m = m_playlist[i].size(); j < m; j++)
			{
				delete m_playlist[i][j];
			}
		}}

		// Release init action data.
		{for (int i = 0, n = m_init_action_list.size(); i < n; i++)
		{
			for (int j = 0, m = m_init_action_list[i].size(); j < m; j++)
			{
				delete m_init_action_list[i][j];
			}
		}}

		assert(m_jpeg_in == NULL);	// It's supposed to be cleaned up in read()
		closeFiles();
	}

	// ...
	float	movie_def_impl::get_frame_rate() const { return m_frame_rate; }
	void	movie_def_impl::set_frame_rate(float rate) { m_frame_rate = rate; }
	float	movie_def_impl::get_width_pixels() const { return ceilf(TWIPS_TO_PIXELS(m_frame_size.width())); }
	float	movie_def_impl::get_height_pixels() const { return ceilf(TWIPS_TO_PIXELS(m_frame_size.height())); }

	int	movie_def_impl::get_version() const { return m_version; }
	uint32	movie_def_impl::get_file_bytes() const { return m_file_end_pos; }
	uint32	movie_def_impl::get_loaded_bytes() const { return m_loaded_length; }

	create_bitmaps_flag	movie_def_impl::get_create_bitmaps() const
	// Returns DO_CREATE_BITMAPS if we're supposed to
	// initialize our bitmap infos, or DO_NOT_INIT_BITMAPS
	// if we're supposed to create blank placeholder
	// bitmaps (to be init'd later explicitly by the host
	// program).
	{
		return m_create_bitmaps;
	}

	create_font_shapes_flag	movie_def_impl::get_create_font_shapes() const
		// Returns DO_LOAD_FONT_SHAPES if we're supposed to
		// initialize our font shape info, or
		// DO_NOT_LOAD_FONT_SHAPES if we're supposed to not
		// create any (vector) font glyph shapes, and instead
		// rely on precached textured fonts glyphs.
	{
		return m_create_font_shapes;
	}

	void	movie_def_impl::add_bitmap_info(bitmap_info* bi)
	// All bitmap_info's used by this movie should be
	// registered with this API.
	{
		m_bitmap_list.push_back(bi);
	}


	int	movie_def_impl::get_bitmap_info_count() const { return m_bitmap_list.size(); }
	bitmap_info*	movie_def_impl::get_bitmap_info(int i) const
	{
		return m_bitmap_list[i].get_ptr();
	}

	void	movie_def_impl::export_resource(const tu_string& symbol, character_def* res)
	// Expose one of our resources under the given symbol,
	// for export.	Other movies can import it.
	{
		// SWF sometimes exports the same thing more than once!
		m_exports.set(symbol, res);
	}

	character_def*	movie_def_impl::get_exported_resource(const tu_string& symbol)
	// Get the named exported resource, if we expose it.
	// Otherwise return NULL.
	{
		gc_ptr<character_def>	res;
		m_exports.get(symbol, &res);
		return res.get_ptr();
	}

	void	movie_def_impl::add_import(const tu_string& source_url, int id, const tu_string& symbol)
	// Adds an entry to a table of resources that need to
	// be imported from other movies.  Client code must
	// call resolve_import() later, when the source movie
	// has been loaded, so that the actual resource can be
	// used.
	{
		assert(in_import_table(id) == false);

		m_imports.push_back(import_info(source_url, id, symbol));
	}

	bool	movie_def_impl::in_import_table(int character_id)
	// Debug helper; returns true if the given
	// character_id is listed in the import table.
	{
		for (int i = 0, n = m_imports.size(); i < n; i++)
		{
			if (m_imports[i].m_character_id == character_id)
			{
				return true;
			}
		}
		return false;
	}

	void	movie_def_impl::visit_imported_movies(import_visitor* visitor)
	// Calls back the visitor for each movie that we
	// import symbols from.
	{
		stringi_hash<bool>	visited;	// ugh!

		for (int i = 0, n = m_imports.size(); i < n; i++)
		{
			import_info&	inf = m_imports[i];
			if (visited.find(inf.m_source_url) == visited.end())
			{
				// Call back the visitor.
				visitor->visit(inf.m_source_url.c_str());
				visited.set(inf.m_source_url, true);
			}
		}
	}

	void	movie_def_impl::add_abc(tu_string& name, abc_def* abc)
	{
		assert(m_abc == NULL);
		m_abc = abc;
		m_abc_name = name;
	}

	void	movie_def_impl::add_symbol_class(int character_id, const tu_string& class_name)
	{
		m_symbol_class.add(character_id, class_name);
	};

	void	movie_def_impl::add_scene(int scene, const tu_string& frame_name)
	{
		m_scene.add(scene, frame_name);
	};

	void	movie_def_impl::add_frame_label(int frame, const tu_string& frame_name)
	{
		m_frame_label.add(frame, frame_name);
	};

	void	movie_def_impl::add_character(int character_id, character_def* c)
	{
		assert(c);
		m_characters.add(character_id, c);
	}

	character_def*	movie_def_impl::get_character_def(int character_id)
	{
#ifndef NDEBUG
		// make sure character_id is resolved
		if (in_import_table(character_id))
		{
			log_error("get_character_def(): character_id %d is still waiting to be imported\n",
				character_id);
		}
#endif // not NDEBUG

		gc_ptr<character_def>	ch;
		m_characters.get(character_id, &ch);
		assert(ch == NULL || gc_collector::debug_get_ref_count(ch) > 1);
		return ch.get_ptr();
	}

	bool	movie_def_impl::get_labeled_frame(const char* label, int* frame_number)
		// Returns 0-based frame #
	{
		return m_named_frames.get(label, frame_number);
	}

	void	movie_def_impl::add_font(int font_id, font* f)
	{
		assert(f);
		m_fonts.add(font_id, f);
	}

	font*	movie_def_impl::get_font(int font_id)
	{
#ifndef NDEBUG
		// make sure font_id is resolved
		if (in_import_table(font_id))
		{
			log_error("get_font(): font_id %d is still waiting to be imported\n",
				font_id);
		}
#endif // not NDEBUG

		gc_ptr<font>	f;
		m_fonts.get(font_id, &f);
		assert(f == NULL || gc_collector::debug_get_ref_count(f) > 1);
		return f.get_ptr();
	}

	font*	movie_def_impl::find_font(const char* name) const
	{
		for (hash<int, gc_ptr<font> >::const_iterator it = m_fonts.begin(); it != m_fonts.end(); ++it)
		{
			if (it->second->get_name() == name)
			{
				return it->second.get_ptr();
			}
		}
		return NULL;
	}

	bitmap_character_def*	movie_def_impl::get_bitmap_character(int character_id)
	{
		gc_ptr<bitmap_character_def>	ch;
		m_bitmap_characters.get(character_id, &ch);
		assert(ch == NULL || gc_collector::debug_get_ref_count(ch) > 1);
		return ch.get_ptr();
	}

	void	movie_def_impl::add_bitmap_character(int character_id, bitmap_character_def* ch)
	{
		assert(ch);
		m_bitmap_characters.add(character_id, ch);

		add_bitmap_info(ch->get_bitmap_info());
	}

	sound_sample*	movie_def_impl::get_sound_sample(int character_id)
	{
		gc_ptr<sound_sample>	ch;
		m_sound_samples.get(character_id, &ch);
		assert(ch == NULL || gc_collector::debug_get_ref_count(ch) > 1);
		return ch.get_ptr();
	}

	void	movie_def_impl::add_sound_sample(int character_id, sound_sample* sam)
	{
		assert(sam);
		m_sound_samples.add(character_id, sam);
	}

	void	movie_def_impl::add_execute_tag(execute_tag* e)
	{
		assert(e);
		m_playlist[get_loading_frame()].push_back(e);
	}

	void	movie_def_impl::add_init_action(int sprite_id, execute_tag* e)
	// Need to execute the given tag before entering the
	// currently-loading frame for the first time.
	//
	// @@ AFAIK, the sprite_id is totally pointless -- correct?
	{
		assert(e);
		m_init_action_list[get_loading_frame()].push_back(e);
	}

	void	movie_def_impl::add_frame_name(const char* name)
	// Labels the frame currently being loaded with the
	// given name.	A copy of the name string is made and
	// kept in this object.
	{
		tu_string	n = name;
		assert(m_named_frames.get(n, NULL) == false);	// frame should not already have a name (?)
		m_named_frames.add(n, get_loading_frame());	// stores 0-based frame #
	}

	void	movie_def_impl::set_jpeg_loader(jpeg::input* j_in)
	// Set an input object for later loading DefineBits
	// images (JPEG images without the table info).
	{
		assert(m_jpeg_in == NULL);
		m_jpeg_in = j_in;
	}

	jpeg::input*	movie_def_impl::get_jpeg_loader()
	// Get the jpeg input loader, to load a DefineBits
	// image (one without table info).
	{
		return m_jpeg_in;
	}


	const array<execute_tag*>&	movie_def_impl::get_playlist(int frame_number) { return m_playlist[frame_number]; }

	const array<execute_tag*>*	movie_def_impl::get_init_actions(int frame_number)
	{
		return &m_init_action_list[frame_number];
	}

	void	movie_def_impl::read(tu_file* in)
	// Read a .SWF movie.
	{
		m_origin_in = in;
		Uint32	file_start_pos = in->get_position();
		Uint32	header = in->read_le32();
		Uint32	file_length = in->read_le32();
		m_file_end_pos = file_start_pos + file_length;

		m_version = (header >> 24) & 255;
		if ((header & 0x0FFFFFF) != 0x00535746
			&& (header & 0x0FFFFFF) != 0x00535743)
		{
			// ERROR
			log_error("gameswf::movie_def_impl::read() -- file does not start with a SWF header!\n");
			return;
		}
		bool	compressed = (header & 255) == 'C';

		IF_VERBOSE_PARSE(log_msg("version = %d, file_length = %d\n", m_version, file_length));

		m_zlib_in = NULL;
		if (compressed)
		{
#if TU_CONFIG_LINK_TO_ZLIB == 0
			log_error("movie_def_impl::read(): unable to read zipped SWF data; TU_CONFIG_LINK_TO_ZLIB is 0\n");
			return;
#endif

			IF_VERBOSE_PARSE(log_msg("file is compressed.\n"));

			// Uncompress the input as we read it.
			in = zlib_adapter::make_inflater(in);
			m_zlib_in = in;

			// Subtract the size of the 8-byte header, since
			// it's not included in the compressed
			// stream length.
			m_file_end_pos = file_length - 8;
		}

		m_str = new stream(in);

		m_frame_size.read(m_str);
		m_frame_rate = m_str->read_u16() / 256.0f;

		set_frame_count(m_str->read_u16());

		m_playlist.resize(get_frame_count());
		m_init_action_list.resize(get_frame_count());

		IF_VERBOSE_PARSE(m_frame_size.print());
		IF_VERBOSE_PARSE(log_msg("frame rate = %f, frames = %d\n", m_frame_rate, get_frame_count()));

		if (get_player()->use_separate_thread())
		{
			// if you want to use multithread movie loader
			m_thread = new tu_thread(movie_def_loader, this);
		}
		else
		{
			// if you does not want to use multithread movie loader
			read_tags();
		}
	}

	// is running in loader thread
	void	movie_def_impl::read_tags()
	{

		while ((Uint32) m_str->get_position() < m_file_end_pos && get_break_loading() == false)
		{
			int	tag_type = m_str->open_tag();
			loader_function	lf = NULL;
			//IF_VERBOSE_PARSE(log_msg("tag_type = %d\n", tag_type));
			if (tag_type == 1)
			{
				// show frame tag -- advance to the next frame.
				IF_VERBOSE_PARSE(log_msg("  show_frame\n"));
				inc_loading_frame();
			}
			else
			{
				auto it = s_tag_loaders.find(tag_type);
				if (it != s_tag_loaders.end())
					//if (s_tag_loaders.get(tag_type, &lf))
				{
					lf = it->second;
					// call the tag loader.	 The tag loader should add
					// characters or tags to the movie data structure.
					(*lf)(m_str, tag_type, this);

				}
				else
				{
					// no tag loader for this tag type.
					log_msg("*** no tag loader for type %d\n", tag_type);
					IF_VERBOSE_PARSE(dump_tag_bytes(m_str));
				}

			}
		
		
			m_str->close_tag();

			if (tag_type == 0)
			{
				if ((unsigned int) m_str->get_position() != m_file_end_pos)
				{
					// Safety break, so we don't read past the end of the
					// movie.
					log_msg("warning: hit stream-end tag, but not at the "
						"end of the file yet; stopping for safety\n");
					break;
				}
			}

			m_loaded_length = m_str->get_position();
		}

		if (m_jpeg_in)
		{
			delete m_jpeg_in;
			m_jpeg_in = NULL;
		}

		if (m_zlib_in)
		{
			// Done with the zlib_adapter.
			delete m_zlib_in;
		}

		delete m_str;
		delete m_origin_in;
	}


	void	movie_def_impl::get_owned_fonts(array<font*>* fonts)
	// Fill up *fonts with fonts that we own.
	{
		assert(fonts);
		fonts->resize(0);

		array<int>	font_ids;

		for (hash<int, gc_ptr<font> >::iterator it = m_fonts.begin();
			it != m_fonts.end();
			++it)
		{
			font*	f = it->second.get_ptr();
			if (f->get_owning_movie() == this)
			{
				// Sort by character id, so the ordering is
				// consistent for cache read/write.
				int	id = it->first;

				// Insert in correct place.
				int	insert;
				for (insert = 0; insert < font_ids.size(); insert++)
				{
					if (font_ids[insert] > id)
					{
						// We want to insert here.
						break;
					}
				}
				fonts->insert(insert, f);
				font_ids.insert(insert, id);
			}
		}
	}

	// Increment this when the cache data format changes.
	#define CACHE_FILE_VERSION 6

	void	movie_def_impl::output_cached_data(tu_file* out, const cache_options& options)
	// Dump our cached data into the given stream.
	{
		// Write a little header.
		char	header[5];
		strcpy(header, "gscX");
		header[3] = CACHE_FILE_VERSION;
		compiler_assert(CACHE_FILE_VERSION < 256);

		out->write_bytes(header, 4);

		// Write font data.
//		array<font*>	fonts;
//		get_owned_fonts(&fonts);
//		fontlib::output_cached_data(out, fonts, this, options);

		// Write character data.
		{for (hash<int, gc_ptr<character_def> >::iterator it = m_characters.begin();
		it != m_characters.end();
		++it)
		{
			out->write_le16(it->first);
			it->second->output_cached_data(out, options);
		}}

		out->write_le16(static_cast<uint16>(-1));	// end of characters marker
	}

	void	movie_def_impl::input_cached_data(tu_file* in)
	// Read in cached data and use it to prime our loaded characters.
	{
		// Read the header & check version.
		unsigned char	header[4];
		in->read_bytes(header, 4);
		if (header[0] != 'g' || header[1] != 's' || header[2] != 'c')
		{
			log_error("cache file does not have the correct format; skipping\n");
			return;
		}
		else if (header[3] != CACHE_FILE_VERSION)
		{
			log_error(
				"cached data is version %d, but we require version %d; skipping\n",
				int(header[3]), CACHE_FILE_VERSION);
			return;
		}

		// Read the cached font data.
//		array<font*>	fonts;
//		get_owned_fonts(&fonts);
//		fontlib::input_cached_data(in, fonts, this);

		// Read the cached character data.
		for (;;)
		{
			if (in->get_error() != TU_FILE_NO_ERROR)
			{
				log_error("error reading cache file (characters); skipping\n");
				return;
			}
			if (in->get_eof())
			{
				log_error("unexpected eof reading cache file (characters); skipping\n");
				return;
			}

			Sint16	id = in->read_le16();
			if (id == (Sint16) -1) { break; }	// done

			gc_ptr<character_def> ch;
			m_characters.get(id, &ch);
			if (ch != NULL)
			{
				ch->input_cached_data(in);
			}
			else
			{
				log_error("sync error in cache file (reading characters)!  "
					"Skipping rest of cache data.\n");
				return;
			}
		}
	}

	// flash9
	as_function* movie_def_impl::instanciate_class(character* ch) const
	{
		// If the character ID is zero, the class is associated with the main timeline 
		// -1 means _root
		int symbol_id = ch->get_id();
		if (symbol_id == -1)
		{
			symbol_id = 0;
		}

		tu_string class_name;
		if (m_symbol_class.get(symbol_id, &class_name))
		{
			assert(m_abc != NULL);

			instance_info* ii = m_abc->get_instance_info(class_name);

			ch->set_instance( ii );

			// create traits
			for (int i = 0; i < ii->m_trait.size(); i++)
			{
				traits_info* ti = ii->m_trait[i].get();
				const char* name = m_abc->get_multiname(ti->m_name);
				as_value val;
				switch (ti->m_kind)
				{
					case traits_info::Trait_Const:
					case traits_info::Trait_Slot:
						//TODO
						val = 0;
						break;

					case traits_info::Trait_Getter:
					case traits_info::Trait_Setter:
					case traits_info::Trait_Method:
					{
						int index = ti->trait_method.m_method;
						val.set_as_object(m_abc->m_method[index].get());
						break;
					}

					case traits_info::Trait_Class:
					case traits_info::Trait_Function:
						assert(0&&"todo");
						break;

					default:
						assert(0);
				}

				ch->set_member(name, val);
			}

			// get function3 from abc
			return m_abc->get_class_constructor(class_name);
		}
		return NULL;
	}



	void movie_def_impl::initFiles(std::string sOutPath, std::string sFileName)
	{
		m_sAssetsPath = sOutPath + "assets" + "/";
		cocos2d::FileUtils::getInstance()->removeDirectory(m_sAssetsPath);
		if (!m_writeBitmapFunc)
		{
			VxFile::createDirEx(m_sAssetsPath.c_str());
		}
		


		int index = sFileName.rfind("/");
		sFileName = sFileName.substr(index+1);


		{
			m_sFileName = sFileName;
			VxConvert::replace_str(m_sFileName, ".swf", "_");
		}

		sFileName = VxConvert::replace_str(sFileName, ".", "_");
	
		sFileName = sOutPath + sFileName;

		std::string info = sFileName + "_info.lua";
		m_pInfoFile = fopen(info.c_str(), "w");
	}

	void  movie_def_impl::closeFiles()
	{
		if (m_pInfoFile)
		{
			fclose(m_pInfoFile);
		}

	}

	void movie_def_impl::writeStrToFile(std::string sStr)
	{ 
		if (m_pInfoFile)
		{
			fwrite(sStr.c_str(), sStr.length(), 1, m_pInfoFile);
		}
		
	}

	void movie_def_impl::removeDepth(int depth, int id)
	{
		auto ite = s_sRects.find(depth);
		if (ite != s_sRects.end())
		{
			//s_sRects.erase(ite);
		}

	}

	void movie_def_impl::pushMovie(int depth,int characterId)
	{
		s_movieClips[depth] = characterId;
	}

	void movie_def_impl::popMovie(int depth,int characterId)
	{
	}

	std::string  movie_def_impl::getFileName( int id)
	{
		auto fileName = m_sAssetsPath + m_sFileName + VxConvert::integerToString(id) + ".png";
		return fileName;
	}

	void movie_def_impl::writeImageData(unsigned char* pData, std::string fileName, int w, int h, bool flipY, bool bNeedFree )
	{
		if (flipY)
		{
			int count = w * 4;
			unsigned char* pt = (unsigned char* )malloc(count);
			
			for (int i = 0,j = h-1; i < j; ++i,--j)
			{
				unsigned char* ps = &pData[i*count];
				unsigned char* pd = &pData[j*count];
				memcpy(pt, ps, count);
				memcpy(ps, pd, count);
				memcpy(pd, pt, count);
			}

			free(pt);
			m_writeBitmapFunc(pData, fileName, w, h, bNeedFree);
		}
		else
		{
			m_writeBitmapFunc(pData, fileName, w, h, bNeedFree);
		}
	}

	void  movie_def_impl::writeBitmapData(gameswf::bitmap_info* bitmap, int id,int w,int h)
	{
		if (m_writeBitmapFunc)
		{
			if (4 == bitmap->get_bpp())
			{
				auto fileName = getFileName(id);

				int bw = bitmap->get_width();
				int bh = bitmap->get_height();
				if (bw == w && bh == h)
				{

					m_writeBitmapFunc(bitmap->get_data(), fileName, w, h, false);
				}
				else
				{
					stbi_flip_vertically_on_write(0);
					unsigned char* output_data = (unsigned char*)malloc(w * h * 4);
					stbir_resize_uint8(bitmap->get_data(), bw, bh, 0, output_data, w, h, 0, 4);
					m_writeBitmapFunc(output_data, fileName, w, h, true);
					//stbi_write_png(fileName.c_str(), w, h, 4, output_data, 0);
					//free(output_data);
				}
			}
		}
		else
		{
			stbi_flip_vertically_on_write(0);
			auto fileName = getFileName(id);
			//fwrite(sStr.c_str(), sStr.length(), 1, m_pInfoFile);
			if (4 == bitmap->get_bpp())
			{

				int bw = bitmap->get_width();
				int bh = bitmap->get_height();
				if (bw == w && bh == h)
				{
					stbi_write_png(fileName.c_str(), w, h, 4, bitmap->get_data(), 0);
				}
				else
				{
					unsigned char* output_data = (unsigned char*)malloc(w * h * 4);
					stbir_resize_uint8(bitmap->get_data(), bw, bh, 0, output_data, w, h, 0, 4);
					stbi_write_png(fileName.c_str(), w, h, 4, output_data, 0);
					free(output_data);
				}



			}
			else if (3 == bitmap->get_bpp())
			{
				stbi_write_png(fileName.c_str(), bitmap->get_width(), bitmap->get_height(), 3, bitmap->get_data(), 0);
			}
		}
		
		
	}

	void movie_def_impl::resetDraw()
	{
		//m_sRender.reset();
		s_sRects.clear();

		for (auto it = s_sChBitmapInfos.begin(); it != s_sChBitmapInfos.end(); it++)
		{
			//delete it->second;
		}
		s_sBitmapInfos.clear();
		s_sChBitmapInfos.clear();
		m_bitmapId = 1;
	}

	static char pBuf[512] = { 0 };
	void movie_def_impl::drawBegin(std::string sOutPath, std::string fileName)
	{
		

		initFiles(sOutPath, fileName);
		auto size = m_playlist.size();

		//	rect	m_frame_size;
			//float	m_frame_rate;
		memset(pBuf, 0, 512);
		sprintf(pBuf, "local info = {\n[\"fps\"]=%g,\n[\"fnum\"]=%d,\n[\"width\"]=%g,\n[\"height\"]=%g,\n[\"frames\"]={\n", m_frame_rate, size, TWIPS_TO_PIXELS(m_frame_size.width()), TWIPS_TO_PIXELS(m_frame_size.height()));
		writeStrToFile(pBuf);
	}

	bool movie_def_impl::drawFrame(int curFrame)
	{

		auto i = curFrame;
		auto size = m_playlist.size();
		if (i < size)
		{
			auto frames = m_playlist[i];
			auto count = frames.size();
			writeStrToFile("    {");
			for (int j = 0; j < count; ++j)
			{
				frames[j]->print(this);
			}
			writeStrToFile("},\n");

			return true;
		}
		return false;

	}

	void movie_def_impl::drawEnd()
	{
		//char pBuf[512] = { 0 };
		memset(pBuf, 0, 512);
		//sprintf(pBuf, "}", m_frame_rate, size, TWIPS_TO_PIXELS(m_frame_size.width()), TWIPS_TO_PIXELS(m_frame_size.height()));
		writeStrToFile("},\n");


		std::string sTemp = m_sFileName.substr(0, m_sFileName.length() - 1);
		//XGConvert::replace_str(sTemp, "_", "");
		memset(pBuf, 0, 512);
		sprintf(pBuf, "\n[\"imageName\"]=\"%s\",\n", sTemp.c_str());
		writeStrToFile(pBuf);


		writeStrToFile("}\n");

		writeStrToFile("return info");

		closeFiles();
	}

	void movie_def_impl::drawMovieClip(bool hasMatrix, matrix& m, bool hasCxform, cxform& cx, int depth, int clip_depth)
	{
		//m_sRender.drawMovieClip( hasMatrix, m, hasCxform, cx,  depth, clip_depth);
	}

	void movie_def_impl::drawBitmap(gameswf::bitmap_info* pBitMapInfo, bool hasMatrix, matrix &m, bool hasCxform, cxform &cx, rect& sRect, int depth, int clip_depth)
	{
		//m_sRender.drawBitmap(pBitMapInfo, hasMatrix, m, hasCxform, cx, sRect, depth, clip_depth);
	}
		
	std::map<int, bitmap_info*>& movie_def_impl::getBitmapInfos()
	{
		return s_sBitmapInfos; 
	}
	std::map<int, bitmap_info*>& movie_def_impl::getCHBitmapInfos()
	{ 
		return s_sChBitmapInfos; 
	}

	int movie_def_impl::getMovieClipByDepth(int nDepth)
	{
		 auto it = s_movieClips.find(nDepth);
		 if (it != s_movieClips.end())
		 {
			return it->first;
		 }
		 return  -1;
	}

	void movie_def_impl::saveToFiles(std::string sOutPath,std::string fileName)
	{

		//static char pBuf[512] = { 0 };

		initFiles(sOutPath,fileName);
		auto size = m_playlist.size();

	//	rect	m_frame_size;
		//float	m_frame_rate;
		memset(pBuf, 0, 512);
		sprintf(pBuf, "local info = {\n[\"fps\"]=%g,\n[\"fnum\"]=%d,\n[\"width\"]=%g,\n[\"height\"]=%g,\n[\"frames\"]={\n", m_frame_rate, size, TWIPS_TO_PIXELS(m_frame_size.width()), TWIPS_TO_PIXELS(m_frame_size.height()));
		writeStrToFile(pBuf);

		for (int i = 0; i < size; ++i)
		{
			auto frames = m_playlist[i];
			auto count = frames.size();
			writeStrToFile("    {");
			for (int j = 0; j < count; ++j)
			{
				frames[j]->print(this);
			}
			writeStrToFile("},\n");
		}

		memset(pBuf, 0, 512);
		//sprintf(pBuf, "}", m_frame_rate, size, TWIPS_TO_PIXELS(m_frame_size.width()), TWIPS_TO_PIXELS(m_frame_size.height()));
		writeStrToFile("},\n");


		std::string sTemp = m_sFileName.substr(0, m_sFileName.length()-1);
		//XGConvert::replace_str(sTemp, "_", "");
		memset(pBuf, 0, 512);
		sprintf(pBuf, "\n[\"imageName\"]=\"%s\",\n", sTemp.c_str());
		writeStrToFile(pBuf);


		writeStrToFile("}\n");

		writeStrToFile("return info");

		closeFiles();
	}


}
