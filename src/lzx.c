/*
 * LZX (un)compression functions
 *
 * Copyright (C) 2008-2019, Joachim Metz <joachim.metz@gmail.com>
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <memory.h>
#include <types.h>

#include "assorted_libcerror.h"
#include "assorted_libcnotify.h"
#include "bit_stream.h"
#include "huffman_tree.h"
#include "lzx.h"

#if !defined( LZX_ATTRIBUTE_FALLTHROUGH )
#if defined( __GNUC__ ) && __GNUC__ >= 7
#define LZX_ATTRIBUTE_FALLTHROUGH      __attribute__ ((fallthrough))
#else
#define LZX_ATTRIBUTE_FALLTHROUGH
#endif
#endif

const int32_t lzx_compression_offset_base[ 51 ] = {
	-2, -1, 0, 1, 2, 4, 6, 10, 14, 22, 30, 46, 62, 94, 126, 190,
	254, 382, 510, 766, 1022, 1534, 2046, 3070, 4094, 6142, 8190, 12286, 16382, 24574, 32766, 49150,
	65534, 98302, 131070, 196606, 262142, 393214, 524286, 655358, 786430, 917502, 1048574, 1179646, 1310718, 1441790, 1572862, 1703934,
	1835006, 1966078, 2097150 };

const int32_t lzx_base_position[ 51 ] = {
	0, 1, 2, 3, 4, 6, 8, 12, 16, 24, 32, 48, 64, 96, 128, 192,
	256, 384, 512, 768, 1024, 1536, 2048, 3072, 4096, 6144, 8192, 12288, 16384, 24576, 32768, 49152,
	65536, 98304, 131072, 196608, 262144, 393216, 524288, 655360, 786432, 917504, 1048576, 1179648, 1310720, 1441792, 1572864, 1703936,
	1835008, 1966080, 2097152 };

const uint8_t lzx_number_of_footer_bits[ 50 ] = {
	0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
	7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14,
	15, 15, 16, 16, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
	17, 17, 17 };

/* Reads the Huffman code sizes
 * Returns 1 on success or -1 on error
 */
