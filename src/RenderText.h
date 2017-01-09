
/*
   Render text for ESP8266E
   Copyright (C) 2016 Esteban Familiar Rey

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RENDER_TEXT_H_
#define RENDER_TEXT_H_
#include <stdio.h>
#include "LedControl.h"
#include <Arduino.h>


#define DELAY 100


class RenderText {

public:
								RenderText(LedControl lc);
								void reset();
								void handleRender();
								void setText(String text, bool loop, int time);
private:
								LedControl m_lc;
								String m_text;
								bool m_loop;
								byte m_buffer[4][8];
								int m_cntCurrentChar;
								byte m_cntCurrentBit = 7;
								int m_lenText = 0;
								int m_time;
								byte merge(byte target, byte source, byte bit);
								byte getFontByte(byte numByte);
								unsigned long m_oldMillis;
};

#endif /* RENDER_TEXT_H_ */
