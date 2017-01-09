/*
   RenderText for ESP8266E
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
#include "RenderText.h"
#include "font8x8/font8x8_basic.h"
#include "font8x8/font8x8_ext_latin.h"
#include <bitset>

RenderText::RenderText(const LedControl lc) :
        m_lc(lc){
}

void RenderText::setText(String text, bool loop, int time){
        m_text = text;
        m_time = time;
        for(int x=0; x < m_lc.getDeviceCount() -1; x++) {
                m_text.concat(" ");
        }
        m_loop = loop;
        m_lenText = m_text.length();
        for (int x = 0; x < m_lc.getDeviceCount(); x++) {
                m_lc.shutdown(x,false); // Wake up displays
                m_lc.setIntensity(x,5); // Set intensity levels
                m_lc.clearDisplay(x); // Clear Displays
        }
        reset();

}

void RenderText::reset(){
        m_cntCurrentChar = 0;
        m_cntCurrentBit = 7;
        for(int d = 0; d < m_lc.getDeviceCount(); d++ ) {
                for(int x=0; x<8; x++) {
                        m_buffer[d][x] = 0;
                }
        }

}
byte RenderText::merge(byte target, byte source, byte bit){
        byte result = source << bit;
        result = (result >> 7) | (target << 1);
        return result;
}

byte RenderText::getFontByte(byte numByte){
        char character = m_text[m_cntCurrentChar];
        if(character >= 0x0000 && character <= 0x007F) {
                return font8x8_basic[character][numByte];
        }
        //Unicode characters
        if(character == 194) {
                character = m_text[m_cntCurrentChar+1];
                return font8x8_ext_latin[character-160][numByte];
        }
        //Unicode characters
        if(character == 195) {
                character = m_text[m_cntCurrentChar+1];
                return font8x8_ext_latin[character-96][numByte];
        }
        return 0;
}

void RenderText::handleRender(){
        unsigned long now = millis();
        if(now > m_oldMillis + m_time) {
                m_oldMillis = now;
                if((m_lenText !=0) && (m_cntCurrentChar <= m_lenText)) {
                        for(int d = m_lc.getDeviceCount()-1; d > 0; d-- ) {
                                for(int x=0; x<8; x++) {
                                        m_buffer[d][x] = merge(m_buffer[d][x],m_buffer[d-1][x],0);
                                }
                        }
                        for(int x=0; x<8; x++) {
                                byte curRow = getFontByte(x);
                                m_buffer[0][x] = merge(m_buffer[0][x],curRow,m_cntCurrentBit);
                        }
                        for(int x=0; x< m_lc.getDeviceCount(); x++) {
                                for(int y=0; y<8; y++) {
                                        m_lc.setRow(x,y,m_buffer[x][y]);
                                }
                        }

                        if(m_cntCurrentBit == 0) {
                                //Unicode characters
                                if(m_text[m_cntCurrentChar] > 194) {
                                        m_cntCurrentChar++;
                                }
                                m_cntCurrentChar++;
                                m_cntCurrentBit = 7;
                        } else {
                                m_cntCurrentBit--;
                        }
                } else {
                        if (m_loop) {
                                reset();
                        }
                }
        }
}