int lzx_read_huffman_code_sizes(
     bit_stream_t *bit_stream,
     uint8_t *code_size_array,
     int number_of_code_sizes,
     libcerror_error_t **error )
{
	huffman_tree_t *pre_codes_huffman_tree = NULL;
	static char *function                  = "lzx_read_huffman_code_sizes";
	uint32_t symbol                        = 0;
	uint32_t times_to_repeat               = 0;
	uint32_t value_32bit                   = 0;
	int32_t code_size                      = 0;
	uint8_t pre_code_index                 = 0;
	int code_size_index                    = 0;

	if( bit_stream == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid bit-stream.",
		 function );

		return( -1 );
	}
	for( pre_code_index = 0;
	     pre_code_index < 20;
	     pre_code_index++ )
	{
		if( bit_stream_get_value(
		     bit_stream,
		     4,
		     &value_32bit,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve pre-code: %" PRIu8 " size from bit stream.",
			 function,
			 pre_code_index );

			goto on_error;
		}
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: pre-code: % 2" PRIu8 " value\t\t\t\t: %" PRIu32 "\n",
			 function,
			 pre_code_index,
			 value_32bit) ;
		}
		code_size_array[ pre_code_index ] = (uint8_t) value_32bit;
	}
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "\n" );
	}
	if( huffman_tree_initialize(
	     &pre_codes_huffman_tree,
	     20,
	     15,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create pre-codes Huffman tree.",
		 function );

		goto on_error;
	}
	if( huffman_tree_build(
	     pre_codes_huffman_tree,
	     code_size_array,
	     20,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to build pre-codes Huffman tree.",
		 function );

		goto on_error;
	}
	if( memory_set(
	     code_size_array,
	     0,
	     sizeof( uint8_t ) * number_of_code_sizes ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear code size array.",
		 function );

		goto on_error;
	}
	code_size_index = 0;

	while( code_size_index < number_of_code_sizes )
	{
		if( huffman_tree_get_symbol_from_bit_stream(
		     pre_codes_huffman_tree,
		     bit_stream,
		     &symbol,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve symbol from pre-codes Huffman tree.",
			 function );

			goto on_error;
		}
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: code size: % 3" PRIu32 " symbol\t\t\t: %" PRIu32 "\n",
			 function,
			 code_size_index,
			 symbol );
		}
		if( symbol < 17 )
		{
			code_size = code_size_array[ code_size_index ] - symbol;

			if( code_size < 0 )
			{
				code_size += 17;
			}
			if( libcnotify_verbose != 0 )
			{
				libcnotify_printf(
				 "%s: code size: % 3" PRIu32 " value\t\t\t: %" PRIi32 "\n",
				 function,
				 code_size_index,
				 code_size );
			}
			code_size_array[ code_size_index++ ] = (uint8_t) code_size;

			continue;
		}
		code_size = 0;

		if( symbol == 17 )
		{
			if( bit_stream_get_value(
			     bit_stream,
			     4,
			     &times_to_repeat,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
				 "%s: unable to retrieve times to repeat from bit stream.",
				 function );

				goto on_error;
			}
			times_to_repeat += 4;
		}
		else if( symbol == 18 )
		{
			if( bit_stream_get_value(
			     bit_stream,
			     5,
			     &times_to_repeat,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
				 "%s: unable to retrieve times to repeat from bit stream.",
				 function );

				goto on_error;
			}
			times_to_repeat += 20;
		}
		else if( symbol == 19 )
		{
			if( bit_stream_get_value(
			     bit_stream,
			     1,
			     &times_to_repeat,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
				 "%s: unable to retrieve times to repeat from bit stream.",
				 function );

				goto on_error;
			}
			times_to_repeat += 4;

			if( huffman_tree_get_symbol_from_bit_stream(
			     pre_codes_huffman_tree,
			     bit_stream,
			     &symbol,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
				 "%s: unable to retrieve symbol from pre-codes Huffman tree.",
				 function );

				goto on_error;
			}
			if( symbol > 17 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
				 LIBCERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
				 "%s: invalid code size symbol value out of bounds.",
				 function );

				goto on_error;
			}
			code_size = code_size_array[ code_size_index ] - symbol;

			if( code_size < 0 )
			{
				code_size += 17;
			}
		}
		else
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			 LIBCERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
			 "%s: invalid code size symbol value out of bounds.",
			 function );

			goto on_error;
		}
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: times to repeat\t\t\t\t: %" PRIu32 "\n",
			 function,
			 times_to_repeat );
		}
		if( times_to_repeat > (uint32_t) ( number_of_code_sizes - code_size_index ) )
		{
			if( libcnotify_verbose != 0 )
			{
				libcnotify_printf(
				 "%s: times to repeat value out of bounds.\n",
				 function );
			}
			times_to_repeat = (uint32_t) ( number_of_code_sizes - code_size_index );
		}
		while( times_to_repeat > 0 )
		{
			if( libcnotify_verbose != 0 )
			{
				libcnotify_printf(
				 "%s: code size: % 3" PRIu32 " value\t\t\t: %" PRIi32 "\n",
				 function,
				 code_size_index,
				 code_size );
			}
			code_size_array[ code_size_index++ ] = (uint8_t) code_size;

			times_to_repeat--;
		}
	}
	if( huffman_tree_free(
	     &pre_codes_huffman_tree,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
		 "%s: unable to free pre-codes Huffman tree.",
		 function );

		goto on_error;
	}
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "\n" );
	}
	return( 1 );

on_error:
	if( pre_codes_huffman_tree != NULL )
	{
		huffman_tree_free(
		 &pre_codes_huffman_tree,
		 NULL );
	}
	return( -1 );
}

/* Reads and builds the literals and match headers Huffman tree
 * Returns 1 on success or -1 on error
 */
