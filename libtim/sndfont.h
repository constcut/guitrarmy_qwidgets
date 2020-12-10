/*
 * TiMidity -- Experimental MIDI to WAVE converter
 * Copyright (C) 1995 Tuukka Toivonen <toivonen@clinet.fi>
 *
 * sndfont.h: soundfont loader public api
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef TIMIDITY_SNDFONT_H
#define TIMIDITY_SNDFONT_H

#define init_soundfont    TIMI_NAMESPACE(init_soundfont)
#define end_soundfont     TIMI_NAMESPACE(end_soundfont)
#define load_soundfont    TIMI_NAMESPACE(load_soundfont)
#define exclude_soundfont TIMI_NAMESPACE(exclude_soundfont)
#define order_soundfont   TIMI_NAMESPACE(order_soundfont)

void init_soundfont(MidSong *song, const char *fname, int order);
void end_soundfont(void);
MidInstrument *load_soundfont(MidSong *song, int order, int bank, int preset, int keynote);
void exclude_soundfont(int bank, int preset, int keynote);
void order_soundfont(int bank, int preset, int keynote, int order);

#endif /* TIMIDITY_SNDFONT_H */
