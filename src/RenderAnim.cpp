#include "RenderAnim.h"

RenderAnim::RenderAnim(const LedControl lc) :
        m_lc(lc){
}

void RenderAnim::reset(){
        if (m_fileAnim) {
                m_fileAnim.close();
        }
        m_currentFrame = 0;
        m_numFrames = 0;
}

void RenderAnim::setAnim(String fileName, bool loop, int time){
        m_fileName = fileName;
        m_loop = loop;
        m_time = time;
        reset();
        if (!m_fileAnim) {
                SPIFFS.begin();
                m_fileAnim = SPIFFS.open(m_fileName, "r");
                m_numFrames = m_fileAnim.size() / FRAME_SIZE;
                Serial.print(" m_fileAnim.size():");
                Serial.println(m_fileAnim.size());
                Serial.print("Num frames:");
                Serial.println(m_numFrames);
        }
        for (int x = 0; x < m_lc.getDeviceCount(); x++) {
                m_lc.shutdown(x,false); // Wake up displays
                m_lc.setIntensity(x,5); // Set intensity levels
                m_lc.clearDisplay(x); // Clear Displays
        }
}

void RenderAnim::readFrame(){
        if (m_fileAnim) {
                Serial.print("File open");
                Serial.println(m_fileName);
                int size = sizeof(m_buffer);
                uint8_t buffer[size];
                m_fileAnim.read(buffer, size);
                memcpy(&m_buffer, buffer, size);
        }
}

void RenderAnim::renderFrame(){
        for(int x=0; x< m_lc.getDeviceCount(); x++) {
                for(int y=0; y<8; y++) {
                        m_lc.setRow(x,y,m_buffer[x][y]);
                }
        }
}


void RenderAnim::handleRender(){
        unsigned long now = millis();
        if(now > m_oldMillis + m_time) {
                Serial.print("m_time:");
                Serial.println(m_time);
                m_oldMillis = now;
                if(m_currentFrame < m_numFrames) {
                        readFrame();
                        renderFrame();
                        m_currentFrame++;
                }else {
                        if (m_loop) {
                                m_currentFrame = 0;
                                m_fileAnim.seek(0, SeekSet);
                        } else {
                            reset();
                        }
                }
        }
}