int lzx_build_main_huffman_tree(
     bit_stream_t *bit_stream,
     huffman_tree_t *huffman_tree,
     libcerror_error_t **error )
{
	uint8_t code_size_array[ 256 + 240 ];

	static char *function = "lzx_build_main_huffman_tree";

	if( lzx_read_huffman_code_sizes(
	     bit_stream,
	     code_size_array,
	     256,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to read literals Huffman code sizes.",
		 function );

		return( -1 );
	}
	if( lzx_read_huffman_code_sizes(
	     bit_stream,
	     &( code_size_array[ 256 ] ),
	     240,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to read literals Huffman code sizes.",
		 function );

		return( -1 );
	}
	if( huffman_tree_build(
	     huffman_tree,
	     code_size_array,
	     256 + 240,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to build literals and match headers Huffman tree.",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Reads and builds the lengths Huffman tree
 * Returns 1 on success or -1 on error
 */
int lzx_build_lengths_huffman_tree(
     bit_stream_t *bit_stream,
     huffman_tree_t *huffman_tree,
     libcerror_error_t **error )
{
	uint8_t code_size_array[ 249 ];

	static char *function = "lzx_build_lengths_huffman_tree";

	if( lzx_read_huffman_code_sizes(
	     bit_stream,
	     code_size_array,
	     249,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to read literals Huffman code sizes.",
		 function );

		return( -1 );
	}
	if( huffman_tree_build(
	     huffman_tree,
	     code_size_array,
	     249,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to build lengths Huffman tree.",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Reads and builds the aligned offsets Huffman tree
 * Returns 1 on success or -1 on error
 */
int lzx_build_aligned_offsets_huffman_tree(
     bit_stream_t *bit_stream,
     huffman_tree_t *huffman_tree,
     libcerror_error_t **error )
{
	uint8_t code_size_array[ 256 ];

	static char *function = "lzx_build_aligned_offsets_huffman_tree";
	uint32_t code_size    = 0;
	int code_size_index   = 0;

	for( code_size_index = 0;
	     code_size_index < 256;
	     code_size_index++ )
	{
		if( bit_stream_get_value(
		     bit_stream,
		     3,
		     &code_size,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve value from bit stream.",
			 function );

			return( -1 );
		}
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: code size: % 3d value\t\t\t: %" PRIu32 "\n",
			 function,
			 code_size_index,
			 code_size );
		}
		code_size_array[ code_size_index ] = (uint8_t) code_size;
	}
	if( huffman_tree_build(
	     huffman_tree,
	     code_size_array,
	     256,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to build aligned offsets Huffman tree.",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Decodes a Huffman compressed block
 * Returns 1 on success or -1 on error
 */
int lzx_decode_huffman(
     bit_stream_t *bit_stream,
     uint32_t block_size,
     huffman_tree_t *main_huffman_tree,
     huffman_tree_t *lengths_huffman_tree,
     huffman_tree_t *aligned_offsets_huffman_tree,
     uint32_t *recent_compression_offsets,
     const uint8_t *number_of_footer_bits,
     uint8_t *uncompressed_data,
     size_t uncompressed_data_size,
     size_t *uncompressed_data_offset,
     libcerror_error_t **error )
{
	static char *function            = "lzx_decode_huffman";
	size_t data_end_offset           = 0;
	size_t data_offset               = 0;
	uint32_t aligned_offset          = 0;
	uint32_t compression_offset      = 0;
	uint32_t compression_offset_slot = 0;
	uint32_t compression_size        = 0;
	uint32_t symbol                  = 0;
	uint8_t number_of_bits           = 0;

	if( recent_compression_offsets == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid recent compression offsets.",
		 function );

		return( -1 );
	}
	if( uncompressed_data == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid uncompressed data.",
		 function );

		return( -1 );
	}
	if( uncompressed_data_size > (size_t) SSIZE_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid uncompressed data size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( uncompressed_data_offset == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid uncompressed data offset.",
		 function );

		return( -1 );
	}
	data_offset     = *uncompressed_data_offset;
	data_end_offset = data_offset + block_size;

	while( data_offset < data_end_offset )
	{
		if( huffman_tree_get_symbol_from_bit_stream(
		     main_huffman_tree,
		     bit_stream,
		     &symbol,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve symbol from literals and match headers Huffman tree.",
			 function );

			return( -1 );
		}
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: symbol\t\t\t\t\t\t: %" PRIu32 "\n",
			 function,
			 symbol );
		}
		if( symbol < 256 )
		{
			if( data_offset >= uncompressed_data_size )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
				 LIBCERROR_ARGUMENT_ERROR_VALUE_TOO_SMALL,
				 "%s: invalid uncompressed data value too small.",
				 function );

				return( -1 );
			}
			uncompressed_data[ data_offset++ ] = (uint8_t) symbol;
		}
		else
		{
			compression_size        = symbol % 8;
			compression_offset_slot = ( symbol - 256 ) / 8;

			if( compression_size == 7 )
			{
				if( huffman_tree_get_symbol_from_bit_stream(
				     lengths_huffman_tree,
				     bit_stream,
				     &compression_size,
				     error ) != 1 )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_RUNTIME,
					 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
					 "%s: unable to retrieve symbol from lengths Huffman tree.",
					 function );

					return( -1 );
				}
				compression_size += 7;
			}
			compression_size += 2;

			if( compression_offset_slot < 3 )
			{
				compression_offset = recent_compression_offsets[ compression_offset_slot ];

				recent_compression_offsets[ compression_offset_slot ] = recent_compression_offsets[ 0 ];
			}
			else
			{
				number_of_bits = number_of_footer_bits[ compression_offset_slot ];

				if( bit_stream_get_value(
				     bit_stream,
				     number_of_bits,
				     &compression_offset,
				     error ) != 1 )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_RUNTIME,
					 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
					 "%s: unable to retrieve compression offset from bit stream.",
					 function );

					return( -1 );
				}
				if( ( aligned_offsets_huffman_tree != NULL )
				 && ( compression_offset_slot >= 8 ) )
				{
					if( huffman_tree_get_symbol_from_bit_stream(
					     main_huffman_tree,
					     bit_stream,
					     &aligned_offset,
					     error ) != 1 )
					{
						libcerror_error_set(
						 error,
						 LIBCERROR_ERROR_DOMAIN_RUNTIME,
						 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
						 "%s: unable to retrieve symbol from literals and match headers Huffman tree.",
						 function );

						return( -1 );
					}
					if( libcnotify_verbose != 0 )
					{
						libcnotify_printf(
						 "%s: aligned offset\t\t\t\t\t: %" PRIu32 "\n",
						 function,
						 aligned_offset );
					}
					compression_offset <<= 3;
					compression_offset  |= aligned_offset;
				}
				compression_offset += lzx_compression_offset_base[ compression_offset_slot ];

				recent_compression_offsets[ 2 ] = recent_compression_offsets[ 1 ];
				recent_compression_offsets[ 1 ] = recent_compression_offsets[ 0 ];
			}
			recent_compression_offsets[ 0 ] = compression_offset;

			if( libcnotify_verbose != 0 )
			{
				libcnotify_printf(
				 "%s: compression size\t\t\t\t\t: %" PRIu32 "\n",
				 function,
				 compression_size );

				libcnotify_printf(
				 "%s: compression offset slot\t\t\t\t: %" PRIu32 "\n",
				 function,
				 compression_offset_slot );

				libcnotify_printf(
				 "%s: compression offset\t\t\t\t\t: %" PRIu32 "\n",
				 function,
				 compression_offset );
			}
			if( compression_offset > data_offset )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
				 "%s: invalid compression offset value out of bounds.",
				 function );

				return( -1 );
			}
			if( ( data_offset + compression_size ) > uncompressed_data_size )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
				 LIBCERROR_ARGUMENT_ERROR_VALUE_TOO_SMALL,
				 "%s: invalid uncompressed data value too small.",
				 function );

				return( -1 );
			}
			while( compression_size > 0 )
			{
				uncompressed_data[ data_offset ] = uncompressed_data[ data_offset - compression_offset ];

				data_offset++;
				compression_size--;
			}
		}
	}
	*uncompressed_data_offset = data_offset;

	return( 1 );
}

