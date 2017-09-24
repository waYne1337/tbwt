/*
 * tunneling-enc-support.cpp for bwt tunneling
 * Copyright (c) 2017 Uwe Baier All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "tunnel-enc-support.hpp"

#include "aux-encoding.hpp"
#include "block-compressor.hpp"
#include "bwt-config.hpp"
#include "entropy-coder.hpp"
#include "mtf-coder.hpp"
#include "rle0-coder.hpp"

#include <array>
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <stdexcept>
#include <limits>

using namespace std;

//// BLOCK SCORING ////////////////////////////////////////////////////////////

//returns position of highest set bit in x, starting at zero [undefined value if x is zero]
inline t_size_t hibit(t_size_t x) {
	//TODO: offer an alternative if __builtin_clz is not available
	return numeric_limits<t_size_t>::digits - __builtin_clz(x) - 1;
}

void tunnel_enc_support::init_enc_information( const bwt_run_support &bwtrs ) {
	runs = bwtrs.runs;
	runsn1 = 0;
	tc = 0;
	t = 0;

	//compute the number of run characters
	rc = 0;
	for (t_idx_t i = 0; i < runs; i++) {
		rc += hibit(bwtrs.height(i));
		if (bwtrs.height(i) > 1u)
			++runsn1;
	}
	//compute numerator for the average bit size of an RL-character
	// (denumerator is 2^FLT_MANT_DIG)
	avg_rlc_bits_numer = (t_bitsize_t)( 
	                     (1.0f + log2( 1.0f + runs / (float)rc ) )
	                     * (1ul << FLT_MANT_DIG) );
	//create width and brc, each initialized to zero
	width.resize( runs );
	brc.resize( runs );
}

void tunnel_enc_support::add_block_column( const bwt_run_support &bwtrs, t_idx_t b, t_idx_t b_col ) {
	width[b]++;
	brc[b] += hibit( bwtrs.height(b_col) ) - hibit( bwtrs.height(b_col) - bwtrs.height(b) + 1 );
}

t_bitsize_t tunnel_enc_support::blockscore( t_idx_t b ) const {
	return ((brc[b] - brc[b] * 2 / width[b])       //brc[b] - symbols in first and last column
	        * avg_rlc_bits_numer) >> FLT_MANT_DIG; // * log(n / rc) bits
}

void tunnel_enc_support::tunnel_block_symbolic( t_idx_t b ) {
	t++;
	tc += brc[b] - brc[b]*2/width[b];
}

void tunnel_enc_support::reduce_score_of_inner_block( t_idx_t b, t_idx_t ic_b ) {
	
	//amount of information removed by block b on the shortened width of ic_b
	auto ic_ocb_rc = brc[b] * width[ic_b] / width[b]; 

	//height-reduce inner block
	brc[ic_b] = (brc[ic_b] > 0u && ic_ocb_rc > brc[ic_b])
	          ?  ic_ocb_rc - brc[ic_b] //remaining information
	          :  0; //clear blockscore, not enough benefit
}

void tunnel_enc_support::reduce_score_of_outer_block( t_idx_t b, t_idx_t oc_b ) {
	//weight-reduce inner block
	auto oc_b_nw = width[oc_b] - width[b]; //new width of block oc_b

	//linearly reduce width
	brc[oc_b] = brc[oc_b] * oc_b_nw / width[oc_b];

	//adapt weight of block
	width[oc_b] = oc_b_nw;
}

t_bitsize_t tunnel_enc_support::current_rlc_encoding_size() const {
	return (((rc - tc) * avg_rlc_bits_numer) >> FLT_MANT_DIG)      // (rc-tc) * log(n / rc)
	       + current_aux_enc_size(); //encoding of aux
}

t_bitsize_t tunnel_enc_support::current_aux_enc_size() const {
	// 2*t*( log( runsn1 / 2*t ) - 1) * 1,5 bit (RL-Characters)
	//  + 2*t * 3 bit (non-zero entries, 2*t each of 3 bit)
	return 3 * t * (hibit(runsn1) - hibit(t));
}

//// AUX ENCODING AND DECODING ////////////////////////////////////////////////////

void encode_aux( const t_string_t &tbwt_mtf, t_idx_t tbwt_idx, twobitvector &&aux,
                 entropy_encoder<ostream> &entcoder ) {

	assert( aux_encoding::REG == 0 );
	//transfer aux to a run-based representation
	t_idx_t j = 0;
	array<t_idx_t,3> bounds = {(t_idx_t)0u,
	                           tbwt_idx,
	                           (t_idx_t)tbwt_mtf.size()}; //don't forget primary index run
	for (t_idx_t ib = 0; ib != 2; ib++) {
		t_idx_t i = bounds[ib];
		t_idx_t e = bounds[ib+1];

		bool newrun = true;
		while (++i < e) {
			if (tbwt_mtf[i] != 0u) { //new run detected
				newrun = true;
			}
			else if (newrun) { //first run-character of new run
				aux[j++] = aux[i]; //copy aux-value of runs with height > 1
				newrun = false;
			}
		}
	}

	//apply RLE0 to aux, thus trimming regular entries
	aux.resize( j );
	rle0_encoder<twobitvector>::encode_string( aux );

	//write result to entropy encoder
	entcoder.reset( aux_encoding::SIGMA+1 );
	for (t_idx_t i = 0; i < aux.size(); i++) {
		entcoder.encode_char( aux[i] );
	}
}

void decode_aux( const t_string_t &tbwt_mtf, t_idx_t tbwt_idx, twobitvector &aux,
                 entropy_decoder<istream> &entcoder ) {
	//do some setup
	aux.resize( tbwt_mtf.size() + 1 );
	rle0_decoder<twobitvector> rle0coder;
	entcoder.reset( aux_encoding::SIGMA+1 );

	//decode run-based aux
	array<t_idx_t,3> bounds = {(t_idx_t)0u,
	                           tbwt_idx,
	                           (t_idx_t)tbwt_mtf.size()}; //don't forget primary index run
	for (t_idx_t ib = 0; ib != 2; ib++) {
		t_idx_t i = bounds[ib];
		t_idx_t e = bounds[ib+1];

		bool newrun = true;
		twobitvector::value_type a = 0; //last character decoded
		aux[i] = aux_encoding::REG; //set flag for first run
		while (++i < e) {
			if (tbwt_mtf[i] != 0u) { //new run
				newrun = true;
				aux[i] = aux_encoding::REG;
			}
			else { //run character
				if (newrun) {
					//fetch next aux run value
					if (!rle0coder.has_next_char()) {
						rle0coder.decode_char( entcoder.decode_char() );
					}
					a = rle0coder.next_char();
					newrun = false;
				}
				aux[i] = a;
			}
		}
	}
	if (rle0coder.has_next_char()) {
		throw invalid_argument("aux is too long for tbwt");
	}

	//write terminator for aux
	aux[tbwt_mtf.size()] = aux_encoding::REG; //append terminator
}

//// TBWT ENCODING AND DECODING ///////////////////////////////////////////////

pair<streamsize,streamsize> tunnel_enc_support::encode_tunneled_bwt(
                                                          t_string_t &&tbwt, t_idx_t tbwt_idx,
                                                          twobitvector &&aux, ostream &out ) {
	//get some information about bwt
	t_size_t tbwt_size = tbwt.size();

	//MTF-Encode BWT
	auto alph = mtf_coder<t_string_t>::compute_alph( tbwt );
	mtf_coder<t_string_t>::transform( tbwt, alph );

	//write header
	block_compressor::write_primitive<t_size_t>( tbwt_size, out );
	block_compressor::write_primitive<t_idx_t>(  tbwt_idx, out );
	//write bwt alphabet
	out.put( (t_uchar_t)alph.size() ); //store alphabet size (note that this stores 0 if full alphabet is used)
	for (t_idx_t i = 0; i < alph.size(); i++) { //and the alphabet itself
		out.put( alph[i] );
	}

	auto pos_before_tbwt = out.tellp();

	//write rle0-encoded tunneled BWT
	rle0_encoder<t_string_t> rle0coder;
	entropy_encoder<ostream> entcoder( out );
	entcoder.reset( alph.size() + 1 ); //reset entropy coder for bwt alphabet

	for (t_idx_t i = 0; i < tbwt.size(); ) {
		do {
			if (i >= tbwt.size())	break;
			//feed rle0-encoder with mtf coded input until some contents can be written
		} while (rle0coder.encode_char( tbwt[i++] ));

		//move the output of the rle0coder to the entropy coder
		while (rle0coder.has_next_enc_char()) {
			entcoder.encode_char( rle0coder.next_enc_char() );
		}
	}

	auto pos_before_aux = out.tellp();

	//write aux encoding
	encode_aux( tbwt, tbwt_idx, move(aux), entcoder );

	//finish encoding
	entcoder.flush();
	return make_pair<streamsize,streamsize>(
	                 (streamoff)pos_before_aux - (streamoff)pos_before_tbwt,
	                 (streamoff)out.tellp()    - (streamoff)pos_before_aux );
}

t_idx_t tunnel_enc_support::decode_tunneled_bwt( t_string_t &tbwt, twobitvector &aux,
                                                 istream &in, streampos end,
                                                 t_size_t max_tbwt_size ) {
	//read header
	auto tbwt_size =   block_compressor::read_primitive<t_size_t>( in );
	auto tbwt_idx =    block_compressor::read_primitive<t_idx_t>( in );
	if (tbwt_size > max_tbwt_size) {
		throw invalid_argument("tunneled bwt is too big");
	}

	//read bwt alphabet
	t_uchar_t alphsize = in.get();
	t_string_t alph( (alphsize > 0u) 
	                 ? alphsize
	                 : numeric_limits<t_uchar_t>::max()+1 ); //remember that on full alphabet 0 is stored
	for (t_idx_t i = 0; i < alph.size(); i++) {
		alph[i] = in.get();
	}

	//read tunneled bwt
	tbwt.resize( tbwt_size );
	rle0_decoder<t_string_t> rle0coder;
	entropy_decoder<istream> entcoder( in, end );
	entcoder.reset( alph.size() + 1 );

	for (t_idx_t i = 0; i < tbwt_size; ) {
		//feed rle0-decoder with input
		rle0coder.decode_char( entcoder.decode_char() );

		//fetch characters from rle0-decoder
		while (i < tbwt_size && rle0coder.has_next_char()) {
			tbwt[i++] = rle0coder.next_char();
		}
	}
	if (rle0coder.has_next_char()) {
		throw invalid_argument("encoded tunneled bwt is longer than expected");
	}

	//read aux using tbwt
	decode_aux( tbwt, tbwt_idx, aux, entcoder );

	//retransform MTF of tbwt
	mtf_coder<t_string_t>::retransform( tbwt, alph );

	return tbwt_idx;
}