/* Decompresses LZX compressed data
 * Returns 1 on success or -1 on error
 */
int lzx_decompress(
     const uint8_t *compressed_data,
     size_t compressed_data_size,
     uint8_t *uncompressed_data,
     size_t *uncompressed_data_size,
     libcerror_error_t **error )
{
	uint32_t recent_compression_offsets[ 3 ]     = { 1, 1, 1 };

	bit_stream_t *bit_stream                     = NULL;
	huffman_tree_t *aligned_offsets_huffman_tree = NULL;
	huffman_tree_t *lengths_huffman_tree         = NULL;
	huffman_tree_t *main_huffman_tree            = NULL;
	const uint8_t *number_of_footer_bits         = NULL;
	static char *function                        = "lzx_decompress";
	size_t uncompressed_data_offset              = 0;
	uint32_t block_size                          = 0;
	uint32_t block_type                          = 0;

	if( compressed_data == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid compressed data.",
		 function );

		return( -1 );
	}
	if( compressed_data_size > (size_t) SSIZE_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid compressed data size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( uncompressed_data == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid uncompressed data.",
		 function );

		return( -1 );
	}
	if( uncompressed_data_size == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid uncompressed data size.",
		 function );

		return( -1 );
	}
	if( *uncompressed_data_size > (size_t) SSIZE_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid uncompressed data size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( bit_stream_initialize(
	     &bit_stream,
	     compressed_data,
	     compressed_data_size,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create bit-stream.",
		 function );

		goto on_error;
	}
	bit_stream->storage_type = BIT_STREAM_STORAGE_TYPE_16BIT_LITTLE_ENDIAN;

/* TODO find optimized solution to read bit stream from bytes */
	while( bit_stream->byte_stream_offset < bit_stream->byte_stream_size )
	{
		if( bit_stream_get_value(
		     bit_stream,
		     3,
		     &block_type,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve value from bit stream.",
			 function );

			return( -1 );
		}
		if( bit_stream_get_value(
		     bit_stream,
		     1,
		     &block_size,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve value from bit stream.",
			 function );

			return( -1 );
		}
		if( block_size != 0 )
		{
			block_size = 32768;
		}
		else
		{
			if( bit_stream_get_value(
			     bit_stream,
			     16,
			     &block_size,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
				 "%s: unable to retrieve value from bit stream.",
				 function );

				return( -1 );
			}
/* TODO add extended block size support ? */
		}
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: block header block type\t\t\t\t\t: %" PRIu32 " (",
			 function,
			 block_type );

			switch( block_type )
			{
				case LZX_BLOCK_TYPE_ALIGNED:
					libcnotify_printf(
					 "Aligned" );
					break;

				case LZX_BLOCK_TYPE_VERBATIM:
					libcnotify_printf(
					 "Verbatim" );
					break;

				case LZX_BLOCK_TYPE_UNCOMPRESSED:
					libcnotify_printf(
					 "Uncompressed" );
					break;

				default:
					libcnotify_printf(
					 "Invalid" );
					break;
			}
			libcnotify_printf(
			 ")\n" );

			libcnotify_printf(
			 "%s: block header block size\t\t\t\t\t: %" PRIu32 "\n",
			 function,
			 block_size );

			libcnotify_printf(
			 "\n" );
		}
		number_of_footer_bits = lzx_number_of_footer_bits;

		switch( block_type )
		{
			case LZX_BLOCK_TYPE_ALIGNED:
				if( huffman_tree_initialize(
				     &aligned_offsets_huffman_tree,
				     256,
				     16,
				     error ) != 1 )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_RUNTIME,
					 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
					 "%s: unable to create aligned offsets Huffman tree.",
					 function );

					goto on_error;
				}
				if( lzx_build_aligned_offsets_huffman_tree(
				     bit_stream,
				     aligned_offsets_huffman_tree,
				     error ) != 1 )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_RUNTIME,
					 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
					 "%s: unable to build aligned offsets Huffman tree.",
					 function );

					goto on_error;
				}
/* TODO set number_of_footer_bits */

			LZX_ATTRIBUTE_FALLTHROUGH;
			case LZX_BLOCK_TYPE_VERBATIM:
				if( huffman_tree_initialize(
				     &main_huffman_tree,
				     256 + 240,
				     16,
				     error ) != 1 )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_RUNTIME,
					 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
					 "%s: unable to create literals and match headers Huffman tree.",
					 function );

					goto on_error;
				}
				if( lzx_build_main_huffman_tree(
				     bit_stream,
				     main_huffman_tree,
				     error ) != 1 )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_RUNTIME,
					 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
					 "%s: unable to build literals and match headers Huffman tree.",
					 function );

					goto on_error;
				}
				if( huffman_tree_initialize(
				     &lengths_huffman_tree,
				     249,
				     16,
				     error ) != 1 )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_RUNTIME,
					 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
					 "%s: unable to create lengths Huffman tree.",
					 function );

					goto on_error;
				}
				if( lzx_build_lengths_huffman_tree(
				     bit_stream,
				     lengths_huffman_tree,
				     error ) != 1 )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_RUNTIME,
					 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
					 "%s: unable to build lengths Huffman tree.",
					 function );

					goto on_error;
				}
				if( lzx_decode_huffman(
				     bit_stream,
				     block_size,
				     main_huffman_tree,
				     lengths_huffman_tree,
				     aligned_offsets_huffman_tree,
				     recent_compression_offsets,
				     number_of_footer_bits,
				     uncompressed_data,
				     *uncompressed_data_size,
				     &uncompressed_data_offset,
				     error ) != 1 )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_RUNTIME,
					 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
					 "%s: unable to decode fixed Huffman encoded bit stream.",
					 function );

					goto on_error;
				}
				if( aligned_offsets_huffman_tree != NULL )
				{
					if( huffman_tree_free(
					     &aligned_offsets_huffman_tree,
					     error ) != 1 )
					{
						libcerror_error_set(
						 error,
						 LIBCERROR_ERROR_DOMAIN_RUNTIME,
						 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
						 "%s: unable to free aligned offsets Huffman tree.",
						 function );

						goto on_error;
					}
				}
				if( huffman_tree_free(
				     &lengths_huffman_tree,
				     error ) != 1 )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_RUNTIME,
					 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
					 "%s: unable to free lengths Huffman tree.",
					 function );

					goto on_error;
				}
				if( huffman_tree_free(
				     &main_huffman_tree,
				     error ) != 1 )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_RUNTIME,
					 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
					 "%s: unable to free literals and match headers Huffman tree.",
					 function );

					goto on_error;
				}
				break;

			case LZX_BLOCK_TYPE_UNCOMPRESSED:
/* TODO read recent_compression_offsets */
/* TODO implement */
				break;

			default:
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
				 "%s: unsupported block type.",
				 function );

				return( -1 );
		}
/* TODO implement */
		break;
	}
	if( bit_stream_free(
	     &bit_stream,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
		 "%s: unable to free bit-stream.",
		 function );

		goto on_error;
	}
	*uncompressed_data_size = uncompressed_data_offset;

	return( 1 );

on_error:
	if( lengths_huffman_tree != NULL )
	{
		huffman_tree_free(
		 &lengths_huffman_tree,
		 NULL );
	}
	if( main_huffman_tree != NULL )
	{
		huffman_tree_free(
		 &main_huffman_tree,
		 NULL );
	}
	if( aligned_offsets_huffman_tree != NULL )
	{
		huffman_tree_free(
		 &aligned_offsets_huffman_tree,
		 NULL );
	}
	if( bit_stream != NULL )
	{
		bit_stream_free(
		 &bit_stream,
		 NULL );
	}
	return( -1 );
}

